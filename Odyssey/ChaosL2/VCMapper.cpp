/*************************************************************************
*
* This material is a confidential trade secret and proprietary 
* information of ConvergeNet Technologies, Inc. which may not be 
* reproduced, used, sold or transferred to any third party without the 
* prior written consent of ConvergeNet Technologies, Inc.  This material 
* is also copyrighted as an unpublished work under sections 104 and 408 
* of Title 17 of the United States Code.  Law prohibits unauthorized 
* use, copying or reproduction.
*
* Description:
*	This file contains the definition of the Virtual Circuit Mapper class
*   This class listens to the Virtual Device Table and builds and maintains
*   maps of all virtual circuits in the system.  This information does not
*	persist.  It is rebuilt on each boot and after failover.
* 
*************************************************************************/

#include "VCMapper.h"
#include "VirtualDeviceTable.h"
#include "RqOsVirtualMaster.h"
#include "RqPts_T.h"

#define _TRACEF
#define	TRACE_INDEX		TRACE_QUIESCEMASTER
#include "Odyssey_Trace.h"

VCMapper::VCMapper(DdmServices *pParentDdm_) : DdmServices(pParentDdm_)
{
	// Listen to the VirtualDeviceTable.
	Message *pMsg = new VirtualDeviceRecord::RqListen;	// Listen for entire table,Insert,Delete
	Send(pMsg, REPLYCALLBACK(VCMapper,ProcessVirtualDeviceTableListenReply));
}

VCMapper::~VCMapper()
{
}

// stop listening to the PTS
void VCMapper::Stop(VcMapperCallback pVcmCallback_)
{
	Message *pMsg = new  RqPtsStopListen(listenerID);
	Send(pMsg, &pVcmCallback_, REPLYCALLBACK(VCMapper,ProcessStopListenReply));
}


STATUS VCMapper::ProcessStopListenReply(Message *pMsg_)
{
	VcMapperCallback pVcmCallback = *((VcMapperCallback *)pMsg_->GetContext());
	(pParentDdmSvs->*pVcmCallback)(OK);  // got our reply, notify our parent.
	delete pMsg_;
	return OK;
}

// Get an array containing all VDNs that the supplied VDN talks to
// (if it is in the config data, it is considered to be talked to)
// returns a const reference to the array_t of VDNs
const VCMapper::VdnData &VCMapper::GetVdnData(const VDN &vdn_) const
{
	const VdnData *vd = htVdn.Get(vdn_);
	if (vd)
		return *vd;
	return vdEmpty;
}

Array_T<VDN> VCMapper::GetVdnList(TySlot slot_) const
{
	Array_T<VDN> atVdn, atResult;

	htVdn.CopyTo(atVdn);
	
	for (int i = 0; i < atVdn.NextIndex(); ++i)
	{
		const VdnData *vd = htVdn.Get(atVdn[i]);
		assert(vd != NULL); // better not be NULL, we got the array from the same place
		if ((vd->didPrimary != DIDNULL && vd->slotPrimary == slot_) 
			 	|| (vd->didSecondary != DIDNULL && vd->slotSecondary == slot_))
		 	atResult.Append(atVdn[i]);
	}
	
	return atResult;
	
}

// Process the listen reply we got from the PTS

