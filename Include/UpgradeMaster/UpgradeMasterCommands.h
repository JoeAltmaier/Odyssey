/* UpgradeMasterCommands.h
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
// $Log: /Gemini/Include/UpgradeMaster/UpgradeMasterCommands.h $
// 
// 1     11/17/99 3:23p Joehler
// Add command queue to Upgrade Master
// 

#ifndef __DdmUpgradeMasterCommands_H
#define __DdmUpgradeMasterCommands_H

#include "UpgradeCmdQueue.h"

class Cmd
{
public:
	~Cmd() { delete pCmdInfo; }

	void Send(CmdSender* pCmdSender,
		pCmdCompletionCallback_t cb,
		void* pContext = NULL)
	{
#ifdef VAR_CMD_QUEUE
		pCmdSender->csndrExecute(pCmdInfo, cbCmdInfo, cb, pContext); 
#else
		pCmdSender->csndrExecute(pCmdInfo, cb, pContext); 
#endif
	}

	void* pCmdInfo;
	U32 cbCmdInfo;
};


class CmdAddImage : public Cmd
{
public:
	CmdAddImage(U32 cbImage, void* pImage)
	{
		AddImageCmdInfo* pAddCmdInfo;
		pAddCmdInfo = new AddImageCmdInfo(cbImage, pImage);
		cbCmdInfo = pAddCmdInfo->WriteAsStruct(&pCmdInfo);
		delete pAddCmdInfo;
	}
};

class CmdAssociateImage : public Cmd
{
public:
	CmdAssociateImage(RowId imageKey, 
		TySlot slot)
	{
		AssociateImageCmdInfo* pAssocCmdInfo;
		pAssocCmdInfo = new AssociateImageCmdInfo(imageKey, slot);
		cbCmdInfo = pAssocCmdInfo->WriteAsStruct(&pCmdInfo);
		delete pAssocCmdInfo;
	}
};

class CmdUnassociateImage : public Cmd
{
public:
	CmdUnassociateImage(RowId imageKey, 
		TySlot slot)
	{
		UnassociateImageCmdInfo* pUnassocCmdInfo;
		pUnassocCmdInfo = new UnassociateImageCmdInfo(imageKey, slot);
		cbCmdInfo = pUnassocCmdInfo->WriteAsStruct(&pCmdInfo);
		delete pUnassocCmdInfo;
	}
};

class CmdAssignDefaultImage : public Cmd
{
public:
	CmdAssignDefaultImage(RowId imageKey)
	{
		AssignDefaultCmdInfo* pAssignDefaultCmdInfo;
		pAssignDefaultCmdInfo = new AssignDefaultCmdInfo(imageKey);
		cbCmdInfo = pAssignDefaultCmdInfo->WriteAsStruct(&pCmdInfo);
		delete pAssignDefaultCmdInfo;
	}
};

class CmdMakePrimary : public Cmd 
{
public:
	CmdMakePrimary(TySlot slot, RowId imageKey)
	{
		MakePrimaryCmdInfo* pMakePrimaryCmdInfo;
		pMakePrimaryCmdInfo = new MakePrimaryCmdInfo(slot, imageKey);
		cbCmdInfo = pMakePrimaryCmdInfo->WriteAsStruct(&pCmdInfo);
		delete pMakePrimaryCmdInfo;
	}
};

class CmdBootImage : public Cmd
{
public:
	CmdBootImage(TySlot slot, RowId imageKey)
	{
		BootImageCmdInfo* pBootImageCmdInfo;
		pBootImageCmdInfo = new BootImageCmdInfo(slot, imageKey);
		cbCmdInfo = pBootImageCmdInfo->WriteAsStruct(&pCmdInfo);
		delete pBootImageCmdInfo;
	}
};

class CmdGetFileSysInfo : public Cmd
{
public:
	CmdGetFileSysInfo()
	{
		GetFileSysCmdInfo* pGetFileSysCmdInfo;
		pGetFileSysCmdInfo = new GetFileSysCmdInfo();
		cbCmdInfo = pGetFileSysCmdInfo->WriteAsStruct(&pCmdInfo);
		delete pGetFileSysCmdInfo;
	}
};

class CmdDeleteImage : public Cmd 
{
public:
	CmdDeleteImage(RowId imageKey)
	{
		DeleteImageCmdInfo* pDeleteCmdInfo;
		pDeleteCmdInfo = new DeleteImageCmdInfo(imageKey);
		cbCmdInfo = pDeleteCmdInfo->WriteAsStruct(&pCmdInfo);
		delete pDeleteCmdInfo;
	}
};

#endif
