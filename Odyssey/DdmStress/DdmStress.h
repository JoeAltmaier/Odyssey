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
// This class is a Stress device HDM.
// 
// Update Log: 
// 8/17/98 Joe Altmaier: Create file
/*************************************************************************/

#ifndef __DdmStress_h
#define __DdmStress_h

#include "Ddm.h"
#include "RqOsTimer.h"

class DdmStress: public Ddm {

public:
	// Stress tests begin with a message containing this payload.
	typedef struct {
		U32 cbTransfer;
		U32 nIteration;
		U32 nOverlap;
		U16 bRd;
		VDN vdn;
		} Payload;

private:

	// Statistics are kept in the test message context
	typedef struct {
		Message *pMsg;
		RqOsTimerStart *pTimer;
		VDN vdn;
		U32 cbTransfer;
		U32 nIterationTotal;
		I64 timeStart;
		// Reply statistics start here.
		I64 latencyTotal;
		I64 timeTotal;
		U32 nMessage;
		U32 nOutstanding;
		U32 nIterationSoFar;
		} Context;
		
	// Addresses are kept in the transfer message context
	typedef struct {
		Context *pContext;
		char *pBuf;
		U32 cbTransfer;
		int bSend;
		} ContextXfer;
		
	// During tests, return statistics at a configurable interval.			
	static
	struct Config {
		U32 updateInterval;
		} config;

	int nStressTest;
	Message *pMsgQuiesce;

public:

	DdmStress(DID did);

	static
	Ddm *Ctor(DID did);

	static
	void DeviceInitialize();

	STATUS Initialize(Message *pMsg);
	STATUS Quiesce(Message *);
	STATUS Enable(Message *);

protected:

	STATUS Stress_Begin(Message *);
	STATUS Stress_Echo(Message *);
	STATUS Reply_Timer(MessageReply *);	
	STATUS Reply_Xfer(MessageReply *);	

private:
	STATUS IssueRq(ContextXfer &, Message &);
};
#endif