//************************************************************************
// FILE:		ExternalInitiatorPort.h
//
// PURPOSE:		Declares the object used to reprsent initiator-type ports
//				found on loops attached to our NACs
//************************************************************************

#ifndef __EXTERNAL_INITIATOR_PORT_H__
#define	__EXTERNAL_INITIATOR_PORT_H__


#include "ExternalPort.h"
#include "FCPortDatabaseTable.h"

#ifdef WIN32
#pragma pack(4)
#endif


class ExternalInitiatorPort : public ExternalPort {

public:

//************************************************************************
// ExternalInitiatorPort:
//
// PURPOSE:		Default constructor
//************************************************************************

ExternalInitiatorPort( ListenManager *pListenManager )
:ExternalPort( pListenManager, SSAPI_OBJECT_CLASS_TYPE_EXTERNAL_INITIATOR_PORT ){
}


//************************************************************************
// WriteYourselfIntoPtsRow:
//
// PURPOSE:		Populates the table row based on the data members
//
// NOTE:		declared virtual so that subclasses could write their types
//************************************************************************

virtual void WriteYourselfIntoPtsRow( FCPortDatabaseRecord *pRow ){

	pRow->portType	=	FC_PORT_TYPE_INITIATOR;

	ExternalPort::WriteYourselfIntoPtsRow( pRow );
}

};

#endif	// __EXTERNAL_INITIATOR_PORT_H__