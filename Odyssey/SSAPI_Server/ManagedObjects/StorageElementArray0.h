//************************************************************************
// FILE:		StorageElementArray0.h
//
// PURPOSE:		Defines the object that will be used to represent RAID0
//				arays inside the O2K system.
//************************************************************************

#ifndef __STORAGE_ELEMENT_ARRAY_0_H__
#define	__STORAGE_ELEMENT_ARRAY_0_H__

#include "StorageElementArray.h"

#ifdef WIN32
#pragma pack(4)
#endif

class StorageElementArray0 : public StorageElementArray {


public:

//************************************************************************
// StorageElementArray:
//
// PURPOSE:		Default constructor
//************************************************************************

StorageElementArray0( ListenManager *pListenManager, ObjectManager *pManager )
:StorageElementArray( pListenManager, SSAPI_OBJECT_CLASS_TYPE_ARRAY_0_STORAGE_ELEMENT, pManager ){
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

virtual ManagedObject* CreateInstance(){ return new StorageElementArray0( GetListenManager(), GetManager() ); }


//************************************************************************
// ~StorageElementArray:
//
// PURPOSE:		Default destructor
//************************************************************************

virtual ~StorageElementArray0(){}


//************************************************************************
// WriteYourselfIntoCreateArrayDefinition:
//
// PURPOSE:		Writes the array members into the array definition struct
//				for the RAID master
//************************************************************************

virtual bool WriteYourselfIntoCreateArrayDefinition(ValueSet &values, 
													RMSTR_CREATE_ARRAY_DEFINITION *pDef,
													U32 hotCopyPirority  =0,
													ManagedObject	*pHotCopyExportIndex = 0);

//******************************************************************************
// IsExportable:
//
// PURPOSE:		Gives every possible storage element type a chance to voice
//				its abilities
//******************************************************************************

virtual bool IsExportable() { return true; }

};

#endif // __STORAGE_ELEMENT_ARRAY_0_H__
