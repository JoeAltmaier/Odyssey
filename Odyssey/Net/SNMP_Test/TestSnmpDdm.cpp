//TestSnmpDdm.cpp

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"
#include <stdio.h>
#include <String.h>
#include "OsTypes.h"
#include "TestSnmpDdm.h"
#include "BuildSys.h"
#include "UnicodeString.h"
#include "..\msl\osheap.h"
#include "RqOsTimer.h"
#include "Message.h"

#define DDM_SNMP_SESSION 999
#define TENSEC    10000000
#define TWENTYSEC 20000000
#define FIVEMIN  300000000

CLASSNAME(TestSnmpDdm,SINGLE);  // Class Link Name used by Buildsys.cpp

TestSnmpDdm::TestSnmpDdm(DID did): Ddm(did) {
    printf("TestSnmpDdm::constructor\n");
}

Ddm *TestSnmpDdm::Ctor(DID did) {
    printf("TestSnmpDdm::Ctor\n");
    return new TestSnmpDdm(did);
}

STATUS TestSnmpDdm::Initialize(Message *pMsg) {
    printf("TestSnmpDdm::Initialize\n");
	Reply(pMsg,OK);
	return OK;
}

STATUS TestSnmpDdm::Enable(Message *pReqMsg) {
    RqOsTimerStart*    pStartTimerMsg;
    STATUS status;

    printf("TestSnmpDdm::Enable\n");

    pStartTimerMsg = new RqOsTimerStart(TWENTYSEC,0);
    if (pStartTimerMsg)
        status = Send(pStartTimerMsg, (ReplyCallback)&InitSnmp);

    Reply(pReqMsg,OK);
    return OK;
}

STATUS TestSnmpDdm::InitSnmp(Message* pMsg)
{
    RqOsTimerStart* pStartTimerMsg;
    STATUS status;

    printf("TestSnmpDdm::InitSnmp\n");
    delete pMsg;

    UnicodeString* usVersion = MakeUnicodeString("Version 0.36");
    UnicodeString* usProdDate = MakeUnicodeString("10/12/99");
    m_SnmpInterface.SetDescription(*usVersion, *usProdDate);

    UnicodeString* usName = MakeUnicodeString("Gemini");
    m_SnmpInterface.SetName(*usName);

    UnicodeString* usLocation = MakeUnicodeString("Nashua");
    m_SnmpInterface.SetLocation(*usLocation);

    UnicodeString* ifName = MakeUnicodeString( "HBC_in_Slot_0" );
    m_SnmpInterface.SetInterface( 1, *ifName, 1 );

    ifName = MakeUnicodeString( "HBC_in_Slot_1" );
    m_SnmpInterface.SetInterface( 2, *ifName, 1 );

    ifName = MakeUnicodeString( "NAC_in_Slot_a1" );
    m_SnmpInterface.SetInterface( 3, *ifName, 1 );

    ifName = MakeUnicodeString( "NAC_in_Slot_a2" );
    m_SnmpInterface.SetInterface( 4, *ifName, 1 );

    ifName = MakeUnicodeString( "SSD_in_Slot_a3" );
    m_SnmpInterface.SetInterface( 5, *ifName, 1 );

    ifName = MakeUnicodeString( "Filler_in_Slot_a4" );
    m_SnmpInterface.SetInterface( 6, *ifName, 1 );

    ifName = MakeUnicodeString( "NAC_in_Slot_b1" );
    m_SnmpInterface.SetInterface( 7, *ifName, 2 );

    ifName = MakeUnicodeString( "NAC_in_Slot_b2" );
    m_SnmpInterface.SetInterface( 8, *ifName, 1 );

    ifName = MakeUnicodeString( "SSD_in_Slot_b3" );
    m_SnmpInterface.SetInterface( 9, *ifName, 2 );

    ifName = MakeUnicodeString( "Filler_in_Slot_b4" );
    m_SnmpInterface.SetInterface( 10, *ifName, 1 );

    ifName = MakeUnicodeString( "NAC_in_Slot_c1" );
    m_SnmpInterface.SetInterface( 11, *ifName, 1 );

    ifName = MakeUnicodeString( "NAC_in_Slot_c2" );
    m_SnmpInterface.SetInterface( 12, *ifName, 1 );

    ifName = MakeUnicodeString( "SSD_in_Slot_c3" );
    m_SnmpInterface.SetInterface( 13, *ifName, 1 );

    ifName = MakeUnicodeString( "Filler_in_Slot_c4" );
    m_SnmpInterface.SetInterface( 14, *ifName, 1 );

    /*ifName = MakeUnicodeString( "NAC_in_Slot_d1" );
    m_SnmpInterface.SetInterface( 15, *ifName, 2 );

    ifName = MakeUnicodeString( "NAC_in_Slot_d2" );
    m_SnmpInterface.SetInterface( 16, *ifName, 1 );

    ifName = MakeUnicodeString( "SSD_in_Slot_d3" );
    m_SnmpInterface.SetInterface( 17, *ifName, 1 );

    ifName = MakeUnicodeString( "Filler_in_Slot_d4" );
    m_SnmpInterface.SetInterface( 18, *ifName, 1 );

    ifName = MakeUnicodeString( "HDD1_in_bay_0" );
    m_SnmpInterface.SetInterface( 19, *ifName, 1 );

    ifName = MakeUnicodeString( "HDD2_in_bay_1" );
    m_SnmpInterface.SetInterface( 20, *ifName, 2 );

    ifName = MakeUnicodeString( "HDD3_in_bay_2" );
    m_SnmpInterface.SetInterface( 21, *ifName, 1 );

    ifName = MakeUnicodeString( "HDD4_in_bay_3" );
    m_SnmpInterface.SetInterface( 22, *ifName, 1 );

    ifName = MakeUnicodeString( "HDD5_in_bay_4" );
    m_SnmpInterface.SetInterface( 23, *ifName, 1 );

    ifName = MakeUnicodeString( "HDD6_in_bay_5" );
    m_SnmpInterface.SetInterface( 24, *ifName, 1 );

    ifName = MakeUnicodeString( "HDD7_in_bay_6" );
    m_SnmpInterface.SetInterface( 25, *ifName, 2 );

    ifName = MakeUnicodeString( "HDD8_in_bay_7" );
    m_SnmpInterface.SetInterface( 26, *ifName, 1 );

    ifName = MakeUnicodeString( "HDD9_in_bay_8" );
    m_SnmpInterface.SetInterface( 27, *ifName, 1 );

    ifName = MakeUnicodeString( "HDD10_in_bay_9" );
    m_SnmpInterface.SetInterface( 28, *ifName, 1 );

    ifName = MakeUnicodeString( "HDD11_in_bay_10" );
    m_SnmpInterface.SetInterface( 29, *ifName, 1 );

    ifName = MakeUnicodeString( "HDD12_in_bay_11" );
    m_SnmpInterface.SetInterface( 30, *ifName, 1 );

    ifName = MakeUnicodeString( "HDD13_in_bay_12" );
    m_SnmpInterface.SetInterface( 31, *ifName, 1 );

    ifName = MakeUnicodeString( "HDD14_in_bay_13" );
    m_SnmpInterface.SetInterface( 32, *ifName, 1 );

    ifName = MakeUnicodeString( "HDD15_in_bay_14" );
    m_SnmpInterface.SetInterface( 33, *ifName, 1 );

    ifName = MakeUnicodeString( "HDD16_in_bay_15" );
    m_SnmpInterface.SetInterface( 34, *ifName, 1 );

    ifName = MakeUnicodeString( "HDD17_in_bay_16" );
    m_SnmpInterface.SetInterface( 35, *ifName, 1 );

    ifName = MakeUnicodeString( "HDD18_in_bay_17" );
    m_SnmpInterface.SetInterface( 36, *ifName, 1 );

    ifName = MakeUnicodeString( "HDD19_in_bay_18" );
    m_SnmpInterface.SetInterface( 37, *ifName, 1 );

    ifName = MakeUnicodeString( "HDD20_in_bay_19" );
    m_SnmpInterface.SetInterface( 38, *ifName, 1 );*/

    m_SnmpInterface.AddTrapAddress(0x0A1E011CUL);           /* Add 10.30.1.28 (Loon) */
    m_SnmpInterface.AddTrapAddress(0x0A1E0116UL);           /* Add 10.30.1.22 (Moose) */

    NU_Sleep(1000);
    printf("Calling SNMPInitComplete\n");
    m_SnmpInterface.SNMPInitComplete();

    pStartTimerMsg = new RqOsTimerStart(TENSEC, TENSEC);
    if (pStartTimerMsg)
        status = Send(pStartTimerMsg, (ReplyCallback)&SendTraps);

    return OK;
}

