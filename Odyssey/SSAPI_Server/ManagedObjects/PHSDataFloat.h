//************************************************************************
// FILE:		PHSDataFloat.h
//
// PURPOSE:		Defines class PHSData which may be used for 
//				holding/transfering of float PHS values.
//				This class may subclassed further for specific
//				float types
//************************************************************************

#ifndef __PHS_DATA_FLOAT_H__
#define	__PHS_DATA_FLOAT_H__


#include "PHSData.h"

#ifdef WIN32
#pragma pack(4)
#endif

class PHSDataFloat : public PHSData {

protected:


	float			m_value;
	float			m_criticalThreshold;
	float			m_nonCriticalThreshold;
	float			m_fatalThreshold;


public:


//************************************************************************
// PHSDataFloat:
//
// PURPOSE:		Default constructor
//************************************************************************

PHSDataFloat(	ListenManager	*pListenManager, 
				DesignatorId	id,
				VALUE_TYPE		valueType,
				U32				objectTypeClass = SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_FLOAT );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new PHSDataFloat(	GetListenManager(), 
																	GetDesignatorId(),
																	m_valueType); }


//************************************************************************
// ~PHSDataFloat
//
// PURPOSE:		Default destructor
//************************************************************************

virtual ~PHSDataFloat();


//************************************************************************
// SetNonCriticalThreshold:
//
// PURPOSE:		Sets the value for non critical threshold. Also marks
//				the flag that this value is present.
//************************************************************************

void SetNonCriticalThreshold( float t );


//************************************************************************
// SetCriticalThreashold:
//
// PURPOSE:		Sets the value for critical threshold. Also marks the flag
//				that this value is present.
//************************************************************************

void SetCriticalThreashold( float t );


//************************************************************************
// SetFatalThreshold:
//
// PURPOSE:		Sets the value for fatal threshold. Also, marks the flag
//				that this value is present.
//************************************************************************

void SetFatalThreshold( float in );


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
// Assignment operator overloaded
//************************************************************************

const ValueSet& operator=(const ValueSet& obj ){ *(ValueSet *)this = obj; return obj; }


//************************************************************************
// Accessors.
//
// PURPOSE:		Set corresponding values in an instance of the class
//************************************************************************

void SetValue( float value )		{ m_value		= value; }
void SetIsSettable( bool flag )		{ m_isSettable	= flag; }


//************************************************************************
//************************************************************************

protected:

//************************************************************************
//************************************************************************



private:
//************************************************************************
//************************************************************************

};

#endif	//__PHS_DATA_FLOAT_H__