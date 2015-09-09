/****************************************************************************/
/*                                                                          */
/*      Copyright (c) 1993, 1997 By Accelerated Technology, Inc.            */
/*                                                                          */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the subject */
/* matter of this material.  All manufacturing, reproduction, use and sales */
/* rights pertaining to this subject matter are governed by the license     */
/* agreement.  The recipient of this software implicity accepts the terms   */
/* of the license.                                                          */
/*                                                                          */
/****************************************************************************/
/******************************************************************************/
/*                                                                            */
/* FILENAME                                                 VERSION           */
/*                                                                            */
/*  DNS                                                        4.0            */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*  This file contains the Domain Name System (DNS) component.  Given a       */
/*  name this component will discover the matching IP address.                */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*  Glen Johnson,      Accelerated Technology Inc.                            */
/*                                                                            */
/* DATA STRUCTURES                                                            */
/*                                                                            */
/*  DNS_Hosts               A list of hosts names and IP addresses.           */
/*                                                                            */
/* FUNCTIONS                                                                  */
/*                                                                            */
/*  DNS_Initialize              Initialize the DNS component.                 */
/*  DNS_Resolve                 Resolve a host's IP address.                  */
/*  DNS_Build_Query             Build a DNS query packet.                     */
/*  DNS_Query                   Send a DNS query packet.                      */
/*  DNS_Pack_Domain_Name        Convert a name to the format expected by a    */
/*                                format expected by a DNS server.            */
/*  DNS_Unpack_Domain_Name      Upack a domain name.                          */
/*  DNS_Extract_Data            Get info (IP address or name) from the DNS    */
/*                                response.                                   */
/*  DNS_Find_Host_By_Name       Search for a host.                            */
/*  DNS_Add_Host                Add a host to the "hosts file".               */
/*  DNS_Find_Host_By_Addr       Search for a host.                            */
/*  DNS_Addr_To_String          Convert an IP address to ascii string.        */
/*                                                                            */
/* DEPENDENCIES                                                               */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*  NAME                DATE        REMARKS                                   */
/*                                                                            */
/*  Glen Johnson        06/16/97    Created initial version.                  */
/*                                                                            */
/******************************************************************************/

#include "nucleus.h"
#include "externs.h"
#include "data.h"
#include "tcp.h"
#include "net_extr.h"
#include "dns.h"

#if DNS_INCLUDED

/* IP address for a DNS server. */
CHAR DNS_Server[4];

#endif

/* This is the list of hosts.  If a host can not be found in this list, DNS will
   be used to retrieve the information.  The new host will be added to the list.
*/
DNS_HOST_LIST   DNS_Hosts;


