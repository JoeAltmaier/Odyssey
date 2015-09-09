//SnmpInterface.cpp

#ifndef _TRACEF
#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"
#endif

#include <stdio.h>
#include <String.h>
#include "OsTypes.h"

#include "target.h"
#include "SnmpInterface.h"

#ifndef WIN32
#if SNMP_INCLUDED
#include "xtypes.h"
#include "xsnmp.h"
#include "snmp_g.h"
#include "snmp.h"
#include "link.h"
#include "prott.h"
#include "mac.h"

extern "C" i32     xsnmp_initialized;
extern "C" u32     Compare_Mac_OperStatus(u32 iIndex, u32 operStatus);
extern "C" void    SendENTTrap(u16 gen, u16 spec, snmp_object_t *list, u16 listLen);
extern "C" void    AddHost(i32 idx, ul32 hst);
extern "C" void    RemoveAllHostsForComm(i32 idx);
extern "C" i32     GetCommIndex( i8 * );
extern "C" u32     NUM_ACTIVE_PORTS;
extern "C" mac_iface_t *GetMAC( i32 unit );
extern "C" bool    MacUpdate( u32 iIndex, i8 *name, u32 operStatus );
extern "C" i32     x_strcmp( i8 *, i8 * );
extern "C" void    x_bzero(i8 *s1, u32 len);
//#define init_mib2_objectid "1.3.6.1.4.1.2993.1.1.1.1"
#endif
#endif

static int endOfInitFlag = 0;

SnmpInterface::SnmpInterface() {
#ifndef WIN32
#if SNMP_INCLUDED
    //SNMP_sysUpTime(0);
    //memcpy((void *)(rfc1213_vars.rfc1213_sys.sysObjectID), (const void *)init_mib2_objectid, sizeof(init_mib2_objectid));
    //SNMP_sysServices(72);                               //IP host + Applications (8 + 64)
    //SNMP_ipForwarding(2);                               //not-forwarding
#endif
#endif
}

SnmpInterface::~SnmpInterface() {}

char* SnmpInterface::ConvertString(UnicodeString usIN) {
    StringClass stTemp;
    usIN.GetAsciiString(stTemp);
	return stTemp.CString();
}

//the description of this device
void SnmpInterface::SetDescription(UnicodeString usDescription) {
	char* sDescription = ConvertString(usDescription);
    if ( sDescription == NULL )
        return;

#ifndef WIN32
#if SNMP_INCLUDED
    if( xsnmp_initialized ) {
        SNMP_sysDescr(sDescription);
    }
#endif
#else
    printf("snmpDescription %s\n", sDescription);
#endif

	delete[] sDescription;
}

//the description of this device
void SnmpInterface::SetDescription(UnicodeString usDescription, UnicodeString usProdDate) {
	char* sDescription = ConvertString(usDescription);
    if ( sDescription == NULL )
        return;

    char* sProdDate = ConvertString(usProdDate);
    if ( sProdDate == NULL ) {
        delete[] sDescription;
        return;
    }

#ifndef WIN32
#if SNMP_INCLUDED
    char* descr;

    int len = strlen(sDescription);
    len += strlen(" on ");
    len += strlen(sProdDate);
    descr = new char[len + 1];
    sprintf(descr, "%s on %s", sDescription, sProdDate);

    //printf("SnmpInterface::SetDescription - descr = %s\n", descr);
    if( xsnmp_initialized ) {
        SNMP_sysDescr(descr);
    }

    delete[] descr;
#endif
#else
    printf("snmpDescription %s  %s\n", sDescription, sProdDate);
#endif

	delete[] sDescription;
    delete[] sProdDate;
}

//the administrative contact for this device
void SnmpInterface::SetContact(UnicodeString usContact) {
    char* sContact = ConvertString(usContact);
    if ( sContact == NULL )
        return;

#ifndef WIN32
#if SNMP_INCLUDED
    //printf("SnmpInterface::SetContact - sContact = %s\n", sContact);
    if( xsnmp_initialized ) {
        SNMP_sysContact(sContact);
    }
#endif
#else
    printf("snmpContact = %s\n", sContact);
#endif

	delete[] sContact;
}

