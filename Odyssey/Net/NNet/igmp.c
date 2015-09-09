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
/*      IGMP.C                                            4.0            */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      IGMP - Internet Group Management Protocol                        */        
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the IGMP (Internet Group Management Protocol) */
/*      module. IGMP provides the means for hosts to notify routers      */
/*      of the multicasting groups that the host belongs to.             */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      <Data Structure> - <Description>                                 */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      IGMP_Interpret                  Processes received IGMP packets. */
/*      IGMP_Initialize                 Joins the all hosts group on an  */
/*                                      IP multiccasting capable device. */
/*      IGMP_Leave                      Kills any timer events that are  */
/*                                      pending when membership in a     */
/*                                      multicast gorup is dropped.      */
/*      IGMP_Random_Delay               Caclulates a Random delay.       */
/*      IGMP_Send                       Sends an IGMP group membership   */
/*                                      report.                          */
/*      IGMP_Join                       Sends a multicast group          */
/*                                      membership report.               */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      <Dependency>                                                     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/16/98       Created initial version           */
/*                                                                       */
/*************************************************************************/
#include "nucleus.h"
#include "target.h"
#include "externs.h"
#include "ip.h"
#include "igmp.h"
#include "netevent.h"

/* If Multicasting is not desired, then this file should not be compiled 
   into the Nucleus NET library. */
#if INCLUDE_IP_MULTICASTING
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IGMP_Interpret                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function processes receieved IGMP packets.                  */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Interpret                    Interprets an IP packet.         */ 
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      IGMP_Random_Delay               Caclulates a Random delay.       */
/*      IP_Checklen                     Computes the IP checksum.        */
/*      IP_Lookup_Multi                 Lookup the IP address in a       */
/*                                      multicast group.                 */
/*      longswap                        Swaps 4 bytes of a long number   */
/*      MEM_Buffer_Chain_Free           Removes the first node and each  */
/*                                      one in the chain from the source */
/*                                      list and places them at the tail */
/*                                      of the destination list as       */
/*                                      individual nodes.                */
/*      UTL_Timerunset                  Remove all timer events from the */
/*                                      queue that match.                */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      buf_ptr         Pointer to a buffer containg the IGMP packet.    */
/*      hlen            Length of the IP header                          */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      <Outputs>                           <Description>                */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/16/98       Created initial version           */
/*************************************************************************/
STATUS IGMP_Interpret(NET_BUFFER *buf_ptr, INT hlen)
{
    INT         igmp_len;
    IGMP_LAYER  *igmp;
    IPLAYER     *ip;
    uint16      cksum;
    IP_MULTI    *ipm;

    igmp_len = buf_ptr->mem_total_data_len;

    if (igmp_len != sizeof (IGMP_LAYER))
    {
        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
        return -1;
    }

    igmp = (IGMP_LAYER *)buf_ptr->data_ptr;
    ip = (IPLAYER *)(buf_ptr->data_ptr - hlen);

    cksum = igmp->igmp_cksum;
    igmp->igmp_cksum = 0;

    if ( cksum != IP_Checklen ((int8 *)igmp, sizeof(IGMP_LAYER)) )
    {
        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
        return -1;
    }

    switch (igmp->igmp_type)
    {
    
    case IGMP_HOST_MEMBERSHIP_QUERY :
        
        /* All queries should be addressed to the all hosts group. */
        if (*(uint32 *)ip->ipdest != longswap(IGMP_ALL_HOSTS_GROUP))
        {
            MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
            return -1;
        }

        /* Step through all of the multicast groups that the receive interface 
           is a member of. */
        for ( ipm = buf_ptr->mem_buf_device->dev_addr.dev_multiaddrs;
              ipm != NU_NULL;
              ipm = ipm->ipm_next)
        {
            /* Don't send a report if for the all hosts group or if a timer is 
               already set up. */
            if ( (ipm->ipm_device == buf_ptr->mem_buf_device) &&
                 (ipm->ipm_timer == 0) &&
                 (ipm->ipm_addr != longswap(IGMP_ALL_HOSTS_GROUP)) )
            {
                ipm->ipm_timer = IGMP_Random_Delay(ipm);
            }

        }
        break;

    case IGMP_HOST_MEMBERSHIP_REPORT :
        
        /* Make sure the multicast group is valid. */
        if ( !IP_MULTICAST_ADDR(longswap(igmp->igmp_group)) ||
             (igmp->igmp_group != *(uint32 *)ip->ipdest ) )
        {
            MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
            return -1;
        }

        /* See if the receive interface is a member of the group. */
        ipm = IP_Lookup_Multi(igmp->igmp_group, &buf_ptr->mem_buf_device->dev_addr);

        /* If the interface is a member of the group and a report timer 
           currently exists, clear the timer. */
        if (ipm && ipm->ipm_timer)
        {
            ipm->ipm_timer = 0;
            UTL_Timerunset (EV_IGMP_REPORT, (uint32)ipm, 1);
        }
        break;

    default :

        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
        break;
    }

    /* Free the buffer. */
    MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

    return (NU_SUCCESS);

} /* IGMP_Interpret */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IGMP_Initialize                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function joins the all hosts group on an IP multicasting    */
/*      capable device.  All level two conforming hosts are required to  */
/*      join the all hosts group (224.0.0.1).                            */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      DEV_Attach_IP_to_Device        Attaches the IP address to a      */
/*                                     particular device.                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      IP_Add_Multi                   Adds an IP address to a mutlicast */
/*                                     group.                            */
/*      longswap                       Swaps 4 bytes of a long number    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      device          Pointer to the device to init IGMP on.           */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS      Successful completion of the operation.          */
/*      NU_INVAL        failure                                          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/
STATUS  IGMP_Initialize(DV_DEVICE_ENTRY *device)
{
    uint32      all_hosts_group = 0xE0000001; /* IP address 224.0.0.1 */

    /* Join the all hosts group on this device. Note that IP_Add_Multi will not
       report the membership in the all hosts group, and a report should not be
       sent here.  RFC 1112 specifies that membership in the all hosts group is
       never reported.
    */
    if (IP_Add_Multi(longswap(all_hosts_group), device) != NU_NULL)
        return NU_SUCCESS;
    else
        return NU_INVAL;

} /* IGMP_Initialize */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IGMP_Join                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function sends a multicast group membership report.  A timer*/
/*      event is created to send a second copy of the report.            */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Add_Multi                   Adds an IP address to a mutlicast */
/*                                     group.                            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      IGMP_Random_Delay              Caclulates a Random delay.        */
/*      IGMP_Send                      Sends an IGMP group membership    */
/*                                     report.                           */

