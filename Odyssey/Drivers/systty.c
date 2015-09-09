/*
 *
 * MODULE: systty.c - driver for Exar UART
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 09/11/98	- Created by Sudhir
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
 * 02/16/99 Jim Frandeen: Make tty_printreg consistent with declaration.
 *
 */



/*
#define	TEST_NONUCLEUS	1
*/
#ifndef	CONFIG_BOOT
#define	__NUCLEUS__	1
#endif
#ifdef	TEST_NONUCLEUS
#include "nucleus.h"
#endif

#ifdef	__NUCLEUS__
#include "nucleus.h"
#endif	__NUCLEUS__
#include "types.h"
#include "hw.h"
#include "tty.h"
#include "systty.h"
#include "system.h"
#include "ioptypes.h"
#include "bootblock.h"



#define	TTY_SYS_SZ		3000
#ifdef	__NUCLEUS__
unsigned long long tty_stack_memory[TTY_SYS_SZ];
/* Data needed for Nucleus stuff */
NU_HISR  	TTY_HISR;
NU_QUEUE	TTY_Tx_Q[NUM_PORTS];
NU_QUEUE	TTY_Rx_Q[NUM_PORTS];
UNSIGNED	tevt[NUM_PORTS],tsize[NUM_PORTS],revt[NUM_PORTS], rsize[NUM_PORTS];
NU_MEMORY_POOL   Tty_System_Memory;
#endif	__NUCLEUS__
void 		(*old_lisr_tty)(int);

/* Interrupt Handlers */
void 		tty_lih(int );
void		ttyisr();

/* UART data structures */
uartbuf_t	urx[NUM_PORTS];
uartbuf_t	utx[NUM_PORTS];
uart_t		uart;
int		num_ports_found = 2;
extern	const	unsigned long	uart_clk;
extern	void	ttyport_init(I32 port, I32 baud);
#ifdef	INCLUDE_ODYSSEY
extern	bootblock_t bootblock;
#endif

/* Function proto for the driver, these functions are not exposed to the
 * User
 */
I32			tty_addintr(void (*pintr)());
void		setopts(I32 port);
void 		setbaud(I32 port, I32 baud);
void		handle_otherint(I32 port);
void		handle_txint(I32 port);
void		handle_rxint(I32 port);
I32			tty_txwait(I32 port, uartbuf_t *ubuf);

/* Debug stuff */
void		tty_print();
void		tty_printreg(I32 port);



void
ttyinit(I32 port, I32 baud)
{
	
	/* Setup LISR and HISR for UART, also set up the Nucleus Queue for
	 * tx and rx events
	 */
	tty_addintr(ttyisr);
	if ( port >= num_ports_found)
		return;

	/* Init/Flush the rx and tx queues */
	TTYQINIT(&utx[port]);
	TTYQINIT(&urx[port]);
	
	/* Intialize the hardware */
	ttyport_init(port, baud);
	
	/* Initialize the local data structures */
	urx[port].flags = BLOCKING | RUNNING;
	utx[port].flags = BLOCKING | RUNNING;
	uart.rx_count[port] = 0;
	uart.tx_count[port] = 0;
	
	/* Initialize the chip with ths current settings
	 * like Interrupt driven, hardware flow control etc.
	 */
	setopts(port);

	/* Enable the CPU interrupt for UART*/
	ENABLE_INTX(DUART_INT_INDEX);
}
U32	*
tty_get_rxcount_ptr(I32 port)
{
	return  (&urx[port].bcount);
}


