//************************************************************************
// FILE:		LogMessage.h
//
// PURPOSE:		Defines the class used for wrapping up event log messages
//				before they are passed to the CS SSAPI
//************************************************************************

#ifndef __LOG_MESSAGE_H__
#define __LOG_MESSAGE_H__

#include "ManagedObject.h"
#include "Event.h"

#ifdef WIN32
#pragma pack(4)
#endif





class LogMessage : public ManagedObject {

	U32				m_sequenceNumber;
	I64				m_timeStamp;
	STATUS			m_ec;
	U16				m_slot;
	U16				m_facility;
	U16				m_severity;
	DID				m_did;
	VDN				m_vdn;
	ValueSet		*m_pParmVector;	// contains pointers to copes of parameters


public:

//************************************************************************
// LogMessage:
//
// PURPOSE:		Default constructor
//************************************************************************

LogMessage( ListenManager *pListenManager, Event *pEvent );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//
// NOTE:		This method is not supported currently for this object
//************************************************************************

virtual ManagedObject* CreateInstance(){ return NULL; }


//************************************************************************
// ~LogMessage:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~LogMessage();


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
// operator=:
//************************************************************************

virtual const ValueSet& operator=(const ValueSet& obj ){ return *((ValueSet*)this) = obj; }


//************************************************************************
//************************************************************************


};
#endif // __LOG_MESSAGE_H__