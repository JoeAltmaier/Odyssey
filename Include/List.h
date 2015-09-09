/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: List.h
// 
// Description:
// Doubly linked lists are powerful and easy to use.  
// The LIST primitives defined in List.h make it easy to:
// * Define a doubly linked object.
// * Define a list.
// * Initialize a list.
// * Link an object to the head or tail of a list.
// * Check to see if a list is empty.
// * Remove the object at the head or tail of a list and get its pointer.
// * Remove a specified object from the list it is linked to. 
// 
// To define a doubly linked object:
// * Include a LIST object at the beginning of the class being defined. 
// 
//		class Linked_Object 
//		{
			// The LIST object is a structure that contains a forward link and 
			// a backward link.  The LIST object MUST be the first data object
			// in the class or structure definition.
			// LIST 	list;
			// ...
//		};
// 	
// To define a list:
// 
//		LIST	object_avail_list;
// 
// To initialize a LIST:
// 
//		LIST_INITIALIZE (&object_avail_list);
// 
// To add an object to the tail of a list:
// 
//		Linked_Object		*p_object;
//		LIST_INSERT_TAIL(&object_avail_list, &p_object->list);
// 
// To add an object to the head of a list:
// 
//		LIST_INSERT_HEAD(&object_avail_list, &p_object->list);
// 
// To check to see if a list is empty:
// 
//		If (LIST_IS_EMPTY(&object_avail_list))
//		{
			// object_avail_list is empty
//		}
// 
// To check to see if a list entry is the last in the list:
// 
//		If (LIST_ENTRY_IS_LAST(&object_avail_list, &object_list))
//		{
			// object_list is the last in the list
//		}
// 
// To remove the object at the head of a list:
// 
//		Linked_Object 	*p_object;
//		p_object = (Linked_Object *)LIST_REMOVE_HEAD(&object_list);
// 
// To point to the object at the head of a list:
// 
//		Linked_Object 	*p_object;
//		p_object = (Linked_Object *)LIST_POINT_HEAD(&object_list);
// 
// To point to the next object in the list:
// 
//		Linked_Object 	*p_object;
//		p_object = (Linked_Object *)LIST_POINT_NEXT(&object_list);
// 
// To point to the previous object in the list:
// 
//		Linked_Object 	*p_object;
//		p_object = (Linked_Object *)LIST_POINT_PREVIOUS(&object_list);
// 
// To remove the object at the tail of a list:
// 
//		p_object = (Linked_Object *)LIST_REMOVE_TAIL(&object_list);
// 
// To point to the object at the tail of a list:
// 
//		p_object = (Linked_Object *)LIST_POINT_TAIL(&object_list);
// 
// To remove an object from whatever list it is on:
// 
//		LIST_REMOVE (&p_object->list);
// 
//  Critical Sections
//	Any critical section protection must be done outside of the
//	LIST primitives.
// 
// Update Log:
// 8/20/98 Jim Frandeen: Create file
// 12/07/98 Jim Frandeen: Add LIST_POINT_HEAD and LIST_POINT_TAIL
// 05/03/99 Jim Frandeenf: Add LIST_ENTRY_IS_LAST, LIST_POINT_NEXT,
//							LIST_POINT_PREVIOUS
/*************************************************************************/

#if !defined(List_H)
#define List_H

 typedef struct _LIST {
	 struct _LIST *forward_link;
	 struct _LIST *backward_link;
 } LIST;

/*************************************************************************/
// void LIST_INITIALIZE(LIST *p_list_head)
// Set forward_link and backward_link to point to the head of the list.
// This makes the list empty.
/*************************************************************************/
#define LIST_INITIALIZE(p_list_head) (\
    (p_list_head)->forward_link = (p_list_head)->backward_link = (p_list_head))

/*************************************************************************/
// int LIST_IS_EMPTY(LIST *p_list_head)
// If the forward_link points to the list head, the list is empty.
/*************************************************************************/
#define LIST_IS_EMPTY(p_list_head) \
    ((p_list_head)->forward_link == (p_list_head))

/*************************************************************************/
// int LIST_ENTRY_IS_LAST(LIST *p_list_head, LIST *p_list_entry)
// If the forward_link points to the list head, the list entry is last.
/*************************************************************************/
#define LIST_ENTRY_IS_LAST(p_list_head, p_list_entry) \
    ((p_list_entry)->forward_link == (p_list_head))

