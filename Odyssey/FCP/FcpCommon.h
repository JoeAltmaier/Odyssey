/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpCommon.h
// 
// Description:
// This file contains macros and definitions used in all FCP modules.
// This file is included by every FCP module.
// 
// Update Log: 
// 5/19/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 9/2/98 Michael G. Panas: Reorder file, add C++ stuff
// 9/30/98 Michael G. Panas: Support for _DEBUG, add Odyssey.h
// 10/1/98 Michael G. Panas: remove flag used in test only
// 01/24/99 Michael G. Panas: Use new TraceLevel[] array, normalize all trace
/*************************************************************************/

#if !defined(FcpCommon_H)
#define FcpCommon_H


// Debugging is turned on
#ifdef _DEBUG
#define FCP_DEBUG
#endif 

#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_FCP
//#include "Serdrv.h"
#include "Odyssey_Trace.h"
#include "FcpError.h"
#include "FcpTrace.h"
#include "FcpEvent.h"
#include "Nucleus.h"
#include "Pci.h"			// need the BYTE_SWAP() macros for everybody
#include "FcpProto.h"
#include "Odyssey.h"		// Systemwide parameters

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

extern	void	bcopy64(char *, char *, int);
extern	void	bcopy(char *, char *, int);
extern	void	bzero64(char *, int);
extern	void	bzero(char *, int);



/*************************************************************************/
// Define the memory spaces
/*************************************************************************/
#define	KSEG0	0x80000000		// cached kernel memory
#define	KSEG1	0xA0000000		// non-cached kernel memory
		
/*************************************************************************/
//    Read_ISP Macro Definition
/*************************************************************************/
#if defined(FCP_DEBUG) && defined(_DEBUG)
U16 Read_ISP(PINSTANCE_DATA Id, U16 ISP_register);
#else
//#define Read_ISP(Id, ISP_register) \
//	*((volatile U16*)((UNSIGNED)Id->Regs + ISP_register))
inline U16 Read_ISP(PINSTANCE_DATA Id, U16 ISP_register) {U16 tt = *((volatile U16*)((UNSIGNED)Id->Regs + ISP_register)); return(BYTE_SWAP16(tt));}
#endif
	
/*************************************************************************/
//    Write_ISP Macro Definition
/*************************************************************************/
#if defined(FCP_DEBUG) && defined(_DEBUG)
void Write_ISP(PINSTANCE_DATA Id, U16 ISP_register, U16 value);
#else
//#define Write_ISP(Id, ISP_register, value) \
//	*((volatile U16*)((UNSIGNED)Id->Regs + ISP_register)) = value;
inline void Write_ISP(PINSTANCE_DATA Id, U16 ISP_register, U16 value) {*((volatile U16*)((UNSIGNED)Id->Regs + ISP_register)) = BYTE_SWAP16(value);}
#endif

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* FcpCommon_H  */
