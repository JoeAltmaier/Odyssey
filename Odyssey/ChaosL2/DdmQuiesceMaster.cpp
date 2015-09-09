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
*	This file contains the C++ source for the QuiesceMaster DDM
* 
* Update Log: 
* 06/27/99 Bob Butler: Create file
* 10/03/99 Bob Butler: Added VCMapper DdmServices object to create the VC map
* 10/12/99 Bob Butler: Added QuiesceManager to handled routed quiesces to slot local DDMs
* 11/10/99 Bob Butler: Removed command queues
*
*************************************************************************/

#include "DdmQuiesceMaster.h"
#include "QuiesceMasterMsgs.h"
#include "BuildSys.h"
#include "Odyssey_Trace.h"

#include <assert.h>


// Class Link Name used by Buildsys.cpp.  Must match CLASSENTRY in buildsys.cpp
CLASSNAME (DdmQuiesceMaster, SINGLE);  


//  Declare the messages served by this DDM

SERVELOCAL(DdmQuiesceMaster, REQ_QUIESCE_BUS);
SERVELOCAL(DdmQuiesceMaster, REQ_QUIESCE_IOP);
SERVELOCAL(DdmQuiesceMaster, REQ_QUIESCE_VCIRCUIT);




DdmQuiesceMaster::DdmQuiesceMaster (DID did_) : Ddm(did_)
{
}



STATUS DdmQuiesceMaster::Initialize(Message *pMsg_)
{
	Tracef("DdmQuiesceMaster: Initialized.\n");

	DispatchRequest(REQ_QUIESCE_BUS, REQUESTCALLBACK(DdmQuiesceMaster, QuiesceBus));
	DispatchRequest(REQ_QUIESCE_IOP, REQUESTCALLBACK(DdmQuiesceMaster, QuiesceIop));
	DispatchRequest(REQ_QUIESCE_VCIRCUIT, REQUESTCALLBACK(DdmQuiesceMaster, QuiesceVCircuit));

	Reply(pMsg_, OK);
	return OK;
}



STATUS DdmQuiesceMaster::Quiesce(Message *pMsg_)
{
	// this won't be needed until we failover the HBC
	Reply(pMsg_, OK);
	return OK;
}

STATUS DdmQuiesceMaster::Enable(Message *pMsg_)
{
	pVCMapper = new VCMapper(this);
	Reply(pMsg_, OK);
	return OK;
}




/*/ Receive a command from the command server
void DdmQuiesceMaster::cbCmdServerCmd(HANDLE handle_, void *pData_)
{

	// Figure out what we're doing, then break any complex commands into multiple, less
	// complex commands.
	
	// Define an array of hashtables for using in building out this command.  
	// Each VDN identified will be added to the ht, and if other paths contain the VDN
	// additional parents will be added.
	VdnHash_T<QuiesceCmd *> aHtVdn[NSLOT];

	QMCmd_Info QmCmdInfo(*(QMCmd_Info *)pData_);  // copy the original data.  
	
	
	QuiesceCmd *qCmd = new(tZERO) QuiesceCmd;
	qCmd->handle = handle_;
	qCmd->cmdInfo = QmCmdInfo;
	
	switch(qCmd->cmdInfo.opcode)
	{
		case QMCmd_QuiesceVirtualCircuit:
			AddChildren(qCmd, aHtVdn, SLOTNULL);
		break;
		case QMCmd_QuiesceIOP:
			
			AddSlot(qCmd, aHtVdn);
		break;
		
		default:
			Tracef("QuiesceMaster: Unrecognized command.");
			assert(false);
	}
	
	ProcessCmd(qCmd);
}
*/

