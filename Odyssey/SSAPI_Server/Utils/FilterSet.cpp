//************************************************************************
// FILE:		FilterSet.cpp
//
// PURPOSE:		Defines the class FilterSet which wraps up all the 
//				functionality necessary to create filters and get a 
//				filtered set of results by pipelining results from a 
//				previous filter to the following one.
//************************************************************************


#include "FilterSet.h"
#include "SSAPIFilter.h"
#include "ValueSet.h"
#include "CoolVector.h"
#include "ManagedObject.h"

#include "..\msl\osheap.h"


//************************************************************************
// FilterSet:
//
// PURPOSE:		The default constructor.
//
// RECEIVE:		valueSet:	the value set that contains value sets for all
//							necessary filters
//************************************************************************

FilterSet::FilterSet( const ValueSet &valueSet ){

	ValueSet	*pFilterValueSet;
	U32			index;
	Filter		*pFilter;

	m_isProperlyInited = false;

	if( !((ValueSet&)valueSet).GetCount() )
		return;

	m_pFilters = new CoolVector;
	for( index = 0; (int)index < ((ValueSet&)valueSet).GetCount(); index++ ){
		pFilterValueSet = (ValueSet *)((ValueSet&)valueSet).GetValue( index );
		pFilter = Filter::Ctor( *pFilterValueSet );
		if( pFilter )
			m_pFilters->AddAt( (CONTAINER_ELEMENT)pFilter, index );
		else
			return;
	}
	m_isProperlyInited = true;
}


//************************************************************************
// FilterSet:
//
// PURPOSE:		The destructor
//************************************************************************

FilterSet::~FilterSet(){

	Filter		*pFilter;

	while( m_pFilters->Count() ){
		m_pFilters->GetAt( ( CONTAINER_ELEMENT &)pFilter, 0 );
		m_pFilters->RemoveAt( 0 );
		delete pFilter;
	}
	delete m_pFilters;
}


//************************************************************************
// BuildFilteredSet:
//
// PURPOSE:		Runs a set of objects thru a filter set pipelining results.
//				Prepares and returns the filtered set.
//
// RETURN:		true:		was able to do the work successfully
//				false:		something went wrong (maybe, even before this
//							method was called )
//************************************************************************

bool 
FilterSet::BuildFilteredSet(const DesignatorIdVector&	objectsToFilter, 
							DesignatorIdVector&			resultSet ){

	ManagedObject			*pManagedObject;
	Filter					*pFilter;
	U32						objectNumber, filterNumber;
	Filter::FILTER_RC		filterRc;

	if( !m_isProperlyInited )
		return false;

	for( objectNumber = 0; objectNumber < objectsToFilter.GetCount(); objectNumber++ ){
		((DesignatorIdVector&)objectsToFilter).GetAt( pManagedObject, objectNumber );

		pManagedObject->BuildYourValueSet();

		for( filterNumber = 0; filterNumber < m_pFilters->Count(); filterNumber++ ){
			m_pFilters->GetAt( (CONTAINER_ELEMENT &)pFilter, filterNumber );
			filterRc = pFilter->DoesApply( *pManagedObject );
			if( filterRc != Filter::APPLIES ) {
				pManagedObject->Clear();
				break;
			}
		}
		

		if( filterRc == Filter::NO_MORE_ITEMS_WILL_APPLY ) {
			pManagedObject->Clear();
			break;
		}

		if( filterRc == Filter::APPLIES )
			resultSet.Add( pManagedObject, pManagedObject->GetDesignatorId() );

		pManagedObject->Clear();
	}

	m_isProperlyInited	= false;

	return true;
}





