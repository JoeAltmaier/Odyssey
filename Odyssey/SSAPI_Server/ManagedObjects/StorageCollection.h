//************************************************************************
// FILE:		StorageCollection.h
//
// PURPOSE:		Defines the object whose instance will aggregate 
//				storage element based on some logical similarity.
//************************************************************************


#ifndef __STORAGE_COLLECTION_H__
#define __STORAGE_COLLECTION_H__

#include "StorageElementBase.h"
#include "Container.h"
#ifdef WIN32
#pragma pack(4)
#endif


class StorageCollection : public StorageElementBase{


protected:

//************************************************************************
// StorageCollection:
//
// PURPOSE:		Default constructor
//************************************************************************

StorageCollection( ListenManager *pListenManager, U32 objectClassType, ObjectManager *pManager );


public:

//************************************************************************
// ~StorageCollection:
//
// PURPOSE:		Default destructor
//************************************************************************

virtual ~StorageCollection();


//******************************************************************************
// ReportYourUnderlyingDevices:
//
// PURPOSE:		Gives a chance to every Storage Element to report devices that
//				make it up.
//
// FUNTIONALITY: Derived classes must populate the specified vector with IDs of
//				devices that they contain of. Memory allocated will be freed
//				by the caller, so derived classes must not deallocate anything.
//******************************************************************************

virtual void  ReportYourUnderlyingDevices( Container &devices );


//******************************************************************************
// GetPhsDataRowId:
//
// PURPOSE:		Returns a row id of the PHS data objects that apply to the element
//******************************************************************************

virtual void GetPhsDataRowId(Container &container) { container.RemoveAll(); }

protected:

//******************************************************************************
// SetYourState:
//
// PURPOSE:		Requires that super classes set their state and state string
//******************************************************************************

virtual void SetYourState();


//******************************************************************************
// PopulateYourIdInfo:
//
// PURPOSE:		Requests that the element populates it id info object
//
// RETURN:		true:	the element has the id info and has populated it
//				false:	the element has no id info
//******************************************************************************

virtual bool PopulateYourIdInfo( StorageIdInfo& idInfo ) { return false; }

};
#endif // __STORAGE_COLLECTION_H__