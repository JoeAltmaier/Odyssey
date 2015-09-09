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
// 8/7/98 Joe Altmaier: Create file
// 3/23/99 Joe Altmaier: Move ctor here.  Move FailSafe.h to end of include
// list (so compiler won't crash)
/*************************************************************************/

#define _TRACEF
#include "Odyssey_Trace.h"

#include "DdmManager.h"
#include "Messenger.h"
#include "Critical.h"
#include "HeapPci.h"
#include "FailSafe.h"
#include "BuildSys.h"

	DEVICENAME(FailSafe, FailSafe::Initialize);

	Array_T<FailSafe::FsNode> FailSafe::aFailSafe_VDN;
	Array_T<FailSafe::FsNode> FailSafe::aFailSafe_ISlot;

	// Ddm entry points


	STATUS FailSafe::Send(VDN _vdn, Message *_pMsg) {
		if (AddActivity(_vdn, _pMsg))
			return Messenger::Send(_pMsg, DdmManager::Did(_vdn));
		return OK;
		}

	STATUS FailSafe::Forward(VDN _vdn, Message *_pMsg) {
		if (AddActivity(_vdn, _pMsg))
			return Messenger::Send(_pMsg, DdmManager::Did(_vdn));
		return OK;
		}

	STATUS FailSafe::Send(DID _did, Message *_pMsg) {
		if (AddActivity(_did, _pMsg))
			return Messenger::Send(_pMsg, _did);
		return OK;
		}

	STATUS FailSafe::Forward(DID _did, Message *_pMsg) {
		if (AddActivity(_did, _pMsg))
			return Messenger::Send(_pMsg, _did);
		return OK;
		}

	void FailSafe::Initialize() {}

	BOOL FailSafe::AddActivity(VDN _vdn, Message *_pMsg) {
		FailSafe *pFs=GetFailSafe(_vdn);
		if (pFs == NULL)
			pFs=new FailSafe(_vdn);

		InitiatorContext *pIc=_pMsg->pInitiatorContext;
		pIc->Link(pFs);
		pIc->vdn=_vdn;
		
		return (pFs->state == Address::IOP_ACTIVE);
		}

	FailSafe* FailSafe::GetFailSafe(VDN _vdn) {
		Critical section;
		if (_vdn >= aFailSafe_VDN.Size())
			return NULL;
			
		FailSafe *_pFs=aFailSafe_VDN[_vdn];
#if 0
		while (_pFs && _pFs->vdn != _vdn)
			_pFs=_pFs->pFailSafeLink;
#endif
		return _pFs;
		}

	void FailSafe::DeleteFailSafe(VDN _vdn) {
		Critical section;
		FailSafe *_pFs=aFailSafe_VDN[_vdn];
#if 0
		FailSafe *_pFsLast=NULL;
		while (_pFs && _pFs->vdn != _vdn) {
			_pFsLast=_pFs;
			_pFs=_pFs->pFailSafeLink;
			}
			
		if (_pFs && _pFsLast)
			_pFsLast->pFailSafeLink=_pFs->pFailSafeLink;
		else
#endif
			aFailSafe_VDN[_vdn]=NULL;
			
		delete _pFs;
		}
		
	void FailSafe::RetryVdn(VDN _vdn) {
		FailSafe *_pFs=FailSafe::GetFailSafe(_vdn);
		if (_pFs && _pFs->state != Address::IOP_ACTIVE) {
			_pFs->state=Address::IOP_ACTIVE;
			
			IcQueue _queue(1024);
			_queue.Insert(_pFs);
			InitiatorContext *_pIc=_queue.Get();
			while (_pIc) {
				STATUS _status=FailSafe::Send(_pIc->vdn, _pIc->pMsg);
				if(_status != OK) {
					_pIc->pMsg->DetailedStatusCode = _status;
					Messenger::Reply(_pIc->pMsg, (_pIc->pMsg->flags & MESSAGE_FLAGS_LAST) != 0);
					}
				_pIc=_queue.Get();
				}
			}
		}

	void FailSafe::FailVdn(VDN _vdn, STATUS _status) {
		IcQueue *_pFs=FailSafe::GetFailSafe(_vdn);
		if (_pFs) {
			InitiatorContext *_pIc=_pFs->Get();
			while (_pIc) {
				_pIc->pMsg->DetailedStatusCode = _status;
				Messenger::Reply(_pIc->pMsg, (_pIc->pMsg->flags & MESSAGE_FLAGS_LAST) != 0);
				_pIc=_pFs->Get();
				}
			DeleteFailSafe(_vdn);
			}
		}

	BOOL FailSafe::AddActivity(DID _did, Message *_pMsg) {
		FailSafe *pFs=GetFailSafe((TySlot)DeviceId::ISlot(_did));
		if (pFs == NULL)
			pFs=new FailSafe(DeviceId::ISlot(_did));

		InitiatorContext *pIc=_pMsg->pInitiatorContext;
		pIc->Link(pFs);
		pIc->did=_did;
		
		return (pFs->state == Address::IOP_ACTIVE);
		}

	FailSafe* FailSafe::GetFailSafe(TySlot _tySlot) {
		Critical section;
		if (_tySlot >= aFailSafe_ISlot.Size())
			return NULL;
			
		return aFailSafe_ISlot[_tySlot];
		}


	void FailSafe::RetryIop(TySlot _tySlot) {
		FailSafe *_pFs=FailSafe::GetFailSafe(_tySlot);
		if (_pFs && _pFs->state != Address::IOP_ACTIVE) {
			_pFs->state=Address::IOP_ACTIVE;

			IcQueue _queue(1024);
			_queue.Insert(_pFs);
			InitiatorContext *_pIc=_queue.Get();
			while (_pIc) {
				STATUS _status=FailSafe::Send(_pIc->did, _pIc->pMsg);
				if(_status != OK) {
					_pIc->pMsg->DetailedStatusCode = _status;
					Messenger::Reply(_pIc->pMsg, (_pIc->pMsg->flags & MESSAGE_FLAGS_LAST) != 0);
					}
				_pIc=_queue.Get();
				}
			}

		for (VDN _vdn=0; _vdn < aFailSafe_VDN.NextIndex(); _vdn++) {
			if (DeviceId::ISlot(DdmManager::Did(_vdn)) == _tySlot)
				RetryVdn(_vdn);
			}
		}

	void FailSafe::FailIop(TySlot _tySlot, STATUS _status) {
		FailSafe *_pFs=FailSafe::GetFailSafe(_tySlot);
		if (_pFs) {
			InitiatorContext *_pIc=_pFs->Get();
			while (_pIc) {
				_pIc->pMsg->DetailedStatusCode = _status;
				Messenger::Reply(_pIc->pMsg, (_pIc->pMsg->flags & MESSAGE_FLAGS_LAST) != 0);
				_pIc=_pFs->Get();
				}
			aFailSafe_ISlot[_tySlot]=NULL;
			delete _pFs;
			}


		for (VDN _vdn=0; _vdn < aFailSafe_VDN.NextIndex(); _vdn++) {
			if (DeviceId::ISlot(DdmManager::Did(_vdn)) == _tySlot)
				FailVdn(_vdn, _status);
			}
		}
