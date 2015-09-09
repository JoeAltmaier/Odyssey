/****************************************************************************/
/*                                                                          */
/*      Copyright (c) 1996 by Accelerated Technology, Inc.                  */
/*                                                                          */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the subject */
/* matter of this material.  All manufacturing, reproduction, use and sales */
/* rights pertaining to this subject matter are governed by the license     */
/* agreement.  The recipient of this software implicity accepts the terms   */
/* of the license.                                                          */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* FILENAME                                                 VERSION         */
/*                                                                          */
/*    DEV.C                                                 4.0             */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file is responsible for intializing all of the devices that will */
/*    be used with a specific instatiation of Nucleus NET.  This code may   */
/*    be changed into a dynamic module in the future.                       */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Glen Johnson                                                          */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*    Check in file 'dev.h'                                                 */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*      DEV_Attach_IP_To_Device                                             */
/*      DEV_Detach_IP_From_Device                                           */
/*      DEV_Get_Dev_By_Addr                                                 */
/*      DEV_Get_Dev_By_Name                                                 */
/*      DEV_Get_Dev_For_Vector                                              */
/*      DEV_Get_Ether_Address                                               */
/*      DEV_Init_Devices                                                    */
/*      DEV_Init_Route                                                      */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*  other file dependencies                                                 */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME                 DATE    REMARKS                                    */
/*                                                                          */
/* Neil Henderson    02/11/97  Inital version. (Version 1.0)                */
/*                                                                          */
/****************************************************************************/

#include "externs.h"
#include "dev.h"
#include "tcpdefs.h"
#include "net.h"
#include "arp.h"
#include "data.h"
#include "igmp.h"
#if SNMP_INCLUDED
#include "snmp_g.h"
#endif
#include "dec21143.h"

#define	NET_SUPPORT 1

/*  List of devices. */
DV_DEVICE_LIST    DEV_Table;

