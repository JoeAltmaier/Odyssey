//************************************************************************
// FILE:		ProcessRaidUtilityHotCopy.cpp
//
// PURPOSE:		Implements the object used to represent Raid Hot Copy utility
//************************************************************************

#include "ProcessRaidUtilityHotCopy.h"

//************************************************************************
// ProcessRaidUtilityHotCopy:
//
// PURPOSE:		Default constructor
//************************************************************************

ProcessRaidUtilityHotCopy::ProcessRaidUtilityHotCopy( ListenManager *pListenManager, CmdSender *pCmdQ, DesignatorId arrayId )
:ProcessRaidUtility( pListenManager, SSAPI_OBJECT_CLASS_TYPE_RAID_HOT_COPY, pCmdQ, arrayId ){

}


//************************************************************************
// ~ProcessRaidUtilityHotCopy:
//
// PURPOSE:		The destructor
//************************************************************************

ProcessRaidUtilityHotCopy::~ProcessRaidUtilityHotCopy(){
}


//************************************************************************
// Start:
//
// PURPOSE:		Attempts to start the utility
//************************************************************************

bool 
ProcessRaidUtilityHotCopy::Start( ObjectManager *pManager, SsapiResponder *pResponder ){

	return ProcessRaidUtility::Start( pManager, pResponder, RAID_UTIL_LUN_HOTCOPY );
}



//************************************************************************
// GetCanAbort:
// GetCanStart:
// GetCanPause:
//************************************************************************

bool 
ProcessRaidUtilityHotCopy::GetCanAbort(){ return true; }


bool 
ProcessRaidUtilityHotCopy::GetCanStart(){ return false; }


bool 
ProcessRaidUtilityHotCopy::GetCanPause(){ return false; }

