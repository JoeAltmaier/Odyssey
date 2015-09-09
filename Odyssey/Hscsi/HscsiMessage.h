/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiMessage.h
// 
// Description:
// This module defines interfaces to message handling under HSCSI
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiMessage.h $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/
#if !defined(HscsiMessage_H)
#define HscsiMessage_H

#include "HscsiData.h"
#include "HscsiError.h"
#include "Nucleus.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

// Every object has Create, Init, and Destroy methods.
STATUS	HSCSI_Message_Create(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_Message_Destroy();

STATUS	HSCSI_Message_Send_Request(HSCSI_EVENT_CONTEXT *p_context,
			HSCSI_EVENT_ACTION next_action);
STATUS	HSCSI_Message_Send_Response(HSCSI_EVENT_CONTEXT *p_context);
U8		HSCSI_Message_Get_SCSI_Target(void *p_message_frame);

void	*HSCSI_Allocate_Message(U32 type);
void	HSCSI_Free_Message(void *message);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* HscsiMessage_H  */