extern NU_TASK      rip2_task_ptr;
/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  DEV_Init_Devices                                Version 4.0             */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function will initialize all of the devices that will be used by   */
/*  Nucleus NET.  It will be hard coded to set up the devices based on the  */
/*  developer's requirements.                                               */
/*                                                                          */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Neil Henderson,    Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    Application                                                           */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*      NU_Allocate_Memory                                                  */
/*      UTL_Zero                                                            */
/*      dll_enqueue                                                         */
/*      NU_Resume_Task                                                      */
/*      DEV_Attach_IP_To_Device                                             */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME                DATE        REMARKS                               */
/*                                                                          */
/*  Glen Johnson        02/17/96    Initial version.                        */
/*                                                                          */
/****************************************************************************/
STATUS DEV_Init_Devices (DEV_DEVICE *devices, INT dev_count)
{
    STATUS                  status;
    DV_DEVICE_ENTRY         *dev_ptr;
    INT                     i;
    static INT              next_index = 0;
#if (INCLUDE_RIP2)
    static INT              need_rip2 = 0;
#endif


    for ( i = 0; i < dev_count; i++)
    {
        /*  Allocate memory for this device entry.  */
        if ((status = NU_Allocate_Memory(&System_Memory, (VOID **)&dev_ptr,
                                (UNSIGNED)(sizeof(struct _DV_DEVICE_ENTRY)),
                                (UNSIGNED)NU_NO_SUSPEND)) != NU_SUCCESS)
        {
            return (status);
        }

        /*  Clear out the structure.  */
        UTL_Zero(dev_ptr, sizeof(struct _DV_DEVICE_ENTRY));

        /* Adde the new device entry to the list of devices. */
        dll_enqueue((tqe_t *)&DEV_Table, (tqe_t *)dev_ptr);

#if (INCLUDE_RIP2)
        /* Should RIP2 be used on this device. */
        if( devices[i].dv_use_rip2 )
        {
            if (!need_rip2)
                NU_Resume_Task(&rip2_task_ptr);

            need_rip2++;

            /* Set the routing metric for this device. */
            if (devices[i].dv_ifmetric)
                dev_ptr->dev_metric = devices[i].dv_ifmetric;
            else
                dev_ptr->dev_metric = 1;

            dev_ptr->dev_rip2_recvmode = devices[i].dv_recvmode;
            dev_ptr->dev_rip2_sendmode = devices[i].dv_sendmode;
        }
#endif /* INCLUDE_RIP2 */


        /* Now initialize the fields that we can.  The rest will be initialized
           by the driver. */

        /* Set the unit number.  This number will be unique for all devices.
           The first will be numbered at 0 each succeeding device will be
           numbered contiguously. */
        dev_ptr->dev_index = next_index++;

        /* Initialize the device's name. */
        dev_ptr->dev_net_if_name = devices[i].dv_name;

        /* Get the flags set by the application */
        dev_ptr->dev_flags = devices[i].dv_flags;

        /* Is this a serial link? */
        if (devices[i].dv_flags & DV_POINTTOPOINT)
        {
            /* Store the options that pertain only to a serial link. All of
               these may not be used. It depends on the UART. */
            dev_ptr->dev_com_port  = devices[i].dv_hw.uart.com_port;
            dev_ptr->dev_baud_rate = devices[i].dv_hw.uart.baud_rate;
            dev_ptr->dev_data_mode = devices[i].dv_hw.uart.data_mode;
            dev_ptr->dev_parity    = devices[i].dv_hw.uart.parity;
            dev_ptr->dev_stop_bits = devices[i].dv_hw.uart.stop_bits;
            dev_ptr->dev_data_bits = devices[i].dv_hw.uart.data_bits;
        }
        else
        {
            /* Store the options that pertain only to an ethernet link */
            dev_ptr->dev_irq     = devices[i].dv_hw.ether.dv_irq;
            dev_ptr->dev_sm_addr = devices[i].dv_hw.ether.dv_shared_addr;
            dev_ptr->dev_io_addr = devices[i].dv_hw.ether.dv_io_addr;
        }

        if( (status = (devices[i].dv_init)(dev_ptr)) != NU_SUCCESS)
            return status;

        /* Indicate that the device is Running. */
        dev_ptr->dev_flags |= DV_RUNNING;

        /* If an IP address was specified and this is not a PPP device then
           go ahead and attach the IP address and the subnet mask.  Otherwise
           it is assumed Bootp will be used to discover the IP address and
           attach it at a later time. */
        if (( *(uint32 *)devices[i].dv_ip_addr != 0 ) &&
                (!(devices[i].dv_flags & DV_POINTTOPOINT)))
        {

            if ((status = DEV_Attach_IP_To_Device(dev_ptr->dev_net_if_name,
                                         (uint8 *)devices[i].dv_ip_addr,
                                         (uint8 *)devices[i].dv_subnet_mask))
                                         != NU_SUCCESS)
            {
                return (status);
            }
        }
        else
            /* Clear the devices IP address. */
            memcpy(dev_ptr->dev_addr.dev_ip_addr, IP_Null, 4);
    }

    /*  Everything is OK.  */
    return( NU_SUCCESS );

}  /* DEV_Init_Devices. */


/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  DEV_Get_Dev_For_Vector                       Version 4.0                */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Given a vector number, this routine will find the device that resides   */
/*  there and return the device table address.                              */
/*                                                                          */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Neil Henderson,    Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    network device drivers                                                */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    vector - the vector for which the requestor is finding a device       */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    address of the device table entry for the device found or 0 if not    */
/*    found                                                                 */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME                DATE        REMARKS                               */
/*                                                                          */
/*  Glen Johnson        02/17/96    Initial version.                        */
/*                                                                          */
/****************************************************************************/

DV_DEVICE_ENTRY *DEV_Get_Dev_For_Vector( int vector )
{

    DV_DEVICE_ENTRY   *temp_dev_table;

    /*  Look at the first in the list. */
    temp_dev_table = DEV_Table.dv_head;

    /*  Search for a match.  */
    while ((temp_dev_table -> dev_vect != vector) &&
           (temp_dev_table != (DV_DEVICE_ENTRY *) 0))
        temp_dev_table = temp_dev_table->dev_next;


    return( temp_dev_table);
}