I32
tty_addintr(void (*pintr)())
{
#ifdef	__NUCLEUS__
	int i;
	STATUS  status;
	unsigned char *mptr1[NUM_PORTS];
	unsigned char *mptr2[NUM_PORTS];

	/* This function should be executed only one, so if this is the
	 * nth time exit
	 */
	if( uart.hisr_stack )
		return(TTY_SUCCESS);
	
	NU_Create_Memory_Pool(&Tty_System_Memory, "TTY_SYSMEM",
						(char *)tty_stack_memory, (8*TTY_SYS_SZ), 56, NU_FIFO);
	
	for(i=0; i < NUM_PORTS; i++) {
		mptr1[i] = (unsigned char *)0;
		mptr2[i] = (unsigned char *)0;
	}
#ifdef	INCLUDE_ODYSSEY
	if ( bootblock.b_type == IOPTY_HBC ) {
		num_ports_found = 6;
		status = NU_Register_LISR(6, tty_lih, &old_lisr_tty);
		if ( status != NU_SUCCESS ) {
			goto tty_addintr_abort;
		}
	}
#endif
	
	/* Rigister LISR */
	status = NU_Register_LISR(DUART_INT_INDEX, tty_lih, &old_lisr_tty);
	if ( status != NU_SUCCESS ) {
		goto tty_addintr_abort;
	}
	
	/* Allocate memory for HISR stack */
	status = NU_Allocate_Memory(&Tty_System_Memory,(void **)&(uart.hisr_stack),
				   TTY_HISR_STACK_SIZE, NU_NO_SUSPEND);	
	if ( status != NU_SUCCESS ) {
		goto tty_addintr_abort;
	}
		
	/* Create HISR */
	status = NU_Create_HISR(&TTY_HISR, "tty_hisr", pintr, 2,
							uart.hisr_stack, TTY_HISR_STACK_SIZE);
	if ( status != NU_SUCCESS ) {
		goto tty_addintr_abort;
	}

	for(i=0; i < NUM_PORTS; i++) {
		/* Allocate memory for Tx and Rx queues */
		status = NU_Allocate_Memory(&Tty_System_Memory,(void **)&(mptr1[i]),
				   1024, NU_NO_SUSPEND);	
		if ( status != NU_SUCCESS ) {
			goto tty_addintr_abort;
		}
		status = NU_Allocate_Memory(&Tty_System_Memory,(void **)&(mptr2[i]),
				   1024, NU_NO_SUSPEND);	
		if ( status != NU_SUCCESS ) {
			goto tty_addintr_abort;
		}
	}

	for(i=0; i < NUM_PORTS; i++) {
		/* Create Tx and Rx queues */
		char	lbuf[10];
		sprintf(lbuf, "TTYTxQ%d", i);
		status = NU_Create_Queue(&TTY_Tx_Q[i], lbuf, mptr1[i], 10, 
					NU_FIXED_SIZE, 1, NU_FIFO);
	
		if ( status != NU_SUCCESS ) {
			goto tty_addintr_abort;
		}
		sprintf(lbuf, "TTYRxQ%d", i);
		status = NU_Create_Queue(&TTY_Rx_Q[i], lbuf, mptr2[i], 10, 
					NU_FIXED_SIZE, 1, NU_FIFO);
	
		if ( status != NU_SUCCESS ) {
			goto tty_addintr_abort;
		}
	}
	
	/* Create Tx and Rx queues */
	

	return(TTY_SUCCESS);
	
tty_addintr_abort:
	
	/* In case of error deallocate the memories */
	if ( uart.hisr_stack)
		NU_Deallocate_Memory(uart.hisr_stack);
	for(i=0; i < NUM_PORTS; i++) {
		if (mptr1[i])
			NU_Deallocate_Memory(mptr1[i]);
		if (mptr2[i])
			NU_Deallocate_Memory(mptr2[i]);

	}
	return(TTY_FAIL);

#else	__NUCLEUS__
#ifdef	TEST_NONUCLEUS
	
	/* Rigister LISR */
	NU_Register_LISR(DUART_INT_INDEX, pintr, &old_lisr_tty);

#else	TEST_NONUCLEUS
#pragma unused (pintr)
#ifdef	INCLUDE_ODYSSEY
	if ( bootblock.b_type == IOPTY_HBC ) {
		num_ports_found = 6;
	}
#endif
#endif	TEST_NONUCLEUS
	return(TTY_SUCCESS);	
#endif	__NUCLEUS__
}

