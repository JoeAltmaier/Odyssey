//************************************************************************
// FILE:		StorageElementArray.h
//
// PURPOSE:		Defines the object that will be used to represent RAID
//				arays inside the O2K system. This is an abstract class
//				to be extended by specific RAID level type objects
//************************************************************************

#ifndef __STORAGE_ELEMENT_ARRAY_H__
#define	__STORAGE_ELEMENT_ARRAY_H__

#include "StorageElement.h"
#include "RaidDefs.h"
#include "ArrayDescriptor.h"
#include "RmstrCmnds.h"

#ifdef WIN32
#pragma pack(4)
#endif



class StorageManager;

class StorageElementArray : public StorageElement {

protected:

	I64					m_memberCapacity;
	I64					m_dataBlockSize;
	I64					m_parityBlockSize;
	U32					m_peckingOrder;
	bool				m_isInited;
	U32					m_serialNumber;
	LocalizedDateTime	m_creationTime;
	DesignatorId		m_hostSparePoolId;		// host object id

	U32					m_numberOfUtilsRunning;			// internal
	U32					m_numberOfMembers;				// internal
	U32					m_numberOfSpares;				// internal
	RowId		 		m_members[MAX_ARRAY_MEMBERS];	// internal
	RowId		 		m_spares[MAX_ARRAY_SPARES];		// internal
	RAID_ARRAY_STATUS	m_status;						// internal
	RowId				m_arrayRid;						// internal


protected:

	friend class StorageManager;

//************************************************************************
// StorageElementArray:
//
// PURPOSE:		Default constructor
//************************************************************************

StorageElementArray( ListenManager *pListenManager, U32 objectClassType, ObjectManager *pManager );


public:


//************************************************************************
// ~StorageElementArray:
//
// PURPOSE:		Default destructor
//************************************************************************

virtual ~StorageElementArray();


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

virtual bool IsYourElement( StorageElementBase &element, StorageManager *pManager );


//******************************************************************************
// Smart/virtual Accessors
//******************************************************************************

virtual bool GetCanBeRemoved() { return m_numberOfUtilsRunning? false : true; }


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates data members based on the row information.
//				Marks spares and sets members' state
//************************************************************************

virtual void BuildYourselfFromPtsRow(	RAID_ARRAY_DESCRIPTOR	*pRow,
										ObjectManager			*pManager );


//************************************************************************
// IsYourDedicatedSpare:
//
// PURPOSE:		Checks if the element id a dedicated spare of this array
//************************************************************************

bool IsYourDedicatedSpare( StorageElement &element, StorageManager *pManager );
RowId GetArrayRid() const { return m_arrayRid; }

//******************************************************************************
// operator =
//******************************************************************************

virtual const ValueSet& operator=(const ValueSet& obj ){ return *((ValueSet *)this) = obj; }


//************************************************************************
// WriteYourselfIntoCreateArrayDefinition:
//
// PURPOSE:		Writes the array members into the array definition struct
//				for the RAID master
//************************************************************************

virtual bool WriteYourselfIntoCreateArrayDefinition(ValueSet &values, 
													RMSTR_CREATE_ARRAY_DEFINITION *pDef,
													U32 hotCopyPirority  =0,
													ManagedObject	*pHotCopyExportIndex = 0) = 0;


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

virtual bool DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder );


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


private:

//************************************************************************
// IsAMemberOfThisArray:
//
// PURPOSE:		Checks if a 'rid' is in the array 'rids'
//************************************************************************

bool IsAMemberOfThisArray( RowId rid, RowId rids[], U32 size );


//************************************************************************
//************************************************************************

};

#endif // __STORAGE_ELEMENT_ARRAY_H__
