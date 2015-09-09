//************************************************************************
// FILE:		Nac.cpp
//					
// PURPOSE:		Define class Nac : public Iop that will be used to 
//				represent NAC board in the O2K product
//************************************************************************

#include "Nac.h"
#include "FcPort.h"

//************************************************************************
// NacLogicalDeviceHierachy:
//
// PURPOSE:		Contains class type ids of the devices that this device
//				owns.
//************************************************************************

static const U32 NacLogicalDeviceHierachy[] = {
		SSAPI_OBJECT_CLASS_TYPE_FC_PORT,
		SSAPI_OBJECT_CLASS_TYPE_INTERNAL_FC_PORT
};



//************************************************************************
// Nac:		
//
// PURPOSE:		Default constructor
//************************************************************************

Nac::Nac( ListenManager *pListenManager, U32 classType ) 
	:Iop( pListenManager, classType ){
}


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		CAssigns the name and state (via Iop class)
//************************************************************************

void 
Nac::ComposeYourOverallState(){
	Iop::ComposeYourOverallState();
	m_name	= CTS_SSAPI_NAC_NAME;
}


//************************************************************************
// IsYourDevice:
//
// PURPOSE:		Determines if a gven device belongs to this device
//				in the logical hierachy. 
//
// RETURN:		true:		yes
//				false:		no
//************************************************************************

bool 
Nac::IsYourDevice( Device &device ){


	bool				rc;

	rc = IsThisClassTypeInThisArray(	device.GetClassType(), 
										NacLogicalDeviceHierachy, 
										sizeof( NacLogicalDeviceHierachy ) / sizeof( NacLogicalDeviceHierachy[0] ) );

	if( !rc )
		return false;

	return ( ((FcPort&)device).GetSlotNumber() == (U32)m_slotNumber ) ? true : false;
}
