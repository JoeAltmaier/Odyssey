//************************************************************************
// FILE:		FloatVectorFilter.cpp
//
// PURPOSE:		Implements the IntVectorFilter object that will be used
//				to filter result sets for ObjectManager::ListIds()
//************************************************************************

#include "FloatVectorFilter.h"
#include "CoolVector.h"
#include "Comparator.h"


//************************************************************************
// FloatVectorFilter:
//
// PURPOSE:		The default constructor
//************************************************************************

FloatVectorFilter::FloatVectorFilter( const ValueSet &valueSet )
				:Filter( valueSet ){

	m_pComparator		= NULL;
	m_pValueVector		= new CoolVector;
	m_fieldId			= -1;
}


//************************************************************************
// ~FloatVectorFilter:
//
// PURPOSE:		The destructor
//************************************************************************

FloatVectorFilter::~FloatVectorFilter(){

	delete m_pComparator;

	ClearValueVector();
	delete m_pValueVector;
}


//************************************************************************
// DoesApply:
//
// PURPOSE:		Gives a chance to the filter to decide if a given valueset
//				applies to the filtered set
//************************************************************************

Filter::FILTER_RC 
FloatVectorFilter::DoesApply( ValueSet &valueSet ){

	U32			index;
	float		*pValue;
	ValueSet	*pVector;

	if( !m_pValueVector )
		return NO_MORE_ITEMS_WILL_APPLY;

	pVector = (ValueSet *)valueSet.GetValue( m_fieldId );
	if( !pVector->GetCount() && !m_pValueVector->Count() )
		return APPLIES;

	for( index = 0; index < m_pValueVector->Count(); index++ ){
		if( !m_pValueVector->GetAt( (CONTAINER_ELEMENT &)pValue, index ) )
			return DOES_NOT_APPLY;

		if( m_pComparator->DoesApply( &valueSet, m_fieldId, *pValue ) )
			return APPLIES;
	}

	return DOES_NOT_APPLY;
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
FloatVectorFilter::BuildYourselfFromAValueSet( ValueSet &valueSet ){

	int			comparatorType, index;
	ValueSet	*pValueVector;
	bool		rc;
	float		*pValue, value;

	rc = Filter::BuildYourselfFromAValueSet( valueSet );

	ClearValueVector();
	delete m_pComparator;

	if( !valueSet.GetInt( SSAPI_INT_FLOAT_VECTOR_FILTER_FID_FIELD_ID, &m_fieldId ) )
		return false;

	if( !valueSet.GetInt( SSAPI_INT_FLOAT_VECTOR_FILTER_FID_COMPARATOR_TYPE, &comparatorType ) )
		return false;

	m_pComparator = new Comparator( (Comparator::COMPARATOR_TYPE)comparatorType );

	pValueVector	= (ValueSet *)valueSet.GetValue( SSAPI_INT_FLOAT_VECTOR_FILTER_FID_VALUE_VECTOR );

	if( !pValueVector )
		return false;

	for( index = 0; index < pValueVector->GetCount(); index++ ){
		if( !pValueVector->GetFloat( index, &value ) )
			return false;
		pValue = new float; *pValue = value;
		m_pValueVector->AddAt( (CONTAINER_ELEMENT)pValue, index );
	}

	return rc;
}


//************************************************************************
// ClearValueVector:
//
// PURPOSE:		Deallocates memory taken for elements in thw m_valueVector
//************************************************************************

void 
FloatVectorFilter::ClearValueVector(){
	float			*pTemp;

	while( m_pValueVector->Count() ){
		m_pValueVector->GetAt( (CONTAINER_ELEMENT &)pTemp, 0 );
		m_pValueVector->RemoveAt( 0 );
		delete pTemp;
	}
}