STATUS VCMapper::ProcessVirtualDeviceTableListenReply(Message *pMsg_)
{
	TRACE_PROC(VCMapper::ProcessVirtualDeviceTableListenReply);

	if (pMsg_->Status() != OK) 
	{
		Tracef("\n*** Replied with status=%u (VCMapper::ProcessVirtualDeviceTableListenReply)\n",pMsg_->Status());
		delete pMsg_;
		return OK;
	}
	
	VirtualDeviceRecord::RqListen *pListen = (VirtualDeviceRecord::RqListen *) pMsg_;

	VirtualDeviceRecord *pVdt;
	U32 nVdt;
	
	// get the data and count
	pVdt = pListen->GetRowPtr(&nVdt);	
	Tracef("Listen Reply; cRows=%u\n",nVdt);
	
	// if first time, get the listener ID so we can stop later
	if (pListen->IsInitialReply())
		listenerID = pListen->GetListenerId();
	
	// step through the entries we got in the listen reply.  There will only be multiple
	// entries on the initial reply, although both this code and the PTS interface allow
	// for multiple entries on subsequent replies
	for (int i=0; i < nVdt; ++i)
	{
		VDN vdn = (pVdt + i)->rid.GetRow();
		// if not a delete, go find the VDNs that this Virtual Device talks to.
		if (pListen->GetListenTypeRet() != ListenOnDeleteAnyRow)
		{
			VdnData vd;
			vd.didPrimary = (pVdt + i)->didPrimary;
			vd.didSecondary = (pVdt + i)->didSecondary;
			vd.slotPrimary = (pVdt + i)->slotPrimary;
			vd.slotSecondary = (pVdt + i)->slotSecondary;
			htVdn.Set(vdn, vd);
			RqOsVirtualMasterGetConfig *pMsg = new RqOsVirtualMasterGetConfig(vdn, true);
			Send(pMsg, REPLYCALLBACK(VCMapper, ProcessVirtualMasterGetConfigReply));
		}
		else // if a delete, remove from our hashtable
			htVdn.Remove(vdn);
	}
	delete pMsg_;
	return OK;
}

// Process the data for a Virtual Device and build its list of VDNs 
STATUS VCMapper::ProcessVirtualMasterGetConfigReply(Message *pMsg_)
{
	TRACE_PROC(VCMapper::ProcessVirtualDeviceTableListenReply);

	if (pMsg_->Status() != OK) 
	{
		Tracef("\n*** Replied with status=%u (VCMapper::ProcessVirtualMasterGetConfigReply)\n",pMsg_->Status());
		delete pMsg_;
		return OK;
	}
	
	RqOsVirtualMasterGetConfig *pMsg = (RqOsVirtualMasterGetConfig *)pMsg_;
	
	TRACEF(TRACE_L2, ("VCMapper:  Mapping VDN %d:\n", pMsg->payload.vdn));
	VdnData *pVd = htVdn.Get(pMsg->payload.vdn);
	
	VDN vdn;
	U32 cFields;
	fieldDef *pFieldDefs = pMsg->GetFieldDefsPtr(&cFields);
	void *pConfigData = pMsg->GetConfigDataPtr();
	char *p = (char *)pConfigData;
	
	for (int i=0; i < cFields; ++i)
	{
		if ((pFieldDefs + i)->iFieldType == VDN_FT)
		{
			memcpy(&vdn, p, sizeof(VDN));
			TRACEF(TRACE_L2, ("  %d", vdn));
			// TEMPORARY KLUDGE - Bob, Jerry and Jaymie.  This eventually
			// must be removed to allow multiple paths through this
			// virtual device...
			bool found = false;
			for (int j=0; j<pVd->vdnList.NextIndex() && !found; ++j)
				found =  (pVd->vdnList[j] == vdn);
			if (!found)				
				pVd->vdnList.Append(vdn);
			// END TEMPORARY KLUDGE 
		}
		p += (pFieldDefs + i)->cbField;
	}
	TRACEF(TRACE_L2, ("\n"));
	
	delete pMsg;	
	return OK;
}

/*************************************************************************/
// Update Log:
//	$Log: /Gemini/Odyssey/ChaosL2/VCMapper.cpp $
// 
// 3     2/15/00 4:14p Bbutler
// DFCT13085
// 
// 2     1/31/00 4:36p Joehler
// This is a temporary fix made by Jaymie, Jerry and Bob to get Quiesce
// IOP to work. This will need to be modified to allow multiple
// destinations from the virtual device.
// 
// 1     12/09/99 1:45a Iowa
// 1     10/3/1999 		Bbutler
//		Created
/*************************************************************************/
