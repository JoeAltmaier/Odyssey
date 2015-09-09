/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiBuffer.h
// 
// Description:
// This file defines interfaces for the HSCSI buffer allocation 
// and deallocation. 
// 
// Update Log:
//	$Log: /Gemini/Odyssey/Hscsi/HscsiBuffer.h $ 
// 
// 1     9/14/99 7:23p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#if !defined(HscsiBuffer_H)
#define HscsiBuffer_H

#include "HscsiIOCB.h"
#include "Nucleus.h"
#include "HscsiConfig.h"
#include "HscsiISP.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


#if 1
// For this implementation, our buffer size is 64K bytes,
// and each buffer begins on an 64K boundary
#define HSCSI_BUFFER_SIZE (8192*8)
#define HSCSI_BUFFER_MASK		0xFFFF0000
#define HSCSI_BUFFER_MASK_LOW	0x0000FFFF

#define HSCSI_BUFFER_SHIFT	16	// number of zeros in buffer address
#else
// For this implementation, our buffer size is 8K bytes,
// and each buffer begins on an 8K boundary
#define HSCSI_BUFFER_SIZE 8192
#define HSCSI_BUFFER_MASK		0xFFFFE000
#define HSCSI_BUFFER_MASK_LOW	0x00001FFF

#define HSCSI_BUFFER_SHIFT	13	// number of zeros in buffer address
#endif

STATUS	HSCSI_Buffer_Allocate(PHSCSI_INSTANCE_DATA Id, UNSIGNED buffer_size, void **pp_buffer);
STATUS  HSCSI_Buffer_Deallocate(PHSCSI_INSTANCE_DATA Id, void* p_buffer);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif /* HscsiBuffer_H  */
