//TestSnmpDdmX.cpp

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"
#include <stdio.h>
#include <String.h>
#include "OsTypes.h"
#include "TestSnmpDdmX.h"
#include "BuildSys.h"
#include "UnicodeString.h"
#include "..\msl\osheap.h"
#include "RqOsTimer.h"
#include "Message.h"

#define DDM_SNMP_SESSION 999
#define TENSEC    10000000
#define TWENTYSEC 20000000
#define ONEMIN    60000000
#define FIVEMIN  300000000

CLASSNAME(TestSnmpDdmX,SINGLE);  // Class Link Name used by Buildsys.cpp

TestSnmpDdmX::TestSnmpDdmX(DID did): Ddm(did) {
    printf("TestSnmpDdmX::constructor\n");
}

Ddm *TestSnmpDdmX::Ctor(DID did) {
    printf("TestSnmpDdmX::Ctor\n");
    return new TestSnmpDdmX(did);
}

STATUS TestSnmpDdmX::Initialize(Message *pMsg) {
    printf("TestSnmpDdmX::Initialize\n");
	Reply(pMsg,OK);
	return OK;
}

STATUS TestSnmpDdmX::Enable(Message *pReqMsg) {
    RqOsTimerStart*    pStartTimerMsg;
    STATUS status;

    printf("TestSnmpDdmX::Enable\n");

    pStartTimerMsg = new RqOsTimerStart(ONEMIN,0);
    if (pStartTimerMsg)
        status = Send(pStartTimerMsg, (ReplyCallback)&InitSnmp);

    Reply(pReqMsg,OK);
    return OK;
}

STATUS TestSnmpDdmX::InitSnmp(Message* pMsg)
{
    RqOsTimerStart* pStartTimerMsg;
    STATUS status;

    //printf("TestSnmpDdmX::InitSnmp\n");
    delete pMsg;

    UnicodeString*  ifName = MakeUnicodeString( "Filler_in_Slot_a4" );
    m_SnmpInterface.SetInterface( 5, *ifName, 1 );

    pStartTimerMsg = new RqOsTimerStart(ONEMIN, ONEMIN);
    if (pStartTimerMsg)
        status = Send(pStartTimerMsg, (ReplyCallback)&SendEvent);

    return OK;
}

