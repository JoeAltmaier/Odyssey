/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: FlashMonitorIsm.cpp
// 
// Description:
// This file is the Flash Monitor ISM definition.  This class uses to
// discover any changes in status of the Flash memory.
// 
// $Log: /Gemini/Odyssey/IsmFlashMonitor/FlashMonitorIsm.cpp $
// 
// 2     10/22/99 11:31a Hdo
// Re-write of the table methods
// 
// 1     10/11/99 4:56p Hdo
// Initial check in
// 
// 07/13/99 Huy Do: Create file
/*************************************************************************/

#define _DEBUG

#include "Nucleus.h"
#include "OsTypes.h"
#include "Message.h"
#include "Odyssey.h"

#include "Trace_Index.h"
#define TRACE_INDEX TRACE_FLASH_MONITOR
#include "Odyssey_Trace.h"

#include "FlashMonitorIsm.h"

#include "BuildSys.h"
//#include "CtEvent.h"
//#include "RequestCodes.h"

CLASSNAME(FlashMonitorIsm, SINGLE);

/*************************************************************************/
// Constructor
// Constructor method for the class FlashMonitorIsm
/*************************************************************************/
inline FlashMonitorIsm::FlashMonitorIsm(DID did) : Ddm( did )
{
	TRACE_ENTRY(FlashMonitorIsm::FlashMonitorIsm);
}

/*************************************************************************/
// Ctor
// Create a new instance of the Drive Monitor
/*************************************************************************/
inline Ddm *FlashMonitorIsm::Ctor(DID did)
{
	TRACE_ENTRY(FlashMonitorIsm::Ctor);

	return new FlashMonitorIsm(did);
}

/*************************************************************************/
// Initialize
// Start up the hardware belonging to this derived class
/*************************************************************************/
STATUS FlashMonitorIsm::Initialize(Message *pMsg)
{
	TRACE_ENTRY(FlashMonitorIsm::Initialize);

	SetConfigAddress(&m_config, sizeof(m_config));

	// Start the table operations for the SSD_Descriptor and SRC tables
	Desc_Table_Initialize(pMsg);

	return OK;
}

/*************************************************************************/
// Enable
// Update the fUsed field in the SRC table to used.
// Update the IopState in the SSD_Descriptor table to ???
/************************************************************************/
STATUS FlashMonitorIsm::Enable(Message *pMsg)
{
	TRACE_ENTRY(FlashMonitorIsm::Enable);

	// TODO
	Reply(pMsg);

	return OK;
}

/*************************************************************************/
// Quiesce
// Update the fUsed field in the SRC table to unused.
// Update the IopState in the SSD_Descriptor table to IOPS_QUIESCENT
/*************************************************************************/
inline STATUS FlashMonitorIsm::Quiesce(Message *pMsg)
{
	TRACE_ENTRY(FlashMonitorIsm::Quiesce);

	// TODO
	Reply(pMsg);

	return OK;
}

/*************************************************************************/
// DoWork
// This derived classes method to receive messages and replies
/*************************************************************************/
//STATUS FlashMonitorIsm::DoWork(Message *pMsg)
//{
//	TRACE_ENTRY(FlashMonitorIsm::DoWork);

//	return OK;
//}

