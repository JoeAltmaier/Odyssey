//************************************************************************
// FILE:		ProcessRaidUtilityInitialize.cpp
//
// PURPOSE:		Implements the object used to represent Raid Verify utility
//************************************************************************

#include "ProcessRaidUtilityInitialize.h"


//************************************************************************
// ProcessRaidUtilityInitialize:
//
// PURPOSE:		Default constructor
//************************************************************************

ProcessRaidUtilityInitialize::ProcessRaidUtilityInitialize( ListenManager *pListenManager, CmdSender *pCmdQ, DesignatorId arrayId )
:ProcessRaidUtility( pListenManager, SSAPI_OBJECT_CLASS_TYPE_RAID_INITIALIZE, pCmdQ, arrayId ){

}


//************************************************************************
// ~ProcessRaidUtilityInitialize:
//
// PURPOSE:		The destructor
//************************************************************************

ProcessRaidUtilityInitialize::~ProcessRaidUtilityInitialize(){
}


//************************************************************************
// Start:
//
// PURPOSE:		Attempts to start the utility
//************************************************************************

bool 
ProcessRaidUtilityInitialize::Start( ObjectManager *pManager, SsapiResponder *pResponder ){

	return ProcessRaidUtility::Start( pManager, pResponder, RAID_UTIL_BKGD_INIT );
}



//************************************************************************
// GetCanAbort:
// GetCanStart:
// GetCanPause:
//************************************************************************

bool 
ProcessRaidUtilityInitialize::GetCanAbort(){ return false; }


bool 
ProcessRaidUtilityInitialize::GetCanStart(){ return false; }


bool 
ProcessRaidUtilityInitialize::GetCanPause(){ return false; }

