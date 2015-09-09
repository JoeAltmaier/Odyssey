/*************************************************************************
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * File: cmbdrv.c
 * 
 * Description:
 * This file contains CMB driver 
 * 
 * Update Log:
 * 08/04/99 Raghava Kondapalli: Created 
 ************************************************************************/

#include "stdio.h"
#include "simple.h"
#include "cmbdrv.h"
#include "system.h"
#include "diags.h"

extern U32 __debug;
static volatile U8	*cmb_datap;

#ifdef CONFIG_E1
#ifdef CONFIG_HBC
static volatile U64	*cmb_statusp;
#else
static volatile U8	*cmb_statusp;
#endif
#else
static volatile U8	*avr_clrp;
#endif

#define MAX_TIME	10000	/* Wait time for CMB to respond in Milliseconds */


#ifndef CONFIG_E1
STATUS	cmb_read_nibble(U8 *data, U32 timeout);
STATUS	cmb_write_nibble(U8 data, U32 timeout);
void	cmb_init(void);
STATUS	cmb_read_byte(U8 *data, U8 check_for_first, U32 timeout);
STATUS	cmb_write_byte(U8 data, U8 is_first, U32 timeout);
#endif

#ifdef CONFIG_E1
STATUS
cmb_read_nibble(U8 *data, U32 timeout)
{
	U32		timeout = 5000;
#ifdef CONFIG_HBC
	U64	t;
#else
	U8	t;
#endif
	
	while (1) {
#ifdef CONFIG_HBC
		t = *(volatile U64 *)cmb_statusp;
#else
		t = *(volatile U8 *)cmb_statusp;
#endif
		if (!(t & CMB_INT_BIT))
		       break;	
		if (timeout) {
			delay_ms(10);
			nms += 10;
			if (nms == MAX_TIME) {
				printf("cmb_read: Timeout\n\r"); 
				return (1);
			}	
		} else {
			delay_ms(10);
			nms += 10;
			if (nms == 10000) {
				printf("cmb_read: Waiting on for AVR to respond\n\r");
				nms = 0;
			}
		}
	}
	*data = *cmb_datap;
	return (0);
}	

STATUS
cmb_write_nibble(U8 data, U32 timeout)
{
	U32	nms = 0;
#ifdef CONFIG_HBC
	U64	t;
#else
	U8	t;
#endif

	while (1) {
#ifdef CONFIG_HBC
		t = *(volatile U64 *)cmb_statusp;
#else
		t = *(volatile U8 *)cmb_statusp;
#endif
		if (!(t & CMB_RDY_FLAG_BIT))
			break;
		if (timeout) {
			delay_ms(10);
			nms += 10;
			if (nms == MAX_TIME) {
				printf("cmb_read: Timeout\n\r"); 
				return (1);
			}	
		} else {
			delay_ms(10);
			nms += 10;
			if (nms == 10000) {
				printf("cmb_write: Waiting on for AVR to respond\n\r");
				nms = 0;
			}
		}
	}
	*cmb_datap = data;
	return (0);
}	

void
cmb_init(void)
{
#ifdef CONFIG_HBC
	U8	dummy;
#else
	U64	dummy;
#endif


	cmb_datap = (volatile U8 *)0xBC0C0000;
	dummy = *cmb_datap;
#ifdef CONFIG_HBC
	cmb_statusp = (volatile U64 *)0xBF000000;
#else
	cmb_statusp = (volatile U8 *)0xBF000001;
#endif

	while (1) {
#ifdef CONFIG_HBC
		dummy = *(volatile U64 *)cmb_statusp;
#else
		dummy = *(volatile U8 *)cmb_statusp;
#endif
		if (!(dummy & CMB_RDY_FLAG_BIT))
			break;
		delay_ms(10);
		nms += 10;
		if (nms == MAX_TIME) {
			printf("cmb_init: Timeout\n\r"); 
			return (1);
		}	
	}
}
#else CONFIG_E1

STATUS
cmb_read_nibble(U8 *data, U32 timeout)
{
	U8	dummy;
	U32	cause;
	U32	nms = 0;

	while (1) {
		cause = r5k_get_cause();
		if (cause & CMB_WRITE_INT_MASK) {
			*data = *cmb_datap;
			dummy = *avr_clrp;	
			return (0);
		}
		if (timeout) {
			delay_ms(1);
			nms += 1;
			if (nms == MAX_TIME) {
				printf("cmb_read: Timeout\n\r"); 
				return (1);
			}	
		} else {
			delay_ms(1);
			nms += 1;
			if (nms == 10000) {
				printf("cmb_read: Waiting for AVR to respond\n\r");
				nms = 0;
			}
		}

	}
	return (1);

}