void
ttyioctl(I32 port, I32 cmd, I32 arg)
{

	switch(cmd){
		/* Make Tx and Rx Interrupt driven inividually */
		case FIOCINT:
			if( arg & TTYTX) 
				utx[port].flags |= INT_DRIVEN;

			if( arg & TTYRX) 
				urx[port].flags |= INT_DRIVEN;
			break;
			
		/* Make Tx and Rx polling inividually */
		case FIOCINTNO:
			if( arg & TTYTX) 
				utx[port].flags &= ~INT_DRIVEN;

			if( arg & TTYRX) 
				urx[port].flags &= ~INT_DRIVEN;
			break;
			
		/* Make tx and Rx blocking. In this mode ttyin() and 
		 * ttyout() will wait indefinitely, in case of not ready */
		case FIOCBLOCKING:
			if( arg & TTYTX) 
				utx[port].flags |= BLOCKING;

			if( arg & TTYRX) 
				urx[port].flags |= BLOCKING;
			break;

		/* Make tx and Rx non-blocking. In this mode ttyin() and 
		 * ttyout() will return with error code, if tx not ready
		 * or rx not available 
		 */
		case FIOCBLOCKINGNO:
			if( arg & TTYTX) 
				utx[port].flags &= ~BLOCKING;

			if( arg & TTYRX) 
				urx[port].flags &= ~BLOCKING;
			break;
			
		/* Enable software flow control with XON/XOFF */
		case FIOCFLOW_X:
			utx[port].flags |= FLOW_X;
			break;
			
		/* Disable software flow control with XON/XOFF */
		case FIOCFLOW_XNO:
			utx[port].flags &= ~FLOW_X;
			break;

		/* Not supported */
		case FIOCFLOW_HARDWARE:
			utx[port].flags |= FLOW_HARDWARE;
			break;

		/* Not supported */
		case FIOCFLOW_HARDWARENO:
			utx[port].flags &= ~FLOW_HARDWARE;
			break;
			
		/* Reset any house-keeping counters */
		case FIOCRESET_COUNTERS:
			if( arg & TTYTX) 
				uart.tx_count[port] = 0;

			if( arg & TTYRX) 
				uart.rx_count[port] = 0;
			break;

		/* Set the port speed */
		case FIOCBAUD:
			setbaud(port, arg);
			break;

		default:
			return;
	}
	setopts(port);
}


I32
ttyout(I32 port, I32 data)
{
	uartbuf_t *ubuf = &utx[port];
	int dly =0;

	/* Find out whether tx is stopped due to XOFF*/
	if (!TTYRUNNING(ubuf) && tty_txwait(port, ubuf)) {
		return(-1);
	}

	DISABLE_INT;
	if ( ubuf->flags & INT_DRIVEN) {
		/* if the tx Q is full, wait till at least one char is 
		 * transmitted
		 */
		while(TTYQFULL(ubuf)) {
			if(!TTYBLOCKING(ubuf)) {
				ENABLE_INT;
				return(-1);
			}
			ENABLE_INT;
#ifdef	__NUCLEUS__
			NU_Receive_From_Queue(&TTY_Tx_Q[port], &tevt[port], 1,
									&tsize[port], NU_SUSPEND);
			DISABLE_INT;
#endif	__NUCLEUS__
		}
#ifndef	__NUCLEUS__
		DISABLE_INT;
#endif	__NUCLEUS__
		/* Transmit the first byte, or else we wont get an interrupt*/
		if (TTYQEMPTY(ubuf) && TTY_TXREADY(port)) {
			TTY_TXPUT(port, (unsigned char)data);
			uart.tx_count[port]++;
			ENABLE_INT;
			return(TTY_SUCCESS);
		}
		/* The que is not empty, put the date into the que*/
		TTYQPUT(ubuf, (U8)data);
		uart.tx_count[port]++;
		ENABLE_INT;
		return(TTY_SUCCESS);
	}
	ENABLE_INT;
	/* Polling mode, wait till tx becomes empty */
	while(!TTY_TXREADY(port)) {
		if(!TTYBLOCKING(ubuf)) 
			return(-1);
		dly++;
		/* This kludge is for an unknown bug I found with AMD chip */
		if (dly > 1000) {
			dly = 0;
			TTY_TXRESET(port);
		}
	}
	/* Send the byte */
	TTY_TXPUT(port, (unsigned char)data);
	uart.tx_count[port]++;
	return(TTY_SUCCESS);
}


I32
ttyhit(I32 port)
{
	if ( urx[port].flags & INT_DRIVEN) {
		/* Check at least on byte is present in the rx Q */
		if ( !TTYQEMPTY(&urx[port]))
			return(1);
	} else {
		/* Check whether chip received a byte */
		if ( TTY_RXREADY(port))
			return(1);
	}
	return(0);
}


