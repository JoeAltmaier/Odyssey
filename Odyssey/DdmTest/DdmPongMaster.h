/* DdmPongMaster.h -- Test Timer DDM
 *
 * Copyright (C) ConvergeNet Technologies, 1999 
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
**/

// Revision History:
//  3/27/99 Tom Nelson: Created
//


#ifndef __DdmPongMaster_H
#define __DdmPongMaster_H

// VirtualEntry config data
#include "PtsCommon.h"
#include "RqOsVirtualMaster.h"

struct PongRecord {
	static const fieldDef rgFieldDefs[];
	static const U32      cbFieldDefs;

	const RowId		rid;				// rowID of this record.
	const U32		size;				// Size of VirtualDeviceRecord in bytes.
	const U32 		version;			// Version of VirtualDeviceRecord.

	VDN vdnSlave;
	VDN vdnSlave2;
	
	static const fieldDef *FieldDefs()		{ return rgFieldDefs;}
	static const U32 FieldDefsSize() 		{ return cbFieldDefs;  }
	static const U32 FieldDefsTotal()		{ return FieldDefsSize() / FieldDefsSize(); }
	static const char *TableName()			{ return "PongTable"; }
	
	PongRecord(VDN _vdnSlave=0) : rid(0),size(sizeof(PongRecord)),version(0)	{ vdnSlave = _vdnSlave; }
};

#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"

class DdmPongMaster: public Ddm {
public:
	PongRecord config;
	U32 count;
	BOOL cTicks;
	VDN vdnSlave;
	VDN vdnSlave2;
	
	static DdmPongMaster *pDdm;
	
	static void KillSlave() {
		if (pDdm) {
			// Delete Virtual Device
			pDdm->Action(pDdm,ACTIONCALLBACK(DdmPongMaster,DdmPongMaster::ProcessKillSlaveAction));
		}
	}
	static void KillLocalSlave() {
		if (pDdm) {
			// Delete Virtual Device
			pDdm->Action(pDdm,ACTIONCALLBACK(DdmPongMaster,DdmPongMaster::ProcessKillLocalSlaveAction));
		}
	}


	static Ddm *Ctor(DID did) { return new DdmPongMaster(did); }

	DdmPongMaster(DID did);

	ERC Enable(Message *pMsg);
	ERC ProcessLoadVirtualDeviceReply(Message *_pReply);
	ERC ProcessLoadDemoLocalReply(Message *_pReply);

	ERC ProcessKillSlaveAction(void *pContext);
	ERC ProcessDeleteVirtualDeviceReply(Message *_pReply);

	ERC ProcessKillLocalSlaveAction(void *pContext);
	ERC ProcessDeleteLocalDeviceReply(Message *_pReply);
	
	ERC ProcessPongReply(Message *pMsg);
	ERC ProcessPongClockReply(Message *pArgMsg);
};
#endif	// __DdmPongMaster_H

