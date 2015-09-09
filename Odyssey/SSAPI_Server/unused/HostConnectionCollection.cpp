//************************************************************************
// FILE:		HostConnectionCollection.cpp
//
// PURPOSE:		Implements class HostConnectionCollection that will serve
//				as an abstract base class for all host connection collection
//				classes used in the system. Provided to enforce the 
//				policy of how a LUN is exported (to a COLLECTION of host
//				connections!)
//************************************************************************


#include "HostConnectionCollection.h"
#include "DdmSsapi.h"
#include "HostManager.h"


//************************************************************************
// BuildYourselfFropmPtsRow:
//
// PURPOSE:		Populates data members based on the PTS row
//************************************************************************

void 
HostConnectionCollection::BuildYourselfFromPtsRow( HostCollectionDescriptorRecord *pRow ){

	m_isActive			= pRow->isActive? true :  false;
	m_pointerTableName	= StringClass( pRow->collectionTableName );
	m_description		= UnicodeString( pRow->description );
	m_lunId				= DesignatorId( RowId( pRow->ridLUN ), SSAPI_OBJECT_CLASS_TYPE_LUN_MAP_ENTRY );
	m_name				= UnicodeString( pRow->name );
}


//************************************************************************
// ~HostConnectionCollection:
//************************************************************************

HostConnectionCollection::~HostConnectionCollection(){

	CHILD_INFO_CELL		*pCell;

	while( childInfo.Count() ){
		childInfo.GetAt( (CONTAINER_ELEMENT &)pCell, 0 );
		childInfo.RemoveAt( 0 );
		delete pCell;
	}

}


//************************************************************************
// WriteYourselfIntoPtsRow:
//
// PURPOSE:		Populates pts row based on the data members
//************************************************************************

void 
HostConnectionCollection::WriteYourselfIntoPtsRow( HostCollectionDescriptorRecord *pRow ){

	memset( pRow, 0, sizeof(HostCollectionDescriptorRecord) );

	pRow->size				= sizeof(HostCollectionDescriptorRecord);
	pRow->version			= HOST_COLLECTION_DESCRIPTOR_TABLE_VERSION;
	pRow->isActive			= m_isActive? 1 : 0;
	m_pointerTableName.CString( pRow->collectionTableName, sizeof( pRow->collectionTableName ) );
	m_description.CString( pRow->description, sizeof( pRow->description ) );
	pRow->ridLUN			= m_lunId.GetRowId();

	m_name.CString( pRow->name, sizeof(pRow->name) );
}


//************************************************************************
// BuildYourselfFromRowIdCollectionRows:
//
// PURPOSE:		Builds its children ids from the table
//************************************************************************

void 
HostConnectionCollection::BuildYourselfFromRowIdCollectionRows( RowIdCollectionRecord *pRows, U32 rowCount ){
	
	DesignatorId	*pId;
	HostManager		*pManager = (HostManager *)((DdmSSAPI *)pParentDdmSvs)->GetObjectManager( SSAPI_MANAGER_CLASS_TYPE_HOST_MANAGER );	

	for( U32 i = 0; i < rowCount; i++, pRows++ ){
		pId = new DesignatorId( pRows->ridPointer, SSAPI_OBJECT_CLASS_TYPE_HOST_CONNECTION );
		AddAChild( pManager->GetManagedObject( pId ), pRows->rid );
	}
}


//************************************************************************
// WriteYourselfIntoRowIdCollectionRow:
//
// PURPOSE:		Writes its children ids into the table
//************************************************************************

void 
HostConnectionCollection::WriteYourselfIntoRowIdCollectionRow( RowIdCollectionRecord *pRows, U32 rowCount ){

	DesignatorId			id;

	for( U32 i = 0; i < rowCount; i++, pRows++ ){
		id = GetChildId( i );
		pRows->version		= ROWID_COLLECTION_TABLE_VERSION;
		pRows->size			= sizeof(RowIdCollectionRecord);
		pRows->ridPointer	= id.GetRowId().GetRowID();
	}
}


//************************************************************************
// DeleteAChild:
//
// PURPOSE:		Deletes a child id
//************************************************************************

void 
HostConnectionCollection::DeleteAChild( ManagedObject *pObj ){
	
	CHILD_INFO_CELL			*pCell;

	for( U32 i = 0; i < childInfo.Count(); i++ ){
		childInfo.GetAt( (CONTAINER_ELEMENT &)pCell, i );
		if( pCell->childId == pObj->GetDesignatorId() ){
			delete pCell;
			childInfo.RemoveAt( i );
			DeleteChildId( pObj );
		}
	}

	ASSERT(0);
}


//************************************************************************
// AddAChild:
//
// PURPOSE:		Adds a connection id
//************************************************************************

void 
HostConnectionCollection::AddAChild( ManagedObject *pObj, RowId rowId ){
	
	CHILD_INFO_CELL					*pCell = new CHILD_INFO_CELL( pObj->GetDesignatorId(), rowId );
	
	childInfo.Add( (CONTAINER_ELEMENT)pCell );

	AddChildId( pObj );
}



//************************************************************************
// GetRowIdIdOfChild:
//
// PURPOSE:		Performs a lookup of row id by child id
//************************************************************************

RowId 
HostConnectionCollection::GetRowIdIdOfChild( DesignatorId childId ){

	CHILD_INFO_CELL		*pCell;
	U32					i;

	for( i = 0; i < childInfo.Count(); i++ ){
		childInfo.GetAt( (CONTAINER_ELEMENT &)pCell, i );
		if( pCell->childId == childId )
			return pCell->rowId;
	}

	ASSERT(0);

	return RowId();
}


//************************************************************************
// GetChildId:
//
// PURPOSE:		Returns child id at the colcation requested
//************************************************************************

DesignatorId 
HostConnectionCollection::GetChildId( U32 position ){

	DesignatorId			*pId;

	m_children.GetAt( (CONTAINER_ELEMENT &)pId, position );

	return *pId;
}


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Responsible for state reconcilation
//************************************************************************

void 
HostConnectionCollection::ComposeYourOverallState(){

	int						state, savedState = m_state;
	HostConnectionElement	*pElement;
	
	if( StatusReporterInterface::CanChangeStateToGood() ){
		m_state = SSAPI_OBJECT_STATE_GOOD;
		m_stateString = CTS_SSAPI_OBJECT_STATE_NAME_GOOD;

		for( U32 i = 0; i < m_children.Count(); i++ ){
			pElement	= (HostConnectionElement *)GetChild( i );
			if( (state = pElement->GetState() ) > m_state ){
				m_stateString = pElement->GetStateString();
				m_state = state;
			}
		}

		if( m_state != savedState )
			FireEventObjectModifed();
	}
}

//************************************************************************
// AreThereAnyTooLongStrings
//
// PURPOSE:		Checks every string's size against the maximum in the PTS 
//				row. Will raise an exception if something's wrong.
//				Checks strings in the value set, not the members. 
//
// RETURN:		true:	all strings are OK, may proceed
//				false:	an exception was rased, terminate normal execution
//************************************************************************

bool 
HostConnectionCollection::AreThereAnyTooLongStrings( SsapiResponder *pResponder ){

	HostCollectionDescriptorRecord		row;

	BuildYourselfFromYourValueSet();

	if( m_name.GetSize() > sizeof( row.name ) ){
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_EXCEPTION_NAME_TOO_LONG);
		return false;
	}

	if( m_description.GetSize() > sizeof( row.description ) ){
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_EXCEPTION_DESCRIPTION_TOO_LONG );
		return false;
	}

	row.size = 1;
	return true;
}