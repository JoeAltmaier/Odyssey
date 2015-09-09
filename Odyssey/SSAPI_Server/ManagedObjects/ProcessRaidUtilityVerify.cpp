//************************************************************************
// FILE:		ProcessRaidUtilityVerify.cpp
//
// PURPOSE:		Implements the object used to represent Raid Verify utility
//************************************************************************

#include "ProcessRaidUtilityVerify.h"

//************************************************************************
// ProcessRaidUtilityVerify:
//
// PURPOSE:		Default constructor
//************************************************************************

ProcessRaidUtilityVerify::ProcessRaidUtilityVerify( ListenManager *pListenManager, CmdSender *pCmdQ, DesignatorId arrayId )
:ProcessRaidUtility( pListenManager, SSAPI_OBJECT_CLASS_TYPE_RAID_VERIFY, pCmdQ, arrayId ){

}


//************************************************************************
// ~ProcessRaidUtilityVerify:
//
// PURPOSE:		The destructor
//************************************************************************

ProcessRaidUtilityVerify::~ProcessRaidUtilityVerify(){
}


//************************************************************************
// Start:
//
// PURPOSE:		Attempts to start the utility
//************************************************************************

bool 
ProcessRaidUtilityVerify::Start( ObjectManager *pManager, SsapiResponder *pResponder ){

	return ProcessRaidUtility::Start( pManager, pResponder, RAID_UTIL_VERIFY );
}



//************************************************************************
// GetCanAbort:
// GetCanStart:
// GetCanPause:
//************************************************************************

bool 
ProcessRaidUtilityVerify::GetCanAbort(){ return true; }


bool 
ProcessRaidUtilityVerify::GetCanStart(){ return m_state == SSAPI_PROCESS_STATE_RUNNING? false : true; }


bool 
ProcessRaidUtilityVerify::GetCanPause(){ return false; }

