/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiTarget.h
// 
// Description:
// This file defines the interfaces to HSCSI_Target_Task.
// Each SCSI command that is received by the target driver is executed by a 
// thread of control called a Target Command Task. A Target Command Task is 
// a Nucleus Plus task. A Target Command Task often has to wait for an 
// asynchronous event to occur.  While one task is waiting, another task 
// can be executing another command.  Examples of asynchronous events:
// - Waiting for data to be transferred by the ISP.
// - Waiting for a command to be executed by the ISP.
// - Waiting for a command to be executed by another driver.
//
// For now, this is just a placeholder since HSCSI is initiator only
// 
// Update Log 
//	$Log: /Gemini/Odyssey/Hscsi/HscsiTarget.h $
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#if !defined(HscsiTarget_H)
#define HscsiTarget_H

#include "HscsiData.h"
#include "HscsiEvent.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

STATUS	HSCSI_Target_Create(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_Target_Destroy();

// Methods to be called by HSCSI_Event_Task 
STATUS	HSCSI_Handle_CTIO_Final(HSCSI_EVENT_CONTEXT *p_context);
STATUS	HSCSI_Handle_CTIO_Write(HSCSI_EVENT_CONTEXT *p_context);
STATUS	HSCSI_Handle_I2O_Response(HSCSI_EVENT_CONTEXT *p_context);
STATUS	HSCSI_Handle_Accept_Target_IO(HSCSI_EVENT_CONTEXT *p_context);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* HscsiTarget_H  */
