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
// This class is a SGL-test HDM.
// 
// Update Log: 
// 4/21/99 Joe Altmaier: Create file
/*************************************************************************/

#ifndef __DdmSgl_h
#define __DdmSgl_h

#include "Ddm.h"
#include "RqOsTimer.h"

class DdmSgl: public Ddm {
	// Sgl tests begin with a message containing this payload.
	typedef struct {
		U32 cbTransfer;
		U32 nOverlap;
		U32 nIteration;
		VDN vdn;
		TySlot tySlot;
		} Payload;

	// Statistics are kept in the test message context
	typedef struct {
		Message *pMsg;
		VDN vdn;
		TySlot tySlot;
		U32 cbTransfer;
		U32 nOutstanding;
		U32 nIterationSoFar;
		U32 nIterationTotal;
		} Context;
		
	// Addresses are kept in the transfer message context
	typedef struct {
		Context *pContext;
		U32 cbTransfer;
		} ContextXfer;

	char *pData;		
	int nSglTest;
	Message *pMsgQuiesce;

public:

	DdmSgl(DID did);

	static
	Ddm *Ctor(DID did);

	STATUS Initialize(Message *);
	STATUS Quiesce(Message *);
	STATUS Enable(Message *);

protected:

	STATUS Sgl_Begin(Message *);
	STATUS Sgl_Echo(Message *);
	STATUS Reply_Timer(MessageReply *);	
	STATUS Reply_Xfer(MessageReply *);	

private:
	STATUS IssueRq(ContextXfer &, Message &);
};
#endif