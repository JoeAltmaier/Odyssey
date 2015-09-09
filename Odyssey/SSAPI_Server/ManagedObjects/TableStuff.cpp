//************************************************************************
// FILE:		TableStuff.cpp
//
// PURPOSE:		Defines the object that represents a PTS table in the 
//				O2K system.
//************************************************************************


#include "TableStuff.h"
#include "ShadowTable.h"
#include "TableStuffManager.h"
#include "TableMetaData.h"


//************************************************************************
//
//************************************************************************

TableStuff::TableStuff( ListenManager *pLM, ShadowTable *pST, DesignatorId id )
:ManagedObject( pLM, SSAPI_OBJECT_CLASS_TYPE_PTS_TABLE ){

	m_id	= id;
	pShadowTable = pST;
}


//************************************************************************
// 
//************************************************************************

TableStuff::~TableStuff(){

	delete pShadowTable;
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
TableStuff::BuildYourValueSet(){

	fieldDef		*pField = pShadowTable->GetFieldDefArray();
	U32				index;
	ValueSet		*pFieldsVs = new ValueSet();
	CoolVector		container;
	UnicodeString	s;
	DesignatorId	id;
	ManagedObject	*pObj;
	RowId			rid;
	
	((TableStuffManager *)GetManager())->GetTableMetaData()->GetTableIdByTableName( pShadowTable->GetName(), rid );
	((TableStuffManager *)GetManager())->StuffContainerWithRowObjectsForTable(	rid.GetTable(),
																				container );
	for( index = 0; index < container.Count(); index++ ){
		container.GetAt( (CONTAINER_ELEMENT &)pObj, index );
		AddChildId( pObj, false );
		pObj->AddParentId( this, false );
	}

	ManagedObject::BuildYourValueSet();
	
 	for( index = 0; index < pShadowTable->GetNumberOfCols(); index++, pField++ ){
		s = StringClass( pField->name );
		pFieldsVs->AddString( &s, index );
	}
	AddValue( pFieldsVs, SSAPI_PTS_TABLE_FID_FIELD_DEF_VECTOR );

	delete pFieldsVs;
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
TableStuff::BuildYourselfFromYourValueSet(){

	ManagedObject::BuildYourselfFromYourValueSet();

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
TableStuff::DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder ){
	
	TableStuffManager *pM = (TableStuffManager *)GetManager();

	pM->DeleteTableStuff( (TableStuff &)objectValues, pResponder );
	return true;
}
