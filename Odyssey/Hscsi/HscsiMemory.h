/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiMemory.h
// 
// Description:
// This is the external interface to system memory allocators used by
// the HSCSI Library. Regular memory or PCI window memory can be allocated.
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiMemory.h $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#if !defined(HscsiMemory_H)
#define HscsiMemory_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


STATUS	HSCSI_Memory_Create(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_Memory_Destroy();

void	* HSCSI_Alloc(U32 type, U32 size);
void	HSCSI_Free(void *p_memory);

void * HSCSI_Get_DMA_Address(void * addr);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif  // HscsiMemory_H

