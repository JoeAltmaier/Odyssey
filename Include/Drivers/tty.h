/*
 *
 * MODULE: tty.h - header file for the Exar UART
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
 */

#ifndef		_TTY_
#define		_TTY_


/* prototype decl, common for Odyssey & Eval hardware */

// general functions
extern  void	ttyport_init(I32 port, I32 baud);
extern  STATUS	ttyport_out(I32 port, I32 data);
extern  I32		ttyport_in(I32 port);
extern  STATUS	ttyport_poll(I32 port);

extern  void	ttyA_init(I32 baud);
extern  void	ttyB_init(I32 baud);
extern	void	ttyC_init(I32 baud);
extern	void	ttyD_init(I32 baud);
extern	void	ttyE_init(I32 baud);
extern	void	ttyF_init(I32 baud);
extern	void	ttyG_init(I32 baud);
extern	void	ttyH_init(I32 baud);
extern	STATUS	ttyA_out(I32 data);
extern	STATUS	ttyB_out(I32 data);
extern	STATUS	ttyC_out(I32 data);
extern	STATUS	ttyD_out(I32 data);
extern	STATUS	ttyE_out(I32 data);
extern	STATUS	ttyF_out(I32 data);
extern	STATUS	ttyG_out(I32 data);
extern	STATUS	ttyH_out(I32 data);
extern	I32		ttyA_in();
extern	I32		ttyB_in();
extern	I32		ttyC_in();
extern	I32		ttyD_in();
extern	I32		ttyE_in();
extern	I32		ttyF_in();
extern	I32		ttyG_in();
extern	I32		ttyH_in();
extern	STATUS	ttyA_poll();
extern	STATUS	ttyB_poll();
extern	STATUS	ttyC_poll();
extern	STATUS	ttyD_poll();
extern	STATUS	ttyE_poll();
extern	STATUS	ttyF_poll();
extern	STATUS	ttyG_poll();
extern	STATUS	ttyH_poll();


#ifdef INCLUDE_ODYSSEY  /* Odyssey-native UART hardware (16550-compatible) */

#define		NUM_PORTS		8
/* Define the offsets for the Internal Regs */
#define	ST_RREG_RHR			0
#define	ST_RREG_ISR			2
#define	ST_RREG_LSR			5
#define	ST_RREG_MSR			6

#define	ST_WREG_THR			0
#define	ST_WREG_IER			1
#define	ST_WREG_FCR			2
#define	ST_WREG_LCR			3
#define	ST_WREG_MCR			4

#define	ST_REG_SPR			7
#define	ST_REG_DLL			0
#define	ST_REG_DLM			1

typedef struct {
	volatile unsigned char	holding;
	volatile unsigned char	wier_dlm;
	volatile unsigned char	risr_wfcr;
	volatile unsigned char	wlcr;
	volatile unsigned char	wmcr;
	volatile unsigned char	rlsr;
	volatile unsigned char	rmsr;
	volatile unsigned char	spr;
} uartreg_t;


/* Interrupt Enable Register */
#define	ST_IER_RHR			0x01
#define	ST_IER_THR			0x02
#define	ST_IER_LSR			0x04
#define	ST_IER_MSR			0x08

/* Interrupt Status Register */
#define	ST_ISR_INTMASK		0x0F
#define	ST_ISR_LSR			0x06
#define	ST_ISR_RXRDY		0x04
#define	ST_ISR_RXRDY_TOUT	0x0C
#define	ST_ISR_TXRDY		0x02
#define	ST_ISR_MSR			0x00

#define	ST_ISR_FIFO_ENABLED	0xC0


/* FIFO Control Register */
#define	ST_FCR_ENABLE		0x01
#define	ST_FCR_RCVR_RESET	0x02
#define	ST_FCR_XMIT_RESET	0x04
#define	ST_FCR_DMA			0x08
#define	ST_FCR_TRIGGER4		0x40
#define	ST_FCR_TRIGGER8		0x80
#define	ST_FCR_TRIGGER14	0xC0

/* Line Control Register */
#define	ST_LCR_5BIT			0x00
#define	ST_LCR_6BIT			0x01
#define	ST_LCR_7BIT			0x02
#define	ST_LCR_8BIT			0x03
#define	ST_LCR_STOP1		0x00
#define	ST_LCR_STOP2		0x04

