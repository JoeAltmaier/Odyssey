//
// DdmTestSample.h
//

#ifndef __DdmTestSample_H
#define __DdmTestSample_H

#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"
#include "TestMsgs.h"

typedef struct args {
	int numTicks;
} args_t;

class DdmTestSample : public Ddm {

public:
	static Ddm *Ctor(DID did);
	DdmTestSample(DID did);
	
	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);	

	// Message handlers
	STATUS Registered(Message *pMsgReq);
	STATUS TestProc(Message *pMsgReq);
	
	args_t *args;

};

#endif // __DdmTestSample_H 