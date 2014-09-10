#ifndef SERVER_HEADER_H
#define SERVER_HEADER_H
#include "common_header.h"
typedef enum
{
    CLOSE_REQUEST_SOCKET=0,
    OPEN_SERVER_CONNECTION=1,
    OPEN_RECV_DATA_CONNECTION=2,
    OPEN_SEND_DATA_CONNECTION=3,
    CLOSE_ALL_CONNECTION=4,
    UNKNOWN_REQUEST=5

} REQUEST_TYPE;
extern char* server_ip,* server_port,* listen_port;
extern const int grecv_Content_Length;
/*
// @param in - request_socket : which socket to recv
// @param out - buf
// @parem inout - limit_len
// @return -1: error
// @return the enum
*/
REQUEST_TYPE Wrapper_Request_Connection_Reader(int request_socket, int* pgsend_Content_Length);
int CONNECT_TO_PTT(int* ptt_socket);
int Wrapper_Response_Open_Recv_Data_Connection_OK(int request_socket);
int Wrapper_Response_Open_Recv_Data_Connection_NOTOK(int request_socket);
int Wrapper_Response_Open_Server_Connection_OK(int request_socket);
int Wrapper_Response_Open_Server_Connection_NOTOK(int request_socket);
#endif