//the user-settable name of this device
void SnmpInterface::SetName(UnicodeString usName) {
    char* sName = ConvertString(usName);
    if ( sName == NULL )
        return;

#ifndef WIN32
#if SNMP_INCLUDED
    //printf("SnmpInterface::SetName - sName = %s\n", sName);
    if( xsnmp_initialized ) {
        SNMP_sysName(sName);
    }
#endif
#else
    printf("snmpName = %s\n", sName);
#endif

	delete[] sName;
}

//the string describing the physical location of this device
void SnmpInterface::SetLocation(UnicodeString usLocation) {
    char* sLocation = ConvertString(usLocation);
    if ( sLocation == NULL )
        return;

#ifndef WIN32
#if SNMP_INCLUDED
    //printf("SnmpInterface::SetLocation - sLocation = %s\n", sLocation);
    if( xsnmp_initialized ) {
        SNMP_sysLocation(sLocation);
    }
#endif
#else
    printf("snmpLocation = %s\n", sLocation);
#endif

	delete[] sLocation;
}

//to set the relevant data for a given index...
//should add the interface if it doesn't exist... update it if it does
void SnmpInterface::SetInterface(int iIndex, UnicodeString usName, int iOperStatus) {
    char* sName = ConvertString(usName);
    if ( sName == NULL )
        return;

#ifndef WIN32
#if SNMP_INCLUDED
    mac_iface_t     *mac;

    if ( (! xsnmp_initialized) || (iIndex + 1 > MAX_PORTS) || (iIndex < 0) ) {
        delete[] sName;
        return;
    }

    mac = GetMAC( iIndex );
    int opstat = 2;
    if ( mac->statusOper )
            opstat = 1;


    // new name or status ??
    if ( (x_strcmp((i8 *)sName, (i8 *)&mac->descr[0]) != 0) || (Compare_Mac_OperStatus(iIndex, iOperStatus) == 0) ) {
        MacUpdate( (u32)iIndex, (i8 *)sName, (u32)iOperStatus);
        printf("SnmpInterface::SetInterface - iIndex = %d, sName = %s, iOperStatus = %d\n", iIndex, sName, iOperStatus);
        if ( iIndex + 1 > NUM_ACTIVE_PORTS ) {
            NUM_ACTIVE_PORTS = iIndex + 1;
        }
        if ( endOfInitFlag != 0 ) {
            if ( iOperStatus == SNMP_OPER_STATUS_UP ) {
                SendLinkUp( iIndex + 1, sName );
            }
            else {
                SendLinkDown( iIndex + 1, sName );
            }
        }
    }
#endif
#else
    printf("snmpSetInterface(%d, %s, %d)\n", iIndex, sName, iOperStatus);
#endif

    delete[] sName;
}

    /* Called by ddmSNMP when initialization processing complete. */
void SnmpInterface::SNMPInitComplete() {
#ifndef WIN32
    //printf("SnmpInterface::SNMPInitComplete\n");
    if ( xsnmp_initialized ) {
        SendColdStart();
    }
    endOfInitFlag = 1;
#else
    printf("snmpSetInterface initialization complete\n");
#endif
}

