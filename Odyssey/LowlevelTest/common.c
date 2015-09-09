#include "nucleus.h"
#include "types.h"
#include "pcimap.h"
#include "gt.h"
#include "common.h"


void 
mem_print(U32 start_address, U32 size)
{
	U32 *ptr = (U32 *)start_address;
	int i, j;
	int ch;
	
	j = 1;
	printf("\n\r%08X : ", &ptr[0]);
	for(i=0; i < size/4 ; i++) {
		printf("%08X", ptr[i]);
		
		if ( (j % 20) == 0) {
			while(ttyA_poll()) ;
			ch = ttyA_in();
			if ( ch != '\r')
				return;
		}
		if ( (j % 4) == 0)
			printf("\n\r%08X : ", &ptr[i+1]);
		else
			printf("  ");

		j++;
	}
}


void
mem_write(U32 start, U32 flag, U32 order)
{
	buf_t	*mptr = (buf_t *)start;
	int			i;
	unsigned long long	csum = 0;
	unsigned long long	val;
	
	mptr->size = MSIZE;
	for(i =0; i < MSIZE; i++) {
		val = 0xA5A5A5A5A5A5A5A5;
		csum += val;
		mptr->buf[i] = val;
	}
	mptr->csum = csum;
	mptr->flag = flag;
}


I32
mem_read(U32 start)
{
	buf_t	*mptr = (buf_t *)start;
	int			i;
	unsigned long long 		csum = 0;
	

	for(i =0; i < MSIZE; i++) {
		csum += mptr->buf[i];
	}
	if ( mptr->csum == csum) {
		printf("Checksum OK\n\r");
		return(1);
	} else {
		printf("Checksum Fail........!!!!!!!!!!!!!\n\r");
		return(0);
	}
}


int
chtoi(int ch)
{
	if ( (ch >= '0') && (ch <= '9') )
		return(ch - '0');
	if ( (ch >= 'A') && (ch <= 'Z') )
		return(ch - 'A' + 10);
	if ( (ch >= 'a') && (ch <= 'z') )
		return(ch - 'a' + 10);
	
	return(0);
		
}
