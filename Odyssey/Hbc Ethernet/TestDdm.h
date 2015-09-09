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
// This class tests Ddm functionality.
// 
// Update Log: 
// 8/05/98 Joe Altmaier: Create file
/*************************************************************************/

#ifndef __Test_h
#define __Test_h

//#include "Nucleus.h"
#include "Message.h"
#include "Ddm.h"


class TestDdm: public Ddm {
	struct {
		VDN vd;
		} config;
	
	unsigned long lba;
	unsigned long ticks;
	unsigned long nTimes;
	void *pBuf;

	static
	TestDdm *pTestDdm;
			
public:
	TestDdm(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *);
	static
	Ddm *Ctor(DID did);

	static
	void StartStress();
	static
	void StartSgl();
	static
	void StartProfile();
	static
	void StopProfile();
	static
	void ClearProfile();
private:
	STATUS ReplyTimer(MessageReply *pMsg);
	STATUS ReplyStress(MessageReply *pMsg);
	STATUS ReplySgl(MessageReply *pMsg);

	static
	STATUS ActionMethod(void *pPayload);
};
#endif