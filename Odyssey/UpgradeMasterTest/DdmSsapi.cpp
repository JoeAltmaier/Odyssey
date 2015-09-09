/* DdmSSAPI.cpp 
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
// $Log: /Gemini/Odyssey/UpgradeMasterTest/DdmSsapi.cpp $
// 
// 8     12/09/99 4:08p Joehler
// Added modify default image message testing
// 
// 7     11/17/99 3:28p Joehler
// add command queues to upgrade master
// 
// 6     10/21/99 3:41p Joehler
// 
// 5     10/14/99 5:33p Joehler
// Modifications for testing variable length data fields in PTS table
// 
// 4     10/12/99 10:25a Joehler
// Modifications to test driver for variable PTS entries and new query
// mechanism.
// 
// 3     10/07/99 9:59a Joehler
// modified testing of default images and add image
// 
// 2     10/06/99 4:23p Joehler
// added error checking
// 
// 1     9/30/99 7:50a Joehler
// First cut of Upgrade Master test driver
// 
// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_UPGRADE	// set this modules index to your index	
#include "Odyssey_Trace.h"

//DdmSSAPI.cpp

#include "BuildSys.h"
#include "DdmSSAPI.h"
#include "imghdr.h"
#include "UpgradeMasterMessages.h"
#include "UpgradeMasterCommands.h"
#include "UpgradeEvents.h"

extern void QuitTest();

CLASSNAME(DdmSSAPI,SINGLE);  //Class link name used by buildsys

// upgrade master testing globals
RowId imageOneKey;
void* pImageOne;
//U32 cbImageOne = 1024 * 1000; // 1M 
U32 cbImageOne = (ONEK * .5) - 88;
RowId imageTwoKey;
void* pImageTwo;
//U32 cbImageTwo = 1024 * 1000 * .3; // 300K
//U32 cbImageTwo = ONEK * .5 * .3;
U32 cbImageTwo = 156 - 88;
RowId imageThreeKey;
void* pImageThree;
//U32 cbImageThree = 1024 * 1000 * .2; // 200K
//U32 cbImageThree = ONEK * .5 * .2;
U32 cbImageThree = 104 - 88;
RowId imageFourKey;
void* pImageFour;
//U32 cbImageFour = 1024 * 1000 * .5; // 500K
U32 cbImageFour = (ONEK * .5 * .5) - 88;

DdmSSAPI::DdmSSAPI(DID did) : Ddm(did) { }

Ddm* DdmSSAPI::Ctor(DID did) 
{
	
	TRACE_ENTRY(DdmSSAPI::Ctor(DID));
	return (new DdmSSAPI (did));
}

STATUS DdmSSAPI::Initialize(Message* pMsg) 
{
	TRACE_ENTRY(DdmSSAPI::Initialize(Message*));

	m_pCmdSender = NULL;
	m_pInitMsg = pMsg;

#ifdef VAR_CMD_QUEUE
	m_pCmdSender = new CmdSender(
		UPMSTR_CMD_QUEUE_TABLE,
		this
		);
#else
	m_pCmdSender = new CmdSender(
		UPMSTR_CMD_QUEUE_TABLE,
		sizeof(CmdInfoBase),
		sizeof(CmdEventBase),
		this
		);
#endif

	m_pCmdSender->csndrInitialize((pInitializeCallback_t)
		&DdmSSAPI::cmdsenderInitializedReply);

	return OK;
}

void DdmSSAPI::cmdsenderInitializedReply(STATUS status) 
{
	if (!status) {
		// we are initialized
		m_pCmdSender->csndrRegisterForEvents((pEventCallback_t)
			&DdmSSAPI::cmdsenderEventHandler);

		Reply(m_pInitMsg, OK);
	}
}

STATUS DdmSSAPI::Enable(Message* pMsg) 
{
	TRACE_ENTRY(DdmSSAPI::Enable(Message*));
	Reply(pMsg,OK);
	UpgradeMasterMessageControl();
	return OK;
}
	
void DdmSSAPI::MakeImages() 
{
	img_hdr_t* pImgHdr;
	char* pImage;
	char* imageName;

	// image one
	pImageOne = new(tZERO) char[sizeof(img_hdr_t) + cbImageOne];
	assert(pImageOne);
	pImgHdr = (img_hdr_t*)pImageOne;
	pImgHdr->i_signature = IMG_SIGNATURE;
	pImgHdr->i_headerversion = HEADER_VERSION;
	pImgHdr->i_mjr_ver = 1;
	pImgHdr->i_mnr_ver = 1;
	pImgHdr->i_zipsize = cbImageOne;
	imageName = (char*)&pImgHdr->i_imagename;
	strcpy(imageName, "imageone");
	// JLO TEMP IMAGE TYPE
	pImgHdr->i_type = HBC_IMAGE;
	pImgHdr->i_imageoffset = sizeof(img_hdr_t);
	pImgHdr->i_day = 1;
	pImgHdr->i_month = 1;
	pImgHdr->i_year = 1991;
	pImgHdr->i_sec = 1;
	pImgHdr->i_min = 1;
	pImgHdr->i_hour = 1;
	pImage = (char*)pImageOne + sizeof(img_hdr_t);
	memset(pImage, 1, cbImageOne);

	// image two
	pImageTwo = new(tZERO) char[sizeof(img_hdr_t) + cbImageTwo];
	assert(pImageTwo);
	pImgHdr = (img_hdr_t*)pImageTwo;
	pImgHdr->i_signature = IMG_SIGNATURE;
	pImgHdr->i_headerversion = HEADER_VERSION;
	pImgHdr->i_mjr_ver = 1;
	pImgHdr->i_mnr_ver = 2;
	pImgHdr->i_zipsize = cbImageTwo;
	imageName = (char*)&pImgHdr->i_imagename;
	strcpy(imageName, "imagetwo");
	// JLO TEMP IMAGE TYPE
	pImgHdr->i_type = HBC_IMAGE;
	pImgHdr->i_imageoffset = sizeof(img_hdr_t);
	pImgHdr->i_day = 2;
	pImgHdr->i_month = 2;
	pImgHdr->i_year = 1992;
	pImgHdr->i_sec = 2;
	pImgHdr->i_min = 2;
	pImgHdr->i_hour =21;
	pImage = (char*)pImageTwo + sizeof(img_hdr_t);
	memset(pImage, 1, cbImageTwo);

	// image three
	pImageThree = new(tZERO) char[sizeof(img_hdr_t) + cbImageThree];
	assert(pImageThree);
	pImgHdr = (img_hdr_t*)pImageThree;
	pImgHdr->i_signature = IMG_SIGNATURE;
	pImgHdr->i_headerversion = HEADER_VERSION;
	pImgHdr->i_mjr_ver = 1;
	pImgHdr->i_mnr_ver = 3;
	pImgHdr->i_zipsize = cbImageThree;
	imageName = (char*)&pImgHdr->i_imagename;
	strcpy(imageName, "imagethree");
	// JLO TEMP IMAGE TYPE
	pImgHdr->i_type = HBC_IMAGE;
	pImgHdr->i_imageoffset = sizeof(img_hdr_t);
	pImgHdr->i_day = 3;
	pImgHdr->i_month = 3;
	pImgHdr->i_year = 1993;
	pImgHdr->i_sec = 3;
	pImgHdr->i_min = 3;
	pImgHdr->i_hour = 3;
	pImage = (char*)pImageThree + sizeof(img_hdr_t);
	memset(pImage, 1, cbImageThree);

	// image four
	pImageFour = new(tZERO) char[sizeof(img_hdr_t) + cbImageFour];
	assert(pImageFour);
	pImgHdr = (img_hdr_t*)pImageFour;
	pImgHdr->i_signature = IMG_SIGNATURE;
	pImgHdr->i_headerversion = HEADER_VERSION;
	pImgHdr->i_mjr_ver = 1;
	pImgHdr->i_mnr_ver = 4;
	pImgHdr->i_zipsize = cbImageFour;
	imageName = (char*)&pImgHdr->i_imagename;
	strcpy(imageName, "imagefour");
	// JLO TEMP IMAGE TYPE
	pImgHdr->i_type = RAC_IMAGE;
	pImgHdr->i_imageoffset = sizeof(img_hdr_t);
	pImgHdr->i_day = 4;
	pImgHdr->i_month = 4;
	pImgHdr->i_year = 1994;
	pImgHdr->i_sec = 4;
	pImgHdr->i_min = 4;
	pImgHdr->i_hour = 4;
	pImage = (char*)pImageFour + sizeof(img_hdr_t);
	memset(pImage, 1, cbImageFour);

}

void DdmSSAPI::cmdsenderEventHandler(STATUS eventCode,
									void* pStatusData)
{

	TRACE_ENTRY(DdmSSAPI::cmdsenderEventHandler());
	RowId imageKey;
	TySlot slot;
	U32 availableSpace, usedSpace, totalSpace;
	STATUS status;

	switch (eventCode) 
	{
	case EVT_IMAGE_ADDED:
		{
		AddImageEvent* pEvt = (AddImageEvent*)pStatusData;
		imageKey = pEvt->GetImageKey();
		status = pEvt->Status();
		break;
		}
	case EVT_IMAGE_ASSOCIATED:
		{
		AssociateImageEvent* pEvt = (AssociateImageEvent*)pStatusData;
		imageKey = pEvt->GetImageKey();
		slot = pEvt->GetSlot();
		status = pEvt->Status();
		break;
		}
	case EVT_IMAGE_UNASSOCIATED:
		{
		UnassociateImageEvent* pEvt = (UnassociateImageEvent*)pStatusData;
		imageKey = pEvt->GetImageKey();
		slot = pEvt->GetSlot();
		status = pEvt->Status();
		break;
		}
	case EVT_DEFAULT_IMAGE_ASSIGNED:
		{
		AssignDefaultEvent* pEvt = (AssignDefaultEvent*)pStatusData;
		imageKey = pEvt->GetImageKey();
		status = pEvt->Status();
		break;
		}
	case EVT_FILESYS_INFO_GOTTEN:
		{
		GetFileSysInfoEvent* pEvt = (GetFileSysInfoEvent*)pStatusData;
		availableSpace = pEvt->GetAvailableSpace();
		totalSpace = pEvt->GetTotalSpace();
		usedSpace = pEvt->GetUsedSpace();
		status = pEvt->Status();
		break;
		}
	case EVT_MADE_PRIMARY:
		{
		MakePrimaryEvent* pEvt = (MakePrimaryEvent*)pStatusData;
		imageKey = pEvt->GetImageKey();
		slot = pEvt->GetSlot();
		status = pEvt->Status();
		break;
		}
	case EVT_IMAGE_BOOTED:
		{
		BootImageEvent* pEvt = (BootImageEvent*)pStatusData;
		imageKey = pEvt->GetImageKey();
		slot = pEvt->GetSlot();
		status = pEvt->Status();
		break;
		}
	case EVT_IMAGE_DELETED:
		{
		DeleteImageEvent* pEvt = (DeleteImageEvent*)pStatusData;
		imageKey = pEvt->GetImageKey();
		status = pEvt->Status();
		break;
		}
	default:
		assert(0);
		break;
	}

}

void DdmSSAPI::CommandCompletionReply(
			STATUS				completionCode,
			void				*pStatusData,
			void				*pCmdData,
			void				*pCmdContext)
{
	RowId imageKey;
	TySlot slot;
	STATUS status;
	CmdInfoBase *pCmdInfo = new CmdInfoBase(pCmdData);
	static i = 1;

	switch (pCmdInfo->GetOpcode())
	{
	case CMND_ADD_IMAGE:
		{
			AddImageCmdInfo* pCmdInfo = (AddImageCmdInfo*) pCmdData;
			AddImageEvent* pEvt = (AddImageEvent*)pStatusData;
			if (i==1)
			{
				imageFourKey = pEvt->GetImageKey();
				i++;
			}
			else
				imageTwoKey = pEvt->GetImageKey();
			status = pEvt->Status();
			break;
		}
	case CMND_ASSOCIATE_IMAGE:
		{
			AssociateImageCmdInfo* pCmdInfo = (AssociateImageCmdInfo*) pCmdData;
			imageKey = pCmdInfo->GetImageKey();
			slot = pCmdInfo->GetSlot();
			AssociateImageEvent* pEvt = (AssociateImageEvent*)pStatusData;
			imageKey = pEvt->GetImageKey();
			slot = pEvt->GetSlot();
			status = pEvt->Status();
			break;
		}
	case CMND_UNASSOCIATE_IMAGE:
		{
			UnassociateImageCmdInfo* pCmdInfo = (UnassociateImageCmdInfo*) pCmdData;
			imageKey = pCmdInfo->GetImageKey();
			slot = pCmdInfo->GetSlot();
			UnassociateImageEvent* pEvt = (UnassociateImageEvent*)pStatusData;
			imageKey = pEvt->GetImageKey();
			slot = pEvt->GetSlot();
			status = pEvt->Status();
			break;
		}
	case CMND_DELETE_IMAGE:
		{
			DeleteImageCmdInfo* pCmdInfo = (DeleteImageCmdInfo*) pCmdData;
			imageKey = pCmdInfo->GetImageKey();
			DeleteImageEvent* pEvt = (DeleteImageEvent*)pStatusData;
			imageKey = pEvt->GetImageKey();
			status = pEvt->Status();
			break;
		}
	case CMND_ASSIGN_DEFAULT:
		{
			AssignDefaultCmdInfo* pCmdInfo = (AssignDefaultCmdInfo*) pCmdData;
			imageKey = pCmdInfo->GetImageKey();
			AssignDefaultEvent* pEvt = (AssignDefaultEvent*)pStatusData;
			imageKey = pEvt->GetImageKey();
			status = pEvt->Status();
			break;
		}
	case CMND_GET_FILESYS_INFO:
		{
			GetFileSysCmdInfo* pCmdInfo = (GetFileSysCmdInfo*) pCmdData;
			GetFileSysInfoEvent* pEvt = (GetFileSysInfoEvent*)pStatusData;
			U32 usedSpace = pEvt->GetUsedSpace();
			U32 totalSpace = pEvt->GetTotalSpace();
			U32 availableSpace = pEvt->GetAvailableSpace();
			status = pEvt->Status();
			break;
		}
	case CMND_BOOT_IMAGE:
		{
			BootImageCmdInfo* pCmdInfo = (BootImageCmdInfo*) pCmdData;
			imageKey = pCmdInfo->GetImageKey();
			slot = pCmdInfo->GetSlot();
			BootImageEvent* pEvt = (BootImageEvent*)pStatusData;
			imageKey = pEvt->GetImageKey();
			slot = pEvt->GetSlot();
			status = pEvt->Status();
			break;
		}
	case CMND_MAKE_PRIMARY:
		{
			MakePrimaryCmdInfo* pCmdInfo = (MakePrimaryCmdInfo*) pCmdData;
			imageKey = pCmdInfo->GetImageKey();
			slot = pCmdInfo->GetSlot();
			MakePrimaryEvent* pEvt = (MakePrimaryEvent*)pStatusData;
			imageKey = pEvt->GetImageKey();
			slot = pEvt->GetSlot();
			status = pEvt->Status();
			break;
		}
	default:
		assert(0);
		break;
	}

	UpgradeMasterMessageControl();

}

STATUS 	DdmSSAPI::AddImageMsg(void* image, U32 cbImage)
{
	TRACE_ENTRY(DdmSSAPI::AddImageMsg());
	MsgAddImage* pMsg = new MsgAddImage(cbImage, image);
	assert(pMsg);
	return Send(pMsg, REPLYCALLBACK(DdmSSAPI, ProcessAddImageReply));
}

STATUS 	DdmSSAPI::AddImageCmd(void* image, U32 cbImage)
{
	TRACE_ENTRY(DdmSSAPI::AddImageCmd());
	CmdAddImage* pCmd = new CmdAddImage(cbImage, image);
	assert(pCmd);
	pCmd->Send(m_pCmdSender,
		(pCmdCompletionCallback_t)&DdmSSAPI::CommandCompletionReply);
	delete pCmd;

	return OK;

}

STATUS 	DdmSSAPI::DeleteImage(RowId imageKey)
{
	TRACE_ENTRY(DdmSSAPI::DeleteImage());
/*	MsgDeleteImage* pMsg = new MsgDeleteImage(imageKey);
	assert(pMsg);
	return Send(pMsg, REPLYCALLBACK(DdmSSAPI, ProcessDeleteImageReply));*/

	CmdDeleteImage* pCmd = new CmdDeleteImage(imageKey);
	assert(pCmd);
	pCmd->Send(m_pCmdSender,
		(pCmdCompletionCallback_t)&DdmSSAPI::CommandCompletionReply);
	delete pCmd;

	return OK;
}

