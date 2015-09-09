//TestSnmpDdm.h

#ifndef __TestSnmpDdmX_H
#define __TestSnmpDdmX_H

#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"
#include "SsapiRequestMessage.h"
#include "SnmpInterface.h"

class TestSnmpDdmX: public Ddm {
public:
	static Ddm *Ctor(DID did);
    TestSnmpDdmX(DID did);

	STATUS Initialize(Message *pMsg);
    STATUS Enable(Message *pMsg);
    STATUS Quiesce(Message *pMsg);
    STATUS RequestDefault(Message *pMsg);
    STATUS ReplyDefault(Message *pMsg);
    STATUS InitSnmp(Message *pMsg);
    STATUS SendEvent(Message *pMsg);
	SnmpInterface m_SnmpInterface;
	UnicodeString* MakeUnicodeString(char* sTemp);

};

#endif



