//************************************************************************
// FILE:		CountFilter.h
//
// PURPOSE:		Defines the CountFilter object that will be used
//				to filter result sets for ObjectManager::ListIds()
//************************************************************************

#ifndef __COUNT_FILTER_H__
#define __COUNT_FILTER_H__

#include "SSAPIFilter.h"
#include "SSAPITypes.h"


#ifdef WIN32
#pragma pack(4)
#endif


class CountFilter : public Filter{
		
	U32				m_maxCount;
	U32				m_currentCount;

protected:

//************************************************************************
// CountFilter:
//
// PURPOSE:		The default constructor
//************************************************************************

CountFilter( const ValueSet &valueSet );


public:


//************************************************************************
// ~CountFilter:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~CountFilter();


//************************************************************************
// Ctor:
//
// PURPOSE:		A means of dynamic, self-contained type creation.
//				new()s and returns an instance
//************************************************************************

static Filter* Ctor( const ValueSet &valueSet ){ 
	return new CountFilter( valueSet ); 
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



#endif	// __COUNT_FILTER_H__