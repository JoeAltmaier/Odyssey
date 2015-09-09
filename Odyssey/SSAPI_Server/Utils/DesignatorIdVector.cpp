//************************************************************************
// FILE:		DesignatorIdVector.cpp
//
// PURPOSE:		The file implements an ADT singly linked link list.
//
// NOTES:	
//************************************************************************


#include "DesignatorIdVector.h"
#ifdef _DEBUG
#include "ManagedObject.h"
#endif


//************************************************************************
// DesignatorIdVector:
//
// PURPOSE:		Default constructor
//************************************************************************

DesignatorIdVector::DesignatorIdVector(	U32 initialSize, 
										U32 sizeIncrement ) {

	m_count				= 0;
	m_size				= initialSize;
	m_sizeIncrement		= sizeIncrement;
	m_pBag				= new VECTOR_CELL*[ m_size ];

}


//************************************************************************
// ~DesignatorIdVector:
//
// PURPOSE:		The destructor. Removes all elements from the list
//************************************************************************

DesignatorIdVector::~DesignatorIdVector(){

	U32 i;
	
	for( i = 0; i < m_count; i++ )
		delete m_pBag[i];

	delete [] m_pBag;
	m_count = m_size = 0;
}


//************************************************************************
// Add:
//
// PURPOSE:		The method adds an element to the list. The caller
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
DesignatorIdVector::Add( ManagedObject* element, const DesignatorId &key ){

	if( m_count == m_size )
		Grow( m_sizeIncrement );
	
	m_pBag[ m_count ] = new VECTOR_CELL( element, (DesignatorId)key );
	m_count++;
	return TRUE;
}


//************************************************************************
// Set:
//
// PURPOSE:		Sets new value at the position where the current element 
//				has the 'key' specified. The caller is responsible for
//				all memory clean-up related to the old element
//************************************************************************

BOOL 
DesignatorIdVector::Set( ManagedObject *pElement, const DesignatorId &key ){

	U32	i;

	for( i = 0; i < m_count; i++ ){
		if( m_pBag[i]->key == key ){
			m_pBag[i]->element = pElement;
			return TRUE;
		}
	}
	return FALSE;
}


