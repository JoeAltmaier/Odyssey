/*
 *
 * MODULE: tty.c - driver for Exar UART for boot code
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 09/11/98  SK: Created by Sudhir
 * 02/11/99 JSN: Streamlined EV64120 ttyport_in() routine for use at 56K.
 * 02/15/00 JSN: Removed EV64120 table based port initialization.
 *               Replaced 'unsigned char' with 'U8' to match tty.h.
 *
 *
 *
 * This material is a confidential trade secret and proprietary information
 * of ConvergeNet Technologies, Inc. which may not be reproduced, used, sold
 * or transferred to any third party without the prior written consent of
 * ConvergeNet Technologies, Inc.  This material is also copyrighted as an
 * unpublished work under sections 104 and 408 of Title 17 of the United
 * States Code.  Law prohibits unauthorized use, copying or reproduction.
 *
 */


#include "types.h"
#include "hw.h"

#include "tty.h"
#include "system.h"
#ifdef	INCLUDE_ODYSSEY

/* The clock speed might change */
#if 0
const	unsigned long	uart_clk = 1843200;	/* Hz */
#else
const	unsigned long	uart_clk = 7372800;
#endif
/* These addresses of the UART register's will be changed */
const	uartreg_t *uartreg[NUM_PORTS] = { 	
										(uartreg_t *)UART_PORTA_KSEG1_ADDR,
		                       			(uartreg_t *)UART_PORTB_KSEG1_ADDR,
							   			(uartreg_t *)UART_PORTC_KSEG1_ADDR,
							   			(uartreg_t *)UART_PORTD_KSEG1_ADDR,
							   			(uartreg_t *)UART_PORTE_KSEG1_ADDR,
							   			(uartreg_t *)UART_PORTF_KSEG1_ADDR,
							   			(uartreg_t *)UART_PORTG_KSEG1_ADDR,
							   			(uartreg_t *)UART_PORTH_KSEG1_ADDR };

void	ttyport_init(I32 port, I32 baud);
STATUS	ttyport_out(I32 port, I32 data);
I32		ttyport_in(I32 port);
STATUS	ttyport_poll(I32 port);

/* Driver for The Boot Code */

void
ttyA_init(I32 baud)
{
	ttyport_init(0, baud);
}

void
ttyB_init(I32 baud)
{
	ttyport_init(1, baud);
}

void
ttyC_init(I32 baud)
{
	ttyport_init(2, baud);
}

void
ttyD_init(I32 baud)
{
	ttyport_init(3, baud);
}
void
ttyE_init(I32 baud)
{
	ttyport_init(4, baud);
}
void
ttyF_init(I32 baud)
{
	ttyport_init(5, baud);
}
void
ttyG_init(I32 baud)
{
	ttyport_init(6, baud);
}
void
ttyH_init(I32 baud)
{
	ttyport_init(7, baud);
}

STATUS
ttyA_out(I32 data)
{
	return(ttyport_out(0, data));
}

STATUS
ttyB_out(I32 data)
{
	return(ttyport_out(1, data));
}

STATUS
ttyC_out(I32 data)
{
	return(ttyport_out(2, data));
}

STATUS
ttyD_out(I32 data)
{
	return(ttyport_out(3, data));
}
STATUS
ttyE_out(I32 data)
{
	return(ttyport_out(4, data));
}
STATUS
ttyF_out(I32 data)
{
	return(ttyport_out(5, data));
}
STATUS
ttyG_out(I32 data)
{
	return(ttyport_out(6, data));
}
STATUS
ttyH_out(I32 data)
{
	return(ttyport_out(7, data));
}


I32
ttyA_in()
{
	return(ttyport_in(0));
}

I32
ttyB_in()
{
	return(ttyport_in(1));
}
I32
ttyC_in()
{
	return(ttyport_in(2));
}
I32
ttyD_in()
{
	return(ttyport_in(3));
}
I32
ttyE_in()
{
	return(ttyport_in(4));
}
I32
ttyF_in()
{
	return(ttyport_in(5));
}
I32
ttyG_in()
{
	return(ttyport_in(6));
}
I32
ttyH_in()
{
	return(ttyport_in(7));
}

