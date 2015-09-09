/*************************************************************************/
/*                                                                       */
/*    Copyright (c)  1993 - 1999 Accelerated Technology, Inc.            */
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
/*      MEM.C                                           NET 4.0          */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Buffer management routines used by the network stack.               */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*  Uriah T. Pollock     02/04/98           Created Initial Version      */
/*  Uriah T. Pollock     01/18/99           Changed MEM_Buffer_Dequeue   */
/*                                           to zero out the correct     */
/*                                           fields since the buffer     */
/*                                           structure changed. Done to  */
/*                                           support the DEC21143 driver.*/
/*                                                                       */
/*************************************************************************/
#include "protocol.h"
#include "tcpdefs.h"
#include "socketd.h"
#include "data.h"
#include "tcp_errs.h"
#include "externs.h"
#include "types.h"
#include "pcidev.h"
#define UTL_Zero    bzero


/* Declare the NET pointers for holding incoming, and empty packet buffers.
   Also declare the buffer suspension list to hold tasks waiting for 
   buffers.  */
NET_BUFFER_HEADER           MEM_Buffer_List;
NET_BUFFER_HEADER           MEM_Buffer_Freelist;
NET_BUFFER_SUSPENSION_LIST  MEM_Buffer_Suspension_List;

/* Declare the suspension hisr. When buffers become available this HISR
   will wake up tasks that are suspended waiting for buffers. */
NU_HISR NET_Buffer_Suspension_HISR;

/* A global counter of the number of buffers that are currently allocated.
 * Initialized in MEM_Init. */
