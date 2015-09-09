/*************************************************************************/
// This material is a confidential trade secret and proprietary
// information of ConvergeNet Technologies, Inc. which may not be
// reproduced, used, sold or transferred to any third party without the
// prior written consent of ConvergeNet Technologies, Inc.  This material
// is also copyrighted as an unpublished work under sections 104 and 408
// of Title 17 of the United States Code.  Law prohibits unauthorized
// use, copying or reproduction.
//
// Description:
// This class implements initialization of the network services
//
// Update Log:
// 8/27/99 Ryan Braun: Create file
/*************************************************************************/

#ifndef _TRACEf
#define _TRACEf
#include "Trace_Index.h"
#include "Odyssey_trace.h"
#endif

#define TRACE_INDEX		TRACE_NETWORK

#include "Oos\Network.h"

#if (SNMP_INCLUDED!=0)
extern "C" STATUS NU_Init_Snmp(VOID *);
extern "C" STATUS SNMP_Initialize(VOID);
extern "C" void set_hostid(uint32);
extern "C" void set_portmask(uint32);
#endif

#ifdef _ODYSSEY
#include "HbcSubnet.h"
#endif

#include "String.h"
#include "BuildSys.h"

CLASSNAME(Network, SINGLE);
DEVICENAME(Network, Network::DeviceInitialize);
FAILNAME(Network, Network::Suspend);
SUSPENDNAME(Network, Network::Suspend, Network::Resume);

#ifndef WIN32
NU_MEMORY_POOL System_Memory;
NU_DEVICE Network::devices[MAX_DEVICES];
#endif

#ifdef DBUGPLUS
void *Network::dbugStack;
CT_Task *Network::dbugTask;
#endif

int Network::legacyRunstate;

int Network::ip_addr[MAX_DEVICES];
int Network::subnet_mask[MAX_DEVICES];
int Network::gateway[MAX_DEVICES];

#ifdef WIN32
char GetNextPart(FILE* fp) {
	char sTemp[4];
	int i=0;
	char ch = getc(fp);
	while(i<3 && ch != '.' && !feof(fp)) {
		sTemp[i++] = ch;
		ch = getc(fp);
	}
	sTemp[i] = 0;

	return (char) atoi(sTemp);
}
#else
extern unsigned char box_IP_address[4];
#endif

Network::Network(DID did) : Ddm(did) {

	TRACE_ENTRY(Network::Network);

}

Ddm *Network::Ctor(DID did) {
	return new Network(did);
}

STATUS Network::Initialize(Message *pMsg) {

	TRACE_ENTRY(Network::Initialize);

	// Set the legacy runstate to unstarted
	legacyRunstate = LEG_NOT_STARTED;

#ifndef WIN32

	SystemConfigSettingsRecord::RqDefineTable *preqDefTab = 
		new SystemConfigSettingsRecord::RqDefineTable(Persistant_PT, 1);
	assert (preqDefTab);
		
	// send table definition request
	Send(preqDefTab, REPLYCALLBACK (Network, SystemConfigSettingsTableInitialized));
	
#else

	char cIPa, cIPb, cIPc, cIPd;
	FILE * fp = fopen("ssapi_server.config","r");
	if(fp) {
		cIPa = GetNextPart(fp);
		cIPb = GetNextPart(fp);
		cIPc = GetNextPart(fp);
		cIPd = GetNextPart(fp);
		fclose(fp);
	}
	else {
		printf("\nError reading from ssapi_server.config... IP address set to josh's ;)\n");
		cIPa = (char)10;
		cIPb = (char)40;
		cIPc = (char)20;
		cIPd = (char)100;
	}

	FillIP(&ip_addr[ETHER_EXT], cIPa, cIPb, cIPc, cIPd);
	FillIP(&subnet_mask[ETHER_EXT], 255, 255, 255, 0);
	FillIP(&gateway[ETHER_EXT], cIPa, cIPb, cIPc, 1);

#endif
	
	pInitMsg = pMsg;
	
	return OK;

}

STATUS Network::Enable(Message *pMsg) {

	TRACE_ENTRY(Network::Enable);

	Reply(pMsg, OK);
	return OK;

}