I32
ttyin(I32 port)
{
	uartbuf_t *ubuf = &urx[port];
	unsigned char val;

	if ( ubuf->flags & INT_DRIVEN) {
		DISABLE_INT;
		/* Wait till at least one byte is available in the rx Queue */
		while(TTYQEMPTY(ubuf)) {
			if( !TTYBLOCKING(ubuf)) {
				ENABLE_INT;
				return(-1);
			}
			ENABLE_INT;
#ifdef	__NUCLEUS__
			NU_Receive_From_Queue(&TTY_Rx_Q[port], &revt[port], 
							1,&rsize[port], NU_SUSPEND);
			DISABLE_INT;
#endif	__NUCLEUS__
		}
		/* Get the data from the rx Queue */
#ifndef	__NUCLEUS__
		DISABLE_INT;
#endif	__NUCLEUS__
		val = TTYQGET(ubuf);
		if ( !TTYRUNNING(ubuf) ) {
			if ( TTYQCOUNT(ubuf) < (BSIZE - TTY_WATER_MARK)) {
				TTYQINIT(ubuf);
				TTY_FLUSH_RX(port);
				ubuf->flags |= RUNNING;
				TTY_ENABLE_RX(port);
			}
		}
		ENABLE_INT;
	} else {
		/* Pollong mode, wait till a byte is available in the chip */
		while(!TTY_RXREADY(port)) {
			if( !TTYBLOCKING(ubuf))
				return(-1);
		}
		/* Read the byte from the chip */
		val = TTY_RXGET(port);
		/* If software flow control is enabled, check whether we have rxed
		 * XON/XOFF
		 */
		if ( utx[port].flags & FLOW_X ) {
			if( val == XOFF ) 
				utx[port].flags &= ~RUNNING;
			if( val == XON ) 
				utx[port].flags |= RUNNING;
		}
	}
	uart.rx_count[port]++;
	return((int)val);

}


I32
tty_txwait(I32 port, uartbuf_t *ubuf)
{
	/* If the tx is stopped due to XOFF, wwait idefinitely
	 * till we receive XON, Exception is non-blocking mode */
	while(!TTYRUNNING(ubuf)) {
		if(!TTYBLOCKING(ubuf)) 
			return(1);
		if(ttyhit(port) && (ttyin(port) == XON))
			ubuf->flags |= RUNNING;
	}
	return(0);
}

void
handle_txint(I32 port)
{
	uartbuf_t *ubuf = &utx[port];

	DISABLE_INT;
	/* If the tx has at least one byte transmit it */
	if (!TTYQEMPTY(ubuf)) {
		if (TTYQFULL(ubuf) && TTYBLOCKING(ubuf)) {
#ifdef	__NUCLEUS__
			/* Wake up the ttyout(), if it is blocked due to 
			 * tx Que being full
			 */
			NU_Send_To_Queue(&TTY_Tx_Q[port], &tevt[port], 1, NU_NO_SUSPEND); 
#endif	__NUCLEUS__
		}
		/* Send the byte */
		TTY_TXPUT(port, (U8)TTYQGET(ubuf));
	}
	ENABLE_INT;
}

void
handle_rxint(I32 port)
{
	uartbuf_t *ubuf = &urx[port];
	unsigned char	val;
#ifdef	__NUCLEUS__
	int qempty;

	DISABLE_INT;
	qempty = TTYQEMPTY(ubuf);
#endif	__NUCLEUS__
	/* Get the data from the chip */
	val = TTY_RXGET(port);
	
	/* If rx Queue is not full, put the data in the Queue,
	 * orelse drop the data 
	 **/
	if (!TTYQFULL(ubuf)) {
		TTYQPUT(ubuf, val);
	} else {
		TTY_DISABLE_RX(port);
		urx[port].flags &= ~RUNNING;
	}
#ifdef	__NUCLEUS__
	/* Wake up the ttyin() thread if it is waiting for the data
	 */
	if(qempty && TTYBLOCKING(ubuf)) {
		NU_Send_To_Queue(&TTY_Rx_Q[port], &revt[port], 1, NU_NO_SUSPEND); 
	}
	ENABLE_INT;
	/* If software flow control is enabled, check whether we have rxed
	 * XON/XOFF
	 */
	if ( utx[port].flags & FLOW_X ) {
		if( val == XOFF ) 
			utx[port].flags &= ~RUNNING;
		if( val == XON ) {
			utx[port].flags |= RUNNING;
			handle_txint(port);
		}
	}
#endif	__NUCLEUS__
}



#ifdef	__NUCLEUS__
void
tty_lih(int vector)
{
	int i = vector;		/* To get rid of the warning */

	/* Disable UART Interrupt */
	for(i=0; i < num_ports_found; i++)
		TTY_DISABLE_INT(i);
	uart.int_count++;
	/* Activate the HISR */
	NU_Activate_HISR(&TTY_HISR);
}
#endif	__NUCLEUS__





/* Chip Dependant code starts from here */

#ifdef INCLUDE_ODYSSEY

