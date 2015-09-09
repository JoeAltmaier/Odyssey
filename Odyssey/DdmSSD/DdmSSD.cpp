/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DdmSSD.cpp
// 
// Description:
// This file implements the Device Dependent Module for the
// Solid State Drive. 
// 
// Update Log 
// 
// 02/25/99 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD1
#include "Trace_Index.h"
#include "Odyssey_Trace.h"
#include "DdmSSD.h"
#include "BuildSys.h"
#include "RequestCodes.h"

CLASSNAME(SSD_Ddm);	// Class Link Name used by Buildsys.cpp

/*************************************************************************/
// SSD_Ddm constructor.
/*************************************************************************/
SSD_Ddm::SSD_Ddm(DID did): Ddm(did)
{
	// Tell the base class Ddm where my config area is.
	SetConfigAddress(&m_config, sizeof(m_config));	
	
} // SSD_Ddm
	
/*************************************************************************/
// SSD_Ddm Ctor -- Create an instance of this DDM and return a pointer to it.
/*************************************************************************/
Ddm * SSD_Ddm::Ctor(DID did)
{
	return new SSD_Ddm(did);	
} // SSD_Ddm
	
/*************************************************************************/
// SSD_Ddm::Initialize -- gets called only once.
/*************************************************************************/
STATUS SSD_Ddm::Initialize(Message *pMsg)	// virtual
{ 
	// Tell Chaos what messages to send us.
	DispatchRequest(BSA_BLOCK_READ, (RequestCallback) &Process_Read);
	
	// Tell Chaos what signals we are using.
	DispatchSignal(0 /* signal number */, (SignalCallback) &Signal_Request_Complete);
	
	// We are finished initializing.  Tell Chaos.
	return Ddm::Initialize(pMsg);
}

/*************************************************************************/
// SSD_Ddm::Signal_Request_Complete 
// Come here when the request has been completed.
/*************************************************************************/
STATUS SSD_Ddm::Signal_Request_Complete(U16 signal_number, void *p_context)
{

	return OK;
}

/*************************************************************************/
// SSD_Ddm::Process_Read 
// Come here when we receive a BSA read message
/*************************************************************************/
STATUS SSD_Ddm::Process_Read(Message *pMsg)
{
	// Create a context
	
	return OK;
}