STATUS TestSnmpDdmX::SendEvent(Message* pMsg)
{
    static U32 total = 0;

    //printf("TestSnmpDdmX::SendEvent\n");
    delete pMsg;

    switch (++total) {
    case 1:
        UnicodeString* ifName = MakeUnicodeString( "NAC_in_Slot_b1" );
        m_SnmpInterface.SetInterface( 6, *ifName, 1 );
    break;
    case 3:
        ifName = MakeUnicodeString( "NAC_in_Slot_b2" );
        m_SnmpInterface.SetInterface( 7, *ifName, 2 );
    break;
    case 6:
        ifName = MakeUnicodeString( "SSD_in_Slot_b3" );
        m_SnmpInterface.SetInterface( 8, *ifName, 1 );
    break;
    case 10:
        ifName = MakeUnicodeString( "Filler_in_Slot_b4" );
        m_SnmpInterface.SetInterface( 9, *ifName, 1 );
    break;
    case 15:
        ifName = MakeUnicodeString( "NAC_in_Slot_c1" );
        m_SnmpInterface.SetInterface( 10, *ifName, 1 );
    break;
    case 22:
        ifName = MakeUnicodeString( "NAC_in_Slot_c2" );
        m_SnmpInterface.SetInterface( 11, *ifName, 1 );
    break;
    case 27:
        ifName = MakeUnicodeString( "SSD_in_Slot_c3" );
        m_SnmpInterface.SetInterface( 12, *ifName, 1 );
    break;
    case 28:
        ifName = MakeUnicodeString( "Filler_in_Slot_c4" );
        m_SnmpInterface.SetInterface( 13, *ifName, 1 );
    break;
    case 33:
        ifName = MakeUnicodeString( "NAC_in_Slot_d1" );
        m_SnmpInterface.SetInterface( 14, *ifName, 1 );
    break;
    case 40:
        ifName = MakeUnicodeString( "NAC_in_Slot_d2" );
        m_SnmpInterface.SetInterface( 15, *ifName, 1 );
    break;
    case 51:
        ifName = MakeUnicodeString( "SSD_in_Slot_d3" );
        m_SnmpInterface.SetInterface( 16, *ifName, 2 );
    break;
    case 55:
        ifName = MakeUnicodeString( "Filler_in_Slot_d4" );
        m_SnmpInterface.SetInterface( 17, *ifName, 1 );
    break;
    case 59:
        ifName = MakeUnicodeString( "HDD1_in_bay_0" );
        m_SnmpInterface.SetInterface( 18, *ifName, 1 );
    break;
    case 64:
        ifName = MakeUnicodeString( "HDD1_in_bay_1" );
        m_SnmpInterface.SetInterface( 19, *ifName, 1 );
    break;
    case 68:
        ifName = MakeUnicodeString( "HDD1_in_bay_2" );
        m_SnmpInterface.SetInterface( 20, *ifName, 1 );
    break;
    case 76:
        ifName = MakeUnicodeString( "HDD1_in_bay_3" );
        m_SnmpInterface.SetInterface( 21, *ifName, 1 );
    break;
    case 87:
        ifName = MakeUnicodeString( "SSD_in_Slot_c3" );
        UnicodeString* usMessage = MakeUnicodeString( "Exceeded 80 percent of capacity" );
        m_SnmpInterface.SendTrap(*ifName, *usMessage, SNMP_TRAP_SPECIFIC_TYPE_WARNING);
    break;
    case 94:
        ifName = MakeUnicodeString( "SSD_in_Slot_c3" );
        usMessage = MakeUnicodeString( "Exceeded 90 percent of capacity" );
        m_SnmpInterface.SendTrap(*ifName, *usMessage, SNMP_TRAP_SPECIFIC_TYPE_MINOR);
    break;
    case 100:
        ifName = MakeUnicodeString( "SSD_in_Slot_c3" );
        m_SnmpInterface.SetInterface( 13, *ifName, 2 );
        ifName = MakeUnicodeString( "SSD_in_Slot_c3" );
        usMessage = MakeUnicodeString( "Exceeded 100 percent of capacity" );
        m_SnmpInterface.SendTrap(*ifName, *usMessage, SNMP_TRAP_SPECIFIC_TYPE_CRITICAL);
    break;
    case 120:
        ifName = MakeUnicodeString( "HDD1_in_bay_2" );
        m_SnmpInterface.SetInterface( 20, *ifName, 2 );
        ifName = MakeUnicodeString( "HDD1_in_bay_2" );
        usMessage = MakeUnicodeString( "Drive failed" );
        m_SnmpInterface.SendTrap(*ifName, *usMessage, SNMP_TRAP_SPECIFIC_TYPE_CRITICAL);
    break;
    case 128:
        ifName = MakeUnicodeString( "SSD_in_Slot_c3" );
        m_SnmpInterface.SetInterface( 13, *ifName, 1 );
        ifName = MakeUnicodeString( "SSD_in_Slot_c3" );
        usMessage = MakeUnicodeString( "Below 80 percent of capacity" );
        m_SnmpInterface.SendTrap(*ifName, *usMessage, SNMP_TRAP_SPECIFIC_TYPE_MINOR);
    }

    return OK;
}

STATUS TestSnmpDdmX::Quiesce(Message *pMsg)
{
    printf("TestSnmpDdmX::Quiesce\n");
	return OK;
}

STATUS TestSnmpDdmX::RequestDefault(Message *pMsg)
{
    printf("TestSnmpDdmX::RequestDefault\n");
	return OK;
}

STATUS TestSnmpDdmX::ReplyDefault(Message *pMsg)
{
    printf("TestSnmpDdmX::ReplyDefault\n");
	return OK;
}

UnicodeString* TestSnmpDdmX::MakeUnicodeString(char* sTemp) {
	StringClass stTemp(sTemp);
	return new UnicodeString(stTemp);
}

