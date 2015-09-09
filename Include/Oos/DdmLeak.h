#include "Ddm.h"

class DdmLeak: public Ddm {

public:
	DdmLeak(DID);
	static
	Ddm *Ctor(DID);
	static
	void DeviceInitialize();
	
private:
	Status Clear(Message *);
	Status Deliver(Message *);
};