//************************************************************************
// FILE:		TableMetaData.h
//
// PURPOSE:		Defines the object that will contain the enumeration of
//				all PTS tables.
//************************************************************************

#ifndef __TABLE_META_DATA_H__
#define	__TABLE_META_DATA_H__

#include "ManagedObject.h"
#include "UnicodeString.h"
#include "PtsCommon.h"
#include "CoolVector.h"

class TableInfo : public ValueSet{
	
	RowId			m_rid;

public:
	StringClass		m_tableName;
	RowId			m_tableRid;

	TableInfo( StringClass name, DesignatorId id, RowId  rid ){
		UnicodeString	us = name;

		AddGenericValue( (char *)&id, sizeof(id), SSAPI_TABLE_INFO_FID_ID );
		AddString( &us, SSAPI_TABLE_INFO_FID_NAME );
		m_rid = rid;
		m_tableName = name;
		m_tableRid = id.GetRowId();
	}

	RowId& GetRid() const { return (RowId &)m_rid; }
};

class TableMetaData : public ManagedObject {

	CoolVector			m_infoCells;

public:

//************************************************************************
// TableMetaData:
//************************************************************************

TableMetaData( ListenManager *pListenManager );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new TableMetaData( GetListenManager() ); }


//************************************************************************
// ~TableMetaData:
//************************************************************************

virtual ~TableMetaData();


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
// AddTableInfoCell:
//************************************************************************

void AddTableInfoCell( rowDef *pRow, bool postEvent = false );


//************************************************************************
// RemoveTableInfoCell:
//************************************************************************

void RemoveTableInfoCell( rowDef *pRow, bool postEvent = false );
void RemoveTableInfoCell( RowId &rid, bool postEvent = false );

//************************************************************************
//
//************************************************************************

void GetTableIdByTableName( StringClass name, RowId &rid );
U32	 GetTableCount() const { return m_infoCells.Count(); }	
TableInfo* GetTableAt( U32 index );

private:

//************************************************************************
// ClearInfoCells:		
//					
// PURPOSE:			Memory clean-up.
//************************************************************************

void ClearInfoCells();


};

#endif	// __TABLE_META_DATA_H__