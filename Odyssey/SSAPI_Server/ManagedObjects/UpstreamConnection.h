//************************************************************************
// FILE:		UpstreamConnection.h
//
// PURPOSE:		Declares the object used to reprsent initiator-type ports
//				found on loops attached to our NACs
//************************************************************************

#ifndef __UPSTREAM_CONNECTION_H__
#define	__UPSTREAM_CONNECTION_H__


#include "Connection.h"
#include "FCPortDatabaseTable.h"

#ifdef WIN32
#pragma pack(4)
#endif


class UpstreamConnection : public ConnectionBase {

public:

//************************************************************************
// UpstreamConnection:
//
// PURPOSE:		Default constructor
//************************************************************************

UpstreamConnection( ListenManager *pListenManager, ObjectManager *pManager )
:ConnectionBase( pListenManager, SSAPI_OBJECT_CLASS_TYPE_UPSTREAM_CONNECTION, pManager  ){
}



//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new UpstreamConnection( GetListenManager(), GetManager() ); }


//************************************************************************
// WriteYourselfIntoPtsRow:
//
// PURPOSE:		Populates the table row based on the data members
//
// NOTE:		declared virtual so that subclasses could write their types
//************************************************************************

virtual void WriteYourselfIntoPtsRow( FCPortDatabaseRecord *pRow ){

	pRow->portType	=	FC_PORT_TYPE_INITIATOR;

	ConnectionBase::WriteYourselfIntoPtsRow( pRow );
}

};

#endif	// __UPSTREAM_CONNECTION_H__