STATUS 	DdmSSAPI::AssociateImage(RowId imageKey, TySlot slot)
{
	TRACE_ENTRY(DdmSSAPI::AssociateImage());
/*	MsgAssociateImage* pMsg = new MsgAssociateImage(imageKey, slot);
	assert(pMsg);
	return Send(pMsg, REPLYCALLBACK(DdmSSAPI, ProcessAssociateImageReply));*/
		
	CmdAssociateImage* pCmd = new CmdAssociateImage(imageKey, slot);
	assert(pCmd);
	pCmd->Send(m_pCmdSender,
		(pCmdCompletionCallback_t)&DdmSSAPI::CommandCompletionReply);
	delete pCmd;

	return OK;

}

STATUS 	DdmSSAPI::UnassociateImage(RowId imageKey, TySlot slot)
{
	TRACE_ENTRY(DdmSSAPI::UnassociateImage());
/*	MsgUnassociateImage* pMsg = new MsgUnassociateImage(imageKey, slot);
	assert(pMsg);
	return Send(pMsg, REPLYCALLBACK(DdmSSAPI, ProcessUnassociateImageReply));*/

	CmdUnassociateImage* pCmd = new CmdUnassociateImage(imageKey, slot);
	assert(pCmd);
	pCmd->Send(m_pCmdSender,
		(pCmdCompletionCallback_t)&DdmSSAPI::CommandCompletionReply);
	delete pCmd;

	return OK;
}

