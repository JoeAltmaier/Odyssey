//************************************************************************
// FILE:		ProcessRaidUtilityRegenerate.cpp
//
// PURPOSE:		Implements the object used to represent Raid Regenerate utility
//************************************************************************

#include "ProcessRaidUtilityRegenerate.h"

//************************************************************************
// ProcessRaidUtilityRegenerate:
//
// PURPOSE:		Default constructor
//************************************************************************

ProcessRaidUtilityRegenerate::ProcessRaidUtilityRegenerate( ListenManager *pListenManager, CmdSender *pCmdQ, DesignatorId arrayId )
:ProcessRaidUtility( pListenManager, SSAPI_OBJECT_CLASS_TYPE_RAID_REGENERATE, pCmdQ, arrayId ){

}


//************************************************************************
// ~ProcessRaidUtilityRegenerate:
//
// PURPOSE:		The destructor
//************************************************************************

ProcessRaidUtilityRegenerate::~ProcessRaidUtilityRegenerate(){
}


//************************************************************************
// Start:
//
// PURPOSE:		Attempts to start the utility
//************************************************************************

bool 
ProcessRaidUtilityRegenerate::Start( ObjectManager *pManager, SsapiResponder *pResponder ){

	return ProcessRaidUtility::Start( pManager, pResponder, RAID_UTIL_REGENERATE );
}



//************************************************************************
// GetCanAbort:
// GetCanStart:
// GetCanPause:
//************************************************************************

bool 
ProcessRaidUtilityRegenerate::GetCanAbort(){ return true; }


bool 
ProcessRaidUtilityRegenerate::GetCanStart(){ return m_state == SSAPI_PROCESS_STATE_RUNNING? false : true; }


bool 
ProcessRaidUtilityRegenerate::GetCanPause(){ return false; }

