/* Critical.cpp -- Critical Section (nestable)
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
**/

// $Log: /Gemini/Odyssey/Oos/Critical.cpp $
// 
// 13    1/07/00 5:37p Agusev
// Fixed WIN32 build. 
// Is this the last time or what?!
// 
// 12    12/09/99 2:05a Iowa
// 
// 11    9/01/99 7:33p Iowa
// Cached code/data works!
// Static Send for non-ddm.
// Dma callback properly.
// 
// 10    7/08/99 10:49a Tnelson
// Initial Support for Failover
// 
// 1     6/28/99 11:33a Tnelson
// 
// 9     6/22/99 5:34p Jhatwich
// 
// 8     6/21/99 12:37p Jhatwich
// 
// 7     5/07/99 7:31p Ewedel
// Folded in Carl W's Green Hills asm support, and added EOLs desired by
// Green Hills.
//  4/20/99 Joe Altmaier: WIN32 version
//  3/26/99 Tom Nelson: Create file

#include "OsTypes.h"
#include "Critical.h"

#ifdef _WIN32

#include "windows.h"
#include <stdlib.h>
#include <stdio.h>

static CRITICAL_SECTION singleton_critical;
static BOOL bCriticalInitialized = FALSE;
static BOOL bSingletonEnabled = FALSE;
BOOL Critical::fEnabled;

void Critical::initialize_critical() {
	if(!bCriticalInitialized) {
		bCriticalInitialized = TRUE;
		InitializeCriticalSection(&singleton_critical);
	}
}

initialize_critical();

BOOL Critical::IsEnabled() {
	return fEnabled;
	//return bCriticalEnabled;//TryEnterCriticalSection(&singleton_critical);
}

void Critical::InternalEnable() {
	fEnabled = TRUE;
	Enable();
}

// void Enable();
void Critical::Enable() {
	initialize_critical();
	LeaveCriticalSection(&singleton_critical);
}

void Critical::InternalDisable() {
	Disable();
	fEnabled = FALSE;
}

// void Disable();
void Critical::Disable() {
	if(!bCriticalInitialized)
		int i=0;
	else
		EnterCriticalSection(&singleton_critical);
}

#elif defined(IS_GREEN_HILLS)

// Returns MIPS StatusReg ($12) IntEnable bit (1)
//
// BOOL IsEnabled();
//
BOOL Critical::IsEnabled() { 
	asm (" mfc0 $v0,$12 ");
	asm (" nop ");
	asm (" lui $t0,0 ");
	asm (" ori $t0,$t0,0x0001 ");
	asm (" and $v0,$v0,$t0 ");
	asm (" jr $ra ");
	asm (" nop ");
}

//
// void Enable();
//
void Critical::Enable() { 
	asm (" mfc0 $t0,$12 ");
	asm (" nop ");
	asm (" ori $t0,$t0,0x0001 ");
	asm (" mtc0 $t0,$12 ");
	asm (" nop ");
	asm (" jr $ra ");
	asm (" nop ");
}
//
// void Disable();
//
void Critical::Disable() {
	asm (" mfc0 $t0,$12 ");
	asm (" nop ");
	asm (" lui $t1, 0xffff ");
	asm (" ori $t1, $t1, 0xfffe ");
	asm (" and $8, $8,$9 ");
	asm (" mtc0 $t0,$12 ");
	asm (" nop ");
	asm (" jr $ra ");
	asm (" nop ");
}

#else//MIPS with Metrowerks CodeWarrior

// Returns MIPS StatusReg ($12) IntEnable bit (1)
//
// BOOL IsEnabled();
//
asm BOOL Critical::IsEnabled() { 
	mfc0 v0,$12;
	nop
	lui t0,0
	ori t0,t0,0x0001
	and v0,v0,t0
	jr ra;
	nop;
}

//
// void Enable();
//
asm void Critical::Enable() { 
	mfc0 t0,$12;
	nop;
	ori t0,t0,0x0001;
	mtc0 t0,$12;
	nop;
	jr ra;
	nop;
}
//
// void Disable();
//
asm void Critical::Disable() {
	mfc0 t0,$12;
	nop;
	lui t1, 0xffff;
	ori t1, t1, 0xfffe;
	and $8, $8,$9;
	mtc0 t0,$12;
	nop;
	jr ra;
	nop;
}

#endif  /* #ifdef _WIN32 */

extern "C" 
CRITICAL CriticalEnter() { 
	BOOL fEnabled = Critical::IsEnabled(); 
	Critical::Disable(); 

	return fEnabled;
}

extern "C"		
void CriticalLeave(CRITICAL flags) {
	if (flags)	
		Critical::Enable();
}
