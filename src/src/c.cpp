#include <stdio.h>
int main()
{
	int N=10;
	int n=1;
	int i;
	int pos = 1;
	for(i=1;i<=N;i++)
	{
		n = n*i;
		while(n%10==0)
		{
			n = n/10;
			pos++;
		}
	}
	printf("%d\r\n", pos);
}
