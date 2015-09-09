//************************************************************************
// FILE:		DeviceCollection.h
//
// PURPOSE:		Defines a class DeviceCollection that will contain
//				somehow associated device objects used to represent
//				devices in the O2K product. The state of a device collection
//				is defined as the worst state of any of its elements
//************************************************************************

#ifndef	__DEVICE_COLECTION_H__
#define	__DEVICE_COLECTION_H__

#include "Device.h"


#ifdef WIN32
#pragma pack(4)
#endif


class DeviceCollection : public Device {

public:


//************************************************************************
// ~DeviceCollection:
//
// PURPOSE:		The destructor
//************************************************************************

	virtual ~DeviceCollection(){}


//************************************************************************
// Assignment operator overloaded
//************************************************************************

const ValueSet& operator=(const ValueSet& obj ){ *(ValueSet *)this = obj; return obj; }


protected:


//************************************************************************
// DeviceCollection:
//
// PURPOSE:		Default constructor
//************************************************************************

DeviceCollection( ListenManager *pListenManager, U32 objectClassType )  
	:Device( pListenManager, objectClassType ){
}

//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before a device object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

virtual void ComposeYourOverallState();

};

#endif	// __DEVICE_COLECTION_H__