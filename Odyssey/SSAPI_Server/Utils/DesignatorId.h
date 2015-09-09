//************************************************************************
// FILE:		DesignatorId.h
//
// PURPOSE:		Defines class DesignatorId used as a hoock on every managed
//				object
//************************************************************************

#ifndef __DESIGNATOR_ID_H__
#define	__DESIGNATOR_ID_H__

#include "CtTypes.h"
#include "RowId.h"
#include "string.h"

#ifdef WIN32
#pragma pack(4)
#else
#pragma pack(8)
#endif



class DesignatorId{
	RowId		rowId;
	U16			uniqueId;
	U16			classId;

public:


//************************************************************************
// DesignatorId:
//
// PURPOSE:		Default constructor	
//************************************************************************

DesignatorId(){

	rowId			= RowId();
	uniqueId		= 0;
	classId			= 0;
}

//************************************************************************
// DesignatorId:
//
// PURPOSE:		A full constructor
//************************************************************************

DesignatorId( const RowId &rI , U16 cI, U16 uI = 0 ){
	rowId			= rI;
	uniqueId		= uI;
	classId			= cI;
}


//************************************************************************
// DesignatorId:
//
// PURPOSE:		A copy constructor
//************************************************************************

DesignatorId( const DesignatorId &obj ){
	// *this = obj;
	rowId		= obj.rowId;	
	uniqueId	= obj.uniqueId;
	classId		= obj.classId;
}


//************************************************************************
// operator ==
//
// PURPOSE:		Overloads operator==
//************************************************************************

bool operator ==( const DesignatorId &obj ){

	if( ( obj.rowId == rowId ) && ( obj.uniqueId == uniqueId ) && (obj.classId == classId ) ) 
		return true;
	else
		return false;
		
}


//************************************************************************
// operator !=
//
// PURPOSE:		Overloads operator!=
//************************************************************************

bool operator !=( const DesignatorId &obj ){

	return !(*this == obj);	
}


//************************************************************************
// operator=
//
// PURPOSE:		Overloads operator=
//
// RETURNS:		the input param
//************************************************************************

const DesignatorId& operator= ( const DesignatorId &obj ){

	rowId		= obj.rowId;	
	uniqueId	= obj.uniqueId;
	classId		= obj.classId;

	return obj;
}

//************************************************************************
// PrintYourself:
//
// PURPOSE:		debugging
//************************************************************************

void PrintYourself(){ 
	rowId.PrintYourself(); 
	printf("%d:%d", classId, uniqueId );
}

//************************************************************************
// Accessors
//
// PURPOSE:		Return copies of corresponding data members
//************************************************************************

RowId	GetRowId()		const { return rowId; }
U16		GetUniqueId()	const { return uniqueId; }
U16		GetClassId()	const { return classId; }

};


#endif	// __DESIGNATOR_ID_H__