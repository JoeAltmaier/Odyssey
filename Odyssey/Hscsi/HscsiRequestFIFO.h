/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiRequestFIFO.h
// 
// Description:
// This file describes interfaces to the ISP request FIFO. 
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiRequestFIFO.h $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#if !defined(HscsiRequestFIFO_H)
#define HscsiRequestFIFO_H

#include "Nucleus.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


STATUS	HSCSI_Request_FIFO_Create(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_Request_FIFO_Destroy();

void*	HSCSI_Request_FIFO_Get_Pointer(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_Request_FIFO_Update_Index(PHSCSI_INSTANCE_DATA Id);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif /* HscsiRequestFIFO_H  */
