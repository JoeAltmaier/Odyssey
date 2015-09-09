//************************************************************************
// FILE:		Nac.h
//					
// PURPOSE:		Define class Nac : public Iop that will be used to 
//				represent NAC board in the O2K product
//************************************************************************

#ifndef __SSAPI_NAC_H__
#define __SSAPI_NAC_H__

#include "Iop.h"
#include "IOPStatusTable.h"

class Nac : public Iop {

public:

//************************************************************************
// Nac:
//
// PURPOSE:		Default constructor
//************************************************************************

Nac( ListenManager *pListenManager, U32 classType = SSAPI_OBJECT_CLASS_TYPE_NAC_BOARD );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new Nac( GetListenManager() ); }


//************************************************************************
// Assignment operator overloaded
//************************************************************************

const ValueSet& operator=(const ValueSet& obj ){ *(ValueSet *)this = obj; return obj; }


//************************************************************************
// IsYourDevice:
//
// PURPOSE:		Determines if a gven device belongs to this device
//				in the logical hierachy. 
//
// RETURN:		true:		yes
//				false:		no
//************************************************************************

virtual bool IsYourDevice( Device &device );


protected:

//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		CAssigns the name and state (via Iop class)
//************************************************************************

virtual void ComposeYourOverallState();

};


#endif // __SSAPI_NAC_H__