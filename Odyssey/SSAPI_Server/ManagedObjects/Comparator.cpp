//************************************************************************
// FILE:		Comparator.cpp
//
// PURPOSE:		The file implements the Comparator object that will encapsulate
//				behaviour necessary to compare values of different types
//************************************************************************

#include "Comparator.h"
#include "SSAPIAssert.h"



//************************************************************************
// Comparator:
//
// PURPOSE:		The default constructor
//************************************************************************

Comparator::Comparator( COMPARATOR_TYPE comparatorType ){

	if( comparatorType == INVALID ){
		ASSERT(0);
		return;
	}

	m_comparatorType	=	comparatorType;
}


//************************************************************************
// ~Comparator:
//
// PURPOSE:		The destructor
//************************************************************************

Comparator::~Comparator(){
}


//************************************************************************
// DoesApply:
//
// PURPOSE:		Checks if a given value would apply to the value in the
//				value set with a given field id for this comparator
//
// RETURN:		true:		the value set applies to this value 
//				false:		it doesn't
//************************************************************************

bool 
Comparator::DoesApply( ValueSet *pObj, U32 fieldId, I64 value ){
	
	I64					temp;
	ValueSet			*pVs;
	int					i;
	

	// first check if this a value or a vector of values
	if( !pObj->GetInt64( fieldId, &temp ) ){
		if( !pObj->GetInt( fieldId, (int *)&i ) ){
			if( !( pVs = (ValueSet *) pObj->GetValue( fieldId ) ) )
				return false;

			// it's a vector, process a value at a time recursively
			bool doesApply = false;
			for( int i = 0; i < pVs->GetCount(); i++ ) {
				if( (doesApply = DoesApply( pVs, i, value )) == true )
					break;
			}
			return doesApply;
		}
		else
			temp = i;
	}

	switch( m_comparatorType ){
		case LESS_THAN:
			return temp < value;

		case LESS_THAN_OR_EQUAL:
			return temp <= value;

		case GREATER_THAN:
			return temp > value;

		case GREATER_THAN_OR_EQUAL:
			return temp >= value;

		case EQUAL:
			return temp == value;

		case NOT_EQUAL:
			return temp != value;

		default:
			ASSERT(0);
			return false;
	}
	return true;
}


//************************************************************************
//
//************************************************************************

bool 
Comparator::DoesApply( ValueSet *pObj, U32 fieldId, float value ){

	float				temp;
	ValueSet			*pVs;

	// first check if this a value or a vector of values
	if( !pObj->GetFloat( fieldId, &temp ) ){
		if( !( pVs = (ValueSet *) pObj->GetValue( fieldId ) ) )
			return false;

		// it's a vector, process a value at a time recursively
		bool doesApply = false;
		for( int i = 0; i < pVs->GetCount(); i++ ) {
			if( (doesApply = DoesApply( pVs, i, value )) == true )
				break;
		}
		return doesApply;
	}
	else {
		switch( m_comparatorType ){
			case LESS_THAN:
				return temp < value;

			case LESS_THAN_OR_EQUAL:
				return temp <= value;

			case GREATER_THAN:
				return temp > value;

			case GREATER_THAN_OR_EQUAL:
				return temp >= value;

			case EQUAL:
				return temp == value;

			case NOT_EQUAL:
				return temp != value;

			default:
				ASSERT(0);
				return false;
		}
	}
	return true;
}


//************************************************************************
//
//************************************************************************

bool 
Comparator::DoesApply( ValueSet *pObj, U32 fieldId, UnicodeString value ){

	UnicodeString		temp;
	ValueSet			*pVs;

	// first check if this a value or a vector of values
	if( !pObj->GetString( fieldId, &temp ) ){
		if( !( pVs = (ValueSet *) pObj->GetValue( fieldId ) ) )
			return false;

		// it's a vector, process a value at a time recursively
		bool doesApply = false;
		for( int i = 0; i < pVs->GetCount(); i++ ) {
			if( (doesApply = DoesApply( pVs, i, value )) == true )
				break;
		}
		return doesApply;
	}
	else {
		switch( m_comparatorType ){
			case LESS_THAN:
				return temp < value;

			case LESS_THAN_OR_EQUAL:
				return temp <= value;

			case GREATER_THAN:
				return temp > value;

			case GREATER_THAN_OR_EQUAL:
				return temp >= value;

			case EQUAL:
				return temp == value;

			case NOT_EQUAL:
				return !(temp == value);

			default:
				ASSERT(0);
				return false;
		}
	}
	return true;
}


//************************************************************************
//
//************************************************************************

bool 
Comparator::DoesApply( ValueSet *pObj, U32 fieldId, DesignatorId value ){

	DesignatorId		temp;
	ValueSet			*pVs;

	// first check if this a value or a vector of values
	if( !pObj->GetGenericValue( (char *)&temp, sizeof( temp ), fieldId ) ){
		if( !( pVs = (ValueSet *) pObj->GetValue( fieldId ) ) )
			return false;

		// it's a vector, process a value at a time recursively
		bool doesApply = false;
		for( int i = 0; i < pVs->GetCount(); i++ ) {
			if( (doesApply = DoesApply( pVs, i, value )) == true )
				break;
		}
		return doesApply;
	}
	else {
		switch( m_comparatorType ){
			case LESS_THAN:
			case LESS_THAN_OR_EQUAL:
			case GREATER_THAN:
			case GREATER_THAN_OR_EQUAL:
				return false;	

			case EQUAL:	
				return temp == value;

			case NOT_EQUAL:
				return (temp == value)? false : true;

			default:
				ASSERT(0);
				return false;
		}
	}

	return true;
}

//************************************************************************
// Hacky
//************************************************************************

I64 
Comparator::GetNextApplicableInteger( I64 base ){

	switch( m_comparatorType ){
			case LESS_THAN:
				return --base;

			case LESS_THAN_OR_EQUAL:
				return base;

			case GREATER_THAN:
				return ++base;

			case GREATER_THAN_OR_EQUAL:
				return base;

			case EQUAL:
				return base;

			default:
				ASSERT(0);
				return 0;
	}
	
	return 0;
}
