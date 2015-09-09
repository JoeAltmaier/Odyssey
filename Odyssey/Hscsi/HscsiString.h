/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiString.h
// 
// Description:
// This file defines methods used by the HSCSI driver that are
// normally found in <String.h>.  
// We define our own so that we do not have to depend on a library. 
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiString.h $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#if !defined(HscsiSring_H)
#define HscsiSring_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


void Mem_Copy(void *p_destination, void *p_source, int num_bytes);
void Mem_Set(void *p_destination, int value, int num_bytes);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* HscsiSring_H  */