STATUS 	DdmSSAPI::QueryImages(ImageType type)
{
	TRACE_ENTRY(DdmSSAPI::QueryImages());
	MsgQueryImages* pMsg = new MsgQueryImages(type);
	assert(pMsg);
	return Send(pMsg, REPLYCALLBACK(DdmSSAPI, ProcessQueryImagesReply));
}

STATUS 	DdmSSAPI::OpenImage(RowId imageKey)
{
	TRACE_ENTRY(DdmSSAPI::OpenImage());
	MsgOpenImage* pMsg = new MsgOpenImage(imageKey);
	assert(pMsg);
	return Send(pMsg, REPLYCALLBACK(DdmSSAPI, ProcessOpenImageReply));
}

STATUS 	DdmSSAPI::GetSysInfo()
{
	TRACE_ENTRY(DdmSSAPI::GetSysInfo());
/*	MsgGetFileSysInfo* pMsg = new MsgGetFileSysInfo();
	assert(pMsg);
	return Send(pMsg, REPLYCALLBACK(DdmSSAPI, ProcessGetSysInfoReply));*/

	CmdGetFileSysInfo* pCmd = new CmdGetFileSysInfo();
	assert(pCmd);
	pCmd->Send(m_pCmdSender,
		(pCmdCompletionCallback_t)&DdmSSAPI::CommandCompletionReply);
	delete pCmd;

	return OK;
}