/******************************************************************************/
/* FUNCTION                                                 VERSION           */
/*                                                                            */
/*   DNS_Initialize                                           4.0             */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function initializes the DNS component.                            */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    protinit                  Initializes the protocol stack.               */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    NU_Allocate_Memory        Allocate memory.                              */
/*    dll_enqueue               Add an item to a list.                        */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    NU_SUCCESS                Indicates successful operation.               */
/*    NU_MEM_ALLOC              Indicates not enough dynamic memory was       */
/*                                available.                                  */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      06/16/97    Created initial version for NET 3.1       */
/*                                                                            */
/******************************************************************************/
STATUS DNS_Initialize(VOID)
{
    struct host         *hst;
    INT                 host_count;
    INT                 i;
    DNS_HOST            *dns_host;
#ifdef VIRTUAL_NET_INCLUDED
    DWORD               valueDWORD, dataSize = 4;
    DWORD               ipAddr;
    HKEY                hKey, subKey;
    UINT                stat;
    char                hostName[80];
    DWORD               hostNameSize = sizeof (hostName);
    FILETIME            Dummy;
#endif

#if DNS_INCLUDED

    /* Initialize the DNS Server's IP address. */
    DNS_Server[0] = (CHAR)DNS_IP_Addr_1;
    DNS_Server[1] = (CHAR)DNS_IP_Addr_2;
    DNS_Server[2] = (CHAR)DNS_IP_Addr_3;
    DNS_Server[3] = (CHAR)DNS_IP_Addr_4;

#endif /* DNS_INCLUDED */

    /* Count the number of hosts. */
    for ( hst = hostTable, host_count = 0;
          hst->name[0];
          hst++, host_count++ );

    /* Allocate a block of memeory to build the hosts list with. */
    if (NU_Allocate_Memory (&System_Memory, (VOID **)&dns_host,
                            sizeof (DNS_HOST) * host_count,
                            NU_NO_SUSPEND) != NU_SUCCESS)
    {
        return (NU_MEM_ALLOC);
    }

    /* Build the HOST list. */
    for (i = 0; i < host_count; i++, dns_host++)
    {
        /* Setup the name and IP address. */
        dns_host->dns_name = hostTable[i].name;

        memcpy(dns_host->dns_ipaddr, hostTable[i].address, 4);

        /* Set the TTL.  A TTL of 0 indicates that an entry is permanent. */
        dns_host->dns_ttl = 0;

        /* Setup the length of the name.  Account for the null terminator. */
        dns_host->dns_name_size = strlen(hostTable[i].name) + 1;

        /* Add this host to the list. */
        dll_enqueue((tqe_t *) &DNS_Hosts, (tqe_t *) dns_host);

    }

#ifdef VIRTUAL_NET_INCLUDED
    /* Now that the host file has been added to the DNS list, add the hosts. */

    /* Open a key to the VNETDRV entry in the system Registry.  Below each
       subkey of this key will be read and added to the "host file".
    */
    if ((stat = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        "SYSTEM\\CURRENTCONTROLSET\\SERVICES\\VNETDRV",
                        0, KEY_ALL_ACCESS, &hKey)) != ERROR_SUCCESS)
        return (-1);

    for (i=0;;i++)
    {
        hostNameSize = sizeof (hostName);

        if ((stat = RegEnumKeyEx(hKey, i, hostName, &hostNameSize, 0, NULL,
                                 NULL, &Dummy)) != ERROR_SUCCESS)
            break;

        /*Windows NT 4.0 has a key 'enum' that should not be */
		/*included in the host list                          */
		if ( stricmp (hostName, "enum") == 0)
			continue;

        if ((stat = RegOpenKeyEx(hKey, hostName,
                        0, KEY_ALL_ACCESS, &subKey)) != ERROR_SUCCESS)
            return (-1);

        dataSize = sizeof(ipAddr);
        if ((stat = RegQueryValueEx(subKey, "IPADDR", NULL, &valueDWORD,
                           (unsigned char *)&ipAddr, &dataSize)) != ERROR_SUCCESS)
            return (-1);

        /* Get memory for the Host entry */
        if (NU_Allocate_Memory(&System_Memory, &dns_host, sizeof(DNS_HOST),
                               (UNSIGNED)NU_NO_SUSPEND) != NU_SUCCESS)
            return -1;

        /* Get memory for the host name */
        if (NU_Allocate_Memory(&System_Memory, &dns_host->dns_name, sizeof(hostName),
                               (UNSIGNED)NU_NO_SUSPEND) != NU_SUCCESS)
            return -1;

        /* Store the host name */
        strcpy(dns_host->dns_name, hostName);

        /* Store the length of the host name */
        dns_host->dns_name_size = strlen (hostName);

        /* Store the IP address */
        *(DWORD *)dns_host->dns_ipaddr = longswap(ipAddr);
        
		/* Set the TTL.  A TTL of 0 indicates that an entry is permanent. */
        dns_host->dns_ttl = 0;

        /* Add this host entry to the host list */
        dll_enqueue((tqe_t *) &DNS_Hosts, (tqe_t *) dns_host);

    }
#endif

    return (NU_SUCCESS);

} /* DNS_Initialize */

