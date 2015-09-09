//************************************************************************
// FILE:		PHSDataInt64.cpp
//
// PURPOSE:		Implements class PHSData which may be used for 
//				holding/transfering of integer64 PHS values.
//				This class may subclassed further for specific
//				integer types
//************************************************************************



#include "PHSDataInt64.h"
#include "ListenManager.h"
#include "ValueSet.h"





//************************************************************************
// PHSDataInt64:
//
// PURPOSE:		Default constructor
//************************************************************************

PHSDataInt64::PHSDataInt64( ListenManager *pListenManager, DesignatorId id, VALUE_TYPE valueType, U32 objectClassType )
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

PHSDataInt64::~PHSDataInt64(){
}


//************************************************************************
// SetNonCriticalThreshold:
//
// PURPOSE:		Sets the value for non critical threshold. Also marks
//				the flag that this value is present.
//************************************************************************

void 
PHSDataInt64::SetNonCriticalThreshold( I64 t ){

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
PHSDataInt64::SetCriticalThreashold( I64 t ){

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
PHSDataInt64::SetFatalThreshold( I64 in ){

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
PHSDataInt64::BuildYourValueSet(){
	
	
	PHSData::BuildYourValueSet();
	
	AddInt64(m_value, SSAPI_PHS_INT_FLOAT_FID_VALUE );

	if( m_hasCriticalThreshold )
		AddInt64(m_criticalThreshold, SSAPI_PHS_INT_FLOAT_FID_CRIT_THRESHOLD);

	if( m_hasNonCriticalThreshold )
		AddInt64(m_nonCriticalThreshold, SSAPI_PHS_INT_FLOAT_FID_NON_CRIT_THRESHOLD );

	if( m_hasFatalThreshold )
		AddInt64(m_fatalThreshold, SSAPI_PHS_INT_FLOAT_FID_FATAL_THRESHOLD );

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
PHSDataInt64::BuildYourselfFromYourValueSet(){
	
	PHSData::BuildYourselfFromYourValueSet();

	GetInt64( SSAPI_PHS_INT_FLOAT_FID_VALUE, &m_value );

	m_hasCriticalThreshold = GetInt64( SSAPI_PHS_INT_FLOAT_FID_CRIT_THRESHOLD, &m_criticalThreshold )? true : false;
	m_hasNonCriticalThreshold = GetInt64( SSAPI_PHS_INT_FLOAT_FID_NON_CRIT_THRESHOLD, &m_nonCriticalThreshold )? true : false;
	m_hasFatalThreshold = GetInt64( SSAPI_PHS_INT_FLOAT_FID_FATAL_THRESHOLD, &m_fatalThreshold )? true : false;

	return true;
}


