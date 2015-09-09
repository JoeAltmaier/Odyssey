//************************************************************************
// FILE:		DList.cpp
//
// PURPOSE:		The file implements an ADT doubly linked link list.
//
// NOTES:	
//************************************************************************


#include "DList.h"


//************************************************************************
// DList:
//
// PURPOSE:		Default constructor
//************************************************************************

DList::DList() : Container() {

	pHead = NULL;
}


//************************************************************************
// DList:
// 
// PURPOSE:		Copy constructor. Creates a copy of the object specified
//************************************************************************

DList::DList( const DList  &original ) : Container( original ){

	CONTAINER_ELEMENT		element;

	pHead = NULL;

	for( U32 index = 0; index < original.count; index++ ){
		((DList) original).GetAt( element, index );
		AddAt( element, index );
	}
}


//************************************************************************
// ~DList:
//
// PURPOSE:		The destructor. Removes all elements from the list
//************************************************************************

DList::~DList(){

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
DList::Add( CONTAINER_ELEMENT element, CONTAINER_KEY key ){
	
	DListCell		*pNewCell;

	pNewCell	=	new DListCell( element, key );
	if( pNewCell )
		return AddAt( pNewCell, 0 );
	else
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
DList::AddAt( CONTAINER_ELEMENT element, U32 position ){

	DListCell		*pNewCell;

	pNewCell	=	new DListCell( element, DEFAULT_KEY_VALUE );
	if( pNewCell )
		return AddAt( pNewCell, position );
	else
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
DList::Remove( CONTAINER_KEY  key ){
	
	DListCell		*pPrev, *pDeletedElement;

	if( !count ){
		return FALSE;
	}
	else if( pHead->key == key ){
		pDeletedElement = pHead;
		pHead = pHead->pNext;
		if( pHead )
			pHead->pPrev = NULL;
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
		pPrev = GetPtrTo1CellBeforeTheElement_ByKey( key );
		if( !pPrev )
			return FALSE;

		pDeletedElement = pPrev->pNext;

		pPrev->pNext = pDeletedElement->pNext;
		if( pDeletedElement->pNext )
			pDeletedElement->pNext->pPrev = pPrev;

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
DList::RemoveAt( U32 position ){
	
	DListCell		*pPrev, *pDeletedElement;

	if( position >= count ){
		return FALSE;
	}
	else if( position == 0 ){
		pDeletedElement = pHead;
		pHead = pHead->pNext;
		if( pHead )
			pHead->pPrev = NULL;
		delete pDeletedElement;
	}
	else {
		pPrev = GetPtrTo1CellBeforeTheElement_ByPosition( position );
		if( !pPrev )
			return FALSE;

		pDeletedElement = pPrev->pNext;

		pPrev->pNext = pDeletedElement->pNext;
		if( pDeletedElement->pNext )
			pDeletedElement->pNext->pPrev = pPrev;

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
DList::Get( CONTAINER_ELEMENT &element, CONTAINER_KEY key ){
	

	DListCell		*pPrev;

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
		pPrev = GetPtrTo1CellBeforeTheElement_ByKey( key );
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
DList::GetAt( CONTAINER_ELEMENT &element, U32 position ){
	

	DListCell		*pPrev;

	if( count <= position ){
		return FALSE;
	}
	else if( position == 0 ){
		element = pHead->element;
	}
	else {
		pPrev = GetPtrTo1CellBeforeTheElement_ByPosition( position );
		if( !pPrev )
			return FALSE;

		element = pPrev->pNext->element;
	}

	return TRUE;
}



//************************************************************************
// 1. GetPtrTo1CellBeforeTheElement_ByPosition
// 2. GetPtrTo1CellBeforeTheElement_ByKey
//
// PURPOSE:		The methods traverse the list and return a pointer to
//				the element that immideatly precedes the element of our
//				interest
//	
// RECEIVE:		1. positionOfInterest
//				2. keyOfInterest
//
// RETURN:		On success - ptr to the element
//				On failure - NULL
//************************************************************************

DListCell*	
DList::GetPtrTo1CellBeforeTheElement_ByPosition( U32 positionOfInterest ){

	DListCell			*pTemp;
	U32					index;

	if( count < 2 )
		return NULL;

	
	for( pTemp = pHead, index = 1; pTemp->pNext && index < positionOfInterest; pTemp = pTemp->pNext, index++ )
		;
				
	return pTemp;
}


DListCell*	
DList::GetPtrTo1CellBeforeTheElement_ByKey( U32 keyOfInterest ){

	DListCell			*pTemp;

	if( count < 2 )
		return NULL;

	for( pTemp = pHead; pTemp->pNext; pTemp = pTemp->pNext )
		if( keyOfInterest == pTemp->pNext->key )
			return pTemp;

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
DList::AddAt( DListCell *pNewCell, U32 position ){

	if( position > count )
		return FALSE;

	if( position == 0 ){
		if( pHead )
			pHead->pPrev = pNewCell;
		pNewCell->pNext = pHead;
		pNewCell->pPrev = NULL;
		pHead = pNewCell;

	}
	else if( count == 1) {
		DListCell *pTemp = pHead;

		pNewCell->pNext = pTemp->pNext;
		pNewCell->pPrev = pTemp;
		if( pTemp->pNext )
			pTemp->pNext->pPrev = pNewCell;
		pTemp->pNext = pNewCell;
	}
	else {
		DListCell *pTemp = GetPtrTo1CellBeforeTheElement_ByPosition( position );

		pNewCell->pNext = pTemp->pNext;
		pNewCell->pPrev = pTemp;
		if( pTemp->pNext )
			pTemp->pNext->pPrev = pNewCell;
		pTemp->pNext = pNewCell;
	}

	count++;

	return TRUE;
}