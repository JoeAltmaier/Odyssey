//************************************************************************
// FILE:		DesignatorIdVector.h
//
// PURPOSE:		The file defines an ADT singly linked link list.
//
// PUBLIC METHODS:
//				DesignatorIdVector()
//				~DesignatorIdVector()
//				Add()
//				Set()
//				Remove()
//				RemoveAt()
//				Get()
//				GetAt()
//
// NOTES:	
//************************************************************************

#ifndef __DESIGNATOR_ID_VECTOR_H__
#define	__DESIGNATOR_ID_VECTOR_H__

#include "DesignatorId.h"

class ManagedObject;

#ifdef WIN32
#pragma pack(4)
#endif

//struct	DesignatorIdCell;

#define	ID_VECTOR_DEFAULT_INITIAL_SIZE		15
#define	ID_VECTOR_DEFAULT_SIZE_INCREMENT	10

class DesignatorIdVector {



	struct	VECTOR_CELL{
		ManagedObject*		element;
		DesignatorId		key;

		VECTOR_CELL( ManagedObject* e = 0, DesignatorId& k = DesignatorId() ){
			element = e;
			key		= k;
		}
	};
	
	U32				m_size;
	U32				m_sizeIncrement;
	VECTOR_CELL		**m_pBag;
	U32				m_count;

#if 0
		DesignatorIdCell	*pHead;				// ptr to the head of the list
		U32					count;

		typedef enum LOOKUP_METHOD{ BY_KEY = 1, BY_POSITION };
#endif

	public:

		

//************************************************************************
// DesignatorIdVector:
//
// PURPOSE:		Default constructor
//************************************************************************

DesignatorIdVector( U32 initialSize = ID_VECTOR_DEFAULT_INITIAL_SIZE, 
					U32 sizeIncrement = ID_VECTOR_DEFAULT_SIZE_INCREMENT);


//************************************************************************
// ~DesignatorIdVector:
//
// PURPOSE:		The destructor. Removes all elements from the list
//************************************************************************

~DesignatorIdVector();


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
// RECEIVE:		pElement	-	a 4-byte value to store
//				key			-	the key
//
// RETURN:		TRUE  - success
//				FALSE - failure
//************************************************************************

BOOL Add( ManagedObject *pElement, const DesignatorId &key );


//************************************************************************
// Set:
//
// PURPOSE:		Sets new value at the position where the current element 
//				has the 'key' specified. The caller is responsible for
//				all memory clean-up related to the old element
//************************************************************************

BOOL Set( ManagedObject *pElement, const DesignatorId &key );


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

BOOL Remove( const DesignatorId  &key );


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

BOOL RemoveAt( U32 position );


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

BOOL Get( ManagedObject* &pElement, const DesignatorId &key );	


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

BOOL GetAt( ManagedObject* &pElement, U32 position );


//************************************************************************
// GetCount:
//
// PURPOSE:		An  accessor
//
// RETURN:		The # of elements in the vector
//************************************************************************

U32 GetCount() const { return m_count; }



private:

//************************************************************************
// Grow:
//
// PURPOSE:		Grows the vector by the number of entries specified
//				The new entries are appended to the end of the vector
//************************************************************************

void Grow( U32 entriesToGrow );


//************************************************************************
// Contract:
//
// PURPOSE:		Decreases the size of the vector by one entry by removing
//				the entry at the position specified and moving all
//				entries above it down by one.
//************************************************************************

void RemoveEntry( U32 position );

#if 0
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

BOOL	AddAt( DesignatorIdCell *pNewCell, U32 position );
#endif

//************************************************************************
// Container:
// 
// PURPOSE:		Copy constructor. Creates a copy of the object specified
//************************************************************************

	DesignatorIdVector( const DesignatorIdVector  &original );

};




//************************************************************************
// struct SListCell:
//
// PURPOSE:		Internal struct to store list elements and their keys
//************************************************************************

struct DesignatorIdCell{
	DesignatorIdCell	*pNext;
	DesignatorId		key;
	ManagedObject*		element;

	DesignatorIdCell( ManagedObject* e = NULL, DesignatorId k = DesignatorId() ){
		key     = k;
		element = e;
		pNext   = NULL;
	}
};

#endif // __DESIGNATOR_ID_VECTOR_H__