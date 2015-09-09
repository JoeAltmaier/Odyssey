//************************************************************************
// FILE:		PHSDataFloat.cpp
//
// PURPOSE:		Implements class PHSData which may be used for 
//				holding/transfering of float PHS values.
//				This class may subclassed further for specific
//				float types
//************************************************************************



#include "PHSDataFloat.h"
#include "ListenManager.h"
#include "ValueSet.h"





//************************************************************************
// PHSDataFloat:
//
// PURPOSE:		Default constructor
//************************************************************************

PHSDataFloat::PHSDataFloat(	ListenManager	*pListenManager, DesignatorId id, VALUE_TYPE valueType, U32 objectClassType )
			:PHSData( pListenManager, objectClassType, id, valueType ){

	m_hasCriticalThreshold		= false;
	m_hasNonCriticalThreshold	= false;
	m_hasFatalThreshold			= false;
	m_isSettable				= false;
}


//************************************************************************
// ~PHSDataFloat
//
// PURPOSE:		Default destructor
//************************************************************************

PHSDataFloat::~PHSDataFloat(){
}


//************************************************************************
// SetNonCriticalThreshold:
//
// PURPOSE:		Sets the value for non critical threshold. Also marks
//				the flag that this value is present.
//************************************************************************

void 
PHSDataFloat::SetNonCriticalThreshold( float t ){

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
PHSDataFloat::SetCriticalThreashold( float t ){

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
PHSDataFloat::SetFatalThreshold( float in ){

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
PHSDataFloat::BuildYourValueSet(){
	
	
	PHSData::BuildYourValueSet();
	
	AddFloat(m_value, SSAPI_PHS_INT_FLOAT_FID_VALUE );

	if( m_hasCriticalThreshold )
		AddFloat(m_criticalThreshold, SSAPI_PHS_INT_FLOAT_FID_CRIT_THRESHOLD);
	
	if( m_hasNonCriticalThreshold )
		AddFloat(m_nonCriticalThreshold, SSAPI_PHS_INT_FLOAT_FID_NON_CRIT_THRESHOLD );

	if( m_hasFatalThreshold )
		AddFloat(m_fatalThreshold, SSAPI_PHS_INT_FLOAT_FID_FATAL_THRESHOLD );

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
PHSDataFloat::BuildYourselfFromYourValueSet(){
	
	PHSData::BuildYourselfFromYourValueSet();

	GetFloat( SSAPI_PHS_INT_FLOAT_FID_VALUE, &m_value );

	m_hasCriticalThreshold = GetFloat( SSAPI_PHS_INT_FLOAT_FID_CRIT_THRESHOLD, &m_criticalThreshold )? true : false;
	m_hasNonCriticalThreshold = GetFloat( SSAPI_PHS_INT_FLOAT_FID_NON_CRIT_THRESHOLD, &m_nonCriticalThreshold )? true : false;
	m_hasFatalThreshold = GetFloat( SSAPI_PHS_INT_FLOAT_FID_FATAL_THRESHOLD, &m_fatalThreshold )? true : false;

	return true;
}


