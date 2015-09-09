//************************************************************************
// FILE:		PHSData.cpp
//
// PURPOSE:		Implements an abstract class PHSData which will define
//				the interface for all PHS managed objects in the 
//				system.
//************************************************************************

#include "PHSData.h"
#include "ListenManager.h"
#include "time.h"


//************************************************************************
// PHSData:
//
// PURPOSE:		Default constructor
//************************************************************************

PHSData::PHSData( ListenManager *pListenManager, U32 objectClassType, DesignatorId id, VALUE_TYPE valueType )
		:ManagedObject( pListenManager, objectClassType ){

	time_t			timeTemp;

	m_id			= id;
	m_hasSampleRate	= false;
	m_isSettable	= false;
	m_valueType		= valueType;
	
	time( &timeTemp );
	m_collectionTimestamp = timeTemp;
	m_collectionTimestamp *= 1000;
}


//************************************************************************
// ~PHSData:
//
// PURPOSE:		The destructor
//************************************************************************

PHSData::~PHSData(){
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
PHSData::BuildYourValueSet(){
	
	ManagedObject::BuildYourValueSet();

	if( m_hasSampleRate )
		AddInt64( m_sampleRate, SSAPI_PHS_FID_SAMPLE_RATE );

	AddInt( m_valueType, SSAPI_PHS_FID_VALUE_TYPE );
	AddInt(m_name, SSAPI_PHS_FID_NAME );
	AddInt64( m_collectionTimestamp, SSAPI_PHS_FID_COLLECTION_TIMESTAMP );
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
PHSData::BuildYourselfFromYourValueSet(){

	int temp;
	
	ManagedObject::BuildYourselfFromYourValueSet();

	GetInt( SSAPI_PHS_FID_IS_SETTABLE, &temp );
	m_isSettable = temp ? true : false;

	m_hasSampleRate = GetInt64( SSAPI_PHS_FID_SAMPLE_RATE, &m_sampleRate )? true : false;

	GetInt( SSAPI_PHS_FID_NAME, (int *)&m_name );
	GetInt( SSAPI_PHS_FID_VALUE_TYPE, &temp );
	GetInt64( SSAPI_PHS_FID_COLLECTION_TIMESTAMP, &m_collectionTimestamp );
	m_valueType = (VALUE_TYPE)temp;

	return true;
}


//************************************************************************
// SetSampleRate:
//
// PURPOSE:		Sets value of the sample rate and the flag that the value
//				is present
//************************************************************************

void 
PHSData::SetSampleRate( I64 sampleRate ){

	m_sampleRate	= sampleRate;
	m_hasSampleRate	= true;
}