STATUS
ttyA_poll()
{
	return(ttyport_poll(0));
}
STATUS
ttyB_poll()
{
	return(ttyport_poll(1));
}

STATUS
ttyC_poll()
{
	return(ttyport_poll(2));
}

STATUS
ttyD_poll()
{
	return(ttyport_poll(3));
}
STATUS
ttyE_poll()
{
	return(ttyport_poll(4));
}
STATUS
ttyF_poll()
{
	return(ttyport_poll(5));
}
STATUS
ttyG_poll()
{
	return(ttyport_poll(6));
}
STATUS
ttyH_poll()
{
	return(ttyport_poll(7));
}


void
ttyport_init(I32 port, I32 baud)
{
	U16	divisor;

		/* Disable Interrupt, reset FIFO, set data bits/par */
	RUART(port)->wier_dlm  	= 0;
	RUART(port)->risr_wfcr 	= ST_FCR_RCVR_RESET  | ST_FCR_XMIT_RESET
				  | ST_FCR_ENABLE | ST_FCR_TRIGGER8;
	RUART(port)->wlcr		= ST_LCR_8BIT | ST_LCR_STOP1 | ST_LCR_NOPARITY;
	
		/* Set the Baud Rate */
	RUART(port)->wlcr 		|= ST_LCR_DL_ENABLE;
	divisor = uart_clk / ( baud * 16);
	RUART(port)->holding 	= divisor & 0xFF;
	RUART(port)->wier_dlm 	= (divisor & 0xFF00) >> 8;
	RUART(port)->wlcr 		&= ~ST_LCR_DL_ENABLE;
}


STATUS
ttyport_out(I32 port, I32 data)
{
	while(!(RUART(port)->rlsr & ST_LSR_TXRDY))
		;
	RUART(port)->holding = (unsigned char)data;
	return(TTY_SUCCESS);
}

I32
ttyport_in(I32 port)
{
	if(RUART(port)->rlsr & ST_LSR_RXRDY)
		return(RUART(port)->holding);
	else
		return (-1);

}

STATUS
ttyport_poll(I32 port)
{
	if ( RUART(port)->rlsr & ST_LSR_RXRDY )
		return(TTY_SUCCESS);
	else
		return(TTY_FAIL);

}

#if 0
STATUS
tty_loopback()
{
	I32 i,j;
	
	for(i=0; i < NUM_PORTS; i++) {
		ttyport_init(i, 9600);
		/* Init the port i in loopback mode */
		RUART(i)->wmcr |= ST_MCR_LOOPBACK;
		
		/* Send data(0-255) and rx it */
		for(j=0; j < 256; j++) {
			bputc(i, j);
			if(bgetc(i) != j) {
				RUART(i)->wmcr &= ~ST_MCR_LOOPBACK;
				return(TTY_FAIL);
			}
		}	
		RUART(i)->wmcr &= ~ST_MCR_LOOPBACK;
	}
	return(TTY_SUCCESS);
}
#endif



#else				/* Code for Eval Board */
/*
 * Driver code for the serial communications controller (SCC) of the
 * Galileo evaluation boards (EV-64120).  The SCC is an AMD AM85C30 UART.
 * On the EV-64120, PortA is the DB9 and PortB is the 6 pin HDR.
 */

/* The clock speed */
const	unsigned long	uart_clk = 3686400;	/* Hz */

void
ttyA_init(I32 baud)
{
	ttyport_init(0, baud);
}
void
ttyB_init(I32 baud)
{
	ttyport_init(0, baud);
}

