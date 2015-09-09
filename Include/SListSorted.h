//************************************************************************
// FILE:		SListSorted.h
//
// PURPOSE:		The file defines an ADT singly linked sorted link list. The
//				class is derived from a singly linked list. The list is 
//				sorted in ascending order.
//
// TYPES:		class SListSorted : public SList
//
// PUBLIC METHODS:
//				SListSorted()
//				Add()

//
// NOTES:		1. Most of the functionality is inherited from SList.
//************************************************************************

#ifndef __SLIST_SORTED_H__
#define	__SLIST_SORTED_H__

#include "SList.h"




class SListSorted : public SList{


	public:


//************************************************************************
// SListSorted:
//
// PURPOSE:		Default constructor. 
//************************************************************************

	SListSorted( );


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
// SListSorted:
//
// PURPOSE:		A copy constructor
//************************************************************************

SListSorted( const SListSorted &original );




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

#endif // __SLIST_SORTED_H__