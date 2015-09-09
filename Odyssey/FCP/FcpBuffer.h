/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpBuffer.h
// 
// Description:
// This file defines interfaces for the FCP buffer allocation 
// and deallocation. 
// 
// Update Log: 
// 5/15/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 9/2/98 Michael G. Panas: add C++ stuff

/*************************************************************************/

#if !defined(FcpBuffer_H)
#define FcpBuffer_H

#include "FcpIOCB.h"
#include "Nucleus.h"
#include "FcpConfig.h"
#include "FcpISP.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


#if 1
// For this implementation, our buffer size is 64K bytes,
// and each buffer begins on an 64K boundary
#define FCP_BUFFER_SIZE (8192*8)
#define FCP_BUFFER_MASK		0xFFFF0000
#define FCP_BUFFER_MASK_LOW	0x0000FFFF

#define FCP_BUFFER_SHIFT	16	// number of zeros in buffer address
#else
// For this implementation, our buffer size is 8K bytes,
// and each buffer begins on an 8K boundary
#define FCP_BUFFER_SIZE 8192
#define FCP_BUFFER_MASK		0xFFFFE000
#define FCP_BUFFER_MASK_LOW	0x00001FFF

#define FCP_BUFFER_SHIFT	13	// number of zeros in buffer address
#endif

STATUS	FCP_Buffer_Allocate(PINSTANCE_DATA Id, UNSIGNED buffer_size, void **pp_buffer);
STATUS  FCP_Buffer_Deallocate(PINSTANCE_DATA Id, void* p_buffer);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif /* FcpBuffer_H  */
