//************************************************************************
// FILE:		StorageElementDisk.h
//
// PURPOSE:		Defines the object whose instances will represent 
//				internal disk storage elements
//************************************************************************

#ifndef __STORAGE_ELEMENT_DISK_H__
#define __STORAGE_ELEMENT_DISK_H__

#include "StorageElement.h"
#include "DiskDescriptor.h"

class StorageManager;
struct PathDescriptor;

#ifdef WIN32
#pragma pack(4)
#endif


class StorageElementDisk : public StorageElement {

protected:

	U32				m_status;				// internal , to determine state
	RowId			m_ridDescriptor;		// to get to the appropriate device object
	INQUIRY			m_inquiry;
	U32				m_targetId;
	U32				m_targetLUN;
	UnicodeString	m_serialNumber;
	StringClass		m_wwName;

	U32				m_instanceNumber;		// internal: FC instnce

public:

//************************************************************************
// StorageElementDisk:
//
// PURPOSE:		Default constructor
//************************************************************************

StorageElementDisk( ListenManager *pListenManager, U32 classType, ObjectManager *pManager );


//************************************************************************
// ~StorageElementDisk:
//
// PURPOSE:		Default destructor
//************************************************************************

virtual ~StorageElementDisk();


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
// IsYourElement:
//
// PURPOSE:		Determines if a gven element belongs to this element
//				in the logical hierachy. 
//
// RETURN:		true:		yes
//				false:		no
//************************************************************************

virtual bool IsYourElement( StorageElementBase &element, StorageManager* ){ return false; }


//******************************************************************************
// Smart/virtual Accessors
//******************************************************************************

virtual bool GetCanBeRemoved(){ return GetIsSpare()? true : false; }


//************************************************************************
// BuildYourselfFromPtsRow
// 
// PURPOSE:		Populates data members based on the info in the row
//************************************************************************

void BuildYourselfFromPtsRow( DiskDescriptor *pRow );


//************************************************************************
// BuildYourselfFromPathRow:
//
// PURPOSE:		Populates data members from the path descriptor
//************************************************************************

void BuildYourselfFromPathRow( PathDescriptor *pPath );


//************************************************************************
// Public accessors:
//************************************************************************

U32 GetInstanceNumber() const { return m_instanceNumber; }


protected:

//******************************************************************************
// SetYourState:
//
// PURPOSE:		Requires that super classes set their state and state string
//******************************************************************************

virtual void SetYourState();


//******************************************************************************
// IsExportable:
//
// PURPOSE:		Gives every possible storage element type a chance to voice
//				its abilities
//******************************************************************************

virtual bool IsExportable() { return !m_isUsed; }


//******************************************************************************
// PopulateYourIdInfo:
//
// PURPOSE:		Requests that the element populates it id info object
//
// RETURN:		true:	the element has the id info and has populated it
//				false:	the element has no id info
//******************************************************************************

virtual bool PopulateYourIdInfo( StorageIdInfo& idInfo ) ;


};

#endif // __STORAGE_ELEMENT_DISK_H__