STATUS Network::SystemConfigSettingsTableInitialized(Message* pReply_)
{

	TRACE_ENTRY(Network::SystemConfigSettingsTableInitialized);
	
	// make sure everything went ok
	assert (pReply_->Status()==OK || pReply_->Status()==ercTableExists);
	
	if (pReply_->Status() == ercTableExists)
	{
		SystemConfigSettingsRecord::RqEnumTable *preqEnumTab = 
			new SystemConfigSettingsRecord::RqEnumTable();
		assert (preqEnumTab);
		
		// send table enumeration request
		Send(preqEnumTab, REPLYCALLBACK (Network, SystemConfigSettingsTableEnumed));
		
	}
	else
	{
		m_pSystemConfigRecord = new SystemConfigSettingsRecord();
		FillIP((int *)&m_pSystemConfigRecord->ipAddress, box_IP_address[0], box_IP_address[1], box_IP_address[2], box_IP_address[3]);
		FillIP((int *)&m_pSystemConfigRecord->subnetMask, 255, 255, 255, 0);
		FillIP((int *)&m_pSystemConfigRecord->gateway, box_IP_address[0], box_IP_address[1], box_IP_address[2], 1);		
		SystemConfigSettingsRecord::RqInsertRow *preqInsertRow =
			new SystemConfigSettingsRecord::RqInsertRow(*m_pSystemConfigRecord);
		assert(preqInsertRow);
		
		// send table insert request
		Send(preqInsertRow, REPLYCALLBACK (Network, SystemConfigSettingsRowInserted));		
	}

	return OK;
		
}

STATUS Network::SystemConfigSettingsTableEnumed(Message* pReply_)
{

	TRACE_ENTRY(Network::SystemConfigSettingsTableEnumed);

	U32 numberOfRows;
	
	// make sure everything went ok
	assert (pReply_->Status()==OK);
	
	SystemConfigSettingsRecord::RqEnumTable* pReply = 
	 	(SystemConfigSettingsRecord::RqEnumTable*) pReply_;
	
	numberOfRows = pReply->GetRowCount();	
	assert(numberOfRows==0 || numberOfRows==1);
	
	if (numberOfRows == 0)
	{
		m_pSystemConfigRecord = new SystemConfigSettingsRecord();
		Network::FillIP((int *)&m_pSystemConfigRecord->ipAddress, box_IP_address[0], box_IP_address[1], box_IP_address[2], box_IP_address[3]);
		Network::FillIP((int *)&m_pSystemConfigRecord->subnetMask, 255, 255, 255, 0);
		Network::FillIP((int *)&m_pSystemConfigRecord->gateway, box_IP_address[0], box_IP_address[1], box_IP_address[2], 1);		
		SystemConfigSettingsRecord::RqInsertRow *preqInsertRow =
			new SystemConfigSettingsRecord::RqInsertRow(*m_pSystemConfigRecord);
		assert(preqInsertRow);
		
		// send table insert request
		Send(preqInsertRow, REPLYCALLBACK (Network, SystemConfigSettingsRowInserted));
	}
	else
	{
		m_pSystemConfigRecord = pReply->GetRowCopy();
	
		// if there is a debug IP address (box_IP_address), update the PTS to use this
		// IP instead of what is currently there
		if ((box_IP_address[0] != 0) && (memcmp(&m_pSystemConfigRecord->ipAddress, box_IP_address, sizeof(IP_ADDRESS))))
		{
		
			TRACEF(TRACE_L4, ("Network: Ignoring IP address in PTS - using debug address and placing in PTS\n"));
	
			IP_ADDRESS debugIP;
			IP_ADDRESS debugSubnet;
			IP_ADDRESS debugGateway;
			
			Network::FillIP((int *)&debugIP, box_IP_address[0], box_IP_address[1], box_IP_address[2], box_IP_address[3]);
			Network::FillIP((int *)&debugSubnet, 255, 255, 255, 0);
			Network::FillIP((int *)&debugGateway, box_IP_address[0], box_IP_address[1], box_IP_address[2], 1);		

		
			SystemConfigSettingsRecord::RqModifyField *preqModifyField;

			// send modify IP request
			preqModifyField = new SystemConfigSettingsRecord::RqModifyField(
					m_pSystemConfigRecord->rid,
					"ipAddress", &debugIP, sizeof(IP_ADDRESS));	
			assert (preqModifyField);
			Send(preqModifyField, REPLYCALLBACK (Network, SystemConfigSettingTableModified));			
			
			// send modify subnet request
			preqModifyField = new SystemConfigSettingsRecord::RqModifyField(
					m_pSystemConfigRecord->rid,
					"subnetMask", &debugSubnet, sizeof(IP_ADDRESS));	
			assert (preqModifyField);
			Send(preqModifyField, REPLYCALLBACK (Network, SystemConfigSettingTableModified));			

			// send modify gateway request
			preqModifyField = new SystemConfigSettingsRecord::RqModifyField(
					m_pSystemConfigRecord->rid,
					"gateway", &debugGateway, sizeof(IP_ADDRESS));	
			assert (preqModifyField);
			Send(preqModifyField, REPLYCALLBACK (Network, SystemConfigSettingTableModified));						

		}
	
		// now init a listen on the table
		ListenOnSysConfigTable();
	}
		
	delete pReply;

	return OK;
	
}

