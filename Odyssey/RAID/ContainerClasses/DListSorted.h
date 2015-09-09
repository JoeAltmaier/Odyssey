//************************************************************************
// FILE:		DListSorted.h
//
// PURPOSE:		The file defines an ADT doubly linked sorted link list. The
//				class is derived from a doubly linked list. The list is 
//				sorted in ascending order.
//
// TYPES:		class DListSorted : public DList
//
// PUBLIC METHODS:
//				DListSorted()
//				Add()

//
// NOTES:		1. Most of the functionality is inherited from SList.
//************************************************************************

#ifndef __DLIST_SORTED_H__
#define	__DLIST_SORTED_H__

#include "DList.h"




class DListSorted : public DList{


	public:


//************************************************************************
// DListSorted:
//
// PURPOSE:		Default constructor. 
//************************************************************************

	DListSorted( );


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



protected:


//************************************************************************
// DListSorted:
//
// PURPOSE:		A copy constructor
//************************************************************************

DListSorted( const DListSorted &original );



//************************************************************************
// AddAt:
//
// PURPOSE:		In this class, AddAt() will allow user to screw up
//				our ordering. Therefore, we re-declare it as "protected"
//				so that outsiders could not use it on instances of this
//				class. 
//************************************************************************

virtual BOOL AddAt( CONTAINER_ELEMENT element, U32 position ) { return TRUE; }

};

#endif // __DLIST_SORTED_H__