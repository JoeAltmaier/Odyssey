//************************************************************************
// FILE:		DList.h
//
// PURPOSE:		The file defines an ADT doubly linked link list. The
//				class is derived from an abstract base class Container.
//
// TYPES:		class DList : public Container
//				struct DListCell
//
// PUBLIC METHODS:
//				DList()
//				~DList()
//				Add()
//				AddAt()
//				Remove()
//				RemoveAt()
//				Get()
//				GetAt()
//
// NOTES:	
//************************************************************************

#ifndef __DLIST_H__
#define	__DLIST_H__

#include "Container.h"

struct	DListCell;

class DList : public Container{

	
	protected:
		
		DListCell			*pHead;				// ptr to the head of the list

	
	public:



//************************************************************************
// DList:
//
// PURPOSE:		Default constructor
//************************************************************************

	DList();


//************************************************************************
// ~DList:
//
// PURPOSE:		The destructor. Removes all elements from the list
//************************************************************************

	~DList();


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

virtual BOOL Add( CONTAINER_ELEMENT element, CONTAINER_KEY key = DEFAULT_KEY_VALUE );


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

virtual BOOL AddAt( CONTAINER_ELEMENT element, U32 position );


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

virtual BOOL Remove( CONTAINER_KEY  key );


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

virtual BOOL RemoveAt( U32 position );


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

virtual BOOL Get( CONTAINER_ELEMENT &element, CONTAINER_KEY key );	


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

virtual BOOL GetAt( CONTAINER_ELEMENT &element, U32 position );



protected:

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

DListCell*	GetPtrTo1CellBeforeTheElement_ByPosition( U32 positionOfInterest );
DListCell*	GetPtrTo1CellBeforeTheElement_ByKey( U32 keyOfInterest );


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

BOOL	AddAt( DListCell *pNewCell, U32 position );


//************************************************************************
// Container:
// 
// PURPOSE:		Copy constructor. Creates a copy of the object specified
//************************************************************************

	DList( const DList  &original );

};




//************************************************************************
// struct DListCell:
//
// PURPOSE:		Internal struct to store list elements and their keys
//************************************************************************

struct DListCell{
	DListCell			*pNext;
	DListCell			*pPrev;
	CONTAINER_KEY		key;
	CONTAINER_ELEMENT	element;

	DListCell( CONTAINER_ELEMENT e = 0, CONTAINER_KEY k = DEFAULT_KEY_VALUE ){
		key     = k;
		element = e;
		pNext   = pPrev = NULL;
	}
};

#endif __DLIST_H__