void
setopts(I32 port)
{
	if ( utx[port].flags & FLOW_HARDWARE )
		RUART(port)->wmcr = ST_MCR_DTR | ST_MCR_RTS;
	else
		RUART(port)->wmcr &= ~(ST_MCR_DTR | ST_MCR_RTS);
	
#if 0
	/* Enable Modem Status Interrupts */
	RUART(port)->wier_dlm = ST_IER_MSR;
#endif

	/* Enable TX Interrupts */
	if ( utx[port].flags & INT_DRIVEN ) {
		RUART(port)->wier_dlm |= ST_IER_THR;
	} else {
		RUART(port)->wier_dlm &= ~ST_IER_THR;
	}

	/* Enable RX Interrupts */
	if ( urx[port].flags & INT_DRIVEN ) {
		RUART(port)->wier_dlm |= ST_IER_RHR;
	} else {
		RUART(port)->wier_dlm &= ~ST_IER_RHR;
	}
	/* Enable Interrupts */
	RUART(port)->wmcr |= ST_MCR_OP2_INT;
	
}



void
setbaud(I32 port, I32 baud)
{
	unsigned short divisor;

	RUART(port)->wlcr 		|= ST_LCR_DL_ENABLE;
	/* Calculate the time constant */
	divisor = uart_clk / ( baud * 16);
	RUART(port)->holding 		= divisor & 0xFF;
	RUART(port)->wier_dlm 	= (divisor & 0xFF00) >> 8;
	RUART(port)->wlcr 		&= ~ST_LCR_DL_ENABLE;
}


void
handle_otherint(I32 port)
{
	int i;
	i = port;
	printf("LSR : %X\n\r", RUART(port)->rlsr);
	printf("MSR : %X\n\r", RUART(port)->rmsr);
}

void
ttyisr()
{
	int i;
	unsigned char int_sum;
	unsigned int	*flags;
	volatile unsigned char  *rlsr;

	for(i=0; i < num_ports_found; i++ ) {
		flags = &(urx[i].flags);
		rlsr  = &(RUART((i))->rlsr);
		int_sum = RUART(i)->risr_wfcr & ST_ISR_INTMASK;
		while ( !(int_sum & 0x01)) {
			/* Pending Interrupt */
			int_sum &= 0x0E;
			if ( (int_sum == 0) || (int_sum == ST_ISR_LSR)) {
				handle_otherint(i);
			}
			if ( (int_sum == ISR_RXRDY) || (int_sum == ST_ISR_RXRDY_TOUT)) {
				while((*rlsr & ST_LSR_RXRDY)  && (*flags & RUNNING) )
					handle_rxint(i);
			}
			if ( int_sum == ISR_TXRDY )
				handle_txint(i);

			int_sum = RUART(i)->risr_wfcr & ST_ISR_INTMASK;
		}
	}
	
#ifdef	__NUCLEUS__
	/* Enable UART Ints */
	for(i=0; i < num_ports_found; i++)
		TTY_ENABLE_INT(i);
#endif	__NUCLEUS__
}


#else

void
setopts(I32 port)
{
	U8	int_mask = 0;

	if ( utx[port].flags & FLOW_HARDWARE ) {
		/* Disable Tx */
		UART_WRITE(port, 0x05, 0x60);
		/* Enable RTS */
		UART_WRITE(port, 0x05, 0xE2);
		/* Enable  Tx */
		UART_WRITE(port, 0x05, 0xEA);
	} else {
#if 0
		/* Disable Tx */
		UART_WRITE(port, 0x05, 0x60);
		/* Disable RTS, Enable Tx */
		UART_WRITE(port, 0x05, 0x68);
#endif
	}
	

#if	0
	/* Enable Ext Interrupt */
	int_mask = 0x01;
#endif

	/* Enable TX Interrupts */
	if ( utx[port].flags & INT_DRIVEN ) {
		int_mask |= 0x02;
	} 

	/* Enable RX Interrupts */
	if ( urx[port].flags & INT_DRIVEN ) {
		int_mask |= 0x10;
	}

	/* Write the value to the register */
	UART_WRITE(port, 0x01, int_mask);
	
	/* Enable Master Interrupt */
	UART_WRITE(0, 0x09, 0x08);
}



void
setbaud(I32 port, I32 baud)
{
	unsigned short divisor;

	/* Calculate the time constant */
	divisor = (uart_clk / ( 2 * baud * 16)) - 2;
	UART_WRITE(port, 0x0C, divisor & 0xFF);
	UART_WRITE(port, 0x0D, (divisor & 0xFF00) >> 8);

}