/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  DEV_Get_Ether_Address                            Version 4.0            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Given a device name, this routine will get the ethernet address of      */
/*  the device.                                                             */
/*                                                                          */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Neil Henderson,    Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    applications                                                          */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    device name - an ASCII string representing the device                 */
/*    ether_addr - the container for the ethernet address to be returned    */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    NU_Success - device was updated                                       */
/*    NU_True - device was NOT updated                                      */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME                DATE        REMARKS                               */
/*                                                                          */
/*  Glen Johnson        02/17/96    Initial version.                        */
/*                                                                          */
/****************************************************************************/

int DEV_Get_Ether_Address(char *name, uchar *ether_addr)
{

    DV_DEVICE_ENTRY   *temp_dev_table;
    int         status = NU_SUCCESS;
    int         i;

    /*  Look at the first in the list. */
    temp_dev_table = DEV_Table.dv_head;

    /*  Search for a match.  */
    while ((strcmp(temp_dev_table -> dev_net_if_name, name) != 0) &&
           (temp_dev_table != (DV_DEVICE_ENTRY *) 0))
        temp_dev_table = temp_dev_table -> dev_next;


    /*  If we found one, then set up the IP address.  */
    if (temp_dev_table != (DV_DEVICE_ENTRY *) 0)
    {

        /*  Copy the IP Address into the interface table. */
        for (i = 0; i < 6; i++)
            ether_addr[i] = temp_dev_table -> dev_mac_addr[i];

    }
    else
        status = -1;

    return(status);

}
/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  DEV_IP_Attach_To_Device                           Version 4.0           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Given a device name, this routine will set the IP number into the       */
/*  network interface table for the device.                                 */
/*                                                                          */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Neil Henderson,    Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEV_Init_Devices                                                      */
/*    NU_Dhcp                                                               */
/*    NU_Bootp                                                              */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*      ARP_Request                                                         */
/*      DEV_Init_Route                                                      */
/*      IGMP_Initialize                                                     */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    name        - an ASCII string representing the device                 */
/*    ip_addr     - the IP address to be associated with the device         */
/*    subnet      - the subnet mask to be associated with the device        */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    NU_Success - device was updated                                       */
/*    NU_True - device was NOT updated                                      */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME                DATE        REMARKS                               */
/*                                                                          */
/*  Glen Johnson        02/17/96    Initial version.                        */
/*                                                                          */
/****************************************************************************/
INT DEV_Attach_IP_To_Device(char *name, uint8 *ip_addr, uint8 *subnet)
{

    DV_DEVICE_ENTRY   *device;
    int         status = 0;
    int         i;

    /*  Look at the first in the list. */
    device = DEV_Table.dv_head;

    /*  Search for a match.  */
    while ((strcmp(device -> dev_net_if_name, name) != 0) &&
           (device != (DV_DEVICE_ENTRY *) 0))
        device = device -> dev_next;


    /*  If we found one, then set up the IP address.  */
    if (device != (DV_DEVICE_ENTRY *) 0)
    {

        /*  Copy the IP Address into the interface table. */
        for (i = 0; i < IP_ADDR_LEN; i++)
            device -> dev_addr.dev_ip_addr[i] = ip_addr[i];

        /*  Copy the subnet mask the device table. */
        device->dev_addr.dev_netmask = *(uint32 *)subnet;

        /* Fill in the network number. */
        device->dev_addr.dev_net = *(uint32 *)device->dev_addr.dev_ip_addr &
                                  device->dev_addr.dev_netmask;
        
        /* Set the network broadcast address. */
        device->dev_addr.dev_net_brdcast = 
                                      *(uint32 *)device->dev_addr.dev_ip_addr 
                                      | (~device->dev_addr.dev_netmask);
 
        /* Update the SNMP IP Address Translation Table. */
        SNMP_ipNetToMediaTableUpdate(SNMP_ADD, device->dev_index,
                                    device->dev_mac_addr, 
                                    device->dev_addr.dev_ip_addr, 4);

        /* Update the address translation group. */
        SNMP_atTableUpdate(SNMP_ADD, device->dev_index, 
                            device->dev_mac_addr, device->dev_addr.dev_ip_addr);

        SNMP_ipAdEntUpdate(SNMP_ADD, device->dev_index, 
                            device->dev_addr.dev_ip_addr, 
                            (uint8 *)&device->dev_addr.dev_netmask, 1, 0);

        status = DEV_Init_Route(device);

        /* Indicate that the device is Running. */
        device->dev_flags |= DV_UP;

#if INCLUDE_IP_MULTICASTING        

        /* If multicasting support is desired then join the all hosts group 
           (224.0.0.1) on all devices that support IP multicasting. */
        if (device->dev_flags & DV_MULTICAST)
            IGMP_Initialize(device);

#endif /* INCLUDE_IP_MULTICASTING */

        /* Send a gratuitous ARP. */
        if (!((device->dev_flags & DV_NOARP) || (device->dev_flags & DV_POINTTOPOINT)))
            ARP_Request(device, (uint32 *)device->dev_addr.dev_ip_addr,
                             (uint8 *)"\0\0\0\0\0\0", EARP, ARPREQ);
    }
    else
        status = -1;

    return(status);

} /* DEV_Attach_IP_To_Device */


