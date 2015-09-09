/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: UpgradeEvents.h
// 
// Description:
// Defines the Upgrade interface for Commands/Status.
//
// $Log: /Gemini/Include/UpgradeMaster/UpgradeEvents.h $
// 
// 1     11/17/99 3:23p Joehler
// Add command queue to Upgrade Master
//
/*************************************************************************/

#ifndef __UpgradeEvents_h
#define __UpgradeEvents_h

typedef enum 
{
	EVT_IMAGE_ASSOCIATED = 1,
	EVT_IMAGE_UNASSOCIATED,
	EVT_DEFAULT_IMAGE_ASSIGNED,
	EVT_FILESYS_INFO_GOTTEN,
	EVT_MADE_PRIMARY,
	EVT_IMAGE_BOOTED,
	EVT_IMAGE_ADDED,
	EVT_IMAGE_DELETED
} UPMSTR_EVENT;

class AddImageEvent;
class AssociateImageEvent;
class UnassociateImageEvent;
class DeleteImageEvent;
class AssignDefaultEvent;
class GetFileSysInfoEvent;
class MakePrimaryEvent;
class BootImageEvent;

class CmdEventBase {
public:
	friend AddImageEvent;
	friend AssociateImageEvent;
	friend UnassociateImageEvent;
	friend DeleteImageEvent;
	friend AssignDefaultEvent;
	friend GetFileSysInfoEvent;
	friend MakePrimaryEvent;
	friend BootImageEvent;

private:
	STATUS status;
	rowID rid;
	ImageType type;
	TySlot slot;
	U32 usedSpace;
	U32 totalSpace;
};

class AddImageEvent : public CmdEventBase
{
public:
	AddImageEvent(STATUS status_, RowId imageKey_)
	{
		  status = status_;
		  rid = imageKey_;
	}

	STATUS Status() { return status; }

	RowId GetImageKey() { return rid; }
};

class AssociateImageEvent : public CmdEventBase
{
public:
	AssociateImageEvent(STATUS status_, RowId imageKey_, TySlot slot_)
	{
		  status = status_;
		  rid = imageKey_;
		  slot = slot_;
	}

	STATUS Status() { return status; }

	RowId GetImageKey() { return rid; }
	
	TySlot GetSlot() { return slot; }
};

class UnassociateImageEvent : public CmdEventBase
{
public:
	UnassociateImageEvent(STATUS status_, RowId imageKey_, TySlot slot_)
	{
		  status = status_;
		  rid = imageKey_;
		  slot = slot_;
	}

	STATUS Status() { return status; }

	RowId GetImageKey() { return rid; }
	
	TySlot GetSlot() { return slot; }
};

class AssignDefaultEvent : public CmdEventBase
{
public:
	AssignDefaultEvent(STATUS status_, RowId imageKey_)
	{
		  status = status_;
		  rid = imageKey_;
	}

	STATUS Status() { return status; }

	RowId GetImageKey() { return rid; }
};

class GetFileSysInfoEvent : public CmdEventBase
{
public:
	GetFileSysInfoEvent(STATUS status_, U32 usedSpace_, U32 totalSpace_)
	{
	  status = status_;
	  usedSpace = usedSpace_;
	  totalSpace = totalSpace_;
	}

	STATUS Status() { return status; }

	U32 GetUsedSpace() { return usedSpace; }
	U32 GetAvailableSpace() { return (totalSpace - usedSpace); }
	U32 GetTotalSpace() { return totalSpace; }
};

class MakePrimaryEvent : public CmdEventBase
{
public:
	MakePrimaryEvent(STATUS status_, RowId imageKey_, TySlot slot_)
	{
		  status = status_;
		  rid = imageKey_;
		  slot = slot_;
	}

	STATUS Status() { return status; }

	RowId GetImageKey() { return rid; }

	TySlot GetSlot() { return slot; }
};

class BootImageEvent : public CmdEventBase
{
public:
	BootImageEvent(STATUS status_, RowId imageKey_, TySlot slot_)
	{
		  status = status_;
		  rid = imageKey_;
		  slot = slot_;
	}

	STATUS Status() { return status; }

	RowId GetImageKey() { return rid; }

	TySlot GetSlot() { return slot; }
};

class DeleteImageEvent : public CmdEventBase
{
public:
	DeleteImageEvent(STATUS status_, RowId imageKey_)
	{
		  status = status_;
		  rid = imageKey_;
	}

	STATUS Status() { return status; }

	RowId GetImageKey() { return rid; }
};

#endif