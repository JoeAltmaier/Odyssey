//******************************************************************************
// FILE:		StorageElementArray1HotCopy.h
//
// PURPOSE:		Defines the object used to reprsent the RAID1 array that 
//				performs a HotCopy operation
//******************************************************************************


#ifndef __STORAGE_ELEMENT_HOT_COPY_H__
#define __STORAGE_ELEMENT_HOT_COPY_H__

#include "StorageElementArray1.h"

#ifdef WIN32
#pragma pack(4)
#endif


class StorageElementArray1HotCopy : public StorageElementArray1 {

protected:


//******************************************************************************
// StorageElementArray1HotCopy:
//
// PURPOSE:		Default constructor
//******************************************************************************

StorageElementArray1HotCopy( ListenManager *pListenManager, U32 classType, ObjectManager *pManager )
:StorageElementArray1( pListenManager, pManager, classType ){}


public:


//******************************************************************************
// ~StorageElementArray1HotCopy:
//
// PURPOSE:		Default destructor
//******************************************************************************

virtual ~StorageElementArray1HotCopy(){}


//************************************************************************
// WriteYourselfIntoCreateArrayDefinition:
//
// PURPOSE:		Writes the array members into the array definition struct
//				for the RAID master
//************************************************************************

virtual bool WriteYourselfIntoCreateArrayDefinition(ValueSet &values, 
													RMSTR_CREATE_ARRAY_DEFINITION *pDef,
													U32				hotCopyPirority  = 0,
													ManagedObject	*pHotCopyExportIndex  = 0){

	ValueSet		*pSelf, *pMembers;
	U32				memberIndex, i;
	DesignatorId	id;

	pSelf		= (ValueSet *)values.GetValue(SSAPI_STORAGE_MANAGER_ADD_ARRAY_ARRAY_OBJECT);
	pMembers	= (ValueSet *)values.GetValue(SSAPI_STORAGE_MANAGER_ADD_ARRAY_MEMBER_ID_VECTOR);

	*((ValueSet *)this) = *pSelf;
	BuildYourselfFromYourValueSet();
	memset( pDef, 0, sizeof(RMSTR_CREATE_ARRAY_DEFINITION) );

	pDef->raidLevel		= RAID1;
	pDef->totalCapacity	= ( m_memberCapacity * (pMembers->GetCount() / 2) ) / BLOCK_SIZE;
	pDef->memberCapacity= this->m_memberCapacity / BLOCK_SIZE;
	pDef->peckingOrder	= (RAID_PECKING_ORDER)m_peckingOrder;
	pDef->numberMembers	= pMembers->GetCount();
	pDef->numberSpares	= 0;


	for( i = memberIndex = 0; i < pMembers->GetCount(); i++, memberIndex++ ){
		pMembers->GetGenericValue( (char *)&id, sizeof(id), i );
		pDef->arrayMembers[memberIndex] = id.GetRowId().GetRowID();

		if( id == m_preferredMemberId )
			pDef->preferredMemberIndex = i;

		if( id == m_sourceMemberId )
			pDef->sourceMemberIndex = i;
	}

	pDef->hotCopyPriority = (RAID_UTIL_PRIORITY)hotCopyPirority;
	pDef->peckingOrder	= NEVER_PECK;

	return true;
}

};

#endif // __STORAGE_ELEMENT_HOT_COPY_H__