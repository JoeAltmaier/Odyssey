//************************************************************************
// FILE:		IntVectorFilter.h
//
// PURPOSE:		Defines the IntVectorFilter object that will be used
//				to filter result sets for ObjectManager::ListIds()
//************************************************************************

#ifndef __INT_VECTOR_FILTER_H__
#define	__INT_VECTOR_FILTER_H__

#include "SSAPIFilter.h"
#include "SSAPITypes.h"

class Container;
class Comparator;

#ifdef WIN32
#pragma pack(4)
#endif


class IntVectorFilter : public Filter{

	Container		*m_pValueVector;		// contains pointers to I64
	int				m_fieldId;
	Comparator		*m_pComparator;

protected:

//************************************************************************
// IntVectorFilter:
//
// PURPOSE:		The default constructor
//************************************************************************

IntVectorFilter( const ValueSet &valueSet );


public:


//************************************************************************
// ~IntVectorFilter:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~IntVectorFilter();


//************************************************************************
// Ctor:
//
// PURPOSE:		A means of dynamic, self-contained type creation.
//				new()s and returns an instance
//************************************************************************

static Filter* Ctor( const ValueSet &valueSet ){ 
	return new IntVectorFilter( valueSet ); 
}


//************************************************************************
// DoesApply:
//
// PURPOSE:		Gives a chance to the filter to decide if a given valueset
//				applies to the filtered set
//************************************************************************

virtual Filter::FILTER_RC DoesApply( ValueSet &valueSet );


//************************************************************************
// Accessors:
//
//************************************************************************

int GetFieldId() const { return m_fieldId; }

I64 GetNextApplicableNumber() const;


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
// PURPOSE:		Deallocates memory taken for elements in thw m_valueVector
//************************************************************************

void ClearValueVector();

};



#endif	// __SSAPI_INT_VECTOR_FILTER_H__