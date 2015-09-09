//******************************************************************************
// FILE:		Process.cpp
//
// PURPOSE:		Implements an abstract base class used to represent processes
//				that may run in the O2K.
//******************************************************************************


#include "CtProcess.h"


//******************************************************************************
// Process:
//
// PURPOSE:		Default constructor
//******************************************************************************

Process::Process( ListenManager *pListenManager, U32 classType )
:ManagedObject( pListenManager, classType ){


}


//******************************************************************************
// ~Process:
//
// PURPOSE:		The destructor
//******************************************************************************

Process::~Process(){
}


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

bool 
Process::BuildYourValueSet(){

	int		temp;
	

	m_name		= GetName();
	m_canAbort	= GetCanAbort();
	m_canStart	= GetCanStart();
	m_canPause	= GetCanPause();
	m_ownerId	= GetOwnerId();
	m_ownerManagerClassType = GetOwnerManegerClassType();

	ManagedObject::BuildYourValueSet();

	temp = m_canStart? 1 : 0;
	AddInt( temp, SSAPI_PROCESS_FID_CAN_START );

	temp = m_canAbort? 1 : 0;
	AddInt( temp, SSAPI_PROCESS_FID_CAN_ABORT );

	temp = m_canPause? 1 : 0;
	AddInt( temp, SSAPI_PROCESS_FID_CAN_PAUSE );
	AddU32( m_percentComplete, SSAPI_PROCESS_FID_PERCENT_COMPLETE );
	AddU32( m_priority, SSAPI_PROCESS_FID_PRIORITY );
	AddInt64( m_timeStarted, SSAPI_PROCESS_FID_TIME_STARTED );
	AddU32( m_state, SSAPI_PROCESS_FID_STATE );
	AddU32( m_name, SSAPI_PROCESS_FID_NAME );
	AddU32( m_ownerManagerClassType, SSAPI_PROCESS_FID_OWNER_MANAGER_CLASS_TYPE);
	AddGenericValue( (char *)&m_ownerId, sizeof(m_ownerId), SSAPI_PROCESS_FID_OWNER_ID );

	return true;
}


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

bool 
Process::BuildYourselfFromYourValueSet(){

	int					temp;
	U32					ls;
	
	ManagedObject::BuildYourselfFromYourValueSet();

	GetInt( SSAPI_PROCESS_FID_CAN_START, &temp );
	m_canStart = temp? true : false;

	GetInt( SSAPI_PROCESS_FID_CAN_ABORT, &temp );
	m_canAbort = temp? true : false;

	GetInt( SSAPI_PROCESS_FID_CAN_PAUSE, &temp );
	m_canPause = temp? true : false;

	GetU32( SSAPI_PROCESS_FID_PERCENT_COMPLETE, &m_percentComplete );
	GetU32( SSAPI_PROCESS_FID_PRIORITY, &m_priority );
	GetInt64( SSAPI_PROCESS_FID_TIME_STARTED, &m_timeStarted );
	GetU32( SSAPI_PROCESS_FID_STATE, &m_state );

	if( GetU32( SSAPI_PROCESS_FID_NAME, &ls ) )
		m_name = ls;

	GetU32( SSAPI_PROCESS_FID_OWNER_MANAGER_CLASS_TYPE, &m_ownerManagerClassType );
	GetGenericValue( (char *)&m_ownerId, sizeof(m_ownerId), SSAPI_PROCESS_FID_OWNER_ID );

	return true;

}

//************************************************************************
// SetPriority:
// SetPercentComplete:
//
// PURPOSE:		Called by the ProcessManager to inform the object it's
//				been modified.
//************************************************************************

void 
Process::SetPercentComplete( U32 percentComplete ){

	m_percentComplete = percentComplete;
	FireEventObjectModifed();
}


void 
Process::SetPriority( U32 priority ){

	m_priority = priority;
	FireEventObjectModifed();
}


