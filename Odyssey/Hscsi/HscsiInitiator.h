/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiInitiator.h
// 
// Description:
// This file defines the interfaces to HSCSI_Initiator_Task.
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiInitiator.h $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#if !defined(HscsiInitiator_H)
#define HscsiInitiator_H

#include "HscsiData.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

STATUS	HSCSI_Initiator_Create(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_Initiator_Destroy();

// Methods to be called by HSCSI_Event_Task 
STATUS	HSCSI_Handle_Command_Response(HSCSI_EVENT_CONTEXT *p_context);
STATUS	HSCSI_Handle_SCSI_Request(HSCSI_EVENT_CONTEXT *p_context);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif /* HscsiInitiator_H  */
