#include<stdio.h>
#include "base64/base64.h"
int main()
{
    int len;
    char str[]="jh.nian:su3cl3";
    printf("%s\r\n", base64_encode( str, strlen(str), &len ));
    printf("Line:%d in %s", __LINE__, __FILE__);
    return 0;
}
