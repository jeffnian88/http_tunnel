#include "client_header.h"
static char request_connection_open[]=
"GET http://%s:%s/ HTTP/1.1\r\n"
"Host: %s:%s\r\n"
"My-Request: connection_open\r\n"
"Pragma: no-cache\r\n"
"Proxy-Connection: Keep-Alive\r\n"
"Proxy-Authorization: Basic %s\r\n"
"\r\n";
static char request_connection_close[]=
"GET http://%s:%s/ HTTP/1.1\r\n"
"Host: %s:%s\r\n"
"My-Request: connection_close\r\n"
"Pragma: no-cache\r\n"
"Proxy-Connection: Keep-Alive\r\n"
"Proxy-Authorization: Basic %s\r\n"
"\r\n";
static char request_connection_recv_data[]=
"GET http://%s:%s/ HTTP/1.1\r\n"
"Host: %s:%s\r\n"
"My-Request: connection_read_data\r\n"
"Re-Open: %d\r\n"
"Pragma: no-cache\r\n"
"Proxy-Connection: Keep-Alive\r\n"
"Proxy-Authorization: Basic %s\r\n"
"\r\n";
static char request_connection_send_data[]=
"POST http://%s:%s/ HTTP/1.1\r\n"
"Host: %s:%s\r\n"
"My-Request: connection_send_data\r\n"
"Re-Send: %d\r\n"
"Content-Length: %d\r\n"
"Pragma: no-cache\r\n"
"Proxy-Connection: Keep-Alive\r\n"
"Proxy-Authorization: Basic %s\r\n"
"\r\n";
static char request_connection_post_data[]=
"POST http://%s:%s/ HTTP/1.1\r\n"
"Host: %s:%s\r\n"
"My-Request: connection_send_data\r\n"
"Content-Length: %d\r\n"
"Pragma: no-cache\r\n"
"Proxy-Connection: Keep-Alive\r\n"
"Proxy-Authorization: Basic %s\r\n"
"\r\n";
int Wrapper_Request_Open_Server_Connection(int* proxy_server_socket)
{
    if(*proxy_server_socket == -1)
    {
        // create proxy_server_socket
        if((*proxy_server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("create proxy_server_socket:");
            printf("Line:%d in %s\r\n", __LINE__, __FILE__);
            return -1;
        }
        struct sockaddr_in local_proxy_server_sockaddr;
        bzero(&local_proxy_server_sockaddr, sizeof(local_proxy_server_sockaddr));
        local_proxy_server_sockaddr.sin_family = AF_INET;
        local_proxy_server_sockaddr.sin_addr.s_addr = inet_addr(local_proxy_ip);
        local_proxy_server_sockaddr.sin_port = htons(atoi(local_proxy_port));
        // connect proxy_server_socket
        if(connect(*proxy_server_socket, (struct sockaddr*) &local_proxy_server_sockaddr, sizeof(local_proxy_server_sockaddr)) < 0)
        {
            perror("connect");
            printf("Line:%d in %s\r\n", __LINE__, __FILE__);
            return -1;
        }
    }

    // build request
    char request[10240];
    sprintf(request, request_connection_open, remote_proxy_ip, remote_proxy_port, remote_proxy_ip, remote_proxy_port, auth);

    // send request
#if SHOW_MSG == 1
    printf("[send][request]\r\n%s", request);
#endif
    sendn(*proxy_server_socket, request, strlen(request), 0);

    // recv response
    char response[10240];
    int len = 0;
    len = Wrapper_Connection_Read_Header(*proxy_server_socket, response, sizeof(response));
    if(len == -1)
    {
        printf("Line:%d in %s\r\n", __LINE__, __FILE__);
        return -1;
    }
    else
    {
        if(len == 0)
        {
            printf("Line:%d in %s\r\n", __LINE__, __FILE__);
            return -1;
        }
        else
        {
            if(len < 0)
            {
                printf("Unknow Exception\r\n");
                printf("Line:%d in %s\r\n", __LINE__, __FILE__);
                return -1;
            }
            else
            {
                response[len] = 0;
            }
        }
    }
    // parsing response
    if(strstr(response, "My-Response: connection_open_ok\r\n") != NULL)
    {
        // get My-Response
        // get tag_id
        return 0;
    }
    else
    {
        printf("Line:%d in %s\r\n", __LINE__, __FILE__);
        return -1;
    }
}
/*
// @param in - recv_data_socket : which socket to be created and connect
// @return 1 - success
// @return 0 - connection close
// @return -1 : fail
*/
int Wrapper_Request_Open_Recv_Data_Connection(int* recv_data_socket, int* reopen, int* pgrecv_Content_Length)
{
    // recv_data_socket is not valid
    if(*recv_data_socket == -1)
    {
        // create recv_data_socket
        if((*recv_data_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("create recv_data_socket:");
            printf("Line:%d in %s\r\n", __LINE__, __FILE__);
            return -1;
        }
        struct sockaddr_in local_proxy_server_sockaddr;
        bzero(&local_proxy_server_sockaddr, sizeof(local_proxy_server_sockaddr));
        local_proxy_server_sockaddr.sin_family = AF_INET;
        local_proxy_server_sockaddr.sin_addr.s_addr = inet_addr(local_proxy_ip);
        local_proxy_server_sockaddr.sin_port = htons(atoi(local_proxy_port));
        // connect proxy_server_socket
        if(connect(*recv_data_socket, (struct sockaddr*) &local_proxy_server_sockaddr, sizeof(local_proxy_server_sockaddr)) < 0)
        {
            perror("connect");
            printf("Line:%d in %s\r\n", __LINE__, __FILE__);
            return -1;
        }
    }

    // build request
    char request[10240];
    sprintf(request, request_connection_recv_data, remote_proxy_ip, remote_proxy_port, remote_proxy_ip, remote_proxy_port, (*reopen)++, auth);

    // send request
#if SHOW_MSG == 1
    printf("[send][request]\r\n%s", request);
#endif
    int result;
    result = sendn(*recv_data_socket, request, strlen(request), 0);
    if(result == -1)
    {
        printf("Line:%d in %s\r\n", __LINE__, __FILE__);
        return -1;
    }

    // recv response
    char response[1024];
    result = Wrapper_Connection_Read_Header(*recv_data_socket, response, sizeof(response));
    if(result < 0)
    {
        printf("Line:%d in %s\r\n", __LINE__, __FILE__);
        return -1;
    }
    else
    {
        if(result == 0)
        {
            printf("Suddenly close socket when recv response\r\n");
            printf("Line:%d in %s\r\n", __LINE__, __FILE__);
            return 0;
        }
        else
        {
            response[result] = 0;
        }
    }

    // parsing response
    if(strstr(response, "My-Response: connection_read_data_ok\r\n") != NULL)
    {
        char* p = strstr(response, "Content-Length: ");
        if(p != NULL)
        {
            sscanf(p, "Content-Length: %d\r\n", pgrecv_Content_Length);
            return 1;
        }
        else
        {
            printf("Line:%d in %s\r\n", __LINE__, __FILE__);
            return -1;
        }
    }
    else
    {
        printf("Open_Recv_Data_Connection : Rejected.\r\n");
        printf("Line:%d in %s\r\n", __LINE__, __FILE__);
        return -1;
    }
}
/*
// @param in - send_data_socke : which socket to send
// @return 1 : success
// @return 0 : socket close
// @return -1 : fail
*/
int Wrapper_Request_Open_Send_Data_Connection(int* send_data_socket, int* resend)
{
    // send_data_socket is not valid
    if(*send_data_socket == -1)
    {
        // create send_data_socket
        if((*send_data_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("create send_data_socket:");
            printf("Line:%d in %s\r\n", __LINE__, __FILE__);
            return -1;
        }
        struct sockaddr_in local_proxy_server_sockaddr;
        bzero(&local_proxy_server_sockaddr, sizeof(local_proxy_server_sockaddr));
        local_proxy_server_sockaddr.sin_family = AF_INET;
        local_proxy_server_sockaddr.sin_addr.s_addr = inet_addr(local_proxy_ip);
        local_proxy_server_sockaddr.sin_port = htons(atoi(local_proxy_port));
        // connect send_data_socke
        if(connect(*send_data_socket, (struct sockaddr*) &local_proxy_server_sockaddr, sizeof(local_proxy_server_sockaddr)) < 0)
        {
            perror("connect");
            printf("Line:%d in %s\r\n", __LINE__, __FILE__);
            return -1;
        }
    }

    // build request
    char request[40960];
    sprintf(request, request_connection_send_data, remote_proxy_ip, remote_proxy_port, remote_proxy_ip, remote_proxy_port, (*resend)++, gsend_Content_Length, auth);

    // send request
#if SHOW_MSG == 1
    printf("[send][request]\r\n%s", request);
#endif
    int result;
    result = sendn(*send_data_socket, request, strlen(request), 0);
    if(result == -1)
    {
        printf("Wrapper_Request_Open_Send_Data_Connection()->sendn():error\r\n");
        printf("Line:%d in %s\r\n", __LINE__, __FILE__);
        return -1;
    }
    return 1;
}
