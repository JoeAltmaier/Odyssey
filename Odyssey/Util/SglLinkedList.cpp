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
//
// $Log: /Gemini/Odyssey/Util/SglLinkedList.cpp $
// 
// 8     8/23/99 4:17p Vnguyen
// Add RemoveCurrentItem() method.
// 
// 7     5/18/99 11:27a Jaltmaier
// Fix include duplicate 'UNSIGNED'.
// Add return to methods missing return.
// Prototype methods to avoid warning.
// 
// 6     5/10/99 7:01p Jlane
// Fix AddToTail on a new list.
//
// 10/26/98	JFL	created.
// 10/28/98	JFL	Added SglLinkedListIterator.
// 02/22/99 Jim Frandeen: Use Simple.h instead of null.h so as not to
//				require ansi_parms.h
//
/*************************************************************************/

#include "SglLinkedList.h"
#include "Simple.h"

//
// SglLinkedList - This is the constructor.  It just initializes the fields.
//
SglLinkedList::SglLinkedList()
{
	m_pListHeadNode = m_pListTailNode = NULL;
	nNodeCount = 0;
};



//
// ~SglLinkedList - This is the destructor.  
// It cleans up by deleting any nodes in the list.
//
SglLinkedList::~SglLinkedList()
{
	while (m_pListHeadNode)
		void*	pNode = RemoveFromHead(); 
};


//
// AddtoHead - This adds a node at the head of the list pointing to pNewItem.
//
void SglLinkedList::AddToHead( void *pNewItem )
{
	SglLinkedListNode*	pNewNode;
	
	// Allocate a new list node and point it at the passed item.
	pNewNode = new SglLinkedListNode;
	pNewNode->pListItem = pNewItem;
	
	// Point the new node at the old head of the list.
	pNewNode->pNextNode = m_pListHeadNode;
	
	// Point the head of the list at the new node.
	m_pListHeadNode = pNewNode;
	
	// If this is first item added to list tail points here too.
	if (!m_pListTailNode)
		m_pListTailNode = pNewNode;
	
	// Increment the count of things in the list.
	nNodeCount++;
};


//
// AddToTail - This adds a node to the tail of the list pointing to pNewItem.
//
void SglLinkedList::AddToTail( void *pNewItem )
{
	SglLinkedListNode*	pNewNode;
	
	// Allocate a new list node and point it at the passed item.
	pNewNode = new SglLinkedListNode;
	pNewNode->pListItem = pNewItem;
	
	// Clear new node's next pointer since it will be last.
	pNewNode->pNextNode = NULL;
	
	// If the List is empty call just insert the item.
	if (m_pListTailNode == NULL)
	{
		// Point the head of the list and the end of the list at the new node.
		m_pListHeadNode = m_pListTailNode = pNewNode;
	}
	else
	{
		// Point the current last node in the list at the new node./
		m_pListTailNode->pNextNode = pNewNode;
	
		// Point the tail of the list at the new node.
		m_pListTailNode = pNewNode;
	}
	
	// Increment the count of things in the list.
	nNodeCount++;

};


//
// RemoveFromHead - this removes a node from the head of the list and 
// returns the item pointer..
//	
void* SglLinkedList::RemoveFromHead()
{
	void*				pRemovedItem = NULL;
	SglLinkedListNode*	pRemovedNode = NULL;

	// If there's anything in the list..
	if (nNodeCount)
	{
		// If this is the last item in the list null the tail pointer.
		if (m_pListTailNode == m_pListHeadNode)
			m_pListTailNode = NULL;
		
		// Get a pointer to the first item in the list...
		pRemovedNode = m_pListHeadNode;
		// ...and the item to return
		pRemovedItem = pRemovedNode->pListItem;
		
		// Update the list head pointer 
		m_pListHeadNode = m_pListHeadNode->pNextNode;
		
		// delete the removed node
		delete pRemovedNode;
		
		// Decremnent count of items in list.
		nNodeCount--;
	}
	
	return pRemovedItem;
};



//
// RemoveFromTail - This removes the last node in the list and returns
// the item pointer.
///
void* SglLinkedList::RemoveFromTail()
{
	SglLinkedListNode*	pTempNode = NULL;
	SglLinkedListNode*	pRemovedNode = NULL;
	void*				pRemovedItem = NULL;

	// If there's anything in the list..
	if (nNodeCount)
	{
		// If there's just one item in the list
		if (m_pListHeadNode == m_pListTailNode)
			return RemoveFromHead();
		
		// Find the node before the last node (i.e. the new tail node).
		pTempNode = m_pListHeadNode;
		while (pTempNode->pNextNode != m_pListTailNode)
			pTempNode = pTempNode->pNextNode;
			
		// Get a pointer to the first item in the list...
		pRemovedNode = m_pListTailNode;
		// ...and the item to return
		pRemovedItem = pRemovedNode->pListItem;
		
		// delete the removed node
		delete pRemovedNode;
		
		// Update the list tail pointer and its next pointer..
		m_pListTailNode = pTempNode;
		m_pListTailNode->pNextNode = NULL;
		
		// Decremnent count of items in list.
		nNodeCount--;
	};
	
	return pRemovedItem;
};


