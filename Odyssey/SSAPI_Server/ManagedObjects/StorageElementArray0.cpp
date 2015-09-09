//************************************************************************
// FILE:		StorageElementArray0.cpp
//
// PURPOSE:		Implements the object to be used to represent RAID0 arrays
//				in the O2K system
//************************************************************************

#include "StorageElementArray0.h"


//************************************************************************
// WriteYourselfIntoCreateArrayDefinition:
//
// PURPOSE:		Writes the array members into the array definition struct
//				for the RAID master
//************************************************************************

bool 
StorageElementArray0::WriteYourselfIntoCreateArrayDefinition(ValueSet &values, 
															 RMSTR_CREATE_ARRAY_DEFINITION *pDef,
															 U32 , 
															 ManagedObject	*  ){

	ValueSet		*pSelf, *pMembers, *pSpares;
	U32				memberIndex, i;
	DesignatorId	id;

	pSelf		= (ValueSet *)values.GetValue(SSAPI_STORAGE_MANAGER_ADD_ARRAY_ARRAY_OBJECT);
	pMembers	= (ValueSet *)values.GetValue(SSAPI_STORAGE_MANAGER_ADD_ARRAY_MEMBER_ID_VECTOR);
	pSpares		= (ValueSet *)values.GetValue(SSAPI_STORAGE_MANAGER_ADD_ARRAY_SPARE_ID_VECTOR);

	*((ValueSet *)this) = *pSelf;
	BuildYourselfFromYourValueSet();
	memset( pDef, 0, sizeof(RMSTR_CREATE_ARRAY_DEFINITION) );

	pDef->raidLevel		= RAID0;
	pDef->totalCapacity	= ( m_memberCapacity * pMembers->GetCount() ) / BLOCK_SIZE;
	pDef->memberCapacity= m_memberCapacity / BLOCK_SIZE;
	pDef->peckingOrder	= (RAID_PECKING_ORDER)m_peckingOrder;
	pDef->numberMembers	= pMembers->GetCount();
	pDef->numberSpares	= pSpares? pSpares->GetCount() : 0;
	pDef->dataBlockSize	= m_dataBlockSize / BLOCK_SIZE;

	for( i = memberIndex = 0; i < pMembers->GetCount(); i++, memberIndex++ ){
		pMembers->GetGenericValue( (char *)&id, sizeof(id), i );
		pDef->arrayMembers[memberIndex] = id.GetRowId().GetRowID();
	}

	for( i = 0; pSpares && (i < pSpares->GetCount()); i++, memberIndex++ ){
		pSpares->GetGenericValue( (char *)&id, sizeof(id), i );
		pDef->arrayMembers[memberIndex] = id.GetRowId().GetRowID();
	}

	return true;
}