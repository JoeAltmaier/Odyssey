#include "DdmLeak.h"
#include "BuildSys.h"
#include "RqLeak.h"

	CLASSNAME(DdmLeak, SINGLE);
	DEVICENAME(Leak, DdmLeak::DeviceInitialize);
	
	SERVELOCAL(DdmLeak, RqLeakClear::requestcode);
	SERVELOCAL(DdmLeak, RqLeakDeliver::requestcode);
	
#ifdef WIN32
	static int aassdd = 0;

	extern  void bzero(void *, U32);
	
	extern  U32 addrCodeLow;
	extern  U32 addrCodeHigh;
	extern  U32 *pHistAlloc;
	extern  U32 *pHistMax;
	extern  U32 *pHistThrash;

#else
	extern "C" void bzero(void *, U32);
	extern "C" char _ftext[];
	extern "C" char _etext[];
	extern "C" U32 addrCodeLow;
	extern "C" U32 addrCodeHigh;
	extern "C" U32 *pHistAlloc;
	extern "C" U32 *pHistMax;
	extern "C" U32 *pHistThrash;
#endif


	
	DdmLeak::DdmLeak(DID did): Ddm(did) {
		DispatchRequest(RqLeakClear::requestcode, REQUESTCALLBACK( DdmLeak, Clear ));
		DispatchRequest(RqLeakDeliver::requestcode, REQUESTCALLBACK( DdmLeak, Deliver ));
		}
	
	Ddm *DdmLeak::Ctor(DID did) {
		return new DdmLeak(did);
		}
	
	void DdmLeak::DeviceInitialize() {
#ifdef WIN32
		addrCodeLow = 2685403136;
		addrCodeHigh =  2685513616;
#else
		addrCodeLow=(U32)_ftext;
		addrCodeHigh=(U32)_etext;
#endif
		pHistAlloc=new (tBIG | tZERO) U32[(addrCodeHigh - addrCodeLow) >> 2];
		pHistMax=new (tBIG | tZERO) U32[(addrCodeHigh - addrCodeLow) >> 2];
		pHistThrash=new (tBIG | tZERO) U32[(addrCodeHigh - addrCodeLow) >> 2];
		}
		
	Status DdmLeak::Clear(Message *pMsgArg) {
#ifdef WIN32
		aassdd = 0;
#endif

		RqLeakClear *pMsg=(RqLeakClear*)pMsgArg;

		if (pMsg->maskClear & RqLeakClear::REQ_LEAK_CLEAR_ALLOC)
			bzero(pHistAlloc, addrCodeHigh-addrCodeLow);
		if (pMsg->maskClear & RqLeakClear::REQ_LEAK_CLEAR_MAX)
			bzero(pHistMax, addrCodeHigh-addrCodeLow);
		if (pMsg->maskClear & RqLeakClear::REQ_LEAK_CLEAR_THRASH)
			bzero(pHistThrash, addrCodeHigh-addrCodeLow);

		Reply(pMsg, OK);
		return OK;
		}

	Status DdmLeak::Deliver(Message *pMsgArg) {
		RqLeakDeliver *pMsg=(RqLeakDeliver*)pMsgArg;
		// Return part of log asked for.
#ifdef WIN32
		char pLog[ 8192 ];
		for(int i=0;i<8192;i=i+4) {
			memcpy((char *)pLog + i, &aassdd, 4);
		}
		pMsg->CopyToSgl(0, 0, pLog, 8192);
		aassdd++;

#else
		pMsg->CopyToSgl(0, 0, &pHistAlloc[(pMsg->addrWant - addrCodeLow) >> 2], 8192);
		if (pMsg->GetCSgl() >= 2)
			pMsg->CopyToSgl(1, 0, &pHistMax[(pMsg->addrWant - addrCodeLow) >> 2], 8192);
		if (pMsg->GetCSgl() >= 3)
			pMsg->CopyToSgl(2, 0, &pHistThrash[(pMsg->addrWant - addrCodeLow) >> 2], 8192);
#endif	// WIN32

		Reply(pMsg, OK);
		return OK;
		}
				