//
// SglLinkedListIterator - Construct an iterator for a singly linked list.
// 
SglLinkedListIterator::SglLinkedListIterator( SglLinkedList	*pSglLinkedList )
{
	m_pSglLinkedList = pSglLinkedList;
	if (m_pSglLinkedList)
		m_pCurrentNode = m_pSglLinkedList->m_pListHeadNode;
};


//
// FirstItem - Return a pointer to the first item in a linked list or null.
//
void* SglLinkedListIterator::FirstItem()
{
	if (!m_pSglLinkedList)
		return NULL;

	// Reset the current node pointer to the first node in the list...
	m_pCurrentNode = m_pSglLinkedList->m_pListHeadNode;
	
	// ...and return the item pointed to by that node if there is one.
	if (m_pCurrentNode)
		return m_pCurrentNode->pListItem;

	return NULL;
};


//
// CurrentItem - Return a pointer to the current item in the list.
//
void* SglLinkedListIterator::CurrentItem()
{
	// If we have no list, OR no current node return NULL
	if (!m_pSglLinkedList || !m_pCurrentNode)
		return NULL;

	// Rreturn the item pointed to by the current node.
	return m_pCurrentNode->pListItem;
};


//
// NextItem - Advance the current node pointer and return a pointer to the next item.
//
void* SglLinkedListIterator::NextItem()
{
	// If we have no list, OR no current node OR no next node return NULL
	if (!m_pSglLinkedList || !m_pCurrentNode || !m_pCurrentNode->pNextNode)
		return NULL;

	// Advance the node pointer...
	m_pCurrentNode = m_pCurrentNode->pNextNode;
	
	// ...and return the item pointed to by that node.
	return m_pCurrentNode->pListItem;
};


//
// EndOfList - 
// Return true if the current node pointer is the last item in the list
//
int SglLinkedListIterator::EndOfList()
{
	if (!m_pSglLinkedList)
		return true;

	// Check and return whether or not we're at the end of the list.
	return (m_pCurrentNode == m_pSglLinkedList->m_pListTailNode);
};


//
// RemoveCurrentItem - Delete the current item from the list.  Advance the 
// current node pointer and return a pointer to the next item.
//
void* SglLinkedListIterator::RemoveCurrentItem()
{
	SglLinkedListNode*	pTempNode;

	// case 1:  No list or current node does not point to a node, may be 
	// empty list, or we have just deleted the tail node.
	if (!m_pSglLinkedList || !m_pCurrentNode)
		return NULL;

	// case 2:  Current Node points to the head of list.
	// Need to advance m_pListHeadNode.  Also check if removing the
	// last item in list.  If so, set m_pListTailNode to NULL.
	if (m_pSglLinkedList->m_pListHeadNode == m_pCurrentNode)
	{
		m_pCurrentNode = m_pCurrentNode->pNextNode;
		delete m_pSglLinkedList->m_pListHeadNode;
		m_pSglLinkedList->nNodeCount--;
		m_pSglLinkedList->m_pListHeadNode = m_pCurrentNode;
		
		// Now we check to see if we just removed the last item
		if (!m_pCurrentNode)
		{
			m_pSglLinkedList->m_pListTailNode = NULL;
			return NULL;
		}
		else
			return m_pCurrentNode->pListItem;
	} /* if */
	

	// case 3:  The normal case.  The current node is somewhere in the 
	// linked list including the last item.

	// Find the node before the current node
	pTempNode = m_pSglLinkedList->m_pListHeadNode;
	while (pTempNode->pNextNode != m_pCurrentNode)
			pTempNode = pTempNode->pNextNode;
	
	// Advance to the next node
	m_pCurrentNode = m_pCurrentNode->pNextNode;
	
	// Delete the node and adjust the node count.
	delete pTempNode->pNextNode;
	m_pSglLinkedList->nNodeCount--;
	
	// Now relink the linked list
	pTempNode->pNextNode = m_pCurrentNode;

	// Now we check to see if we just removed the last item
	if (!m_pCurrentNode)
	{
		m_pSglLinkedList->m_pListTailNode = NULL;
		return NULL;
	}
	else
		return m_pCurrentNode->pListItem;
};


