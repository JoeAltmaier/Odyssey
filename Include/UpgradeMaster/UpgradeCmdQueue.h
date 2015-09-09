/* UpgradeCmdQueue.h
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
// $Log: /Gemini/Include/UpgradeMaster/UpgradeCmdQueue.h $
// 
// 2     1/26/00 2:33p Joehler
// allocate memory from big heap
// 
// 1     11/17/99 3:23p Joehler
// Add command queue to Upgrade Master
// 

#ifndef __DdmUpgradeCmdQueue_H
#define __DdmUpgradeCmdQueue_H

#include "CTTypes.h"
#include "Address.h"
#include "assert.h"
#include "UpgradeImageType.h"
#include "CmdSender.h"
#include "CmdServer.h"

/********************
*
* Set up for the Upgrade Command Server
*
********************/

// name of Upgrade Master Command Queue Table
#define UPMSTR_CMD_QUEUE_TABLE "UpgradeQueueTable\0"

typedef enum
{
	CMND_ASSOCIATE_IMAGE = 1,
	CMND_UNASSOCIATE_IMAGE,
	CMND_ASSIGN_DEFAULT,
	CMND_GET_FILESYS_INFO,
	CMND_MAKE_PRIMARY,
	CMND_BOOT_IMAGE,
	CMND_ADD_IMAGE,
	CMND_DELETE_IMAGE
}	UPMSTR_CMND; 

class AddImageCmdInfo;
class AssociateImageCmdInfo;
class UnassociateImageCmdInfo;
class DeleteImageCmdInfo;
class AssignDefaultCmdInfo;
class GetFileSysCmdInfo;
class MakePrimaryCmdInfo;
class BootImageCmdInfo;

class CmdInfoBase {
public:
	friend AddImageCmdInfo;
	friend AssociateImageCmdInfo;
	friend UnassociateImageCmdInfo;
	friend DeleteImageCmdInfo;
	friend AssignDefaultCmdInfo;
	friend GetFileSysCmdInfo;
	friend MakePrimaryCmdInfo;
	friend BootImageCmdInfo;

	CmdInfoBase() 
	{ 
		cib.cbImage = 0;
		pImage = 0;
	}

	CmdInfoBase(void* pData);

	~CmdInfoBase() 
	{ 
		if (pImage)
			delete pImage; 
	}

	U32 WriteAsStruct(void** pData);

	UPMSTR_CMND GetOpcode() { return cib.opcode; }

private:
	struct {
		UPMSTR_CMND opcode;
		rowID rid;
		ImageType type;
		TySlot slot;
		U32 cbImage;
	} cib;
	void* pImage;
};

class AddImageCmdInfo : public CmdInfoBase
{
public:
	AddImageCmdInfo(U32 cbImage, void* pImage_)
	{
		cib.opcode = CMND_ADD_IMAGE;
		cib.cbImage = cbImage;
		pImage = new (tBIG) char[cbImage];
		memcpy(pImage, pImage_, cbImage);
	}

	U32 GetImageSize() { return cib.cbImage; }
	
	U32 GetImage(void** pImage_) 
	{ 
		*pImage_ = new (tBIG) char[cib.cbImage];
		memcpy(*pImage_, pImage, cib.cbImage);
		return cib.cbImage;
	}
};

class AssociateImageCmdInfo : public CmdInfoBase
{
public:
	AssociateImageCmdInfo(RowId imageKey, TySlot slot)
	{
		cib.opcode = CMND_ASSOCIATE_IMAGE;
		cib.rid = imageKey;
		cib.slot = slot;
	}

	RowId GetImageKey() { return cib.rid; }
	
	TySlot GetSlot() { return cib.slot; }
};

class UnassociateImageCmdInfo : public CmdInfoBase
{
public:
	UnassociateImageCmdInfo(RowId imageKey, TySlot slot)
	{
		cib.opcode = CMND_UNASSOCIATE_IMAGE;
		cib.rid = imageKey;
		cib.slot = slot;
	}

	RowId GetImageKey() { return cib.rid; }
	
	TySlot GetSlot() { return cib.slot; }
};

class AssignDefaultCmdInfo : public CmdInfoBase
{
public:
	AssignDefaultCmdInfo(RowId imageKey)
	{
		cib.opcode = CMND_ASSIGN_DEFAULT;
		cib.rid = imageKey;
	}

	RowId GetImageKey() { return cib.rid; }
};

class GetFileSysCmdInfo : public CmdInfoBase
{
public:
	GetFileSysCmdInfo()
	{
		cib.opcode = CMND_GET_FILESYS_INFO;
	}
};

class MakePrimaryCmdInfo : public CmdInfoBase
{
public:
	MakePrimaryCmdInfo(TySlot slot, RowId imageKey)
	{
		cib.opcode = CMND_MAKE_PRIMARY;
		cib.rid = imageKey;
		cib.slot = slot;
	}

	RowId GetImageKey() { return cib.rid; }

	TySlot GetSlot() { return cib.slot; }
};

class BootImageCmdInfo : public CmdInfoBase
{
public:
	BootImageCmdInfo(TySlot slot, RowId imageKey)
	{
		cib.opcode = CMND_BOOT_IMAGE;
		cib.rid = imageKey;
		cib.slot = slot;
	}

	RowId GetImageKey() { return cib.rid; }

	TySlot GetSlot() { return cib.slot; }
};

class DeleteImageCmdInfo : public CmdInfoBase
{
public:
	DeleteImageCmdInfo(RowId imageKey)
	{
		cib.opcode = CMND_DELETE_IMAGE;
		cib.rid = imageKey;
	}

	RowId GetImageKey() { return cib.rid; }
};

#endif
