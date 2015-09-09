/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdRequest.h
// 
// Description: 
// This file defines the context used to schedule methods that run
// in the thread of the context scheduler.
// All methods that begin with SSD_Request_Context:: run in the thread
// of the context scheduler.
// 
// 
// Update Log 
// 8/27/98 Jim Frandeen: Create file
/*************************************************************************/

#if !defined(SsdRequest_H)
#define SsdRequest_H

#include "Callback.h"
#include "Message.h"

class SSD_Ddm;

class SSD_Request_Context : public Callback_Context
{
private: // member data

	// SSD_Ddm is a friend so it can access its context data
	friend class SSD_Ddm;
	
	// If operation did not succeed, logical byte address of failure.
	I64 m_logical_byte_address;
	
	// Pointer to message that spawned request.   
	Message *m_p_message;
	
	// Number of bytes successfully transferred
	U32 m_transfer_byte_count;
	
	// Pointer to DDM that created this context.
	// We need this in order to Signal the DDM
	// when a request has completed.
	SSD_Ddm *m_p_ddm;

	// Handle of flash file system
	FF_HANDLE 		 m_flash_handle;
	
}; // SSD_Request_Context


#endif //   SsdRequest_H
