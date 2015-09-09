/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This class is a singly linked list container class.
// 
// Update Log: 
// 10/26/98	JFL	created.
//
/*************************************************************************/

#ifndef __SglLinkedList_h
#define __SglLinkedList_h

#include "OsStatus.h"

typedef struct SglLinkedListNode {
	struct SglLinkedListNode	*pNextNode;
	void						*pListItem;
} SglLinkedListNode;


//
// SglLinkedList -
// This class defines an object whose job is to maintain a singly 
// linked list data structure.
//
class SglLinkedList
{

public:

	SglLinkedList();
	~SglLinkedList();

	void	AddToHead( void *pNewItem );
	void	AddToTail( void *pNewItem );
	
	void*	RemoveFromHead();
	void*	RemoveFromTail();
	
	SglLinkedListNode	*m_pListHeadNode;
	SglLinkedListNode	*m_pListTailNode;
	int					nNodeCount;
};


//
// SglLinkedListIterator -
// This calss is an iterator over list managed by the the previously 
// declared SglLinkedList class.
//
class SglLinkedListIterator
{
public:
	SglLinkedListIterator( SglLinkedList	*pSglLinkedList);
		
	void*	FirstItem();
	void*	NextItem();
	void*	CurrentItem();
	int		EndOfList();
	void*	RemoveCurrentItem(); // and return the next item or NULL

private:
	SglLinkedList		*m_pSglLinkedList;
	SglLinkedListNode	*m_pCurrentNode;
};


#endif	// __SinglyLinkedList_h

