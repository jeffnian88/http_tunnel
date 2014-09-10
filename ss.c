#include "common_header.h"
#include "server_header.h"
char* server_ip,* server_port,* listen_port;

typedef struct tagWEB_CLIENT
{
    int tag_id;
    int ptt_socket;
    int recv_data_socket;
    int send_data_socket;
    char ptt_to_recv_buf[8*1024+1];
    size_t ptt_to_recv_buf_h;
    size_t ptt_to_recv_buf_t;
    char send_to_ptt_buf[8*1024+1];
    size_t send_to_ptt_buf_h;
    size_t send_to_ptt_buf_t;

} WEB_CLIENT;


const int grecv_Content_Length = 100*1024*1024;
int total_recv_data_len = 0;

int gsend_Content_Length = -1;
int total_send_data_len = 0;
char ptt_to_recv_buf[8*1024+1];
size_t ptt_to_recv_buf_h = 0;
size_t ptt_to_recv_buf_t = 0;
char send_to_ptt_buf[8*1024+1];
size_t send_to_ptt_buf_h = 0;
size_t send_to_ptt_buf_t = 0;
int ptt_socket = -1, recv_data_socket = -1, send_data_socket = -1;


int main(int argc, char **argv)
{
    if(argc < 4)
    {
        printf("./server server_ip server_port local_listen_port\r\n");
        exit(-1);
    }
	server_ip = argv[1];
	server_port = argv[2];
	listen_port = argv[3];

    int listen_socket;
    // create listen_socket
    if((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("create listen_sock");
        exit(-1);
    }
    struct sockaddr_in server;
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(listen_port));
    server.sin_addr.s_addr = INADDR_ANY;
    // bind listen_socket
    if((bind(listen_socket, (struct sockaddr *)&server, sizeof(server))) == -1)
    {
        perror("bind listen_sock");
        exit(-1);
    }
#define MAX_CLIENTS 1
    // listen listen_socket
    if((listen(listen_socket, MAX_CLIENTS)) == -1)
    {
        perror("listen listen_sock");
        exit(-1);
    }

    // prepare select multiplexing
    fd_set readfds, oldrfds;
    fd_set writefds, oldwfds;
    int request_socket;
    int result;
	int request_timeout;

    FD_ZERO(&oldrfds);
    FD_ZERO(&oldwfds);
    FD_SET(listen_socket, &oldrfds);
    request_timeout = request_socket = -1;
    // wait timeout initial
	struct timeval wait;
    for(;;)
    {
        memcpy(&readfds, &oldrfds, sizeof(readfds));
        memcpy(&writefds, &oldwfds, sizeof(writefds));
        printf("------------------\r\n");
        printf("Waiting select....\r\n");
        fflush(stdout);
     	wait.tv_sec = 1;
	    wait.tv_usec = 0;
        // add timeout
        if ( (select(10, &readfds, &writefds, NULL, &wait)) < 0)
        {
            perror("select()");
            break;
        }


        // reading ptt_socket
        if((ptt_socket != -1) && (FD_ISSET(ptt_socket, &readfds)))
        {
            printf("Reading ptt_socket(%d)...\r\n\r\n", ptt_socket);
            // check if buffer is not full
            if(ptt_to_recv_buf_h != (ptt_to_recv_buf_t+1)%sizeof(ptt_to_recv_buf))
            {
                result = recv_from_socket_to_circle_buf(ptt_socket, &ptt_to_recv_buf_h, &ptt_to_recv_buf_t, ptt_to_recv_buf, sizeof(ptt_to_recv_buf));
                if(result <= 0)
                {
                    // do something, it will skip following recv&send socket
                    close(ptt_socket);
                    close(recv_data_socket);
                    close(send_data_socket);
                    ptt_socket = -1;
                    recv_data_socket = -1;
                    send_data_socket = -1;

                    FD_ZERO(&oldrfds);
                    FD_ZERO(&oldwfds);
                    FD_SET(listen_socket, &oldrfds);
                    request_socket = -1;

                    if(result < 0)
                    {
                        printf("Line:%d in %s :\t", __LINE__, __FILE__);
                        printf("ptt_socket error.\r\n");
                    }
                    printf("ptt_socket is closed.\r\n");
					continue;
                }
                else
                {
                    printf("ptt_socket to ptt_to_recv_buf [%d]\r\n", result);
                }
            }
            // buffer is not empty
            if((recv_data_socket != -1) && (ptt_to_recv_buf_h != ptt_to_recv_buf_t))
            {
                FD_SET(recv_data_socket, &oldwfds);
            }
        }

        // reading send_data_socket
        if((send_data_socket != -1) && (FD_ISSET(send_data_socket, &readfds)))
        {
            printf("Reading send_data_socket(%d)...\r\n\r\n", send_data_socket);
            // check if buffer is not full
            if(send_to_ptt_buf_h != (send_to_ptt_buf_t+1)%sizeof(send_to_ptt_buf))
            {
                result = recv_from_socket_to_circle_buf(send_data_socket, &send_to_ptt_buf_h, &send_to_ptt_buf_t, send_to_ptt_buf, sizeof(send_to_ptt_buf));
                if(result <= 0)
                {
                    // do something
                    FD_CLR(send_data_socket, &oldrfds);
                    close(send_data_socket);
                    send_data_socket = -1;
                    if(result < 0)
                    {
                        printf("Line:%d in %s :\t", __LINE__, __FILE__);
                        printf("send_data_socket error.\r\n");
                    }
                    printf("send_data_socket is closed.\r\n");
                }
                else
                {
                    total_send_data_len = total_send_data_len + result;
                    printf("send_data_socket to send_to_ptt_buf [%d][%d][c:%d]\r\n", result, total_send_data_len, gsend_Content_Length);
                    if(total_send_data_len == gsend_Content_Length)
                    {
                        FD_CLR(send_data_socket, &oldrfds);
                        close(send_data_socket);
                        send_data_socket = -1;
                        printf("send_data_socket is full and closed.\r\n");
                    }
                }
            }
            // buffer is not empty
            if((ptt_socket != -1) && (send_to_ptt_buf_h != send_to_ptt_buf_t))
            {
                FD_SET(ptt_socket, &oldwfds);
            }
        }

        // reading recv_data_socket
        if((recv_data_socket != -1) && (FD_ISSET(recv_data_socket, &readfds)))
        {
            printf("Reading recv_data_socket(%d)...\r\n\r\n", recv_data_socket);
            char buf[10240];
            result = recv(recv_data_socket, buf, sizeof(buf), 0);
            if(result <= 0)
            {
                    FD_CLR(recv_data_socket, &oldrfds);
                    FD_CLR(recv_data_socket, &oldwfds);
                    close(recv_data_socket);
                    recv_data_socket = -1;
                    if(result < 0)
                    {
                        printf("Line:%d in %s :\t", __LINE__, __FILE__);
                        printf("errno:%d\r\n", errno);
                    }
                    printf("recv_data_socket is closed.\r\n");
            }
            else
            {
                buf[result] = 0;
                printf("Line:%d in %s :\t", __LINE__, __FILE__);
                printf("Unknown Exception\r\n");
                printf("%s\r\n", buf);
            }
        }
        // writing ptt_socket
        if((ptt_socket != -1) && (FD_ISSET(ptt_socket, &writefds)))
        {
            printf("Writing ptt_socket(%d)...\r\n\r\n", ptt_socket);
            // check if buffer is not empty
            if(send_to_ptt_buf_h != send_to_ptt_buf_t)
            {
                result = send_from_circle_buf_to_socket(ptt_socket, &send_to_ptt_buf_h, &send_to_ptt_buf_t, send_to_ptt_buf, sizeof(send_to_ptt_buf), sizeof(send_to_ptt_buf));
                if(result <= 0)
                {
                    // do something
                    close(ptt_socket);
                    close(recv_data_socket);
                    close(send_data_socket);
                    ptt_socket = -1;
                    recv_data_socket = -1;
                    send_data_socket = -1;

                    FD_ZERO(&oldrfds);
                    FD_ZERO(&oldwfds);
                    FD_SET(listen_socket, &oldrfds);
                    request_socket = -1;

                    if(result < 0)
                    {
                        printf("Line:%d in %s :\t", __LINE__, __FILE__);
                        printf("ptt_socket error.\r\n");
                    }
                    printf("ptt_socket is closed.\r\n");
					// skip following code because the disconnection
					continue;
                }
                else
                {
                    printf("send_to_ptt_buf to ptt_socket [%d]\r\n", result);
                }
            }
            // buffer is empty
            if((ptt_socket != -1) && (send_to_ptt_buf_h == send_to_ptt_buf_t))
            {
                FD_CLR(ptt_socket, &oldwfds);
            }
        }

        // writing recv_data_socket
        if((recv_data_socket != -1) && (FD_ISSET(recv_data_socket, &writefds)))
        {
            printf("Writing recv_data_socket(%d)...\r\n\r\n", recv_data_socket);
            // check if buffer is empty
            if(ptt_to_recv_buf_h != ptt_to_recv_buf_t)
            {
                size_t limit_len;
                limit_len = (grecv_Content_Length - total_recv_data_len);
                result = send_from_circle_buf_to_socket(recv_data_socket, &ptt_to_recv_buf_h, &ptt_to_recv_buf_t, ptt_to_recv_buf, sizeof(ptt_to_recv_buf), limit_len);
                if(result <= 0)
                {
                    // do something
                    FD_CLR(recv_data_socket, &oldrfds);
                    FD_CLR(recv_data_socket, &oldwfds);
                    close(recv_data_socket);
                    recv_data_socket = -1;
                    if(result < 0)
                    {
                        printf("Line:%d in %s :\t", __LINE__, __FILE__);
                        printf("recv_data_socket error.\r\n");
                    }
                    printf("recv_data_socket is closed.\r\n");
                }
                else
                {
                    total_recv_data_len = total_recv_data_len + result;
                    printf("ptt_to_recv_buf to recv_data_socket [%d][%d][%d]\r\n", result, total_recv_data_len, grecv_Content_Length);
                    if(total_recv_data_len == grecv_Content_Length)
                    {
                        FD_CLR(recv_data_socket, &oldrfds);
                        FD_CLR(recv_data_socket, &oldwfds);
                        close(recv_data_socket);
                        recv_data_socket = -1;
                        printf("recv_data_socket is full.\r\n");
                    }
                }
            }
            // buffer is empty
            if((recv_data_socket != -1) && (ptt_to_recv_buf_h == ptt_to_recv_buf_t))
            {
                FD_CLR(recv_data_socket, &oldwfds);
            }
        }

        // reading request_socket or listen_socket
        if(request_socket != -1)
        {
            // reading request_socket
            if(FD_ISSET(request_socket, &readfds))
            {
                printf("Reading request_socket(%d)...\r\n\r\n", request_socket);
                REQUEST_TYPE erequest_type = Wrapper_Request_Connection_Reader(request_socket, &gsend_Content_Length);
                switch(erequest_type)
                {
                    // OPEN SERVER CONNECTION
                    case OPEN_SERVER_CONNECTION:
                        printf("OPEN_SERVER_CONNECTION:%d\r\n", ptt_socket);
                        // CONNECT TO PTT
                        if((ptt_socket != -1) && (send_data_socket != -1) && (recv_data_socket != -1))
                        {
							// repeat to request ptt connection
                        	/*
                            FD_CLR(ptt_socket, &oldrfds);
                            FD_CLR(ptt_socket, &oldwfds);
                            close(ptt_socket);
                            printf("close ptt_socket(%d).\r\n", ptt_socket);
                            ptt_socket = -1;
                            ptt_to_recv_buf_h = 0;
                            ptt_to_recv_buf_t = 0;
                            send_to_ptt_buf_h = 0;
                            send_to_ptt_buf_t = 0;
                            */
                            Wrapper_Response_Open_Server_Connection_NOTOK(request_socket);
                        }
                        else
                        {
							if(ptt_socket != -1)
							{
                            	FD_CLR(ptt_socket, &oldrfds);
                            	FD_CLR(ptt_socket, &oldwfds);
                            	close(ptt_socket);
                            	printf("LINE:%d close ptt_socket(%d).\r\n", __LINE__, ptt_socket);
                            	ptt_socket = -1;
							}
							// clear buffer
                            ptt_to_recv_buf_h = 0;
                            ptt_to_recv_buf_t = 0;
                            send_to_ptt_buf_h = 0;
                            send_to_ptt_buf_t = 0;

	                        if(CONNECT_TO_PTT(&ptt_socket) == -1)
	                        {
	                            // response header to request_socket
	                            // close request_socket
	                            printf("Connect to PTT : fail.\r\n");
	                            close(ptt_socket);
	                            ptt_socket = -1;
	                            Wrapper_Response_Open_Server_Connection_NOTOK(request_socket);
	                        }
	                        else
	                        {
	                            // ASSIGN A TAG_ID from USER_LIST
	                            // response header to request_socket
	                            // close request_socket
	                            printf("Connect to PTT : ok.\r\n");
	                            FD_SET(ptt_socket, &oldrfds);
	                            Wrapper_Response_Open_Server_Connection_OK(request_socket);
	                            printf("Create ptt_socket(%d)\r\n", ptt_socket);
	                        }
						}
                    break;

                    // OPEN RECV DATA CONNECTION
                    case OPEN_RECV_DATA_CONNECTION:
                        printf("OPEN_RECV_DATA_CONNECTION:%d\r\n", recv_data_socket);
                        if(ptt_socket != -1)
                        {
                            if(recv_data_socket != -1)
                            {
                            	Wrapper_Response_Open_Recv_Data_Connection_NOTOK(request_socket);
								/*
                                FD_CLR(recv_data_socket, &oldrfds);
                                close(recv_data_socket);
                                printf("close recv_data_socket(%d).\r\n", recv_data_socket);
                                recv_data_socket = -1;
								*/
                            }
							else
							{
								// request socket transfer into recv socket
                            	recv_data_socket = request_socket;
                            	FD_SET(recv_data_socket, &oldrfds);
                            	if(ptt_to_recv_buf_h != ptt_to_recv_buf_t)
                           		{
                                	FD_SET(recv_data_socket, &oldwfds);
                            	}
                            	request_socket = -1;
                            	FD_SET(listen_socket, &oldrfds);
                            	Wrapper_Response_Open_Recv_Data_Connection_OK(recv_data_socket);
                            	total_recv_data_len = 0;
                            	printf("Create recv_data_socket(%d)\r\n", recv_data_socket);
							}
                        }
                        else
                        {
                            Wrapper_Response_Open_Recv_Data_Connection_NOTOK(request_socket);
							/*
                            FD_CLR(request_socket, &oldrfds);
                            close(request_socket);
                            request_socket = -1;
                            FD_SET(listen_socket, &oldrfds);
							*/
                        }
                    break;

                    // SEND DATA CONNECTION
                    case OPEN_SEND_DATA_CONNECTION:
                        printf("OPEN_SEND_DATA_CONNECTION:%d\r\n", send_data_socket);
                        if(ptt_socket == -1)
                        {
                        	FD_CLR(request_socket, &oldrfds);
                        	close(request_socket);
                        	request_socket = -1;
                        	FD_SET(listen_socket, &oldrfds);
                        }
						else
						{
                        	if(send_data_socket != -1)
                        	{
                        		FD_CLR(request_socket, &oldrfds);
                        		close(request_socket);
                        		request_socket = -1;
                        		FD_SET(listen_socket, &oldrfds);
							}
							else
							{
                        		send_data_socket = request_socket;
								total_send_data_len = 0;
		                        FD_SET(send_data_socket, &oldrfds);
        		                request_socket = -1;
                		        FD_SET(listen_socket, &oldrfds);
		                        printf("Create send_data_socket(%d)\r\n", send_data_socket);
							}
						}
                    break;

                    // Delete ALL CONNECTION
                    case CLOSE_ALL_CONNECTION:
                        printf("CLOSE_ALL_CONNECTION\r\n");

                        close(ptt_socket);
                        close(recv_data_socket);
                        close(send_data_socket);
                        ptt_socket = -1;
                        recv_data_socket = -1;
                        send_data_socket = -1;

                        FD_ZERO(&oldrfds);
                        FD_ZERO(&oldwfds);
                        FD_SET(listen_socket, &oldrfds);
                        request_socket = -1;

                        break;
                    break;

                    case CLOSE_REQUEST_SOCKET:
                        printf("CLOSE_REQUEST_SOCKET:%d\r\n", request_socket);

                        FD_CLR(request_socket, &oldrfds);
                        close(request_socket);
                        request_socket = -1;
                        FD_SET(listen_socket, &oldrfds);
                    break;

                    case UNKNOWN_REQUEST:
                        printf("UNKNOWN_REQUEST\r\n");

                        FD_CLR(request_socket, &oldrfds);
                        close(request_socket);
                        request_socket = -1;
                        FD_SET(listen_socket, &oldrfds);
                    break;

                    default:
                        printf("Unknown Exception\r\n");

                        FD_CLR(request_socket, &oldrfds);
                        close(request_socket);
                        request_socket = -1;
                        FD_SET(listen_socket, &oldrfds);
                    break;
                }
            }
			else
			{
				// update request_timeout
				request_timeout--;
				if(request_timeout < 0)
				{
					FD_CLR(request_socket, &oldrfds);
                    close(request_socket);
                    request_socket = -1;
                    FD_SET(listen_socket, &oldrfds);

				}
			}
		}
        else
        {
            if(FD_ISSET(listen_socket, &readfds))
            {
                printf("Reading listen_socket(%d)...\r\n\r\n", listen_socket);
                struct sockaddr_in client_sockaddr;
                int client_sockaddr_len = sizeof(struct sockaddr_in);
                if((request_socket = accept(listen_socket, (struct sockaddr *)&client_sockaddr, &client_sockaddr_len)) == -1)
                {
                    perror("accept");
                }
                printf("New Client from IP %s and port %d\r\n", inet_ntoa(client_sockaddr.sin_addr), ntohs(client_sockaddr.sin_port));
                FD_SET(request_socket, &oldrfds);
                FD_CLR(listen_socket, &oldrfds);
                printf("Create request_socket(%d)\r\n", request_socket);
				request_timeout = 5;
            }
        }
    }
    return 0;
}
