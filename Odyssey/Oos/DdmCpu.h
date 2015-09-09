#include "Ddm.h"

#ifndef WIN32
	#include "TimerStatic.h"
#endif

class DdmCpu: public Ddm {
	struct {
		int cIdleLast;
		int cHisrLast;
		int cTaskLast;
		int cIdleAvg;
		int cHisrAvg;
		int cTaskAvg;
		} payload;

#ifndef WIN32
	TimerStatic *pTimer;
#endif

public:
	DdmCpu(DID);
	static
	Ddm *Ctor(DID);
	Status Initialize(Message *);
	static
	void DeviceInitialize();
	
private:
	Status Stats(Message *);
	static
	void TimerTick(void *);
	void ProcessTimerTick();
};