/*      longswap                       Swaps 4 bytes of a long number    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      ipm         A pointer to the multicast group structure.          */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      <Outputs>                           <Description>                */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/16/98       Created initial version           */
/*************************************************************************/
VOID IGMP_Join(IP_MULTI *ipm)
{

	/* Membership in the all hosts group is not reported. It is assumed that all
       level 2 conforming multicasts hosts are members of this group. */
    /* NOTE: When loop back of multicast packets is supported, membership should 
       not be reported in that case either. The interface should be checked here.
    */
    if (ipm->ipm_addr == longswap(IGMP_ALL_HOSTS_GROUP))
    {
        ipm->ipm_timer = 0;
    }
    else
    {
        /* Send a report of the membership.*/
        IGMP_Send(ipm);

        /* Set up timer event to send the report again just in case something 
           goes wrong with the first report. */
        ipm->ipm_timer = IGMP_Random_Delay(ipm);
    }

} /* IGMP_Join */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IGMP_Send                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Send an IGMP group membership report.                            */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IGMP_Join                       Sends a multicast group          */
/*                                      membership report.               */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      IP_Checklen                     Computes the IP checksum.        */
/*      IP_Send                         Sends an IP packet.              */
/*      MEM_Buffer_Chain_Dequeue        Dequeues any incoming buffers.   */
/*      MEM_Buffer_Enqueue              Insert an item at the end of a   */
/*                                      linked list.                     */
/*      UTL_Zero                        Zeroes out a Variable.           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      ipm         A pointer to the multicast group structure.          */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      <Outputs>                           <Description>                */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/16/98       Created initial version           */
/*************************************************************************/
VOID IGMP_Send(IP_MULTI *ipm)
{
    NET_BUFFER          *buf_ptr;
    IGMP_LAYER          *igmp;
    IP_MULTI_OPTIONS    mopt;
    STATUS              status;
    
    /* Allocate a buffer chain to build the IGMP report in. */
    buf_ptr = MEM_Buffer_Chain_Dequeue ( &MEM_Buffer_Freelist, 
                                         NET_MAX_IGMP_HEADER_SIZE);
    if (buf_ptr == NU_NULL)
        return;

    /* Initialize the size of the data in this buffer. */
    buf_ptr->mem_total_data_len = buf_ptr->data_len = sizeof (IGMP_LAYER);

    /* Set the deallocation list pointer. */
    buf_ptr->mem_dlist = &MEM_Buffer_Freelist;

    /* Point to the location within the buffer where the IGMP header will 
       be placed. */
    buf_ptr->data_ptr = buf_ptr->mem_parent_packet + NET_MAX_IP_HEADER_SIZE;

    /* Overlay the IGMP header. */
    igmp = (IGMP_LAYER *)buf_ptr->data_ptr;

    /* Initialize the IGMP header. */
    igmp->igmp_type = IGMP_HOST_MEMBERSHIP_REPORT;
    igmp->igmp_unused = 0;
    igmp->igmp_group = ipm->ipm_addr;
    igmp->igmp_cksum = 0;
    igmp->igmp_cksum = IP_Checklen ((int8 *)igmp, sizeof(IGMP_LAYER));

    /* Set up the multicast options that will be passed to the IP Layer. */
    UTL_Zero(&mopt, sizeof(mopt));
    mopt.ipo_device = ipm->ipm_device;
    mopt.ipo_ttl = 1;
    mopt.ipo_loop = 0;

    /* Send the IGMP report. */
    status = IP_Send(buf_ptr, NU_NULL, ipm->ipm_addr, *(uint32 *)IP_Brd_Cast, 0, 
                        1, IP_IGMP_PROT, 0, &mopt);

    if ((status != NU_SUCCESS) && (status != NU_UNRESOLVED_ADDR))
    {
        /* The packet was not sent.  Dealocate the buffer.  If the packet was
           transmitted it will be deallocated later by TCP. */
        MEM_One_Buffer_Chain_Free (buf_ptr, &MEM_Buffer_Freelist);
    }
} /* IGMP_Send */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IGMP_Random_Delay                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Compute a "random" delay to be used for sending the next IGMP    */
/*      group membership report.                                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IGMP_Interpret                  Processes received IGMP packets. */
/*      IGMP_Join                       Sends a multicast group          */
/*                                      membership report.               */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Retrieve_Clock               Retrieves the clock.             */                             
/*      UTL_Timerset                    Sets an async timer and when time*/
/*                                      elapses it sticks an event in the*/
/*                                      network event queue.             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      ipm         A pointer to the multicast group structure.          */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      The length of time in ticks before the next report should be     */
/*      sent.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/16/98       Created initial version           */
/*************************************************************************/
uint32 IGMP_Random_Delay(IP_MULTI *ipm)
{
    uint32 delay;

    /* Compute a "random" delay before an IGMP report will be sent. Base it on 
       the system clock + the group addr + the IP addr of the interface on which 
       the group was joined. The delay should be between 0 and 10 seconds. 1 
       tick is added to keep the delay from being zero. */
    delay = NU_Retrieve_Clock();
    delay += ipm->ipm_addr + *(uint32 *)ipm->ipm_device->dev_addr.dev_ip_addr;
    delay = (delay % (TICKS_PER_SECOND * 10)) + 1;

    UTL_Timerset (EV_IGMP_REPORT, (uint32)ipm, delay, 0);
    
    return (delay);

} /* IGMP_Random_Delay */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IGMP_Leave                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function kills any timer events that are pending when       */
/*      membership in a multicast group is dropped.                      */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Delete_Multi                 Removes an IP address from a     */
/*                                      Multicast group.                 */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      UTL_Timerunset                  Remove all timer events from the */
/*                                      queue that match.                */
/* INPUTS                                                                */
/*                                                                       */
/*      ipm         A pointer to the multicast group structure.          */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      <Outputs>                           <Description>                */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/16/98       Created initial version           */
/*************************************************************************/
VOID IGMP_Leave(IP_MULTI *ipm)
{
    /* If there is a timer event pending to send a group membership report, 
       clear the timer event. */
    if (ipm->ipm_timer)
    {
        ipm->ipm_timer = 0;
        UTL_Timerunset (EV_IGMP_REPORT, (uint32)ipm, 1);
    }

}/* IGMP_Leave */

#endif /* INCLUDE_IP_MULTICASTING */