STATUS 	DdmSSAPI::AssignDefaultImage(RowId imageKey)
{
	TRACE_ENTRY(DdmSSAPI::AssignDefaultImage());
/*	MsgAssignDefaultImage* pMsg = new MsgAssignDefaultImage(imageKey);
	assert(pMsg);
	return Send(pMsg, REPLYCALLBACK(DdmSSAPI, ProcessAssignDefaultImageReply));*/

	CmdAssignDefaultImage* pCmd = new CmdAssignDefaultImage(imageKey);
	assert(pCmd);
	pCmd->Send(m_pCmdSender,
		(pCmdCompletionCallback_t)&DdmSSAPI::CommandCompletionReply);
	delete pCmd;

	return OK;
}

STATUS 	DdmSSAPI::ModifyDefaultImage(RowId imageKey, U32 cbImage,
									 void* image)
{
	TRACE_ENTRY(DdmSSAPI::ModifyDefaultImage());
	MsgModifyDefaultImage* pMsg = new MsgModifyDefaultImage(imageKey, 
		cbImage, image);
	assert(pMsg);
	return Send(pMsg, REPLYCALLBACK(DdmSSAPI, ProcessModifyDefaultImageReply));
}

STATUS	DdmSSAPI::BootImage(RowId imageKey, TySlot slot)
{
	TRACE_ENTRY(DdmSSAPI::BootImage());
	CmdBootImage* pCmd = new CmdBootImage(slot, imageKey);
	assert(pCmd);
	pCmd->Send(m_pCmdSender,
		(pCmdCompletionCallback_t)&DdmSSAPI::CommandCompletionReply);
	delete pCmd;

	return OK;}

