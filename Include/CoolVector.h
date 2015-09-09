//************************************************************************
// FILE:		CoolVector.h
//
// PURPOSE:		The file defines the Vector object that implements Container
//				interface and is a dynamically growing/contracting array
//				with access time == 1 
//
// NOTES:	
//************************************************************************

#ifndef __CONTAINER_VECTOR_H__
#define	__CONTAINER_VECTOR_H__


#include "Container.h"


#define	VECTOR_DEFAULT_INITIAL_SIZE		10
#define	VECTOR_DEFAULT_SIZE_INCREMENT	8

class CoolVector : public Container {

	struct	VECTOR_CELL{
		CONTAINER_ELEMENT	element;
		CONTAINER_KEY		key;

		VECTOR_CELL( CONTAINER_ELEMENT e = 0, CONTAINER_KEY k = DEFAULT_KEY_VALUE ){
			element = e;
			key		= k;
		}
	};
	
	U32				m_size;
	U32				m_sizeIncrement;
	VECTOR_CELL		**m_pBag;

	public:


//************************************************************************
// CoolVector:
//
// PURPOSE:		Default constructor
//************************************************************************

CoolVector(	U32 initialSize = VECTOR_DEFAULT_INITIAL_SIZE, 
			U32 sizeIncrement = VECTOR_DEFAULT_SIZE_INCREMENT );


//************************************************************************
// ~CoolVector:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~CoolVector();


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

virtual BOOL Add( CONTAINER_ELEMENT element, CONTAINER_KEY key = DEFAULT_KEY_VALUE );


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

virtual BOOL AddAt( CONTAINER_ELEMENT element, U32 position );


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

virtual BOOL GetAt( CONTAINER_ELEMENT &element, U32 position );


U32 GetSize() const { return m_size; }


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


//************************************************************************
// operator=
//************************************************************************

const CoolVector& operator= (const CoolVector& obj ) { return obj; }

//************************************************************************
// CoolVector:
// 
// PURPOSE:		Copy constructor, disabled on purpose
//************************************************************************

CoolVector( const CoolVector  &original ) { count = original.count; }

};

#endif __CONTAINER_VECTOR_H__