void
ttyport_init(I32 port, I32 baud)
{
	U16	divisor;
	int	i;

	if ( port == 0) {
		UART_WRITE(port, 0x09, 0xC0);
		for(i=0; i < 1000; i++) ;
	}

	/* Disable all Interrupts */
	UART_WRITE(port, 0x01, 0x00);

	/* X16 clock , 8 bit sync, 1 stop bit, parity disable */ 
	UART_WRITE(port, 0x04, 0x46);

	/* Interrupt vector */
	UART_WRITE(port, 0x02, 0x00);

	/* Rx  8 bits, rx disable */
	UART_WRITE(port, 0x03, 0xC0);

	/* Tx  8 bits, tx disable */
	UART_WRITE(port, 0x05, 0x60);

	/* Master Int Disable */
	UART_WRITE(port, 0x09, 0x00);
	
	/* NRZ */
	UART_WRITE(port, 0x0A, 0x00);

	/* Tx/Rx = BRG out, TRxC = BRG out */
	UART_WRITE(port, 0x0B, 0x56);
	
	/* set the baud rate */
	divisor = (uart_clk / ( 2 * baud * 16)) - 2;
	UART_WRITE(port, 0x0C, divisor & 0xFF);
	UART_WRITE(port, 0x0D, (divisor & 0xFF00) >> 8);
	
	/* BRG in = RTxC */
	UART_WRITE(port, 0x0E, 0xA0);

	/* BRG Enable */
	UART_WRITE(port, 0x0E, 0xA1);

	/* Rx Enable */
	UART_WRITE(port, 0x03, 0xC1);

	/* Tx Enable */
	UART_WRITE(port, 0x05, 0x68);
}

/*
 * Function to reuturn a charactor from Port A. Assume charactor is there
 */
I32
ttyA_in()
{
	return (ttyport_in((U8 *)CONTROL_A, (U8 *)DATA_A));
}

/*
 * Function to reuturn a charactor from Port B. Assume charactor is there
 */
I32
ttyB_in()
{
	return (ttyport_in((U8 *)CONTROL_B, (U8 *)DATA_B));
}

/*
 * Function that actually does the work for character input
 */
I32
ttyport_in(U8 *cntrl, U8 *data)
{
	int  delay = 0;

	*cntrl = 0;
	DLY;
	if (*cntrl & RxFULL) {
		return (*data & 0xFF);
	} else
		return  (-1);
}


/*
 * Check if a charactor is present in Port A
 */
STATUS
ttyA_poll(void)
{
	return (ttyport_poll((U8 *)CONTROL_A));
}

/*
 * Check if a charactor is present in Port B
 */
STATUS
ttyB_poll(void)
{
	return (ttyport_poll((U8 *)CONTROL_B));
}


STATUS
ttyport_poll(U8 *cntrl)
{
	unsigned char  status = 0;

	*cntrl = 0;
	DLY;
	status = *cntrl;
	if(status & RxFULL) 
		return 0;
	else 
		return 1;
}

/*
 * Send a charactor to Port A
 */
STATUS
ttyA_out(I32 c)
{
	return (ttyport_out((U8 *)CONTROL_A, (U8 *)DATA_A, c));
}

/*
 * Send a charactor to Port B
 */
STATUS
ttyB_out(I32 c)
{
	return (ttyport_out((U8 *)CONTROL_B, (U8 *)DATA_B, c));
}


STATUS
ttyport_out (U8 *cntrl, U8 *data, I32 c)
{
	int  delay = 0;
	int  status = 0;

	while (1) {
		DISABLE_INT;
		*cntrl = 0;
		DLY;
		status = *cntrl;
		if (status & TxEMPTY) {
			*data = c;
			ENABLE_INT;
			return (0);
		}
		ENABLE_INT;
		delay++;
		if ( delay > 1000) {
			/* Disable tx */
			*cntrl = 5;
			DLY;
			*cntrl = 0x60;
			DLY;
			/* Enable tx */
			*cntrl = 5;
			DLY;
			*cntrl = 0x68;
			DLY;

			/* Disable rx */
			*cntrl = 3;
			DLY;
			*cntrl = 0xC0;
			DLY;

			/* Enable rx */
			*cntrl = 3;
			DLY;
			*cntrl = 0xC1;
			DLY;
			delay = 0;
		}
	}
	return (1);
}


#endif
