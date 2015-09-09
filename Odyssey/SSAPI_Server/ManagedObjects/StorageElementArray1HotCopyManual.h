//******************************************************************************
// FILE:		StorageElementArray1HotCopyManual.h
//
// PURPOSE:		Defines the object to represent manually - killed hot copy
//******************************************************************************

#ifndef __HOT_COPY_MANUAL_H__
#define	__HOT_COPY_MANUAL_H__

#include "StorageElementArray1HotCopy.h"

#ifdef WIN32
#pragma pack(4)
#endif

class StorageElementArray1HotCopyManual : public StorageElementArray1HotCopy{


public:

//******************************************************************************
// StorageElementArray1HotCopyManual:
//
// PURPOSE:		Default constructor
//******************************************************************************

StorageElementArray1HotCopyManual( ListenManager *pListenManager, ObjectManager *pManager )
:StorageElementArray1HotCopy( pListenManager, SSAPI_OBJECT_CLASS_TYPE_ARRAY_HOT_COPY_MANUAL, pManager ) {}


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new StorageElementArray1HotCopyManual( GetListenManager(), GetManager() ); }


//******************************************************************************
// ~StorageElementArray1HotCopyManual:
//
// PURPOSE:		The destructor
//******************************************************************************

virtual ~StorageElementArray1HotCopyManual() {}


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

	bool					rc;
	
	rc = StorageElementArray1HotCopy::WriteYourselfIntoCreateArrayDefinition( values, pDef, hotCopyPirority, pHotCopyExportIndex  );
	
	pDef->createPolicy.StartHotCopyWithManualBreak = 1;

	return rc ;
}


};

#endif // __HOT_COPY_MANUAL_H__