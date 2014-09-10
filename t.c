#include <stdio.h>

extern int __bss_start, _end;

int a, b, c, d;  /* un-initialized */
extern int _start;
int main()
{
		printf("%x %x\n", _start, &_start);
		printf("%x %x\n", __bss_start, &__bss_start);
		printf("%x %x\n", _end, &_end);
		        int *ptr;
				        a = 1, b = 2, c = 3, d = 4;

						        for (ptr = &__bss_start; ptr != &_end; ptr++) {
										                printf("%d\n", *ptr);
														        }
								        return 0;
}
