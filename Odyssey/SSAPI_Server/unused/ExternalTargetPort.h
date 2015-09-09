//************************************************************************
// FILE:		ExternalTargetPort.h
//
// PURPOSE:		Declares the object used to reprsent initiator-type ports
//				found on loops attached to our NACs
//************************************************************************

#ifndef __EXTERNAL_TARGET_PORT_H__
#define	__EXTERNAL_TARGET_PORT_H__


#include "ExternalPort.h"
#include "FCPortDatabaseTable.h"

#ifdef WIN32
#pragma pack(4)
#endif


class ExternalTargetPort : public ExternalPort {

public:
//************************************************************************
// ExternalTargetPort:
//
// PURPOSE:		Default constructor
//************************************************************************

ExternalTargetPort( ListenManager *pListenManager )
:ExternalPort( pListenManager, SSAPI_OBJECT_CLASS_TYPE_EXTERNAL_TARGET_PORT ){
}


//************************************************************************
// WriteYourselfIntoPtsRow:
//
// PURPOSE:		Populates the table row based on the data members
//
// NOTE:		declared virtual so that subclasses could write their types
//************************************************************************

virtual void WriteYourselfIntoPtsRow( FCPortDatabaseRecord *pRow ){

	pRow->portType	=	FC_PORT_TYPE_TARGET;

	ExternalPort::WriteYourselfIntoPtsRow( pRow );
}

};

#endif	// __EXTERNAL_TARGET_PORT_H__