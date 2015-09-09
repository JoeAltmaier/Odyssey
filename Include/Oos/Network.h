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
// This class implements initialization and maintenence of the network services
// Includes a DEVICEENTRY and Ddm
// The device initializes the devices and stack, setting the address of the
//  backplane ethernet (readying it for transport functions).  The external
//  ethernet is set to 0.0.0.0
// The Ddm checks with PTS when it comes up, obtains the current IP address,
//  and sets it.  Once set it starts the legacy services.  It continnues to
//  listen for changes to the address.  When changes are seen, it sends an
//  IPChange message to the DdmNetMgr.  Upon reply, it notifies the legacy
//  services of the change.
//
// Update Log:
// 8/27/99 Ryan Braun: Create file
/*************************************************************************/

#ifndef __Network_h
#define __Network_h

#ifdef _ODYSSEY
#define MAX_DEVICES 2
#define ETHER_EXT 0
#define ETHER_INT 1
//#define PPP 2
#else
#define MAX_DEVICES 1
#define ETHER_EXT 0
#endif _ODYSSEY

#ifndef PLUS
#define PLUS
#endif

// Define to turn Nucleus DBUG+ on (port 30)
#undef DBUGPLUS

#ifndef WIN32
// Network Includes
#include "externs.h"
#include "nucleus.h"
#include "socketd.h"
#include "tcpdefs.h"
#include "dec21143.h"
#ifdef PPP
#include "ppp.h"
#endif PPP
#endif WIN32

#include "Kernel.h"
#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"
#include "SystemConfigSettingsTable.h"
#include "NetMsgs.h"

#include "Services.h"

#ifndef WIN32
// External functions
extern "C" void	init_galileo();
extern "C" STATUS Init_Hardware();
extern "C" void DEC21143_Printcounters (DV_DEVICE_ENTRY *device);
extern "C" void DEMO_Telnet_Task(void *pParam);
#endif

extern "C" U32 gSizeReserved;

// IP address (U32) manipulators
#define IPa(addr) \
	(addr >> 24 & 0xFF)
#define IPb(addr) \
	(addr >> 16 & 0xFF)
#define IPc(addr) \
	(addr >> 8 & 0xFF)
#define IPd(addr) \
	(addr & 0xFF)

typedef U32 IP_ADDRESS;

// Legacy application states
enum {
	LEG_NOT_STARTED = 0,
	LEG_RUNNING
};

class Network : public Ddm {

public:
	Network(DID did);
	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);
	static Ddm *Ctor(DID did);

	static void DeviceInitialize();
	static void Suspend();
	static void Resume();
	static STATUS IPChange (int deviceNum, char *newIP);
	static STATUS IPChange (int deviceNum, char *newIP, char *newSubnet);
	static STATUS GatewayChange (int deviceNum, char *newGateway);
	static void FillIP(char *ip, int ipA, int ipB, int ipC, int ipD);
	static void FillIP(int *ip, int ipA, int ipB, int ipC, int ipD);
    static void FillIP(char *ip, char ipA, char ipB, char ipC, char ipD);
#ifndef WIN32
	static NU_DEVICE devices[MAX_DEVICES];
#endif

	// private member accessor functions
	static int getIP(int deviceNum);
	static int getSubnet(int deviceNum);
	static int getGateway(int deviceNum);

private:
	Message* pInitMsg;

	// network initialiation functions
#ifndef WIN32
	static STATUS NetInit ();
	static STATUS InitDevices ();
	static PPPInit (int deviceNum);
	static STATUS EtherInit (int deviceNum);
#endif

	// System Config table handlers
	STATUS SystemConfigSettingsTableInitialized(Message* pReply_);
	STATUS SystemConfigSettingsTableEnumed(Message* pReply_);
	STATUS SystemConfigSettingsRowInserted(Message* pReply_);
	void ListenOnSysConfigTable();
	STATUS SystemConfigSettingsListenReply(Message* pReply_);
	STATUS SystemConfigSettingTableModified(Message* pReply_);

	void ProcessSettingsChange(SystemConfigSettingsRecord *_Record);
	void ProcessDoneChangingIP(Message *pMsgReq);	

	// Functions for legacy initialization and IP change
	static void LegacyInit();
	static void LegacyIPChange();
    static void SnmpChangeIP();	

	// Debugger junk
#ifdef DBUGPLUS
	static void	*dbugStack;
	static CT_Task	*dbugTask;
#endif

	// cached copies of the current addresses
	static int ip_addr[MAX_DEVICES];
	static int subnet_mask[MAX_DEVICES];
	static int gateway[MAX_DEVICES];
	
	SystemConfigSettingsRecord *m_pSystemConfigRecord;	

	static int legacyRunstate;

};

#endif
