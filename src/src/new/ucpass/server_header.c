#include <string.h>
#include "./base64/base64.h"
char response_connection_open_[]=
"HTTP/1.0 200 OK\r\n"
"My-Response: connection_open_%s\r\n"
"Connection: keep-alive\r\n"
"Content-Length: 0\r\n"
"\r\n";
char response_connection_close_[]=
"HTTP/1.0 200 OK\r\n"
"My-Response: connection_close_%s\r\n"
"Connection: keep-alive\r\n"
"Content-Length: 0\r\n"
"\r\n";
char response_connection_read_data_[]=
"HTTP/1.0 200 OK\r\n"
"My-Response: connection_read_data_%s\r\n"
"Connection: keep-alive\r\n"
"Content-Length: %d\r\n"
"\r\n";
char response_connection_send_data_[]=
"HTTP/1.0 200 OK\r\n"
"My-Response: connection_send_data_%s\r\n"
"Connection: keep-alive\r\n"
"Content-Length: 0\r\n"
"\r\n";
typedef enum
{
    CLOSE_REQUEST_SOCKET=0,
    OPEN_SERVER_CONNECTION=1,
    OPEN_RECV_DATA_CONNECTION=2,
    OPEN_SEND_DATA_CONNECTION=3,
    CLOSE_ALL_CONNECTION=4,
    UNKNOWN_REQUEST=5

} REQUEST_TYPE;
/*
// @param in - request_socket : which socket to recv
// @param out - buf
// @parem inout - limit_len
// @return -1: error
// @return the enum
*/
static char rquest_header_buf[20*1024];
REQUEST_TYPE Wrapper_Request_Connection_Reader(int request_socket)
{
    int result;
    result = Wrapper_Connection_Read_Header(request_socket, rquest_header_buf, sizeof(rquest_header_buf));
    if(result == 0)
    {
        return CLOSE_REQUEST_SOCKET;
    }
    else
    {
        if(result < 0)
        {
            printf("Line:%d in %s\r\n", __LINE__, __FILE__);
            return -1;
        }
    }
    char* str;
    if((str = strstr(rquest_header_buf, "My-Request: connection_open\r\n")) != NULL)
    {
        return OPEN_SERVER_CONNECTION;
    }
    if((str = strstr(rquest_header_buf, "My-Request: connection_read_data\r\n")) != NULL)
    {
        return OPEN_RECV_DATA_CONNECTION;
    }
    if((str = strstr(rquest_header_buf, "My-Request: connection_send_data\r\n")) != NULL)
    {
        if((str = strstr(rquest_header_buf, "Content-Length: ")) != NULL)
        {
            sscanf(str, "Content-Length: %d", &gsend_Content_Length);
            return OPEN_SEND_DATA_CONNECTION;
        }
    }
    if((str = strstr(rquest_header_buf, "My-Request: connection_close\r\n")) != NULL)
    {
        return CLOSE_ALL_CONNECTION;
    }
    printf("[Unknown request]\r\n%s\r\n", rquest_header_buf);
    return UNKNOWN_REQUEST;
}
int CONNECT_TO_PTT(int* ptt_socket)
{
    // create ptt_socket
    if((*ptt_socket = socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
    {
        perror("create ptt_socket:");
        printf("Line:%d in %s\r\n", __LINE__, __FILE__);
        return -1;
    }
    struct sockaddr_in ptt_server_sockaddr;
    bzero(&ptt_server_sockaddr, sizeof(ptt_server_sockaddr));
    ptt_server_sockaddr.sin_family = AF_INET;
    ptt_server_sockaddr.sin_addr.s_addr = inet_addr(server_ip);
    ptt_server_sockaddr.sin_port = htons(atoi(server_port));
    // connect proxy_server_socket
    if(connect(*ptt_socket, (struct sockaddr*) &ptt_server_sockaddr, sizeof(ptt_server_sockaddr)) < 0)
    {
        perror("connect");
        printf("Line:%d in %s\r\n", __LINE__, __FILE__);
        return -1;
    }
    return 0;
}

int Wrapper_Response_Open_Send_Data_Connection_OK(int request_socket)
{
    // build response
    char response[20480];
    sprintf(response, response_connection_send_data_, "ok");

    // send response
#if SHOW_MSG == 1
    printf("[response]\r\n%s", response);
#endif
    int result;
    result = sendn(request_socket, response, strlen(response), 0);
    if(result < 0)
    {
        printf("Line:%d in %s\r\n", __LINE__, __FILE__);
        return -1;
    }
    else
    {
        return result;
    }
}
int Wrapper_Response_Open_Send_Data_Connection_NOTOK(int request_socket)
{
    // build response
    char response[20480];
    sprintf(response, response_connection_send_data_, "notok");

    // send response
#if SHOW_MSG == 1
    printf("[response]\r\n%s", response);
#endif
    int result;
    result = sendn(request_socket, response, strlen(response), 0);
    if(result < 0)
    {
        printf("Line:%d in %s\r\n", __LINE__, __FILE__);
        return -1;
    }
    else
    {
        return result;
    }
}

int Wrapper_Response_Open_Recv_Data_Connection_OK(int request_socket)
{
    // build response
    char response[20480];
    sprintf(response, response_connection_read_data_, "ok", grecv_Content_Length);

    // send response
#if SHOW_MSG == 1
    printf("[response]\r\n%s", response);
#endif
    int result;
    result = sendn(request_socket, response, strlen(response), 0);
    if(result < 0)
    {
        printf("Line:%d in %s\r\n", __LINE__, __FILE__);
        return -1;
    }
    else
    {
        return result;
    }
}
int Wrapper_Response_Open_Recv_Data_Connection_NOTOK(int request_socket)
{
    // build response
    char response[20480];
    sprintf(response, response_connection_read_data_, "notok");

    // send response
#if SHOW_MSG == 1
    printf("[response]\r\n%s", response);
#endif
    int result;
    result = sendn(request_socket, response, strlen(response), 0);
    if(result < 0)
    {
        printf("Line:%d in %s\r\n", __LINE__, __FILE__);
        return -1;
    }
    else
    {
        return result;
    }
}

int Wrapper_Response_Open_Server_Connection_OK(int request_socket)
{
    // build response
    char response[20480];
    sprintf(response, response_connection_open_, "ok");

    // send response
#if SHOW_MSG == 1
    printf("[response]\r\n%s", response);
#endif
    int result;
    result = sendn(request_socket, response, strlen(response), 0);
    if(result < 0)
    {
        printf("Line:%d in %s\r\n", __LINE__, __FILE__);
        return -1;
    }
    else
    {
        return result;
    }
}
int Wrapper_Response_Open_Server_Connection_NOTOK(int request_socket)
{
    // build response
    char response[20480];
    sprintf(response, response_connection_open_, "notok");

    // send response
#if SHOW_MSG == 1
    printf("[response]\r\n%s", response);
#endif
    int result;
    result = sendn(request_socket, response, strlen(response), 0);
    if(result < 0)
    {
        printf("Line:%d in %s\r\n", __LINE__, __FILE__);
        return -1;
    }
    else
    {
        return result;
    }
}
