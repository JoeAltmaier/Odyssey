//************************************************************************
// FILE:		CountFilter.cpp
//
// PURPOSE:		Implements the CountFilter object that will be used
//				to filter result sets for ObjectManager::ListIds()
//************************************************************************

#include "CountFilter.h"


//************************************************************************
// CountFilter:
//
// PURPOSE:		The default constructor
//************************************************************************

CountFilter::CountFilter( const ValueSet &valueSet )
			:Filter( valueSet ){

	m_currentCount	= m_maxCount	= 0;
}


//************************************************************************
// ~CountFilter:
//
// PURPOSE:		The destructor
//************************************************************************

CountFilter::~CountFilter(){
}


//************************************************************************
// DoesApply:
//
// PURPOSE:		Gives a chance to the filter to decide if a given valueset
//				applies to the filtered set
//************************************************************************

Filter::FILTER_RC 
CountFilter::DoesApply( ValueSet &valueSet ){

	if( m_currentCount < m_maxCount ){
		m_currentCount++;
		return APPLIES;
	}
	
	return NO_MORE_ITEMS_WILL_APPLY;
	
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
CountFilter::BuildYourselfFromAValueSet( ValueSet &valueSet ){
	
	if( !Filter::BuildYourselfFromAValueSet( valueSet ) )
		return false;

	m_currentCount	= 0;

	return valueSet.GetInt( SSAPI_COUNT_FILTER_MAX_COUNT, (int *)&m_maxCount )? true : false;
}

