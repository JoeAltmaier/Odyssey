//************************************************************************
// FILE:		StorageElementArray.cpp
//
// PURPOSE:		Implements the object that will be used to represent RAID
//				arays inside the O2K system. This is an abstract class
//				to be extended by specific RAID level type objects
//************************************************************************


#include "StorageElementArray.h"
#include "StorageManager.h"


//************************************************************************
// StorageElementArray:
//
// PURPOSE:		Default constructor
//************************************************************************

StorageElementArray::StorageElementArray( ListenManager *pListenManager, U32 objectClassType, ObjectManager *pManager )
:StorageElement( pListenManager, objectClassType, pManager ){
}


//************************************************************************
// ~StorageElementArray:
//
// PURPOSE:		Default destructor
//************************************************************************

StorageElementArray::~StorageElementArray(){
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
StorageElementArray::BuildYourValueSet(){

	int temp;
	
	StorageElement::BuildYourValueSet();

	AddInt64( m_memberCapacity, SSAPI_STORAGE_ELEMENT_ARRAY_FID_MEMBER_CAPACITY );
	AddInt64( m_dataBlockSize, SSAPI_STORAGE_ELEMENT_ARRAY_FID_DATA_BLOCK_SIZE );
	AddInt64( m_parityBlockSize, SSAPI_STORAGE_ELEMENT_ARRAY_FID_PARITY_BLOCK_SIZE );
	AddU32( m_peckingOrder, SSAPI_STORAGE_ELEMENT_ARRAY_FID_PECKING_ORDER );

	temp = m_isInited? 1 : 0;
	AddInt( temp, SSAPI_STORAGE_ELEMENT_ARRAY_FID_IS_INITED );
	AddU32( m_serialNumber, SSAPI_STORAGE_ELEMENT_ARRAY_FID_SERIAL_NUMBER );
	AddGenericValue( (char *)&m_hostSparePoolId, sizeof(m_hostSparePoolId), SSAPI_STORAGE_ELEMENT_ARRAY_FID_HOST_SPAREPOOL_ID ); 
	AddInt64( m_creationTime, SSAPI_STORAGE_ELEMENT_ARRAY_FID_CREATION_TIME );

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
StorageElementArray::BuildYourselfFromYourValueSet(){

	int temp;
	
	StorageElement::BuildYourselfFromYourValueSet();

	GetInt64( SSAPI_STORAGE_ELEMENT_ARRAY_FID_MEMBER_CAPACITY, &m_memberCapacity );
	GetInt64( SSAPI_STORAGE_ELEMENT_ARRAY_FID_DATA_BLOCK_SIZE, &m_dataBlockSize );
	GetInt64( SSAPI_STORAGE_ELEMENT_ARRAY_FID_PARITY_BLOCK_SIZE, &m_parityBlockSize );
	GetInt( SSAPI_STORAGE_ELEMENT_ARRAY_FID_PECKING_ORDER, (int *)&m_peckingOrder );

	GetInt( SSAPI_STORAGE_ELEMENT_ARRAY_FID_IS_INITED, &temp );
	m_isInited = temp? true : false;
	GetU32( SSAPI_STORAGE_ELEMENT_ARRAY_FID_SERIAL_NUMBER, &m_serialNumber );
	GetGenericValue( (char *)&m_hostSparePoolId, sizeof(m_hostSparePoolId), SSAPI_STORAGE_ELEMENT_ARRAY_FID_HOST_SPAREPOOL_ID ); 
	GetInt64( SSAPI_STORAGE_ELEMENT_ARRAY_FID_CREATION_TIME, &m_creationTime );

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
StorageElementArray::IsYourElement( StorageElementBase &element, StorageManager *pManager ){

	RowId		rid;

	if( pManager->GetMemberIdBySRCId( element.GetDesignatorId().GetRowId(), rid ) )
		if( IsAMemberOfThisArray( rid, m_members, m_numberOfMembers ) )
			return true;

	if( pManager->GetSpareIdBySRCId( element.GetDesignatorId().GetRowId(), rid ) )
		if( IsAMemberOfThisArray( rid, m_spares, m_numberOfSpares ) )
			return true;

	return false;
}


//************************************************************************
// IsYourDedicatedSpare:
//
// PURPOSE:		Checks if the element id a dedicated spare of this array
//************************************************************************

bool 
StorageElementArray::IsYourDedicatedSpare( StorageElement &element, StorageManager *pManager ){
	
	RowId		rid;

	if( pManager->GetSpareIdBySRCId( element.GetDesignatorId().GetRowId(), rid ) ){
		if( IsAMemberOfThisArray( rid, m_spares, m_numberOfSpares ) )
			return true;
		else
			return false;
	}

	return false;
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates data members based on the row information.
//				Marks spares and sets members' state
//************************************************************************

void
StorageElementArray:: BuildYourselfFromPtsRow(	RAID_ARRAY_DESCRIPTOR	*pRow,
												ObjectManager			*pManager ){

	m_memberCapacity	= pRow->memberCapacity * BLOCK_SIZE;
	m_dataBlockSize		= pRow->dataBlockSize * BLOCK_SIZE;
	m_parityBlockSize	= pRow->parityBlockSize * BLOCK_SIZE;
	m_peckingOrder		= pRow->peckingOrder;
	m_isInited			= pRow->initStatus == RAID_INIT_COMPLETE? true : false;
	m_serialNumber		= pRow->serialNumber;
	m_creationTime		= pRow->creationDate;
	m_hostSparePoolId	= DesignatorId( RowId(pRow->hostForSparePool), SSAPI_OBJECT_CLASS_TYPE_HOST );	
	m_numberOfUtilsRunning = pRow->numberUtilities;
	m_numberOfMembers	= pRow->numberMembers;
	m_numberOfSpares	= pRow->numberSpares;
	memcpy( &m_members, &pRow->members, sizeof(rowID) * MAX_ARRAY_MEMBERS );
	memcpy( &m_spares, &pRow->spares, sizeof(rowID) * MAX_ARRAY_SPARES );
	m_status			= pRow->health;
	m_arrayRid			= pRow->thisRID;
}


//******************************************************************************
// SetYourState:
//
// PURPOSE:		Requires that super classes set their state and state string
//******************************************************************************

void StorageElementArray::SetYourState(){

	switch( m_status ){
		case RAID_OFFLINE:
			m_state		= SSAPI_OBJECT_STATE_DEAD;
			m_stateString = CTS_SSAPI_ARRAY_STATE_OFFLINE;
			break;

		case RAID_FAULT_TOLERANT:
			m_state		= SSAPI_OBJECT_STATE_GOOD;
			m_stateString = CTS_SSAPI_ARRAY_STATE_FAULT_TALERANT;
			break;

		case RAID_OKAY:
			m_state		= SSAPI_OBJECT_STATE_GOOD;
			m_stateString = CTS_SSAPI_ARRAY_STATE_ONLINE;
			break;

		case RAID_VULNERABLE:
		case RAID_CRITICAL:
			m_state		= SSAPI_OBJECT_STATE_WARNING;
			m_stateString = CTS_SSAPI_ARRAY_STATE_CRITICAL;
			break;
		
		default:
			m_state		= SSAPI_OBJECT_STATE_UNKNOWN;
			m_stateString = CTS_SSAPI_OBJECT_STATE_NAME_UNKNOWN;
			break;
	}
}


//************************************************************************
// IsAMemberOfThisArray:
//
// PURPOSE:		Checks if a 'rid' is in the array 'rids'
//************************************************************************

bool 
StorageElementArray::IsAMemberOfThisArray( RowId rid, RowId rids[], U32 size ){

	for( U32 i = 0; i < size; i++ ){
		if( rids[i] == rid )
			return true;
	}

	return false;
}


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

bool 
StorageElementArray::DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	((StorageManager *)GetManager())->DeleteArray( this, pResponder );
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
StorageElementArray::ReportYourUnderlyingDevices( Container &devices ){

	SList			tempContainer;
	StorageElement	*pElement;
	U32				index;
	DesignatorId	*pId;

	devices.RemoveAll();

	for( index = 0 ; index < GetChildCount(); index++ ){

		// request a list for every child
		pElement = (StorageElement *)GetChild( index );
		pElement->ReportYourUnderlyingDevices( tempContainer );

		// move ids into the main vector
		while( tempContainer.Count() ){
			tempContainer.GetAt( (CONTAINER_ELEMENT &)pId, 0 );
			tempContainer.RemoveAt( 0 );
			devices.Add( (CONTAINER_ELEMENT) pId );
		}
	}
}
