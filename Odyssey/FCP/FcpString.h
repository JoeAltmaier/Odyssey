/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpString.h
// 
// Description:
// This file defines methods used by the FCP driver that are
// normally found in <String.h>.  
// We define our own so that we do not have to depend on a library. 
// 
// Update Log 
// 5/29/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 9/2/98 Michael G. Panas: add C++ stuff
/*************************************************************************/

#if !defined(FcpSring_H)
#define FcpSring_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


void Mem_Copy(void *p_destination, void *p_source, int num_bytes);
void Mem_Copy64(I64 *pDest, I64 *pSrc, int Count);
void Mem_Set(void *p_destination, int value, int num_bytes);
void Mem_Set64(I64 *pDest, I64 value, int Count);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* FcpSring_H  */
