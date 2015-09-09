/*
 *
 * MODULE: system.h - CPU related stuffs
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 12/07/98	- Created by Sudhir
 * 03/04/99 rlw Added coprocessor exception regs group.
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


#ifndef		_SYSTEM_
#define		_SYSTEM_

extern	void	delay_ms(I32 ms);
extern	void	delay_us(U32 us);
extern U32		r5k_get_badvaddr(void);
extern U32		r5k_get_cause(void);
extern U32		r5k_get_compare(void);
extern U32		r5k_get_context(void);
extern U32		r5k_get_count(void);
extern U32		r5k_get_epc(void);
extern	U32		r5k_get_intmask(void);
extern void	r5k_set_compare(U32 val);
extern	void	r5k_set_intmask(U32 sr);

	
/* Disables the Hardware Interrupts, This Macro can be used whenever
 * a critacal section of the code is entered */
#define	DISABLE_INT	(r5k_set_intmask(r5k_get_intmask() & 0xFFFFFFFE))

/* Enable the Hardware Interrupts, This Macro can be used whenever
 * a critical section of the code is executed */
#define	ENABLE_INT	(r5k_set_intmask(r5k_get_intmask() | 0x01))

/* Disable a particular Hardware interrupts, 
 * Input : 0-5
 */
#define	DISABLE_INTX(X)	\
			(r5k_set_intmask(r5k_get_intmask() & (~(0x400 << (X)))))

/* Enable a particular Hardware interrupts, 
 * Input : 0-5
 */
#define	ENABLE_INTX(X)	\
			(r5k_set_intmask(r5k_get_intmask() | (0x400 << (X))))
		
/* Macro for micro seconds delay */
#define	DELAY_US(X)		delay_us((X))

/* Macro for milli seconds delay */
#define	DELAY_MS(X)		delay_ms((X))

#endif		/* _SYSTEM_ */