uint16 MEM_Buffers_Used;

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MEM_Init                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function initializes the Memory buffer component of         */
/*      Nucleus NET.                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      netinit                         Handles all the intialization to */
/*                                      bring up the network connection. */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory              Allocate Memory.                 */          
/*      normalize_ptr                   Returns a normalized pointer.    */
/*      NU_TCP_Log_Error                Logs Errors.                     */
/*      MEM_Buffer_Enqueue              Insert an item at the end of a   */
/*                                      linked list.                     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*      NAME             DATE              REMARKS                       */
/*                                                                       */
/*                                   This function used to be called     */
/*                                   initbuffer and was found in net.c   */
/*  Glen Johnson        03/06/98     Moved this function to MEM.C and    */
/*                                   renamed it.                         */
/*                                                                       */
/*************************************************************************/
STATUS MEM_Init(VOID)
{
    static int16    i;
    char HUGE       *ptr;
    STATUS          ret_status;


    /* Initialize the global buffer pointers. */
    MEM_Buffer_List.head            = NU_NULL;
    MEM_Buffer_List.tail            = NU_NULL;
    MEM_Buffer_Freelist.head        = NU_NULL;
    MEM_Buffer_Freelist.tail        = NU_NULL;
    MEM_Buffer_Suspension_List.head = NU_NULL;
    MEM_Buffer_Suspension_List.tail = NU_NULL;

    /* Break the block of memory up and place each block into the
     * buffer_freelist.
     */

    for (i = 0; i < MAXBUFFERS; i++)
    {

        ret_status = NU_Allocate_Memory(&System_Memory, (VOID **)&ptr,
                                (UNSIGNED)(sizeof(NET_BUFFER)) + 32,
                                (UNSIGNED)NU_NO_SUSPEND);

		ptr = (void *)ALIGN((uint32)ptr, 32);

        if (ret_status != NU_SUCCESS)
        {
            NU_Tcp_Log_Error (TCP_NO_MEMORY, TCP_FATAL, __FILE__, __LINE__);
            return (-1);
        }

        MEM_Buffer_Enqueue(&MEM_Buffer_Freelist, (NET_BUFFER *)ptr);
       
    }

    /* Initialize the number of buffers that are currently allocated. */
    MEM_Buffers_Used = 0;


    /* Allocate memory for the buffer suspension HISR. */
    ret_status = NU_Allocate_Memory (&System_Memory, (VOID **)&ptr, 
                                    1000, NU_NO_SUSPEND);

    /* Did we get the memory? */
    if (ret_status == NU_SUCCESS)
    {
        /* Create the HISR. */
        ret_status = NU_Create_HISR (&NET_Buffer_Suspension_HISR, "buf_susp", 
                           MEM_Buffer_Suspension_HISR, 2, ptr, 1000);
    }
    
    
    return (ret_status);

}  /* MEM_Init */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MEM_Buffer_Dequeue                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Remove and return the first node in a linked list.               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ICMP_Interpret                  Interprets an ICMP request.      */
/*      ICMP_Send_Error                 Sends an ICMP packet.            */
/*      IP_Interpret                    Interprets an IP packet.         */ 
/*      IP_Fragment                     Handles IP fragmentation.        */
/*      IP_Reassemble                   Handles IP Reassembly.           */
/*      ARP_Build_Pkt                   Builds ARP packet.               */
/*      UDP_Interpret                   Process incoming UDP packet.     */
/*      tcpresetfin                     Build TCP reset fin packet.      */
/*      tcp_sendack                     Build TCP ACK packet.            */
/*      tcp_ooo_packet                  Handles TCP out of Order packets.*/
/*      send_syn_fin                    Sends a TCP Syn fin packet.      */
/*      windowprobe                     Performs a windowprobe operation.*/
/*      MEM_Buffer_Chain_Dequeue        Dequeue a linked chain of buffers*/
/*      MEM_Update_Buffer_Lists         Remove the first node from the   */
/*                                      source list and places it at the */
/*                                      tail of the destination list.    */
/*      MEM_Buffer_Chain_Free           Removes the first node and each  */
/*                                      one in the chain from the source */
/*                                      list and places them at the tail */
/*                                      of the destination list as       */
/*                                      individual nodes.                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      UTL_Zero                        Zeroes out a Variable.           */
/*      NU_Control_Interrupts           Turns interrupts on or off.      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*      NAME             DATE              REMARKS                       */
/*                                                                       */
/*  Uriah T. Pollock   12/23/97      Created Initial Version             */
/*                                                                       */
/*************************************************************************/
NET_BUFFER *MEM_Buffer_Dequeue(NET_BUFFER_HEADER *hdr)
{
    NET_BUFFER  *node;
    INT         old_level;

    /*  Temporarily lockout interrupts to protect the global buffer variables. */
    old_level = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    /* If there is a node in the list we want to remove it. */
    if (hdr->head)
    {
        /* Get the node to be removed */
        node = hdr->head;

        /* Make the hdr point the second node in the list */
        hdr->head = node->next;

        /* If this is the last node the headers tail pointer needs to be nulled
           Also we do not need to clear the nodes next since it is already null */
        if (!(hdr->head))
            hdr->tail = NU_NULL;

        /* Is a buffer being removed from the buffer_freelist.  If so increment
           the buffers_used counter and clear the buffer header. */
        if (hdr == &MEM_Buffer_Freelist)
        {
            /* Zero the header info. */
            UTL_Zero ((char *)(&node->me_data.me_pkthdr.me_buf_hdr),
                                                sizeof(struct _me_bufhdr));

            /* Zero the pointers. */
            UTL_Zero ((char *)(&node->next), 16);

            MEM_Buffers_Used++;
        }
    }
    else
        node = NU_NULL;

    /*  Restore the previous interrupt lockout level.  */
    NU_Control_Interrupts(old_level);

    /* Return a pointer to the removed node */
    return(node);
}  /* end MEM_Buffer_Dequeue */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MEM_Buffer_Enqueue                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Insert an item at the end of a linked list.                      */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IGMP_Send                                                        */
/*      MEM_Update_Buffer_Lists                                          */
/*      MEM_BUffer_Chain_Free                                            */
/*      MEM_One_Buffer_Chain_Free                                        */
/*      UDP_Append                                                       */
/*      IP_Forward                                                       */
/*      IP_Reassembly                                                    */
/*      NET_Ether_Send                                                   */
/*      tcpresetfin                                                      */
/*      tcp_sendack                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Control_Interrupts                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*      NAME             DATE              REMARKS                       */
/*                                                                       */
/*  Uriah T. Pollock   12/23/97      Created Initial Version             */
/*                                                                       */
/*************************************************************************/
NET_BUFFER *MEM_Buffer_Enqueue(NET_BUFFER_HEADER *hdr, NET_BUFFER *item)
{
    INT old_level;

    /*  Temporarily lockout interrupts to protect the global buffer variables. */
    old_level = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    /* Set node's next to point at NULL */
    item->next = NU_NULL;

    /*  If there is currently a node in the linked list, we want to add the
        new node to the end. */
    if (hdr->head) 
    {
        /* Make the last node's next point to the new node. */
        hdr->tail->next = item;
       
        /* Make the roots tail point to the new node */
        hdr->tail = item;
    }
    /* If the linked list was empty, we want both the root's head and
       tial to point to the new node. */
    else 
    {
        hdr->head = item;
        hdr->tail = item;
    }

    /* If a buffer is being moved back onto the buffer free list, then decrement
       the the number of buffers that are currently used. */
    if (hdr == &MEM_Buffer_Freelist)
    {
        MEM_Buffers_Used--;
    }

    /*  Restore the previous interrupt lockout level.  */
    NU_Control_Interrupts(old_level);

  return(item);
} /* end MEM_Bufffer_Enqueue */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MEM_Buffer_Chain_Dequeue                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Dequeue a linked chain of buffer(s) large enough to hold the     */
/* number of bytes of packet data.                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      NU_Dhcp                                                          */
/*      NU_Dhcp_Release                                                  */
/*      NU_Bootp                                                         */
/*      IGMP_Send                                                        */
/*      netusend                                                         */
/*      IP_Fragment                                                      */
/*      netwrite                                                         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Control_Interrupts                                            */
/*      MEM_Buffer_Dequeue                                               */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*      NAME             DATE              REMARKS                       */
/*                                                                       */
/*  Uriah T. Pollock   12/26/97      Created Initial Version             */
/*                                                                       */
/*************************************************************************/
NET_BUFFER *MEM_Buffer_Chain_Dequeue (NET_BUFFER_HEADER *header, INT nbytes)
{
    NET_BUFFER *ret_buf_ptr, *work_buf_ptr;
    int         x, num_bufs;
    INT         old_level;

    /*  Temporarily lockout interrupts to protect the global buffer variables. */
    old_level = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    /* Go ahead and dequeue the parent buffer of the chain. */
    ret_buf_ptr = MEM_Buffer_Dequeue (header);

    /* Make sure we got a buffer. */
    if (ret_buf_ptr != NU_NULL)
    {

        /* NULL the pointer */
        ret_buf_ptr->next           = NU_NULL;
        ret_buf_ptr->next_buffer    = NU_NULL;

        /* Check to see if we need to chain some buffers together. */
        num_bufs = (nbytes + (sizeof(struct _me_bufhdr) - 1))/ NET_MAX_BUFFER_SIZE;

        /* If we need more get the first one */
        if (num_bufs)
        {
            ret_buf_ptr->next_buffer = work_buf_ptr = MEM_Buffer_Dequeue (header);

            /* Make sure we got a buffer. If not, the ones we did get will be put
               back on the buffer freelist and NULL returned to caller. */
            if (work_buf_ptr != NU_NULL)
            {

                /* Now get the rest and link them together. */
                for (x = 1; x < num_bufs; x++)
                {
                    /* Dequeue a buffer and link it to the buffer chain. */
                    work_buf_ptr->next_buffer   = MEM_Buffer_Dequeue (header);

                    /* Make sure we got a buffer. If not, the ones we did get will be put
                       back on the buffer freelist and NULL returned to caller. */
                    if (work_buf_ptr->next_buffer != NU_NULL)
                    {
                        work_buf_ptr->next      = NU_NULL;

                        /* Move the work pointer to the next buffer. */
                        work_buf_ptr = work_buf_ptr->next_buffer;
                    }
                    else
                    {
                        /* Give the buffers back. */
                        MEM_One_Buffer_Chain_Free (ret_buf_ptr, &MEM_Buffer_Freelist);

                        /* Null the return pointer. */
                        ret_buf_ptr = NU_NULL;

                        /* Get out of the for loop */
                        x = num_bufs;
                    }

                }

                /* Make sure we are not falling through because of lack of buffers. */
                if (ret_buf_ptr != NU_NULL)
                {
                    /* Null the end of the chain. */
                    work_buf_ptr->next_buffer   = NU_NULL;
                    work_buf_ptr->next          = NU_NULL;
                }
            }
            else
            {
                /* Give the buffers back. */
                MEM_One_Buffer_Chain_Free (ret_buf_ptr, &MEM_Buffer_Freelist);

                /* Null the return pointer. */
                ret_buf_ptr = NU_NULL;
            }
        }
    }

    /*  Restore the previous interrupt lockout level.  */
    NU_Control_Interrupts(old_level);

    /* Return the head off the buffer chain. */
    return (ret_buf_ptr);
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  MEM_Update_Buffer_Lists                                                 */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function removes the first node from the source list and places    */
/*  it at the tail of the destination list. Then returns a pointer to the   */
/*  moved node.                                                             */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/* Uriah T. Pollock                                                         */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*      tcp_ooo_packet                                                      */
/*      check_ooo_list                                                      */
/*      enqueue                                                             */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*      NU_Control_Interrupts                                               */
/*      MEM_Buffer_Dequeue                                                  */
/*      MEM_Buffer_Enqueue                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*     source - a pointer to the list from which the node will be removed.  */
/*     dest   - a pointer to the list to which the node will be placed.     */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*     tmp_ptr - pointer to the node that was moved.                        */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME             DATE              REMARKS                          */
/*                                                                          */
/*  Uriah T. Pollock   12/29/97      Created Initial Version                */
/*                                                                          */
/****************************************************************************/
NET_BUFFER *MEM_Update_Buffer_Lists (NET_BUFFER_HEADER *source, 
                                     NET_BUFFER_HEADER *dest)
{
    NET_BUFFER          *tmp_ptr;
    INT                 old_level;

    /*  Temporarily lockout interrupts to protect the global buffer variables. */
    old_level = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    /* Get a pointer to the lists. */
    tmp_ptr = MEM_Buffer_Dequeue(source);

    /* Make sure there was a node to move. */
    if (tmp_ptr != NU_NULL)
        MEM_Buffer_Enqueue(dest, tmp_ptr);

    /*  Restore the previous interrupt lockout level.  */
    NU_Control_Interrupts(old_level);

    return(tmp_ptr);
}  /* end MEM_Update_Buffer_Lists routine */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  MEM_Buffer_Chain_Free                                                   */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function removes the first node and each one in the chain from the */
/*  source list and places them at the tail of the destination list as      */
/*  individual nodes, not a chain.                                          */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/* Uriah T. Pollock                                                         */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*      ICMP_Interpret                                                      */
/*      IGMP_Interpret                                                      */
/*      neturead                                                            */
/*      udpinterpret                                                        */
/*      IP_Interpret                                                        */
/*      IP_Reassembly                                                       */
/*      NET_Ether_Input                                                     */
/*      TCP_Interpret                                                       */
/*      tcpdo                                                               */
/*      estab1986                                                           */
/*      dequeue                                                             */
/*      rmqueue                                                             */
/*      MEM_Buffer_Cleanup                                                  */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*     NU_Control_Interrupts                                                */
/*     MEM_Buffer_Enqueue                                                   */
/*     MEM_Buffer_Dequeue                                                   */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*     source - a pointer to the list from which the node will be removed.  */
/*     dest   - a pointer to the list to which the node will be placed.     */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*     none                                                                 */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME             DATE              REMARKS                          */
/*                                                                          */
/*  Uriah T. Pollock   12/29/97      Created Initial Version                */
/*                                                                          */
/****************************************************************************/
VOID MEM_Buffer_Chain_Free (NET_BUFFER_HEADER *source, NET_BUFFER_HEADER  *dest)
{
    NET_BUFFER  *tmp_ptr;
    INT         old_level;

    /*  Temporarily lockout interrupts to protect the global buffer variables. */
    old_level = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    tmp_ptr = MEM_Buffer_Dequeue(source);

    /* Go through the entire buffer chain moving each buffer to the 
       destination list */
    while (tmp_ptr != NU_NULL)
    {
        /* Put one part of the chain onto the destination list */
        MEM_Buffer_Enqueue(dest, tmp_ptr);

        /* Move to the next buffer in the chain */
        tmp_ptr = tmp_ptr->next_buffer;
    }

    /*  Restore the previous interrupt lockout level.  */
    NU_Control_Interrupts(old_level);

    /* If there are enough free buffers and a task is waiting for 
       buffers then activate a HISR to resume the task. */
    if (MEM_Buffer_Suspension_List.head && 
                ((MAXBUFFERS - MEM_Buffers_Used) > NET_FREE_BUFFER_THRESHOLD))
            NU_Activate_HISR (&NET_Buffer_Suspension_HISR);


}  /* end MEM_Buffer_Chain_Free routine */



/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  MEM_One_Buffer_Chain_Free                                               */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function removes all nodes in the source chain and puts them  at   */
/*  the end of the destination list, if the destination list is the buffer  */
/*  freelist. Otherwise it just puts the source node to the end of the dest */
/*  list.                                                                   */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/* Uriah T. Pollock                                                         */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*      ICMP_Reflect                                                        */
/*      IP_Fragment                                                         */
/*      IP_Free_Qeueue_Element                                              */
/*      IP_Reassembly_Event                                                 */
/*      ARP_Event                                                           */
/*      NET_Ether_Send                                                      */
/*      tcpsend                                                             */
/*      tcp_retransmit                                                      */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*     NU_Control_Interrupts                                                */
/*     MEM_Buffer_Enqueue                                                   */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*     source - a pointer to the buffer chain that will be removed.         */
/*     dest   - a pointer to the list to which the node(s) will be placed.  */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*     none                                                                 */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME             DATE              REMARKS                          */
/*                                                                          */
/*  Uriah T. Pollock   12/29/97      Created Initial Version                */
/*                                                                          */
/****************************************************************************/
VOID MEM_One_Buffer_Chain_Free (NET_BUFFER *source, NET_BUFFER_HEADER *dest)
{
    INT         old_level;

    /*  Temporarily lockout interrupts to protect the global buffer variables. */
    old_level = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    /* Only deallocate all the buffers in the chain if the destination list
       is the buffer free list. Otherwise just move the parent buffer to
       the end of the dest list. */
    if (dest == &MEM_Buffer_Freelist)
    {
        /* Go through the entire buffer chain moving each buffer to the 
           destination list */
        while (source != NU_NULL)
        {
            /* Put one part of the chain onto the destination list */
            MEM_Buffer_Enqueue(dest, source);

            /* Move to the next buffer in the chain */
            source = source->next_buffer;
        }
    }
    else
        MEM_Buffer_Enqueue(dest, source);
    
    /*  Restore the previous interrupt lockout level.  */
    NU_Control_Interrupts(old_level);

    /* If there are enough free buffers and a task is waiting for 
       buffers then activate a HISR to resume the task. */
    if (MEM_Buffer_Suspension_List.head && 
                ((MAXBUFFERS - MEM_Buffers_Used) > NET_FREE_BUFFER_THRESHOLD))
            NU_Activate_HISR (&NET_Buffer_Suspension_HISR);

}  /* end MEM_One_Buffer_Chain_Free routine */

/* The next two functions are only used by the IP reassembly code. */
#if INCLUDE_IP_REASSEMBLY

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  MEM_Cat                                                                 */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function concatenates the source buffer chain and the destination  */
/*  buffer chain.                                                           */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/* Glen Johnson                                                             */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*      IP_Reassembly                                                       */
/*      IP_Reassembly_Event                                                 */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*     n - the source chain. will be added to the end of the destination    */
/*         chain.                                                           */
/*     m - the destination chain                                            */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*     none                                                                 */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME             DATE              REMARKS                          */
/*                                                                          */
/*  Glen Johnson       01/16/97      Created Initial Version                */
/*                                                                          */
/****************************************************************************/
VOID MEM_Cat (NET_BUFFER *dest, NET_BUFFER *src)
{

    NET_BUFFER  *dest_work;
        
    /* Get work pointers */
    dest_work = dest;

    /* Find the last buffer in the chain. */
    while (dest_work->next_buffer)
        dest_work = dest_work->next_buffer;

    /* Add the src chain to the destination chain. */
    dest_work->next_buffer = src;

} /* MEM_Cat */
#endif /* INCLUDE_IP_REASSEMBLY */

#if INCLUDE_IP_REASSEMBLY || INCLUDE_IP_FRAGMENT

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  MEM_Trim                                                                */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function trims data from the start of an buffer chain.             */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/* Glen Johnson                                                             */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*      IP_Reassembly                                                       */
/*      IP_Fragment                                                         */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*     NU_Control_Interrupts                                                */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*     n - the source chain. will be added to the end of the destination    */
/*         chain.                                                           */
/*     m - the destination chain                                            */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*     none                                                                 */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME             DATE              REMARKS                          */
/*                                                                          */
/*  Glen Johnson       01/16/97      Created Initial Version                */
/*                                                                          */
/****************************************************************************/
VOID MEM_Trim (NET_BUFFER *buf_ptr, INT length)
{
    INT         t_len = length;
    NET_BUFFER  *m;
    INT         count;
    INT         old_level;

    /* Perform some basic error checking. */
    if ( ((m = buf_ptr) == NU_NULL))
        return;

    /*  Temporarily lockout interrupts to protect the global buffer variables. */
    old_level = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    /* If length is greater than 0 then trim data from the start. */
    if (t_len >= 0)
    {
        /* Start with the first buffer in the chain and remove data as necessary. */
        while (m != NU_NULL && t_len > 0)
        {
            if (m->data_len <= (uint32)t_len)
            {
                t_len -= m->data_len;
                m->data_len = 0;
                m = m->next;
            }
            else
            {
                m->data_len -= t_len;
                m->data_ptr += t_len;
                t_len = 0;
            }
        }

        /* Update the total number of bytes in this packet. */
        buf_ptr->mem_total_data_len -= (length - t_len);
    }
    else
    {
        t_len = -t_len;
        count = 0;

        /* Get a count of the total number of bytes in this chain. */
        for(;;)
        {
            count += m->data_len;
            if(m->next_buffer == NU_NULL)
                break;
            m = m->next_buffer;
        }

        /* If the adjustment only affects the last buffer in the chain, make the 
           adjustment and return. */
        if (m->data_len >= (uint32)t_len)
        {
            m->data_len -= t_len;
            buf_ptr->mem_total_data_len -= t_len;

            /*  Restore the previous interrupt lockout level.  */
            NU_Control_Interrupts(old_level);
            return;
        }

        count -= t_len;
        if (count < 0)
            count = 0;

        /* The correct length for the chain is "count". */
        m = buf_ptr;
        m->mem_total_data_len = count;

        /* Find the buffer with the last data.  Adjust its length. */ 
        for(; m ; m = m->next_buffer)
        {
            if (m->data_len >= (uint32)count)
            {
                m->data_len = count;
                break;
            }
            count -= m->data_len;
        }

        /* Toss out the data from the remaining buffers. */
        while (m = m->next_buffer)
            m->data_len = 0;
    }

    /*  Restore the previous interrupt lockout level.  */
    NU_Control_Interrupts(old_level);

} /* MEM_Trim */

#endif /* INCLUDE_IP_REASSEMBLY || INCLUDE_IP_FRAGMENT */


#if INCLUDE_IP_FRAGMENT
/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  MEM_Chain_Copy                                                          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function copies a memory chain.                                    */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/* Glen Johnson                                                             */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*      IP_Fragment                                                         */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME             DATE              REMARKS                          */
/*                                                                          */
/*  Glen Johnson       01/16/97      Created Initial Version                */
/*  Patti Hill		   03/30/98      Now considers IP Header                */
/*  Patti Hill         04/06/98      Now considers copying when the source  */
/*                                   is buffer chained                      */
/*                                                                          */
/****************************************************************************/

VOID MEM_Chain_Copy(NET_BUFFER *dest, NET_BUFFER *src, INT off, INT len)
{
    NET_BUFFER      *s = src;
    NET_BUFFER      *d = dest;
    INT             first_buffer = 0;
    INT             bytes_to_copy = 0;
    INT             data_off = 0;
    INT             max_buf_size = 0;

    if ((off < 0) || (len < 0) || (d == NU_NULL) || (s == NU_NULL))
        return;

    /* First we need to find the buffer within the chain where the first byte 
       specified by offset is found. */
    while (off > 0)
    {
        /* Is this the one. */
        if (off < (INT)s->data_len)
            break;

        /* Now check the next one. */
		off -= s->data_len;
        s = s->next_buffer;
    }

	d->data_len = sizeof(IPLAYER);

    while ((len > 0) && d && s)
    {
        /* Choose the min of the data in the source and the total data length 
           we wish to copy. */
        if (len < (INT)s->data_len - off)
        {
			/* Set the maximum buffer size */
			if(first_buffer == 0)
			{
				max_buf_size = NET_PARENT_BUFFER_SIZE;
			}
			else
			{
				max_buf_size = NET_MAX_BUFFER_SIZE;
			}

			/* Check to see if data is already in buffer */
			if(d->data_len > 0)
			{
				/* Set the offset past the data already in the buffer */
				data_off = d->data_len;

				/* Ensure that the data being copied in does not exceed */
				/* the max buffer size */
				if((d->data_len + len) > (uint32)max_buf_size)
				{        
					/* Set the bytes to copy and the data_len */
					bytes_to_copy = max_buf_size - d->data_len;
					d->data_len = max_buf_size;
				}
				else
				{
					/* Set the bytes to copy and the data_len */
					d->data_len = d->data_len + len;
					bytes_to_copy = len;
				}
			}
			else
			{
				/* Set the bytes to copy and the data_len */
				d->data_len = len;
				bytes_to_copy = d->data_len;
			}
        }
        else
        {
			/* Check to see if there is data already in the buffer */
			if(d->data_len > 0)
			{
				/* Set the offset past the data in the buffer */
				data_off = d->data_len;

				/* Set the max buffer size */
				if(first_buffer == 0)
					max_buf_size = NET_PARENT_BUFFER_SIZE;
				else
					max_buf_size = NET_MAX_BUFFER_SIZE;

				/* Ensure that the data being copied in does not exceed */
				/* the max buffer size */
				if((d->data_len + s->data_len) > (uint32)max_buf_size)
				{
					/* Set the bytes to copy and the data_len */
					bytes_to_copy = max_buf_size - d->data_len;
					if((s->data_len - off) < (uint32)bytes_to_copy)
					{
						bytes_to_copy = s->data_len - off;
					}
					d->data_len = d->data_len + bytes_to_copy;
				}
				else
				{
					/* Set the bytes to copy and the data_len */
					bytes_to_copy = s->data_len;
					if((s->data_len - off) < (uint32)bytes_to_copy)
					{
						bytes_to_copy = s->data_len - off;
					}
					d->data_len = d->data_len + bytes_to_copy;
				}
			}
			else
			{
				/* Set the bytes to copy and the data_len */
				d->data_len = (s->data_len - off);
				bytes_to_copy = d->data_len;
			}
        }

		/* Decrement the total data left to copy in */
		len -= bytes_to_copy;
		
		/* Copy the data to the destination */
		memcpy(d->data_ptr + data_off, s->data_ptr + off, bytes_to_copy);

		/* Set the max buffer size */
		if(first_buffer == 0)
			max_buf_size = NET_PARENT_BUFFER_SIZE;
		else
			max_buf_size = NET_MAX_BUFFER_SIZE;

		/* Done copying data for this fragment */
		if(len == 0)
		{
			d->next_buffer = NU_NULL;
        }
		else if(d->data_len >= (uint32)max_buf_size)
		{
			/* Advance to next dest buffer */
			d = d->next_buffer;
			d->data_ptr = d->mem_packet;
			d->data_len = 0;
			first_buffer = 1;
		}

		/* Advance to next source buffer */
		if((uint32)(off + bytes_to_copy) >= s->data_len)
		{
			s = s->next_buffer;
			off = 0;
		}
		else
		{
			off = off + bytes_to_copy;
		}

		data_off = 0;
              
	}

} /* MEM_Chain_Copy */

#endif /* INCLUDE_IP_FRAGMENT */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MEM_Buffer_Remove                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Removes a node from a buffer list.                                  */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      tcp_retranmsit                                                   */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/*************************************************************************/
VOID MEM_Buffer_Remove (NET_BUFFER_HEADER *hdr, NET_BUFFER *item)
{
    NET_BUFFER  *ent, *pre_ent;


    /*  Search the linked list until item is found or the end of the list
     *  is reached.
     */
    for (ent = hdr->head;((ent) && (ent != item)); pre_ent = ent, ent = ent->next)
        ;

    /* Make sure the item we are looking for was found. */
    if (ent)
    {

        /* If we're deleting the list head, this is just a dequeue operation */
        if (hdr->head == item)
            MEM_Buffer_Dequeue(hdr);
        else
            /*  If we are deleting the list tail, we need to reset the tail pointer
             *  and make the new tail point forward to 0.
             */
            if (hdr->tail == item) 
            {
                hdr->tail = pre_ent;
                hdr->tail->next = NU_NULL;
            }
            else  /* We're removing this entry from the middle of the list */
                pre_ent->next = item->next;
    }

}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MEM_Buffer_Cleanup                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   This function takes a list header and moves all of the buffers off  */
/*   of the list and onto the MEM_Buffer_Freelist.  Mainly used to       */
/*   deallocate any unprocessed buffers when ever a TCP connection is    */
/*   closed.                                                             */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      TCP_Cleanup                                                      */
/*      netclose                                                         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      MEM_Buffer_Chain_Free                                            */
/*                                                                       */
/* INPUTS                                                                */
/*      hdr             pointer to the head of a linked list.            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*     U. Pollock        02/02/98            Created initial version.    */
/*                                                                       */
/*************************************************************************/
VOID MEM_Buffer_Cleanup (NET_BUFFER_HEADER *hdr)
{
    while(hdr->head)
        MEM_Buffer_Chain_Free (hdr, &MEM_Buffer_Freelist);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MEM_Buffer_Insert                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Insert an item into a linked list just before lpos and just after   */
/* fpos.                                                                 */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      tcp_ooo_packet                                                   */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/*************************************************************************/
NET_BUFFER *MEM_Buffer_Insert(NET_BUFFER_HEADER *hdr, NET_BUFFER *item, 
                              NET_BUFFER *lpos, NET_BUFFER *fpos)
{
    /* Make item's next point to lpos */
    item->next = lpos;

    /* If lpos was the first node in the linked list.  We need to make
     * hdr's head point to item, which is the new first node.
    */
    if (lpos == hdr->head)
        hdr->head = item;
    else
        /* Make fpos point to item. */
        fpos->next = item;
    
    /* If fpos is the tail of the list then we neec to update the 
       headers tail pointer to point at item. */
    if (fpos == hdr->tail)
        hdr->tail = item;

    return(item);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MEM_Buffer_Suspension_HISR                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   This function resumes the first task on the buffer suspension list. */
/* It then removes that tasks entry from the list.                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Activated by the buffer chain free routines.                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Resume_Task                                                   */
/*      dll_dequeue                                                      */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*     U. Pollock        02/02/98            Created initial version.    */
/*                                                                       */
/*************************************************************************/
VOID MEM_Buffer_Suspension_HISR (VOID)
{

    /* Make sure there is really a task to wake up. */
    if (MEM_Buffer_Suspension_List.head)
    {
        /* Resume the task. */
        NU_Resume_Task (MEM_Buffer_Suspension_List.head->waiting_task);

        /* Remove this element from the suspension list. */
        dll_dequeue (&MEM_Buffer_Suspension_List);
    }

}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MEM_Buffer_Enqueue_nocrit                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Insert an item at the end of a linked list without disabling     */
/*      the interrupt.                                                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*		DEC21143_RX_Packet                                               */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*      NAME             DATE              REMARKS                       */
/*                                                                       */
/*  Sudhir Kasargod   06/07/99      Created Initial Version              */
/*                                                                       */
/*************************************************************************/
NET_BUFFER *MEM_Buffer_Enqueue_nocrit(NET_BUFFER_HEADER *hdr, NET_BUFFER *item)
{

    /* Set node's next to point at NULL */
    item->next = NU_NULL;

    /*  If there is currently a node in the linked list, we want to add the
        new node to the end. */
    if (hdr->head) 
    {
        /* Make the last node's next point to the new node. */
        hdr->tail->next = item;
       
        /* Make the roots tail point to the new node */
        hdr->tail = item;
    }
    /* If the linked list was empty, we want both the root's head and
       tial to point to the new node. */
    else 
    {
        hdr->head = item;
        hdr->tail = item;
    }

    /* If a buffer is being moved back onto the buffer free list, then decrement
       the the number of buffers that are currently used. */
    if (hdr == &MEM_Buffer_Freelist)
    {
        MEM_Buffers_Used--;
    }

  return(item);
} /* end MEM_Bufffer_Enqueue */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MEM_Buffer_Dequeue_nocrit                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Remove and return the first node in a linked list without        */
/*      disabling the interrupt.                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*		DEC21143_RX_Packet                                               */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      UTL_Zero                        Zeroes out a Variable.           */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*      NAME             DATE              REMARKS                       */
/*                                                                       */
/*  Sudhir Kasargod   06/07/99      Created Initial Version              */
/*                                                                       */
/*************************************************************************/
NET_BUFFER *MEM_Buffer_Dequeue_nocrit(NET_BUFFER_HEADER *hdr)
{
    NET_BUFFER  *node;


    /* If there is a node in the list we want to remove it. */
    if (hdr->head)
    {
        /* Get the node to be removed */
        node = hdr->head;

        /* Make the hdr point the second node in the list */
        hdr->head = node->next;

        /* If this is the last node the headers tail pointer needs to be nulled
           Also we do not need to clear the nodes next since it is already null */
        if (!(hdr->head))
            hdr->tail = NU_NULL;

        /* Is a buffer being removed from the buffer_freelist.  If so increment
           the buffers_used counter and clear the buffer header. */
        if (hdr == &MEM_Buffer_Freelist)
        {
            /* Zero the header info. */
            UTL_Zero ((char *)(&node->me_data.me_pkthdr.me_buf_hdr),
                                                sizeof(struct _me_bufhdr));

            /* Zero the pointers. */
            UTL_Zero ((char *)(&node->next), 16);

            MEM_Buffers_Used++;
        }
    }
    else
        node = NU_NULL;

    /* Return a pointer to the removed node */
    return(node);
}  /* end MEM_Buffer_Dequeue */