//************************************************************************
// Remove:		
//
// PURPOSE:		The method removes an element from the list. Lookup's
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
DesignatorIdVector::Remove( const DesignatorId  &key ){
	
	for( U32 i = 0; i < m_count; i++ )
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
DesignatorIdVector::RemoveAt( U32 position ){
	
	if( position >= m_count )
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
DesignatorIdVector::Get( ManagedObject* &element, const DesignatorId &key ){
	
	for( U32 i = 0; i < m_count; i++ )
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
// PURPOSE:		Retrieves element from the list at the position 
//				specified
//
// RECEIVE:		position	-	an absolute position in the list
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
DesignatorIdVector::GetAt( ManagedObject* &element, U32 position ){
	
	if( position >= m_count ){
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
DesignatorIdVector::Grow( U32 entriesToGrow ){

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
DesignatorIdVector::RemoveEntry( U32 position ){

	delete m_pBag[ position ];

	if( m_size - m_count == 1 ){
		VECTOR_CELL	**pTemp = new VECTOR_CELL*[ m_count - 1 ];
		memcpy( pTemp, m_pBag, sizeof(VECTOR_CELL*) * position);
		memcpy( pTemp + position, m_pBag + position + 1, sizeof(VECTOR_CELL*) * m_count - position - 1);
		delete [] m_pBag;
		m_pBag = pTemp;
		m_size -= m_sizeIncrement;
	}
	else{
		U32			temp = m_count - position - 1;
		VECTOR_CELL	**pTemp = new VECTOR_CELL*[ temp ];
		memcpy( pTemp, m_pBag + position + 1, sizeof(VECTOR_CELL*) * temp );
		memcpy( m_pBag + position, pTemp, sizeof(VECTOR_CELL*) *temp );
		delete [] pTemp;
	}
	
	m_count--;
}

#if 0
//************************************************************************
// DesignatorIdVector:
//
// PURPOSE:		Default constructor
//************************************************************************

DesignatorIdVector::DesignatorIdVector() {

	pHead = NULL;
	count = 0;
}


//************************************************************************
// DesignatorIdVector:
// 
// PURPOSE:		Copy constructor. Creates a copy of the object specified
//************************************************************************

DesignatorIdVector::DesignatorIdVector( const DesignatorIdVector  &original ) {


}


//************************************************************************
// ~DesignatorIdVector:
//
// PURPOSE:		The destructor. Removes all elements from the list
//************************************************************************

DesignatorIdVector::~DesignatorIdVector(){

	U32		size = count;

	for( U32 index = 0; index < size; index++ )
		RemoveAt( 0 );
}


//************************************************************************
// Add:
//
// PURPOSE:		The method adds an element to the list. The caller
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
DesignatorIdVector::Add( ManagedObject* element, DesignatorId &key ){
	
	DesignatorIdCell		*pNewCell;

	pNewCell	=	new DesignatorIdCell( element, key );
	if( pNewCell )
		return AddAt( pNewCell, 0 );
	else
		return FALSE;
}


//************************************************************************
// Set:
//
// PURPOSE:		Sets new value at the position where the current element 
//				has the 'key' specified. The caller is responsible for
//				all memory clean-up related to the old element
//************************************************************************

BOOL 
DesignatorIdVector::Set( ManagedObject *pElement, DesignatorId &key ){

	DesignatorIdCell *pPrev = GetPtrTo1CellBeforeTheElement( BY_KEY, (void *)&key );

	if( count && pHead->key == key ){
		pHead->element = pElement;
		return TRUE;
	}

	if( !pPrev )
		return FALSE;
	
#ifdef _DEBUG
	pElement->CheckIfValid();
#endif

	pPrev->pNext->element = pElement;

	return TRUE;
}


//************************************************************************
// Remove:		
//
// PURPOSE:		The method removes an element from the list. Lookup's
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
DesignatorIdVector::Remove( DesignatorId  &key ){
	
	DesignatorIdCell		*pPrev, *pDeletedElement;

	if( !count ){
		return FALSE;
	}
	else if( pHead->key == key ){
		pDeletedElement = pHead;
		pHead = pHead->pNext;
		delete pDeletedElement;
	}
	else if( count == 1 ){
		if( pHead->key == key ){
			delete pHead;
			pHead = NULL;
		}
		else {
			return FALSE;
		}
	}
	else {
		pPrev = GetPtrTo1CellBeforeTheElement( BY_KEY, (void *)&key );
		if( !pPrev )
			return FALSE;

		pDeletedElement = pPrev->pNext;
		pPrev->pNext = pDeletedElement->pNext;

		delete pDeletedElement;
	}

	count--;

	return TRUE;
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
DesignatorIdVector::RemoveAt( U32 position ){
	
	DesignatorIdCell		*pPrev, *pDeletedElement;

	if( position >= count ){
		return FALSE;
	}
	else if( position == 0 ){
		pDeletedElement = pHead;
		pHead = pHead->pNext;
		delete pDeletedElement;
	}
	else {
		pPrev = GetPtrTo1CellBeforeTheElement( BY_POSITION, (void *)&position );
		if( !pPrev )
			return FALSE;

		pDeletedElement = pPrev->pNext;
		pPrev->pNext = pDeletedElement->pNext;

		delete pDeletedElement;
	}

	count--;

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
DesignatorIdVector::Get( ManagedObject* &element, DesignatorId &key ){
	

	DesignatorIdCell		*pPrev;

	element = NULL;

	if( !count ){
		return FALSE;
	}
	else if( pHead->key == key ){
		element = pHead->element;
	}
	else if( count == 1 ){
		if( pHead->key == key ){
			element = pHead->element;
		}
		else {
			return FALSE;
		}
	}
	else {
		pPrev = GetPtrTo1CellBeforeTheElement( BY_KEY, (void *)&key );
		if( !pPrev )
			return FALSE;

		element = pPrev->pNext->element;
	}

	return TRUE;
}


//************************************************************************
// GetAt:
//
// PURPOSE:		Retrieves element from the list at the position 
//				specified
//
// RECEIVE:		position	-	an absolute position in the list
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
DesignatorIdVector::GetAt( ManagedObject* &element, U32 position ){
	

	DesignatorIdCell		*pPrev;

	element = NULL;

	if( count <= position ){
		return FALSE;
	}
	else if( position == 0 ){
		element = pHead->element;
	}
	else {
		pPrev = GetPtrTo1CellBeforeTheElement( BY_POSITION, (void *)&position );
		if( !pPrev )
			return FALSE;

		element = pPrev->pNext->element;
	}

	return TRUE;
}


//************************************************************************
// GetPtrTo1CellBeforeTheElement
//
// PURPOSE:		The method traverses the list and returns a pointer to
//				the element that immideatly precedes the element of our
//				interest
//	
// RECEIVE:		method				-	one of the lookup methods
//				pMethodDependentKey	-	ptr to a method-dependent key.
//										(for ex., we lookup by position, this
//										would point to the position).
//
// RETURN:		On success - ptr to the element
//				On failure - NULL
//************************************************************************

DesignatorIdCell*
DesignatorIdVector::GetPtrTo1CellBeforeTheElement(	LOOKUP_METHOD method, 
										void*         pMethodDependentKey ){

	DesignatorIdCell			*pTemp;

	if( count < 2 )
		return NULL;

	switch( method ){
		case BY_KEY:
			{
				DesignatorId		key = *((DesignatorId *)pMethodDependentKey);

				for( pTemp = pHead; pTemp->pNext; pTemp = pTemp->pNext ){
					if( key == pTemp->pNext->key )
						return pTemp;
				}

				return NULL;
			}

		case BY_POSITION:
			{
				U32			position = *((U32 *)pMethodDependentKey), index;

				for( pTemp = pHead, index = 1; pTemp->pNext && index < position; pTemp = pTemp->pNext, index++ )
					;
				return pTemp;
			}

		default:
			return NULL;
	}

	return NULL;
}


//************************************************************************
// AddAt:
//
// PURPOSE:		The method inserts a cell pointed by 'pNewCell' in to the
//				list.
//
// RECEIVE:		pNewCell	- ptr to new cell. Memory freed on "Remove()"
//				position	- position to insert at
//
// RETURN:		TRUE	-	success
//				FALSE	-	failure
//
// NOTES:		1. The function bumps up the counter on success
//************************************************************************

BOOL
DesignatorIdVector::AddAt( DesignatorIdCell *pNewCell, U32 position ){

#ifdef _DEBUG
	pNewCell->element->CheckIfValid();
#endif

	if( position > count )
		return FALSE;

	if( position == 0 ){
		pNewCell->pNext = pHead;
		pHead = pNewCell;
	}
	else if( count == 1){
		DesignatorIdCell *pTemp = pHead;

		pNewCell->pNext = pTemp->pNext;
		pTemp->pNext = pNewCell;
	}
	else {
		DesignatorIdCell *pTemp = GetPtrTo1CellBeforeTheElement( BY_POSITION, (void *)&position );

		pNewCell->pNext = pTemp->pNext;
		pTemp->pNext = pNewCell;
	}

	count++;

	return TRUE;
}
#endif