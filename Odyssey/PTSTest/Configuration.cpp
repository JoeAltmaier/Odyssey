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
// This class implements an api for PersistentData service, and the Ddm
// that accomplishes the service.
// 
// Update Log: 
// 8/11/98 Joe Altmaier: Create file
// 9/29/98 Jim Frandeen: Use CtTypes.h instead of stddef.h
/*************************************************************************/

#include <String.h>
#include "CtTypes.h"
#include "DeviceId.h"
#include "Configuration.h"
#include "BuildSys.h"

	DEVICENAME(Configuration, Configuration::Initialize);


	struct {int iCabinet; TySlot iSlot;} DdmManagerParams
		={0, IOP_HBC0};

	struct {long cbMsg; long sMsg; long cbBuf; long sBuf; long pciWindow; long sWindow; } MessengerParams
		={256000, 256, 1048536, 8192, 0x2000000, 0x1000000};
	
	char *Configuration::aTag[NTAG]={
		"DdmManager",
		"Transport",
		};
	void *Configuration::aPValue[NTAG]={
		&DdmManagerParams,
		&MessengerParams,
		};
	int Configuration::aSize[NTAG]={
		sizeof(DdmManagerParams),
		sizeof(MessengerParams),
		};


	// Public API

	void* Configuration::Get(char *pTag) {
		// Dummy implementation: look in static table for stuff to return.
		for (int i=0; i < NTAG; i++)
			if (strcmp(pTag, aTag[i]) == 0)
				return (void*)aPValue[i];

		return NULL;
		}

	void Configuration::Initialize() {}
