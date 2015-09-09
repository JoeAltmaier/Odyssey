/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 1993-1998 Accelerated Technology, Inc.           */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*      csc.c                                           PLUS 1.3         */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      CS -    Common Services                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains linked list manipulation facilities used      */
/*      throughout the Nucleus PLUS system.  These facilities operate    */
/*      on doubly-linked circular lists.                                 */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      CSC_Place_On_List                   Place node at the end of a   */
/*                                            list                       */
/*      CSC_Priority_Place_On_List          Place node in priority order */
/*                                            on a list                  */
/*      CSC_Remove_From_List                Remove a node from a list    */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      nucleus.h                           Nucleus PLUS constants       */
/*      cs_defs.h                           Common service definitions   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      D. Lamie        04-19-1993      Verified version 1.0             */
/*      W. Lamie        03-01-1994      Changed void to VOID, removed    */
/*                                        clearing link pointers during  */
/*                                        removal of a node from a list, */
/*                                        resulting in version 1.1       */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*      M.Q. Qian       04-17-1996      updated to version 1.2           */
/*      M. Trippi       03-24-1998      Released version 1.3.            */
/*                                                                       */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_defs.h"                 /* Include CS definitions    */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      CSC_Place_On_List                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function places the specified node at the end of specified  */
/*      linked list.                                                     */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      various components                                               */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      head                                Pointer to head pointer      */
/*      node                                Pointer to node to add       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      modified list                                                    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      D. Lamie        04-19-1993      Verified version 1.0             */
/*                                                                       */
/*************************************************************************/
VOID    CSC_Place_On_List(CS_NODE **head, CS_NODE *new_node)
{

    /* Determine if the list in non-empty.  */
    if (*head)
    {
    
        /* The list is not empty.  Add the new node to the end of
           the list.  */
        new_node -> cs_previous =               (*head) -> cs_previous;
        (new_node -> cs_previous) -> cs_next =  new_node;
        new_node -> cs_next =                   (*head);
        (new_node -> cs_next) -> cs_previous =  new_node;
    }
    else
    {
    
        /* The list is empty, setup the head and the new node.  */
        (*head) =  new_node;
        new_node -> cs_previous =  new_node;
        new_node -> cs_next =      new_node;
    }
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      CSC_Priority_Place_On_List                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function places the specified node after all other nodes on */
/*      the list of equal or greater priority.  Note that lower          */
/*      numerical values indicate greater priority.                      */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      various components                                               */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      head                                Pointer to head pointer      */
/*      node                                Pointer to node to add       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      modified list                                                    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      D. Lamie        04-19-1993      Verified version 1.0             */
/*                                                                       */
/*************************************************************************/
VOID    CSC_Priority_Place_On_List(CS_NODE **head, CS_NODE *new_node)
{

CS_NODE         *search_ptr;                /* List search pointer       */

    /* Determine if the list in non-empty.  */
    if (*head)
    {
    
        /* Search the list to find the proper place for the new node.  */
        search_ptr =  (*head);
        
        /* Check for insertion before the first node on the list.  */
        if (search_ptr -> cs_priority > new_node -> cs_priority)
        {
        
            /* Update the head pointer to point at the new node.  */
            (*head) =  new_node;
        }
        else
        {

            /* We know that the new node is not the highest priority and
               must be placed somewhere after the head pointer.  */
            
            /* Move search pointer up to the next node since we are trying
               to find the proper node to insert in front of. */
            search_ptr =  search_ptr -> cs_next;
            while ((search_ptr -> cs_priority <= new_node -> cs_priority) &&
                   (search_ptr != (*head)))
            {
            
                /* Move along to the next node.  */
                search_ptr =  search_ptr -> cs_next;
            } 
        }

        /* Insert before search pointer.  */
        new_node -> cs_previous =               search_ptr -> cs_previous;
        (new_node -> cs_previous) -> cs_next =  new_node;
        new_node -> cs_next =                   search_ptr;
        (new_node -> cs_next) -> cs_previous =  new_node;
    }
    else
    {
    
        /* The list is empty, setup the head and the new node.  */
        (*head) =  new_node;
        new_node -> cs_previous =  new_node;
        new_node -> cs_next =      new_node;
    }
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      CSC_Remove_From_List                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function removes the specified node from the specified      */
/*      linked list.                                                     */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      various components                                               */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      head                                Pointer to head pointer      */
/*      node                                Pointer to node to add       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      modified list                                                    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      D. Lamie        04-19-1993      Verified version 1.0             */
/*      W. Lamie        03-01-1994      Removed clearing link pointers   */
/*                                        during removal of a node from  */
/*                                        list, resulting in version 1.1 */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
VOID    CSC_Remove_From_List(CS_NODE **head, CS_NODE *node)
{

    /* Determine if this is the only node in the system.  */
    if (node -> cs_previous == node)
    {
    
        /* Yes, this is the only node in the system.  Clear the node's
           pointers and the head pointer.  */
        (*head) =              NU_NULL;
    }
    else
    {
    
        /* Unlink the node from a multiple node list.  */
        (node -> cs_previous) -> cs_next =  node -> cs_next;
        (node -> cs_next) -> cs_previous =  node -> cs_previous;
        
        /* Check to see if the node to delete is at the head of the
           list. */
        if (node == *head)
        
            /* Move the head pointer to the node after.  */
            *head =  node -> cs_next;
    }
}