/******************************************************************************/
/* FUNCTION                                                 VERSION           */
/*                                                                            */
/*   DNS_Find_Host_By_Name                                    4.0             */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function searches the "hosts" file for a host with a matching name.*/
/*    If the host is not found DNS is used to resolve the host name if        */
/*    if possible.                                                            */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    NU_Get_Host_By_Name       Retrieve host information based on a name.    */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    DNS_Add_Host              Add a host to the "hosts file"                */
/*    DNS_Resolve               Resolve a host name or address                */
/*    NU_Retrieve_Clock         Get the current clock tick.                   */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    name                      The name of the host to find.                 */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    NULL is returned if the host is not found, otherwise a host pointer.    */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      06/17/97    Created initial version for NET 3.1       */
/*                                                                            */
/******************************************************************************/
DNS_HOST *DNS_Find_Host_By_Name(CHAR *name)
{
    DNS_HOST        *host;
    CHAR            ip_addr[4];
    UNSIGNED        ttl;


    /* Search for a matching host. */
    for ( host = DNS_Hosts.dns_head; host ; host = host->dns_next)
    {
        /* Is this one we're looking for. */
        if (stricmp((const char *)host->dns_name, name) == 0)
        {
            /* We found a match.  If this is a permanent entry, return it. */
            if (!host->dns_ttl)
                return host;
            else
            {
                /* If the ttl has not expired return the host.  Else this entry
                   should be updated, so break. */
                if (INT32_CMP(host->dns_ttl, NU_Retrieve_Clock()) > 0)
                    return host;
                else
                    break;
            }
        }
    }

#if DNS_INCLUDED

    /* If the host was not found in the local cache or it had expired, then try
       to resolve it through DNS. */
    if (DNS_Resolve(name, ip_addr, &ttl, DNS_TYPE_A) == NU_SUCCESS)
        return (DNS_Add_Host(name, ip_addr, ttl));

#endif /* DNS_INCLUDED */

    return (NU_NULL);
} /* DNS_Find_Host_By_Name */

/******************************************************************************/
/* FUNCTION                                                 VERSION           */
/*                                                                            */
/*   DNS_Find_Host_By_Addr                                    4.0             */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function searches the "hosts" file for a host with a matching addr.*/
/*    If the host is not found DNS is used to resolve the host address        */
/*    if possible.                                                            */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    NU_Get_Host_By_Addr       Retrieve host information based on an address.*/
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    DNS_Add_Host              Add a host to the "hosts file"                */
/*    DNS_Resolve               Resolve a host name or address                */
/*    NU_Retrieve_Clock         Get the current clock tick.                   */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    addr                      The address of the host to find.              */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    NULL is returned if the host is not found, otherwise a host pointer.    */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      06/17/97    Created initial version for NET 3.1       */
/*                                                                            */
/******************************************************************************/
DNS_HOST *DNS_Find_Host_By_Addr(CHAR *addr)
{
    DNS_HOST        *host;
    CHAR            name[255];
    UNSIGNED        ttl;


    /* Search for a matching host. */
    for ( host = DNS_Hosts.dns_head; host ; host = host->dns_next)
    {
        /* Is this one we're looking for. */
        if (*(uint32 *)host->dns_ipaddr == *(uint32 *)addr)
        {
            /* We found a match.  If this is a permanent entry, return it. */
            if (!host->dns_ttl)
                return host;
            else
            {
                /* If the ttl has not expired return the host.  Else this entry
                   should be updated, so break. */
                if (INT32_CMP(host->dns_ttl, NU_Retrieve_Clock()) > 0)
                    return host;
                else
                    break;
            }
        }
    }

#if DNS_INCLUDED

    /* If the host was not found in the local cache or it had expired, then try
       to resolve it through DNS. */
    if (DNS_Resolve(name, addr, &ttl, DNS_TYPE_PTR) == NU_SUCCESS)
        return (DNS_Add_Host(name, addr, ttl));

#endif /* DNS_INCLUDED */

    return (NU_NULL);
} /* DNS_Find_Host_By_Addr */

