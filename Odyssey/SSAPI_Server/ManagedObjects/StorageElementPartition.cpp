//************************************************************************
// FILE:		StorageElementPartition.cpp
//
// PURPOSE:		Implements the object whose instance will be used to represent
//				internally created partitions in the O2K system.
//************************************************************************

#include "StorageElementPartition.h"
#include "StorageManager.h"


//************************************************************************
// StorageElementPartition:
//
// PURPOSE:		Default constructor
//************************************************************************

StorageElementPartition::StorageElementPartition( ListenManager *pLM, ObjectManager *pManager )
:StorageElement( pLM, SSAPI_OBJECT_CLASS_TYPE_PARTITION_STORAGE_ELEMENT, pManager ){
}


//************************************************************************
// ~StorageElementPartition:
//
// PURPOSE:		The destructor
//************************************************************************

StorageElementPartition::~StorageElementPartition(){
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates data members based on the pts row
//************************************************************************

void 
StorageElementPartition::BuildYourselfFromPtsRow( PARTITION_DESCRIPTOR *pRow ){

	bool			assertFlag;
	StorageManager	*pSManager = (StorageManager *)GetManager();

	m_partitionRid = pRow->rid;

	assertFlag	= pSManager->GetDesignatorIdByRowId( pRow->parentSRCTRID, m_ownerId );

	if( RowId( pRow->nextRowId ).IsClear() == false )
		assertFlag	= pSManager->GetDesignatorIdByRowId( pRow->nextRowId, m_nextPartitionId );

	if( RowId( pRow->previousRowId ).IsClear() == false )
		assertFlag	= pSManager->GetDesignatorIdByRowId( pRow->previousRowId, m_prevPartitionId );
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
StorageElementPartition::BuildYourValueSet(){
	
	StorageElement::BuildYourValueSet();

	AddGenericValue((char *) &m_ownerId, sizeof(m_ownerId), SSAPI_PARTITION_STORAGE_ELEMENT_FID_OWNER_ID );
	AddGenericValue((char *) &m_prevPartitionId, sizeof(m_prevPartitionId), SSAPI_PARTITION_STORAGE_ELEMENT_FID_PREV_ID );
	AddGenericValue((char *) &m_nextPartitionId, sizeof(m_nextPartitionId), SSAPI_PARTITION_STORAGE_ELEMENT_FID_NEXT_ID );

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
StorageElementPartition::BuildYourselfFromYourValueSet(){

	StorageElement::BuildYourselfFromYourValueSet();

	GetGenericValue((char *) &m_ownerId, sizeof(m_ownerId), SSAPI_PARTITION_STORAGE_ELEMENT_FID_OWNER_ID );
	GetGenericValue((char *) &m_prevPartitionId, sizeof(m_prevPartitionId), SSAPI_PARTITION_STORAGE_ELEMENT_FID_PREV_ID );
	GetGenericValue((char *) &m_nextPartitionId, sizeof(m_nextPartitionId), SSAPI_PARTITION_STORAGE_ELEMENT_FID_NEXT_ID );

	return true;
}


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

void 
StorageElementPartition::ReportYourUnderlyingDevices( Container &devices ){

	StorageElement *pElement = (StorageElement *)((StorageManager *)GetManager())->GetManagedObject( &m_ownerId );

	if( pElement )
		pElement->ReportYourUnderlyingDevices( devices );
	else
		ASSERT( pElement );
}


//******************************************************************************
// SetYourState:
//
// PURPOSE:		Requires that sub classes set their state and state string
//******************************************************************************

void 
StorageElementPartition::SetYourState(){

	// TBDGAI!!!
	m_state			= SSAPI_OBJECT_STATE_GOOD;
	m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
}