// Recursively build the command structure to quiesce the Virtual Circuit.
void DdmQuiesceMaster::AddChildren(QuiesceCmd *qCmd_, VdnHash_T<QuiesceCmd *> *aHtVdn_, TySlot slotP_ /*, TySlot slotS_*/) 
{
	// retrieve the list of VDNs that the current VDN talks to
	const VCMapper::VdnData &vd = pVCMapper->GetVdnData(qCmd_->cmdInfo.vdn);
	
	Tracef("    ** JLO  AddChildren for vdn %d JLO **\n", qCmd_->cmdInfo.vdn);

	for (int i = 0; i < vd.vdnList.NextIndex(); ++i)
	if (vd.vdnList[i] > 0)
	{
		TySlot slotP = slotP_;
		// If we ever allow non-vertically aligned Failover partners the following code will need fixed.
		TySlot slotS = Address::GetFopForIop(slotP);
		
		// if IOP Local, use the slot of its predecessor
		if (vd.slotPrimary != IOP_LOCAL)
			slotP = DeviceId::ISlot(vd.didPrimary);
			
		// determine if we have already mapped this child VDN
		QuiesceCmd *qc;
		QuiesceCmd **ppqc = aHtVdn_[slotP].Get(vd.vdnList[i]);  
		if (ppqc) // is there a command representing this child VDN on this slot in the map already?
		{
			Tracef("      ** previously mapped vdn %d\n", vd.vdnList[i]);
			
			// if so, establish new links to the parent VDN represented by qCmd_
			qc = *ppqc;
			
			Tracef("        ** Make %d a parent of %d\n", qCmd_->cmdInfo.vdn, qc->cmdInfo.vdn);
				
			qc->atpParent.Append(qCmd_);  // add to the parent array
			qc->cParentPending++;	

			Tracef("        ** Make %d a child of %d\n", qc->cmdInfo.vdn, qCmd_->cmdInfo.vdn);
			
			qCmd_->atpChildren.Append(qc);  // add this child to the child array of the parent VDN
			qCmd_->cChildPending++;
		}
		else  // the child is not currently mapped, add a new command
		{
		
			Tracef("      ** Vdn %d not currently mapped\n", vd.vdnList[i]);
			qc = new(tZERO) QuiesceCmd;

			qc->cmdInfo.vdn = vd.vdnList[i];
			qc->cmdInfo.slot = slotP;
			qc->atpParent.Append(qCmd_);  // connect it to the parent
			qc->cParentPending++;

			Tracef("        ** Make %d a parent of %d\n", qCmd_->cmdInfo.vdn, qc->cmdInfo.vdn);
			
			qCmd_->atpChildren.Append(qc);  // and connect the parent to the it
			qCmd_->cChildPending++;

			Tracef("        ** Make %d a child of %d\n",
				qc->cmdInfo.vdn, qCmd_->cmdInfo.vdn);
			
			aHtVdn_[slotP].Set(qc->cmdInfo.vdn, qc);
			
			if (vd.slotPrimary == IOP_LOCAL)
			{
				Tracef("          ** IOP LOCAL, so quiesce both paths\n");
				
				// if IOP Local, we need to quiesce both paths since we don't have the real slot
				qc->cmdInfo.cmdcode = QMCmd_QuiesceVdnIopLocal;
				AddChildren(qc, aHtVdn_, slotP);
				
			}
			else
			{
				Tracef("          ** not IOP LOCAL, so quiesce VDN\n");
			
				qc->cmdInfo.cmdcode = QMCmd_QuiesceVdn;
				AddChildren(qc, aHtVdn_, slotP);
			}	
			
			Tracef("          ** Fall thru to add children for vdn %d\n",qc->cmdInfo.vdn);
		}																			
	}	
}

void DdmQuiesceMaster::AddSlot(QuiesceCmd *pqCmd_, VdnHash_T<QuiesceCmd *> *aHtVdn_) 
{
	Array_T<VDN> atVdn = pVCMapper->GetVdnList(pqCmd_->cmdInfo.slot);
	Array_T<QuiesceCmd *> atpQc(atVdn.NextIndex());
	
	Tracef("  ** JLO  AddSlot - slot has %d virtual devices\n", atVdn.NextIndex());
	
	int i;
	for (i=0; i < atVdn.NextIndex(); ++i)  // create a sub-command for each VDN on the slot
	{
		Tracef("    ** Adding command and children for vdn %d\n", atVdn[i]);
		
		atpQc[i] = new(tZERO) QuiesceCmd;
		atpQc[i]->cmdInfo.cmdcode = QMCmd_QuiesceVdn;		
		atpQc[i]->cmdInfo.slot = pqCmd_->cmdInfo.slot;
		atpQc[i]->cmdInfo.vdn = atVdn[i];
		
		aHtVdn_[pqCmd_->cmdInfo.slot].Set(atVdn[i], atpQc[i]);
 
		AddChildren(atpQc[i], aHtVdn_, pqCmd_->cmdInfo.slot);
	}
	
	for (i=0; i < atVdn.NextIndex(); ++i)
	{
		if (atpQc[i]->atpParent.NextIndex() == 0)
		{
			pqCmd_->atpChildren.Append(atpQc[i]);
			atpQc[i]->atpParent.Append(pqCmd_);
		}
	}	
}

STATUS DdmQuiesceMaster::QuiesceBus(Message *pMsg_)
{
	assert(FALSE);
	Reply(pMsg_, OK);
	return OK;
}

STATUS DdmQuiesceMaster::QuiesceIop(Message *pMsg_)
{
	// Define an array of hashtables for using in building out this command.  
	// Each VDN identified will be added to the ht, and if other paths contain the VDN
	// additional parents will be added.
	VdnHash_T<QuiesceCmd *> aHtVdn[NSLOT];

	RqQuiesceIop *pMsg = (RqQuiesceIop *)pMsg_;

	QuiesceCmd *qCmd = new(tZERO) QuiesceCmd;
	qCmd->pMsg = pMsg_;
	qCmd->cmdInfo.cmdcode = QMCmd_QuiesceIOP;
	qCmd->cmdInfo.slot = (TySlot)pMsg->GetSlot();

	AddSlot(qCmd, aHtVdn);
	if (qCmd->atpChildren.NextIndex()==0)
		PostProcessCmd(qCmd);
	else
		ProcessCmd(qCmd);
	return OK;
}