STATUS Network::SystemConfigSettingsRowInserted(Message* pReply_)
{

	TRACE_ENTRY(Network::SystemConfigSettingsRowInserted);

	SystemConfigSettingsRecord::RqInsertRow* pReply = 
		(SystemConfigSettingsRecord::RqInsertRow*) pReply_;
	
	// make sure everything went ok
	assert (pReply_->Status()==OK);
	
	m_pSystemConfigRecord->rid = *pReply->GetRowIdPtr();
	
	delete pReply;

	// now init a listen on the table
	ListenOnSysConfigTable();
				
	return OK;
	
}

void Network::ListenOnSysConfigTable()
{
	TRACE_ENTRY(Network::ListenOnSysConfigTable);
	
	SystemConfigSettingsRecord::RqListen* preqListen = 
		new SystemConfigSettingsRecord::RqListen();
		
	// send the listen request
	Send(preqListen, REPLYCALLBACK(Network,	SystemConfigSettingsListenReply));
	
}

// We received notification of our listen request
// The first one coming back should have a good configuration in it
STATUS Network::SystemConfigSettingsListenReply(Message* pReply_)
{
	TRACE_ENTRY(Network::SystemConfigSettingsListenReply);

	SystemConfigSettingsRecord::RqListen* pReply = 
		(SystemConfigSettingsRecord::RqListen*) pReply_;
		
	// normal listen reply
	assert(pReply->GetTableCount() == 1);
	delete m_pSystemConfigRecord;
	m_pSystemConfigRecord = pReply->GetRowCopy();
	
	ProcessSettingsChange(m_pSystemConfigRecord);
	
	if (pReply->IsInitialReply())
		Reply(pInitMsg);
	
	return OK;
}

STATUS Network::SystemConfigSettingTableModified(Message* pReply)
{
	TRACE_ENTRY(Network::SystemConfigSettingTableModified);
		
	delete pReply;
	
	return OK;
}

// Notification of a change in the SystemConfigSettings record
// Process any changes in the IP, subnet, or gateway
// When done, notify the legacy applications that the IP changed
void Network::ProcessSettingsChange(SystemConfigSettingsRecord *_Record)
{
	TRACE_ENTRY(Network::ProcessSettingsChange);

	IP_ADDRESS newIP;
	IP_ADDRESS newSubnet;
	IP_ADDRESS newGateway;

	memcpy(&newIP, &_Record->ipAddress, sizeof(IP_ADDRESS));
	memcpy(&newSubnet, &_Record->subnetMask, sizeof(IP_ADDRESS));
	memcpy(&newGateway, &_Record->gateway, sizeof(IP_ADDRESS));		

	if ((newIP != ip_addr[ETHER_EXT]) || (newSubnet != subnet_mask[ETHER_EXT]))
	{
#ifndef WIN32
		NetMsgChangeIP *pIPMsg = new NetMsgChangeIP(IPa(newIP), IPb(newIP), IPc(newIP), IPd(newIP), IPa(newSubnet), IPb(newSubnet), IPc(newSubnet), IPd(newSubnet));
		Send(pIPMsg, NULL, REPLYCALLBACK(Network, ProcessDoneChangingIP));
#endif

		memcpy(&ip_addr[ETHER_EXT], &newIP, 4);
		memcpy(&subnet_mask[ETHER_EXT], &newSubnet, 4);
	}

	if (newGateway != gateway[ETHER_EXT])
	{
		NetMsgChangeGateway *pGWMsg = new NetMsgChangeGateway(IPa(newGateway), IPb(newGateway), IPc(newGateway), IPd(newGateway));
		Send(pGWMsg, NULL, REPLYCALLBACK(DdmOsServices, DiscardOkReply));
		memcpy(&gateway[ETHER_EXT], &newGateway, 4);
	}

}

