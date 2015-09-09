/*************************************************************************/
/*                                                                       */
/*     Copyright (c) 1997        Accelerated Technology, Inc.            */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/
/******************************************************************************/
/*                                                                            */
/* FILENAME                                                 VERSION           */
/*                                                                            */
/*  SNMP_G                                                     2              */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*  This file contains the glue for XACT's SNMP Agent, MIB2, etc.             */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*      Glen Johnson,       Accelerated Technology                            */
/*                                                                            */
/* DATA STRUCTURES                                                            */
/*                                                                            */
/*      None of our own.  XACT's data structures are accessed.                */
/*                                                                            */
/* FUNCTIONS                                                                  */
/*                                                                            */
/*      SNMP_Initialize                 Initialize the SNMP component.        */
/*      SNMP_System_Group_Initialize    Initialize the System Group.          */
/*      SNMP_atTableUpdate              Add or delete an entry in the Address */
/*                                        Translation Table.                  */
/*      SNMP_ipAdEntUpdate              Add or delete an entry in the IP      */
/*                                        address table.                      */
/*      SNMP_ipRouteTableUpdate         Add or delete an entry in the route   */
/*                                        table.                              */
/*      SNMP_ipNetToMediaTableUpdate    Add or delete an ntry in the IP       */
/*                                        address translation table           */
/*      SNMP_udpListenTableUpdate       Add or delete an entry in the UDP     */
/*                                        listen table.                       */
/*      SNMP_tcpConnTableUpdate         Add or delete an entry in the TCP     */
/*                                        connection table.                   */
/*                                                                            */
/* DEPENDENCIES                                                               */
/*                                                                            */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*  NAME                DATE        REMARKS                                   */
/*                                                                            */
/*  Glen Johnson      04/21/97      Created initial version.                  */
/*  Glen Johnson      08/06/97      Updated for the release 1.1 of XACT's     */
/*                                    SNMP Agent and MIBs.                    */
/*                                                                            */
/******************************************************************************/
#include"nucleus.h"
#include"snmp_g.h"
#include"target.h"

void x_bzero( i8 *s1, u32 len );
i32 Add1213AtTab(rfc1213_at_t *entry);
void Del1213AtTab(rfc1213_at_t *entry);
i32 Add1213IpAddrTab(ipaddrtab_t *entry);
void Del1213IpAddrTab(ipaddrtab_t *entry);
i32 Add1213IpRouteTab(iproutetab_t *entry);
void Del1213IpRouteTab(iproutetab_t *entry);
i32 Add1213IpNet2MediaTab(ipnet2media_t *entry);
void Del1213IpNet2MediaTab(ipnet2media_t *entry);
i32 Add1213UdpListTab(udplisttab_t *entry);
void Del1213UdpListTab(udplisttab_t *entry);
i32 Add1213TcpTab(tcpcontab_t *entry);
void Del1213TcpTab(tcpcontab_t *entry);


/* This is the global data structure where all MIB2 data is stored.  The
   definition of this structure is in 1213XXXX.H. */
rfc1213_vars_t  rfc1213_vars;

SNMP_DRIVER_STATS   SNMP_Drv_Stats[MAX_1213_IF];



