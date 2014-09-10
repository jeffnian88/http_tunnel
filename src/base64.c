#include <sys/types.h>
#include <stdio.h>
#include "base64.h"
inline static int base64_val(char x)\
{
	switch(x){
		case '=': return -1;
		case 'A': return 0;
		case 'B': return 1;
		case 'C': return 2;
		case 'D': return 3;
		case 'E': return 4;
		case 'F': return 5;
		case 'G': return 6;
		case 'H': return 7;
		case 'I': return 8;
		case 'J': return 9;
		case 'K': return 10;
		case 'L': return 11;
		case 'M': return 12;
		case 'N': return 13;
		case 'O': return 14;
		case 'P': return 15;
		case 'Q': return 16;
		case 'R': return 17;
		case 'S': return 18;
		case 'T': return 19;
		case 'U': return 20;
		case 'V': return 21;
		case 'W': return 22;
		case 'X': return 23;
		case 'Y': return 24;
		case 'Z': return 25;
		case 'a': return 26;
		case 'b': return 27;
		case 'c': return 28;
		case 'd': return 29;
		case 'e': return 30;
		case 'f': return 31;
		case 'g': return 32;
		case 'h': return 33;
		case 'i': return 34;
		case 'j': return 35;
		case 'k': return 36;
		case 'l': return 37;
		case 'm': return 38;
		case 'n': return 39;
		case 'o': return 40;
		case 'p': return 41;
		case 'q': return 42;
		case 'r': return 43;
		case 's': return 44;
		case 't': return 45;
		case 'u': return 46;
		case 'v': return 47;
		case 'w': return 48;
		case 'x': return 49;
		case 'y': return 50;
		case 'z': return 51;
		case '0': return 52;
		case '1': return 53;
		case '2': return 54;
		case '3': return 55;
		case '4': return 56;
		case '5': return 57;
		case '6': return 58;
		case '7': return 59;
		case '8': return 60;
		case '9': return 61;
		case '+': return 62;
		case '/': return 63;
	}
	return 0;
}

/**
 * Decode from base64 to base256.
 * @param buf - input character buffer
 * @param len - length of input buffer
 * @param newlen - int updated with the length in base 256
 * @returns a pkg_malloced char buffer with the base256 value
 */
char * base64_decode( const char *buf, unsigned int len, int *newlen )
{
	int i,j,x1,x2,x3,x4, count=0;
	char *out;
	out = (char *)malloc( ( len * 3/4 ) + 8 );
	for(i=0,j=0;i+3<len;i+=4){
		x1=base64_val(buf[i]);
		x2=base64_val(buf[i+1]);
		x3=base64_val(buf[i+2]);if(x3 == -1) count++;
		x4=base64_val(buf[i+3]);if(x4 == -1) count++;
		out[j++]=(x1<<2) | ((x2 & 0x30)>>4);
		out[j++]=((x2 & 0x0F)<<4) | ((x3 & 0x3C)>>2);
		out[j++]=((x3 & 0x03)<<6) | (x4 & 0x3F);
	}
	if (i<len) {
		x1 = base64_val(buf[i]);
		if (i+1<len)
			x2=base64_val(buf[i+1]);
		else
			x2=-1;
		if (i+2<len)
			x3=base64_val(buf[i+2]);
		else
			x3=-1;
		if(i+3<len)
			x4=base64_val(buf[i+3]);
		else x4=-1;
		if (x2!=-1) {
			out[j++]=(x1<<2) | ((x2 & 0x30)>>4);
			if (x3==-1) {
				out[j++]=((x2 & 0x0F)<<4) | ((x3 & 0x3C)>>2);
				if (x4==-1) {
					out[j++]=((x3 & 0x03)<<6) | (x4 & 0x3F);
				}
			}
		}

	}

	out[j] = 0;
	*newlen=j-count;
	return out;
}

/** conversion from int to base64 constant */
char base64[64]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
/**
 * Convert a string from base256 to base64.
 * the output is padded with base64 terminating characters '='
 * @param buf - input character buffer
 * @param len - length of input buffer
 * @param newlen - int updated with the length in base64
 * @returns a pkg_malloced char buffer with the base64 value
 */
char * base64_encode( const char *buf, unsigned int len, int *newlen )
{
	int i,k;
	int triplets,rest;
	char *out,*ptr;

	triplets = len/3;
	rest = len%3;
	out = (char *)malloc( ( triplets * 4 ) + 8 );

	ptr = out;
	for(i=0;i<triplets*3;i+=3){
		k = (((unsigned char) buf[i])&0xFC)>>2;
		*ptr=base64[k];ptr++;

		k = (((unsigned char) buf[i])&0x03)<<4;
		k |=(((unsigned char) buf[i+1])&0xF0)>>4;
		*ptr=base64[k];ptr++;

		k = (((unsigned char) buf[i+1])&0x0F)<<2;
		k |=(((unsigned char) buf[i+2])&0xC0)>>6;
		*ptr=base64[k];ptr++;

		k = (((unsigned char) buf[i+2])&0x3F);
		*ptr=base64[k];ptr++;
	}
	i=triplets*3;
	switch(rest){
		case 0:
			break;
		case 1:
			k = (((unsigned char) buf[i])&0xFC)>>2;
			*ptr=base64[k];ptr++;

			k = (((unsigned char) buf[i])&0x03)<<4;
			*ptr=base64[k];ptr++;

			*ptr='=';ptr++;

			*ptr='=';ptr++;
			break;
		case 2:
			k = (((unsigned char) buf[i])&0xFC)>>2;
			*ptr=base64[k];ptr++;

			k = (((unsigned char) buf[i])&0x03)<<4;
			k |=(((unsigned char) buf[i+1])&0xF0)>>4;
			*ptr=base64[k];ptr++;

			k = (((unsigned char) buf[i+1])&0x0F)<<2;
			*ptr=base64[k];ptr++;

			*ptr='=';ptr++;
			break;
	}
	//fprintf(stderr,"base64=%.*s >> %d\n",ptr-out,out,ptr-out);
    //*ptr = 0;
	*newlen = ptr-out;
	return out;
}