//sends a udp trap packet to all addresses in the trap list
void SnmpInterface::SendTrap(UnicodeString usName, UnicodeString usDescription, int iSeverity) {
    char* sName = ConvertString(usName);
    if ( sName == NULL )
        return;

    char* sDescription = ConvertString(usDescription);
    if ( sDescription == NULL ) {
        delete[] sName;
        return;
    }

#ifndef WIN32
#if SNMP_INCLUDED
    snmp_object_t   *trap;

    printf("SnmpInterface::SendTrap - sName = %s, sDescription = %s, iSeverity = %d\n", sName, sDescription, iSeverity);

    if ( ! xsnmp_initialized ) {
        delete[] sName;
        delete[] sDescription;
        return;
    }

    trap = new snmp_object_t[2];
    x_bzero((i8 *)trap, sizeof(snmp_object_t));

    trap->Request = SNMP_PDU_GET;
    //trap->Id = {1, 3, 6, 1, 4, 1, 2993, 3, 1, 1, 2};
    trap->Id[0] = 1;
    trap->Id[1] = 3;
    trap->Id[2] = 6;
    trap->Id[3] = 1;
    trap->Id[4] = 4;
    trap->Id[5] = 1;
    trap->Id[6] = 2993;
    trap->Id[7] = 3;
    trap->Id[8] = 1;
    trap->Id[9] = 1;
    trap->Id[10] = 2;
    trap->IdLen = 11;
    trap->Type = SNMP_OCTETSTR;
    sprintf((i8 *) trap->Syntax.BufChr, sName );
    trap->SyntaxLen = strlen(sName);

    trap++;
    x_bzero((i8 *)trap, sizeof(snmp_object_t));
    trap->Request = SNMP_PDU_GET;
    //trap->Id = {1, 3, 6, 1, 4, 1, 2993, 3, 1, 1, 3};
    trap->Id[0] = 1;
    trap->Id[1] = 3;
    trap->Id[2] = 6;
    trap->Id[3] = 1;
    trap->Id[4] = 4;
    trap->Id[5] = 1;
    trap->Id[6] = 2993;
    trap->Id[7] = 3;
    trap->Id[8] = 1;
    trap->Id[9] = 1;
    trap->Id[10] = 3;
    trap->IdLen = 11;
    trap->Type = SNMP_OCTETSTR;
    sprintf((i8 *) trap->Syntax.BufChr, sDescription );
    trap->SyntaxLen = strlen(sDescription);
    trap--;

    if ( (iSeverity == SNMP_TRAP_SPECIFIC_TYPE_WARNING) ||
         (iSeverity == SNMP_TRAP_SPECIFIC_TYPE_MINOR) ||
         (iSeverity == SNMP_TRAP_SPECIFIC_TYPE_CRITICAL) ) {
        SendENTTrap(SNMP_TRAP_GENERAL_TYPE_ENTERPRISE, iSeverity, trap, 2);
    }

    delete trap;
#endif
#else
    printf("SendENTTrap called for %s, msg = %s, severity = %i\n", sName, sDescription, iSeverity);
#endif

    delete[] sName;
    delete[] sDescription;
}

//the interface link with usName is up
void SnmpInterface::SendLinkUp(int iIndex, char *sName) {
#if SNMP_INCLUDED
    snmp_object_t   *trap;

    trap = new snmp_object_t[2];
    x_bzero((i8 *)trap, sizeof(snmp_object_t));

    trap->Request = SNMP_PDU_GET;
    //trap->Id = {1, 3, 6, 1, 4, 1, 2993, 3, 1, 1, 1};
    trap->Id[0] = 1;
    trap->Id[1] = 3;
    trap->Id[2] = 6;
    trap->Id[3] = 1;
    trap->Id[4] = 4;
    trap->Id[5] = 1;
    trap->Id[6] = 2993;
    trap->Id[7] = 3;
    trap->Id[8] = 1;
    trap->Id[9] = 1;
    trap->Id[10] = 1;
    trap->IdLen = 11;
    trap->Type = SNMP_INTEGER;
    trap->Syntax.LngInt = iIndex;

    trap++;
    x_bzero((i8 *)trap, sizeof(snmp_object_t));
    trap->Request = SNMP_PDU_GET;
    //trap->Id = {1, 3, 6, 1, 4, 1, 2993, 3, 1, 1, 2};
    trap->Id[0] = 1;
    trap->Id[1] = 3;
    trap->Id[2] = 6;
    trap->Id[3] = 1;
    trap->Id[4] = 4;
    trap->Id[5] = 1;
    trap->Id[6] = 2993;
    trap->Id[7] = 3;
    trap->Id[8] = 1;
    trap->Id[9] = 1;
    trap->Id[10] = 2;
    trap->IdLen = 11;
    trap->Type = SNMP_OCTETSTR;
    sprintf((i8 *) trap->Syntax.BufChr, sName );
    trap->SyntaxLen = strlen(sName);
    trap--;

    SendENTTrap(SNMP_TRAP_GENERAL_TYPE_ENTERPRISE, SNMP_TRAP_SPECIFIC_TYPE_LINKUP, trap, 2);

    delete trap;
    delete[] sName;
#endif
}

