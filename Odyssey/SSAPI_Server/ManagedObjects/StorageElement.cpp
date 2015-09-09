//******************************************************************************
// FILE:		StorageElement.cpp
//
// PURPOSE:		Implements the class that will serve as an bastract base class
//				for storage elements in the O2K that can be used for creation 
//				of other storage elements.
//******************************************************************************

#include "StorageElement.h"
#include "StorageManager.h"

//******************************************************************************
// StorageElement:
//
// PURPOSE:		The default constructor
//******************************************************************************

StorageElement::StorageElement( ListenManager *pListenManager, U32 objectClassType, ObjectManager *pManager )
:StorageElementBase( pListenManager, objectClassType, pManager ) {}


//******************************************************************************
// ~StorageElement:
//
// PURPOSE:		The destructor
//******************************************************************************

StorageElement::~StorageElement(){
}


//************************************************************************
// GetCoreTableMask:
//
// PURPOSE:		The methods returns the integer mask of the core table
//				withing the StorageManager. The core table for a storage 
//				element is the table which defines the very existance
//				of the storage element.
//				Before, we only had one such table -- the StorageRollCall
//				table. Now, there is another one - the DeviceDescriptor.
//************************************************************************

U32 
StorageElement::GetCoreTableMask( ){

	return SSAPI_SM_SRC_TABLE;
}