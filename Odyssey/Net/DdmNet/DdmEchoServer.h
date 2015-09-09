//DdmEchoServer.h

#ifndef __DdmEchoServer_H
#define __DdmEchoServer_H

#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"
#include "NetMsgs.h"

#define PLUS

class DdmEchoServer: public Ddm {
	
public:
	static Ddm *Ctor(DID did);
	DdmEchoServer(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);

	STATUS ListenReplyHandler(Message *pMsg);
//	STATUS ConnectReplyHandler(Message *pMsg);	
//	STATUS KillReplyHandler(Message *pMsg);
	STATUS WriteReplyHandler(Message *pMsg);
	STATUS ReadReplyHandler(Message *pMsg);

	int echomode;
	int iSessionID;

};

#endif