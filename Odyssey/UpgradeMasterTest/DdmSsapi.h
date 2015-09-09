/* DdmSSAPI.h 
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
// $Log: /Gemini/Odyssey/UpgradeMasterTest/DdmSsapi.h $
// 
// 4     12/09/99 4:09p Joehler
// Added modify default image message testing
// 
// 3     11/17/99 3:28p Joehler
// add command queues to upgrade master
// 
// 2     10/06/99 4:23p Joehler
// added error checking
// 
// 1     9/30/99 7:50a Joehler
// First cut of Upgrade Master test driver
//


#ifndef __DdmSsapi_H
#define __DdmSsapi_H

#include "Ddm.h"
#include "CTtypes.h"
#include "UpgradeMasterMessages.h"
#include "FileSystemMessages.h"
#include "CmdSender.h"

class DdmSSAPI : public Ddm {
public:
	DdmSSAPI(DID did);
	static Ddm * Ctor(DID did);
	STATUS Enable(Message* pMsg);
	STATUS Initialize(Message* pMsg);

private:
	// Upgrade Master Testing functions
	void MakeImages();
	STATUS 	AddImageMsg(void* image, U32 cbImage);
	STATUS 	AddImageCmd(void* image, U32 cbImage);
	STATUS 	DeleteImage(RowId imageKey);
	STATUS  OpenImage(RowId imageKey);
	STATUS  AssociateImage(RowId imageKey, TySlot slot);
	STATUS	UnassociateImage(RowId imageKey, TySlot slot);
	STATUS 	QueryImages(ImageType type = ALL_IMAGES);
	STATUS  GetSysInfo();
	STATUS  AssignDefaultImage(RowId imageKey);
	STATUS  ModifyDefaultImage(RowId imageKey, U32 cbImage, void* image);
	STATUS	BootImage(RowId imageKey, TySlot slot);
	STATUS	MakePrimary(RowId imageKey, TySlot slot);

	// File System Testing functions
	void MakeFiles();
	STATUS AddFile(void* file, U32 cbFile, UnicodeString16 fileName);
	STATUS DeleteFile(RowId fileKey);
	STATUS ListFiles(FileType type = ALL_FILES);
	STATUS ListFiles(RowId fileKey);
	STATUS OpenFile(RowId fileKey);
	STATUS GetFileSysInfo();

private:

	void UpgradeMasterMessageControl();
	void FileSysMessageControl();

	Message* m_pInitMsg;
	CmdSender* m_pCmdSender;
	void cmdsenderInitializedReply(STATUS status);

	void cmdsenderEventHandler(STATUS eventCode, void* pStatusData);
	void CommandCompletionReply(STATUS, void*, void*, void*);

	// call back functions from Upgrade Master Testing
	STATUS ProcessAddImageReply(Message* pMsg);
	STATUS ProcessModifyDefaultImageReply(Message* pMsg);
	STATUS ProcessOpenImageReply(Message* pMsg);
	STATUS ProcessQueryImagesReply(Message* pMsg);

	// call back functions from File System Testing
	STATUS ProcessAddFileReply(Message* pMsg);
	STATUS ProcessDeleteFileReply(Message* pMsg);
	STATUS ProcessListFilesReply(Message* pMsg);
	STATUS ProcessOpenFileReply(Message* pMsg);
	STATUS ProcessGetFileSysInfoReply(Message* pMsg);

};


#endif	