/******************************************************************************/
/* FUNCTION                                                                   */
/*                                                                            */
/*   SNMP_Initialize                                                          */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function initializes the SNMP glue component.                      */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*  Glen Johnson      04/21/97      Created initial version.                  */
/*  Glen Johnson      08/06/97      Updated for the release 1.1 of XACT's     */
/*                                    SNMP Agent and MIBs.                    */
/*                                                                            */
/******************************************************************************/
STATUS SNMP_Initialize(VOID)
{
    INT         i;
 
    /* Clear the SNMP data structure. */
    x_bzero((char *)&rfc1213_vars, sizeof (rfc1213_vars_t)  );
    x_bzero((char *)&SNMP_Drv_Stats, sizeof (SNMP_Drv_Stats)  );

    /* Initialize the Interface Group. */

    SNMP_ifNumber(MAX_1213_IF);
  
    for(i=0; i < MAX_1213_IF; i++)
    {
        /* For status use a value of 2 to indicate that the interface is down.
           I am not sure this is what we want to do.  Maybe a better idea is to
           use 0 until a driver registers with the interface.  0 is not a valid
           entry for an interface in use. */
        SNMP_ifAdminStatus(i, RFC1213_IF_DOWN);
        SNMP_ifOperStatus(i, RFC1213_IF_DOWN);
    }

    /* Initialize the IP group */
    /* For the time being Nucleus NET cannot act as a gateway.  The only valid
       entry for this field is 2.  If Nucleus NET could act as a gateway there
       would be 2 possible values :  forwarding(1) and not-forwarding(2).
    */
    SNMP_ipForwarding(RFC1213_IP_NO_FORWARD);

    /* Right now Nucleus NET is hard coded to use a value of 100 for the TTL.
       This will have to be modified.  Later I assume that we will want to
       change the TTL Dynamically via SNMP. */
    SNMP_ipDefaultTTL(100);

    /* Initialize the RTO algorithm to other. */
    SNMP_tcpRtoAlgorithm(1);
    SNMP_tcpRtoMin(MINRTO);
    SNMP_tcpRtoMax(MAXRTO);
    SNMP_tcpMaxCon(NPORTS);

    return (NU_SUCCESS);

} /* SNMP_Initialize */


/******************************************************************************/
/* FUNCTION                                                                   */
/*                                                                            */
/*   SNMP_System_Group_Initialize                                             */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function should be called by applications to initialize the MIB2   */
/*    system group.                                                           */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*  Glen Johnson      04/21/97      Created initial version.                  */
/*  Glen Johnson      08/06/97      Updated for the release 1.1 of XACT's     */
/*                                    SNMP Agent and MIBs.                    */
/*                                                                            */
/******************************************************************************/
STATUS SNMP_System_Group_Initialize(rfc1213_sys_t *sys_group)
{
    /* Perform error checking to make sure the parameters are valid. */
    if( strlen((const char *)sys_group->sysDescr) > MAX_1213_STRSIZE       ||
        strlen((const char *)sys_group->sysObjectID) > MAX_1213_STRSIZE    ||
        strlen((const char *)sys_group->sysContact) > MAX_1213_STRSIZE     ||
        strlen((const char *)sys_group->sysName) > MAX_1213_STRSIZE        ||
        strlen((const char *)sys_group->sysLocation) > MAX_1213_STRSIZE )
    {
        return SNMP_INVALID_PARAMETER;
    }

    if ( sys_group->sysDescr == NU_NULL       ||
         sys_group->sysObjectID == NU_NULL    ||
         sys_group->sysContact == NU_NULL     ||
         sys_group->sysName == NU_NULL        ||
         sys_group->sysLocation == NU_NULL )
    {
        return SNMP_INVALID_POINTER;
    }

    if ( sys_group->sysServices > 127 )
        return SNMP_INVALID_PARAMETER;

    /* Initialize the System Group. */
    SNMP_sysDescr(sys_group->sysDescr);
    SNMP_sysObjectID(sys_group->sysObjectID);
    SNMP_sysContact(sys_group->sysContact);
    SNMP_sysName(sys_group->sysName);
    SNMP_sysLocation(sys_group->sysLocation);
    SNMP_sysServices(sys_group->sysServices);

    return (SNMP_SUCCESS);

} /* SNMP_System_Group_Initialize */

