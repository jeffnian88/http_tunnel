#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<strings.h>
#include<unistd.h>
#include<arpa/inet.h>
#include <sys/select.h>
#include<errno.h>
#include "common_header.c"

char* local_proxy_ip,* local_proxy_port,* remote_proxy_ip,* remote_proxy_port;

int reopen = 1;
int resend = 1;
#define ERROR   -1
#define MAX_DATA    1024
//#define DEBUG

int grecv_Content_Length = -1;
int gtotal_recv_data_len = 0;

const int gsend_Content_Length = 100*1024*1024;
int gtotal_send_data_len = 0;

#include "client_header.c"

char pcman_to_send_buf[8*1024+1];
size_t pcman_to_send_buf_h = 0;
size_t pcman_to_send_buf_t = 0;
char recv_to_pcman_buf[8*1024+1];
size_t recv_to_pcman_buf_h = 0;
size_t recv_to_pcman_buf_t = 0;
int pcman_socket = -1, recv_data_socket = -1, send_data_socket = -1;

int main(int argc, char **argv)
{
    // command mehtod
#ifndef DEBUG
    if(argc < 6)
    {
        printf("./client local-proxy-ip local-proxy-port remote-proxy-ip remote-proxy-port port\r\n");
        exit(-1);
    }
#else
    if(argc < 5)
    {
        printf("./client local-proxy-ip local-proxy-port remote-proxy-ip remote-proxy-port\r\n");
        exit(-1);
    }
#endif

    // set global variable
    local_proxy_ip = argv[1];
    local_proxy_port = argv[2];
    remote_proxy_ip = argv[3];
    remote_proxy_port = argv[4];

    // initial listen_socket
#ifndef DEBUG
    int listen_socket;
    // create listen_socket
    if((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
    {
        perror("create listen_socket: ");
        exit(-1);
    }
    struct sockaddr_in server;
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[5]));
    server.sin_addr.s_addr = INADDR_ANY;
    // bind listen_socket
    if((bind(listen_socket, (struct sockaddr *)&server, sizeof(server))) == ERROR)
    {
        perror("bind listen_socket: ");
        exit(-1);
    }
#define MAX_CLIENTS 2
    // listen listen_socket
    if((listen(listen_socket, MAX_CLIENTS)) == ERROR)
    {
        perror("listen listen_socket:");
        exit(-1);
    }
#endif


    int result;
    while(1)
    {
        pcman_socket = recv_data_socket = send_data_socket = -1;
        pcman_to_send_buf_h = 0;
        pcman_to_send_buf_t = 0;
        recv_to_pcman_buf_h = 0;
        recv_to_pcman_buf_t = 0;
        reopen = 1;
        resend = 1;

        struct sockaddr_in client_sockaddr;
        int client_sockaddr_len = sizeof(struct sockaddr_in);
        // accept a listen_socket
        printf("-----------------\r\n");
        printf("Waiting Accept...\r\n");
        if((pcman_socket = accept(listen_socket, (struct sockaddr *)&client_sockaddr, &client_sockaddr_len)) == ERROR)
        {
            perror("accept listen_socket");
            continue;
        }
        printf("New Client connected from port no %d and IP %s\n", ntohs(client_sockaddr.sin_port), inet_ntoa(client_sockaddr.sin_addr));


        int proxy_server_socket = -1;
        printf("Open Server Connection...\r\n");
        result = Wrapper_Request_Open_Server_Connection(&proxy_server_socket);
        if(result == -1)
        {
            close(pcman_socket);
            close(proxy_server_socket);
            continue;
        }
        close(proxy_server_socket);
        printf("Open Server Connection ok..\r\n\r\n");




        // Open Recv Data Connection
        printf("Open Recv Data Connection...\r\n");
        result = Wrapper_Request_Open_Recv_Data_Connection(&recv_data_socket);
        if(result == -1)
        {
            close(pcman_socket);
            close(recv_data_socket);
            continue;
        }
        gtotal_recv_data_len = 0;
        printf("Open Recv Data Connection ok..\r\n\r\n");

        // Open Send Data Connection
        printf("Open Send Data Connection...\r\n");
        result = Wrapper_Request_Open_Send_Data_Connection(&send_data_socket);
        if(result == -1)
        {
            close(pcman_socket);
            close(recv_data_socket);
            close(send_data_socket);
            continue;
        }
        gtotal_send_data_len = 0;
        printf("Open Send Data Connection ok..\r\n\r\n");

        // prepare select multiplexing
        // fd initial
        fd_set readfds, writefds, oldrfds, oldwfds;
        FD_ZERO(&oldrfds);
        FD_ZERO(&oldwfds);
        FD_SET(pcman_socket, &oldrfds);
        FD_SET(recv_data_socket, &oldrfds);
        FD_SET(send_data_socket, &oldrfds);

        // wait timeout initial
        struct timeval wait;
        wait.tv_sec = 1;
        wait.tv_usec = 0;

        for(;;)
        {
            memcpy(&readfds, &oldrfds, sizeof(readfds));
            memcpy(&writefds, &oldwfds, sizeof(writefds));
            printf("------------------\r\n");
            printf("Waiting select....\r\n");
            fflush(stdout);
            if ( select(10, &readfds, &writefds, NULL, &wait) < 0)
            {
                perror("select()");
                break;
            }

            // reading pcman_socket
            if((pcman_socket != -1) && (FD_ISSET(pcman_socket, &readfds)))
            {
                printf("Reading pcman_socket(%d)...\r\n\r\n", pcman_socket);
                // check if buffer is full
                if(pcman_to_send_buf_h != (pcman_to_send_buf_t+1)%sizeof(pcman_to_send_buf))
                {
                    result = recv_from_socket_to_circle_buf(pcman_socket, &pcman_to_send_buf_h, &pcman_to_send_buf_t, pcman_to_send_buf, sizeof(pcman_to_send_buf));
                    if(result < 0)
                    {
                        // pcman_socket error
                        // do something
                        close(pcman_socket);
                        close(recv_data_socket);
                        close(send_data_socket);

                        printf("Line:%d in %s :\t", __LINE__, __FILE__);
                        printf("pcman_socket recv error.\r\n");
                        printf("pcman_socket close socket.\r\n");
                        break;// can be replaced with accept
                    }
                    else
                    {
                        if(result == 0)
                        {
                            // pcman_socket close socket
                            close(pcman_socket);
                            close(recv_data_socket);
                            close(send_data_socket);
                            printf("pcman_socket close socket.\r\n");
                            break;// can be replaced with accept
                        }
                        else
                        {
                            printf("pcman_socket to pcman_to_send_buf [%d]\r\n", result);
                        }
                    }
                }
                // buffer is not empty
                if(pcman_to_send_buf_h != pcman_to_send_buf_t)
                {
                    if(send_data_socket != -1)
                    {
                        FD_SET(send_data_socket, &oldwfds);
                    }
                }
            }

            // reading recv_data_socket
            if((recv_data_socket != -1) && (FD_ISSET(recv_data_socket, &readfds)))
            {
                printf("Reading recv_data_socket(%d)...\r\n\r\n", recv_data_socket);
                // check if buffer is full
                if(recv_to_pcman_buf_h != (recv_to_pcman_buf_t+1)%sizeof(recv_to_pcman_buf))
                {
                    result = recv_from_socket_to_circle_buf(recv_data_socket, &recv_to_pcman_buf_h, &recv_to_pcman_buf_t, recv_to_pcman_buf, sizeof(recv_to_pcman_buf));
                    if(result < 0)
                    {
                        // do something
                        // recv_data_socket error
                        FD_CLR(recv_data_socket, &oldrfds);
                        close(recv_data_socket);
                        recv_data_socket = -1;
                        printf("Line:%d in %s :\t", __LINE__, __FILE__);
                        printf("recv_data_socket recv error.\r\n");
                        printf("recv_data_socket close socket.\r\n");
                    }
                    else
                    {
                        if(result == 0)
                        {
                            // recv_data_socket close socket
                            FD_CLR(recv_data_socket, &oldrfds);
                            close(recv_data_socket);
                            recv_data_socket = -1;
                            printf("recv_data_socket close socket\r\n");
                        }
                        else
                        {
                            gtotal_recv_data_len = gtotal_recv_data_len + result;
                            printf("recv_data_socket to recv_to_pcman_buf [%d][%d]\r\n", result, gtotal_recv_data_len);
                            if(gtotal_recv_data_len == grecv_Content_Length)
                            {
                                FD_CLR(recv_data_socket, &oldrfds);
                                close(recv_data_socket);
                                recv_data_socket = -1;
                                printf("recv_data_socket close socket.\r\n");
                            }
                        }
                    }
                }
                // buffer is not empty
                if(recv_to_pcman_buf_h != recv_to_pcman_buf_t)
                {
                    if(pcman_socket != -1)
                    {
                        FD_SET(pcman_socket, &oldwfds);
                    }
                }
            }

            // reading send_data_socket
            if((send_data_socket != -1) && (FD_ISSET(send_data_socket, &readfds)))
            {
                char buf[1024];
                int rbyte;
                rbyte = recv(send_data_socket, buf, sizeof(buf), 0);
                if(rbyte < 0)
                {
                    FD_CLR(send_data_socket, &oldrfds);
                    FD_CLR(send_data_socket, &oldwfds);
                    close(send_data_socket);
                    send_data_socket = -1;
                    printf("Line:%d in %s :\t", __LINE__, __FILE__);
                    printf("send_data_socket recv error, errno:%d\r\n", errno);
                    printf("send_data_socket close socket.\r\n");
                }
                else
                {
                    if(rbyte == 0)
                    {
                        FD_CLR(send_data_socket, &oldrfds);
                        FD_CLR(send_data_socket, &oldwfds);
                        close(send_data_socket);
                        send_data_socket = -1;
                        printf("send_data_socket close socket.\r\n");
                    }
                    else
                    {
                        buf[rbyte] = 0;
                        printf("Line:%d in %s :\t", __LINE__, __FILE__);
                        printf("Unknown Exception\r\n");
                        printf("%s\r\n", buf);
                    }
                }
            }

            // writing send_data_socket
            if((send_data_socket != -1) && (FD_ISSET(send_data_socket, &writefds)))
            {
                printf("Writing send_data_socket(%d)...\r\n\r\n", send_data_socket);
                // check if buffer is not empty & limie_len is not zero
                size_t limit_len;
                limit_len = (gsend_Content_Length - gtotal_send_data_len);
                if((pcman_to_send_buf_h != pcman_to_send_buf_t) && (limit_len != 0))
                {
                    result = send_from_circle_buf_to_socket(send_data_socket, &pcman_to_send_buf_h, &pcman_to_send_buf_t, pcman_to_send_buf, sizeof(pcman_to_send_buf), limit_len);
                    if(result < 0)
                    {
                        // do something
                        FD_CLR(send_data_socket, &oldrfds);
                        FD_CLR(send_data_socket, &oldwfds);
                        close(send_data_socket);
                        send_data_socket = -1;
                        printf("Line:%d in %s :\t", __LINE__, __FILE__);
                        printf("send_data_socket send error.\r\n");
                    }
                    else
                    {
                        if(result == 0)
                        {
                            FD_CLR(send_data_socket, &oldrfds);
                            FD_CLR(send_data_socket, &oldwfds);
                            close(send_data_socket);
                            send_data_socket = -1;
                            printf("Line:%d in %s :\t", __LINE__, __FILE__);
                            printf("send_data_socket close socket.\r\n");
                        }
                        else
                        {
                            gtotal_send_data_len = gtotal_send_data_len + result;
                            printf("pcman_to_send_buf to send_data_socket [%d][%d]\r\n", result, gtotal_send_data_len);
                            if(gtotal_send_data_len == gsend_Content_Length)
                            {
                                printf("send_data_socket is full\r\n");
                            }
                        }
                    }
                }
                // buffer is empty
                if((send_data_socket != -1) && (pcman_to_send_buf_h == pcman_to_send_buf_t))
                {
                    FD_CLR(send_data_socket, &oldwfds);
                }
            }

            // writing pcman_socket
            if((pcman_socket != -1) && (FD_ISSET(pcman_socket, &writefds)))
            {
                printf("Writing pcman_socket(%d)...\r\n\r\n", pcman_socket);
                // check if buffer is empty
                if(recv_to_pcman_buf_h != recv_to_pcman_buf_t)
                {
                    result = send_from_circle_buf_to_socket(pcman_socket, &recv_to_pcman_buf_h, &recv_to_pcman_buf_t, recv_to_pcman_buf, sizeof(recv_to_pcman_buf), sizeof(recv_to_pcman_buf));
                    if(result < 0)
                    {
                        // pcman_socket error
                        // do something
                        close(pcman_socket);
                        close(recv_data_socket);
                        close(send_data_socket);

                        printf("Line:%d in %s :\t", __LINE__, __FILE__);
                        printf("pcman_socket recv error.\r\n");
                        printf("pcman_socket close socket.\r\n");
                        break;// can be replaced with accept
                    }
                    else
                    {
                        if(result == 0)
                        {
                            // pcman_socket close socket
                            close(pcman_socket);
                            close(recv_data_socket);
                            close(send_data_socket);

                            printf("Line:%d in %s :\t", __LINE__, __FILE__);
                            printf("pcman_socket send error.\r\n");
                            printf("pcman_socket close socket.\r\n");
                            break;// can be replaced with accept
                        }
                        else
                        {
                            printf("recv_to_pcman_buf to pcman_socket [%d]\r\n", result);
                        }
                    }
                }
                // buffer is empty
                if((pcman_socket!=-1) && (recv_to_pcman_buf_h == recv_to_pcman_buf_t))
                {
                    FD_CLR(pcman_socket, &oldwfds);
                }
            }

            // open recv_data_socket
            if(recv_data_socket == -1)
            {
                // Open Recv Data Connection
                printf("Waiting Connection Recv Data Open...\r\n");
                result = Wrapper_Request_Open_Recv_Data_Connection(&recv_data_socket);
                if(result == -1)
                {
                    close(pcman_socket);
                    close(recv_data_socket);
                    close(send_data_socket);
                    break;
                }
                FD_SET(recv_data_socket, &oldrfds);
                gtotal_recv_data_len = 0;
                printf("Open Recv Data Connection ok..\r\n");
            }

            // open send_data_socket
            if(send_data_socket == -1)
            {
                // Open Send Data Connection
                printf("Waiting Connection Send Data Open...\r\n");
                result = Wrapper_Request_Open_Send_Data_Connection(&send_data_socket);
                if(result == -1)
                {
                    close(pcman_socket);
                    close(recv_data_socket);
                    close(send_data_socket);
                    break;
                }
                FD_SET(send_data_socket, &oldrfds);
                if(pcman_to_send_buf_h != pcman_to_send_buf_t)
                {
                    FD_SET(send_data_socket, &oldwfds);
                }
                gtotal_send_data_len = 0;
                printf("Open Send Data Connection ok..\r\n");
            }

        } // for loop select
    } // while loop
    return 0;
}