STATUS	DdmSSAPI::MakePrimary(RowId imageKey, TySlot slot)
{
	TRACE_ENTRY(DdmSSAPI::MakePrimary());
/*	MsgMakePrimary* pMsg = new MsgMakePrimary(slot, imageKey);
	assert(pMsg);
	return Send(pMsg, REPLYCALLBACK(DdmSSAPI, ProcessMakePrimaryReply));*/

	CmdMakePrimary* pCmd = new CmdMakePrimary(slot, imageKey);
	assert(pCmd);
	pCmd->Send(m_pCmdSender,
		(pCmdCompletionCallback_t)&DdmSSAPI::CommandCompletionReply);
	delete pCmd;

	return OK;
}

void DdmSSAPI::UpgradeMasterMessageControl()
{
	TRACE_ENTRY(DdmSSAPI::UpgradeMasterMessageControl());

	static U16 i = 0;
	static U16 j = 0;

	switch (i)
	{
	case 0:
		MakeImages();
		AddImageMsg(pImageThree, sizeof(img_hdr_t) + cbImageThree);
		break;
	case 1:
		AddImageCmd(pImageFour, sizeof(img_hdr_t) + cbImageFour);
		break;
	case 2:
		AddImageMsg(pImageOne, sizeof(img_hdr_t) + cbImageOne);
		break;
	case 3:
		AddImageCmd(pImageTwo, sizeof(img_hdr_t) + cbImageTwo);
		break;
	case 4:
		AssociateImage(imageOneKey, IOP_HBC0);
		break;
	case 5:
		AssociateImage(imageTwoKey, IOP_HBC0);
		break;
	case 6:
		// error no empty
		AssociateImage(imageThreeKey, IOP_HBC0);
		break;
	case 7:
		UnassociateImage(imageOneKey, IOP_HBC0);
		break;
	case 8:
		AssociateImage(imageThreeKey, IOP_HBC0);
		break;
	case 9:
		MakePrimary(imageThreeKey, IOP_HBC0);
		break;
	case 10:
		// error not associated
		MakePrimary(imageOneKey, IOP_HBC0);
		break;
	case 11:
		// error not associated
		UnassociateImage(imageOneKey, IOP_HBC0);
		break;
	case 12:
		// error primary
		UnassociateImage(imageThreeKey, IOP_HBC0);
		break;
	case 13:
		QueryImages();
		break;
	case 14:
		QueryImages(HBC_IMAGE);
		break;
	case 15:
		QueryImages(RAC_IMAGE);
		break;
	case 16:
		GetSysInfo();
		break;
	case 17:
		AssignDefaultImage(imageOneKey);
		break;
	case 18:
		// error default
		DeleteImage(imageOneKey);
		break;
	case 19:
		AssociateImage(imageOneKey, IOP_HBC1);
		break;
	case 20:
		// error already associated
		AssociateImage(imageOneKey, IOP_HBC1);
		break;
	case 21:
		AssignDefaultImage(imageFourKey);
		break;
	case 22:
		AssignDefaultImage(imageTwoKey);
		break;
	case 23:
		// error busy
		DeleteImage(imageOneKey);
		break;
	case 24:
		QueryImages();
		break;
	case 25:
		QueryImages(HBC_IMAGE);
		break;
	case 26:
		UnassociateImage(imageOneKey, IOP_HBC1);
		break;
	case 27:
		DeleteImage(imageOneKey);
		break;
	case 28:
		// error not found
		DeleteImage(imageOneKey);
		break;
	case 29:
		// error not found
		AssociateImage(imageOneKey, IOP_HBC1);
		break;
	case 30:
		// error not found 
		MakePrimary(imageOneKey, IOP_HBC1);
		break;
	case 31:
		// error not found 
		AssignDefaultImage(imageOneKey);
		break;
	case 32:
		OpenImage(imageOneKey);
		break;
	case 33:
		OpenImage(imageTwoKey);
		break;
	case 34:
		QueryImages();
		break;
	case 35:
		QueryImages(HBC_IMAGE);
		break;
	case 36:
		GetSysInfo();
		break;
	case 37:
		ModifyDefaultImage(imageTwoKey,
			sizeof(img_hdr_t) + cbImageOne, pImageOne);
		break;
	case 38:
		QueryImages();
		break;
	case 39:
		if (j==0)
		{
			//j++;
			printf("loop around\n");
			UnassociateImage(imageTwoKey, IOP_HBC0);
		}
		else
			FileSysMessageControl();
		break;
	case 40:
		AssignDefaultImage(imageThreeKey);
		break;
	case 41:
		DeleteImage(imageTwoKey);
		i = 1;
		break;
	default:
		// need to add boot image testing
		// add unassociate erc_upgrade_image_current testing
		assert(0);
		break;
	}

	i++;

}

