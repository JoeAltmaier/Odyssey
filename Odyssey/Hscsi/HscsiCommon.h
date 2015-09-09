/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiCommon.h
// 
// Description:
// This file contains macros and definitions used in all HSCSI modules.
// This file is included by every HSCSI module.
// 
// Update Log:
//	$Log: /Gemini/Odyssey/Hscsi/HscsiCommon.h $ 
// 
// 1     9/14/99 7:23p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#if !defined(HscsiCommon_H)
#define HscsiCommon_H

// Debugging is turned on
#define HSCSI_DEBUG 

#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_HSCSI
//#include "Serdrv.h"
#include "Odyssey_Trace.h"
#include "HscsiError.h"
#include "HscsiTrace.h"
#include "HscsiEvent.h"
#include "Nucleus.h"
#include "Pci.h"			// need the BYTE_SWAP() macros for everybody
#include "HscsiProto.h"
#include "Odyssey.h"		// Systemwide parameters

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


/*************************************************************************/
// Define the memory spaces
/*************************************************************************/
// #define	KSEG0	0x80000000		// cached kernel memory
#define	KSEG1	0xA0000000		// non-cached kernel memory
		
/*************************************************************************/
//    Read_ISP1040 Macro Definition
/*************************************************************************/
#if defined(HSCSI_DEBUG) && defined(_DEBUG)
U16 Read_ISP1040(PHSCSI_INSTANCE_DATA Id, U16 ISP_register);
#else
#define Read_ISP1040(ISP_register) \
	*((U16*)((UNSIGNED)Id->Regs + ISP_register))
#endif
	
/*************************************************************************/
//    Write_ISP1040 Macro Definition
/*************************************************************************/
#if defined(HSCSI_DEBUG) && defined(_DEBUG)
void Write_ISP1040(PHSCSI_INSTANCE_DATA Id, U16 ISP_register, U16 value);
#else
#define Write_ISP1040(ISP_register, value) \
	*((U16*)((UNSIGNED)Id->Regs + ISP_register)) = value;
#endif

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* HscsiCommon_H  */
