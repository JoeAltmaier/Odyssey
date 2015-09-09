//******************************************************************************
// FILE:		StorageElementBase.h
//
// PURPOSE:		Defines the class that will serve as an bastract base class
//				for all storage elements in the O2K
//******************************************************************************

#ifndef __STORAGE_ELEMENT_BASE_H__
#define __STORAGE_ELEMENT_BASE_H__

#include "ManagedObject.h"
#include "StatusReporterInterface.h"
#include "SSAPITypes.h"
#include "RaidDefs.h"
#include "StorageRollCallTable.h"
#include "SList.h"

class StorageManager;
class StorageIdInfo;

#ifdef WIN32
#pragma pack(4)
#endif

#define BLOCK_SIZE		512

class StorageElementBase : public ManagedObject, public StatusReporterInterface{

	bool				m_canBeRemoved;
	UnicodeString		m_name;
	bool				m_isSpare;
	bool				m_isExportable;
	bool				m_isMember;			// internal - if yes, all status rollup is done automagically
	bool				m_isPoolSpare;		// internal
	DesignatorId		m_dedicatedHostId;	// internal

protected:

	bool				m_isUsed;
	I64					m_capacity;
	friend class StorageManager;
	RowId				m_ridStatusRecord;		// for internal purposes (to find PHSs)
	RowId				m_ridPerformanceRecord;	// for internal purposes (to find PHSs)	
	RowId				m_ridName;				// internal
	RowId				m_ridDescriptor;		// internal

//******************************************************************************
// StorageElement:
//
// PURPOSE:		The default constructor
//******************************************************************************

StorageElementBase( ListenManager *pListenManager, U32 objectClassType, ObjectManager *pManager );

public:

//******************************************************************************
// ~StorageElement:
//
// PURPOSE:		The destructor
//******************************************************************************

virtual ~StorageElementBase();


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
// ApplyNecessaryStatusRollupRules:
//
// PURPOSE:		Applies applicabale rollup rules.
//				The object that implements this method is expected to call
//				the protected method with the same name and specify 
//				necessary parms.
//
// RETURN:		true:			object's state's been changed
//				false:			object's state's remained unchanged
//************************************************************************

virtual bool ApplyNecessaryStatusRollupRules();


//************************************************************************
// IsYourElement:
//
// PURPOSE:		Determines if a gven element belongs to this element
//				in the logical hierachy. 
//
// RETURN:		true:		yes
//				false:		no
//************************************************************************

virtual bool IsYourElement( StorageElementBase &element, StorageManager *pManager ) = 0;


//******************************************************************************
// BuildYourselfFromSRCRow:
//
// PURPOSE:		Populates data members based on the SRC row
//******************************************************************************

void BuildYourselfFromSRCRow( StorageRollCallRecord *pRow );


//******************************************************************************
// Smart/virtual Accessors
//******************************************************************************

virtual bool GetCanBeRemoved() = 0;


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

virtual U32 GetCoreTableMask( ) = 0 ;


//************************************************************************
// operator=:
//
//************************************************************************

virtual const ValueSet& operator=(const ValueSet& obj ){ return *((ValueSet *)this) = obj; }


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

virtual void ReportYourUnderlyingDevices( Container &devices ) = 0;


//******************************************************************************
// Accessors:
//******************************************************************************

bool GetIsUsed() const { return m_isUsed; }
I64	 GetCapacity() const { return m_capacity; }
UnicodeString GetName()	const { return m_name; };
bool GetIsSpare() const { return m_isSpare; }
bool GetIsMember() const { return m_isMember; }
bool GetIsPoolSpare() const { return m_isPoolSpare; }
DesignatorId GetDedicatedHostId() const { return m_dedicatedHostId; }
RowId GetRidName() const { return m_ridName; }
RowId GetRidDescriptor() const { return m_ridDescriptor; }
RowId GetRidStatus() { return m_ridStatusRecord; }
RowId GetRidPerformance() { return m_ridPerformanceRecord; }

void SetRidName( RowId rid ) { if( RowId() == rid ) m_name = ""; m_ridName = rid; }
void SetMemberStatus( RAID_MEMBER_STATUS status, bool postEvent = true );
void UnsetAsMember() { m_isMember = false; m_isUsed = false; }
void SetAsDedicatedSpare(){ m_isSpare = true; m_isUsed = true; }
void SetAsPoolSpare() { m_isSpare = true; m_isPoolSpare = true; m_isUsed = true;}
void UnsetAsSpare() { m_isSpare = m_isPoolSpare = false; m_isUsed = false; }
void SetName( UnicodeString name, bool postEvent = true ) { if( name != m_name ) {m_name = name; if( postEvent ) FireEventObjectModifed();} }
void SetIsUsed( bool isUsed, bool postEvent = false ) { m_isUsed = isUsed; if(postEvent) FireEventObjectModifed(); }


virtual bool IsExportable() = 0;


//******************************************************************************
// GetPhsDataRowId:
//
// PURPOSE:		Returns a row id of the PHS data objects that apply to the element
//
// NOTE:		The caller must de-allocate the pointer in the container
//				(instances of RowId);
//******************************************************************************

virtual void GetPhsDataRowId( Container &container ){
	container.Add( (CONTAINER_ELEMENT) new RowId(m_ridStatusRecord) );
	container.Add( (CONTAINER_ELEMENT) new RowId(m_ridPerformanceRecord) );
}

protected:

//******************************************************************************
// SetYourState:
//
// PURPOSE:		Requires that sub classes set their state and state string
//******************************************************************************

virtual void SetYourState() = 0;


//******************************************************************************
// PopulateYourIdInfo:
//
// PURPOSE:		Requests that the element populates it id info object
//
// RETURN:		true:	the element has the id info and has populated it
//				false:	the element has no id info
//******************************************************************************

virtual bool PopulateYourIdInfo( StorageIdInfo& idInfo ) = 0;


};

#endif // __STORAGE_ELEMENT_BASE_H__