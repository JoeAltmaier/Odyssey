/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpInitiator.h
// 
// Description:
// This file defines the interfaces to FCP_Initiator_Task.
// 
// Update Log 
// 5/5/98 Jim Frandeen: Use C++ comment style
// 4/14/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 9/2/98 Michael G. Panas: add C++ stuff
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/

#if !defined(FcpInitiator_H)
#define FcpInitiator_H

#include "FcpData.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

STATUS	FCP_Initiator_Create(PINSTANCE_DATA Id);
void	FCP_Initiator_Destroy();

// Methods to be called by FCP_Event_Task 
STATUS	FCP_Handle_Command_Response(FCP_EVENT_CONTEXT *p_context);
STATUS	FCP_Handle_SCSI_Request(FCP_EVENT_CONTEXT *p_context);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif /* FcpInitiator_H  */
