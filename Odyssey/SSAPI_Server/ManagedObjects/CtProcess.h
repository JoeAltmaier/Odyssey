//******************************************************************************
// FILE:		CtProcess.h
//
// PURPOSE:		Defines an abstract base class used to represent processes
//				that may run in the O2K.
//******************************************************************************

#ifndef __SSAPI_PROCESS_H__
#define	__SSAPI_PROCESS_H__


#include "ManagedObject.h"
#include "SSAPITypes.h"

class SsapiResponder;

#ifdef WIN32
#pragma pack(4)
#endif


class Process : public ManagedObject {

protected:

	bool				m_canStart;
	bool				m_canPause;
	bool				m_canAbort;
	LocalizedString		m_name;
	U32					m_percentComplete;
	U32					m_priority;
	LocalizedDateTime	m_timeStarted;
	U32					m_state;
	DesignatorId		m_ownerId;
	U32					m_ownerManagerClassType;


//******************************************************************************
// Process:
//
// PURPOSE:		Default constructor
//******************************************************************************

Process( ListenManager *pListenManager, U32 classType );



public:

//******************************************************************************
// ~Process:
//
// PURPOSE:		The destructor
//******************************************************************************

virtual ~Process();


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
// operator =
//******************************************************************************

virtual const ValueSet& operator=(const ValueSet& obj ){ return *((ValueSet *)this) = obj; }


//******************************************************************************
// Virtual accessors
//******************************************************************************

virtual LocalizedString GetName() = 0;
virtual bool GetCanAbort() = 0;
virtual bool GetCanStart() = 0;
virtual bool GetCanPause() = 0;
virtual U32 GetOwnerManegerClassType() = 0;
virtual DesignatorId GetOwnerId() = 0;


//******************************************************************************
// Start:
//
// PURPOSE:		Attempts to start process
//******************************************************************************

virtual bool Start( ObjectManager *pManager, SsapiResponder *pResponder ) = 0;


//******************************************************************************
// Pause:
//
// PURPOSE:		Attempts to pause the process
//******************************************************************************

virtual bool Pause( SsapiResponder *pResponder ) = 0;


//******************************************************************************
// Resume:
//
// PURPOSE:		Attempts to resume the process
//******************************************************************************

virtual bool Resume( SsapiResponder *pResponder ) = 0;


//******************************************************************************
// Abort:
//
// PURPOSE:		Attempts to abort the process
//******************************************************************************

virtual bool Abort( SsapiResponder *pResponder ) = 0;


//******************************************************************************
// ChangePriority:
//
// PURPOSE:		Attempts to change priority of the process
//******************************************************************************

virtual bool ChangePriority( U32 newPriority, SsapiResponder *pResponder ) = 0;


//************************************************************************
// SetPriority:
// SetPercentComplete:
//
// PURPOSE:		Called by the ProcessManager to inform the object it's
//				been modified.
//************************************************************************

void SetPercentComplete( U32 percentComplete );
void SetPriority( U32 priority );


//******************************************************************************
//******************************************************************************

};

#endif // __SSAPI_PROCESS_H__