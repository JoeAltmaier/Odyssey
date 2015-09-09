/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiMessage.c
// 
// Description:
// This module contain message allocation methods
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiMessage.cpp $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/
#include "HscsiCommon.h"
#include "HscsiI2O.h"
#include "HscsiMessage.h"
#include "HscsiMessageFormats.h"
#include "HscsiString.h"
#include "Message.h"
#include "OsTypes.h"

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// MSG globals
/*************************************************************************/


/*************************************************************************/
// HSCSI_Message_Create
// Create HSCSI_Message object
// On return, this pointer is updated.
/*************************************************************************/
STATUS HSCSI_Message_Create(PHSCSI_INSTANCE_DATA Id)
{
 	HSCSI_TRACE_ENTRY(HSCSI_Message_Create);
	
	return NU_SUCCESS;
	
} // HSCSI_Message_Create

/*************************************************************************/
// HSCSI_Message_Destroy
// Destroy MSG object
/*************************************************************************/
void HSCSI_Message_Destroy()
{
	// TODO
	
} // HSCSI_Message_Destroy

/*************************************************************************/
// HSCSI_Allocate_Message
// Allocate a message of MESSAGESIZE bytes using new
/*************************************************************************/
void *HSCSI_Allocate_Message(U32 type) {

	TRACE_ENTRY(HSCSI_Allocate_Message);

	return (void *) new Message(type, sizeof(HSCSI_MSG_SIZE));
	
}	// HSCSI_Allocate_Message

/*************************************************************************/
// HSCSI_Free_Message
// De-Allocate a message allocated by new
/*************************************************************************/
void HSCSI_Free_Message(void *message) {

	TRACE_ENTRY(HSCSI_Free_Message);

	delete (Message *)message;
	
}	// HSCSI_Free_Message

