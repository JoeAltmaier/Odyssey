//************************************************************************
// FILE:		HostConnectionCollection.h
//
// PURPOSE:		Defines class HostConnectionCollection that will serve
//				as an abstract base class for all host connection collection
//				classes used in the system. Provided to enforce the 
//				policy of how a LUN is exported (to a COLLECTION of host
//				connections!)
//************************************************************************

#ifndef __HOST_CONNECTION_COLLECTION_H__
#define __HOST_CONNECTION_COLLECTION_H__

#include "HostConnectionElement.h"
#include "HostCollectionDescriptorTable.h"
#include "StringClass.h"
#include "RowIdCollectionTable.h"
#include "SList.h"

class ShadowTable;

#pragma pack(4)


class HostConnectionCollection : public HostConnectionElement{

protected:

	ShadowTable			*m_pPointerTable;
	StringClass			m_pointerTableName;
	DesignatorId		m_lunId;

	SList				childInfo;		// internal

public:

//************************************************************************
// HostConnectionCollection:
//
// PURPOSE:		Default constructor
//************************************************************************

HostConnectionCollection( ListenManager *pListenManager, U32 objectClassType )
	:HostConnectionElement( pListenManager, objectClassType ){}



//************************************************************************
// ~HostConnectionCollection:
//************************************************************************

virtual ~HostConnectionCollection();



//************************************************************************
// Accessors:
//
//************************************************************************

ShadowTable*	GetShadowTable() const	{ return m_pPointerTable; }
void SetShadowTable( ShadowTable *pT )	{ m_pPointerTable = pT; }	
StringClass GetPointerTableName() const	{ return m_pointerTableName; }


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates data members based on the PTS row
//************************************************************************

virtual void BuildYourselfFromPtsRow( HostCollectionDescriptorRecord *pRow );


//************************************************************************
// WriteYourselfIntoPtsRow:
//
// PURPOSE:		Populates pts row based on the data members
//************************************************************************

virtual void WriteYourselfIntoPtsRow( HostCollectionDescriptorRecord *pRow );


//************************************************************************
// BuildYourselfFromRowIdCollectionRows:
//
// PURPOSE:		Builds its children ids from the table
//************************************************************************

void BuildYourselfFromRowIdCollectionRows( RowIdCollectionRecord *pRows, U32 rowCount );


//************************************************************************
// WriteYourselfIntoRowIdCollectionRow:
//
// PURPOSE:		Writes its children ids into the table
//************************************************************************

void WriteYourselfIntoRowIdCollectionRow( RowIdCollectionRecord *pRows, U32 rowCount );


//************************************************************************
// DeleteAChild:
//
// PURPOSE:		Deletes a child id
//************************************************************************

void DeleteAChild( ManagedObject *pObj );


//************************************************************************
// AddAChild:
//
// PURPOSE:		Adds a connection id
//************************************************************************

void AddAChild( ManagedObject *pObj, RowId rowIdInCollectionTable );


//************************************************************************
// GetRowIdIdOfChild:
//
// PURPOSE:		Performs a lookup of row id by child id
//************************************************************************

RowId GetRowIdIdOfChild( DesignatorId childId );


//************************************************************************
//************************************************************************

virtual const ValueSet& operator=(const ValueSet& obj ){

	return *(ValueSet*)this = obj;
}


//************************************************************************
// GetChildId:
//
// PURPOSE:		Returns child id at the colcation requested
//************************************************************************

DesignatorId GetChildId( U32 position );


//************************************************************************
// GetChildCount:
//************************************************************************

U32 GetChildCount() const { return m_children.Count(); }


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Responsible for state reconcilation
//************************************************************************

virtual void ComposeYourOverallState();


//************************************************************************
// AreThereAnyTooLongStrings
//
// PURPOSE:		Checks every string's size against the maximum in the PTS 
//				row. Will raise an exception if something's wrong.
//				Checks strings in the value set, not the members. 
//
// RETURN:		true:	all strings are OK, may proceed
//				false:	an exception was rased, terminate normal execution
//************************************************************************

virtual bool AreThereAnyTooLongStrings( SsapiResponder *pResponder );

};

struct CHILD_INFO_CELL{
	DesignatorId		childId;
	RowId				rowId;

	CHILD_INFO_CELL( DesignatorId id, RowId rid ){ childId = id; rowId = rid; }
};

#endif __HOST_CONNECTION_COLLECTION_H__