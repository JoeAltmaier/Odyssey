/*  Metrowerks Standard Library  Version 2.1.2b6  1997 April  */

/*
 *	critical_regions.mac.c
 *	
 *		Copyright © 1995-1996 Metrowerks, Inc.
 *		All rights reserved.
 *	
 *	Routines
 *	--------
 *		__init_critical_regions
 *		__begin_critical_region
 *		__end_critical_region
 *
 *	Implementation
 *	--------------
 *
 *		Disable interrupts globally.
 *
 */

#include "critical_regions.h"

long long gCCritical=0;

void __init_critical_regions(void)
{
}

void __kill_critical_regions(void)
{
}

asm void __begin_critical_region(int region)
{
		mfc0 t0,$12;
		nop;
		lui t1, 0xffff;
		ori t1, t1, 0xfffe;
		and $8, $8,$9;
		mtc0 t0,$12;
		ld  t0, gCCritical;
		addiu t0, t0, 1;
		jr ra;
		sd  t0, gCCritical;
}

asm void __end_critical_region(int region)
{
		ld  t0, gCCritical;
		addiu t0, t0, -1;
		beqz  t0, unmask;
		sd  t0, gCCritical;
		jr ra;
		nop;
unmask:
		mfc0 t0,$12;
		nop;
		ori t0,t0,0x0001;
		mtc0 t0,$12;
		nop;
		jr ra;
		nop;
}
/*  Change Record
 *	16-Oct-95 JFH  First code release.
 *  13-Dec-98 Joe Altmaier use interrupt disable.
*/
