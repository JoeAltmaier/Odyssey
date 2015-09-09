//******************************************************************************
// FILE:		ClassTypeMap.h
//
// PURPOSE:		Defines the class that wraps up the knowledge of our class
//				diagram in terms of who is derived from whom by using 
//				class type SSAPI codes.
//
// NOTE:		Every class that is derived from will only have ids of its 
//				immediate sub-classes. Recursion is used to reach indirectly
//				dervied subclasses.
//******************************************************************************

#ifndef	__CLASS_TYPE_MAP_H__
#define	__CLASS_TYPE_MAP_H__

#include "SSAPITypes.h"
#include "SSAPI_Codes.h"
#include "CtTypes.h"

class Container;
struct ClassTypeMapCell;

#ifdef WIN32
#pragma pack(4)
#endif

#define	SSAPI_MAX_DIRECTLY_DERIVED_OBJECTS			24

class ClassTypeMap{

public:

//******************************************************************************
// ClassTypeMap:
//
// PURPOSE:		The default constructor
//******************************************************************************

ClassTypeMap();


//******************************************************************************
// ~ClassTypeMap:
//
// PURPOSE:		The destructor
//******************************************************************************

~ClassTypeMap();


//******************************************************************************
// IsADerivedClass:
//
// PURPOSE:		Determines if a given object is derived (maybe indirectly)
//				from another given object
//
// FUNCTIONALITY:	The method uses recursive decent
//******************************************************************************

bool IsADerivedClass( U32 baseClassId, U32 classIdInQuestion, bool includeDerivations );

};


//******************************************************************************
// struct ClassTypeMapCell
//
// PURPOSE:		Establishes a structure of a cell for the class type map
//******************************************************************************

struct ClassTypeMapCell{
	char		*pClassName;
	U32			classTypeId;
	U32			derivedClassIds[SSAPI_MAX_DIRECTLY_DERIVED_OBJECTS];
};

#endif // __CLASS_TYPE_MAP_H__