//the interface link with usName is down
void SnmpInterface::SendLinkDown(int iIndex, char *sName) {
#if SNMP_INCLUDED
    snmp_object_t   *trap;

    trap = new snmp_object_t[2];
    x_bzero((i8 *)trap, sizeof(snmp_object_t));

    trap->Request = SNMP_PDU_GET;
    //trap->Id = {1, 3, 6, 1, 4, 1, 2993, 3, 1, 1, 1};
    trap->Id[0] = 1;
    trap->Id[1] = 3;
    trap->Id[2] = 6;
    trap->Id[3] = 1;
    trap->Id[4] = 4;
    trap->Id[5] = 1;
    trap->Id[6] = 2993;
    trap->Id[7] = 3;
    trap->Id[8] = 1;
    trap->Id[9] = 1;
    trap->Id[10] = 1;
    trap->IdLen = 11;
    trap->Type = SNMP_INTEGER;
    trap->Syntax.LngInt = iIndex;

    trap++;
    x_bzero((i8 *)trap, sizeof(snmp_object_t));
    trap->Request = SNMP_PDU_GET;
    //trap->Id = {1, 3, 6, 1, 4, 1, 2993, 3, 1, 1, 2};
    trap->Id[0] = 1;
    trap->Id[1] = 3;
    trap->Id[2] = 6;
    trap->Id[3] = 1;
    trap->Id[4] = 4;
    trap->Id[5] = 1;
    trap->Id[6] = 2993;
    trap->Id[7] = 3;
    trap->Id[8] = 1;
    trap->Id[9] = 1;
    trap->Id[10] = 2;
    trap->IdLen = 11;
    trap->Type = SNMP_OCTETSTR;
    sprintf((i8 *) trap->Syntax.BufChr, sName );
    trap->SyntaxLen = strlen(sName);
    trap--;

    SendENTTrap(SNMP_TRAP_GENERAL_TYPE_ENTERPRISE, SNMP_TRAP_SPECIFIC_TYPE_LINKDOWN, trap, 2);

    delete trap;
    delete[] sName;
#endif
}

//tells mgmt station to poll this device
void SnmpInterface::SendColdStart() {
#if SNMP_INCLUDED
    snmp_object_t   *trap;

    trap = new snmp_object_t[1];
    x_bzero((i8 *)trap, sizeof(snmp_object_t));

    trap->Request = SNMP_PDU_GET;
    //trap->Id = {1, 3, 6, 1, 4, 1, 2993, 3, 1, 1, 2};
    trap->Id[0] = 1;
    trap->Id[1] = 3;
    trap->Id[2] = 6;
    trap->Id[3] = 1;
    trap->Id[4] = 4;
    trap->Id[5] = 1;
    trap->Id[6] = 2993;
    trap->Id[7] = 3;
    trap->Id[8] = 1;
    trap->Id[9] = 1;
    trap->Id[10] = 2;
    trap->IdLen = 11;
    trap->Type = SNMP_OCTETSTR;
    sprintf((i8 *) trap->Syntax.BufChr, "coldStart" );
    trap->SyntaxLen = strlen( "coldStart" );

    SendENTTrap( SNMP_TRAP_GENERAL_TYPE_ENTERPRISE, SNMP_TRAP_SPECIFIC_TYPE_COLDSTART, trap, 1);

    delete trap;
#endif
}

//adds a trap to the list if that address isn't found
void SnmpInterface::AddTrapAddress(int iAddress) {
#ifndef WIN32
#if SNMP_INCLUDED
    //printf("SnmpInterface::AddTrapAddress\n");
    if ( xsnmp_initialized ) {
        AddHost( GetCommIndex("public"), iAddress );
    }
#endif
#else
    printf("snmpAddTrapAddress called\n");
#endif
}

/* remove all addresses from the trap list. */
void SnmpInterface::ClearTrapAddresses() {
#ifndef WIN32
#if SNMP_INCLUDED
    //printf("SnmpInterface::ClearTrapAddresses\n");
    if ( xsnmp_initialized ) {
        RemoveAllHostsForComm( GetCommIndex("public") );
    }
#endif
#else
    printf("snmpClearTrapAddresses called\n");
#endif
}

