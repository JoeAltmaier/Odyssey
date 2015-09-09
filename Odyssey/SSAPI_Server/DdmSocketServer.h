//DdmSocketServer.h

#ifndef __DdmSocketServer_H
#define __DdmSocketServer_H

#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"
#include "NetMsgs.h"
#include "SessionManager.h"
#include "SsapiRequestMessage.h"
#include "MultiPacketMessageOrganizer.h"

class DdmSocketServer: public Ddm {
	struct {
		unsigned char cIPa;
		unsigned char cIPb;
		unsigned char cIPc;
		unsigned char cIPd;
		int iPort;
	} config;
	
private:
	SessionManager m_oSessionManager;
	MultiPacketMessageOrganizer* m_pMultiPacketMessageOrganizer;
	
public:
	static Ddm *Ctor(DID did);
	DdmSocketServer(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);

	STATUS ProcessSsapiResponse(Message* pMsgReq);
	
	void CleanupSession(int iSessionID);

	void FromSsapi(SsapiRequestMessage* pSsapiMsg);
	STATUS SsapiReplyHandler(Message* pMsgReq);
	
	STATUS ListenReplyHandler(Message *pMsg);
	STATUS ConnectReplyHandler(Message *pMsg);	
	STATUS KillReplyHandler(Message *pMsg);
	STATUS WriteReplyHandler(Message *pMsg);
	STATUS ReadReplyHandler(Message *pMsg);

};

#endif