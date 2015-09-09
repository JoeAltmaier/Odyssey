/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpMessage.h
// 
// Description:
// This module defines interfaces to message handling used in the FCP Library
// 
// Update Log 
// 5/5/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 9/2/98 Michael G. Panas: add C++ stuff
// 9/3/98 Michael G. Panas: add message allocator
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/
#if !defined(FcpMessage_H)
#define FcpMessage_H

#include "FcpData.h"
#include "FcpError.h"
#include "Nucleus.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

// Every object has Create, Init, and Destroy methods.
STATUS	FCP_Message_Create(PINSTANCE_DATA Id);
void	FCP_Message_Destroy();

STATUS	FCP_Message_Send_Request(FCP_EVENT_CONTEXT *p_context,
			FCP_EVENT_ACTION next_action);
STATUS	FCP_Message_Send_Response(FCP_EVENT_CONTEXT *p_context);
U8		FCP_Message_Get_SCSI_Target(void *p_message_frame);
void 	FCP_Send_Enable_Reply(PINSTANCE_DATA Id);

void	*FCP_Allocate_Message(U32 type);
I64 	FCP_Message_Latency(void *p_msg);
void	FCP_Free_Message(void *message);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* FcpMessage_H  */
