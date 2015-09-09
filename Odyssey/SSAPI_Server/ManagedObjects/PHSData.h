//************************************************************************
// FILE:		PHSData.h
//
// PURPOSE:		Declares an abstract class PHSData which will define
//				the interface for all PHS managed objects in the 
//				system.
//************************************************************************

#ifndef	__PHS_DATA_MO_H__
#define	__PHS_DATA_MO_H__

#include "ManagedObject.h"
#include "CTEvent.h"
#include "..\SSAPITypes.h"

#ifdef WIN32
#pragma pack(4)
#endif


class PHSData : public ManagedObject{
public:

	enum VALUE_TYPE{	COUNTER	= SSAPI_PHS_DATA_VALUE_TYPE_COUNTER, 
						DELTA	= SSAPI_PHS_DATA_VALUE_TYPE_DELTA, 
						SNAPSHOT= SSAPI_PHS_DATA_VALUE_TYPE_SNAPSHOT, 
						RATIO	= SSAPI_PHS_DATA_VALUE_TYPE_RATIO
					};

protected:

	bool				m_hasNonCriticalThreshold;
	bool				m_hasCriticalThreshold;
	bool				m_hasFatalThreshold;
	bool				m_isSettable;
	bool				m_hasSampleRate;
	LocalizedString		m_name;
	I64					m_sampleRate;		// in microseconds
	VALUE_TYPE			m_valueType;
	LocalizedDateTime	m_collectionTimestamp;

public:

//************************************************************************
// PHSData:
//
// PURPOSE:		Default constructor
//************************************************************************

PHSData( ListenManager *pListenManager, U32 objectClassType, DesignatorId id,
		 VALUE_TYPE    valueType );


//************************************************************************
// ~PHSData:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~PHSData();


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



//************************************************************************
// SetName:
//
// PURPOSE:		Sets name id of the data item
//************************************************************************

void SetName( LocalizedString name )	{ m_name	= name; }


//************************************************************************
// SetSampleRate:
//
// PURPOSE:		Sets value of the sample rate and the flag that the value
//				is present
//************************************************************************

void SetSampleRate( I64 sampleRate );

protected:

//************************************************************************
//************************************************************************


private:
//************************************************************************
//************************************************************************
};

#endif	// __PHS_DATA_MO_H__