void
handle_otherint(I32 port)
{
	U8 val;
	
	*UCONTROL(port) = 0x0F;
	DLY;
	val = *UCONTROL(port);
	DLY;
}

void
ttyisr()
{
	int ttdly = 0;
	unsigned char int_sum;
	

	*UCONTROL(0) = 0x03;
	ttdly++; ttdly++; ttdly++; if(ttdly) ttdly++;
	int_sum = *UCONTROL(0);
	ttdly++; ttdly++; ttdly++; if(ttdly) ttdly++;

	/* tx int, port 1 */
	if ( int_sum & 0x02) {
		TTY_TXINTP_CLR(1);
		handle_txint(1);
	}
	/* tx int, port 0 */
	if ( int_sum & 0x10) {
		TTY_TXINTP_CLR(0);
		handle_txint(0);
	}
	/* Rx int, port 1 */
	if ( int_sum & 0x04) {
		TTY_RXINTP_CLR(1);
		while(TTY_RXREADY(1))
			handle_rxint(1);
	}
	/* Rx int, port 0 */
	if ( int_sum & 0x20) {
		TTY_RXINTP_CLR(0);
		while(TTY_RXREADY(0))
			handle_rxint(0);
	}
	/* Ext int, port 1 */
	if ( int_sum & 0x01) {
		TTY_EXTINTP_CLR(1);
		handle_otherint(1);
	}
	/* port 0 */
	if ( int_sum & 0x80) {
		TTY_EXTINTP_CLR(0);
		handle_otherint(0);
	}
#ifdef	__NUCLEUS__
	TTY_ENABLE_INT(0);
#endif	__NUCLEUS__
}


/* Debug Stuff */

unsigned char	rbuf[50];
int				rlen = 0;

void
tty_storereg(I32 port)
{
	U8	val;
	U8	reg;
	int i;

	i = 0;
	reg = 0;
	*UCONTROL(port) = reg;
	DLY;
	val = *UCONTROL(port);
	rbuf[i] = reg;
	i++;
	rbuf[i] = val;
	i++;

	reg = 1;
	*UCONTROL(port) = reg;
	DLY;
	val = *UCONTROL(port);
	rbuf[i] = reg;
	i++;
	rbuf[i] = val;
	i++;

	reg = 3;
	*UCONTROL(0) = reg;
	DLY;
	val = *UCONTROL(0);
	rbuf[i] = reg;
	i++;
	rbuf[i] = val;
	i++;

	reg = 6;
	*UCONTROL(port) = reg;
	DLY;
	val = *UCONTROL(port);
	rbuf[i] = reg;
	i++;
	rbuf[i] = val;
	i++;

	reg = 7;
	*UCONTROL(port) = reg;
	DLY;
	val = *UCONTROL(port);
	rbuf[i] = reg;
	i++;
	rbuf[i] = val;
	i++;

	reg = 10;
	*UCONTROL(port) = reg;
	DLY;
	val = *UCONTROL(port);
	rbuf[i] = reg;
	i++;
	rbuf[i] = val;
	i++;

	reg = 15;
	*UCONTROL(port) = reg;
	DLY;
	val = *UCONTROL(port);
	rbuf[i] = reg;
	i++;
	rbuf[i] = val;
	i++;
	rlen = i;
		
}
void
tty_printreg(I32 port)
{
	int i;
	
#pragma unused (port)

	for(i=0; i < rlen; i=i+2) 
		printf("Reg%d	: %x\r\n", rbuf[i],  rbuf[i+1]);
}
#endif INCLUDE_ODYSSEY
void
tty_print()
{
	printf("Int Count 	: %d\n\r", uart.int_count);
	printf("hisr_count 	: %d\n\r", uart.hisr_count);
	printf("txput_count 	: %d\n\r", uart.txput_count);
	printf("qfull_count 	: %d\n\r", uart.qfull_count);
	printf("qunfull_count 	: %d\n\r", uart.qunfull_count);
	printf("qput_count 	: %d\n\r", uart.qput_count);
	printf("qget_count 	: %d\n\r", uart.qget_count);
	printf("qempty_count 	: %d\n\r", uart.qempty_count);
	printf("noq_count 	: %d\n\r", uart.noq_count);
	printf("\n\rTotal rx	: %d\n\r", uart.rx_count[1]); 
}
