//************************************************************************
// FILE:		DataPath.h
//
// PURPOSE:		Defines class that will be used as the base class for
//				all DataPath objects thus is a virtual base class for all the 
//				data path management related stuff.
//************************************************************************

#ifndef __DATA_PATH_H__
#define __DATA_PATH_H__

#include "ManagedObject.h"
#include "StatusReporterInterface.h"

#ifdef WIN32
#pragma pack(4)
#endif

class ConnectionBase;
struct HostConnectionDescriptorRecord;

class DataPath : public ManagedObject, public StatusReporterInterface{

protected:

	// Connection ids are stored as children
	UnicodeString			m_description;	
	UnicodeString			m_name;
	RowId					m_ridName;					// internal
	RowId					m_ridDescription;			// internal
	U32						m_correctConnectionCount;	// internal


public:

//************************************************************************
// DataPath:
//
// PURPOSE:		Default constructor
//************************************************************************

DataPath( ListenManager *pListenManager, U32 objectClassType, ObjectManager *pManager );


//************************************************************************
// ~DataPath:
//
// PURPOSE:		The desructor
//************************************************************************

virtual ~DataPath();


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
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates its data members based on the data in the PTS row
//************************************************************************

bool BuildYourselfFromPtsRow( HostConnectionDescriptorRecord *pRow );


//************************************************************************
// WriteYourselfIntoPtsRow:
//
// PURPOSE:		Populates the PTS row based on its data members
//************************************************************************

bool WriteYourselfIntoPtsRow( HostConnectionDescriptorRecord *pRow );


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Responsible for state reconcilation
//************************************************************************

virtual void ComposeYourOverallState();


//************************************************************************
// Accessors:
//************************************************************************

UnicodeString GetName() const { return m_name; }
UnicodeString GetDescription() const { return m_description; }


//************************************************************************
// IsYourConnection:
//
// PURPOSE:		Checks if a given external port object is hoocked to the 
//				connection
//************************************************************************

bool IsYourConnection( ConnectionBase & );


//************************************************************************
// Set() Methods:
//
// PURPOSE:		Generate events
//************************************************************************

void SetName( UnicodeString name, bool shouldPostEvent = true );
void SetDescription( UnicodeString description, bool shouldPostEvent = true );


//************************************************************************
// SetConnectionState:
//
// PURPOSE:		Informs the object of the state of the connection it's connected
//				to. Currently, the state of the connection is the state of the
//				data path.
//************************************************************************

void SetConnectionState( int connState, U32 connStateString );


//************************************************************************
// Accessors:
//************************************************************************

RowId	GetRidName() const { return m_ridName; }
RowId   GetRidDescription() const { return m_ridDescription; }
void	SetRidName( RowId r ) {  m_ridName = r; }
void	SetRidDescription( RowId r ) {  m_ridDescription = r; } 

public:

//************************************************************************
// operator=:
//
// 
//************************************************************************

virtual const ValueSet& operator=(const ValueSet& obj ){

	return *(ValueSet*)this = obj;
}


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

virtual bool DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// IsAValidConfiguration:
//
// PURPOSE:		Checks the connections assigned to this data path and
//				responds if this is a valid set of connections
//
// OUTPUT:		If the configuration is invalid, 'errorString' will
//				contain the localized string that explains the problem
//************************************************************************

virtual bool IsAValidConfiguration( U32 &errorString ) = 0;


//************************************************************************
// GetPathType:
//
// PURPOSE:		Returns an integer to be stored in the PTS row so that
//				the right type of object could be built next time.
//
// NOTE:		declared pure virtual to enforce implementation of the 
//				method
//************************************************************************

virtual U32 GetPathType() = 0;


};

#endif // __DATA_PATH_H__