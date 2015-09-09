//************************************************************************
// FILE:		ObjectClassTypeFilter.h
//
// PURPOSE:		Defines the ObjectClassTypeFilter object that will be used
//				to filter result sets for ObjectManager::ListIds().
//				The filter is intellegent enough to be able to tell if
//				a given object is derived (directly or indirectly) from
//				another given object.
//************************************************************************

#ifndef __OBJECT_CLASS_TYPE_FILTER_H__
#define	__OBJECT_CLASS_TYPE_FILTER_H__

#include "SSAPIFilter.h"
#include "SSAPITypes.h"

class ClassTypeMap;

#ifdef WIN32
#pragma pack(4)
#endif


class ObjectClassTypeFilter : public Filter{

	int					m_baseClassId;
	int					m_shouldIncludeDerivations;
	ClassTypeMap		*m_pMap;

protected:

//************************************************************************
// ObjectClassTypeFilter:
//
// PURPOSE:		The default constructor
//************************************************************************

ObjectClassTypeFilter( const ValueSet &valueSet );


public:


//************************************************************************
// ~ObjectClassTypeFilter:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~ObjectClassTypeFilter();


//************************************************************************
// Ctor:
//
// PURPOSE:		A means of dynamic, self-contained type creation.
//				new()s and returns an instance
//************************************************************************

static Filter* Ctor( const ValueSet &valueSet ){ 
	return new ObjectClassTypeFilter( valueSet ); 
}


//************************************************************************
// DoesApply:
//
// PURPOSE:		Gives a chance to the filter to decide if a given valueset
//				applies to the filtered set
//************************************************************************

virtual Filter::FILTER_RC DoesApply( ValueSet &valueSet );



protected:


//************************************************************************
// BuildYourselfFromAValueSet:
//
// PURPOSE:		Populates data members with values from the value set.
//				Derived classes should override this method if they want
//				to populate their data members AND THEY MUST call this
//				method on their base class before doing any processing!
//************************************************************************

virtual bool BuildYourselfFromAValueSet( ValueSet &valueSet );

};



#endif	// __OBJECT_CLASS_TYPE_FILTER_H__