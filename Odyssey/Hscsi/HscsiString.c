/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiString.c
// 
// Description:
// This file implements methods used by the HSCSI driver that are
// normally found in <String.h>.  
// We define our own so that we do not have to depend on a library. 
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiString.c $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#include "HscsiError.h"
#include "HscsiString.h"

/*************************************************************************/
// Mem_Copy 
/*************************************************************************/
void Mem_Copy(void *p_destination, void *p_source, int num_bytes)
{
	int		index;
	
	for (index = 0; index < num_bytes; index++)
		((char*)p_destination)[index] = ((char*)p_source)[index];
		
} //Mem_Copy

/*************************************************************************/
// Mem_Set 
/*************************************************************************/
void Mem_Set(void *p_destination, int value, int num_bytes)
{
	int		index;
	
	for (index = 0; index < num_bytes; index++)
		((char*)p_destination)[index] = value;
		
} //Mem_Set


