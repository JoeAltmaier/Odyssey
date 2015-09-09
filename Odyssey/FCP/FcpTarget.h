/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpTarget.h
// 
// Description:
// This file defines the interfaces to FCP_Target_Task.
// Each SCSI command that is received by the target driver is executed by a 
// thread of control called a Target Command Task. A Target Command Task is 
// a Nucleus Plus task. A Target Command Task often has to wait for an 
// asynchronous event to occur.  While one task is waiting, another task 
// can be executing another command.  Examples of asynchronous events:
// - Waiting for data to be transferred by the ISP.
// - Waiting for a command to be executed by the ISP.
// - Waiting for a command to be executed by another driver.
// 
// Update Log 
// 5/5/98 Jim Frandeen: Use C++ comment style
// 4/14/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 9/2/98 Michael G. Panas: add C++ stuff
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/

#if !defined(FcpTarget_H)
#define FcpTarget_H

#include "FcpData.h"
#include "FcpEvent.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

STATUS	FCP_Target_Create(PINSTANCE_DATA Id);
void	FCP_Target_Destroy();

// Methods to be called by FCP_Event_Task 
STATUS	FCP_Handle_CTIO_Final(FCP_EVENT_CONTEXT *p_context);
STATUS	FCP_Handle_CTIO_Write(FCP_EVENT_CONTEXT *p_context);
STATUS	FCP_Handle_I2O_Response(FCP_EVENT_CONTEXT *p_context);
STATUS	FCP_Handle_Accept_Target_IO(FCP_EVENT_CONTEXT *p_context);

// external entry methods
STATUS FCP_Send_CTIO(FCP_EVENT_CONTEXT *p_context, 
	FCP_EVENT_ACTION next_action, 
	U8 SCSI_status, UNSIGNED flags);

STATUS FCP_Send_CTIO_Check(FCP_EVENT_CONTEXT *p_context, 
	FCP_EVENT_ACTION next_action, U8 *SCSI_sense,
	UNSIGNED sense_length, UNSIGNED flags);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* FcpTarget_H  */
