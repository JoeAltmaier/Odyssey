//************************************************************************
// FILE:		StorageElementArray1.h
//
// PURPOSE:		Defines the object that will be used to represent RAID1
//				arays inside the O2K system.
//************************************************************************

#ifndef __STORAGE_ELEMENT_ARRAY_1_H__
#define	__STORAGE_ELEMENT_ARRAY_1_H__

#include "StorageElementArray.h"

#ifdef WIN32
#pragma pack(4)
#endif

class StorageElementArray1 : public StorageElementArray {

protected:
	DesignatorId			m_preferredMemberId;
	DesignatorId			m_sourceMemberId;

public:

//************************************************************************
// StorageElementArray:
//
// PURPOSE:		Default constructor
//************************************************************************

StorageElementArray1( ListenManager *pListenManager, ObjectManager *pManager, U32 classType = SSAPI_OBJECT_CLASS_TYPE_ARRAY_1_STORAGE_ELEMENT )
:StorageElementArray( pListenManager,  classType, pManager ){
}


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new StorageElementArray1( GetListenManager(), GetManager() ); }


//************************************************************************
// ~StorageElementArray:
//
// PURPOSE:		Default destructor
//************************************************************************

virtual ~StorageElementArray1(){}


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

virtual bool BuildYourValueSet();


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

virtual bool BuildYourselfFromYourValueSet();



//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates data members based on the row information.
//				Marks spares and sets members' state
//************************************************************************

virtual void BuildYourselfFromPtsRow(	RAID_ARRAY_DESCRIPTOR	*pRow,
										ObjectManager			*pManager );


//************************************************************************
// WriteYourselfIntoCreateArrayDefinition:
//
// PURPOSE:		Writes the array members into the array definition struct
//				for the RAID master
//************************************************************************

virtual bool WriteYourselfIntoCreateArrayDefinition(ValueSet &values, 
													RMSTR_CREATE_ARRAY_DEFINITION *pDef,
													U32 hotCopyPirority  =0,
													ManagedObject	*pHotCopyExportIndex  =0);

//******************************************************************************
// IsExportable:
//
// PURPOSE:		Gives every possible storage element type a chance to voice
//				its abilities
//******************************************************************************

virtual bool IsExportable() { return !m_isUsed; }

};

#endif // __STORAGE_ELEMENT_ARRAY_1_H__
