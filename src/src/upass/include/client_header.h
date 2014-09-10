#ifndef CLIENT_HEADER_H
#define CLIENT_HEADER_H
#include "common_header.h"

extern char* local_proxy_ip,* local_proxy_port,* remote_proxy_ip,* remote_proxy_port, *auth;
extern const int gsend_Content_Length;
int Wrapper_Request_Open_Server_Connection(int* proxy_server_socket);
/*
// @param in - recv_data_socket : which socket to be created and connect
// @return 1 - success
// @return 0 - connection close
// @return -1 : fail
*/
int Wrapper_Request_Open_Recv_Data_Connection(int* recv_data_socket, int* reopen, int* pgrecv_Content_Length);
/*
// @param in - send_data_socke : which socket to send
// @return 1 : success
// @return 0 : socket close
// @return -1 : fail
*/
int Wrapper_Request_Open_Send_Data_Connection(int* send_data_socket, int* resend);
#endif
