//************************************************************************
// FILE:		TableStuffRow.cpp
//
// PURPOSE:		Implements that object whose instances will be used to
//				represent PTS table rows.
//************************************************************************

#include "TableStuffRow.h"
#include "ShadowTable.h"
#include "SsapiAssert.h"
#include "TableStuffManager.h"


extern U32   acbDataTypeSize [] ;

//************************************************************************
//
//************************************************************************

TableStuffRow::TableStuffRow(	ListenManager *pLM, 
								ObjectManager *pManager,
								ShadowTable	  *pTable,
								void		  *pData,
								U32			  dataSize )

:ManagedObject( pLM, SSAPI_OBJECT_CLASS_TYPE_PTS_TABLE_ROW, pManager ){

	RowId			rid;
	DesignatorId	id;
	
	ASSERT( pManager );
	ASSERT( pTable );
	
	m_pTable	= pTable;
	m_pRowData	= new char[ m_rowDataSize = dataSize ];
	memcpy( m_pRowData, pData, dataSize );

	memcpy( &rid, pData, sizeof(rid) );
	id = DesignatorId( rid, SSAPI_OBJECT_CLASS_TYPE_PTS_TABLE_ROW );
	memcpy( &m_id, &id, sizeof( id ) );
}


//************************************************************************
//
//************************************************************************

TableStuffRow::~TableStuffRow(){

	delete [] m_pRowData;
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
TableStuffRow::BuildYourValueSet(){

	char			*pCurr = (char *)m_pRowData;
	U32				i, j;
	fieldDef		*pField = m_pTable->GetFieldDefArray();
	ValueSet		*pVs = new ValueSet(), *pArray;
	UnicodeString	s;
		

	ManagedObject::BuildYourValueSet();

	for( i = 0; i < m_pTable->GetNumberOfCols(); i++, pField++  ){

		// see if it's an array
		if( (pField->iFieldType!=BINARY_FT) && (acbDataTypeSize[ pField->iFieldType ] != pField->cbField) ){
			// add each value into special VS
			pArray = new ValueSet();
			for( j=0; j < pField->cbField/acbDataTypeSize[ pField->iFieldType ]; j++ )
				PutValue( pField, pArray, j, pCurr );


			pVs->AddValue( pArray, i );
			delete pArray;
		}
		else{
			PutValue( pField, pVs, i, pCurr );
		}

	}
	AddValue( pVs, SSAPI_TABLE_ROW_FID_VALUE_VECTOR );
	delete pVs;
	return true;
}


//************************************************************************
// PutValue:		Based on the value type, adds it properly into the
//					value set provided
//************************************************************************

void 
TableStuffRow::PutValue( fieldDef *pField, ValueSet *pVs, U32 i, char* &pCurr ){

	UnicodeString	s;

	switch( pField->iFieldType ){
			case BINARY_FT:
				pVs->AddGenericValue( pCurr, pField->cbField, i );
				pCurr += pField->cbField;
				break;

			case S32_FT:
			case DID_FT:
				pVs->AddInt( *((int *)pCurr), i );
				pCurr += sizeof(S32);
				break;

			case U32_FT:
				pVs->AddU32( *((U32 *)pCurr), i );
				pCurr += sizeof(U32);
				break;

			case S64_FT:
				pVs->AddInt64( *((I64 *)pCurr), i );
				pCurr += sizeof(I64);
				break;

			case U64_FT:
				pVs->AddU64( *((U64 *)pCurr), i );
				pCurr += sizeof(U64);
				break;

			case STRING16_FT:
				*(pCurr + 15) = 0;			// make sure it's z-terminated
				s = StringClass( pCurr );
				pVs->AddString( &s, i );
				pCurr += sizeof(String16);
				break;

			case STRING32_FT:
				*(pCurr + 31) = 0;			// make sure it's z-terminated
				s = StringClass( pCurr );
				pVs->AddString( &s, i );
				pCurr += sizeof(String32);
				break;

			case STRING64_FT:
				*(pCurr + 63) = 0;			// make sure it's z-terminated
				s = StringClass( pCurr );
				pVs->AddString( &s, i );
				pCurr += sizeof(String64);
				break;	


			case ROWID_FT:
				pVs->AddRowID( pCurr, i );
				pCurr += sizeof(rowID);
				break;

			case BOOL_FT:
				s = *((int *)pCurr)? "true" : "false";
				pVs->AddString( &s, i );
				pCurr += sizeof(U32);
				break;

			case VDN_FT:
				pVs->AddInt( *((int *)pCurr), i );
				pCurr += sizeof(int);
				break;

			case USTRING16_FT:
				s = UnicodeString( (void *)pCurr );
				pVs->AddString( &s, i );
				pCurr += 32;
				break;

			case USTRING32_FT:
				s = UnicodeString( (void *)pCurr );
				pVs->AddString( &s, i );
				pCurr += 64;
				break;

			case USTRING64_FT:
				s = UnicodeString( (void *)pCurr );
				pVs->AddString( &s, i );
				pCurr += 128;
				break;

			case USTRING128_FT:
				s = UnicodeString( (void *)pCurr );
				pVs->AddString( &s, i );
				pCurr += 256;
				break;

			case USTRING256_FT:
				s = UnicodeString( (void *)pCurr );
				pVs->AddString( &s, i );
				pCurr += 512;
				break;

			default:
				ASSERT(0);
				break;
		}
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
TableStuffRow::BuildYourselfFromYourValueSet(){

	ValueSet		*pVs;
	char			*pNewRow, *pCurr;
	U32				i;
	fieldDef		*pField;
	Value			*pValue;

	ManagedObject::BuildYourselfFromYourValueSet();

	pVs = (ValueSet *)GetValue(SSAPI_TABLE_ROW_FID_VALUE_VECTOR);
	if( pVs ){

		pNewRow = new(tZERO) char[ m_rowDataSize ];
		
		// populate the buffer from the value set
		pCurr = pNewRow;
		for( i = 0, pField = m_pTable->GetFieldDefArray(); i < m_pTable->GetNumberOfCols(); i++, pField++ ){
			pValue = pVs->GetValue( i );
			ASSERT(0);
/*
			switch( pValue->m_iType ){
				case SSAPI_TYPE_STRING:
				SSAPI_TYPE_INT 			= 1,
				SSAPI_TYPE_FLOAT 		= 2,
				SSAPI_TYPE_VECTOR 		= 3,
				SSAPI_TYPE_BOOLEAN		= 4,
				SSAPI_TYPE_GENERIC 		= 5,
				SSAPI_TYPE_INT_64 		= 6,
				SSAPI_TYPE_U8			= 7,
				SSAPI_TYPE_U16			= 8,
				SSAPI_TYPE_U64			= 9,
				SSAPI_TYPE_INT8			= 10,
				SSAPI_TYPE_INT16		= 11,
				SSAPI_TYPE_U32
			}
*/
		}
		delete m_pRowData;
		m_pRowData = pNewRow;
	}

	return true;
}


//************************************************************************
// ModifyObject:
//
// PURPOSE:		Modifes contents of the object
//
// NOTE:		Must be overridden by objects that can be modified
//************************************************************************

bool
TableStuffRow::ModifyObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	TableStuffManager *pM = (TableStuffManager *)GetManager();

	pM->ModifyTableRow( (TableStuffRow &)objectValues, pResponder );

	return true;
}


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

bool 
TableStuffRow::DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	TableStuffManager *pM = (TableStuffManager *)GetManager();

	pM->DeleteTableRow( (TableStuffRow &)objectValues, pResponder );
	return true;
}

