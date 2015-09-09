//******************************************************************************
// FILE:		StorageCollectionSparePool.h
//
// PURPOSE:		Defines object to be used to represent sparepools in the
//				O2K system
//******************************************************************************

#ifndef __STORAGE_SPARE_POOL_H__
#define	__STORAGE_SPARE_POOL_H__

#include "StorageCollection.h"

#ifdef WIN32
#pragma pack(4)
#endif

class StorageCollectionSparePool : public StorageCollection {

	DesignatorId			m_dedicatedHostId;		// id of the designated host

public:

//******************************************************************************
// StorageCollectionSparePool:
// 
// PURPOSE:		Default constructor
//******************************************************************************

StorageCollectionSparePool( ListenManager *pListenManager, DesignatorId hostId, ObjectManager *pManager );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new StorageCollectionSparePool( GetListenManager(), m_dedicatedHostId, GetManager() ); }


//******************************************************************************
// ~StorageCollectionSparePool:
//
// PURPOSE:		The destructor
//******************************************************************************

~StorageCollectionSparePool();


//************************************************************************
// IsYourElement:
//
// PURPOSE:		Determines if a gven element belongs to this element
//				in the logical hierachy. 
//
// RETURN:		true:		yes
//				false:		no
//************************************************************************

virtual bool IsYourElement( StorageElementBase &element );


//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		The method is responsible for adding all data to be 
//				transfered to a client into its value set. All derived 
//				objects must override this method if they have data members
//				they want client proxies to have. 
//
// NOTE:		If a derived object overrides this this method, it MUST call
//				it in its base class BEFORE doing any work!.
//
// RETURN:		true:		success
//************************************************************************

virtual bool BuildYourValueSet();


//************************************************************************
// BuildYourselfFromYourValueSet:
//
// PURPOSE:		Populates data members based on the underlying value set
//
// NOTE:		All subclasses that override this method must call to it
//				somewhere in the overriding method
//
// RETURN:		true:		success
//************************************************************************

virtual bool BuildYourselfFromYourValueSet();


//************************************************************************
// IsYourElement:
//
// PURPOSE:		Determines if a gven element belongs to this element
//				in the logical hierachy. 
//
// RETURN:		true:		yes
//				false:		no
//************************************************************************

virtual bool IsYourElement( StorageElementBase &element, StorageManager *pManager );


//******************************************************************************
// Smart/virtual Accessors
//******************************************************************************

virtual LocalizedString GetName() { return CTS_SSAPI_SPARE_POOL_NAME; }
virtual bool GetCanBeRemoved() { return false; }
virtual bool IsExportable() { return false; }

//******************************************************************************
// ReconcileCapacity:
//
// PURPOSE:		Checks the capacity of the spare pool and posts an event if it has
//				changed.
//******************************************************************************

void ReconcileCapacity();


//************************************************************************
// GetCoreTableMask:
//
// PURPOSE:		The methods returns the integer mask of the core table
//				withing the StorageManager. The core table for a storage 
//				element is the table which defines the very existance
//				of the storage element.
//				Before, we only had one such table -- the StorageRollCall
//				table. Now, there is another one - the DeviceDescriptor.
//
// 
//************************************************************************

virtual U32 GetCoreTableMask( );

};
#endif // __STORAGE_SPARE_POOL_H__