#include "DdmProfile.h"

#ifndef WIN32
	#include "tc_defs.h"
#endif

#include "BuildSys.h"
#include "RqProfile.h"

	CLASSNAME(DdmProfile, SINGLE);
	DEVICENAME(Profile, DdmProfile::DeviceInitialize);
	
	SERVELOCAL(DdmProfile, RqProfileStart::requestcode);
	SERVELOCAL(DdmProfile, RqProfileStop::requestcode);
	SERVELOCAL(DdmProfile, RqProfileClear::requestcode);
	SERVELOCAL(DdmProfile, RqProfileDeliver::requestcode);

#ifndef WIN32
	extern "C" char _ftext[];
	extern "C" char _etext[];
	extern "C" TC_TCB *TCD_Execute_Task;
#endif

#ifdef WIN32
	void bzero(void *p, U32 s);
#else
	extern "C" void bzero(void *, U32);
#endif

#define IP_OFFSET 316/8
	
	U32 DdmProfile::addrLow;
	U32 DdmProfile::addrHigh;
	U32 *DdmProfile::pLog;
	
	DdmProfile::DdmProfile(DID did): Ddm(did) {
		DispatchRequest(RqProfileStart::requestcode, REQUESTCALLBACK( DdmProfile, Start ));
		DispatchRequest(RqProfileStop::requestcode, REQUESTCALLBACK( DdmProfile, Stop ));
		DispatchRequest(RqProfileClear::requestcode, REQUESTCALLBACK( DdmProfile, Clear ));
		DispatchRequest(RqProfileDeliver::requestcode, REQUESTCALLBACK( DdmProfile, Deliver ));
		
		}
	
	Ddm *DdmProfile::Ctor(DID did) {
		return new DdmProfile(did);
		}
	
	void DdmProfile::DeviceInitialize() {
#ifdef WIN32
		addrLow = 2685403136;
		addrHigh =  2685513616;
#else
		addrLow=(U32)_ftext;
		addrHigh=(U32)_etext;
#endif
		pLog=new (tBIG | tZERO) U32[(addrHigh - addrLow) >> 2];
		}
		
	Status DdmProfile::Initialize(Message *pMsg) {
#ifndef WIN32
		pTimer=new TimerStatic(TimerTick, this);
#endif
		Reply(pMsg);
		return OK;
		}
	
	Status DdmProfile::Start(Message *pMsgArg) {
		RqProfileStart *pMsg=(RqProfileStart*)pMsgArg;
#ifndef WIN32
		pTimer->Enable(pMsg->timeInterval, pMsg->timeInterval);
#endif
		Reply(pMsg, OK);
		return OK;
		}

	Status DdmProfile::Stop(Message *pMsg) {
#ifndef WIN32
		pTimer->Disable();
#endif
		Reply(pMsg, OK);
		return OK;
		}
		
#ifdef WIN32
	static int iCount = 0;
#endif

	Status DdmProfile::Clear(Message *pMsg) {
		bzero(pLog, addrHigh-addrLow);
#ifdef WIN32
		iCount = 0;
#endif
		Reply(pMsg, OK);
		return OK;
		}

	Status DdmProfile::Deliver(Message *pMsgArg) {
		RqProfileDeliver *pMsg=(RqProfileDeliver*)pMsgArg;

#ifdef WIN32
		iCount++;
		for(int i=0;i<8192;i=i+4) {
			memcpy((char *)pLog + i, &iCount, 4);
		}
		pMsg->CopyToSgl(0, 0, pLog, 8192);
		Reply(pMsg, OK);
#else

		// Return part of log asked for.
		pMsg->CopyToSgl(0, 0, &pLog[(pMsg->addrWant - addrLow) >> 2], 8192);
		Reply(pMsg, OK);
#endif
		return OK;
		}
				
	void DdmProfile::TimerTick(void *pThis) {
#ifndef WIN32
		DdmProfile *pDdm=(DdmProfile*)pThis;
		TC_TCB *pTask=TCD_Execute_Task;
		if (pTask) {
			I64 *pStack=(I64*)pTask->tc_stack_pointer;
			U32 ip=pStack[IP_OFFSET];
			if (ip >= pDdm->addrLow && ip < pDdm->addrHigh)
				pDdm->pLog[(ip - addrLow) >> 2]++;
			}
#endif
		}
