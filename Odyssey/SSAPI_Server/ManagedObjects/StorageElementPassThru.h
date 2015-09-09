//************************************************************************
// FILE:		StorageElementPassThru.h
//
// PURPOSE:		Defines an abstract base class for all PASS_THROUGH type
//				of devices in the O2K
//************************************************************************

#ifndef __STORAGE_ELEMENT_PASS_THRU_H__
#define	__STORAGE_ELEMENT_PASS_THRU_H__

#include "StorageElementBase.h"
#include "Scsi.h"

#ifdef WIN32
#pragma pack(4)
#endif

struct DeviceDescriptor;
struct PathDescriptor;

class StorageElementPassThru : public StorageElementBase {


protected:

	U32				m_targetId;
	U32				m_targetLUN;
	UnicodeString	m_serialNumber;
	INQUIRY			m_inquiry;
	U32				m_instanceNumber;
	U32				m_internalState;		// to determine generic status	

//************************************************************************
// StorageElementBase:
//
// PURPOSE:		Default constructor
//************************************************************************

StorageElementPassThru( ListenManager *pListenManager, U32 classType, ObjectManager *pManager )
:StorageElementBase( pListenManager, classType, pManager ){}


public:


//************************************************************************
// ~StorageElementPassThru:
// 
// PURPOSE:		The destructor
//************************************************************************

virtual ~StorageElementPassThru() {}


//************************************************************************
// Virtual vector handlers
//************************************************************************

virtual bool IsExportable()		{ return true; }
virtual bool GetCanBeRemoved()	{ return false;}
virtual bool IsYourElement( StorageElementBase &element, StorageManager *pManager )
								{ return false; }


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


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates data members based on the contents of the PTS 
//				row supplied.
//************************************************************************

void BuildYourselfFromPtsRow( DeviceDescriptor *pRow );


//************************************************************************
// BuildyourselfFromPathRow:
//
// PURPOSE:		Populates some of data members. Filters out inactive
//				paths, only taking data from the sactive one (one is in 
//				1st release, after that --- could be more )
//************************************************************************

void BuildYourselfFromPathRow( PathDescriptor *pPath );


protected:

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

#endif	// __STORAGE_ELEMENT_PASS_THRU_H__