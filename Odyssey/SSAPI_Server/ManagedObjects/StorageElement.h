//******************************************************************************
// FILE:		StorageElement.cpp
//
// PURPOSE:		Implements the class that will serve as an bastract base class
//				for storage elements in the O2K that can be used for creation 
//				of other storage elements.
//******************************************************************************

#ifndef __STORAGE_ELEMENT_H__
#define __STORAGE_ELEMENT_H__

#include "StorageElementBase.h"

class StorageManager;
#ifdef WIN32
#pragma pack(4)
#endif



class StorageElement : public StorageElementBase{

	
protected:

//******************************************************************************
// StorageElement:
//
// PURPOSE:		The default constructor
//******************************************************************************

StorageElement( ListenManager *pListenManager, U32 objectClassType, ObjectManager *pManager );


public:

//******************************************************************************
// ~StorageElement:
//
// PURPOSE:		The destructor
//******************************************************************************

virtual ~StorageElement();


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

virtual U32 GetCoreTableMask( );

};

#endif // __STORAGE_ELEMENT_H__