/******************************************************************************/
/* FUNCTION                                                                   */
/*                                                                            */
/*   SNMP_atTableUpdate                                                       */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function can add or delete an entry to the MIB2 Address Translation*/
/*    Table.  The first parameter specifies whether an addition or deletion   */
/*    should be performed.                                                    */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*      command         SNMP_ADD or SNMP_DELETE :  Indicates whether an       */
/*                        addition or deletion to the table should be made.   */
/*      index           An index into the table.                              */
/*      phys_addr       The Physical Address (ethernet address).              */
/*      net_addr        The network address (IP address).                     */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*  Glen Johnson      08/06/97      Created initial version                   */
/*                                                                            */
/******************************************************************************/
STATUS SNMP_atTableUpdate(INT command, INT index, uint8 *phys_addr,
                            uint8 *net_addr)
{
    rfc1213_at_t    aentry;

    x_bzero((char *)&aentry, sizeof(rfc1213_at_t)  );

    aentry.atIfIndex = index;

    aentry.atPhysAddress[0] = phys_addr[0];
    aentry.atPhysAddress[1] = phys_addr[1];
    aentry.atPhysAddress[2] = phys_addr[2];
    aentry.atPhysAddress[3] = phys_addr[3];
    aentry.atPhysAddress[4] = phys_addr[4];
    aentry.atPhysAddress[5] = phys_addr[5];

    aentry.atNetAddress[0] = net_addr[0];
    aentry.atNetAddress[1] = net_addr[1];
    aentry.atNetAddress[2] = net_addr[2];
    aentry.atNetAddress[3] = net_addr[3];

    switch (command)
    {
        case SNMP_ADD :

            Add1213AtTab(&aentry);
            break;

        case SNMP_DELETE :

            Del1213AtTab(&aentry);
            break;

        default :
            ;
    }

    return(NU_SUCCESS);

} /* SNMP_atTableUpdate */

/******************************************************************************/
/* FUNCTION                                                                   */
/*                                                                            */
/*   SNMP_ipAdEntUpdate                                                       */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function can add or delete an entry to the MIB2 IP Address         */
/*    Table.  The first parameter specifies whether an addition or deletion   */
/*    should be performed.                                                    */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*      command         SNMP_ADD or SNMP_DELETE :  Indicates whether an       */
/*                        addition or deletion to the table should be made.   */
/*      index           An index into the table.                              */
/*      ip_addr         The IP address                                        */
/*      mask            The network mask.                                     */
/*      bcast           The network broad cast address.                       */
/*      reasm_size      The maximum size packet that can be re-assembled.     */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*  Glen Johnson      08/06/97      Created initial version                   */
/*                                                                            */
/******************************************************************************/
STATUS SNMP_ipAdEntUpdate(INT command, INT index, uint8 *ip_addr, uint8 *mask,
                            INT bcast, INT reasm_size)
{
    ipaddrtab_t     ientry;

    x_bzero((char *)&ientry, sizeof(ipaddrtab_t)  );

    ientry.ipAdEntAddr[0] = ip_addr[0];
    ientry.ipAdEntAddr[1] = ip_addr[1];
    ientry.ipAdEntAddr[2] = ip_addr[2];
    ientry.ipAdEntAddr[3] = ip_addr[3];

    ientry.ipAdEntIfIndex = index;

    ientry.ipAdEntBcastAddr = bcast;

    ientry.ipAdEntReasmMaxSize = reasm_size;

    ientry.ipAdEntNetMask[0] = mask[0];
    ientry.ipAdEntNetMask[1] = mask[1];
    ientry.ipAdEntNetMask[2] = mask[2];
    ientry.ipAdEntNetMask[3] = mask[3];

    switch (command)
    {
        case SNMP_ADD :

            Add1213IpAddrTab(&ientry);
            break;

        case SNMP_DELETE :

            Del1213IpAddrTab(&ientry);
            break;

        default :
            ;
    }

    return (NU_SUCCESS);

} /* SNMP_ipAdEntUpdate */

