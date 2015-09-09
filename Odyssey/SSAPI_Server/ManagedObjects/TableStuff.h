//************************************************************************
// FILE:		TableStuff.h
//
// PURPOSE:		Defines the object that represents a PTS table in the 
//				O2K system.
//************************************************************************


#ifndef __TABLE_STUFF_H__
#define	__TABLE_STUFF_H__

#include "ManagedObject.h"
#include "CoolVector.h"

class ShadowTable;


class TableStuff : public ManagedObject {

	ShadowTable			*pShadowTable;

public:	

//************************************************************************
//
//************************************************************************

TableStuff( ListenManager *pLM, ShadowTable *pST, DesignatorId id );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new TableStuff( GetListenManager(), pShadowTable, GetDesignatorId() ); }


//************************************************************************
// 
//************************************************************************

virtual ~TableStuff();


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
//
//************************************************************************

virtual const ValueSet& operator=(const ValueSet& obj ){ return *((ValueSet *)this) = obj; }


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

virtual bool DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder );


ShadowTable* GetShadowTable() { return pShadowTable; }
};
#endif	// __TABLE_MO_H__