STATUS DdmSSAPI::ProcessAddImageReply(Message* pMsg)
{

	STATUS status;

	TRACE_ENTRY(DdmSSAPI::ProcessAddImageReply());

	status = pMsg->Status();

	MsgAddImage* msg = (MsgAddImage*) pMsg;
	
	static i = 1;

/*	switch (i)
	{
	case 1:
		imageThreeKey = msg->GetImageKey();
		break;
	case 2:
		imageFourKey = msg->GetImageKey();
		break;
	case 3:
		imageOneKey = msg->GetImageKey();
		break;
	case 4:
		imageTwoKey = msg->GetImageKey();
		i = 2;
		break;
	default:
		assert(0);
		break;
	}
	i++;*/

	if (i==1)
	{
		imageThreeKey = msg->GetImageKey();
		i++;
	}
	else
		imageOneKey = msg->GetImageKey();

	UpgradeMasterMessageControl();

	return status;
}

STATUS DdmSSAPI::ProcessOpenImageReply(Message* pMsg)
{
	STATUS status;
	void* pImage;
	U32 cbImage;

	TRACE_ENTRY(DdmSSAPI::ProcessOpenImageReply());

	status = pMsg->Status();

	if (status==OK)
	{
		MsgOpenImage* msg = (MsgOpenImage*) pMsg;

		cbImage = msg->GetImage(&pImage);

		delete pImage;
	}

	UpgradeMasterMessageControl();

	return status;
}

STATUS DdmSSAPI::ProcessModifyDefaultImageReply(Message* pMsg)
{
	STATUS status;

	TRACE_ENTRY(DdmSSAPI::ProcessModifyDefaultImageReply());

	status = pMsg->Status();

	UpgradeMasterMessageControl();

	return status;
}

STATUS DdmSSAPI::ProcessQueryImagesReply(Message* pMsg)
{

	STATUS status;

	TRACE_ENTRY(DdmSSAPI::ProcessQueryImagesReply());

	status = pMsg->Status();

	MsgQueryImages* msg = (MsgQueryImages*) pMsg;
	ImageIterator* pII;
	//const 
	Image* pImage;
	//const 
	ImageDesc* pImageDesc;

	msg->GetImages(&pII);
	for (pImage = pII->GetFirst(); pImage; pImage = pII->GetNext() )
	{
		pImageDesc = pImage->GetAssociation(IOP_HBC0);
		delete pImageDesc;
		pImageDesc = pImage->GetAssociation(IOP_HBC1);
		delete pImageDesc;
		delete pImage;
	}

	delete pII;

	UpgradeMasterMessageControl();

	return status;
}

