//************************************************************************
// FILE:		DListSorted.cpp
//
// PURPOSE:		The implementation of an ADT doubly linked sorted link list. 
//
// NOTES:		1. Most of the functionality is inherited from DList.
//************************************************************************


#include "DListSorted.h"


//************************************************************************
// DListSorted:
//
// PURPOSE:		Default constructor. 
//************************************************************************

DListSorted::DListSorted(  )
			:DList(){

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
DListSorted::Add( CONTAINER_ELEMENT element, CONTAINER_KEY key ){
	
	DListCell		*pNewCell, *pTempCell, *pPrev = NULL;;

	pNewCell = new DListCell( element, key );
	if( !pNewCell )
		return FALSE;

	for( pTempCell = pHead; pTempCell && (pTempCell->key < key); pTempCell = pTempCell->pNext ) 
		pPrev = pTempCell;

	if( pTempCell == pHead ){
		if( pHead )
			pHead->pPrev = pNewCell;
		pNewCell->pNext = pHead;
		pNewCell->pPrev = NULL;
		pHead = pNewCell;
	}
	else {
		pNewCell->pNext = pPrev->pNext;
		pNewCell->pPrev = pPrev;
		if( pPrev->pNext )
			pPrev->pNext->pPrev = pNewCell;
		pPrev->pNext = pNewCell;
	}

	count++;
	return TRUE;
}



//************************************************************************
// DListSorted:
//
// PURPOSE:		A copy constructor
//************************************************************************

DListSorted::DListSorted( const DListSorted &original )
			:DList( original ){
	

}
