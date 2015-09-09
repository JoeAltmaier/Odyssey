/*
 *
 * MODULE: system.c - CPU related stuff
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 12/07/98	- Created by Sudhir
 * 03/04/99 rlw Merged now defunct r5k module for exception registers
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
#include	"mips_reg.h"
#include	"types.h"
#include	"system.h"



#define CLK		(263000000)		/* 225 Mhz */
#define COUNT_DIVISOR	(2)	
#define COUNT_SPEED	(CLK / COUNT_DIVISOR)
#define COUNT_TICKS_US	(COUNT_SPEED / 1000000)	/* Ticks in a microsecond */
void
delay_us(U32 us)
{
	I64	c_tk;
	I64	l_tk;
	I64	ticks;
	I64	c = 0;

	ticks = (us * COUNT_TICKS_US);
	l_tk = r5k_get_count();
	while (1) {
		c_tk = r5k_get_count();
		if (l_tk > c_tk)
			c += (0xFFFFFFFF - l_tk) + c_tk;
		else
			c += (c_tk - l_tk);
		l_tk = c_tk;
		if (c > ticks)
			break;
	}
}
						
/*
 * Delay in Milli seconds
 */
void
delay_ms(I32 ms)
{
	while ((ms--) > 0)
		delay_us(1000);
}

/*
 * Delay in seconds
 */
void
delay(I32 secs)
{
	while ((secs--) > 0)
		delay_us(1000000);
}

// Returns the contents of the CPU Bad Virtual Address register
asm U32
r5k_get_badvaddr()
{
    .set noreorder
    mfc0 t0, C0_BADVADDR
    nop
    jr   ra
    move v0, t0
    .set reorder
}


// Returns the contents of the CPU Cause register
asm U32
r5k_get_cause()
{
    .set noreorder
    mfc0 t0, C0_CAUSE
    nop
    jr   ra
    move v0, t0
    .set reorder
}


// Returns the contents of the CPU Compare register
asm U32
r5k_get_compare()
{
    .set noreorder
    mfc0 t0, C0_COMPARE
    nop
    jr   ra
    move v0, t0
    .set reorder
}


// Returns the contents of the CPU Context register
asm U32
r5k_get_context()
{
    .set noreorder
    mfc0 t0, C0_CONTEXT
    nop
    jr   ra
    move v0, t0
    .set reorder
}


// Returns the contents of the CPU free running Count register
asm U32
r5k_get_count()
{
	.set noreorder
	mfc0	v0, C0_COUNT
	nop
	nop
	nop
	nop
	jr	ra
	nop
	.set reorder
}


// Returns the contents of the CPU Error Program Counter register
asm U32
r5k_get_epc()
{
    .set noreorder
    mfc0 t0, C0_EPC
    nop
    jr   ra
    move v0, t0
    .set reorder
}


// Returns the contents of the CPU Status register
asm	U32
r5k_get_intmask(void)
{
	.set	noreorder
	mfc0	v0, C0_STATUS
	nop
	nop
	nop
	jr		ra
	nop
}


/*************************************************************************/
// Note that r5k_set_count() does not exist!  Not arbitrarily set.
/*************************************************************************/
asm void
r5k_set_compare(U32 val)
{
    .set noreorder
    move t0, a0
    mtc0 t0, C0_COMPARE
    nop
    nop
    nop
    jr   ra
    nop
   .set reorder
}


// Sets the contents of the CPU Status register
asm	void
r5k_set_intmask(U32 sr)
{
	.set	noreorder
	mtc0	a0,	C0_STATUS
	nop
	nop
	nop
	jr		ra
	nop
}

asm	void
r5k_get_config(U32 sr)
{
	.set	noreorder
	mfc0	v0,	C0_CONFIG
	nop
	nop
	nop
	jr		ra
	nop
}



void
wheel(I32 flag)
{
	static unsigned char _wh[] = "|/-\\";
	static int id = 0;
	static U32	next = 0;
	U32	cur;

	cur = r5k_get_count();
	if (cur >= next || flag) {
		printf("\b\b%c ", _wh[id++ & 3]);
		next = cur + (COUNT_SPEED / 2);
	}
}

