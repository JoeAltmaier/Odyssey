//******************************************************************************
// FILE:		StorageElementDiskInternal.h
//
// PURPOSE:		Defines the object used to represent internal disks
//******************************************************************************

#ifndef __DISK_INTERNAL_H__
#define __DISK_INTERNAL_H__

#include "StorageElementDisk.h"
#include "DeviceManager.h"

#ifdef WIN32
#pragma pack(4)
#endif


class StorageElementDiskInternal : public StorageElementDisk {

public:

//******************************************************************************
// StorageElementDiskInternal
//
// PURPOSE:		Default constructor
//******************************************************************************

StorageElementDiskInternal( ListenManager *pListenManager, ObjectManager *pManager )
:StorageElementDisk( pListenManager, SSAPI_OBJECT_CLASS_TYPE_DISK_INTERNAL, pManager ) {}


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new StorageElementDiskInternal( GetListenManager(), GetManager() ); }


//******************************************************************************
// ~StorageElementDiskInternal:
//
// PURPOSE:		The destructor
//******************************************************************************

virtual ~StorageElementDiskInternal() {}


//******************************************************************************
// ReportYourUnderlyingDevices:
//
// PURPOSE:		Gives a chance to every Storage Element to report devices that
//				make it up.
//
// FUNTIONALITY: Derived classes must populate the specified vector with IDs of
//				devices that they contain of. Memory allocated will be freed
//				by the caller, so derived classes must not deallocate anything.
//******************************************************************************

virtual void ReportYourUnderlyingDevices( Container &devices ){

	DesignatorId	*pId = new DesignatorId();
	DeviceManager	*pDManager = (DeviceManager *)GetObjectManager( GetManager(), SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER );

	if( !pDManager->GetDesignatorIdByRowId( m_ridDescriptor, *pId ) ){
		delete pId;
		ASSERT( 0 );
	}
	else {
		devices.Add( (CONTAINER_ELEMENT) pId );
	}
}

};

#endif // __DISK_INTERNAL_H__