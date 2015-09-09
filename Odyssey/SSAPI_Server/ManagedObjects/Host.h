//************************************************************************
// FILE:		Host.h
//
// PURPOSE:		Defines the class Host used to represent host data and
//				connections in the O2K product.
//************************************************************************

#ifndef __HOST_MO_H__
#define __HOST_MO_H__

#include "ManagedObject.h"
#include "SSAPITypes.h"
#include "SSAPIAssert.h"
#include "UnicodeString.h"
#include "HostDescriptorTable.h"
#include "StatusReporterInterface.h"
#include "CoolVector.h"

class ShadowTable;
class Container;
class Connection;


#ifdef WIN32
#pragma pack(4)
#endif

class Host : public ManagedObject, public StatusReporterInterface {

	UnicodeString		m_name;
	UnicodeString		m_description;
	int					m_os;
	int					m_ipAddress;
	CoolVector			m_connectionIds;	// stores ptrs to DesignatorId objects					
	U32					m_correctEipCount;	// internal, shows what *must* be


public:


//************************************************************************
// Host:
//
// PURPOSE:		The default constructor
//************************************************************************

Host( ListenManager *pListenManager, ObjectManager *pManager );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new Host( GetListenManager(), GetManager() ); }


//************************************************************************
// ~Host:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~Host();


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
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

virtual bool DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// ModifyObject:
//
// PURPOSE:		Modifes contents of the object
//
// NOTE:		Must be overridden by objects that can be modified
//************************************************************************

virtual bool ModifyObject( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// operator=:
//
//************************************************************************

virtual const ValueSet& operator=(const ValueSet& obj ){ return *((ValueSet *)this) = obj; }


//************************************************************************
// WriteYourSelfIntoHostDescriptorRow:
//
// PURPOSE:		Writes out data from data members into the PTS record
//************************************************************************

bool WriteYourSelfIntoHostDescriptorRow( HostDescriptorRecord *pRow );


//************************************************************************
// BuildYourSelfFromHostDescriptorRow:
//
// PURPOSE:		Populates data members based on the data from the PTS
//				row
//************************************************************************

bool BuildYourSelfFromHostDescriptorRow( HostDescriptorRecord *pRow );


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
// AreThereAnyTooLongStrings
//
// PURPOSE:		Checks every string's size against the maximum in the PTS 
//				row. Will raise an exception if something's wrong.
//				Checks strings in the value set, not the members. 
//
// RETURN:		true:	all strings are OK, may proceed
//				false:	an exception was rased, terminate normal execution
//************************************************************************

bool AreThereAnyTooLongStrings( SsapiResponder *pResponder );


//************************************************************************
// ComposeYourOverallStatus:
//
// PURPOSE:		Runs thru its connections checking if they are present and
//				what their status is. Detemines its state based ob the
//				information collected.
//************************************************************************

virtual void ComposeYourOverallState();


//************************************************************************
// Connection Methods:
//
// PURPOSE:		The following methods do basic operations on the 
//				connection id vector data member
//************************************************************************

bool IsYourConnection( const DesignatorId &id );
void AddConnectionId( const DesignatorId &id );
bool GetConnectionIdAt( U32 position, DesignatorId &id );
bool RemoveConnectionId( const DesignatorId &id );
bool RemoveConnectionIdAt( U32 position );
U32	 GetConnectionIdCount() const { return m_connectionIds.Count(); }

private:

//************************************************************************
// ClearIdVector:
//
// PURPOSE:		Deallocates memory taken by DesignatorId objects in the
//				vector specified.
//************************************************************************

void ClearIdVector( Container *pIdVector );

//************************************************************************
//************************************************************************


};


#endif // __HOST_MO_H__