// file system master testing globals
RowId fileOneKey;
void* pFileOne;
//U32 cbFileOne = 1024 * 1000; // 1M
U32 cbFileOne = ONEK * .5; // 1M
RowId fileTwoKey;
void* pFileTwo;
//U32 cbFileTwo = 1024 * 1000 * .5; // 500K
U32 cbFileTwo = ONEK * .5 * .5; 
RowId fileThreeKey;
RowId fileFourKey;
RowId fileSixKey;
RowId fileSevenKey;
RowId fileEightKey;

void DdmSSAPI::MakeFiles()
{	
	// file one
	if (pFileOne==NULL)
	{
		pFileOne = new (tZERO) char[cbFileOne];
		assert(pFileOne);
		memset(pFileOne, 1, cbFileOne);
	}

	// file two
	if (pFileTwo==NULL)
	{
		pFileTwo = new (tZERO) char[cbFileTwo];
		assert(pFileTwo);
		memset(pFileTwo, 1, cbFileTwo);
	}

}

STATUS DdmSSAPI::AddFile(void* file, U32 cbFile, UnicodeString16 fileName)
{
	MsgAddFile* pMsg = new MsgAddFile(cbFile,
		file, GENERIC_TYPE, fileName);
	assert(pMsg);
	return Send(pMsg, REPLYCALLBACK(DdmSSAPI, ProcessAddFileReply));
}

STATUS DdmSSAPI::DeleteFile(RowId fileKey)
{
	MsgDeleteFile* pMsg = new MsgDeleteFile(fileKey);
	assert(pMsg);
	return Send(pMsg, REPLYCALLBACK(DdmSSAPI, ProcessDeleteFileReply));
}

STATUS DdmSSAPI::ListFiles(FileType type)
{
	if (type == ALL_FILES)
	{
		MsgListFiles* pMsg = new MsgListFiles();
		assert(pMsg);
		return Send(pMsg, REPLYCALLBACK(DdmSSAPI, ProcessListFilesReply));
	}
	else
	{
		MsgListFiles* pMsg = new MsgListFiles(type);
		assert(pMsg);
		return Send(pMsg, REPLYCALLBACK(DdmSSAPI, ProcessListFilesReply));
	}
}

STATUS DdmSSAPI::ListFiles(RowId fileKey)
{
	MsgListFiles* pMsg = new MsgListFiles(fileKey);
	assert(pMsg);
	return Send(pMsg, REPLYCALLBACK(DdmSSAPI, ProcessListFilesReply));
}

STATUS DdmSSAPI::OpenFile(RowId fileKey)
{
	MsgOpenFile* pMsg = new MsgOpenFile(fileKey);
	assert(pMsg);
	return Send(pMsg, REPLYCALLBACK(DdmSSAPI, ProcessOpenFileReply));
}
	
STATUS DdmSSAPI::GetFileSysInfo()
{
	MsgGetSysInfo* pMsg = new MsgGetSysInfo();
	assert(pMsg);
	return Send(pMsg, REPLYCALLBACK(DdmSSAPI, ProcessGetFileSysInfoReply));
}

