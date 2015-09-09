//DdmSnmp.h

#ifndef __DdmSnmp_H
#define __DdmSnmp_H

#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"
#include "SsapiRequestMessage.h"
#include "SnmpInterface.h"
#include "SNMPTrapDataHolder.h"

class DdmSnmp: public Ddm {	
public:
	static Ddm *Ctor(DID did);
	DdmSnmp(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);

	void SendDeviceListen();
	void SendAlarmListen();
	void SendDeviceList();

	STATUS ListenReplyHandler(Message* pMsgReq);
	STATUS DeviceListReplyHandler(Message* pMsgReq);
	STATUS SourceResolutionReplyHandler(Message* pMsgReq);
	void AlarmResolutionComplete(int iTrapKey);

	void UpdateDevice(ValueSet* pvsDevice);
	void UpdateChassisData(ValueSet* pvsChassis);
	int FindIndex(int iLocation);

	int m_aIndexMap[100];
	int m_iIndexMapEntries;

	SNMPTrapDataHolder m_SnmpTrapHolder;

	SnmpInterface m_SnmpInterface;

	UnicodeString* MakeUnicodeString(char* sTemp);
private:
	void PrefixString(char* sPrefix, UnicodeString& usTarget, ValueSet* pVS, int iSsapiCode);
};

#endif