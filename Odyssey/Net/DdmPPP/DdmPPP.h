//
// DdmPPP.h
//

#ifndef __DdmPPP_H
#define __DdmPPP_H

#include "OsTypes.h"
#include "Ddm.h"
#include "Message.h"
#include "MessageBroker.h"

class DdmPPP : public Ddm {

protected:
	Message *pPPPMsg;
	CT_Task	pppTask;
	void	*pppStack;
	
public:
	static Ddm *Ctor(DID did);
	DdmPPP(DID did);
	
	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);
	
	STATUS BeginPPP(Message *pMsg);
	STATUS HangupPPP(Message *pMsg);

	STATUS HungupPPPc();
	STATUS HungupPPP(Message *pMsg);	
	
	VOID SerialPoll();
	
	char *Get_Modem_String(char *response);

	static MessageBroker *pMsgBroker;

};

#endif