#if DNS_INCLUDED
/******************************************************************************/
/* FUNCTION                                                 VERSION           */
/*                                                                            */
/*   DNS_Resolve                                              4.0             */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This routine will handle domain based name lookup.  It can hadle both   */
/*    queries for names and quiries for IP addresses.  Either the name and ttl*/
/*    are returned or an IP address and ttl are returned.                     */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    DNS_Find_Host_By_Name             Given a name find a host.             */
/*    DNS_Find_Host_By_Addr             Given an address find a host.         */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    NU_Deallocate_Memory      Deallocate a block of memory.                 */
/*    DNS_Build_Query           Create a DNS query packet.                    */
/*    DNS_Query                 Send the DNS query.                           */
/*    DNS_Extract_Data          Extract an IP address or name from the        */
/*                                response.                                   */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    name                      A host name.                                  */
/*    ip_addr                   The host IP address, filled in by this        */
/*                                function.                                   */
/*    ttl                       Time To Live, length of time the host info.   */
/*                                can be cached.  Filled in here.             */
/*    type                      Type of query, name or address.               */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    NU_SUCCESS                Indicates successful operation.               */
/*    a negative value            Indicates failure.                          */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      06/16/97    Created initial version for NET 3.1       */
/*                                                                            */
/******************************************************************************/
STATUS DNS_Resolve(CHAR *name, CHAR *ip_addr, UNSIGNED *ttl, INT type)
{
    STATUS                  stat;
    CHAR                    *buffer;
    int16                   q_size;


    /* Is there a DNS Server. */
    if ( !(*(uint32 *)DNS_Server) )
    {
        return (-1);
    }

    /* Build the DNS query. */
    switch (type)
    {
        case DNS_TYPE_A :

            if ((q_size = DNS_Build_Query(name, (VOID **)&buffer, type)) < 0)
                return -1;

            break;

        case DNS_TYPE_PTR :

            if ((q_size = DNS_Build_Query(ip_addr, (VOID **)&buffer, type)) < 0)
                return -1;

            break;

        default:

            return -1;
    }

    /* Query the DNS server.  Upon returning the buffer will contain the
       response to our query. */
    if (DNS_Query(buffer, q_size) < 0)
    {
        NU_Deallocate_Memory(buffer);

        return -1;
    }

    /* Now process the response. */
    switch (type)
    {
        case DNS_TYPE_A :

            stat = DNS_Extract_Data ((DNS_PKT_HEADER *)buffer, ip_addr, ttl,
                                      type);
            break;

        case DNS_TYPE_PTR :

            stat = DNS_Extract_Data ((DNS_PKT_HEADER *)buffer, name, ttl, type);
            break;

        default:

            return -1;
    }

    /* Deallocate the memory buffer. */
    NU_Deallocate_Memory(buffer);

    if (stat != NU_SUCCESS)
        return (NU_DNS_ERROR);
    else
        return (stat);

}  /* DNS_Resolve */

/******************************************************************************/
/* FUNCTION                                                 VERSION           */
/*                                                                            */
/*   DNS_Build_Query                                          4.0             */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function will build a DNS query packet.                            */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    DNS_Resolve            Resolve a host IP address.                       */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    NU_Allocate_Memory        Allocate memory.                              */
/*    DNS_Pack_Domain_Name      Convert a host name into format expected by   */
/*                                a DNS server.                               */
/*    DNS_Addr_To_String        Convert an IP address into a character string.*/
/*    intswap                   Swap the bytes in an integer.                 */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    name                      A host name.                                  */
/*    buffer                    A pointer to a buffer pointer.                */
/*    type                      A query type.                                 */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    > 0                       The size of the packed name.                  */
/*    < 0                       Failure                                       */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      06/16/97    Created initial version for NET 3.1       */
/*                                                                            */
/******************************************************************************/
INT  DNS_Build_Query(CHAR *data, VOID **buffer, INT type)
{
    DNS_PKT_HEADER      *dns_pkt;
    CHAR                *q_ptr;
    INT                 name_size;
    CHAR                name[30];

    /* Allocate a block of memeory to build the query packet in. */
    if (NU_Allocate_Memory (&System_Memory, (VOID **)&dns_pkt,
                            DNS_MAX_MESSAGE_SIZE, NU_NO_SUSPEND) != NU_SUCCESS)
    {
        return (NU_MEM_ALLOC);
    }

    /* Setup the packet. */
    dns_pkt->dns_id = intswap(1);
    dns_pkt->dns_flags = intswap(DNS_RD);   /* Set the Recursion desired bit. */
    dns_pkt->dns_qdcount = intswap(1);      /* There is only one query.       */
    dns_pkt->dns_ancount = 0;
    dns_pkt->dns_nscount = 0;
    dns_pkt->dns_arcount = 0;

    /* Point to the query section of the packet. */
    q_ptr = (CHAR *)(dns_pkt + 1);

    /* If we have a IP address and trying to get a name, convert the address to
       a character string. */
    if (type == DNS_TYPE_PTR)
        DNS_Addr_To_String(data, name);

    /* Pack the domain name, i.e., put it in a format the server will
       understand.  The packed domain name will be copied into the location
       pointer to by q_ptr. */
    switch (type)
    {
        case DNS_TYPE_A :
            if ((name_size = DNS_Pack_Domain_Name((CHAR *)q_ptr, data)) < 0 )
                return (name_size);
            break;

        case DNS_TYPE_PTR :
            if ((name_size = DNS_Pack_Domain_Name((CHAR *)q_ptr, name)) < 0)
                return (name_size);
            break;

        default :

            return -1;
    }

    /* Move the pointer past the end of the name. */
    q_ptr += name_size;

    /* Load the type and class fields of the query structure a byte at a time so
       that platforms that require word alignment won't choke. */
    *q_ptr++ = 0;
    *q_ptr++ = type;        /* The type is < 255, so high byte=0. */

    /* Fill in the class. */
    *q_ptr++ = 0;
    *q_ptr   = DNS_CLASS_IN;      /* The class is < 255, so high byte=0. */

    /* Return a pointer to the DNS packet. */
    *buffer = dns_pkt;

    return (sizeof(DNS_PKT_HEADER) + name_size + 4);

} /* DNS_Build_Query */