void DdmSSAPI::FileSysMessageControl()
{
	UnicodeString16 us;

	TRACE_ENTRY(DdmSSAPI::FileSysMessageControl());

	static U16 i = 0;
	static U16 j = 0;

	switch (i)
	{
	case 0:
		printf("MakeFiles %d\n", j++);
		MakeFiles();
		UnicodeString(StringClass("fileOne")).CString(us, sizeof(us));
		AddFile(pFileOne, cbFileOne, us);
		break;
	case 1:
		UnicodeString(StringClass("fileTwo")).CString(us, sizeof(us));
		AddFile(pFileTwo, cbFileTwo, us);
		break;
	case 2:
		UnicodeString(StringClass("fileThree")).CString(us, sizeof(us));
		AddFile(pFileOne, cbFileOne, us);
		break;
	case 3:
		UnicodeString(StringClass("fileFour")).CString(us, sizeof(us));
		AddFile(pFileOne, cbFileOne, us);
		break;
	case 4:
		UnicodeString(StringClass("fileFive")).CString(us, sizeof(us));
		// error - out of memory
		AddFile(pFileOne, cbFileOne, us);
		break;
	case 5:
		GetFileSysInfo();
		break;
	case 6:
		ListFiles();
		break;
	case 7:
		ListFiles(GENERIC_TYPE);
		break;
	case 8:
		ListFiles(IMAGE_TYPE);
		break;
	case 9:
		ListFiles(fileOneKey);
		break;
	case 10:
		UnicodeString(StringClass("fileSix")).CString(us, sizeof(us));
		AddFile(pFileTwo, cbFileTwo, us);
		break;
	case 11:
		DeleteFile(fileOneKey);
		break;
	case 12:
		// error - file not found
		DeleteFile(fileOneKey);
		break;
	case 13:
		GetFileSysInfo();
		break;
	case 14:
		ListFiles();
		break;
	case 15:
		ListFiles(GENERIC_TYPE);
		break;
	case 16:
		ListFiles(IMAGE_TYPE);
		break;
	case 17:
		// error - file not found
		ListFiles(fileOneKey);
		break;
	case 18:
		UnicodeString(StringClass("fileSeven")).CString(us, sizeof(us));
		AddFile(pFileTwo, cbFileTwo, us);
		break;
	case 19:
		UnicodeString(StringClass("fileEight")).CString(us, sizeof(us));
		AddFile(pFileTwo, cbFileTwo, us);
		break;
	case 20:
		// error - out of memory
		UnicodeString(StringClass("fileNine")).CString(us, sizeof(us));
		AddFile(pFileTwo, cbFileTwo, us);
		break;
	case 21:
		GetFileSysInfo();
		break;
	case 22:
		ListFiles();
		break;
	case 23:
		ListFiles(GENERIC_TYPE);
		break;
	case 24:
		ListFiles(IMAGE_TYPE);
		break;
	case 25:
		ListFiles(fileTwoKey);
		break;
	case 26:
		// error - file not found
		OpenFile(fileOneKey);
		break;
	case 27:
		OpenFile(fileTwoKey);
		break;
	case 28:
		DeleteFile(fileTwoKey);
		break;
	case 29:
		DeleteFile(fileThreeKey);
		break;
	case 30:
		DeleteFile(fileFourKey);
		break;
	case 31:
		DeleteFile(fileSixKey);
		break;
	case 32:
		DeleteFile(fileSevenKey);
		break;
	case 33:
		DeleteFile(fileEightKey);
		i = -1;
		break;
	default:
		assert(0);
		break;
	}

	i++;

}

STATUS DdmSSAPI::ProcessAddFileReply(Message* pMsg)
{
	STATUS status;

	TRACE_ENTRY(DdmSSAPI::ProcessAddFileReply());

	status = pMsg->Status();

	MsgAddFile* msg = (MsgAddFile*) pMsg;
	
	static i = 1;
	switch (i)
	{
	case 1:
		fileOneKey = msg->GetFileKey();
		break;
	case 2:
		fileTwoKey = msg->GetFileKey();
		break;
	case 3:
		fileThreeKey = msg->GetFileKey();
		break;
	case 4:
		fileFourKey = msg->GetFileKey();
		break;
	case 5:
		break;
	case 6:
		fileSixKey = msg->GetFileKey();
		break;
	case 7:
		fileSevenKey = msg->GetFileKey();
		break;
	case 8:
		fileEightKey = msg->GetFileKey();
		break;
	case 9:
		i = 1;
		break;
	default:
		assert(0);
		break;
	}
	i++;

	FileSysMessageControl();

	return status;
}

STATUS DdmSSAPI::ProcessDeleteFileReply(Message* pMsg)
{
	STATUS status;

	TRACE_ENTRY(DdmSSAPI::ProcessDeleteFileReply());

	status = pMsg->Status();

	FileSysMessageControl();

	return status;
}

STATUS DdmSSAPI::ProcessListFilesReply(Message* pMsg)
{
	STATUS status;
	SystemInfo* pSysInfo;
	const Entry* pEntry;

	TRACE_ENTRY(DdmSSAPI::ProcessListFilesReply());

	status = pMsg->Status();

	MsgListFiles* msg = (MsgListFiles*) pMsg;

	msg->GetSysInfo(&pSysInfo);
	for (pEntry = pSysInfo->GetFirst(); pEntry; pEntry = pSysInfo->GetNext() )
	{
	}
	
	delete pSysInfo;

	FileSysMessageControl();

	return status;
}

STATUS DdmSSAPI::ProcessOpenFileReply(Message* pMsg)
{
	STATUS status;
	Entry* pEntry;

	TRACE_ENTRY(DdmSSAPI::ProcessOpenFileReply());

	status = pMsg->Status();

	if (status==OK)
	{

		MsgOpenFile* msg = (MsgOpenFile*) pMsg;

		msg->GetFile(&pEntry);

		delete pEntry;

	}

	FileSysMessageControl();

	return status;
}

STATUS DdmSSAPI::ProcessGetFileSysInfoReply(Message* pMsg)
{
	STATUS status;
	SystemInfo* pSysInfo;

	TRACE_ENTRY(DdmSSAPI::ProcessGetFileSysInfoReply());

	status = pMsg->Status();

	MsgGetSysInfo* msg = (MsgGetSysInfo*) pMsg;

	msg->GetSysInfo(&pSysInfo);

	delete pSysInfo;

	FileSysMessageControl();

	return status;
}