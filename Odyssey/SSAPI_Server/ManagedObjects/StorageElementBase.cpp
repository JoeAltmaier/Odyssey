//******************************************************************************
// FILE:		StorageElementBase.cpp
//
// PURPOSE:		Implements the class that will serve as an bastract base class
//				for all storage elements in the O2K
//******************************************************************************

#include "StorageElementBase.h"
#include "StorageIdInfo.h"

//******************************************************************************
// StorageElement:
//
// PURPOSE:		The default constructor
//******************************************************************************

StorageElementBase::StorageElementBase( ListenManager *pListenManager, U32 objectClassType, ObjectManager* pManager )
:ManagedObject( pListenManager, objectClassType, pManager ){
	

	ASSERT( pManager );
	m_isSpare = m_isMember = m_isPoolSpare = false;

	m_capacity = 0;
	m_isUsed = 0;
}


//******************************************************************************
// ~StorageElement:
//
// PURPOSE:		The destructor
//******************************************************************************


StorageElementBase::~StorageElementBase(){

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
StorageElementBase::BuildYourValueSet(){

	int				temp;
	ValueSet		*pDeviceIdsVs = new ValueSet;
	SList			deviceIds;
	StorageIdInfo	idInfo;

	ManagedObject::BuildYourValueSet();

	AddInt64( m_capacity, SSAPI_STORAGE_ELEMENT_B_FID_CAPACITY );
	AddString( &m_name, SSAPI_STORAGE_ELEMENT_B_FID_NAME );
	
	temp = m_isUsed? 1 : 0;
	AddInt( temp, SSAPI_STORAGE_ELEMENT_B_FID_IN_USE );
	
	temp = m_canBeRemoved? 1 : 0;
	AddInt( temp, SSAPI_STORAGE_ELEMENT_B_FID_CAN_BE_DELETED );

	temp = m_isSpare? 1 : 0;
	AddInt( temp, SSAPI_STORAGE_ELEMENT_B_FID_IS_SPARE );

	temp = m_isExportable? 1 : 0;
	AddInt( temp, SSAPI_STORAGE_ELEMENT_B_FID_IS_EXPORTABLE );

	if( !m_isMember )
		if( !m_isSpare || m_isPoolSpare )
			SetYourState();

	// populate and pack the id info object if appropriate
	if( PopulateYourIdInfo( idInfo ) ){
		idInfo.BuildYourValueSet();
		AddValue( &idInfo, SSAPI_STORAGE_ELEMENT_B_FID_ID_INFO_OBJECT );
		idInfo.Clear();
	}

	// prepare and pack underlying devices' IDs
	ReportYourUnderlyingDevices( deviceIds );
	DumpIdVectorIntoValueSet( pDeviceIdsVs, &deviceIds );
	FreeMemoryForTheContainerWithIds( deviceIds );
	AddValue( pDeviceIdsVs, SSAPI_STORAGE_ELEMENT_B_FID_DEVICE_ID_VECTOR );
	
	delete pDeviceIdsVs;

	return StatusReporterInterface::BuildYourValueSet( *this );
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
StorageElementBase::BuildYourselfFromYourValueSet(){

	int		temp;

	ManagedObject::BuildYourselfFromYourValueSet();

	GetInt64( SSAPI_STORAGE_ELEMENT_B_FID_CAPACITY, &m_capacity );
	GetString( SSAPI_STORAGE_ELEMENT_B_FID_NAME, &m_name );
	
	GetInt( SSAPI_STORAGE_ELEMENT_B_FID_IN_USE, &temp );
	m_isUsed = temp? true : false;
	
	GetInt( SSAPI_STORAGE_ELEMENT_B_FID_CAN_BE_DELETED, &temp );
	m_canBeRemoved = temp? true : false;

	GetInt( SSAPI_STORAGE_ELEMENT_B_FID_IS_SPARE, &temp );
	m_isSpare = temp? true : false;

	GetInt( SSAPI_STORAGE_ELEMENT_B_FID_IS_EXPORTABLE, &temp );
	m_isExportable = temp? true : false;

	return StatusReporterInterface::BuildYourselfFromYourValueSet( *this );
}


//************************************************************************
// ApplyNecessaryStatusRollupRules:
//
// PURPOSE:		Applies applicabale rollup rules.
//				The object that implements this method is expected to call
//				the protected method with the same name and specify 
//				necessary parms.
//
// RETURN:		true:			object's state's been changed
//				false:			object's state's remained unchanged
//************************************************************************

bool 
StorageElementBase::ApplyNecessaryStatusRollupRules(){

	bool				rc;

	SetYourState();

	rc = StatusReporterInterface::ApplyNecessaryStatusRollupRules( this, m_children, m_parents );

	if( rc )
		FireEventObjectModifed();

	return rc;
}


//******************************************************************************
// BuildYourselfFromSRCRow:
//
// PURPOSE:		Populates data members based on the SRC row
//******************************************************************************

void 
StorageElementBase::BuildYourselfFromSRCRow( StorageRollCallRecord *pRow ){

	m_id			= DesignatorId( RowId(pRow->rid), (U16)GetClassType() );
	m_isUsed		= pRow->fUsed? true : false;
	m_capacity		= pRow->Capacity * BLOCK_SIZE;
	m_ridName		= pRow->ridName;
	m_ridDescriptor	= pRow->ridDescriptorRecord;

	m_ridPerformanceRecord	= pRow->ridPerformanceRecord;
	m_ridStatusRecord		= pRow->ridStatusRecord;

	m_canBeRemoved	= this->GetCanBeRemoved();
	m_isExportable	= IsExportable();
}


//******************************************************************************
//
//******************************************************************************

void 
StorageElementBase::SetMemberStatus( RAID_MEMBER_STATUS status, bool postEvent  ){

	m_isMember = true;

	switch( status ){
		case RAID_STATUS_UP:
			m_state		= SSAPI_OBJECT_STATE_GOOD;
			m_stateString = CTS_SSAPI_MEMBER_STATE_UP;
			break;

		case RAID_STATUS_DOWN:
			m_state		= SSAPI_OBJECT_STATE_DEAD;
			m_stateString = CTS_SSAPI_MEMBER_STATE_DOWN;
			break;

		case RAID_STATUS_EMPTY:
			m_state		= SSAPI_OBJECT_STATE_UNKNOWN;
			m_stateString = CTS_SSAPI_OBJECT_STATE_NAME_UNKNOWN;
			break;

		case RAID_STATUS_REGENERATING:
			m_state		= SSAPI_OBJECT_STATE_WARNING;
			m_stateString = CTS_SSAPI_MEMBER_STATE_REGENERATING;
			break;
	}

	m_isUsed = true;
	if( postEvent )
		FireEventObjectModifed();
}


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before an object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

void 
StorageElementBase::ComposeYourOverallState(){

	// TBDGAI
}