// Done changing IP, perform legacy changeover
void Network::ProcessDoneChangingIP(Message *pMsgReq) {

	NetMsgChangeIP *pMsg = (NetMsgChangeIP *)pMsgReq;

	if (legacyRunstate == LEG_NOT_STARTED)
	{
#if (SNMP_INCLUDED!=0)
        SnmpChangeIP();
#endif	
		LegacyInit();
		legacyRunstate = LEG_RUNNING;
	}
	else if (legacyRunstate == LEG_RUNNING)
	{
		// Notify legacy apps of change
		LegacyIPChange();
	}

	delete pMsg;

}

// Notify the SNMP service that the IP changed
void Network::SnmpChangeIP() {

#if (SNMP_INCLUDED!=0)
    set_hostid(ip_addr[ETHER_EXT]);
    set_portmask(subnet_mask[ETHER_EXT]);
#endif

}

// Initialize the legacy (Nucleus sockets layer) services
void Network::LegacyInit() {

	TRACEF(TRACE_L4, ("Initializing legacy network applications..\n"));
#ifndef WIN32

#if (SNMP_INCLUDED!=0)

	SnmpChangeIP();

    void *first_available_memory = (void *) new(tBIG|tPCI|tUNCACHED) U8[200000];

	if (first_available_memory == 0)
	{
      TRACEF(TRACE_L1, ("Unable to allocate system memory for SNMP\n"));
	  return;
	}
	else
	{
      TRACEF(TRACE_L8, ("SNMP first_available_memory = %8x\n", first_available_memory));
    }

    if (NU_Init_Snmp(first_available_memory))
    {
		TRACEF(TRACE_L1, ("SNMP Init returned error\n"));
	}
#endif

#endif WIN32

}

// Notify the legacy (Nucleus sockets layer) services that the IP has changed
void Network::LegacyIPChange() {

	TRACEF(TRACE_L4, ("Notifying legacy network applications of IP change..\n"));
#ifndef WIN32

#if (SNMP_INCLUDED!=0)
	SnmpChangeIP();
#endif

#endif WIN32	
	

}

void Network::DeviceInitialize() {

	TRACE_ENTRY(Network::DeviceInitialize);

#ifndef WIN32
	// start the network subsystem
	NetInit();
#endif

}

#ifndef WIN32
STATUS Network::NetInit ()
{

    void		*first_available_memory;
    STATUS		status;

	TRACE_ENTRY(Network::NetInit);

	// Set IP, Subnet, and gateways to 0.0.0.0
	for (int i=0; i < MAX_DEVICES; i++)
	{
		ip_addr[i] = 0;
		subnet_mask[i] = 0;
		gateway[i] = 0;
	}

	// Set and attach internal ethernet address
#ifdef _ODYSSEY
	if (Address::iSlotMe == IOP_HBC0)
	{
		ip_addr[ETHER_INT] = NETADDR + 1;
	}
	else if (Address::iSlotMe == IOP_HBC1)
	{
		ip_addr[ETHER_INT] = NETADDR + 2;
	}
	else
	{
		TRACEF(TRACE_L1, ("Error - network initializing on non-HBC!\n"));
	}

	subnet_mask[ETHER_INT] = SUBNET;
#endif _ODYSSEY

	// Configure PPP to correct addresses
	// Use 2 host subnet (/30) above internal HBC ethernet subnet
#ifdef PPP
//	ip_addr[PPP] = NETADDR + 5;
//	subnet_mask[PPP] = SUBNET;
	ip_addr[PPP] = 0x0A281501;
	subnet_mask[PPP] = 0xFFFFFF00;
#endif

	// Initialize the network
    first_available_memory = (void *) new(tBIG|tPCI|tUNCACHED) U8[400000];

	if (first_available_memory == 0)
	{
	  TRACEF(TRACE_L1, ("Unable to allocate memory for SYSMEM\n"));
	  return OK;
	}
	else
	{
	  TRACEF(TRACE_L8, ("first_available_memory = %8x\n", first_available_memory));
	}

    status = NU_Create_Memory_Pool(&System_Memory, "SYSMEM", first_available_memory, 800000, 56, NU_FIFO);
   	if (status == NU_SUCCESS)
   	{
      TRACEF(TRACE_L8, ("SYSMEM creation successful\n"));
    }
    else
    {
      TRACEF(TRACE_L1, ("Unable to create SYSMEM memory pool!\n"));
    }

   	status = NU_Init_Net();

    if (status == NU_SUCCESS)
    {
      TRACEF(TRACE_L8, ("Net startup successful\n"));
    }
    else
    {
   	  TRACEF(TRACE_L1, ("NET STARTUP FAIL!\n"));
   	}

	// Initialize Devices
	status = InitDevices();

#ifdef _ODYSSEY
	// Attach the correct address to the internal ethernet
	IPChange(1, (char *)&ip_addr[ETHER_INT], (char *)&subnet_mask[ETHER_INT]);
#endif _ODYSSEY

#ifdef DBUGPLUS
// Start the debugger (port 30)
	TRACEF(TRACE_L4, ("Spawning the debugger\n"));
	dbugTask = new(tZERO) CT_Task;
	dbugStack = new(tZERO) U8[3000];
	Kernel::Create_Thread(*dbugTask, "DBUG+", DEMO_Telnet_Task, NULL, dbugStack, 3000);
	Kernel::Schedule_Thread(*dbugTask);
	TRACEF(TRACE_L4, ("Debugger started\n"));
#endif

	return (OK);

}

