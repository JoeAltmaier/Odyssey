//************************************************************************
// FILE:		SList.cpp
//
// PURPOSE:		The file implements an ADT singly linked link list.
//
// NOTES:	
//************************************************************************


#include "SList.h"


//************************************************************************
// SList:
//
// PURPOSE:		Default constructor
//************************************************************************

SList::SList() : Container() {

	pHead = NULL;
}


//************************************************************************
// SList:
// 
// PURPOSE:		Copy constructor. Creates a copy of the object specified
//************************************************************************

SList::SList( const SList  &original ) : Container( original ){

	CONTAINER_ELEMENT		element;

	pHead = NULL;

	for( U32 index = 0; index < original.count; index++ ){
		((SList) original).GetAt( element, index );
		AddAt( element, index );
	}
}


//************************************************************************
// ~SList:
//
// PURPOSE:		The destructor. Removes all elements from the list
//************************************************************************

SList::~SList(){

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
SList::Add( CONTAINER_ELEMENT element, CONTAINER_KEY key ){
	
	SListCell		*pNewCell;

	pNewCell	=	new SListCell( element, key );
	if( pNewCell )
		return AddAt( pNewCell, 0 );
	
	return FALSE;
}


//************************************************************************
// AddAt:
//
// PURPOSE:		The method adds an element to the list. The caller
//				must specify the position of the element in the list.
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
SList::AddAt( CONTAINER_ELEMENT element, U32 position ){

	SListCell		*pNewCell;

	pNewCell	=	new SListCell( element, DEFAULT_KEY_VALUE );
	if( pNewCell )
		return AddAt( pNewCell, position );
	
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
SList::Remove( CONTAINER_KEY  key ){
	
	SListCell		*pPrev, *pDeletedElement;

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
SList::RemoveAt( U32 position ){
	
	SListCell		*pPrev, *pDeletedElement;

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
SList::Get( CONTAINER_ELEMENT &element, CONTAINER_KEY key ){
	

	SListCell		*pPrev;

	element = 0;

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
SList::GetAt( CONTAINER_ELEMENT &element, U32 position ){
	

	SListCell		*pPrev;

	element = 0;

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

SListCell*
SList::GetPtrTo1CellBeforeTheElement(	LOOKUP_METHOD method, 
										void*         pMethodDependentKey ){

	SListCell			*pTemp;

	if( count < 2 )
		return NULL;

	switch( method ){
		case BY_KEY:
			{
				CONTAINER_KEY		key = *((CONTAINER_KEY *)pMethodDependentKey);

				for( pTemp = pHead; pTemp->pNext; pTemp = pTemp->pNext )
					if( key == pTemp->pNext->key )
						return pTemp;

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
SList::AddAt( SListCell *pNewCell, U32 position ){

	if( position > count )
		return FALSE;

	if( position == 0 ){
		pNewCell->pNext = pHead;
		pHead = pNewCell;
	}
	else if( count == 1){
		SListCell *pTemp = pHead;

		pNewCell->pNext = pTemp->pNext;
		pTemp->pNext = pNewCell;
	}
	else {
		SListCell *pTemp = GetPtrTo1CellBeforeTheElement( BY_POSITION, (void *)&position );

		pNewCell->pNext = pTemp->pNext;
		pTemp->pNext = pNewCell;
	}

	count++;

	return TRUE;
}