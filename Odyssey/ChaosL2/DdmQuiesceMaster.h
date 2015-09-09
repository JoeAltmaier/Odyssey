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
*	This file contains the definition of the QuiesceMaster DDM
* 
* Update Log: 
* 06/27/99 Bob Butler: Create file
* 11/10/99 Bob Butler: Remove Command Queues
*
*************************************************************************/

#ifndef DdmQuiesceMaster_h
#define DdmQuiesceMaster_h

#include "ddm.h"

#include "VCMapper.h"
#include "VdnHash_T.h"


class DdmQuiesceMaster : public Ddm
{

public:

   //  Constructor
	DdmQuiesceMaster (DID did_);
   //  Create an instance of this DDM
	static Ddm *Ctor (DID did_) { return new DdmQuiesceMaster(did_); }

private: 

	/********************************************************************
	*
	* Commands used internally by the QuiesceMaster to break a complex operation
	* into smaller pieces.
	*
	********************************************************************/
	
	enum QMCmd { QMCmd_QuiesceBus, QMCmd_QuiesceIOP, QMCmd_QuiesceVirtualCircuit,
			QMCmd_QuiesceVdn, QMCmd_QuiesceVdnIopLocal };
	
	/********************************************************************
	*
	*  Command info structure for the Quiesce Command.  The cmdcode
	*  determines whether quiescing by bus, slot or VDN.  If quiescing
	*  a Virtual Circuit, VDN is the starting point to Quiesce in the VC.
	*
	********************************************************************/
	
	
	struct QMCmd_Info
	{
		QMCmd cmdcode;
		TySlot	slot;  // outside of the union because slot is needed by QMCmd_QuiesceVdnIopLocal
		union {
			U16		bus;
			VDN		vdn;	
		};
	};




	// this is the definition of a node in the Quiesce command tree that
	// is built when complex Quiesce commands are received.
	struct QuiesceCmd
	{
		Message		*pMsg;			// message to reply to on completion
		QMCmd_Info	cmdInfo;		// command data
		U16			cRepliesPending; // number of replies pending for this specific command (not incl. children)
		U16			cParentPending;	// number of predecessors that must complete.  As each completes, this is decremented
		U16			cChildPending;	// number of successors that need to complete before this is considered completed
		
		Array_T<QuiesceCmd *> atpChildren;	// list of immediate successors to process in parallel

		Array_T<QuiesceCmd *> atpParent;	// list of immediate predecessors to be notified when done
	} *qCmd;

protected:

	virtual STATUS Initialize (Message *pMsg_);
	virtual STATUS Quiesce (Message *pMsg_);
	virtual STATUS Enable (Message *pMsg_);

	STATUS ProcessQuiesceReply(Message *pMsg);
	void ProcessCmd(QuiesceCmd *pQCmd_);
	void PostProcessCmd(QuiesceCmd *pQCmd_);



	void AddChildren(QuiesceCmd *qCmd_, VdnHash_T<QuiesceCmd *> *aHtVdn_, TySlot slot_);
	void AddSlot(QuiesceCmd *qCmd_, VdnHash_T<QuiesceCmd *> *aHtVdn_);

	STATUS QuiesceBus (Message *pMsg_);
	STATUS QuiesceIop (Message *pMsg_);
	STATUS QuiesceVCircuit (Message *pMsg_);

private:

	VCMapper		*pVCMapper;
}; 
#endif