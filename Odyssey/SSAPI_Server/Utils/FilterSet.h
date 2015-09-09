//************************************************************************
// FILE:		FilterSet.h
//
// PURPOSE:		Defines the class FilterSet which wraps up all the 
//				functionality necessary to create filters and get a 
//				filtered set of results by pipelining results from a 
//				previous filter to the following one.
//************************************************************************

#ifndef __SSAPI_FILTER_SET_H__
#define __SSAPI_FILTER_SET_H__

#include "DesignatorIdVector.h"
#include "CoolVector.h"
class Filter;
class ValueSet;


#ifdef WIN32
#pragma pack(4)
#endif



class FilterSet {

	Container		*m_pFilters;
	bool			m_isProperlyInited;


public:

//************************************************************************
// FilterSet:
//
// PURPOSE:		The default constructor.
//
// RECEIVE:		valueSet:	the value set that contains value sets for all
//							necessary filters
//************************************************************************

FilterSet( const ValueSet &valueSet );


//************************************************************************
// FilterSet:
//
// PURPOSE:		The destructor
//************************************************************************

~FilterSet();


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

bool BuildFilteredSet(	const DesignatorIdVector&	objectsToFilter, 
						DesignatorIdVector&			resultSet );


//************************************************************************
// IsProperlyInited:
//
// PURPOSE:		An accessor. Returns the flag that indicates if all filters
//				were able to initalize themselves and are ready for work.
//************************************************************************

bool IsProperlyInited() const { return m_isProperlyInited; }


//************************************************************************
// GetFilterCount:
//************************************************************************

U32 GetFilterCount() const { return m_pFilters->Count(); }


//************************************************************************
//************************************************************************

const Filter& GetFilterAt( U32 position ) const { 
	Filter *p; 
	
	m_pFilters->GetAt( (CONTAINER_ELEMENT &)p, position );
	return *p;
}


private:




};

#endif  // __SSAPI_FILTER_SET_H__