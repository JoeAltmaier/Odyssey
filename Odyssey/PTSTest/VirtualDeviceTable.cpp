#include "DdmManager.h"

	// For each HDM, declare one of these:
	//  name starting with "HDM_", stack size, 
	DdmManager::Vdef vdefDm={"HDM_DMGR", 10240, true, 0,0};
	DdmManager::Vdef vdefPts={"HDM_PTS", 10240, true, 0,0};
	DdmManager::Vdef vdefTpt={"HDM_TPT", 10240, true, 0,0};
	DdmManager::Vdef vdefTest={"HDM_TEST", 10240, true, 0,0};
	DdmManager::Vdef vdefPart={"HDM_PART", 10240, true, 0,0};
	DdmManager::Vdef vdefNull={"HDM_NULL", 10240, true, 0,0};
	DdmManager::Vdef vdefTime={"HDM_TIMR", 10240, true, 0,0};


	void DdmManager::VdtInitialize() {
		// Virtual device table is born with
		// DdmManager on this slot, plus
		// Persistent Table Service mirrored on HBCs 0 and 1.
		pPVdef=new Vdef*[8];

		vdefDm.didPrimary=DeviceId::Did(iCabinet, iSlot, DID_DDMMANAGER);
		pPVdef[1]=&vdefDm;

		vdefPts.didPrimary=DeviceId::Did(iCabinet,IOP_HBC0,1);
		vdefPts.didStandby=DeviceId::Did(iCabinet,IOP_HBC1,1);
		pPVdef[VD_PERSISTENTDATA]=&vdefPts;

		vdefTpt.didPrimary=DeviceId::Did(iCabinet, iSlot, 2);
		pPVdef[2]=&vdefTpt;

		vdefTime.didPrimary=DeviceId::Did(iCabinet, iSlot, 3);
		pPVdef[3]=&vdefTime;

		// Dummy in for test.  In real life, we would listen to the VDT.
		// The PTS would update the VDT.
		vdefPart.didPrimary=DeviceId::Did(iCabinet, iSlot, 4);
		pPVdef[4]=&vdefPart;
		vdefTest.didPrimary=DeviceId::Did(iCabinet, iSlot, 5);
		pPVdef[5]=&vdefTest;
		vdefNull.didPrimary=DeviceId::Did(iCabinet, IOP_HBC1, 6);
		vdefNull.didStandby=DeviceId::Did(iCabinet, IOP_HBC0, 6);
		pPVdef[6]=&vdefNull;
		pPVdef[7]=NULL;
		}
