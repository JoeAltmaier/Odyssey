//************************************************************************
// FILE:		SNac.h
//
// PURPOSE:		Defines the objects used to represent special NAC boards
//				in the O2K
//************************************************************************

#ifndef __SNAC_MO_H__
#define __SNAC_MO_H__

#include "Nac.h"

#ifdef WIN32
#pragma pack(4)
#endif

class SNac : public Nac {

public:

//************************************************************************
// SNac:
//
// PURPOSE:		Default constructor
//************************************************************************

SNac( ListenManager *pListenManager ) : Nac( pListenManager, SSAPI_OBJECT_CLASS_TYPE_SNAC_BOARD ) {}; 



//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new SNac( GetListenManager() ); }


protected:

//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		CAssigns the name and state (via Iop class)
//************************************************************************

virtual void ComposeYourOverallState(){

	Iop::ComposeYourOverallState();
	m_name	= CTS_SSAPI_SNAC_DEVICE_NAME;
}

};

#endif //__SNAC_MO_H__