/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  DEV_IP_Detach_From_Device                           Version 4.0         */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Given a device name, this routine will remove the IP number from the    */
/*  network interface table for the device.                                 */
/*                                                                          */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah Pollock,    Accelerated Technology Inc.                         */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEV_Init_Devices                                                      */
/*    NU_Dhcp                                                               */
/*    NU_Bootp                                                              */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*      RTAB_Delete_Node                                                    */
/*      RTAB_Find_Route                                                     */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    name        - an ASCII string representing the device                 */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    NU_Success - device was updated                                       */
/*    NU_True - device was NOT updated                                      */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME                DATE        REMARKS                               */
/*                                                                          */
/*  Glen Johnson        02/17/96    Initial version.                        */
/*                                                                          */
/****************************************************************************/
INT DEV_Detach_IP_From_Device(char *name)
{
    DV_DEVICE_ENTRY     *device;
    int                 status = 0;
    int                 i;
    uint8               ip_addr[] = {0, 0, 0, 0};
    SCK_SOCKADDR_IP     device_ip_num;
    ROUTE_NODE          *rt;

    /*  Look at the first in the list. */
    device = DEV_Table.dv_head;

    /*  Search for a match.  */
    while ((strcmp(device -> dev_net_if_name, name) != 0) &&
           (device != (DV_DEVICE_ENTRY *) 0))
        device = device -> dev_next;


    /*  If we found one, then set up the IP address.  */
    if (device != (DV_DEVICE_ENTRY *) 0)
    {

        /* Store the IP address in this socket structure. This will be used 
           below to remove the route associated with this IP address. */
        memcpy (&device_ip_num.sck_addr, device->dev_addr.dev_ip_addr, IP_ADDR_LEN);

        /* Find the node that contains this route and remove it. Note that the
           device_ip_num structure only has the IP address filled in. The 
           RTAB_Find_Route function only uses the IP address. */
        rt = RTAB_Find_Route(&device_ip_num);

        /* The refernece count was incremented by RTAB_Find_Route. We are not
           using the route so decrement the refernce count. */
        rt->rt_refcnt--;

        /* Delete the route. */
        RTAB_Delete_Node (rt);

        /*  Copy the IP Address into the interface table. This will be zero for 
            removing the device. */
        for (i = 0; i < IP_ADDR_LEN; i++)
            device -> dev_addr.dev_ip_addr[i] = ip_addr[i];


        /* Update the SNMP IP Address Translation Table. */
        SNMP_ipNetToMediaTableUpdate(SNMP_DELETE, device->dev_index,
                                    device->dev_mac_addr, 
                                    device->dev_addr.dev_ip_addr, 4);

        SNMP_atTableUpdate(SNMP_DELETE, device->dev_index, 
                            device->dev_mac_addr, device->dev_addr.dev_ip_addr);

        SNMP_ipAdEntUpdate(SNMP_DELETE, device->dev_index, 
                            device->dev_addr.dev_ip_addr, 
                            (uint8 *)&device->dev_addr.dev_netmask, 1, 0);

        /*  Copy the subnet mask the device table. */
        device->dev_addr.dev_netmask = *(uint32 *) ip_addr;

        /* Fill in the network number. */
        device->dev_addr.dev_net = *(uint32 *)device->dev_addr.dev_ip_addr &
                                  device->dev_addr.dev_netmask;
        
        /* Set the network broadcast address. */
        device->dev_addr.dev_net_brdcast = 
                                      *(uint32 *)device->dev_addr.dev_ip_addr 
                                      | (~device->dev_addr.dev_netmask);

        /* Indicate that the device is not up and running. */
        device->dev_flags &= ~DV_UP;

    }
    else
        status = -1;

    return(status);

} /* end DEV_Detach_IP_From_Device */


