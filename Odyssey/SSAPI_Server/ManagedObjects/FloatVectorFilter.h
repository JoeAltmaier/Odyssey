//************************************************************************
// FILE:		FloatVectorFilter.h
//
// PURPOSE:		Defines the FloatVectorFilter object that will be used
//				to filter result sets for ObjectManager::ListIds()
//************************************************************************

#ifndef __FLOAT_VECTOR_FILTER_H__
#define	__FLOAT_VECTOR_FILTER_H__

#include "SSAPIFilter.h"
#include "SSAPITypes.h"

class Container;
class Comparator;

#ifdef WIN32
#pragma pack(4)
#endif


class FloatVectorFilter : public Filter{

	Container		*m_pValueVector;		// contains pointers to float
	int				m_fieldId;
	Comparator		*m_pComparator;

protected:

//************************************************************************
// FloatVectorFilter:
//
// PURPOSE:		The default constructor
//************************************************************************

FloatVectorFilter( const ValueSet &valueSet );


public:


//************************************************************************
// ~FloatVectorFilter:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~FloatVectorFilter();


//************************************************************************
// Ctor:
//
// PURPOSE:		A means of dynamic, self-contained type creation.
//				new()s and returns an instance
//************************************************************************

static Filter* Ctor( const ValueSet &valueSet ){ 
	return new FloatVectorFilter( valueSet ); 
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


private:

//************************************************************************
// ClearValueVector:
//
// PURPOSE:		Deallocates memory taken for elements in the m_valueVector
//************************************************************************

void ClearValueVector();

};



#endif	// __SSAPI_FLOAT_VECTOR_FILTER_H__