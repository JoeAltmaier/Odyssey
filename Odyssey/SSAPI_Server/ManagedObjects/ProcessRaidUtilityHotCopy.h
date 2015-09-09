//************************************************************************
// FILE:		ProcessRaidUtilityHotCopy.h
//
// PURPOSE:		Defines the object used to represent Raid Hot Copy utility
//************************************************************************

#ifndef __PROCESS_RAID_HOT_COPY_H__
#define	__PROCESS_RAID_HOT_COPY_H__

#include "ProcessRaidUtility.h"

class SsapiRepsonder;

#ifdef WIN32
#pragma pack(4)
#endif


class ProcessRaidUtilityHotCopy : public ProcessRaidUtility{


public:


//************************************************************************
// ProcessRaidUtilityHotCopy:
//
// PURPOSE:		Default constructor
//************************************************************************

ProcessRaidUtilityHotCopy( ListenManager *pListenManager, CmdSender *pCmdQ, DesignatorId arrayId );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new ProcessRaidUtilityHotCopy(	GetListenManager(),
																				m_pRaidCommandQ,
																				m_ownerId ); }


//************************************************************************
// ~ProcessRaidUtilityHotCopy:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~ProcessRaidUtilityHotCopy();


//************************************************************************
// GetName:
//************************************************************************

virtual LocalizedString GetName(){ return CTS_SSAPI_RAID_HOT_COPY_NAME; }


//************************************************************************
// Start:
//
// PURPOSE:		Attempts to start the utility
//************************************************************************

virtual bool Start( ObjectManager *pManager, SsapiResponder *pResponder );


//************************************************************************
// GetCanAbort:
// GetCanStart:
// GetCanPause:
//************************************************************************

virtual bool GetCanAbort();
virtual bool GetCanStart();
virtual bool GetCanPause();

};

#endif // __PROCESS_RAID_HOT_COPY_H__