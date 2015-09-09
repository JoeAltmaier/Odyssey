//TestSnmpDdm.h

#ifndef __TestSnmpDdm_H
#define __TestSnmpDdm_H

#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"
#include "SsapiRequestMessage.h"
#include "SnmpInterface.h"

class TestSnmpDdm: public Ddm {
public:
	static Ddm *Ctor(DID did);
    TestSnmpDdm(DID did);

	STATUS Initialize(Message *pMsg);
    STATUS Enable(Message *pMsg);
    STATUS Quiesce(Message *pMsg);
    STATUS RequestDefault(Message *pMsg);
    STATUS ReplyDefault(Message *pMsg);
    STATUS InitSnmp(Message *pMsg);
    STATUS SendTraps(Message *pMsg);
	SnmpInterface m_SnmpInterface;
	UnicodeString* MakeUnicodeString(char* sTemp);

};

#endif



