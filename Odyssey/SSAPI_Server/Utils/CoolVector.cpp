//************************************************************************
// FILE:		Vector.cpp
//
// PURPOSE:		The file implements the Vector object that implements Container
//				interface and is a dynamically growing/contracting array
//				with access time == 1 
//
// NOTES:	
//************************************************************************


#include "CoolVector.h"
#include <string.h>


//************************************************************************
// CoolVector:
//
// PURPOSE:		Default constructor
//************************************************************************

CoolVector::CoolVector( U32 initialSize, U32 sizeIncrement ){

	count				= 0;
	m_size				= initialSize;
	m_sizeIncrement		= sizeIncrement;
	m_pBag				= new VECTOR_CELL*[ m_size ];
}


//************************************************************************
// ~CoolVector:
//
// PURPOSE:		The destructor
//************************************************************************

CoolVector::~CoolVector(){

	U32 i;
	
	for( i = 0; i < count; i++ )
		delete m_pBag[i];

	delete [] m_pBag;
	count = m_size = 0;
}

//************************************************************************
// Add:
//
// PURPOSE:		The method adds an element to the container. The caller
//				needs to submit a key which should be unique.It's fine for 
//				a key to not be unique; however, all retrievals and deletions
//				will work on the first element found that has the key...
//				The key may be omitted at all. Then, however, the element
//				may only be deleted/retrieved by its position.
//
// RECEIVE:		element		-	a 4-byte value to store
//				key			-	the key
//
// RETURN:		TRUE  - success
//				FALSE - failure
//************************************************************************


BOOL 
CoolVector::Add( CONTAINER_ELEMENT element, CONTAINER_KEY key ){

	if( count == m_size )
		Grow( m_sizeIncrement );
	
	m_pBag[ count ] = new VECTOR_CELL( element, key );
	count++;
	return TRUE;
}


//************************************************************************
// AddAt:
//
// PURPOSE:		The method adds an element to the container. The caller
//				must specify the position of the element in the container.
//				
// RECEIVE:		element		-	a 4-byte value to store
//				position	-	the absolute position ( 0 - based )
//
// NOTES:		1. If position exceeds the count, the method fails!
//				2. The position may change as other elements added/deleted
//
// RETURN:		TRUE  - success
//				FALSE - failure
//************************************************************************

BOOL 
CoolVector::AddAt( CONTAINER_ELEMENT element, U32 position ){

	if( position > count  )
		return FALSE;

	if( count == m_size  )
		Grow( m_sizeIncrement );
	
	if( position != count ){
		// need to shuffle pts up
		U32			temp = count - position;
		VECTOR_CELL	**pTemp = new VECTOR_CELL* [ temp ];
		memcpy( pTemp, m_pBag + position, sizeof(VECTOR_CELL*) * temp );
		memcpy( m_pBag + position + 1, pTemp, sizeof(VECTOR_CELL*) * temp );
		delete [] pTemp;
	}

	m_pBag[ position ] = new VECTOR_CELL( element );
	count++;
	return TRUE;
}


//************************************************************************
// Remove:		
//
// PURPOSE:		The method removes an element from the container. Lookup's
//				is done by the key.
//
// RECEIVE:		key	-	key to search for
//
// NOTES:		1. The function fails if the element is not found
//				2. If the key isn't unique, only one element is deleted
//
// RETURN:		TRUE  - success
//				FALSE - failure
//************************************************************************

BOOL 
CoolVector::Remove( CONTAINER_KEY  key ){

	for( U32 i = 0; i < count; i++ )
		if( m_pBag[i]->key == key ){
			RemoveEntry( i );
			return TRUE;
		}

	return FALSE;
}


//************************************************************************
// RemoveAt:
//
// PURPOSE:		The method removes the element at the position specified
//
// RECEIVE:		position - absolute position of the element to remove
//						   (0 - based)
//
// NOTES:		1. The function fails if the position is out of range
//				2. If the key isn't uniqe, only one element is deleted
//
// RETURN:		TRUE  - success
//				FALSE - failure
//************************************************************************

BOOL 
CoolVector::RemoveAt( U32 position ){

	if( position >= count )
		return FALSE;

	RemoveEntry( position );
	return TRUE;
}


//************************************************************************
// Get:
//
// PURPOSE:		The method retrieves an element with the key specified
//
// RECEIVE:		key		-	the key to search for
//				
// OUTPUT:		On success, 'element' will hold the retrieved value
//
// NOTES:		1. The function fails if the element is not found
//				2. If the key isn't unique, a first element is retieved
//
// RETURN:		TRUE  - success
//				FALSE - failure
//************************************************************************

BOOL 
CoolVector::Get( CONTAINER_ELEMENT &element, CONTAINER_KEY key ){

	for( U32 i = 0; i < count; i++ )
		if( m_pBag[i]->key == key ){
			element = m_pBag[i]->element;
			return TRUE;
		}

	element = 0;
	return FALSE;
}


//************************************************************************
// GetAt:
//
// PURPOSE:		Retrieves element from the container at the position 
//				specified
//
// RECEIVE:		position	-	an absolute position in the container
//								(0-based)
//
// OUTPUT:		On success, 'element' will hold the retrieved value
//
// NOTES:		1. The function fails if the position is out of range.
//
// RETURN:		TRUE  - success
//				FALSE - failure
//************************************************************************

BOOL 
CoolVector::GetAt( CONTAINER_ELEMENT &element, U32 position ){

	if( position >= count ){
		element = 0;
		return FALSE;
	}

	element = m_pBag[position]->element;
	return TRUE;
}



//************************************************************************
// Grow:
//
// PURPOSE:		Grows the vector by the number of entries specified
//				The new entries are appended to the end of the vector
//************************************************************************

void 
CoolVector::Grow( U32 entriesToGrow ){

	VECTOR_CELL	**pTemp = new VECTOR_CELL*[ m_size + entriesToGrow ];
	memcpy( pTemp, m_pBag, sizeof(VECTOR_CELL*) * m_size);
	delete [] m_pBag;
	m_pBag = pTemp;
	m_size += entriesToGrow;
}


//************************************************************************
// Contract:
//
// PURPOSE:		Decreases the size of the vector by one entry by removing
//				the entry at the position specified and moving all
//				entries above it down by one.
//************************************************************************

void 
CoolVector::RemoveEntry( U32 position ){

	delete m_pBag[ position ];

	if( m_size - m_sizeIncrement == count - 1 ){
		// we need to descrease the size of the underlying pointer array
		VECTOR_CELL	**pTemp = new VECTOR_CELL*[ count - 1 ];
		memcpy( pTemp, m_pBag, sizeof(VECTOR_CELL*) * position);
		memcpy( pTemp + position, m_pBag + position + 1, sizeof(VECTOR_CELL*) * (count - position - 1));
		delete [] m_pBag;
		m_pBag = pTemp;
		m_size -= m_sizeIncrement;
	}
	else{
		// simply move entries to left from the point of the "position"
		U32			temp = count - position - 1;
		VECTOR_CELL	**pTemp = new VECTOR_CELL*[ temp ];
		memcpy( pTemp, m_pBag + position + 1, sizeof(VECTOR_CELL*) * temp );
		memcpy( m_pBag + position, pTemp, sizeof(VECTOR_CELL*) *temp );
		delete [] pTemp;
	}
	
	count--;
}


