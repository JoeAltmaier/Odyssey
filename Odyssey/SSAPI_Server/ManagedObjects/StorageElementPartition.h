//************************************************************************
// FILE:		StorageElementPartition.h
//
// PURPOSE:		Defines the object whose instance will be used to represent
//				internally created partitions in the O2K system.
//************************************************************************

#ifndef	__PARTITION_SE_H__
#define	__PARTITION_SE_H__

#include "StorageElement.h"
#include "PartitionTable.h"


class StorageElementPartition : public StorageElement{

	DesignatorId		m_ownerId;
	DesignatorId		m_prevPartitionId;
	DesignatorId		m_nextPartitionId;

	
	RowId				m_partitionRid;			// internal
public:


//************************************************************************
// StorageElementPartition:
//
// PURPOSE:		Default constructor
//************************************************************************

StorageElementPartition( ListenManager *pLM, ObjectManager *pManager );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new StorageElementPartition( GetListenManager(), GetManager() ); }


//************************************************************************
// ~StorageElementPartition:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~StorageElementPartition();


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates data members based on the pts row
//************************************************************************

void BuildYourselfFromPtsRow( PARTITION_DESCRIPTOR *pRow );


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
// operator=:
//************************************************************************

virtual const ValueSet& operator=(const ValueSet& obj ){ return *((ValueSet*)this) = obj; }


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
// Public Accessors:
//************************************************************************

DesignatorId&	GetOwnerId() const { return (DesignatorId&)m_ownerId; }
DesignatorId&	GetNextPartitionId() const { return (DesignatorId&)m_nextPartitionId; }
DesignatorId&	GetPrevPartitionId() const { return (DesignatorId&)m_prevPartitionId; }
RowId&			GetPartitionRid() const { return (RowId&)m_partitionRid; }

//******************************************************************************
// Smart/virtual Accessors
//******************************************************************************

virtual bool GetCanBeRemoved() { return m_isUsed? false : true; }
virtual bool IsExportable(){ return m_isUsed? false : true ; }

protected:

//******************************************************************************
// SetYourState:
//
// PURPOSE:		Requires that sub classes set their state and state string
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

virtual bool PopulateYourIdInfo( StorageIdInfo& idInfo ){ return false; }
};

#endif	// __PARTITION_SE_H__