STATUS Network::InitDevices ()
{

	STATUS status;

	TRACE_ENTRY(Network::InitDevices);

	// setup descriptors for init
#ifdef _ODYSSEY
#ifdef ETHER_EXT
	status = EtherInit(ETHER_EXT);
#endif ETHER_EXT
#ifdef ETHER_INT
	status = EtherInit(ETHER_INT);
#endif ETHER_INT
#ifdef PPP
	status = PPPInit(PPP);
#endif

#else
	status = EtherInit(0);
#endif _ODYSSEY

	TRACEF(TRACE_L4, ("Performing network device initialization\n"));
	// init devices
    if (NU_Init_Devices (devices, MAX_DEVICES) != NU_SUCCESS)
    {
        TRACEF(TRACE_L1, ("Failed to initailize the device(s).\n"));
    }
    else
    {
    	TRACEF(TRACE_L4, ("Network device initialization completed.\n"));
    }
    
	return (OK);

}

STATUS Network::EtherInit (int deviceNum)
{

	// Attach null IP to device - it will be determined and attached later
	uchar               ip_addr[] = {0, 0, 0, 0};
    uchar               subnet[]  = {255, 255, 255, 0};

	TRACEF(TRACE_L7, ("Initializing data for ethernet [%d]\n", deviceNum));

	/* Initialize the Hardware */
#ifdef  _ODYSSEY
#else
        init_galileo();
#endif  _ODYSSEY

	/* set up the ethernet */
if (deviceNum == 0)
	devices[deviceNum].dv_name = "DEC21143_EXT";
else if (deviceNum == 1)
	devices[deviceNum].dv_name = "DEC21143_INT";
#ifdef INCLUDE_ODYSSEY
    devices[deviceNum].dv_hw.ether.dv_irq = 0;			   /* Will be obtained from PCI config space */
#else
	devices[deviceNum].dv_hw.ether.dv_irq = 0;			   /* Required for use on eval system */
#endif
    devices[deviceNum].dv_hw.ether.dv_io_addr = 0x0L;      /* unused    			*/
    devices[deviceNum].dv_hw.ether.dv_shared_addr = 0;     /* paramaters.           */

    devices[deviceNum].dv_init = DEC21143_Init;
    devices[deviceNum].dv_flags = 0;
    memcpy (devices[deviceNum].dv_ip_addr, ip_addr, 4);
    memcpy (devices[deviceNum].dv_subnet_mask, subnet, 4);
    devices[deviceNum].dv_use_rip2 = 0;
    devices[deviceNum].dv_ifmetric = 0;
    devices[deviceNum].dv_recvmode = 0;
    devices[deviceNum].dv_sendmode = 0;

	return (OK);

}

#ifdef PPP
STATUS Network::PPPInit (int deviceNum)
{

	// Attach null IP to device - it will be determined and attached later
	uchar               ip_addr[] = {0, 0, 0, 0};
    uchar               subnet[]  = {255, 255, 255, 252};

	TRACEF(TRACE_L7, ("Initializing data for PPP [%d]\n", deviceNum));

	/* PPP Initialization */
    memcpy (devices[0].dv_ip_addr, ip_addr, 4);         /* Not use by PPP. */
    memcpy (devices[0].dv_subnet_mask, subnet, 4); /* Not use by PPP. */
    devices[PPP].dv_name = "PPP_Console";
    devices[PPP].dv_init = PPP_Initialize;
    devices[PPP].dv_flags = (DV_POINTTOPOINT | DV_NOARP);

    devices[PPP].dv_hw.uart.com_port         = COM1;
    devices[PPP].dv_hw.uart.baud_rate        = 57600;
    devices[PPP].dv_hw.uart.parity           = PARITY_NONE;
    devices[PPP].dv_hw.uart.stop_bits        = STOP_BITS_1;
    devices[PPP].dv_hw.uart.data_bits        = DATA_BITS_8;

	return (OK);

}
#endif PPP