#define	ST_LCR_NOPARITY		0x00
#define	ST_LCR_PARITY		0x08
#define	ST_LCR_ODD			0x00
#define	ST_LCR_EVEN			0x10
#define	ST_LCR_NOFORCE_PAR	0x00
#define	ST_LCR_FORCE_PAR	0x20

#define	ST_LCR_BREAK		0x40
#define	ST_LCR_DL_ENABLE	0x80

/* Modem Control Register */
#define	ST_MCR_DTR			0x01
#define	ST_MCR_RTS			0x02
#define	ST_MCR_OP1			0x04
#define	ST_MCR_OP2_INT		0x08
#define	ST_MCR_LOOPBACK		0x10

/* Modem Status Register */
#define	ST_MSR_CTS			0x10
#define	ST_MSR_DSR			0x20
#define	ST_MSR_RI			0x40
#define	ST_MSR_CD			0x80

/* Line Status Register */
#define	ST_LSR_RXRDY		0x01
#define	ST_LSR_OVERRUN		0x02
#define	ST_LSR_PAR_ERROR	0x04
#define	ST_LSR_FRAME_ERROR	0x08
#define	ST_LSR_BREAK_INT	0x10
#define	ST_LSR_TXRDY		0x20
#define	ST_LSR_TX_EMPTY		0x40
#define	ST_LSR_FIFO_ERROR	0x80

#define RUART(a)	((uartreg_t *)(uartreg[(a)]))


#else INCLUDE_ODYSSEY		/* Eval Board UART */

#define	NUM_PORTS		2
#define TxEMPTY   4
#define RxFULL    1

#define	DLY		{ int tdly =0; tdly++; tdly++; tdly++; if( tdly) tdly++; }

#define CONTROL_A ((volatile char *)0xBD000008)  // channel A
#define DATA_A    ((volatile char *)0xBD00000C)  // channel A
#define CONTROL_B ((volatile char *)0xBD000000)  // channel B
#define DATA_B    ((volatile char *)0xBD000004)  // channel B
#define	UCONTROL(X)	((X)?((volatile char *)0xBD000000):((volatile char *)0xBD000008))
#define	UDATA(X)	((X)?((volatile char *)0xBD000004):((volatile char *)0xBD00000C))
#define	UART_WRITE(port, reg, val) \
	{*(UCONTROL(port)) = reg; DLY; *(UCONTROL(port)) = val; DLY; }

#endif INCLUDE_ODYSSEY

#define	BSIZE	4096
#define	BMASK	(BSIZE - 1)
typedef struct {
	unsigned char	stbuf[BSIZE];
	unsigned int	rptr;
	unsigned int	wptr;
	unsigned int	bcount;
	unsigned int	flags;
} uartbuf_t;

typedef	struct {
	unsigned char *hisr_stack;
	unsigned long rx_count[NUM_PORTS];
	unsigned long tx_count[NUM_PORTS];
	/* Debug Info */
	unsigned long int_count;
	unsigned long hisr_count;
	unsigned long txput_count;
	unsigned long qfull_count;
	unsigned long qunfull_count;
	unsigned long qput_count;
	unsigned long qget_count;
	unsigned long qempty_count;
	unsigned long noq_count;
} uart_t;


/* Definition for Flasgs */
#define	INT_DRIVEN			0x01
#define	BLOCKING			0x02
#define	FLOW_X				0x04
#define	FLOW_HARDWARE		0x08
#define	RUNNING				0x10

/* Ioctl args */
#define	FIOCINT				1
#define	FIOCINTNO			2
#define	FIOCBLOCKING		3
#define	FIOCBLOCKINGNO		4
#define	FIOCFLOW_X			5
#define	FIOCFLOW_XNO		6
#define	FIOCFLOW_HARDWARE	7
#define	FIOCFLOW_HARDWARENO	8
#define	FIOCRESET_COUNTERS	9
#define	FIOCBAUD			10

#define	TTYTX				1
#define	TTYRX				2