/******************************************************************************/
/* FUNCTION                                                 VERSION           */
/*                                                                            */
/*   DNS_Query                                                4.0             */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function queries a DNS server.  It sends a DNS query packet and    */
/*    waits for the response.                                                 */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    DNS_Resolve               Resolve a host IP address.                    */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    NU_Close_Socket           Shut down a socket.                           */
/*    NU_FD_Init                Zero out the a bit map.                       */
/*    NU_FD_Set                 Set a bit in a bit map.                       */
/*    NU_Recv_From              Receive a UDP packet.                         */
/*    NU_Select                 Check for data on a socket.                   */
/*    NU_Send_To                Send a UDP packet.                            */
/*    NU_Socket                 Create a communication socket.                */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    buffer                    A pointer to a buffer containing a query pkt. */
/*    q_size                    The size of the query.                        */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    > 0                       The size of the received response.            */
/*    < 0                       Failure                                       */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      06/17/97    Created initial version for NET 3.1       */
/*                                                                            */
/******************************************************************************/
INT  DNS_Query(CHAR *buffer, int16 q_size)
{
    struct addr_struct      dns_addr;
    FD_SET                  readfs, writefs, exceptfs;
    STATUS                  stat;
    INT                     attempts = DNS_MAX_ATTEMPTS;
    int16                   r_size = 0;
    int16                   bytes_sent;
    int16                   socketd;

    if ((socketd = NU_Socket(NU_FAMILY_IP, NU_TYPE_DGRAM, 0)) < 0)
        return (socketd);

    /* Fill in a structure with the server address */
    dns_addr.family    = NU_FAMILY_IP;
    dns_addr.port      = DNS_PORT;
    dns_addr.id.is_ip_addrs[0]  = DNS_Server[0];
    dns_addr.id.is_ip_addrs[1]  = DNS_Server[1];
    dns_addr.id.is_ip_addrs[2]  = DNS_Server[2];
    dns_addr.id.is_ip_addrs[3]  = DNS_Server[3];
    dns_addr.name = "";

    /* Initially all the bits should be cleared. */
    NU_FD_Init(&readfs);

    while (attempts)
    {
        /* Send the DNS query. */
        bytes_sent = NU_Send_To(socketd, buffer, q_size, 0, &dns_addr, 0);

        /*  If the data was not sent, we have a problem. */
        if (bytes_sent < 0)
            break;

        /* Decrement the number of attempts left. */
        attempts--;

        /* Specify which socket we want to select on. */
        NU_FD_Set(socketd, &readfs);

        /* Select on the specified socket for one second. */
        stat = NU_Select(NPORTS, &readfs, &writefs, &exceptfs, TICKS_PER_SECOND);

        /* If there is no data on the socket, either send the query again or
           give up. */
        if (stat != NU_SUCCESS)
            continue;

        /*  Go get the server's response.  */
        r_size = NU_Recv_From(socketd, buffer, 1000, 0, &dns_addr, 0);

        break;

    }

    /* Close the socket. */
    NU_Close_Socket(socketd);

    if (r_size > 0)
        return r_size;
    else
        return (NU_FAILED_QUERY);

} /* DNS_Query */

