//******************************************************************************
// FILE:		StorageCollectionSparePool.cpp
//
// PURPOSE:		Implements object to be used to represent sparepools in the
//				O2K system
//******************************************************************************

#include "StorageCollectionSparePool.h"
#include "StorageElement.h"
#include "StorageManager.h"

//******************************************************************************
// StorageCollectionSparePool:
// 
// PURPOSE:		Default constructor
//******************************************************************************

StorageCollectionSparePool::StorageCollectionSparePool( ListenManager *pListenManager, DesignatorId hostId, ObjectManager *pManager )
:StorageCollection( pListenManager, SSAPI_OBJECT_CLASS_TYPE_STORAGE_COLL_SPARE_POOL, pManager){

	m_id = DesignatorId( RowId(), SSAPI_OBJECT_CLASS_TYPE_STORAGE_COLL_SPARE_POOL, hostId.GetRowId().LoPart );
	m_dedicatedHostId = hostId;
	m_isUsed = true;
}


//******************************************************************************
// ~StorageCollectionSparePool:
//
// PURPOSE:		The destructor
//******************************************************************************

StorageCollectionSparePool::~StorageCollectionSparePool(){
}


//************************************************************************
// IsYourElement:
//
// PURPOSE:		Determines if a gven element belongs to this element
//				in the logical hierachy. 
//
// RETURN:		true:		yes
//				false:		no
//************************************************************************

bool 
StorageCollectionSparePool::IsYourElement( StorageElementBase &element ){

	if( (element.GetIsPoolSpare()) && (m_dedicatedHostId == element.GetDedicatedHostId() ) )
		return true;
	
	return false;
}

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

bool 
StorageCollectionSparePool::BuildYourValueSet(){
	
	U32					i;
	StorageElement		*pElement;
	DesignatorId		id;

	// collect your size
	m_capacity = 0;
	for( i = 0; i < GetChildCount(); i++ ){
		pElement = (StorageElement *)GetChild( i );
		m_capacity += pElement->GetCapacity();
	}
	
	
	StorageCollection::BuildYourValueSet();

	AddGenericValue( (char *)&m_dedicatedHostId, sizeof(m_dedicatedHostId), SSAPI_STORAGE_COLL_SPAREPOOL_FID_HOST_ID );

	return true;
}


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

bool 
StorageCollectionSparePool::BuildYourselfFromYourValueSet(){

	StorageCollection::BuildYourselfFromYourValueSet();

	GetGenericValue( (char *)&m_dedicatedHostId, sizeof(m_dedicatedHostId), SSAPI_STORAGE_COLL_SPAREPOOL_FID_HOST_ID );

	return true;

}


//************************************************************************
// IsYourElement:
//
// PURPOSE:		Determines if a gven element belongs to this element
//				in the logical hierachy. 
//
// RETURN:		true:		yes
//				false:		no
//************************************************************************

bool 
StorageCollectionSparePool::IsYourElement( StorageElementBase &element, StorageManager *){

	return IsYourChild( element.GetDesignatorId() );
}


//******************************************************************************
// ReconcileCapacity:
//
// PURPOSE:		Checks the capacity of the spare pool and posts an event if it has
//				changed.
//******************************************************************************

void 
StorageCollectionSparePool::ReconcileCapacity(){
	
	I64				capacity = 0;
	StorageElement	*pElement;
	U32				i;

	for( i = 0; i < GetChildCount(); i++ ){
		pElement = (StorageElement *)GetChild( i );
		capacity += pElement->GetCapacity();
	}

	if( capacity != m_capacity ){
		m_capacity	= capacity;
		FireEventObjectModifed();
	}
}


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

U32 
StorageCollectionSparePool::GetCoreTableMask( ){

	return SSAPI_SM_SRC_TABLE;
}