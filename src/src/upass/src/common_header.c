#include "common_header.h"
/*
// @param in - socket : which socket to send
// @param in - buf
// @parem in - len
// @parem in - flag
// @return the number of the transmitted data: succeed
// @return -1 : fail
*/
int sendn(int socket, const char *buf, size_t len, int flags)
{
    int total_len = 0, result;
    while(total_len != len)
    {
        result = send(socket, &buf[total_len], len-total_len, flags);
        if(result > 0)
        {
            total_len = total_len + result;
        }
        else
        {
            printf("Line:%d in %s, errno:%d\r\n", __LINE__, __FILE__, errno);
            return -1;
        }
    }
    return total_len;
}
/*
// @param in - socket : which socket to recv
// @param out - buf
// @parem in - limit_len
// @return the number of the received data: succeed and append byte '\0' into last byte
// @return 0 : Socket close/shutdown
// @return -1 : fail
*/
int Wrapper_Connection_Read_Header(int socket, char* buf, size_t limit_len)
{
    size_t len = 0;
    int result;
    while(1)
    {
        if(len < (limit_len-1))
        {
            result = recv(socket, &buf[len], 1, 0);
        }
        else
        {
            printf("Wrapper_Read_Header():Buffer of Header is too small.\r\n");
            printf("Line:%d in %s\r\n", __LINE__, __FILE__);
            return -1;
        }
        if(result == 1)
        {
            len++;
            if(len >= 4)
            {
                if((buf[len-4]=='\r') && (buf[len-3]=='\n') && (buf[len-2]=='\r') && (buf[len-1]=='\n'))
                {
                    buf[len] = 0;
#if SHOW_MSG == 1
                    printf("[Wrapper_Connection_Read_Header]\r\n%s", buf);
#endif
                    return len;
                }
            }
        }
        else
        {
            if(result == 0)
            {
                buf[len] = 0;
                printf("[%u][%s]\r\n", len, buf);
                printf("Line:%d in %s\r\n", __LINE__, __FILE__);
                return 0;
            }
            else
            {
                printf("Line:%d in %s, errno:%d\r\n", __LINE__, __FILE__, errno);
                return -1;
            }
        }
    }
}

/*
// @param in - sock : which socket to recv
// @param in - h : head to buf
// @param in - t : tail to buf
// @parem inout - buf : pointer to buffer
// @return -1 : buffer is full or other error
// @return 0 : Socket close/shutdown
// @return postive number : success
*/
int recv_from_socket_to_circle_buf(int sock, size_t* h, size_t* t, char* buf, size_t buf_size)
{
    int result;
    size_t rbyte;
    // check if buffer is full
    if(*h == ((*t+1)%buf_size))
    {
        printf("Line:%d in %s :\t", __LINE__, __FILE__);
        printf("Buffer is full\r\n");
        return -1;
    }
    else
    {
        if(*h <= *t)
        {
            rbyte = buf_size - *t;
        }
        else
        {
            rbyte = *h - *t -1;
        }
        // check t+rbyte exceed h when h<=t
        if((*t+rbyte)%buf_size == (*h))
        {
            rbyte = rbyte -1;
        }
        // check if rbyte is zero ?
        if(rbyte == 0)
        {
            printf("Line:%d in %s :\t", __LINE__, __FILE__);
            printf("rbyte = 0\r\n");
            return -1;
        }

        result = recv(sock, &buf[*t], rbyte, 0);
        if(result < 0)
        {
            printf("Line:%d in %s, errno:%d\r\n", __LINE__, __FILE__, errno);
            return -1;
        }
        else
        {
            if(result == 0)
            {
                return 0;
            }
            else
            {
                *t = (*t + result) % buf_size;
                return result;
            }
        }
    }
}
/*
// @param in - sock : which socket to send
// @param in - h : head to buf
// @param in - t : tail to buf
// @parem inout - buf : pointer to buffer
// @return -1 : buffer is empty or other error
// @return 0 : Socket close/shutdown
// @return postive number : success
*/

int send_from_circle_buf_to_socket(int sock, size_t* h, size_t* t, char* buf, size_t buf_size, size_t limit_len)
{
    int result;
    size_t wbyte;
    if(*h == *t)
    {
        printf("Line:%d in %s :\t", __LINE__, __FILE__);
        printf("Buffer is empty\r\n");
        return -1;
    }
    else
    {
        if(*h <= *t)
        {
            wbyte = *t - *h;
        }
        else
        {
            wbyte = buf_size - *h;
        }

        // check if wbyte exceed limit_len
        wbyte = wbyte < limit_len ? wbyte:limit_len;
        // cehck if wbyte is zero?
        if(wbyte == 0)
        {
            printf("Line:%d in %s :\t", __LINE__, __FILE__);
            printf("wbyte = 0\r\n");
            return -1;
        }
        // send
        result = send(sock, &buf[*h], wbyte, 0);
        if(result < 0)
        {
            printf("Line:%d in %s, errno:%d\r\n", __LINE__, __FILE__, errno);
            return -1;
        }
        else
        {
            if(result == 0)
            {
                printf("Line:%d in %s\r\n", __LINE__, __FILE__);
                return 0;
            }
            else
            {
                *h = (*h + result) % buf_size;
                return result;
            }
        }
    }
}