/******************************************************************************/
/* FUNCTION                                                 VERSION           */
/*                                                                            */
/*   DNS_Pack_Domain_Name                                     4.0             */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function takes a domain name and converts it to a format a DNS     */
/*    server expects.                                                         */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    DNS_Build_Query           Create a DNS query packet.                    */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    dst                       The converted name.                           */
/*    src                       The original name.                            */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    > 0                       The size of the new name.                     */
/*    < 0                       Failure                                       */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      06/17/97    Created initial version for NET 3.1       */
/*                                                                            */
/******************************************************************************/
INT DNS_Pack_Domain_Name (CHAR *dst, CHAR *src)
{
    CHAR        *p;
    CHAR        *original_dest;
    INT         n;

    /* If this is a null string return an error. */
    if (!*src)
        return (NU_INVALID_LABEL);

    p = original_dest = dst;

    do
    {
        /* Move past the byte where the length will be saved. */
        dst++;

        /* Assume all labels have been copied until proven otherwise. */
        *p = 0;

        /* Copy the label. */
        for ( n = 0;
              *src && (*src != '.') && (n <= DNS_MAX_LABEL_SIZE);
              *dst++ = *src++, ++n );

        /* Check to see if the label exceded the maximum length. */
        if ( n > DNS_MAX_LABEL_SIZE)
            return (NU_INVALID_LABEL);

        /* Store the length of the label. */
        *p = dst - p - 1;

        /* Point to where the next length value will be stored. */
        p = dst;

        if (*src)
            src++;

    } while (*src);

    /* The end of the name is marked with a 0 length label. */
    *p = 0;

    dst++;

    return (dst - original_dest);

} /* DNS_Pack_Domain_Name */

/******************************************************************************/
/* FUNCTION                                                 VERSION           */
/*                                                            4.0             */
/*   DNS_Unpack_Domain_Name                                                   */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function packed name and converts it to a character string.        */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    DNS_Extract_Data          Get data from a DNS response.                 */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    dst                       The new name.                                 */
/*    src                       The original name.                            */
/*    buf_begin                 Pointer to start od response packet.          */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    The size of the new name.                                               */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      06/17/97    Created initial version for NET 3.1       */
/*                                                                            */
/******************************************************************************/
INT DNS_Unpack_Domain_Name(CHAR *dst, CHAR *src, CHAR *buf_begin)
{
    int16           size;
    INT             i, retval = 0;
    CHAR            *savesrc;

	savesrc = src;

    /* The end of the name is marked by a 0 length label. */
    while (*src)
	{
        /* Get the size of the label. */
        size = *src;

        /* Check to see if this is a pointer instead of a label size. */
        while ((size & 0xC0) == 0xC0)
		{
            /* If we have not encountered a pointer yet compute the size of the
               name so far. */
            if (!retval)
			{
				retval = src - savesrc + 2;
			}

            src++;

            /* Point to the new location. */
            src = &buf_begin[(size & 0x3f) * 256 + *src];
            size = *src;
		}

        /* Move the pointer past the label size. */
        src++;

        /* Copy the label. */
        for (i = 0; i < (size & 0x3f); i++)
		{
			*dst++ = *src++;
		}

        /* Insert the period between labels. */
        *dst++ = '.';
	}

    /* Add the terminator. */
    *(--dst) = 0;

    /* Account for the terminator on src. */
    src++;

    /* If the name included a pointer then the return value has already been
       computed. */
    if (!retval)
	{
		retval = src - savesrc;
	}

	return (retval);
}  /* DNS_Unpack_Domain_Name */

