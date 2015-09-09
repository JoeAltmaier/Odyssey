//************************************************************************
// FILE:		ProcessRaidUtilitySmartCopy.cpp
//
// PURPOSE:		Implements the object used to represent Raid Smart Copy utility
//************************************************************************

#include "ProcessRaidUtilitySmartCopy.h"

//************************************************************************
// ProcessRaidUtilitySmartCopy:
//
// PURPOSE:		Default constructor
//************************************************************************

ProcessRaidUtilitySmartCopy::ProcessRaidUtilitySmartCopy( ListenManager *pListenManager, CmdSender *pCmdQ, DesignatorId arrayId )
:ProcessRaidUtility( pListenManager, SSAPI_OBJECT_CLASS_TYPE_RAID_SMART_COPY, pCmdQ, arrayId ){

}


//************************************************************************
// ~ProcessRaidUtilitySmartCopy:
//
// PURPOSE:		The destructor
//************************************************************************

ProcessRaidUtilitySmartCopy::~ProcessRaidUtilitySmartCopy(){
}


//************************************************************************
// Start:
//
// PURPOSE:		Attempts to start the utility
//************************************************************************

bool 
ProcessRaidUtilitySmartCopy::Start( ObjectManager *pManager, SsapiResponder *pResponder ){

	return ProcessRaidUtility::Start( pManager, pResponder, RAID_UTIL_MEMBER_HOTCOPY );
}



//************************************************************************
// GetCanAbort:
// GetCanStart:
// GetCanPause:
//************************************************************************

bool 
ProcessRaidUtilitySmartCopy::GetCanAbort(){ return true; }


bool 
ProcessRaidUtilitySmartCopy::GetCanStart(){ return false; }


bool 
ProcessRaidUtilitySmartCopy::GetCanPause(){ return false; }

