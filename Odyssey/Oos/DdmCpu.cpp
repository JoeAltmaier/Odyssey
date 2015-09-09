#include "DdmCpu.h"

#ifndef WIN32
	#include "tc_defs.h"
#endif

#include "BuildSys.h"
#include "Critical.h"

#define ONESEC 1000000


	CLASSNAME(DdmCpu, SINGLE);
	DEVICENAME(CpuStatistics, DdmCpu::DeviceInitialize);
	
	SERVELOCAL(DdmCpu, REQ_OS_CPU_STATS);

	extern "C" int TCD_IDLE_COUNTER;
	extern "C" int TCD_HISR_COUNTER;
	extern "C" int TCD_TASK_COUNTER;

	DdmCpu::DdmCpu(DID did): Ddm(did) {
		DispatchRequest(REQ_OS_CPU_STATS, REQUESTCALLBACK( DdmCpu, Stats ));
		}

	Ddm *DdmCpu::Ctor(DID did) {
		return new DdmCpu(did);
		}

	void DdmCpu::DeviceInitialize() {
		TCD_IDLE_COUNTER = 0;
		TCD_HISR_COUNTER = 0;
		TCD_TASK_COUNTER = 0;
		}

	Status DdmCpu::Initialize(Message *pMsg) {
#ifndef WIN32
		(new TimerStatic(TimerTick, this))->Enable(ONESEC, ONESEC);
#endif
		Reply(pMsg, OK);
		return OK;
		}
					
	Status DdmCpu::Stats(Message *pMsg) {
		pMsg->AddPayload(&payload, sizeof(payload));
		Reply(pMsg, OK);
		return OK;
		}
				
	void DdmCpu::TimerTick(void *pThis) {
#ifndef WIN32
		((DdmCpu*)pThis)->ProcessTimerTick();
#endif
		}

	void DdmCpu::ProcessTimerTick() {
		Critical section;
		payload.cIdleLast=TCD_IDLE_COUNTER;
		TCD_IDLE_COUNTER=0;
		payload.cHisrLast=TCD_HISR_COUNTER;
		TCD_HISR_COUNTER=0;
		payload.cTaskLast=TCD_TASK_COUNTER;
		TCD_TASK_COUNTER=0;

		payload.cIdleAvg = ( payload.cIdleAvg >> 1 ) + ( payload.cIdleLast >> 1 );
		payload.cHisrAvg = ( payload.cHisrAvg >> 1 ) + ( payload.cHisrLast >> 1 );
		payload.cTaskAvg = ( payload.cTaskAvg >> 1 ) + ( payload.cTaskLast >> 1 );
		}