/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  Dev_Init_Route                                     Version 4.0          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Given a device structure, this routine will initialize the route for    */
/*  the device.                                                             */
/*                                                                          */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Neil Henderson,    Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEV_Attach_IP_to_Device            Attaches and IP address to         */
/*                                       device.                            */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    RTAB_Add_Route                     Adds a route to the RTAB           */
/*                                       structure.                          */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME                DATE        REMARKS                               */
/*                                                                          */
/*  Glen Johnson        02/17/96    Initial version.                        */
/*                                                                          */
/****************************************************************************/

STATUS  DEV_Init_Route(DV_DEVICE_ENTRY *device)
{
    uint32          dest;
    DEV_IF_ADDRESS  *dv_addr;

    dv_addr = &device->dev_addr;

    dest = *((uint32 *)dv_addr->dev_ip_addr) & dv_addr->dev_netmask;

    return ( RTAB_Add_Route(device, dest, dv_addr->dev_netmask, 
             *(uint32 *)dv_addr->dev_ip_addr, RT_UP) );

} /* DEV_Init_Route */


/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  Dev_Get_Device_By_Name                          Version 4.0             */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Given a device name, this routine will return a pointer to the device   */
/*  structure of the device that wasd named.                                */
/*                                                                          */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Neil Henderson,    Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    applications.																			 */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    name        - an ASCII string representing the device                 */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    dev         - pointer to the device structure of that name.           */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME                DATE        REMARKS                               */
/*                                                                          */
/*  Glen Johnson        02/17/96    Initial version.                        */
/*                                                                          */
/****************************************************************************/

DV_DEVICE_ENTRY *DEV_Get_Dev_By_Name( CHAR *name )
{

    DV_DEVICE_ENTRY   *dev;

    /*  Look at the first in the list. */
    dev = DEV_Table.dv_head;

    /*  Search for a match.  */
    while ( (dev != (DV_DEVICE_ENTRY *) 0) &&
            (strcmp(dev->dev_net_if_name, name) != 0) )
        dev = dev->dev_next;


    return(dev);

} /* DEV_Get_Dev_By_Name */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  DEV_Get_Dev_By_Addr                           Version 4.0               */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    Find the device by IP address                                         */
/*                                                                          */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Kelly Wiles,       Xact Inc.                                          */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    applications.                                                         */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    IP Address - the IP address to be assocated with the device           */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME                DATE        REMARKS                               */
/*                                                                          */
/****************************************************************************/

DV_DEVICE_ENTRY *DEV_Get_Dev_By_Addr( uint8 *addr )
{
    DV_DEVICE_ENTRY   *dev;

    /*  Look at the first in the list. */
    dev = DEV_Table.dv_head;

    /*  Search for a match.  */
    while ( (dev != (DV_DEVICE_ENTRY *) 0) &&
            (memcmp(dev->dev_addr.dev_ip_addr, addr, 3) != 0) )
        dev = dev->dev_next;


    return(dev);

} /* DEV_Get_Dev_By_Addr */

