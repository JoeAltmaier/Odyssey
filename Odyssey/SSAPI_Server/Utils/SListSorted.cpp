//************************************************************************
// FILE:		SListSorted.cpp
//
// PURPOSE:		The implementation of an ADT singly linked sorted link list. 
//
// NOTES:		1. Most of the functionality is inherited from SList.
//************************************************************************


#include "SListSorted.h"


//************************************************************************
// SListSorted:
//
// PURPOSE:		Default constructor. 
//************************************************************************

SListSorted::SListSorted(  )
			:SList(){

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
SListSorted::Add( CONTAINER_ELEMENT element, CONTAINER_KEY key ){
	
	SListCell		*pNewCell, *pTempCell, *pPrev = NULL;;

	pNewCell = new SListCell( element, key );
	if( !pNewCell )
		return FALSE;

	for( pTempCell = pHead; pTempCell && (pTempCell->key < key); pTempCell = pTempCell->pNext ) 
		pPrev = pTempCell;

	if( pTempCell == pHead ){
		pNewCell->pNext = pHead;
		pHead = pNewCell;
	}
	else {
		pNewCell->pNext = pTempCell;
		pPrev->pNext    = pNewCell;
	}

	count++;
	return TRUE;
}



//************************************************************************
// SListSorted:
//
// PURPOSE:		A copy constructor
//************************************************************************

SListSorted::SListSorted( const SListSorted &original )
			:SList( original ){






}