#endif WIN32

STATUS Network::IPChange (int deviceNum, char *newIP)
{

#ifndef WIN32
    return(IPChange(deviceNum, newIP, (char *)devices[deviceNum].dv_subnet_mask));
#else
	return(0);
#endif

}

STATUS Network::IPChange (int deviceNum, char *newIP, char *newSubnet)
{

#ifndef WIN32
	STATUS status;
#if 0
	// Reset the IP of the PPP device so it does not conflict with the ethernet device
	if ((newIP[0] == 192) && (newIP[1] == 168))
	{
		// Set address to 172.16.1.1
		ip_addr[PPP] = 0xAC100101;
	}
	else
	{
		// Set address to 192.168.1.1
		ip_addr[PPP] = 0xC0A80101;
	}
#endif

    memcpy (devices[deviceNum].dv_ip_addr, newIP, 4);
	memcpy (devices[deviceNum].dv_subnet_mask, newSubnet, 4);

	// Does Attach_IP reset the network route? hmmm - better check because we need to
	if ((status = DEV_Attach_IP_To_Device(devices[deviceNum].dv_name,
    	(uint8 *)devices[deviceNum].dv_ip_addr,
	 	(uint8 *)devices[deviceNum].dv_subnet_mask)) != NU_SUCCESS)
	{
		TRACEF(TRACE_L1, ("Failed to attach new IP to device (%s)\n\r", devices[deviceNum].dv_name));
		return(-1);
	}
	TRACEF(TRACE_L4, ("Attaching IP to %s\n", devices[deviceNum].dv_name));
#else WIN32
	Tracef("Attaching IP to interface\n");
#endif
   	return(0);
}

STATUS Network::GatewayChange (int deviceNum, char *newGateway)
{

#ifndef WIN32
	STATUS status;
	uchar nullip[4] = {0, 0, 0, 0,};
	if ((status = NU_Add_Route(nullip, devices[deviceNum].dv_subnet_mask, (uchar *)newGateway)) != NU_SUCCESS)
	{
		TRACEF(TRACE_L1, ("Failed to change gateway\n\r"));
		return(-1);
	}
	TRACEF(TRACE_L4, ("Changing gateway of %s\n", devices[deviceNum].dv_name));
#else WIN32
	Tracef("Changing gateway\n");
#endif

	return(0);

}

void Network::FillIP (int *ip, int ipA, int ipB, int ipC, int ipD)
{
	*ip = (ipA << 24) | (ipB << 16) | (ipC << 8) | ipD;
}

void Network::FillIP (char *ip, int ipA, int ipB, int ipC, int ipD)
{

	ip[0] = ipA;
	ip[1] = ipB;
	ip[2] = ipC;
	ip[3] = ipD;

}

void Network::FillIP (char *ip, char ipA, char ipB, char ipC, char ipD)
{

	ip[0] = ipA;
	ip[1] = ipB;
	ip[2] = ipC;
	ip[3] = ipD;

}


int Network::getIP(int deviceNum)
{
	if (deviceNum < MAX_DEVICES)
	{
		return ip_addr[deviceNum];
	}
	else
	{
		TRACEF(TRACE_L1, ("Attemping to get IP for invalid device!\n"));
		return 0;
	}
}

int Network::getSubnet(int deviceNum)
{
	if (deviceNum < MAX_DEVICES)
	{
		return subnet_mask[deviceNum];
	}
	else
	{
		TRACEF(TRACE_L1, ("Attemping to get Subnet Mask for invalid device!\n"));
		return 0;
	}
}

int Network::getGateway(int deviceNum)
{
	if (deviceNum < MAX_DEVICES)
	{
		return gateway[deviceNum];
	}
	else
	{
		TRACEF(TRACE_L1, ("Attemping to get gateway address for invalid device!\n"));
		return 0;
	}
}

void Network::Suspend()
{

	// We need to shut off our networking to allow for the PCI
	// clock switch
	// Should we detach the IP as well?

}

void Network::Resume()
{

	// Safe for PCI traffic again, re-enable networking

}