/******************************************************************************/
/* FUNCTION                                                 VERSION           */
/*                                                                            */
/*   DNS_Extract_Data                                         4.0             */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function takes a DNS response and extracts either an IP address or */
/*    a domain name, which ever the case may be.                              */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    DNS_Resolve               Resolve and IP address or host name.          */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    NU_Allocate_Memory        Allocate memory.                              */
/*    NU_Deallocate_Memory      Deallocate a block of memory.                 */
/*    DNS_Unpack_Domain_Name    Unpack a domain name.                         */
/*    intswap                   Swap the bytes in an integer.                 */
/*    longswap                  Swap the bytes in a long.                     */
/*                                                                            */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    pkt                       A pointer to the DNS response.                */
/*    data                      Put the address or name here.                 */
/*    ttl                       Put the Time To Live here.                    */
/*    type                      The type of query.                            */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    NU_SUCCESS (0)            Indicates success.                            */
/*     < 0                      Indicates failure.                            */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      06/17/97    Created initial version for NET 3.1       */
/*                                                                            */
/******************************************************************************/
STATUS DNS_Extract_Data (DNS_PKT_HEADER *pkt, CHAR *data, UNSIGNED *ttl,
                         INT type)
{
    DNS_RR          *rr_ptr;
    INT             name_size, n_answers, rcode;
    uint16          length;
    CHAR            *p_ptr, *name;
    CHAR            answer_received = 0;

    /* Get the number of answers in this message. */
    n_answers = intswap (pkt->dns_ancount);

    /* Extract the return code. */
    rcode = DNS_RCODE_MASK & intswap (pkt->dns_flags);

    /* Was an error returned? */
    if (rcode)
        return (NU_DNS_ERROR);

    /* If there is at least one answer and this is a response, process it. */
    if ((n_answers > 0) && (intswap (pkt->dns_flags) & DNS_QR))
    {
        /* Point to where the question starts. */
        p_ptr = (CHAR *)(pkt + 1);

        /* Allocate a block of memory to put the name in. */
        if (NU_Allocate_Memory (&System_Memory, (VOID **)&name,
                                DNS_MAX_NAME_SIZE,
                                NU_NO_SUSPEND) != NU_SUCCESS)
        {
            return (NU_MEM_ALLOC);
        }

        /* Extract the name. */
        name_size = DNS_Unpack_Domain_Name (name, p_ptr, (CHAR *)pkt);

        /*  Move the pointer past the name QTYPE and QCLASS to point at the
            answer section of the response. */
        p_ptr += name_size + 4;

        /*
        *  At this point, there may be several answers.  We will take the first
		*  one which has an IP number.	There may be other types of answers that
		*  we want to support later.
		*/
        while ((n_answers--) > 0)
        {
            /* Extract the name from the answer. */
            name_size = DNS_Unpack_Domain_Name (name, p_ptr, (CHAR *)pkt);

            /* Move the pointer past the name. */
            p_ptr += name_size;

            /* Point to the resource record. */
            rr_ptr = (DNS_RR *)p_ptr;

            /* Since the is no guarantee that the resource record will be on a
               word boundary check the fields a byte at a time. */
            if ((!*p_ptr) && (*(p_ptr + 1) == type)
                && (!*(p_ptr + 2)) && (*(p_ptr + 3) == DNS_CLASS_IN))
			{
                switch (type)
                {
                    case DNS_TYPE_A :

                        /* The answer has the correct type and class. Copy
                           the IP addr. */
                        memcpy (data, rr_ptr->dns_rdata, 4);
                        break;

                    case DNS_TYPE_PTR :

                        /* The answer has the correct type and class. Get the name. */
                        name_size = DNS_Unpack_Domain_Name (data,
                                                            rr_ptr->dns_rdata,
                                                            (CHAR *)pkt);
                        break;

                    default :
                        return -1;
                }

                /* Get the time to live for this RR. */
                *ttl = longswap (rr_ptr->dns_ttl);

                /* Indicate an answer was found. */
                answer_received = 1;

                break;
			}

            /* Copy the length of this answer. */
            length = rr_ptr->dns_rdlength;

            /* Point to the next answer, if any.  The rdlength field is the
               length of the data section of the RR.  Add 10 for the sizes of
               the type, class, ttl, and rdlength.
            */
            p_ptr += (10 + intswap (length));
		}
	}

    NU_Deallocate_Memory(name);

    if (answer_received)
        return (NU_SUCCESS);
    else
        return (-1);

}  /* DNS_Extract_Data */

