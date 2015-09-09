//************************************************************************
// FILE:		StorageElementArray1.cpp
//
// PURPOSE:		Implements the object used to reprsent RAID 1 arrays in
//				the O2K system.
//************************************************************************

#include "StorageElementArray1.h"
#include "StorageManager.h"

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
StorageElementArray1::BuildYourValueSet(){
	
	StorageElementArray::BuildYourValueSet();

	AddGenericValue( (char *)&m_preferredMemberId, sizeof(m_preferredMemberId), SSAPI_STORAGE_ELEMENT_ARRAY1_FID_PREF_MEMBER_ID );
	AddGenericValue( (char *)&m_sourceMemberId, sizeof(m_sourceMemberId), SSAPI_STORAGE_ELEMENT_ARRAY1_FID_SOURCE_MEMBER_ID );

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
StorageElementArray1::BuildYourselfFromYourValueSet(){

	StorageElementArray::BuildYourselfFromYourValueSet();

	GetGenericValue( (char *)&m_preferredMemberId, sizeof(m_preferredMemberId), SSAPI_STORAGE_ELEMENT_ARRAY1_FID_PREF_MEMBER_ID );
	GetGenericValue( (char *)&m_sourceMemberId, sizeof(m_sourceMemberId), SSAPI_STORAGE_ELEMENT_ARRAY1_FID_SOURCE_MEMBER_ID );

	return true;
}



//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates data members based on the row information.
//				Marks spares and sets members' state
//************************************************************************

void 
StorageElementArray1::BuildYourselfFromPtsRow(	RAID_ARRAY_DESCRIPTOR	*pRow,
												ObjectManager			*pManager ){

	RowId		rid;

	StorageElementArray::BuildYourselfFromPtsRow( pRow, pManager );

	((StorageManager *)pManager)->GetSRCIdByMemberId( pRow->members[pRow->preferredMemberIndex], rid );
	pManager->GetDesignatorIdByRowId( rid, m_preferredMemberId );

	((StorageManager *)pManager)->GetSRCIdByMemberId( pRow->members[pRow->sourceMemberIndex], rid );
	pManager->GetDesignatorIdByRowId( rid, m_sourceMemberId );
}


//************************************************************************
// WriteYourselfIntoCreateArrayDefinition:
//
// PURPOSE:		Writes the array members into the array definition struct
//				for the RAID master
//************************************************************************

bool 
StorageElementArray1::WriteYourselfIntoCreateArrayDefinition( ValueSet &values, RMSTR_CREATE_ARRAY_DEFINITION *pDef, U32, ManagedObject	* ){

	ValueSet		*pSelf, *pMembers, *pSpares;
	U32				memberIndex, i;
	DesignatorId	id;

	pSelf		= (ValueSet *)values.GetValue(SSAPI_STORAGE_MANAGER_ADD_ARRAY_ARRAY_OBJECT);
	pMembers	= (ValueSet *)values.GetValue(SSAPI_STORAGE_MANAGER_ADD_ARRAY_MEMBER_ID_VECTOR);
	pSpares		= (ValueSet *)values.GetValue(SSAPI_STORAGE_MANAGER_ADD_ARRAY_SPARE_ID_VECTOR);

	*((ValueSet *)this) = *pSelf;
	BuildYourselfFromYourValueSet();
	memset( pDef, 0, sizeof(RMSTR_CREATE_ARRAY_DEFINITION) );

	pDef->raidLevel		= RAID1;
	pDef->totalCapacity	= ( m_memberCapacity * (pMembers->GetCount() / 2) ) / BLOCK_SIZE;
	pDef->memberCapacity= this->m_memberCapacity / BLOCK_SIZE;
	pDef->peckingOrder	= (RAID_PECKING_ORDER)m_peckingOrder;
	pDef->numberMembers	= pMembers->GetCount();
	pDef->numberSpares	= pSpares? pSpares->GetCount() : 0;


	for( i = memberIndex = 0; i < pMembers->GetCount(); i++, memberIndex++ ){
		pMembers->GetGenericValue( (char *)&id, sizeof(id), i );
		pDef->arrayMembers[memberIndex] = id.GetRowId().GetRowID();

		if( id == m_preferredMemberId )
			pDef->preferredMemberIndex = i;

		if( id == m_sourceMemberId )
			pDef->sourceMemberIndex = i;
	}

	for( i = 0; pSpares && (i < pSpares->GetCount()); i++, memberIndex++ ){
		pSpares->GetGenericValue( (char *)&id, sizeof(id), i );
		pDef->arrayMembers[memberIndex] = id.GetRowId().GetRowID();
	}

	return true;
}

