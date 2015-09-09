/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpMemory.h
// 
// Description:
// This is the external interface to system memory allocators used by
// the FCP Library. Regular memory or PCI window memory can be allocated.
// 
// Update Log 
// 11/30/98 Michael G. Panas: Create file
/*************************************************************************/

#if !defined(FcpMemory_H)
#define FcpMemory_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


STATUS	FCP_Memory_Create(PINSTANCE_DATA Id);
void	FCP_Memory_Destroy();

void	* FCP_Alloc(U32 type, U32 size);
void	FCP_Free(void *p_memory);

void * FCP_Get_DMA_Address(void * addr);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif  // FcpMemory_H