/******************************************************************************/
/* FUNCTION                                                 VERSION           */
/*                                                                            */
/*   DNS_Add_Host                                             4.0             */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function adds a new host to the "hosts file".                      */
/*    A block of memory is alocated each time we want to add a new host.      */
/*    This can be wasteful of memory because Nucleus PLUS has a minimum       */
/*    allocation size of 50 bytes.  The alternative would have been to        */
/*    allocate enough memory for several of these at one time.  The problem   */
/*    with that is the RFC defines the maximum size of a name at 255 bytes.   */
/*    We would have to assume that a maximum sized name would have to be      */
/*    stored and allocate memory accordingly.  Since most names are nowhere   */
/*    close to 255 bytes this method would have been even more wasteful.      */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    DNS_Find_Host_By_Addr     Find a host by address.                       */
/*    DNS_Find_Host_By_Name     Find a host by name.                          */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    dll_enqueue               Add an item to a list.                        */
/*    NU_Allocate_Memory        Allocate memory.                              */
/*    NU_Retrieve_Clock         Get the current clock tick.                   */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    name                      The name of the host to add.                  */
/*    ip_addr                   The IP address of the host to add.            */
/*    ttl                       The Time To Live for this entry.              */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    NULL is returned if the host can not be added, otherwise a host pointer.*/
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      06/17/97    Created initial version for NET 3.1       */
/*                                                                            */
/******************************************************************************/
DNS_HOST *DNS_Add_Host(CHAR *name, CHAR *ip_addr, UNSIGNED ttl)
{
    INT             size;
    DNS_HOST        *dns_host;
    UNSIGNED        time;

    /* Get the size of the name. */
    size = strlen(name) + 1;

    /* Retrieve the current time. */
    time = NU_Retrieve_Clock();

    /* Before allocating memory for a new entry, check to see if the TTL of an
       old entry has expired.  If so re-use that entry. */
    for ( dns_host = DNS_Hosts.dns_head;
          dns_host;
          dns_host = dns_host->dns_next )
    {
        /* First check to see if this entry has a ttl.  A ttl of 0 is used to
           indicate permanent entries.
        */
        if (dns_host->dns_ttl)

            /* Check to see if the ttl has expired. */
            if (INT32_CMP(dns_host->dns_ttl, time) <= 0)

                /* Good so far.  Check to see if the name will fit in this
                   entry. */
                if (dns_host->dns_name_size >= size)
                    break;
    }

    /* Did we find an entry that can be re-used. */
    if (!dns_host)
    {
        /* A new host structure must be created. */
        /* We need to compute the size of the block of memory to allocate.  The
           size will include the size of a host structure + length of the name +
           1 (for the null terminator).  We may want to reuse this entry later
           for other hosts.  So allocate a minimum number of bytes for the name.
           This will make it more likely that this entry can be reused.
        */

        /* If the size is less than the minimum, then bump it up. */
        if (size < DNS_MIN_NAME_ALLOC)
            size = DNS_MIN_NAME_ALLOC;

        /* Allocate a block of memory for the new DNS host structure and for the
           name. */
        if (NU_Allocate_Memory (&System_Memory, (VOID **)&dns_host,
                                size + sizeof(DNS_HOST),
                                NU_NO_SUSPEND) != NU_SUCCESS)
        {
            return (NU_NULL);
        }

        /* The name will be stored immediately after the structure. */
        dns_host->dns_name = (CHAR *)(dns_host + 1);

        /* Add this host to the list. */
        dll_enqueue((tqe_t *) &DNS_Hosts, (tqe_t *) dns_host);

    }

    /* Copy the name and IP address. */
    strcpy(dns_host->dns_name, name);
    memcpy(dns_host->dns_ipaddr, ip_addr, 4);

    /* TTL is specified in seconds.  So multiply by the number ticks per
       second. */
    ttl *= TICKS_PER_SECOND;

    /* Initialize ttl. */
    dns_host->dns_ttl = ttl + NU_Retrieve_Clock();

    /* TTL value of 0 is used to indicate that the entry is permanent.  So if by
       some chance the value computed is 0, add 1. */
    if (!dns_host->dns_ttl)
        dns_host->dns_ttl++;

    /* Initialize the size of the name of this host. */
    dns_host->dns_name_size = size;

    return (dns_host);

} /* DNS_Add_Host */

/******************************************************************************/
/* FUNCTION                                                 VERSION           */
/*                                                                            */
/*   DNS_Addr_To_String                                       4.0             */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function takes an IP address and converts it to a character string.*/
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    DNS_Build_Query           Create a DNS query packet.                    */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    itoa                      Convert an intger to a ascii string.          */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    addr                      The address of to convert.                    */
/*    new_name                  The address in ascii format.                  */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    NU_SUCCESS                Indicates success.                            */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      06/17/97    Created initial version for NET 3.1       */
/*                                                                            */
/******************************************************************************/
STATUS  DNS_Addr_To_String(CHAR *addr, CHAR *new_name)
{
    INT         i;
    CHAR        *ptr = new_name;
    uchar       *a = (uchar *)addr;

    /* We need to add each octet of the address to the name in reverse order. */
    for (i = 3; i >= 0; i--)
    {
        /* Convert the octet to a character string. */
        itoa(a[i], ptr, 10);

        /* Move past the string just added. */
        ptr += strlen(ptr);

        /* Add the dot. */
        *ptr++ = '.';
    }

    strcpy(ptr, "IN-ADDR.ARPA");

    return NU_SUCCESS;

} /* DNS_Addr_To_String */

#endif /* DNS_INCLUDED */
