//******************************************************************************
// FILE:		StorageElementDiskExternal.h
//
// PURPOSE:		Defines the object used to represent external disks
//******************************************************************************

#ifndef __DISK_EXTERNAL_H__
#define __DISK_EXTERNAL_H__

#include "StorageElementDisk.h"
#include "DeviceManager.h"

#ifdef WIN32
#pragma pack(4)
#endif


class StorageElementDiskExternal : public StorageElementDisk {

public:

//******************************************************************************
// StorageElementDiskExternal
//
// PURPOSE:		Default constructor
//******************************************************************************

StorageElementDiskExternal( ListenManager *pListenManager, ObjectManager *pManager )
:StorageElementDisk( pListenManager, SSAPI_OBJECT_CLASS_TYPE_DISK_EXTERNAL, pManager ) {}


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new StorageElementDiskExternal( GetListenManager(), GetManager() ); }


//******************************************************************************
// ~StorageElementDiskExternal:
//
// PURPOSE:		The destructor
//******************************************************************************

virtual ~StorageElementDiskExternal() {}


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

	DesignatorId	id;
	DeviceManager	*pDManager = (DeviceManager *)GetObjectManager( GetManager(), SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER );
	
	id = pDManager->GetPortByInstanceNumber( m_instanceNumber );
	if( id == DesignatorId() ){
		ASSERT( 0 );
		return;
	}
	devices.Add( (CONTAINER_ELEMENT) new DesignatorId( id ) );
}


};

#endif // __DISK_INTERNAL_H__