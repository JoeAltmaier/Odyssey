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
// This class implements message retry upon failover.
// 
// Update Log: 
// 6/12/98 Joe Altmaier: Create file
// 7/29/98 Joe Altmaier: Rename to FailSafe
/*************************************************************************/

#ifndef __FailSafe_h
#define __FailSafe_h

#include "OsTypes.h"
#include "Array_T.h"
#include "Message.h"


class FailSafe: public IcQueue {
	struct FsNode {
		FailSafe *pFs; 
		FsNode() {pFs=NULL;}
		FsNode(FailSafe *_pFs): pFs(_pFs) {}
		operator FailSafe*() { return pFs; }
		};
	
	static
	Array_T<FsNode> aFailSafe_VDN;
	
	static
	Array_T<FsNode> aFailSafe_ISlot;

public:
	VDN vdn;
	TySlot tySlot;
	Address::StateIop state;

	FailSafe(VDN _vdn) : IcQueue(1024), vdn(_vdn), state(Address::IOP_ACTIVE) {
		Critical section;
		aFailSafe_VDN.Set(_vdn, (FsNode)this);
		};
	
	FailSafe(TySlot _tySlot) : IcQueue(1024), tySlot(_tySlot), state(Address::IOP_ACTIVE) {
		Critical section;
		aFailSafe_ISlot.Set(_tySlot, (FsNode)this);
		};
	
	static
	FailSafe *GetFailSafe(VDN _vdn);
	static
	void DeleteFailSafe(VDN _vdn);
	static
	BOOL AddActivity(VDN, Message *);

	static
	FailSafe *GetFailSafe(TySlot);
	static
	void DeleteFailSafe(DID);
	static
	BOOL AddActivity(DID, Message *);

	static
	void RetryVdn(VDN vdn);
	static
	void FailVdn(VDN vdn, STATUS status);
	static
	void RetryIop(TySlot tySlot);
	static
	void FailIop(TySlot tySlot, STATUS status);

	// Ddm entry points
	
	static
	STATUS Send(VDN _vdn, Message *_pMsg);

	static
	STATUS Forward(VDN vd, Message *pMsg);

	static
	STATUS Send(DID _did, Message *_pMsg);

	static
	STATUS Forward(DID _did, Message *pMsg);

	static
	void Initialize();	

};
#endif


