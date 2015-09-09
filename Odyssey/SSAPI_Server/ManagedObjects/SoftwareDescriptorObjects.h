//************************************************************************
// FILE:		SoftwareDescriptorObjects.h
//
// PURPOSE:		Defines software descriptor objects for all iops
//************************************************************************

#ifndef __SOFTWARE_DESCRIPTOR_OBJECTS_H__
#define	__SOFTWARE_DESCRIPTOR_OBJECTS_H__

#include "SoftwareDescriptor.h"


//************************************************************************
// class SoftwareDescriptorIop:
//
// PURPOSE:		An abstract base class for iop descriptors of sw images
//************************************************************************

class SoftwareDescriptorIop : public SoftwareDescriptor {
protected:

SoftwareDescriptorIop( ListenManager *pLM, U32 classType, ObjectManager *pManager )
:SoftwareDescriptor( pLM, pManager, classType ) {}
};


//************************************************************************
// SoftwareDescriptorNac:
//
// PURPOSE:		Descriptor object for NAC images
//************************************************************************

class SoftwareDescriptorNac : public SoftwareDescriptorIop{
public:
SoftwareDescriptorNac( ListenManager *pLM, ObjectManager *pManager )
:SoftwareDescriptorIop( pLM, SSAPI_OBJECT_CLASS_TYPE_NAC_SW_IMAGE_DESCRIPTOR , pManager ) {}


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new SoftwareDescriptorNac( GetListenManager(), GetManager() ); }
};



//************************************************************************
// SoftwareDescriptorHbc:
//
// PURPOSE:		Descriptor object for HBC images
//************************************************************************

class SoftwareDescriptorHbc : public SoftwareDescriptorIop{
public:
SoftwareDescriptorHbc( ListenManager *pLM, ObjectManager *pManager )
:SoftwareDescriptorIop( pLM, SSAPI_OBJECT_CLASS_TYPE_HBC_SW_IMAGE_DESCRIPTOR , pManager ) {}


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new SoftwareDescriptorHbc( GetListenManager(), GetManager() ); }

};



//************************************************************************
// SoftwareDescriptorSsd:
//
// PURPOSE:		Descriptor object for SSD images
//************************************************************************

class SoftwareDescriptorSsd : public SoftwareDescriptorIop{
public:
SoftwareDescriptorSsd( ListenManager *pLM, ObjectManager *pManager )
:SoftwareDescriptorIop( pLM, SSAPI_OBJECT_CLASS_TYPE_SSD_SW_IMAGE_DESCRIPTOR , pManager ) {}

//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new SoftwareDescriptorSsd( GetListenManager(), GetManager() ); }

};

#endif // __SOFTWARE_DESCRIPTOR_OBJECTS_H__