/*************************************************************************/
// void LIST_INSERT_TAIL(LIST *p_list_head, LIST *p_list_entry)

 /*************************************************************************/
// LIST *LIST_REMOVE_HEAD(LIST *p_list_head)
// Remove the first object from the list and return its pointer.
/*************************************************************************/
#define LIST_REMOVE_HEAD(p_list_head) \
    (p_list_head)->forward_link;/* assign to LHS */ \
    {LIST_REMOVE((p_list_head)->forward_link)}

/*************************************************************************/
// LIST *LIST_POINT_HEAD(LIST *p_list_head)
// Point to the first object in the list and return its pointer.
/*************************************************************************/
#define LIST_POINT_HEAD(p_list_head) \
    (p_list_head)->forward_link

/*************************************************************************/
// LIST *LIST_POINT_NEXT(LIST *p_list_entry)
// Point to the next object in the list and return its pointer.
/*************************************************************************/
#define LIST_POINT_NEXT(p_list_entry) \
    (p_list_entry)->forward_link

/*************************************************************************/
// LIST *LIST_POINT_PREVIOUS(LIST *p_list_entry)
// Point to the previous object in the list and return its pointer.
/*************************************************************************/
#define LIST_POINT_PREVIOUS(p_list_entry) \
    (p_list_entry)->backward_link

/*************************************************************************/
// LIST *LIST_REMOVE_TAIL(LIST *p_list_head)
// Remove the last object from the list and return its pointer.
/*************************************************************************/
#define LIST_REMOVE_TAIL(p_list_head) \
    (p_list_head)->backward_link; /* assign to LHS */ \
    {LIST_REMOVE((p_list_head)->backward_link)}

/*************************************************************************/
// LIST *LIST_POINT_TAIL(LIST *p_list_head)
// Point to the last object in the list and return its pointer.
/*************************************************************************/
#define LIST_POINT_TAIL(p_list_head) \
    (p_list_head)->backward_link

/*************************************************************************/
// void LIST_REMOVE(LIST *p_list_entry)
// Remove the object from the list.
// Move the forward_link from the object to the forward_link of the 
// previous object.  Move the backward link of the object to the backward
// link of the next object.
/*************************************************************************/
#define LIST_REMOVE(p_list_entry) {\
    LIST *_forward_link;\
    LIST *_backward_link;\
    _forward_link = (p_list_entry)->forward_link;\
    _backward_link = (p_list_entry)->backward_link;\
    _backward_link->forward_link = _forward_link;\
    _forward_link->backward_link = _backward_link;\
    }

/*************************************************************************/
// void LIST_INSERT_TAIL(LIST *p_list_head, LIST *p_list_entry)
// Insert object at tail of list pointed to by p_list_head.
// If p_list_head points to an object, the new object gets inserted 
// before this object.
// Save backward_link of list head. 
// Set forward_link of object to point to list head.
// Set backward_link of object to saved backward_link of list head.
// Set backward_link of list head to point to object.
/*************************************************************************/
#define LIST_INSERT_TAIL(p_list_head, p_list_entry) {\
    LIST *_backward_link;\
    LIST *_p_list_head;\
    _p_list_head = (p_list_head);\
    _backward_link = _p_list_head->backward_link;\
    (p_list_entry)->forward_link = _p_list_head;\
    (p_list_entry)->backward_link = _backward_link;\
    _backward_link->forward_link = (p_list_entry);\
    _p_list_head->backward_link = (p_list_entry);\
    }

/*************************************************************************/
// void LIST_INSERT_HEAD(LIST *p_list_head, LIST *p_list_entry)
// Insert object at head of list pointed to by p_list_head.
// If p_list_head points to an object, the new object gets inserted 
// before this object.
// Save forward_link of list head. 
// Set forward_link of object to point to saved forward_link.
// Set backward_link of object to saved list head.
// Set forward_link of list head to point to object.
/*************************************************************************/
#define LIST_INSERT_HEAD(p_list_head,p_list_entry) {\
    LIST *_forward_link;\
    LIST *_p_list_head;\
    _p_list_head = (p_list_head);\
    _forward_link = _p_list_head->forward_link;\
    (p_list_entry)->forward_link = _forward_link;\
    (p_list_entry)->backward_link = _p_list_head;\
    _forward_link->backward_link = (p_list_entry);\
    _p_list_head->forward_link = (p_list_entry);\
    }


#endif /* List_H  */