/******************************************************************************/
/* FUNCTION                                                                   */
/*                                                                            */
/*   SNMP_ipRouteTableUpdate                                                  */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function can add or delete an entry to the MIB2 IP routing         */
/*    Table.  The first parameter specifies whether an addition or deletion   */
/*    should be performed.                                                    */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*      command         SNMP_ADD or SNMP_DELETE :  Indicates whether an       */
/*                        addition or deletion to the table should be made.   */
/*      index           An index into the table.                              */
/*      dest            The destination for this route.                       */
/*      metric1         Metrics to be used for this route.                    */
/*      metric2                                                               */
/*      metric3                                                               */
/*      metric4                                                               */
/*      metric5                                                               */
/*      next_hop        The next hop address.                                 */
/*      type            Route type.                                           */
/*      proto           Route protocol.                                       */
/*      age             Route age.                                            */
/*      mask            Route mask.                                           */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*  Glen Johnson      08/06/97      Created initial version                   */
/*                                                                            */
/******************************************************************************/
STATUS SNMP_ipRouteTableUpdate(INT command, INT index, uint8 *dest,
                        UNSIGNED metric1, UNSIGNED metric2, UNSIGNED metric3,
                        UNSIGNED metric4, UNSIGNED metric5, uint8 *next_hop,
                        UNSIGNED type, UNSIGNED proto, UNSIGNED age,
                        uint8 *mask, CHAR *info)
{
    iproutetab_t        rentry;

    x_bzero((char *)&rentry, sizeof(iproutetab_t)  );

    rentry.ipRouteIfIndex = index;

    rentry.ipRouteDest[0] = dest[0];
    rentry.ipRouteDest[1] = dest[1];
    rentry.ipRouteDest[2] = dest[2];
    rentry.ipRouteDest[3] = dest[3];

    rentry.ipRouteMetric1 = metric1;
    rentry.ipRouteMetric2 = metric2;
    rentry.ipRouteMetric3 = metric3;
    rentry.ipRouteMetric4 = metric4;
    rentry.ipRouteMetric5 = metric5;

    rentry.ipRouteNextHop[0] = next_hop[0];
    rentry.ipRouteNextHop[1] = next_hop[1];
    rentry.ipRouteNextHop[2] = next_hop[2];
    rentry.ipRouteNextHop[3] = next_hop[3];

    rentry.ipRouteType = type;

    rentry.ipRouteProto = proto;

    rentry.ipRouteAge = age;

    rentry.ipRouteMask[0] = mask[0];
    rentry.ipRouteMask[1] = mask[1];
    rentry.ipRouteMask[2] = mask[2];
    rentry.ipRouteMask[3] = mask[3];

    strcpy((char *)rentry.ipRouteInfo, info);

    switch (command)
    {
        case SNMP_ADD :

            Add1213IpRouteTab(&rentry);
            break;

        case SNMP_DELETE :

            Del1213IpRouteTab(&rentry);
            break;

        default :
            ;
    }

    return (NU_SUCCESS);

} /* SNMP_ipRouteTableUpdate */

/******************************************************************************/
/* FUNCTION                                                                   */
/*                                                                            */
/*   SNMP_ipNetToMediaTableUpdate                                             */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function can add or delete an entry to the MIB2 IP Address         */
/*    Translation Table.  The first parameter specifies whether an addition or*/
/*    deletion should be performed.                                           */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*      command         SNMP_ADD or SNMP_DELETE :  Indicates whether an       */
/*                        addition or deletion to the table should be made.   */
/*      index           An index into the table.                              */
/*      phys_addr       The Physical Address (ethernet address).              */
/*      net_addr        The network address (IP address).                     */
/*      type            The type of mapping.                                  */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*  Glen Johnson      08/06/97      Created initial version                   */
/*                                                                            */
/******************************************************************************/
STATUS SNMP_ipNetToMediaTableUpdate(INT command, UNSIGNED index,
                                    uint8 *phys_addr, uint8 *net_addr,
                                    UNSIGNED type)
{
    ipnet2media_t       nentry;

    x_bzero((char *)&nentry, sizeof(ipnet2media_t)  );

    nentry.ipNetToMediaIfIndex = index;

    nentry.ipNetToMediaPhysAddress[0] = phys_addr[0];
    nentry.ipNetToMediaPhysAddress[1] = phys_addr[1];
    nentry.ipNetToMediaPhysAddress[2] = phys_addr[2];
    nentry.ipNetToMediaPhysAddress[3] = phys_addr[3];
    nentry.ipNetToMediaPhysAddress[4] = phys_addr[4];
    nentry.ipNetToMediaPhysAddress[5] = phys_addr[5];

    nentry.ipNetToMediaNetAddress[0] = net_addr[0];
    nentry.ipNetToMediaNetAddress[1] = net_addr[1];
    nentry.ipNetToMediaNetAddress[2] = net_addr[2];
    nentry.ipNetToMediaNetAddress[3] = net_addr[3];

    nentry.ipNetToMediaType = type;

    switch (command)
    {
        case SNMP_ADD :

            Add1213IpNet2MediaTab(&nentry);
            break;

        case SNMP_DELETE :

            Del1213IpNet2MediaTab(&nentry);
            break;

        default :
            ;
    }

    return (NU_SUCCESS);

} /* SNMP_ipNetToMediaTableUpdate */

