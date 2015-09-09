/* OsTypes.h -- Simple types used by OS
 *
 * Copyright (C) ConvergeNet Technologies, 1998 
 * Copyright (C) Dell Computer, 2000
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

// Revision History:
// 10-05-98  Tom Nelson: Created
// ** Log at end-of-file **

#ifndef OsTypes_H
#define OsTypes_H

#ifndef _WIN32
	#include "Nucleus.h"			//***NO LONGER NEEDED BY OOS except in Kernel.h ***
	#include "simple.h"
#else
	#include "simple.h"

	typedef int STATUS;
	typedef short SHORT;
	typedef long LONG;
	typedef void* HANDLE;
	typedef char CHAR;

	//Tasks
	typedef unsigned long CT_Task;
	typedef VOID (*CT_Task_Entry)(VOID *);

	// Semaphones
	typedef HANDLE CT_Semaphore;
	#define SEMAPHORE_MAX 100
	#define CT_SUSPEND		1
	#define CT_NO_SUSPEND	0

	// Timers
	typedef int CT_Timer;
	typedef VOID (*CT_Timer_Callback)(UNSIGNED);
	typedef unsigned char OPTION;
	#define CT_ENABLE_TIMER		TRUE
	#define CT_DISABLE_TIMER	FALSE
	#define CT_USEC_MULT		10000		// Tick multiplier for usecs	

#endif

/* OS specific types */

typedef S32 VDN;			// Virtual Device Number (Same as VirtualDevice)
typedef U32 DID;			// Device ID - Must be different from VirtualDevice

#define DIDNULL ((DID) (-1))
#define SLOTNULL ((TySlot) (-1))
#define VDNNULL	0

typedef U32 REQUESTCODE;
typedef U16 SIGNALCODE;
typedef I64 REFNUM;			// Reference Number
typedef I64 Timestamp;

#define sCLASSNAME		32	// Maximum Class Name Length

typedef enum {
	UNINITIALIZED, INITIALIZING, QUIESCENT, QUIESCING, ENABLED, ENABLING, 
	HALTED, HALTING, MODECHANGE, MODECHANGING, TERMINATE, TERMINATING
} DDMSTATE;

typedef enum {
	SYSTEM, PRIMARY, ALTERNATE
} DDMMODE;

/* Oos simple prototypes */

// Specify allocation type to New (useful in any environment)

#define tBESTFIT	0x00	// NoFrag or Block depending on size of allocation
#define tSMALL		0x00	// NoFrag Heap
#define tBIG		0x01	// Block Heap
#define tRAMMASK	0x01

// Allocation modifiers

#define tPCIBIT		0x10
#define tPCI		(tPCIBIT | tUNCACHED)	// PCI Mapped (May not be Cached)
#define tZERO		0x20	// Clear memory
#define tCACHED		0x00	// Default for tSMALL
#define tUNCACHED	0x80	// Default for tBIG
#define tCACHEMASK	0x80

#ifdef __cplusplus

class Ddm;

typedef Ddm * (*CtorFunc)(DID did);
typedef void  (*InitFunc)(void);

typedef U32   ClassFlags;
#define SINGLE			0x0000
#define MULTIPLE		0x0001
#define DDMDEBUG		0x0002
#define ACTIVEACTIVE	0x0004

// Overloaded New/Delete Operators

extern void *operator new(unsigned int);
extern void  operator delete(void *);

extern void *operator new[](unsigned int);
extern void *operator new[](unsigned int,int);
inline void *operator new(unsigned int sRam,int tRam) { return new (tRam) char[sRam]; }
extern void *operator new[](unsigned int, DID, int *);


extern "C" {
#endif

#ifndef _WIN32

void *malloc(unsigned int nBytes);
void free(void *pMem);

#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

void *AllocBlk(U32 nBytes);
void *FreeBlk(void *pMem);

#ifdef __cplusplus
}
/* Obsolete, do not use */
/* Joe's Debug */
extern void Debug_(char*);
extern void Debug_(unsigned long);
extern void Debug_(long);
extern void Debug_(unsigned short);
extern void Debug_(unsigned int);
extern void Debug_(int);
extern void Debug_(char);

#ifdef DEBUG
#define Debug(x) Debug_(x)
#else
#define Debug(x)
#endif

#endif


#endif /* OsTypes_H */

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Include/Oos/OsTypes.h $
// 
// 31    2/08/00 8:48p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 32    2/08/00 6:12p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
//
// 08-08-99  Bob Butler: Timestamp typedef.
// 05-14-99  Jim Frandeen: Make OPTION unsigned char to be same as Nucleus.
// 05-07-99  Eric Wedel: Cast DIDNULL into DID type (Green Hills).
// 11-11-98	 Tom Nelson: Added new with allocation type argument
// 10-05-98  Tom Nelson: Created

