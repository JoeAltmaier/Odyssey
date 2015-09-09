/* RqOsVirtualManager.h -- Request Interface to VirtualManager Ddm.
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99
 * Copyright (C) Dell Computer, 2000
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
//  6/23/99 Tom Nelson: Create file
// ** Log at end-of-file **

// 100 columns ruler
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#ifndef __RqOsVirtualManager_h
#define __RqOsVirtualManager_h

#include "Ddm.h"
#include "RequestCodes.h"
#include "Message.h"
#include "RqOsVirtualMaster.h"


// PingSlot -- Ping the VirtualManager -------------------------------------------DdmVirtualManager-
//
// Send to slot to be pinged.
//
class RqOsVirtualManagerPingSlot : public Message {
public:
	enum { RequestCode = REQ_OS_VIRTUALMANAGER_PING };
	
	struct Payload {
		DID did;	// VirtualManager Did returned

		Payload() 
		: did(DIDNULL) {}
	};
	
	Payload payload;
	
	RqOsVirtualManagerPingSlot() : Message(RequestCode) {
	}
};

// GetConfig -- Get virtual device config data -----------------------------------DdmVirtualManager-
//
// Applications should use RqOsVirtualMasterGetConfig.
//
// This is an OS Core interface
//
class RqOsVirtualManagerGetConfig : public Message {
public:
	enum { RequestCode = REQ_OS_VIRTUALMANAGER_GETCONFIG };

	typedef RqOsVirtualMasterGetConfig::Payload Payload;

	Payload payload;
	
	enum { SGI_CONFIG_RECORD_RETURNED = 0 };
		
	RqOsVirtualManagerGetConfig() : Message(RequestCode) {
		AddSgl(SGI_CONFIG_RECORD_RETURNED, NULL, 0, SGL_DYNAMIC_REPLY);
	}
	RqOsVirtualManagerGetConfig(VDN _vdn) : Message(RequestCode), payload(_vdn) {
		AddSgl(SGI_CONFIG_RECORD_RETURNED, NULL, 0, SGL_DYNAMIC_REPLY);
	}

	// Add reply data
	void AddConfigDataReply(void *pData,U32 cbData) {
		CopyToSgl(SGI_CONFIG_RECORD_RETURNED,0,pData,cbData);
		payload.cbData = cbData;
	}
	
	// Return pointer to config data
	void *GetConfigDataPtr(U32 *pnSize=NULL) {
		return GetSglDataPtr(SGI_CONFIG_RECORD_RETURNED,pnSize);
	}
	// Return size of config data
	U32 GetConfigDataSize() {
		return GetSglDataSize(SGI_CONFIG_RECORD_RETURNED);
	}
};

// ListenIopState - VirtualStateTable summary ------------------------------------DdmVirtualManager-
//
// Applications should use RqOsVirtualMasterListenIopState.
//
// Returns the state of all IOPs on every listen
//
// This is an OS Core interface.
//
class RqOsVirtualManagerListenIopState : public Message {
public:
	enum { RequestCode = REQ_OS_VIRTUALMANAGER_LISTENIOPSTATE };
	
	enum { SGI_STATES_RETURNED = 0 };
	
	typedef RqOsVirtualMasterListenIopState::States States;

	RqOsVirtualManagerListenIopState() : Message(RequestCode) {
		// Returned States SGL
		AddSgl(SGI_STATES_RETURNED, NULL, 0, SGL_DYNAMIC_REPLY);
	}
	
	void AddStatesReply(States *_pStates) {
		CopyToSgl(SGI_STATES_RETURNED,0,_pStates,sizeof(States));
	}
	States *GetStates(States *_pStates) {
		CopyFromSgl(SGI_STATES_RETURNED,0,_pStates,sizeof(States));
		return _pStates;
	}
	
	// Return pointer to States - DEPRICATED: Use GetStates() above!
	States *GetStatesPtr(U32 *pnSize=NULL) {
		return (States*) GetSglDataPtr(SGI_STATES_RETURNED,pnSize);
	}
};

#endif	// __RqOsVirtualManager_h

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Include/Oos/RqOsVirtualManager.h $
// 
// 5     2/08/00 8:46p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// 
// 6     2/08/00 6:54p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