/* Baud Rate */
#define	B2400				2400
#define	B9600				9600
#define	B19200				19200
#define	B38400				38400
#define	B57600				57600

#define	XON					0x11
#define	XOFF				0x13

#define		TTY_SUCCESS				0
#define		TTY_FAIL				-1

#define		TTY_HISR_STACK_SIZE	4096
#define		ISR_TXRDY		0x02
#define		ISR_RXRDY		0x04

/* Q related Macros */
#define	TTYRUNNING(X)	(((X)->flags & RUNNING)? 1:0)
#define	TTYBLOCKING(X)	(((X)->flags & BLOCKING)? 1:0)
#define	TTYQINIT(X)		{(X)->wptr =0; (X)->rptr =0; (X)->bcount =0;}
#define	TTYQEMPTY(X)	(((X)->bcount == 0)? 1:0)
#define	TTYQFULL(X)		(((X)->bcount >= BSIZE)?1:0 )
#define	TTYQCOUNT(X)	(X)->bcount
#define	TTYQGET(X)		(((X)->bcount--)? (X)->stbuf[(X)->rptr++ & BMASK] :-1)
#define	TTYQPUT(X, Y)	{(X)->stbuf[(X)->wptr++ & BMASK] = Y; (X)->bcount++;}

#ifdef	INCLUDE_ODYSSEY
extern const	uartreg_t	*uartreg[];
/* Hardware I/O related stuff */
#define	TTY_TXREADY(port)	( (RUART((port))->rlsr & ST_LSR_TXRDY) ? 1:0)	
#define	TTY_RXREADY(port)	( (RUART((port))->rlsr & ST_LSR_RXRDY) ? 1:0)	
#define	TTY_TXPUT(port,val)	RUART((port))->holding = (val)
#define	TTY_RXGET(port)		RUART((port))->holding
#define	TTY_TXRESET(port)		

/* Hardware Int realted stuff */
#define	TTY_DISABLE_INT(port)	RUART((port))->wmcr &= ~ST_MCR_OP2_INT
#define	TTY_ENABLE_INT(port)	RUART((port))->wmcr |= ST_MCR_OP2_INT
#define	TTY_TXINTP_CLR(port)
#define	TTY_RXINTP_CLR(port)
#define	TTY_EXTINTP_CLR(port)
#define	TTY_ENABLE_RX(port)		RUART((port))->wier_dlm |= ST_IER_RHR
#define	TTY_DISABLE_RX(port)	RUART((port))->wier_dlm &= ~ST_IER_RHR
#define	TTY_FLUSH_RX(port)		RUART(port)->risr_wfcr  |= ST_FCR_RCVR_RESET
#define	TTY_WATER_MARK			200

#else	INCLUDE_ODYSSEY
/* Hardware I/O related stuff */
#define	TTY_TXREADY(port)	( (*UCONTROL((port)) & TxEMPTY) ? 1:0)	
#define	TTY_RXREADY(port)	( (*UCONTROL((port)) & RxFULL) ? 1:0)	
#define	TTY_TXPUT(port,val)	*UDATA((port)) = (val)
#define	TTY_RXGET(port)		(*UDATA((port)))
#define	TTY_TXRESET(port)	\
		{UART_WRITE((port), 0x05, 0x60); UART_WRITE((port), 0x05, 0x68);}

/* Hardware Int realted stuff */
#define	TTY_DISABLE_INT(port)	UART_WRITE(0, 0x09, 0x00)
#define	TTY_ENABLE_INT(port)	UART_WRITE(0, 0x09, 0x08)
#define	TTY_TXINTP_CLR(port)	UART_WRITE((port), 0x00, 0x28)
#define	TTY_RXINTP_CLR(port)	UART_WRITE((port), 0x00, 0x20)
#define	TTY_EXTINTP_CLR(port)	UART_WRITE((port), 0x00, 0x10)
#define	TTY_ENABLE_RX(port)		UART_WRITE(port, 0x03, 0xC1);
#define	TTY_DISABLE_RX(port)	UART_WRITE(port, 0x03, 0xC0);
#define	TTY_FLUSH_RX(port)		
#define	TTY_WATER_MARK			200

#endif	INCLUDE_ODYSSEY

#endif		/* _TTY_ */