STATUS TestSnmpDdm::SendTraps(Message* pMsg)
{
    static U32 total = 0;

    printf("TestSnmpDdm::SendTraps\n");
    delete pMsg;

    switch (++total) {
    case 1:
        UnicodeString* ifName = MakeUnicodeString( "SSD_in_Slot_c3" );
        UnicodeString* usMessage = MakeUnicodeString( "Exceeded 80 percent of capacity" );
        m_SnmpInterface.SendTrap(*ifName, *usMessage, SNMP_TRAP_SPECIFIC_TYPE_WARNING);
		break;
    case 2:
        ifName = MakeUnicodeString( "SSD_in_Slot_c3" );
        usMessage = MakeUnicodeString( "Exceeded 90 percent of capacity" );
        m_SnmpInterface.SendTrap(*ifName, *usMessage, SNMP_TRAP_SPECIFIC_TYPE_MINOR);
		break;
    case 3:
        ifName = MakeUnicodeString( "SSD_in_Slot_c3" );
        m_SnmpInterface.SetInterface( 13, *ifName, 2 );
        NU_Sleep(500);
        ifName = MakeUnicodeString( "SSD_in_Slot_c3" );
        usMessage = MakeUnicodeString( "Exceeded 100 percent of capacity" );
        m_SnmpInterface.SendTrap(*ifName, *usMessage, SNMP_TRAP_SPECIFIC_TYPE_CRITICAL);
	}

    return OK;
}

STATUS TestSnmpDdm::Quiesce(Message *pMsg)
{
    printf("TestSnmpDdm::Quiesce\n");
	return OK;
}

STATUS TestSnmpDdm::RequestDefault(Message *pMsg)
{
    printf("TestSnmpDdm::RequestDefault\n");
	return OK;
}

STATUS TestSnmpDdm::ReplyDefault(Message *pMsg)
{
    printf("TestSnmpDdm::ReplyDefault\n");
	return OK;
}

UnicodeString* TestSnmpDdm::MakeUnicodeString(char* sTemp) {
	StringClass stTemp(sTemp);
	return new UnicodeString(stTemp);
}

