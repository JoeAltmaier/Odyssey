/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiResponseFIFO.h
// 
// Description:
// This file describes interfaces to the ISP response FIFO. 
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiResponseFIFO.h $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#if !defined(HscsiResponseFIFO_H)
#define HscsiResponseFIFO_H

#include "Nucleus.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

STATUS	HSCSI_Response_FIFO_Create(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_Response_FIFO_Destroy();

void	HSCSI_Response_FIFO_Increment_Index();

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* HscsiResponseFIFO_H  */
