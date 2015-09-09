/* DdmUpgrade.h
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
// $Log: /Gemini/Include/UpgradeMaster/DdmUpgrade.h $
// 
// 8     1/03/00 3:51p Joehler
// Removed define of DEBUG_UPGRADE which created dummy tables
// 
// 7     12/09/99 4:09p Joehler
// Added Modify default image message
// 
// 6     11/17/99 3:23p Joehler
// Add command queue to upgrade master
// 
// 5     10/21/99 3:31p Joehler
// Change for metrowerks
// 
// 4     10/12/99 11:14a Joehler
// Modifications for Query to associate IOPs with Image
// 
// 3     10/07/99 9:56a Joehler
// Changed functionality so that the first image added to the system of a
// certain type becomes the default image
// 
// 2     10/06/99 4:20p Joehler
// added error checking
// 
// 1     9/30/99 7:43a Joehler
// First cut of Upgrade Master
// 
//			

#ifndef __DdmUpgrade_H
#define __DdmUpgrade_H

//#define DEBUG_UPGRADE 1

#include "Ddm.h"
#include "CTtypes.h"

#include "CommandProcessingQueue.h"

#include "DefaultImageTable.h"
#include "ImageDescriptorTable.h"
#include "IOPImageTable.h"
#include "IOPStatusTable.h"

#include "UpgradeMasterCommands.h"

class DefaultImages
{
public:
	DefaultImages(ImageType type_, rowID imageKey_, DefaultImages* next_ = NULL) 
		: type(type_), imageKey(imageKey_), next(next_)
	{ }

	ImageType Type() { return type; }

	rowID ImageKey() { return imageKey; }

	DefaultImages* Next() { return next; }

	rowID* Default(ImageType type_)
	{
		DefaultImages* current;

		for ( current = this; current; current = current->Next())
		{
			if (current->Type() == type_)
				return &current->imageKey;
		}
		return NULL;
	}
private:
	ImageType type;
	rowID imageKey;
	DefaultImages* next;
};

class DdmUpgrade: public Ddm 
{
public:
	DdmUpgrade(DID did) : Ddm(did) {}
	static Ddm *Ctor(DID did);

	STATUS Initialize (Message *pMsg);

	STATUS Enable(Message *pMsg);
	
	STATUS ProcessUpgradeMessages(Message *pMsg);

	// Command Queue listener
	void ListenerForCommands(HANDLE handle, void* pCmdData);

private:

	// enum for state of CONTEXT
	enum {
		// define states
		DEFAULT_IMAGE_TABLE_DEFINED = 1,
		IMAGE_DESC_TABLE_DEFINED,
		CMD_SERVER_INITIALIZED,
		// insert states
		DEFAULT_IMAGE_ROW_INSERTED,
		IMAGE_DESC_ROW_INSERTED,
		// read row states
		IOP_STATUS_ROW_READ,
		DEFAULT_IMAGE_ROW_READ,
		IMAGE_DESC_ROW_READ,
		IOP_IMAGE_ROW_READ,
		// modify row states
		IMAGE_DESC_ROW_MODIFIED,
		DEFAULT_IMAGE_ROW_MODIFIED,
		IOP_IMAGE_ROW_MODIFIED,
		// modify field states
		IOP_IMAGE_FIELD_MODIFIED,
		DEFAULT_IMAGE_FIELD_MODIFIED,
		// delete row states
		IMAGE_DESC_ROW_DELETED,
		// enumerate table states
		IOP_IMAGE_TABLE_ENUMERATED,
		IMAGE_DESC_TABLE_ENUMERATED,
		DEFAULT_IMAGE_TABLE_ENUMERATED,
		// file system states
		FILE_ADDED_TO_SYS,
		FILE_DELETED_FROM_SYS,
		FILE_LISTED_FROM_SYS,
		FILE_OPENED_FROM_SYS,
		FILE_INFO_FROM_SYS,
		// failover state
		BOARD_FAILED_OVER,
		// boot manager state
		IMAGE_BOOTED
	};
	U32 m_State;

	typedef struct _CONTEXT {
		Message* msg;
		HANDLE handle;
		CmdInfoBase* pCmdInfo;
		U16 index;
		U32 numberOfImages;
		U32 numberIOPDescs;
		ImageDescRecord* pImageDescRec;
		IOPImageRecord* pIOPImageRec;
		DefaultImageRecord* pDefaultImageRec;
		IOPStatusRecord* pIOPStatusRec;
		ImageIterator* pII;
		DefaultImages* pDefaultImagesHead;
		void* pData;
		RowId rid;
		TySlot slot;
	} CONTEXT;

	void ClearAndDeleteContext(CONTEXT* pContext)
	{
		if (pContext->pImageDescRec)
			delete pContext->pImageDescRec;
		if (pContext->pIOPImageRec)
			delete pContext->pIOPImageRec;
		if (pContext->pDefaultImageRec)
			delete pContext->pDefaultImageRec;
		if (pContext->pIOPStatusRec)
			delete pContext->pIOPStatusRec;
		if (pContext->pII)
			delete pContext->pII;
		if (pContext->pDefaultImagesHead)
			delete pContext->pDefaultImagesHead;
		delete pContext;
	}

	enum
	{
		function_AddImage = 1,
		function_DeleteImage,
		function_AssociateImage,
		function_UnassociateImage,
		function_QueryImages,
		function_AssignDefaultImage,
		function_GetFileSysInfo,
		function_BootImage,
		function_MakePrimary,
		function_OpenImage, 
		function_ModifyDefaultImage
	};

	// command processing queue functionality
	CommandProcessingQueue* m_pUpgradeQueue;
	BOOL processingCommand;
	BOOL NotProcessingCommand() { return (processingCommand == FALSE); }
	void ProcessCommand() { processingCommand = TRUE; }
	void FinishCommand() { processingCommand = FALSE; }

	// Command server for SSAPI
	CmdServer* m_pCmdServer;

	// initialization variables
	BOOL m_Initialized;
	Message* m_pInitMsg;

private:

#ifdef DEBUG_UPGRADE
	STATUS InsertDummyIOPImageTableRows(Message* pMsg);
#endif

	// general command processing functions
	STATUS AddImage(CONTEXT* pContext);
	STATUS DeleteImage(CONTEXT* pContext);
	STATUS AssociateImage(CONTEXT* pContext);
	STATUS UnassociateImage(CONTEXT* pContext);
	STATUS QueryImages(CONTEXT* pContext);
	STATUS OpenImage(CONTEXT* pContext);
	STATUS AssignDefaultImage(CONTEXT* pContext);
	STATUS GetFileSysInfo(CONTEXT* pContext);
	STATUS BootImage(CONTEXT* pContext);
	STATUS MakePrimary(CONTEXT* pContext);
	STATUS ModifyDefaultImage(CONTEXT* pContext);

	// general table accessing methods
	STATUS IopImageRowRead(CONTEXT* pContext,
		ReplyCallback cb,
		TySlot* slot);
	STATUS ImageDescRowRead(CONTEXT* pContext,
		ReplyCallback cb,
		RowId key);
	STATUS DefaultImageRowRead(CONTEXT* pContext, 
		ReplyCallback cb, 
		ImageType* type) ;

	// initialization methods
#ifdef DEBUG_UPGRADE
	STATUS DefineDefaultImageTable();
#endif
	STATUS DefineImageDescTable();
	STATUS InitializeCmdServer();
	void ObjectInitializeReply(STATUS status);
	STATUS InitReplyHandler(Message* pMsg);

	// add image methods
	STATUS InsertImageDescRow(CONTEXT *pContext, RowId fileKey);
	STATUS InsertDefaultImageRow(CONTEXT *pContext);
	STATUS AddImageReplyHandler(Message* pMsg);

	// delete image methods
	STATUS DeleteImageDescRow(CONTEXT* pContext);
	STATUS DeleteImageFile(CONTEXT *pContext);
	STATUS DeleteImageReplyHandler(Message* pMsg);

	// associate image methods
	STATUS AssociateWithImage(CONTEXT* pContext);
	STATUS IncrementIopCount(CONTEXT* pContext);
	STATUS AssociateImageReplyHandler(Message* pMsg);

	// unassociate image methods
	STATUS UnassociateFromImage(CONTEXT* pContext);
	STATUS DecrementIopCount(CONTEXT* pContext);
	STATUS UnassociateImageReplyHandler(Message* pMsg);

	// query images methods
	STATUS QueryDefaultImagesTable(CONTEXT* pContext);
	STATUS QueryImageDescTable(CONTEXT* pContext);
	STATUS ReadImageFile(CONTEXT* pContext);
	STATUS QueryImagesReplyHandler(Message* pMsg);

	// open image methods
	STATUS OpenImageFile(CONTEXT* pContext);
	STATUS OpenImageReplyHandler(Message* pMsg);

	// assign default image methods
	STATUS ModifyDefault(CONTEXT* pContext);
	STATUS AssignDefaultImageReplyHandler(Message* pMsg);

	// modify default image methods
	STATUS ModifyImageDescRow(CONTEXT* pContext);
	STATUS ModifyDefaultImageReplyHandler(Message* pMsg);

	// get file system info methods
	STATUS GetFileSysInfoReplyHandler(Message* pMsg);

	// boot image methods
	STATUS BootNewImage(CONTEXT* pContext);
	STATUS BootImageReplyHandler(Message* pMsg);

	// make primary methods
	STATUS MakeImagePrimary(CONTEXT *pContext);
	STATUS MakePrimaryReplyHandler(Message* pMsg);

	// command processing queue methods
	void SubmitRequest(U32 functionToCall, CONTEXT* pContext);
	void ExecuteFunction(U32 functionToCall, CONTEXT* pContext);
	void ProcessNextCommand();

};

#endif	// __DdmUpgrade_H
