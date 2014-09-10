#ifndef COMMON_HEADER_H
#define COMMON_HEADER_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

#define SHOW_MSG 1
int sendn(int socket, const char *buf, size_t len, int flags);
int Wrapper_Connection_Read_Header(int socket, char* buf, size_t limit_len);
int recv_from_socket_to_circle_buf(int sock, size_t* h, size_t* t, char* buf, size_t buf_size);
int send_from_circle_buf_to_socket(int sock, size_t* h, size_t* t, char* buf, size_t buf_size, size_t limit_len);

#endif
