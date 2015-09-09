#include "Ddm.h"

#ifndef WIN32
	#include "TimerStatic.h"
#endif

class DdmProfile: public Ddm {
	static
	U32 *pLog;
	static
	U32 addrLow;
	static
	U32 addrHigh;
#ifndef WIN32
	TimerStatic *pTimer;
#endif

public:
	DdmProfile(DID);
	static
	Ddm *Ctor(DID);
	Status Initialize(Message *);
	static
	void DeviceInitialize();
	
private:
	Status Start(Message *);
	Status Stop(Message *);
	Status Clear(Message *);
	Status Deliver(Message *);
	static
	void TimerTick(void *);
};