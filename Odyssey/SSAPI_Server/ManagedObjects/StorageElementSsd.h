//************************************************************************
// FILE:		StorageElementSsd.h
//
// PURPOSE:		Defines the object whose instances are used to represent
//				SSD boards as storage elements in the O2K system
//************************************************************************

#ifndef __STORAGE_ELEMENT_SSD_H__
#define	__STORAGE_ELEMENT_SSD_H__

#include "StorageElement.h"


class StorageElementSsd : public StorageElement {

	RowId				m_ridDescriptor;


public:


//************************************************************************
// StorageElementSsd:
//
// PURPOSE:		Default constructor
//************************************************************************

StorageElementSsd( ListenManager *pLM, ObjectManager *pManager );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new StorageElementSsd( GetListenManager(), GetManager() ); }


//************************************************************************
// ~StorageElementSsd:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~StorageElementSsd();


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
// PURPOSE:		Populates data members with information in the PTS row
//************************************************************************

void BuildYourselfFromPtsRow( void *pRow_ );


//************************************************************************
// IsYourElement:
//
// PURPOSE:		Determines if a gven element belongs to this element
//				in the logical hierachy. 
//
// RETURN:		true:		yes
//				false:		no
//************************************************************************

virtual bool IsYourElement( StorageElementBase &element, StorageManager *pManager ){ return false; }


//******************************************************************************
// Smart/virtual Accessors
//******************************************************************************

virtual bool GetCanBeRemoved(){	return false; }
virtual bool IsExportable() { return m_isUsed? false : true; }


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before an object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

virtual void ComposeYourOverallState();


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

virtual void ReportYourUnderlyingDevices( Container &devices );


protected:

//******************************************************************************
// PopulateYourIdInfo:
//
// PURPOSE:		Requests that the element populates it id info object
//
// RETURN:		true:	the element has the id info and has populated it
//				false:	the element has no id info
//******************************************************************************

virtual bool PopulateYourIdInfo( StorageIdInfo& idInfo ){ return false; }


//******************************************************************************
// SetYourState:
//
// PURPOSE:		Requires that sub classes set their state and state string
//******************************************************************************

virtual void SetYourState();

};


#endif