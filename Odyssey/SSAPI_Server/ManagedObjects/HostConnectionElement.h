//************************************************************************
// FILE:		HostConnectionElement.h
//
// PURPOSE:		Defines class that will be used as the base class for
//				all HostConnection objects and all HostConnectionCollection
//				objects and thus is a virtual base class for all the 
//				host connection management related stuff.
//************************************************************************

#ifndef __HOST_CONNECTION_ELEMENT_H__
#define __HOST_CONNECTION_ELEMENT_H__

#include "ManagedObject.h"
#include "StatusReporterInterface.h"

#ifdef WIN32
#pragma pack(4)
#endif


class HostConnectionElement : public ManagedObject, public StatusReporterInterface{

protected:

	UnicodeString				m_description;	
	UnicodeString				m_name;

public:

//************************************************************************
// HostConnectionElement:
//
// PURPOSE:		Default constructor
//************************************************************************

HostConnectionElement( ListenManager *pListenManager, U32 objectClassType );


//************************************************************************
// ~HostConnectionElement:
//
// PURPOSE:		The desructor
//************************************************************************

virtual ~HostConnectionElement();


//************************************************************************
// ApplyNecessaryStatusRollupRules:
//
// PURPOSE:		Applies applicabale rollup rules.
//				The object that implements this method is expected to call
//				the protected method with the same name and specify 
//				necessary parms.
//
// RETURN:		true:			object's state's been changed
//				false:			object's state's remained unchanged
//************************************************************************

virtual bool ApplyNecessaryStatusRollupRules();


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
// ComposeYourOverallState:
//
// PURPOSE:		Responsible for state reconcilation
//************************************************************************

virtual void ComposeYourOverallState() = 0;


//************************************************************************
// Accessors:
//************************************************************************

UnicodeString GetName() const { return m_name; }
UnicodeString GetDescription() const { return m_description; }
};

#endif // __HOST_CONNECTION_ELEMENT_H__