//************************************************************************
// FILE:		Connection.h
//
// PURPOSE:		Defines an abstract base class for connections to external 
//				ports
//************************************************************************

#ifndef __CONNECTION_H__
#define	__CONNECTION_H__

#include "ManagedObject.h"
#include "StatusReporterInterface.h"

struct FCPortDatabaseRecord;

#ifdef WIN32
#pragma pack(4)
#endif

class ConnectionBase : public ManagedObject, public StatusReporterInterface {

	U32					m_i_t_id;		// initiator || target
	UnicodeString		m_wwName;
	DesignatorId		m_portId;
	UnicodeString		m_name;
	RowId				m_ridName;		// internal
	bool				m_isInvalidConfiguration;


public:

//************************************************************************
// ConnectionBase:
//
// PURPOSE:		Default constructor
//************************************************************************

ConnectionBase( ListenManager *pListenManager, U32 classType, ObjectManager *pManager );


//************************************************************************
// ~ConnectionBase:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~ConnectionBase();


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
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates data members based on the contents of the row
//************************************************************************

void BuildYourselfFromPtsRow( FCPortDatabaseRecord *pRow, ObjectManager* );


//************************************************************************
// WriteYourselfIntoPtsRow:
//
// PURPOSE:		Populates the table row based on the data members
//
// NOTE:		declared virtual so that subclasses could write their types
//************************************************************************

virtual void WriteYourselfIntoPtsRow( FCPortDatabaseRecord *pRow );


//************************************************************************
// operator=
//************************************************************************


virtual const ValueSet& operator=(const ValueSet& obj ){ return *this = obj; }


//************************************************************************
// ComposeYourOverallStatus:
//
// PURPOSE:		Runs thru its children checking if they are present and
//				what their status is. Detemines its state based ob the
//				information collected.
//************************************************************************

virtual void ComposeYourOverallState();


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

virtual bool DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// Accessors
//************************************************************************

DesignatorId	GetGeminiPortId() const			{ return m_portId; }
U32				GetTargetInitiatorId() const	{ return m_i_t_id; }
RowId			GetRidName() const				{ return m_ridName; }
void			SetRidName( RowId rid )			{ if( RowId() == rid ) m_name = ""; m_ridName = rid; }
void			SetName( UnicodeString name )	{ m_name = name; FireEventObjectModifed(); }


//************************************************************************
//************************************************************************

protected:


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

bool  ApplyNecessaryStatusRollupRules();

};

#endif	// __CONNECTION_H__