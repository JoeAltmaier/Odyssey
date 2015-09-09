//************************************************************************
// FILE:		DownstreamConnection.h
//
// PURPOSE:		Declares the object used to represent connections to storage
//				found on loops attached to our NACs
//************************************************************************

#ifndef __DOWNSTREAM_CONNECTION_H__
#define	__DOWNSTREAM_CONNECTION_H__


#include "Connection.h"
#include "FCPortDatabaseTable.h"

#ifdef WIN32
#pragma pack(4)
#endif


class DownstreamConnection : public ConnectionBase {

public:
//************************************************************************
// DownstreamConnection:
//
// PURPOSE:		Default constructor
//************************************************************************

DownstreamConnection( ListenManager *pListenManager, ObjectManager *pManager )
:ConnectionBase( pListenManager, SSAPI_OBJECT_CLASS_TYPE_DOWNSTREAM_CONNECTION, pManager ){
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

virtual ManagedObject* CreateInstance(){ return new DownstreamConnection( GetListenManager(), GetManager() ); }


//************************************************************************
// WriteYourselfIntoPtsRow:
//
// PURPOSE:		Populates the table row based on the data members
//
// NOTE:		declared virtual so that subclasses could write their types
//************************************************************************

virtual void WriteYourselfIntoPtsRow( FCPortDatabaseRecord *pRow ){

	pRow->portType	=	FC_PORT_TYPE_TARGET;

	ConnectionBase::WriteYourselfIntoPtsRow( pRow );
}

};

#endif	// __DOWNSTREAM_CONNECTION_H__