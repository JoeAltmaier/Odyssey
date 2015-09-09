//******************************************************************************
// FILE:		StorageElementArray1HotCopyAuto.h
//
// PURPOSE:		Defines the object to represent auto - killed hot copy
//******************************************************************************

#ifndef __HOT_COPY_AUTO_H__
#define	__HOT_COPY_AUTO_H__

#include "StorageElementArray1HotCopy.h"

#ifdef WIN32
#pragma pack(4)
#endif

class StorageElementArray1HotCopyAuto : public StorageElementArray1HotCopy{


public:

//******************************************************************************
// StorageElementArray1HotCopyAuto:
//
// PURPOSE:		Default constructor
//******************************************************************************

StorageElementArray1HotCopyAuto( ListenManager *pListenManager, ObjectManager *pManager )
:StorageElementArray1HotCopy( pListenManager, SSAPI_OBJECT_CLASS_TYPE_ARRAY_HOT_COPY_AUTO, pManager ) {}


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new StorageElementArray1HotCopyAuto( GetListenManager(), GetManager() ); }


//******************************************************************************
// ~StorageElementArray1HotCopyAuto:
//
// PURPOSE:		The destructor
//******************************************************************************

virtual ~StorageElementArray1HotCopyAuto() {}


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

	bool					rc, found;
	U32						i;
	
	rc = StorageElementArray1HotCopy::WriteYourselfIntoCreateArrayDefinition( values, pDef, hotCopyPirority, pHotCopyExportIndex  );
	
	pDef->createPolicy.StartHotCopyWithAutoBreak = 1;
	
	// find the index of the element to be exported once the operation's done
	for( i = 0, found = false; i < pDef->numberMembers; i++ )
		if( pHotCopyExportIndex->GetDesignatorId().GetRowId() == pDef->arrayMembers[i] ){
			pDef->hotCopyExportMemberIndex = i;
			found = true;
			break;
		}

	return rc && found;
}


};

#endif // __HOT_COPY_AUTO_H__