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

#ifndef __SSAPI_FILTER_H__
#define	__SSAPI_FILTER_H__


#include "ValueSet.h"
#include "SSAPITypes.h"


class Comparator;

#ifdef WIN32
#pragma pack(4)
#endif




class Filter{

public:

	

	enum FILTER_RC{
		DOES_NOT_APPLY	= 0,
		APPLIES,
		NO_MORE_ITEMS_WILL_APPLY		// for count and page filter
	};

protected:

	U32					m_filterClassType;



//************************************************************************
// Filter:
//
// PURPOSE:		The default constructor
//************************************************************************

Filter( const ValueSet &valueSet );


public:


//************************************************************************
// ~Filter:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~Filter();


//************************************************************************
// Ctor:
//
// PURPOSE:		A means of dynamic, self-contained type creation.
//				That's an only way to create a filter object. The methods
//				will decide which derived class will be created or return
//				NUL if such filter type is not supported
//************************************************************************

static Filter* Ctor( const ValueSet &valueSet );


//************************************************************************
// DoesApply:
//
// PURPOSE:		Gives a chance to the filter to decide if a given valueset
//				applies to the filtered set
//************************************************************************

virtual FILTER_RC DoesApply( ValueSet &valueSet ) = 0;


//************************************************************************
// GetClassType:
//************************************************************************

U32 GetClassType() const { return m_filterClassType; }


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


//************************************************************************
// struct FilterDescriptorCell:
//
// PURPOSE:		Defines a cell for the table that will be used to enable 
//				EZ injection of new filter objects. (co-o-o-o-o-o-ol, man!!! :-)
//************************************************************************

typedef	Filter* (*FILTER_CTOR)( const ValueSet &valueSet );

struct FilterDescriptorCell{
	int				filterClassType;		// SSAPI_Codes.h, -1 if the last cell
	FILTER_CTOR		pCtor;
};


#endif	// __SSAPI_FILTER_H__