//************************************************************************
// FILE:		TableStuffRow.h
//
// PURPOSE:		Defines that object whose instances will be used to
//				represent PTS table rows.
//************************************************************************


#ifndef __TABLE_STUFF_ROW_H__
#define	__TABLE_STUFF_ROW_H__


#include "ManagedObject.h"
#include "PtsCommon.h"

class ShadowTable;

class TableStuffRow : public ManagedObject {

	ShadowTable		*m_pTable;
	void			*m_pRowData;
	U32				m_rowDataSize;


public:

//************************************************************************
//
//************************************************************************

TableStuffRow(	ListenManager *pLM, 
				ObjectManager *pManager,
				ShadowTable	  *pTable,
				void		  *pData,
				U32			  dataSize );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new TableStuffRow(GetListenManager(),
																  GetManager(),
																  m_pTable,
																  m_pRowData,
																  m_rowDataSize ); }

//************************************************************************
//
//************************************************************************

virtual ~TableStuffRow();


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
// ModifyObject:
//
// PURPOSE:		Modifes contents of the object
//
// NOTE:		Must be overridden by objects that can be modified
//************************************************************************

virtual bool ModifyObject( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

virtual bool DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder );


ShadowTable& GetShadowTable() { return *m_pTable; }


private:

//************************************************************************
// PutValue:		Based on the value type, adds it properly into the
//					value set provided
//************************************************************************

void PutValue( fieldDef *pField, ValueSet *pVs, U32 position, char* &pCurrent );

};
#endif // __TABLE_STUFF_ROW_H__