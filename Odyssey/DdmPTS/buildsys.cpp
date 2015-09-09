#include "buildsys.h"


	

	// 1st four entries are always there
	
	// Classname, stacksize, reference name of class
	
	CLASSENTRY("HDM_PTS", 10240, 1024, PersistentData);
	CLASSENTRY("HDM_DMGR",10240, 1024, DdmManager);
//	CLASSENTRY("HDM_TPT", 10240, 1024, Transport);
	CLASSENTRY("HDM_TIMR",10240, 1024, DdmTimer);
	
	// Customize your test image here
	CLASSENTRY("HDM_TS", 10240, 1024, DdmPTS);
	CLASSENTRY("HDM_TEST",10240, 1024, TestDdm);

	// These are all needed by the OOS
	DEVICEENTRY("Configuration",Configuration);
//	DEVICEENTRY("Interrupt", 	Interrupt);
//	DEVICEENTRY("Dma", 			Dma);
	DEVICEENTRY("DdmManager", 	DdmManager);
//	DEVICEENTRY("Transport", 	Transport);
//	DEVICEENTRY("FailSafe", 	FailSafe)
