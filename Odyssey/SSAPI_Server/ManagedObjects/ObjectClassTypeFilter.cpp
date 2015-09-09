//************************************************************************
// FILE:		ObjectClassTypeFilter.h
//
// PURPOSE:		Defines the ObjectClassTypeFilter object that will be used
//				to filter result sets for ObjectManager::ListIds().
//				The filter is intellegent enough to be able to tell if
//				a given object is derived (directly or indirectly) from
//				another given object.
//************************************************************************

#include "ObjectClassTypeFilter.h"
#include "ClassTypeMap.h"



//************************************************************************
// ObjectClassTypeFilter:
//
// PURPOSE:		The default constructor
//************************************************************************

ObjectClassTypeFilter::ObjectClassTypeFilter( const ValueSet &valueSet )
						:Filter( valueSet ){

	m_pMap	= new ClassTypeMap;
}


//************************************************************************
// ~ObjectClassTypeFilter:
//
// PURPOSE:		The destructor
//************************************************************************

ObjectClassTypeFilter::~ObjectClassTypeFilter(){

	delete m_pMap;
}


//************************************************************************
// DoesApply:
//
// PURPOSE:		Gives a chance to the filter to decide if a given valueset
//				applies to the filtered set
//************************************************************************

Filter::FILTER_RC 
ObjectClassTypeFilter::DoesApply( ValueSet &valueSet ){
	
	int		objectClassType;
	bool	rc;
	
	valueSet.GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &objectClassType );

	if( m_baseClassId ==  objectClassType )
		rc = true;
	else
		rc = m_pMap->IsADerivedClass( m_baseClassId, objectClassType, m_shouldIncludeDerivations? true : false );

	return rc? Filter::APPLIES : Filter::DOES_NOT_APPLY;
}


//************************************************************************
// BuildYourselfFromAValueSet:
//
// PURPOSE:		Populates data members with values from the value set.
//				Derived classes should override this method if they want
//				to populate their data members AND THEY MUST call this
//				method on their base class before doing any processing!
//************************************************************************

bool 
ObjectClassTypeFilter::BuildYourselfFromAValueSet( ValueSet &valueSet ){

	if( !Filter::BuildYourselfFromAValueSet( valueSet ) )
		return true;

	if( !((ValueSet&)valueSet).GetInt( SSAPI_OBJECT_CLASS_TYPE_FILTER_OBJECT_CLASS_TYPE, &m_baseClassId ) )
		return false;

	if( !((ValueSet&)valueSet).GetInt( SSAPI_OBJECT_CLASS_TYPE_FILTER_INCLUDE_DERIVATIONS, &m_shouldIncludeDerivations ) )
		return false;

	return true;
}