STATUS
cmb_write_nibble(U8 data, U32 timeout)
{
	U8	dummy;
	U32	cause;
	U32	nms = 0;

	*cmb_datap = data;
	while (1) {
		cause = r5k_get_cause();
		if (cause & CMB_READ_INT_MASK) {
			dummy = *avr_clrp;	
			return (0);
		}
		if (timeout) {
			delay_ms(1);
			nms += 1;
			if (nms == MAX_TIME) {
				printf("cmb_write: Timeout\n\r"); 
				return (1);
			}	
		} else {
			delay_ms(1);
			nms += 1;
			if (nms == 10000) {
				printf("cmb_write: Waiting for AVR to respond\n\r");
				nms = 0;
			}
		}

	}
	return (1);

}

void
cmb_init(void)
{
	U8	dummy;

	cmb_datap = (volatile U8 *)CMB_DATA_REG_ADDR;
	dummy = *cmb_datap;
	avr_clrp = (volatile U8 *)CMB_CLR_REG_ADDR;
	dummy = *avr_clrp;
}

#endif	CONFIG_E1

STATUS
cmb_read_byte(U8 *data, U8 check_for_first, U32 timeout)
{
	U8	n0 = 0;
	U8	n1 = 0;

read_again:
	if ((cmb_read_nibble(&n1, timeout)) != 0) {
		return (1);
	}
	
	if (check_for_first) {
		if ((n1 & FIRST_BYTE_BIT) == 0) {
			goto read_again;

		}
	}
	if (cmb_read_nibble(&n0, timeout) != 0) {
		return (1);
	}
	*data = ((n1 & 0x0F) << 4) | (n0 & 0x0F);	
	return (0);
}

STATUS
cmb_write_byte(U8 data, U8 is_first, U32 timeout)
{
	U8	b;

	b = (data >> 4) & 0x0F;
	b |= 0x10;
	b = (is_first) ? (b | FIRST_BYTE_BIT) : b;
	if (cmb_write_nibble(b, timeout) != 0) {
		return (1);
	}
	if (cmb_write_nibble(data & 0x0F, timeout) != 0) {
		return (1);
	}
	return (0);
}


STATUS
cmb_send_msg(U8 *msgbuf, U32 n, U32 timeout)
{
	int	i = 0;

	if (n == 0) {
		return (0);
	}
	if ((cmb_write_byte(msgbuf[0], 1, timeout))) {
		printf("cmb_send_msg: Error in 1st byte of message\n\r");
		return (1);
	}
	delay_ms(2);
	for (i = 1; i < n;  i++) {
		if ((cmb_write_byte(msgbuf[i], 0, timeout))) {
			printf("cmb_send_msg: Error in send. Sent %d bytes\n\r",
						   	i);
			return (1);
		}
		delay_ms(2);
	}

	if (__debug) {
		printf("\tSend: ");	
		for (i = 0; i < n; i++)
			printf("%02x:", msgbuf[i] & 0xFF);
		printf("\n\r");
	}

	return (0);
}
	      	
STATUS
cmb_read_msg(U8 *msgbuf, U32 *n, U32 timeout)
{
	int	i = 0;
	int	flag = 1;

	/*
	 * Get DST, SRC, CMB, STATUS, COUNT and CRC
	 */
	*n = 0;
	do {
		if ((cmb_read_byte(&msgbuf[i], flag, timeout))) {
			printf("cmb_read_msg: Error in reading hdr. Got %d bytes\n\r", i);
			return (1);
		}
		flag = 0;
		i++;
	} while (i < 6);

	for (;i < (msgbuf[4] + 6); i++) { 
		if ((cmb_read_byte(&msgbuf[i], 0, timeout))) {
			printf("cmb_read_msg: Error in reading load. Got %d bytes\n\r", i);
			return (1);
		}
	}

	*n = msgbuf[4] + 6;

	if (__debug) {
		printf("\tRead: ");	
		for (i = 0; i < (*n); i++)
			printf("%02x:", msgbuf[i] & 0xFF);
		printf("\n\r");
	}
	return (0);
}


