//************************************************************************
// FILE:		Container.h
//
// PURPOSE:		The file defines the base skeleton for container classes.
//				This is an abstract object and thus must be dertived from 
//				breafo it can be used.
//
// TYPES:		class Container
//				CONTAINER_KEY
//				CONTAINER_ELEMENT
//
// PUBLIC METHODS:
//				Container
//				Container
//				Count()
//				Add()
//				AddAt()
//				Remove()
//				RemoveAt()
//				Get()
//				GetAt()
//				RemoveAll()
//
// NOTES:	
//************************************************************************

#ifndef __CONTAINER_H__
#define	__CONTAINER_H__


#include "CTtypes.h"
#include "Simple.h"



typedef		U32					CONTAINER_KEY;
typedef		U32					CONTAINER_ELEMENT;


#define		DEFAULT_KEY_VALUE	0



class Container {
	
	protected:

		U32			count;					// number of items in the container
	
	public:


//************************************************************************
// Container:
//
// PURPOSE:		Default constructor
//************************************************************************

		Container() {	count = 0; }


//************************************************************************
// ~Container:
// 
// PURPOSE:		The destructor
//************************************************************************

		virtual ~Container() {}



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

virtual BOOL Add( CONTAINER_ELEMENT element, CONTAINER_KEY key = DEFAULT_KEY_VALUE ) = 0;


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

virtual BOOL AddAt( CONTAINER_ELEMENT element, U32 position ) = 0;


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

virtual BOOL Remove( CONTAINER_KEY  key ) = 0;


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

virtual BOOL RemoveAt( U32 position ) = 0;


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

virtual BOOL Get( CONTAINER_ELEMENT &element, CONTAINER_KEY key ) = 0;	


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

virtual BOOL GetAt( CONTAINER_ELEMENT &element, U32 position ) = 0;


//************************************************************************
// Count:
//
// PUPOSE:		An accessor
//
// RETURN:		# of elements in the container
//************************************************************************

U32	 Count() const	{ return count; }


//************************************************************************
// RemoveAll
//
// PURPOSE:		Removes all items from the container
//************************************************************************

void RemoveAll(){
	while( count )
		RemoveAt( 0 );
}



private:
	
//************************************************************************
// operator=
//************************************************************************

const Container& operator= (const Container& obj ) { return obj; }

};

#endif __CONTAINER_H__