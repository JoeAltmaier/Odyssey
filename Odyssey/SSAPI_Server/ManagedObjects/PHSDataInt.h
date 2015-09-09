//************************************************************************
// FILE:		PHSDataInt.h
//
// PURPOSE:		Defines class PHSData which may be used for 
//				holding/transfering of integer PHS values.
//				This class may subclassed further for specific
//				integer types
//************************************************************************

#ifndef __PHS_DATA_INT_H__
#define	__PHS_DATA_INT_H__


#include "PHSData.h"

#ifdef WIN32
#pragma pack(4)
#endif

class PHSDataInt : public PHSData {

protected:


	int				m_value;
	int				m_criticalThreshold;
	int				m_nonCriticalThreshold;
	int				m_fatalThreshold;


public:


//************************************************************************
// PHSDataInt:
//
// PURPOSE:		Default constructor
//************************************************************************

PHSDataInt( ListenManager	*pListenManager, 
		    DesignatorId	id,
			VALUE_TYPE		valueType,
		    U32				objectTypeClass = SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new PHSDataInt(	GetListenManager(),
																GetDesignatorId(),
																m_valueType ); }


//************************************************************************
// ~PHSDataInt
//
// PURPOSE:		Default destructor
//************************************************************************

virtual ~PHSDataInt();


//************************************************************************
// SetNonCriticalThreshold:
//
// PURPOSE:		Sets the value for non critical threshold. Also marks
//				the flag that this value is present.
//************************************************************************

void SetNonCriticalThreshold( int t );


//************************************************************************
// SetCriticalThreashold:
//
// PURPOSE:		Sets the value for critical threshold. Also marks the flag
//				that this value is present.
//************************************************************************

void SetCriticalThreashold( int t );


//************************************************************************
// SetFatalThreshold:
//
// PURPOSE:		Sets the value for fatal threshold. Also, marks the flag
//				that this value is present.
//************************************************************************

void SetFatalThreshold( int in );


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

void SetValue( int value )			{ m_value		= value; }
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

#endif	//__PHS_DATA_INT_H__