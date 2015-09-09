//************************************************************************
// FILE:		TableMetaData.cpp
//
// PURPOSE:		Implements the object that will contain the enumeration of
//				all PTS tables.
//************************************************************************


#include "TableMetaData.h"



//************************************************************************
// TableMetaData:
//************************************************************************

TableMetaData::TableMetaData( ListenManager *pListenManager )
:ManagedObject( pListenManager, SSAPI_OBJECT_CLASS_TYPE_TABLE_META_DATA ){

	m_id = DesignatorId( RowId(), GetClassType() );
}



//************************************************************************
// ~TableMetaData:
//************************************************************************

TableMetaData::~TableMetaData(){

	ClearInfoCells();
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
TableMetaData::BuildYourValueSet(){

	ValueSet			*pVs = new ValueSet();
	U32					index;
	TableInfo			*pCell;

	ManagedObject::BuildYourValueSet();

	for( index = 0; index < m_infoCells.Count(); index++) {
		m_infoCells.GetAt( (CONTAINER_ELEMENT &)pCell, index );
		pVs->AddValue( pCell, index );
	}
	AddValue( pVs, SSAPI_TABLE_META_DATA_FID_TABLE_INFO_VECTOR );
	delete pVs;
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
TableMetaData::BuildYourselfFromYourValueSet(){				

	ManagedObject::BuildYourselfFromYourValueSet();

	// This method is not implemented 'cause it is not needed.
	ASSERT( 0 );

	return true;
}



//************************************************************************
// AddTableInfoCell:
//************************************************************************

void 
TableMetaData::AddTableInfoCell( rowDef *pRow, bool postEvent ){
	
	m_infoCells.AddAt( (CONTAINER_ELEMENT)new TableInfo(	pRow->tableName,
															DesignatorId(pRow->tableId, SSAPI_OBJECT_CLASS_TYPE_PTS_TABLE),
															pRow->rowId ),
						m_infoCells.Count() );
														
	if( postEvent )
		FireEventObjectModifed();
}


//************************************************************************
// RemoveTableInfoCell:
//************************************************************************

void 
TableMetaData::RemoveTableInfoCell( rowDef *pRow, bool postEvent ){

	U32				index;
	TableInfo		*pCell;
	bool			found;

	for( index = 0, found = false; index < m_infoCells.Count(); index++){
		m_infoCells.GetAt( (CONTAINER_ELEMENT &)pCell, index );
		if( pCell->GetRid() == pRow->rowId ){
			found = true;
			m_infoCells.RemoveAt( index );
			delete pCell;
			if( postEvent )
				FireEventObjectModifed();
			break;
		}
	}

	ASSERT( found );
}


void 
TableMetaData::RemoveTableInfoCell( RowId &rid, bool postEvent ){

	U32				index;
	TableInfo		*pCell;
	bool			found;

	for( index = 0, found = false; index < m_infoCells.Count(); index++){
		m_infoCells.GetAt( (CONTAINER_ELEMENT &)pCell, index );
		if( pCell->GetRid() == rid ){
			found = true;
			m_infoCells.RemoveAt( index );
			delete pCell;
			if( postEvent )
				FireEventObjectModifed();
			break;
		}
	}

	ASSERT( found );
}


TableInfo* 
TableMetaData::GetTableAt( U32 index ){

	bool			found;
	TableInfo		*pCell;

	found = m_infoCells.GetAt( (CONTAINER_ELEMENT &)pCell, index )? true : false;
	ASSERT( found );

	return pCell;
}

//************************************************************************
// ClearInfoCells:		
//					
// PURPOSE:			Memory clean-up.
//************************************************************************

void 
TableMetaData::ClearInfoCells(){

	TableInfo		*pCell;

	while( m_infoCells.Count() ){
		m_infoCells.GetAt( (CONTAINER_ELEMENT &)pCell, 0 );
		m_infoCells.RemoveAt( 0 );
		delete pCell;
	}
}


//************************************************************************
//
//************************************************************************

void 
TableMetaData::GetTableIdByTableName( StringClass name, RowId &rid ){
	
	TableInfo		*pCell;
	U32				i;

	for( i = 0; i < m_infoCells.Count(); i++ ){
		m_infoCells.GetAt( (CONTAINER_ELEMENT &)pCell, i );
		if( pCell->m_tableName == name ){
			rid =  pCell->m_tableRid;
			return;
		}
	}

	ASSERT(0);
}
