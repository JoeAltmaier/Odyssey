//************************************************************************
// FILE:		Filter.h
//
// PURPOSE:		Defines a virtual base class Filter. Objects implementing
//				the Filter interface will be the filter objects we use
//				for filtering most of our ListIds() methods.
//				The class also implements the Factory Method design pattern
//				by supplying the method Ctor() capable of creating the right
//				filter object off a request value set.
//************************************************************************



#include "SSAPIFilter.h"
#include "IntVectorFilter.h"
#include "FloatVectorFilter.h"
#include "DesignatorIdVectorFilter.h"
#include "ObjectClassTypeFilter.h"
#include "CountFilter.h"
#include "SSAPIAssert.h"


//*************************************
// Add your new filter objects here  |
// last entry must have a negative	 | 
// field id value					\ /	
//*************************************

static FilterDescriptorCell FilterDescriptorTable[] = {
	{
		SSAPI_FILTER_CLASS_TYPE_INT_VECTOR,
		(FILTER_CTOR)METHOD_ADDRESS( IntVectorFilter, Ctor ),
	},
	{
		SSAPI_FILTER_CLASS_TYPE_COUNT,
		(FILTER_CTOR)METHOD_ADDRESS( CountFilter, Ctor ),
	},
	{
		SSAPI_FILTER_CLASS_TYPE_FLOAT_VECTOR,
		(FILTER_CTOR)METHOD_ADDRESS( FloatVectorFilter, Ctor ),
	},
	{
		SSAPI_FILTER_CLASS_TYPE_DESIGNATOR_ID,
		(FILTER_CTOR)METHOD_ADDRESS( DesignatorIdVectorFilter, Ctor ),
	},
	{	
		SSAPI_FILTER_CLASS_TYPE_OBJECT_CLASS_TYPE,
		(FILTER_CTOR)METHOD_ADDRESS( ObjectClassTypeFilter, Ctor ),
	},
	{
		-1,
		(FILTER_CTOR)0,
	}
};


//************************************************************************
// Filter:
//
// PURPOSE:		The default constructor
//************************************************************************

Filter::Filter( const ValueSet &valueSet ){

}


//************************************************************************
// ~Filter:
//
// PURPOSE:		The destructor
//************************************************************************

Filter::~Filter(){
}


//************************************************************************
// Ctor:
//
// PURPOSE:		A means of dynamic, self-contained type creation.
//				That's an only way to create a filter object. The methods
//				will decide which derived class will be created or return
//				NUL if such filter type is not supported
//************************************************************************

Filter* 
Filter::Ctor( const ValueSet &valueSet ){

	FilterDescriptorCell	*pCell = FilterDescriptorTable;
	int						filterClassType;

	if( !((ValueSet &)valueSet).GetInt( SSAPI_FILTER_FID_FILTER_CLASS_TYPE, &filterClassType ) ){
		ASSERT(0);
		return NULL;
	}

	for( ; pCell->filterClassType >= 0; pCell++ ){
		if( pCell->filterClassType == filterClassType ){
			Filter *pFilter = (*pCell->pCtor)( valueSet );
			if( pFilter->BuildYourselfFromAValueSet( (ValueSet &)valueSet ) )
				return pFilter;
			else
				return NULL;
		}
	}

	return NULL;
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
Filter::BuildYourselfFromAValueSet( ValueSet &valueSet ){

	return valueSet.GetInt( SSAPI_FILTER_FID_FILTER_CLASS_TYPE, (int *)&m_filterClassType )? true : false;
}




