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
*	This file contains the definition of the Boot Manager messages.
* 
*************************************************************************/
// Revision History:
// $Log: /Gemini/Include/BootMgrMsgs.h $
// 
// 3     1/26/00 2:23p Joehler
// Add option to Put Iop Into service to supply an imageKey to boot.  Add
// MsgIopLockUnlockInt
// 
// 2     11/22/99 5:33p Jlane
// Add more msgs.
// 
// 1     10/28/99 9:25a Joehler
// Added message interface between Upgrade Master and Boot Manager

#ifndef BootMgrMsgs_h
#define BootMgrMsgs_h

#include "CTTypes.h"
#include "message.h"
#include "RequestCodes.h"

// MsgIopOutOfService - A message to take a specified IOP out of service.
// parameters:
// 	Slot	: The slot number (TySlot) of the IOP to take out of service.
class MsgIopOutOfService : public Message
{
public:
	
	MsgIopOutOfService(TySlot slot_)
	 : Message(REQ_IOP_OUT_OF_SERVICE), slot(slot_) { }
	 
	TySlot GetSlot() { return slot; }
		
private:
	// send data:
	TySlot slot;		
};


// MsgIopOutOfServiceInt - A BootMgr internal message used while taking an
//	IOP out of service.  NOT INTENDED FOR PUBLIC USE.
// parameters:
// 	Slot	: The slot number (TySlot) of the IOP to take out of service.
class MsgIopOutOfServiceInt : public Message
{
public:
	
	MsgIopOutOfServiceInt(TySlot slot_)
	 : Message(REQ_IOP_OUT_OF_SERVICE_INT), slot(slot_) { }
	 
	TySlot GetSlot() { return slot; }
		
private:
	// send data:
	TySlot slot;		
};


// MsgIopIntoService - A message to put a specified IOP into service.
// parameters:
// 	Slot	: The slot number (TySlot) of the IOP to put in service.
class MsgIopIntoService : public Message
{
public:
	
	// if no imageKey is given, the IOP is booted with the primary image
	MsgIopIntoService(TySlot slot_, RowId imageKey_ = 0)
	 : Message(REQ_IOP_INTO_SERVICE), slot(slot_), imageKey(imageKey_) { }
	 
	TySlot GetSlot() { return slot; }
	
	RowId GetImageKey() { return imageKey; }
		
private:
	// send data:
	TySlot slot;		
	RowId imageKey;
};


// MsgIopIntoServiceInt - A BootMgr internal message used while putting an
//	IOP into service.	  NOT INTENDED FOR PUBLIC USE.
// parameters:
// 	Slot	: The slot number (TySlot) of the IOP to put in service.
class MsgIopIntoServiceInt : public Message
{
public:
	
	MsgIopIntoServiceInt(TySlot slot_, RowId imageKey_)
	 : Message(REQ_IOP_INTO_SERVICE), slot(slot_), imageKey(imageKey_) { }
	 
	TySlot GetSlot() { return slot; }
	RowId GetImageKey() { return imageKey; }
			
private:
	// send data:
	TySlot slot;	
	RowId imageKey;	
};

// MsgIopPowerOn - A message to power on a specified IOP.
// parameters:
// 	Slot	: The slot number (TySlot) of the IOP to power on.
class MsgIopPowerOn : public Message
{
public:
	
	MsgIopPowerOn(TySlot slot_)
	 : Message(REQ_IOP_POWER_ON), slot(slot_) { }
	 
	TySlot GetSlot() { return slot; }
		
private:
	// send data:
	TySlot slot;		
};

// MsgIopLockUnlockInt - A BootMgr internal message used while locking 
//	and unlocking an IOP.	  NOT INTENDED FOR PUBLIC USE.
// parameters:
// 	Slot	: The slot number (TySlot) of the IOP to put in service.
//  Lock	: bool
class MsgIopLockUnlockInt : public Message
{
public:
	
	MsgIopLockUnlockInt(TySlot slot_, bool lock_)
	 : Message(REQ_IOP_LOCK_UNLOCK_INT), slot(slot_), lock(lock_) { }
	 
	TySlot GetSlot() { return slot; }
	bool Lock() { return lock; }
			
private:
	// send data:
	TySlot slot;	
	bool lock;
};

#endif