//************************************************************************
// FILE:		ProcessRaidUtility:
//
// PURPOSE:		Defines the abstract base class that will be used 
//				to represent RAID utilities running in the O2K 
//				box.
//************************************************************************

#ifndef __RAID_PROCESS_H__
#define	__RAID_PROCESS_H__

#include "CtProcess.h"
#include "RaidDefs.h"
#include "RmstrCmnds.h"
#include "RmstrEvents.h"
#include "RmstrErrors.h"
#include "CmdSender.h"
#include "RAIDUtilTable.h"



#ifdef WIN32
#pragma pack(4)
#endif

class ProcessRaidUtility : public Process {

protected:

	CmdSender			*m_pRaidCommandQ;	// internal


//************************************************************************
// ProcessRaidUtility:
//
// PURPOSE:		Default constructor
//************************************************************************

ProcessRaidUtility( ListenManager	*pListenManager,U32				classType, 
					CmdSender		*pCmdQ,			DesignatorId	arrayId	 )

:Process( pListenManager, classType ){

	m_pRaidCommandQ		= pCmdQ;
	m_ownerId			= arrayId;
}





public:



//************************************************************************
// ~ProcessRaidUtility:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~ProcessRaidUtility(){}


//******************************************************************************
// operator =
//******************************************************************************

virtual const ValueSet& operator=(const ValueSet& obj ){ return *((ValueSet *)this) = obj; }


//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		The method is responsible for adding all data to be 
//				transfered to a client into its value set. All derived 
//				objects must override this method if they have data members
//				they want client proxies to have. 
//
// NOTE:		If a derived object overrides this this method, it MUST call
//				it in its base class BEFORE doing any work!.
//
// RETURN:		true:		success
//************************************************************************

virtual bool BuildYourValueSet();


//************************************************************************
// BuildYourselfFromYourValueSet:
//
// PURPOSE:		Populates data members based on the underlying value set
//
// NOTE:		All subclasses that override this method must call to it
//				somewhere in the overriding method
//
// RETURN:		true:		success
//************************************************************************

virtual bool BuildYourselfFromYourValueSet();


//******************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates data members based on the PTS row
//******************************************************************************

void BuildYourselfFromPtsRow( RAID_ARRAY_UTILITY *pRow );


//************************************************************************
// ModifyObject:
//
// PURPOSE:		Modifes contents of the object
//
// NOTE:		Must be overridden by objects that can be modified
//************************************************************************

virtual bool ModifyObject( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

virtual bool DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder );


//******************************************************************************
// Pause:
//
// PURPOSE:		Attempts to pause the process
//******************************************************************************

virtual bool Pause( SsapiResponder *pResponder );


//******************************************************************************
// Resume:
//
// PURPOSE:		Attempts to resume the process
//******************************************************************************

virtual bool Resume( SsapiResponder *pResponder );


//******************************************************************************
// Abort:
//
// PURPOSE:		Attempts to abort the process
//******************************************************************************

virtual bool Abort( SsapiResponder *pResponder );


//******************************************************************************
// ChangePriority:
//
// PURPOSE:		Attempts to change priority of the process
//******************************************************************************

virtual bool ChangePriority( U32 newPriority, SsapiResponder *pResponder );


virtual DesignatorId GetOwnerId() { return m_ownerId; }

protected:

//******************************************************************************
// Start:
//
// PURPOSE:		Common code for Start() method of all utils. OOP "Template Method" 
//				pattern.
//******************************************************************************

bool Start( ObjectManager *pManager, SsapiResponder *pResponder, RAID_UTIL_NAME utilName );


virtual U32 GetOwnerManegerClassType() { return SSAPI_MANAGER_CLASS_TYPE_STORAGE_MANAGER; }

};

#endif // __RAID_PROCESS_H__