STATUS DdmQuiesceMaster::QuiesceVCircuit(Message *pMsg_)
{

	// Define an array of hashtables for using in building out this command.  
	// Each VDN identified will be added to the ht, and if other paths contain the VDN
	// additional parents will be added.
	VdnHash_T<QuiesceCmd *> aHtVdn[NSLOT];

	RqQuiesceVirtualCircuit *pMsg = (RqQuiesceVirtualCircuit *)pMsg_;
	
	
	QuiesceCmd *qCmd = new(tZERO) QuiesceCmd;
	qCmd->pMsg = pMsg_;
	qCmd->cmdInfo.cmdcode = QMCmd_QuiesceVirtualCircuit;
	qCmd->cmdInfo.vdn = pMsg->GetVdn();
	
	AddChildren(qCmd, aHtVdn, SLOTNULL);
	if (qCmd->atpChildren.NextIndex()==0)
		PostProcessCmd(qCmd);
	else
		ProcessCmd(qCmd);
	return OK;
}


void DdmQuiesceMaster::ProcessCmd(QuiesceCmd *pQCmd_)
{
	if (!pQCmd_->cParentPending)
	{
		const VCMapper::VdnData &vd = pVCMapper->GetVdnData(pQCmd_->cmdInfo.vdn);
		Message* pMsg;
		if (pQCmd_->cmdInfo.cmdcode == QMCmd_QuiesceVdnIopLocal)
		{
			if (vd.slotPrimary != NOSLOT) {
				pMsg = new RqRoutedQuiesceIopLocal(pQCmd_->cmdInfo.vdn);
				pQCmd_->cRepliesPending++;
				Send( pQCmd_->cmdInfo.slot,
					  pMsg, pQCmd_, REPLYCALLBACK(DdmQuiesceMaster, ProcessQuiesceReply));
				}
			
			if (vd.slotSecondary != NOSLOT) {
				pMsg = new RqRoutedQuiesceIopLocal(pQCmd_->cmdInfo.vdn);
				pQCmd_->cRepliesPending++;
				Send( Address::GetFopForIop(pQCmd_->cmdInfo.slot), 
					  pMsg, pQCmd_, REPLYCALLBACK(DdmQuiesceMaster, ProcessQuiesceReply));
				}
		}
		else if (pQCmd_->cmdInfo.cmdcode == QMCmd_QuiesceVdn)
		{
			if (vd.slotPrimary != NOSLOT) {
				pQCmd_->cRepliesPending++;
				pMsg = new Message(REQ_OS_DDM_QUIESCE);
				Send(vd.didPrimary, pMsg, pQCmd_, REPLYCALLBACK(DdmQuiesceMaster, ProcessQuiesceReply));
				}

			if (vd.slotSecondary != NOSLOT) {
				pQCmd_->cRepliesPending++;
				pMsg = new Message(REQ_OS_DDM_QUIESCE);
				Send(vd.didSecondary, pMsg, pQCmd_, REPLYCALLBACK(DdmQuiesceMaster, ProcessQuiesceReply));
				}
		}
		else // this is a complex command -- quiesce bus, IOP or VC, so move down a level in the tree
			for (int iChild=0; iChild < pQCmd_->atpChildren.NextIndex(); ++iChild)
				ProcessCmd(pQCmd_->atpChildren[iChild]);
	}
}
// Process the reply to the Quiesce or routed QuiesceIopLocal message
STATUS DdmQuiesceMaster::ProcessQuiesceReply(Message *pMsg_)
{
	QuiesceCmd *pQCmd = (QuiesceCmd *)pMsg_->GetContext();
	if (!pQCmd->atpChildren.NextIndex())  // this must be the last VDN in the VC, so start postprocessing walkback
		PostProcessCmd(pQCmd);
	else // process each child node 
	{
		for (int iChild=0; iChild < pQCmd->atpChildren.NextIndex(); ++iChild)
			if (--pQCmd->atpChildren[iChild]->cParentPending == 0)
				ProcessCmd(pQCmd->atpChildren[iChild]);
	}
	delete pMsg_;
	return OK;
}



void DdmQuiesceMaster::PostProcessCmd(QuiesceCmd *pQCmd_)
{
	if (pQCmd_->pMsg)
	{
		Reply(pQCmd_->pMsg, OK);
		pQCmd_->pMsg=NULL;
	}	

	// Decrement the pending count for each parent, process them if they are ready,
	// then delete the command.  The parent nodes still contain pointers to the 
	// current command, but they will never use them again, so it is safe to delete
	// this command.
	for (int iParent=0; iParent < pQCmd_->atpParent.NextIndex(); ++iParent)
	{
		if ((pQCmd_->atpParent[iParent]->cmdInfo.cmdcode == QMCmd_QuiesceIOP) ||
			 (pQCmd_->atpParent[iParent]->cmdInfo.cmdcode == QMCmd_QuiesceBus) ||
			 (pQCmd_->atpParent[iParent]->cmdInfo.cmdcode == QMCmd_QuiesceVirtualCircuit) ||
			 (--pQCmd_->atpParent[iParent]->cChildPending == 0))
			PostProcessCmd(pQCmd_->atpParent[iParent]);
	}
	delete pQCmd_;
}

