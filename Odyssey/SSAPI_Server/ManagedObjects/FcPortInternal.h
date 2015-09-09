//************************************************************************
// FILE:		FcPortInternal
//
// PURPOSE:		Defines object used to represent FC ports that serve 
//				internal disks in the O2K box.
//************************************************************************

#ifndef __FC_PORT_INTERNAL_H__
#define	__FC_PORT_INTERNAL_H__

#include "FcPort.h"

#ifdef WIN32
#pragma pack(4)
#endif


class FcPortInternal : public FcPort {


public:

//************************************************************************
// FcPortInternal:
//
// PURPOSE:		Default constructor
//************************************************************************

FcPortInternal( ListenManager *pListenManager )
:FcPort( pListenManager, SSAPI_OBJECT_CLASS_TYPE_INTERNAL_FC_PORT ){

	m_name = CTS_SSAPI_INTERN_FCPORT_DEVICE_NAME;
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

virtual ManagedObject* CreateInstance(){ return new FcPortInternal( GetListenManager() ); }

};

#endif // __FC_PORT_INTERNAL_H__