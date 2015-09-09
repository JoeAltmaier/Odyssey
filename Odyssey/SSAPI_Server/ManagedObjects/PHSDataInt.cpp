//************************************************************************
// FILE:		PHSDataInt.cpp
//
// PURPOSE:		Implements class PHSData which may be used for 
//				holding/transfering of integer PHS values.
//				This class may subclassed further for specific
//				integer types
//************************************************************************



#include "PHSDataInt.h"
#include "ListenManager.h"
#include "ValueSet.h"





//************************************************************************
// PHSDataInt:
//
// PURPOSE:		Default constructor
//************************************************************************

PHSDataInt::PHSDataInt( ListenManager *pListenManager, DesignatorId id, VALUE_TYPE valueType, U32 objectClassType )
			:PHSData( pListenManager, objectClassType, id, valueType ){

	m_hasCriticalThreshold		= false;
	m_hasNonCriticalThreshold	= false;
	m_hasFatalThreshold			= false;
	m_isSettable				= false;
}


//************************************************************************
// ~PHSDataInt
//
// PURPOSE:		Default destructor
//************************************************************************

PHSDataInt::~PHSDataInt(){
}


//************************************************************************
// SetNonCriticalThreshold:
//
// PURPOSE:		Sets the value for non critical threshold. Also marks
//				the flag that this value is present.
//************************************************************************

void 
PHSDataInt::SetNonCriticalThreshold( int t ){

	m_nonCriticalThreshold	= t;
	m_hasNonCriticalThreshold = true;
}


//************************************************************************
// SetCriticalThreashold:
//
// PURPOSE:		Sets the value for critical threshold. Also marks the flag
//				that this value is present.
//************************************************************************

void
PHSDataInt::SetCriticalThreashold( int t ){

	m_criticalThreshold		= t;
	m_hasCriticalThreshold	= true;
}


//************************************************************************
// SetFatalThreshold:
//
// PURPOSE:		Sets the value for fatal threshold. Also, marks the flag
//				that this value is present.
//************************************************************************

void 
PHSDataInt::SetFatalThreshold( int in ){

	m_fatalThreshold		= in;
	m_hasFatalThreshold		= true;
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
PHSDataInt::BuildYourValueSet(){
	
	
	PHSData::BuildYourValueSet();
	
	AddInt(m_value, SSAPI_PHS_INT_FLOAT_FID_VALUE );

	if( m_hasCriticalThreshold )
		AddInt(m_criticalThreshold, SSAPI_PHS_INT_FLOAT_FID_CRIT_THRESHOLD);

	if( m_hasNonCriticalThreshold )
		AddInt(m_nonCriticalThreshold, SSAPI_PHS_INT_FLOAT_FID_NON_CRIT_THRESHOLD );

	if( m_hasFatalThreshold )
		AddInt(m_fatalThreshold, SSAPI_PHS_INT_FLOAT_FID_FATAL_THRESHOLD );

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
PHSDataInt::BuildYourselfFromYourValueSet(){
	
	PHSData::BuildYourselfFromYourValueSet();

	GetInt( SSAPI_PHS_INT_FLOAT_FID_VALUE, &m_value );

	m_hasCriticalThreshold = GetInt( SSAPI_PHS_INT_FLOAT_FID_CRIT_THRESHOLD, &m_criticalThreshold )? true : false;
	m_hasNonCriticalThreshold = GetInt( SSAPI_PHS_INT_FLOAT_FID_NON_CRIT_THRESHOLD, &m_nonCriticalThreshold )? true : false;
	m_hasFatalThreshold = GetInt( SSAPI_PHS_INT_FLOAT_FID_FATAL_THRESHOLD, &m_fatalThreshold )? true : false;

	return true;
}


