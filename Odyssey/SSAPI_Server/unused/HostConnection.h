//************************************************************************
// FILE:		HostConnection.h
//
// PURPOSE:		Dedines class used to represent connections between
//				a host's  and NAC's port.
//************************************************************************

#ifndef __HOST_CONNECTION_H__
#define	__HOST_CONNECTION_H__

#include "HostConnectionElement.h"
#include "HostConnectionDescriptorTable.h"

#ifdef WIN32
#pragma pack(4)
#endif

class HostConnection : public HostConnectionElement{
		
	// EIP ids are stored as children
	RowId					m_ridName;
	RowId					m_ridDescription;
	U32						m_mode;
	U32						m_correctEipCount;
	DesignatorId			m_hostId;

	
public:

//************************************************************************
// HostConnection:
//
// PURPOSE:		The default constructor
//************************************************************************

HostConnection( ListenManager *pListenManager );


//************************************************************************
// ~HostConnection:
//
// PURPOSE:		The destructor
//************************************************************************

~HostConnection();


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
// ModifyObject:
//
// PURPOSE:		Modifes contents of the object
//
// NOTE:		Must be overridden by objects that can be modified
//************************************************************************

virtual bool ModifyObject( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

virtual bool DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder );


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
// operator=:
//
// 
//************************************************************************

virtual const ValueSet& operator=(const ValueSet& obj ){

	return *(ValueSet*)this = obj;
}

//************************************************************************
// SetPortState:
//
// PURPOSE:		Informs the object of the state of the port it's connected
//				to. Currently, the state of the port is the state of the
//				connection.
//************************************************************************

void SetPortState( int portState, U32 portStateString );


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Responsible for state reconcilation
//************************************************************************

virtual void ComposeYourOverallState();


//************************************************************************
// IsYourExternalPort:
//
// PURPOSE:		Checks if a given external port object is hoocked to the 
//				connection
//************************************************************************

bool IsYourExternalPort( ExternalPort &port );


//************************************************************************
// Set() Methods:
//
// PURPOSE:		Generate events
//************************************************************************

void SetName( UnicodeString name, bool shouldPostEvent = true );
void SetDescription( UnicodeString description, bool shouldPostEvent = true );


//************************************************************************
// Accessors:
//************************************************************************

RowId	GetRidName() const { return m_ridName; }
RowId   GetRidDescription() const { return m_ridDescription; }
void	SetRidName( RowId r ) {  m_ridName = r; }
void	SetRidDescription( RowId r ) {  m_ridDescription = r; } 


};

#endif	// __HOST_CONNECTION_H__