/******************************************************************************/
/* FUNCTION                                                                   */
/*                                                                            */
/*   SNMP_udpListenTableUpdate                                                */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function can add or delete an entry to the MIB2 UDP Listen         */
/*    Table.  The first parameter specifies whether an addition or deletion   */
/*    should be performed.                                                    */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*      command         SNMP_ADD or SNMP_DELETE :  Indicates whether an       */
/*                        addition or deletion to the table should be made.   */
/*      addr            The IP address that is listened on.                   */
/*      port            The port that is listened on.                         */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*  Glen Johnson      08/06/97      Created initial version                   */
/*                                                                            */
/******************************************************************************/
STATUS SNMP_udpListenTableUpdate(INT command, uint8 *addr, UNSIGNED port)
{
    udplisttab_t        uentry;

    x_bzero((char *)&uentry, sizeof(udplisttab_t)  );

    uentry.udpLocalAddress[0] = addr[0];
    uentry.udpLocalAddress[1] = addr[1];
    uentry.udpLocalAddress[2] = addr[2];
    uentry.udpLocalAddress[3] = addr[3];

    uentry.udpLocalPort = port;

    switch (command)
    {
        case SNMP_ADD :

            Add1213UdpListTab(&uentry);
            break;

        case SNMP_DELETE :
            Del1213UdpListTab(&uentry);
            break;

        default :
            ;
    }

    return (NU_SUCCESS);

} /* SNMP_udpListenTableUpdate */

/******************************************************************************/
/* FUNCTION                                                                   */
/*                                                                            */
/*   SNMP_tcpConnTableUpdate                                                  */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function can add or delete an entry to the MIB2 TCP Connection     */
/*    Table.  The first parameter specifies whether an addition or deletion   */
/*    should be performed.                                                    */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*      command         SNMP_ADD or SNMP_DELETE :  Indicates whether an       */
/*                        addition or deletion to the table should be made.   */
/*      state           The state of the connection.                          */
/*      local_addr      The local address.                                    */
/*      local_port      The local port.                                       */
/*      rem_addr        The remote address.                                   */
/*      rem_port        The remote port.                                      */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*  Glen Johnson      08/06/97      Created initial version                   */
/*                                                                            */
/******************************************************************************/
STATUS  SNMP_tcpConnTableUpdate(INT command, UNSIGNED state, uint8 *local_addr,
                                UNSIGNED local_port, uint8 *rem_addr,
                                UNSIGNED rem_port)
{
    tcpcontab_t         tentry;

    x_bzero((char *)&tentry, sizeof(tcpcontab_t)  );

    tentry.tcpConnState = state;

    tentry.tcpConnLocalAddress[0] = local_addr[0];
    tentry.tcpConnLocalAddress[1] = local_addr[1];
    tentry.tcpConnLocalAddress[2] = local_addr[2];
    tentry.tcpConnLocalAddress[3] = local_addr[3];

    tentry.tcpConnLocalPort = local_port;

    tentry.tcpConnRemAddress[0] = rem_addr[0];
    tentry.tcpConnRemAddress[1] = rem_addr[1];
    tentry.tcpConnRemAddress[2] = rem_addr[2];
    tentry.tcpConnRemAddress[3] = rem_addr[3];

    tentry.tcpConnRemPort = rem_port;

    Add1213TcpTab(&tentry);

    switch (command)
    {
        case SNMP_ADD :

            Add1213TcpTab(&tentry);
            break;

        case SNMP_DELETE :

            Del1213TcpTab(&tentry);
            break;

        default :
            ;
    }

    return (NU_SUCCESS);

} /* SNMP_tcpConnTableUpdate */
