//************************************************************************
// FILE:		Comparator.h
//
// PURPOSE:		The file defines the Comparator object that will encapsulate
//				behaviour necessary to compare values of different types
//************************************************************************

#ifndef __SSAPI_COMPARATOR_H__
#define	__SSAPI_COMPARATOR_H__

#include "ValueSet.h"
#include "SSAPITypes.h"
#include "UnicodeString.h"
#include "DesignatorId.h"
#include "SSAPI_Codes.h"


#ifdef WIN32
#pragma pack(4)
#endif


class Comparator{

public:

	enum COMPARATOR_TYPE{
		INVALID					=	0,
		LESS_THAN				=	SSAPI_COMPARATOR_LESS_THAN,
		LESS_THAN_OR_EQUAL		=	SSAPI_COMPARATOR_LESS_THAN_OR_EQUAL,
		GREATER_THAN			=	SSAPI_COMPARATOR_GREATER_THAN,
		GREATER_THAN_OR_EQUAL	=	SSAPI_COMPARATOR_GREATER_THAN_OR_EQUAL,
		EQUAL					=	SSAPI_COMPARATOR_EQUAL,
		NOT_EQUAL				=	SSAPI_COMPARATOR_NOT_EQUAL
	};


private:

	COMPARATOR_TYPE		m_comparatorType;


public:

//************************************************************************
// Comparator:
//
// PURPOSE:		The default constructor
//************************************************************************

Comparator( COMPARATOR_TYPE comparatorType );


//************************************************************************
// ~Comparator:
//
// PURPOSE:		The destructor
//************************************************************************

~Comparator();


//************************************************************************
// DoesApply:
//
// PURPOSE:		Checks if a given value would apply to the value in the
//				value set with a given field id for this comparator
//
// RETURN:		true:		the value set applies to this value 
//				false:		it doesn't
//************************************************************************

bool DoesApply( ValueSet *pObj, U32 fieldId, I64 value );

bool DoesApply( ValueSet *pObj, U32 fieldId, float value );

bool DoesApply( ValueSet *pObj, U32 fieldId, UnicodeString value );

bool DoesApply( ValueSet *pObj, U32 fieldId, DesignatorId value );


//************************************************************************
// Hacky
//************************************************************************

I64 GetNextApplicableInteger( I64 base );

};

#endif	// __SSAPI_COMPARATOR_H__