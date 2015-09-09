/* DdmUpgrade.cpp
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
// $Log: /Gemini/Odyssey/UpgradeMaster/DdmUpgrade.cpp $  
// 
// 15    1/26/00 2:45p Joehler
// Added Associate Image and Make Primary message interface.
// 
// 13    12/09/99 4:06p Joehler
// Added modify default image message
// 
// 12    12/09/99 2:14a Iowa
// 
// 11    11/21/99 5:01p Jlane
// Enhance AddDefaultImageRow to support img hdr info.
// 
// 10    11/17/99 3:26p Joehler
// add command queues to upgrade master and make failsafe
// 
// 9     10/27/99 8:47a Joehler
// Removed references to trialImage
// 
// 8     10/15/99 2:04p Joehler
// Modified to use Tom's GetRowCopy() and deleted type as arg for
// AssignDefault request
// 
// 7     10/15/99 11:03a Joehler
// Changes error codes from ERC_UPGRADE_* to CTS_UPGRADE_* and moved to
// *.mc files to localize
// 
// 6     10/14/99 5:32p Joehler
// Added key to IOPDesc to be passed to ImageDesc
// 
// 5     10/13/99 4:53p Joehler
// Fixed problem when Querying and no images are in image repository
// 
// 4     10/12/99 10:23a Joehler
// Added Query mechanism to associate IOPs with image for SSAPI
// 
// 3     10/07/99 9:58a Joehler
// modiified functionality to make first image of each type added the
// default image
// 
// 2     10/06/99 4:22p Joehler
// added open image for boot manager and error checking
// 
// 1     9/30/99 7:49a Joehler
// First cut of upgrade master
// 
// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#include "CtEvent.h"

#include "FileSystemMessages.h"
#include "UpgradeMasterMessages.h"
#include "BootMgrMsgs.h"
#include "UpgradeCmdQueue.h"
#include "UpgradeEvents.h"
#include "DdmUpgrade.h"
#include "BuildSys.h"
#include "imghdr.h"

// BuildSys linkage
	
CLASSNAME(DdmUpgrade,SINGLE);

SERVELOCAL(DdmUpgrade, REQ_UPGRADE_ADD_IMAGE);
SERVELOCAL(DdmUpgrade, REQ_UPGRADE_ASSOCIATE_IMAGE);
SERVELOCAL(DdmUpgrade, REQ_UPGRADE_MAKE_PRIMARY);
SERVELOCAL(DdmUpgrade, REQ_UPGRADE_QUERY_IMAGES);
SERVELOCAL(DdmUpgrade, REQ_UPGRADE_OPEN_IMAGE);
SERVELOCAL(DdmUpgrade, REQ_UPGRADE_MODIFY_DEFAULT_IMAGE);

//  DdmUpgrade::Ctor (did)
//	  (static)
//
//  Description:
//    This routine is called by CHAOS when it wants to create
//    an instance of DdmUpgrade.
//
//  Inputs:
//    did - CHAOS "Device ID" of the new instance we are to create.
//
//  Outputs:
//    DdmUpgrade::Ctor - Returns a pointer to the new instance, or NULL.
//

Ddm *DdmUpgrade::Ctor (DID did)
{
	TRACE_ENTRY(DdmUpgrade::Ctor(DID did));
	return (new DdmUpgrade (did));
} 
/*	end DdmUpgrade::Ctor  *****************************************************/

//  DdmUpgrade::Initialize (pMsg)
//    (virtual)
//
//  Description:
//    Called for a DDM instance when the instance is being created.
//    This routine is called after the DDM's constructor, but before
//    DdmUpgrade::Enable().  Please note that this DDM is not completely
//	  initialized when this procedure returns.  It has spawned several
//    DefineTable messages and a call to Initialize the CmdServer and
//    CmdSender().  When these are complete, m_Initialized will be set
//    to true.
//
//  Inputs:
//    pMsg - Points to message which triggered our DDM's fault-in.  This is
//          always an "initialize" message.
//
//  Outputs:
//    DdmUpgrade::Initialize - Returns OK if all is cool, else an error.
//

STATUS DdmUpgrade::Initialize (Message *pMsg)
{

	TRACE_ENTRY(DdmUpgrade::Initialize(Message*));

	// dispatch requests for all messages which the Upgrade Master will service
	DispatchRequest(REQ_UPGRADE_ADD_IMAGE, 
		REQUESTCALLBACK (DdmUpgrade, ProcessUpgradeMessages));
	DispatchRequest(REQ_UPGRADE_ASSOCIATE_IMAGE, 
		REQUESTCALLBACK (DdmUpgrade, ProcessUpgradeMessages));	
	DispatchRequest(REQ_UPGRADE_MAKE_PRIMARY, 
		REQUESTCALLBACK (DdmUpgrade, ProcessUpgradeMessages));
	DispatchRequest(REQ_UPGRADE_OPEN_IMAGE, 
		REQUESTCALLBACK (DdmUpgrade, ProcessUpgradeMessages));
	DispatchRequest(REQ_UPGRADE_QUERY_IMAGES, 
		REQUESTCALLBACK (DdmUpgrade, ProcessUpgradeMessages));
	DispatchRequest(REQ_UPGRADE_MODIFY_DEFAULT_IMAGE, 
		REQUESTCALLBACK (DdmUpgrade, ProcessUpgradeMessages));

	// not yet initialized
	m_Initialized = false;

	// create command processing queue (allows only one message to be
	// processed at one.
	m_pUpgradeQueue = new CommandProcessingQueue;
	assert(m_pUpgradeQueue);

	m_pInitMsg = pMsg;

	m_pCmdServer = NULL;

#ifdef DEBUG_UPGRADE
	// call method which defines the image table
	DefineDefaultImageTable();
#else
	DefineImageDescTable();
#endif

	return OK;
		
}
/*	end DdmUpgrade::Initialize  *****************************************************/

#ifdef DEBUG_UPGRADE
STATUS DdmUpgrade::InsertDummyIOPImageTableRows(Message* pMsg)
{
	static i = 0;
	static Message* pReplyMsg = pMsg;
	IOPImageRecord* pIOPImageRec;
	STATUS status;

	switch (i) 
	{
	case 0:
		{
			// allocate and initialize table definition request
			IOPImageRecord::RqDefineTable * preqDefTab;
			preqDefTab = new IOPImageRecord::RqDefineTable();
			assert(preqDefTab);

			// send table definition request
			status = Send(preqDefTab, 0,
				REPLYCALLBACK (DdmUpgrade, InsertDummyIOPImageTableRows));

			// make sure everything went ok and return status
			assert (status == CTS_SUCCESS);
			break;
		}
	case 1:
		{
			pIOPImageRec = new IOPImageRecord();
			assert(pIOPImageRec);
			pIOPImageRec->currentImage = 0; // need to insert an image
			pIOPImageRec->primaryImage = 0; // and here...
//			pIOPImageRec->trialImage = 0;
			pIOPImageRec->slot = IOP_HBC0;
			pIOPImageRec->timeBooted = 0;
			pIOPImageRec->imageOne = 0;
			pIOPImageRec->imageOneAccepted = FALSE;
			pIOPImageRec->imageTwo = 0;
			pIOPImageRec->imageTwoAccepted = FALSE;

			IOPImageRecord::RqInsertRow* preqInsert;
			preqInsert = new IOPImageRecord::RqInsertRow(*pIOPImageRec);
			assert(preqInsert);

			// send the insert row request
			Send (preqInsert, 0,
						   REPLYCALLBACK (DdmUpgrade, InsertDummyIOPImageTableRows));		
			break;
		}
	case 2:
		{
			RqPtsInsertRow* pReply = (RqPtsInsertRow*) pMsg;
			assert(pReply->GetRowIdDataCount()==1);
			pIOPImageRec = new IOPImageRecord();
			assert(pIOPImageRec);
			pIOPImageRec->currentImage = 0; // need to insert an image
			pIOPImageRec->primaryImage = 0; // and here...
			//pIOPImageRec->trialImage = 0;
			pIOPImageRec->slot = IOP_HBC1;
			pIOPImageRec->timeBooted = 0;
			pIOPImageRec->imageOne = 0;
			pIOPImageRec->imageOneAccepted = FALSE;
			pIOPImageRec->imageTwo = 0;
			pIOPImageRec->imageTwoAccepted = FALSE;

			IOPImageRecord::RqInsertRow* preqInsert;
			preqInsert = new IOPImageRecord::RqInsertRow(*pIOPImageRec);
			assert(preqInsert);

			// send the insert row request
			Send (preqInsert, 0,
						   REPLYCALLBACK (DdmUpgrade, InsertDummyIOPImageTableRows));		
			break;
		}	
	case 3:
		{
			RqPtsInsertRow* pReply = (RqPtsInsertRow*) pMsg;
			assert(pReply->GetRowIdDataCount()==1);
			Reply(pReplyMsg, OK);
		}
		break;
	default:
		assert(0);
		break;
	}
	i++;

	return OK;
}
#endif

//  DdmUpgrade::Enable (pMsgReq)
//
//  Description:
//   Lets go!

STATUS DdmUpgrade::Enable(Message *pMsgReq)
{
	TRACE_ENTRY(DdmUpgrade::Enable());
	
#ifdef DEBUG_UPGRADE
	InsertDummyIOPImageTableRows(pMsgReq);
#else
	Reply(pMsgReq,OK);
#endif
	return OK;
		
}
/* end DdmUpgrade::Enable  *****************************************************/

//  DdmUpgrade::ProcessUpgradeMessages (pMsgReq)
//
//  Description:
//		This adds the message to the command processing queue if a message
//		is currently being processed, or it ispatched it to the appropriate
//		method if the upgrade master is idle.
//
//  Inputs:
//  Outputs:
//    Returns OK if all is cool, else an error.
//

STATUS DdmUpgrade::ProcessUpgradeMessages(Message *pMsgReq) 
{

	TRACE_ENTRY(DdmUpgrade::ProcessUpgradeMessages());

	// allocate and initialize context
	CONTEXT* pContext = new (tZERO) CONTEXT;
	pContext->msg = pMsgReq;
	
	// queue request based upon the message code
	switch (pContext->msg->reqCode)
	{
	case REQ_UPGRADE_ADD_IMAGE:
		SubmitRequest(function_AddImage, pContext);
		break;
	case REQ_UPGRADE_OPEN_IMAGE:
		SubmitRequest(function_OpenImage, pContext);
		break;
	case REQ_UPGRADE_MODIFY_DEFAULT_IMAGE:
		SubmitRequest(function_ModifyDefaultImage, pContext);
		break;
	case REQ_UPGRADE_QUERY_IMAGES:
		SubmitRequest(function_QueryImages, pContext);
		break;
	case REQ_UPGRADE_ASSOCIATE_IMAGE:
		SubmitRequest(function_AssociateImage, pContext);
		break;	
	case REQ_UPGRADE_MAKE_PRIMARY:
		SubmitRequest(function_MakePrimary, pContext);
		break;
	default:
		assert(0);
		break;
	}

	return OK;
}
/* end DdmUpgrade::ProcessUpgradeMessages  ***********************************************/

//  DdmUpgrade::ListenerForCommands (status)
//
//  Description:
//     This is called when any CmdSender objects calls its Execute method.
//     This allows the Upgrade Master to receive commands from the SSAPI and 
//     act upon them
//
//  Inputs:
//	  HANDLE handle - handle to indentify request to command sender
//    void* CmdData - command inputs
//
//  Outputs:
//    none
//
void DdmUpgrade::ListenerForCommands(HANDLE handle, void* pCmdData)
{
	TRACE_ENTRY(DdmUpgrade::ListenerForCommands(HANDLE, void*));

	STATUS status = OK;

	// allocate and initialize context
	CONTEXT* pContext = new (tZERO) CONTEXT;
	pContext->handle = handle;
	pContext->pCmdInfo =  new CmdInfoBase(pCmdData);

	// set notes in pContext

	switch (pContext->pCmdInfo->GetOpcode()) {
	case CMND_ADD_IMAGE:
		SubmitRequest(function_AddImage, pContext);
		break;
	case CMND_ASSOCIATE_IMAGE:
		SubmitRequest(function_AssociateImage, pContext);
		break;
	case CMND_UNASSOCIATE_IMAGE:
		SubmitRequest(function_UnassociateImage, pContext);
		break;
	case CMND_ASSIGN_DEFAULT:
		SubmitRequest(function_AssignDefaultImage, pContext);
		break;
	case CMND_GET_FILESYS_INFO:
		SubmitRequest(function_GetFileSysInfo, pContext);
		break;
	case CMND_MAKE_PRIMARY:
		SubmitRequest(function_MakePrimary, pContext);
		break;
	case CMND_BOOT_IMAGE:
		SubmitRequest(function_BootImage, pContext);
		break;
	case CMND_DELETE_IMAGE:
		SubmitRequest(function_DeleteImage, pContext);
		break;
	default:
		assert(0);
		break;
	}
}
/* end DdmUpgrade::ListenerForCommands  ************************************/

//****************************
// PRIVATE METHODS BEGIN HERE
//****************************

//  DdmUpgrade::IopImageRowRead
//
//  Description:
//		Read a row from the IOP Image Table based upon the slot
//
//  Inputs:
//		pContext - call back context
//		cb - call back routine
//		slot - key to read row with 
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::IopImageRowRead(CONTEXT* pContext, 
								   ReplyCallback cb, 
								   TySlot* slot) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::IopImageRowRead());

	// initialize the callback context state
	m_State = IOP_IMAGE_ROW_READ;

	// allocate and initialize a read row request
	IOPImageRecord::RqReadRow* preqRead;
	preqRead = new IOPImageRecord::RqReadRow(CT_IOP_IMAGE_TABLE_SLOT,
		slot, sizeof(TySlot));
	assert(preqRead);

	// send read row request
	status = Send (preqRead, pContext, cb);

	// make sure everything went ok and return status
    assert (status == CTS_SUCCESS);
	return status;

}
/* end DdmUpgrade::IopImageRowRead  ********************************/

//  DdmUpgrade::ImageDescRowRead
//
//  Description:
//		Read a row from the Image Descriptor Table based upon a RowId
//
//  Inputs:
//		pContext - call back context
//		cb - call back routine
//		key - key to read row with
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::ImageDescRowRead(CONTEXT* pContext, 
								   ReplyCallback cb, 
								   RowId key) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::ImageDescRowRead());

	// initialize correct state in call back context
	m_State = IMAGE_DESC_ROW_READ;

	// allocate and initialize read row request
	ImageDescRecord::RqReadRow* preqRead;
	preqRead = new ImageDescRecord::RqReadRow(key);
	assert(preqRead);

	// send read row request
	status = Send (preqRead, pContext, cb);

	// make sure everything went ok and return status
    assert (status == CTS_SUCCESS);
	return status;
}
/* end DdmUpgrade::ImageDescRowRead  ********************************/

//  DdmUpgrade::DefaultImageRowRead
//
//  Description:
//		Read a row from the Default Image Table based upon type
//
//  Inputs:
//		pContext - call back context
//		cb - call back routine
//		type - type of board
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::DefaultImageRowRead(CONTEXT* pContext, 
								   ReplyCallback cb, 
								   ImageType* type) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::DefaultImageRowRead());

	// initialize correct state in call back context
	m_State = DEFAULT_IMAGE_ROW_READ;

	// allocate and initialize read row request
	DefaultImageRecord::RqReadRow* preqRead;
	preqRead = new DefaultImageRecord::RqReadRow(CT_DEF_IMAGE_TABLE_TYPE,
												type,
												sizeof(U32));
	assert(preqRead);
	// send read row request
	status = Send (preqRead, pContext, cb);

	// make sure everything went ok and return status
    assert (status == CTS_SUCCESS);
	return status;
}
/* end DdmUpgrade::DefaultImageRowRead  ********************************/

#ifdef DEBUG_UPGRADE
//  DdmUpgrade::DefineDefaultImageTable
//
//  Description:
//		Defines the default image table
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::DefineDefaultImageTable() 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::DefineDefaultImageTable());

	m_State = DEFAULT_IMAGE_TABLE_DEFINED;

	// allocate and initialize define table request
	DefaultImageRecord::RqDefineTable * preqDefTab;
	preqDefTab = new DefaultImageRecord::RqDefineTable();
	assert(preqDefTab);

	// send table definition request
	status = Send(preqDefTab, 
		REPLYCALLBACK (DdmUpgrade, InitReplyHandler));

	// make sure everything went ok and return status
	assert (status == CTS_SUCCESS);
	return status;

}
/* end DdmUpgrade::DefineDefaultImageTable  ********************************/
#endif

//  DdmUpgrade::DefineImageDescTable
//
//  Description:
//		Defines image descriptor table if it does not already exist
//
//  Inputs:
//		pContext - call back context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::DefineImageDescTable() 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::DefineImageDescTable());

	// initialize call back state
	m_State = IMAGE_DESC_TABLE_DEFINED;

	// allocate and initialize table definition request
	ImageDescRecord::RqDefineTable * preqDefTab;
	preqDefTab = new ImageDescRecord::RqDefineTable();
	assert(preqDefTab);

	// send table definition request
	status = Send(preqDefTab, 
		REPLYCALLBACK (DdmUpgrade, InitReplyHandler));

	// make sure everything went ok and return status
	assert (status == CTS_SUCCESS);
	return status;

}
/* end DdmUpgrade::DefineImageDescTable  ********************************/

//  DdmUpgrade::InitializeCmdServer
//
//  Description:
//    Initializes the command server for use as an interface between 
//    the AlarmManager and the SSAPI
//
//  Inputs:
//	  none
//
//  Outputs:
//    status -  return OK if ok
//
STATUS DdmUpgrade::InitializeCmdServer()
{
	TRACE_ENTRY(DdmUpgrade::InitializeCmdServer());

	m_State = CMD_SERVER_INITIALIZED;

	// create the cmdserver object and call its initialize method
	// wait for initialization done reply
#ifdef VAR_CMD_QUEUE
	m_pCmdServer = new CmdServer(
		UPMSTR_CMD_QUEUE_TABLE,
		this, 
		(pCmdCallback_t)&DdmUpgrade::ListenerForCommands);
#else
	m_pCmdServer = new CmdServer(
		UPMSTR_CMD_QUEUE_TABLE,
		// will go away with variable size
		sizeof(CmdInfoBase),
		// will go away with variable size
		sizeof(CmdEventBase),
		this, 
		(pCmdCallback_t)&DdmUpgrade::ListenerForCommands);
#endif

	m_pCmdServer->csrvInitialize(
		(pInitializeCallback_t)&DdmUpgrade::ObjectInitializeReply);

	return OK;
}
/* end DdmUpgrade::amstrObjectIntializeReply  ************************************/

//  DdmUpgrade::ObjectIntializeReply (status)
//
//  Description:
//    Waits for initialized reply from command server
//
//  Inputs:
//	  status
//
//  Outputs:
//    none
//
void DdmUpgrade::ObjectInitializeReply(STATUS status)
{
	TRACE_ENTRY(DdmUpgrade::ObjectInitializeReply())

	if (!status) 
	{
		// We are initialized successfully, so we should be ready to accept
		// commmands.

		// allocate CONTEXT and go to standard AlarmInitReplyHandler
		InitReplyHandler(NULL);
	}
}
/* end DdmUpgrade::ObjectIntializeReply  ************************************/

//  DdmUpgrade::InitReplyHandler
//
//  Description:
//		This function is the call back routine use through out the 
//		Upgrade Master initialization process.
//
//  Inputs:
//		pReply_ - reply from request
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::InitReplyHandler(Message* pReply_) 
{
	
	TRACE_ENTRY(DdmUpgrade::InitReplyHandler());

	// make sure reply is valid and extract call back context
	if (pReply_ != NULL)
		assert(pReply_->Status()==OK || pReply_->Status() == ercTableExists);

	// state change based on call back context state
	switch (m_State)
	{
#ifdef DEBUG_UPGRADE
	case DEFAULT_IMAGE_TABLE_DEFINED:
		// make a call to define the image descriptor table
		DefineImageDescTable();
		break;
#endif
	case IMAGE_DESC_TABLE_DEFINED:
		// reply to the original initialization request message and
		// delete the call back context
		InitializeCmdServer();
		break;
	case CMD_SERVER_INITIALIZED:
		Reply(m_pInitMsg, OK);
		ProcessNextCommand();
		break;
	default:
		assert(0);
		break;
	}

	// delete the reply
	if (pReply_)
		delete pReply_;

	// return status
	return CTS_SUCCESS;
}
/* end DdmUpgrade::InitReplyHandler  ********************************/

//  DdmUpgrade::AddImage (CONTEXT* pContext)
//
//  Description:
//		Entry point for an add image request
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::AddImage(CONTEXT *pContext) 
{
	TRACE_ENTRY(DdmUpgrade::AddImage());
	U32 cbImage;

	// initialize call back context state
	m_State = FILE_ADDED_TO_SYS;

	// case msg to correct type
	MsgAddImage* msg = (MsgAddImage*) pContext->msg;
	AddImageCmdInfo* pCmdInfo = (AddImageCmdInfo*) pContext->pCmdInfo;
	
	// get image to be added
	if (msg)
		cbImage = msg->GetImage(&pContext->pData);
	else
		cbImage = pCmdInfo->GetImage(&pContext->pData);

	img_hdr_t* pImgHdr = (img_hdr_t*)pContext->pData;
	char* pImage = (char*) pContext->pData + pImgHdr->i_imageoffset;

	UnicodeString us(StringClass((char*)&pImgHdr->i_imagename));
	UnicodeString16 imageName;
	us.CString(imageName, sizeof(imageName));

	// allocate and initialize a file system add file request message
	MsgAddFile* addFileMsg = new MsgAddFile(
		//pImgHdr->i_zipsize,
		//pImage,
		cbImage,
		pContext->pData,
		IMAGE_TYPE,
		imageName);
	assert(addFileMsg);
	
	// send add file request to file system master
	return Send(addFileMsg, 
		pContext, 
		REPLYCALLBACK(DdmUpgrade, AddImageReplyHandler));	

}
/* end DdmUpgrade::AddImage  ***********************************************/

//  DdmUpgrade::InsertImageDescRow (CONTEXT* pContext)
//
//  Description:
//		Inserts an image descriptor row in the image descriptor table
//		for the image just added to the file system
//
//  Inputs:
//		pContext - call back context
//		fileKey - key of image in the file system
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::InsertImageDescRow(CONTEXT *pContext, 
									  RowId fileKey) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::InsertImageDescRow());

	// initialize the call back state
	m_State = IMAGE_DESC_ROW_INSERTED;

	img_hdr_t* pImgHdr = (img_hdr_t*)pContext->pData;

	// allocate and initialize new Image Desc Record
	// jlo - dummy values will come off image header
	// need to convert type here
	pContext->pImageDescRec = new ImageDescRecord(
		pImgHdr->i_mjr_ver, 
		pImgHdr->i_mnr_ver, 
		pImgHdr->i_day, 
		pImgHdr->i_month, 
		pImgHdr->i_year,
		pImgHdr->i_hour, 
		pImgHdr->i_min, 
		pImgHdr->i_sec, 
		(ImageType)pImgHdr->i_type, 
		fileKey);
	assert(pContext->pImageDescRec);
			
	// allocate and initialize the insert row request
	ImageDescRecord::RqInsertRow* preqInsert;
	preqInsert = new ImageDescRecord::RqInsertRow(*pContext->pImageDescRec);
	assert(preqInsert);

	// send the insert row request
    status = Send (preqInsert, pContext,
                   REPLYCALLBACK (DdmUpgrade, AddImageReplyHandler));

	// make sure everything went ok and return the status
    assert (status == CTS_SUCCESS);
	return status;
}
/* end DdmUpgrade::InsertImageDescRow  ***********************************************/

//  DdmUpgrade::InsertDefaultImageRow (CONTEXT* pContext)
//
//  Description:
//		Entry point for assign default image request
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::InsertDefaultImageRow(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::InsertDefaultImageRow());

	// initialize state of call back context
	m_State = DEFAULT_IMAGE_ROW_INSERTED;

	// extract imageKey
	RowId imageKey = pContext->rid;

	// Construct a new DefaultIMageRecord with info from ImageDescRecord.
	ImageDescRecord* pImageDescRec = pContext->pImageDescRec;
	pContext->pDefaultImageRec = new DefaultImageRecord(
		pImageDescRec->majorVersion,
		pImageDescRec->minorVersion,
		pImageDescRec->day,
		pImageDescRec->month,
		pImageDescRec->year,
		pImageDescRec->hour,
		pImageDescRec->minute,
		pImageDescRec->second,
		pImageDescRec->type,
		imageKey);
	assert(pContext->pDefaultImageRec);

	// allocate and initialize insert row request
	DefaultImageRecord::RqInsertRow* preqInsert;
	preqInsert = new DefaultImageRecord::RqInsertRow(*pContext->pDefaultImageRec);
	assert(preqInsert);

	// send insert row request
	status = Send (preqInsert, pContext,
                   REPLYCALLBACK (DdmUpgrade, AddImageReplyHandler));

	// make sure everything went ok and return status
    assert (status == CTS_SUCCESS);
	return status;
}
/* end DdmUpgrade::InsertDefaultImageRow  ***********************************************/

//  DdmUpgrade::AddImageReplyHandler
//
//  Description:
//		State machine utilized by an add image request
//
//  Inputs:
//		pReply_ - reply from request
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::AddImageReplyHandler(Message* pReply_) 
{
	
	TRACE_ENTRY(DdmUpgrade::AddImageReplyHandler());

	// make sure reply is valid and extract call back context
	assert (pReply_ != NULL);
	assert(pReply_->Status()==OK || pReply_->Status()==ercKeyNotFound
		|| pReply_->Status()==CTS_FILESYS_OUT_OF_MEMORY);
	CONTEXT* pContext = (CONTEXT *) pReply_->GetContext();
	assert (pContext != NULL);

	// state change based on call back context state
	switch (m_State)
	{
	case FILE_ADDED_TO_SYS:
		{
			// cast add file message to correct type
			MsgAddFile* pReply = (MsgAddFile*) pReply_;	
			if (pReply->Status()==CTS_FILESYS_OUT_OF_MEMORY)
			{
				if (pContext->msg)
					Reply(pContext->msg, CTS_FILESYS_OUT_OF_MEMORY);
				else
				{
					// reply to the original add image request
					AddImageEvent* pEvt = 
						new AddImageEvent(CTS_FILESYS_OUT_OF_MEMORY,
						0);
					void* pData;
					U32	cbCmdInfo;
					cbCmdInfo = pContext->pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
					m_pCmdServer->csrvReportCmdStatus(
						pContext->handle,
						CTS_FILESYS_OUT_OF_MEMORY,
						pEvt,
						sizeof(AddImageEvent),
						pData,
						cbCmdInfo);
#else
					m_pCmdServer->csrvReportCmdStatus(
						pContext->handle,
						CTS_FILESYS_OUT_OF_MEMORY,
						pEvt,
						pData);
#endif
					delete pEvt;
					delete pData;			
				}
				ClearAndDeleteContext(pContext);
				ProcessNextCommand();
			}
			else
				// call method to insert the image desc row for this image 
				InsertImageDescRow(pContext, pReply->GetFileKey());
			break;
		}
	case IMAGE_DESC_ROW_INSERTED:
		{
			RqPtsInsertRow* pReply = (RqPtsInsertRow*) pReply_;
			assert(pReply->GetRowIdDataCount()==1);
			pContext->rid = *pReply->GetRowIdDataPtr();
			DefaultImageRowRead(pContext,
				REPLYCALLBACK(DdmUpgrade,AddImageReplyHandler),
				&pContext->pImageDescRec->type);
			break;
		}
	case DEFAULT_IMAGE_ROW_READ:
		{
			DefaultImageRecord::RqReadRow* pReply = (DefaultImageRecord::RqReadRow*) pReply_;
			if (pReply->Status()==ercKeyNotFound)
				InsertDefaultImageRow(pContext);
			else
			{
				// set imagekey and reply
				if (pContext->msg)
				{
					MsgAddImage* msg = (MsgAddImage*) pContext->msg;
					msg->SetImageKey(pContext->rid);
					Reply(msg, OK);
				}
				else
				{
					// reply to the original add image request
					AddImageEvent* pEvt = 
						new AddImageEvent(OK,
						pContext->rid);
					void* pData;
					U32	cbCmdInfo;
					cbCmdInfo = pContext->pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
					m_pCmdServer->csrvReportCmdStatus(
						pContext->handle,
						OK,
						pEvt,
						sizeof(AddImageEvent),
						pData,
						cbCmdInfo);
					m_pCmdServer->csrvReportEvent(
						EVT_IMAGE_ADDED,
						pEvt,
						sizeof(AddImageEvent));
#else
					m_pCmdServer->csrvReportCmdStatus(
						pContext->handle,
						OK,
						pEvt,
						pData);
					m_pCmdServer->csrvReportEvent(
						EVT_IMAGE_ADDED,
						pEvt);
#endif
					delete pEvt;
					delete pData;			
				}
				ClearAndDeleteContext(pContext);
				ProcessNextCommand();
			}
			break;
		}
	case DEFAULT_IMAGE_ROW_INSERTED:
		{
			// set imagekey and reply
			if (pContext->msg)
			{
				MsgAddImage* msg = (MsgAddImage*) pContext->msg;
				msg->SetImageKey(pContext->rid);
				Reply(msg, OK);
			}
			else
			{
				// reply to the original add image request
				AddImageEvent* pEvt = 
					new AddImageEvent(OK,
					pContext->rid);
				void* pData;
				U32	cbCmdInfo;
				cbCmdInfo = pContext->pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
				m_pCmdServer->csrvReportCmdStatus(
					pContext->handle,
					OK,
					pEvt,
					sizeof(AddImageEvent),
					pData,
					cbCmdInfo);
				m_pCmdServer->csrvReportEvent(
					EVT_IMAGE_ADDED,
					pEvt,
					sizeof(AddImageEvent));
#else
				m_pCmdServer->csrvReportCmdStatus(
					pContext->handle,
					OK,
					pEvt,
					pData);
				m_pCmdServer->csrvReportEvent(
					EVT_IMAGE_ADDED,
					pEvt);
#endif
				delete pEvt;
				delete pData;			
			}
			ClearAndDeleteContext(pContext);
			ProcessNextCommand();
			break;
		}
	default:
		assert(0);
		break;
	}

	// delete the reply
	if (pReply_)
		delete pReply_;

	// return status
	return CTS_SUCCESS;

}
/* end DdmUpgrade::AddImageReplyHandler  ********************************/

//  DdmUpgrade::DeleteImage (CONTEXT* pContext)
//
//  Description:
//		Entry point for a delete image request
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::DeleteImage(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::DeleteImage());

	// cast command to correct type
	DeleteImageCmdInfo* pCmdInfo = (DeleteImageCmdInfo*) pContext->pCmdInfo;

	// read image desc row of this image from the image descriptor
	// table based on image key 
    status = ImageDescRowRead(pContext, 
		REPLYCALLBACK(DdmUpgrade, DeleteImageReplyHandler),
		pCmdInfo->GetImageKey());
	
	// make sure everything went ok and return status
	assert (status == CTS_SUCCESS);
	return status;

 }
/* end DdmUpgrade::DeleteImage  ***********************************************/

//  DdmUpgrade::DeleteImageFile (CONTEXT* pContext)
//
//  Description:
//		Delets image file from the file system based upon file key
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::DeleteImageFile(CONTEXT *pContext) 
{

	// initialize call back state
	m_State = FILE_DELETED_FROM_SYS;

	// allocate and initialize file system delete file message
	MsgDeleteFile* deleteFileMsg = new MsgDeleteFile(
		pContext->pImageDescRec->fileDescKey);
	assert(deleteFileMsg);

	// send delete file request
	return Send(deleteFileMsg, 
		pContext, 
		REPLYCALLBACK(DdmUpgrade, DeleteImageReplyHandler));	

}
/* end DdmUpgrade::DeleteImageFile  ***********************************************/

//  DdmUpgrade::DeleteImageDescRow (CONTEXT* pContext)
//
//  Description:
//		Deletes image desc row from the image desc table based upon image key
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::DeleteImageDescRow(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::DeleteImageDescRow());

	// initialize call back state
	m_State = IMAGE_DESC_ROW_DELETED;
	
	// allocate and initialize delete row request
	ImageDescRecord::RqDeleteRow* preqDelete;
	preqDelete = new ImageDescRecord::RqDeleteRow(pContext->pImageDescRec->rid);
	assert(preqDelete);

	// send delete row request
    status = Send (preqDelete, pContext,
                   REPLYCALLBACK (DdmUpgrade, DeleteImageReplyHandler));

	// make sure everything went ok and return status
    assert (status == CTS_SUCCESS);
	return status;
 }
/* end DdmUpgrade::DeleteImageDescRow  ***********************************************/

//  DdmUpgrade::DeleteImageReplyHandler
//
//  Description:
//		State machine to control delete image request message.
//
//  Inputs:
//		pReply_ - reply from request
//
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::DeleteImageReplyHandler(Message* pReply_) 
{
	
	TRACE_ENTRY(DdmUpgrade::DeleteImageReplyHandler());

	// make sure reply is valid and extract call back context
	assert (pReply_ != NULL);
	CONTEXT* pContext = (CONTEXT *) pReply_->GetContext();
	assert (pContext != NULL);

	// case cmdInfo to the correct type
	DeleteImageCmdInfo* pCmdInfo = 
		(DeleteImageCmdInfo*) pContext->pCmdInfo;

	if (pReply_->Status() != OK || 
		(pReply_->Status() == CTS_FILESYS_FILE_NOT_FOUND
		&& m_State == FILE_DELETED_FROM_SYS))
	{
		if (pReply_->Status() == ercKeyNotFound &&
			m_State == IMAGE_DESC_ROW_READ)
		{
			DeleteImageEvent* pEvt = 
				new DeleteImageEvent(CTS_UPGRADE_IMAGE_NOT_FOUND,
				pCmdInfo->GetImageKey());
			void* pData;
			U32 cbCmdInfo;
			cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
			m_pCmdServer->csrvReportCmdStatus(
				pContext->handle,
				CTS_UPGRADE_IMAGE_NOT_FOUND,
				pEvt,
				sizeof(DeleteImageEvent),
				pData, 
				cbCmdInfo);
#else
			m_pCmdServer->csrvReportCmdStatus(
				pContext->handle,
				CTS_UPGRADE_IMAGE_NOT_FOUND,
				pEvt,
				pData);
#endif
			delete pEvt;			
			delete pData;
			// delete the call back context
			ClearAndDeleteContext(pContext);
			ProcessNextCommand();
			return CTS_UPGRADE_IMAGE_NOT_FOUND;
		}
		else
			assert(0);
	}

	// state change based on call back context state
	switch (m_State)
	{
	case IMAGE_DESC_ROW_READ:
		{
			// cast reply to correct type
			ImageDescRecord::RqReadRow* pReply = 
				(ImageDescRecord::RqReadRow*) pReply_;
			assert(pReply->GetRowCount()==1);
			// extract image desc row read from reply
			pContext->pImageDescRec = pReply->GetRowCopy();
			if (pContext->pImageDescRec->iopCount!=0)
			{
				DeleteImageEvent* pEvt = 
					new DeleteImageEvent(CTS_UPGRADE_IMAGE_BUSY,
					pCmdInfo->GetImageKey());
				void* pData;
				U32 cbCmdInfo;
				cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
				m_pCmdServer->csrvReportCmdStatus(
					pContext->handle,
					CTS_UPGRADE_IMAGE_BUSY,
					pEvt,
					sizeof(DeleteImageEvent),
					pData,
					cbCmdInfo);
#else
				m_pCmdServer->csrvReportCmdStatus(
					pContext->handle,
					CTS_UPGRADE_IMAGE_BUSY,
					pEvt,
					pData);
#endif
				delete pEvt;			
				delete pData;
				// delete the call back context
				ClearAndDeleteContext(pContext);
				ProcessNextCommand();
				return CTS_UPGRADE_IMAGE_BUSY;
			}
			DefaultImageRowRead(pContext, 
				REPLYCALLBACK(DdmUpgrade, DeleteImageReplyHandler),
				&pContext->pImageDescRec->type);
			break;
		}
	case DEFAULT_IMAGE_ROW_READ:
		{
			// cast reply to correct type
			DefaultImageRecord::RqReadRow* pReply = 
				(DefaultImageRecord::RqReadRow*) pReply_;

			// extract default image row read from reply
			pContext->pDefaultImageRec = pReply->GetRowCopy();

			if (pContext->pDefaultImageRec->imageKey == pCmdInfo->GetImageKey())
			{
				DeleteImageEvent* pEvt = 
					new DeleteImageEvent(CTS_UPGRADE_IMAGE_DEFAULT,
					pCmdInfo->GetImageKey());
				void* pData;
				U32 cbCmdInfo;
				cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
				m_pCmdServer->csrvReportCmdStatus(
					pContext->handle,
					CTS_UPGRADE_IMAGE_DEFAULT,
					pEvt,
					sizeof(DeleteImageEvent),
					pData,
					cbCmdInfo);
#else
				m_pCmdServer->csrvReportCmdStatus(
					pContext->handle,
					CTS_UPGRADE_IMAGE_DEFAULT,
					pEvt,
					pData);
#endif
				delete pEvt;
				delete pData;
				// delete the call back context
				ClearAndDeleteContext(pContext);
				ProcessNextCommand();
				return CTS_UPGRADE_IMAGE_DEFAULT;
			}
		
			// call method to delete image file from the file system
			DeleteImageFile(pContext);
			break;
		}
	case FILE_DELETED_FROM_SYS:
		// delete the appropriate image desc row
		DeleteImageDescRow(pContext);
		break;
	case IMAGE_DESC_ROW_DELETED:
		{
			// reply to the original delete image request
			DeleteImageEvent* pEvt = 
				new DeleteImageEvent(OK,
				pCmdInfo->GetImageKey());
			void* pData;
			U32	cbCmdInfo;
			cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
			m_pCmdServer->csrvReportCmdStatus(
				pContext->handle,
				OK,
				pEvt,
				sizeof(DeleteImageEvent),
				pData,
				cbCmdInfo);
			m_pCmdServer->csrvReportEvent(
				EVT_IMAGE_DELETED,
				pEvt,
				sizeof(DeleteImageEvent));
#else
			m_pCmdServer->csrvReportCmdStatus(
				pContext->handle,
				OK,
				pEvt,
				pData);
			m_pCmdServer->csrvReportEvent(
				EVT_IMAGE_DELETED,
				pEvt);
#endif
			delete pEvt;
			delete pData;			
			// delete call back context
			ClearAndDeleteContext(pContext);
			ProcessNextCommand();
			break;
		}
	default:
		assert(0);
		break;
	}

	// delete reply 
	if (pReply_)
		delete pReply_;

	// return status
	return CTS_SUCCESS;
}
/* end DdmUpgrade::DeleteImageReplyHandler  ********************************/

//  DdmUpgrade::AssociateImage (CONTEXT* pContext)
//
//  Description:
//		Entry point for an associate image request
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::AssociateImage(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::AssociateImage());

	// cast message to correct type
	MsgAssociateImage* pMsg = (MsgAssociateImage*) pContext->msg;
	AssociateImageCmdInfo* pCmdInfo = (AssociateImageCmdInfo*) pContext->pCmdInfo;

	// call method to read the appropriate IOP Image row from
	// the IOP Image table based upon slot specified in message
	if (pMsg)
		pContext->slot = pMsg->GetSlot();
	else
		pContext->slot = pCmdInfo->GetSlot();

	status = IopImageRowRead(pContext,
		REPLYCALLBACK(DdmUpgrade, AssociateImageReplyHandler),
		&pContext->slot);

	// return status
	return status;

}
/* end DdmUpgrade::AssociateImage  ***********************************************/

//  DdmUpgrade::AssociateWithImage
//
//  Description:
//		Associates specified image with slot in either imageOne or imageTwo
//		position
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::AssociateWithImage(CONTEXT *pContext)
{
	STATUS status;
	TySlot slot;
	RowId imageKey;

	TRACE_ENTRY(DdmUpgrade::AssociateWithImage());

	// initialize call back state
	m_State = IOP_IMAGE_ROW_MODIFIED;

	// cast message to correct type
	MsgAssociateImage* pMsg = (MsgAssociateImage*) pContext->msg;
	AssociateImageCmdInfo* pCmdInfo = (AssociateImageCmdInfo*) pContext->pCmdInfo;

	// extract the slot and imageKey from the message
	if (pMsg)
	{
		slot = pMsg->GetSlot();
		imageKey = pMsg->GetImageKey();
	}
	else
	{
		slot = pCmdInfo->GetSlot();
		imageKey = pCmdInfo->GetImageKey();
	}

	// check to see if it is already associated
	if ((pContext->pIOPImageRec->imageOne == imageKey) ||
		(pContext->pIOPImageRec->imageTwo == imageKey))
	{
		if (pMsg)
			Reply(pMsg, CTS_UPGRADE_IMAGE_ALREADY_ASSOCIATED);
		else
		{
			// this is an error.  there image is already associated
			AssociateImageEvent* pEvt = new AssociateImageEvent(
				CTS_UPGRADE_IMAGE_ALREADY_ASSOCIATED, 
				pCmdInfo->GetImageKey(),
				pCmdInfo->GetSlot());
			void* pData;
			U32 cbCmdInfo;
			cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
			m_pCmdServer->csrvReportCmdStatus(
				pContext->handle,
				CTS_UPGRADE_IMAGE_ALREADY_ASSOCIATED,
				pEvt,
				sizeof(AssociateImageEvent),
				pData,
				cbCmdInfo);
#else
			m_pCmdServer->csrvReportCmdStatus(
				pContext->handle,
				CTS_UPGRADE_IMAGE_ALREADY_ASSOCIATED,
				pEvt,
				pData);
#endif
			delete pEvt;
			delete pData;			
		}
		// delete the call back context
		ClearAndDeleteContext(pContext);
		ProcessNextCommand();
		return CTS_UPGRADE_IMAGE_ALREADY_ASSOCIATED;
	}
	
	// if imageOne in staging area is empty, associate image in this position
	if (pContext->pIOPImageRec->imageOne == NULL)
	{
		assert(pContext->pIOPImageRec->imageOneAccepted == FALSE);
		pContext->pIOPImageRec->imageOne = imageKey;	
	}
	else
	{
		// if imageTwo in staging area is empty, associate image in this position
		if (pContext->pIOPImageRec->imageTwo == NULL)
		{
			assert(pContext->pIOPImageRec->imageTwoAccepted == FALSE);
			pContext->pIOPImageRec->imageTwo = imageKey;
		}
		else
		{
			// this is an error.  there is no room to associate the
			// image with this slot
			if (pMsg)
				Reply(pMsg, CTS_UPGRADE_NO_EMPTY_IMAGE);
			else
			{
				AssociateImageEvent* pEvt = new AssociateImageEvent(
					CTS_UPGRADE_NO_EMPTY_IMAGE, 
					pCmdInfo->GetImageKey(),
					pCmdInfo->GetSlot());
				void* pData;
				U32 cbCmdInfo;
				cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
				m_pCmdServer->csrvReportCmdStatus(
					pContext->handle,
					CTS_UPGRADE_NO_EMPTY_IMAGE,
					pEvt,
					sizeof(AssociateImageEvent),
					pData,
					cbCmdInfo);
#else
				m_pCmdServer->csrvReportCmdStatus(
					pContext->handle,
					CTS_UPGRADE_NO_EMPTY_IMAGE,
					pEvt,
					pData);
#endif
				delete pEvt;
				delete pData;			
			}
			// delete the call back context
			ClearAndDeleteContext(pContext);
			ProcessNextCommand();
			return CTS_UPGRADE_NO_EMPTY_IMAGE;
		}
	}

	// allocate and initialize modify row request
	IOPImageRecord::RqModifyRow* preqModify;
	preqModify = new IOPImageRecord::RqModifyRow(CT_IOP_IMAGE_TABLE_SLOT,
		&slot, sizeof(slot), *pContext->pIOPImageRec);
	assert(preqModify);

	// send modify field request (will either modify imageOne or imageTwo value
	status = Send (preqModify, pContext,
                   REPLYCALLBACK (DdmUpgrade, AssociateImageReplyHandler));

	// make sure everything went ok and return status
    assert (status == CTS_SUCCESS);
	return status;

}
/* end DdmUpgrade::AssociateWithImage  ***********************************************/

//  DdmUpgrade::IncrementIopCount (CONTEXT* pContext)
//
//  Description:
//		increment the value of the iopCount in the Image Desc
//		row associated with this image
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::IncrementIopCount(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::IncrementIopCount());

	// initialize the call back state
	m_State = IMAGE_DESC_ROW_MODIFIED;

	// increment the count in the image desc record
	pContext->pImageDescRec->iopCount++;
	pContext->pImageDescRec->handle = *((RowId*)pContext->handle);

	// allocate and initialize the modify field request
	ImageDescRecord::RqModifyRow* preqModify;
	preqModify = new ImageDescRecord::RqModifyRow(CT_PTS_RID_FIELD_NAME,
		&pContext->pImageDescRec->rid, sizeof(rowID),
		*pContext->pImageDescRec);
	assert(preqModify);

	// send the modify field request
	status = Send (preqModify, pContext,
                   REPLYCALLBACK (DdmUpgrade, AssociateImageReplyHandler));

	// make sure everything went ok and return status
    assert (status == CTS_SUCCESS);

	return status;

}
/* end DdmUpgrade::IncrementIopCount  ***********************************************/

//  DdmUpgrade::AssociateImageReplyHandler
//
//  Description:
//		State machine to control associate image request message.
//
//  Inputs:
//		pReply_ - reply from request
//
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::AssociateImageReplyHandler(Message* pReply_) 
{
	
	TRACE_ENTRY(DdmUpgrade::AssociateImageReplyHandler());
	
	// make sure reply is valid and extract the call back context
	assert (pReply_ != NULL);
	CONTEXT* pContext = (CONTEXT *) pReply_->GetContext();
	assert (pContext != NULL);

	// cast reply to the correct type
	MsgAssociateImage* pMsg = (MsgAssociateImage*) pContext->msg;
	AssociateImageCmdInfo* pCmdInfo = (AssociateImageCmdInfo*) pContext->pCmdInfo;
	
	// state change based on call back context state
	switch (m_State)
	{
	case IOP_IMAGE_ROW_READ:
		{
			IOPImageRecord::RqReadRow* pReply = (IOPImageRecord::RqReadRow*) pReply_;
			// make sure only one row was read
			assert(pReply->GetRowCount() == 1);
			// extract and save IOP Image row read
			pContext->pIOPImageRec = pReply->GetRowCopy();
			if (pMsg)
				ImageDescRowRead(pContext,
					REPLYCALLBACK(DdmUpgrade, AssociateImageReplyHandler),
					pMsg->GetImageKey());
			else
				ImageDescRowRead(pContext,
					REPLYCALLBACK(DdmUpgrade, AssociateImageReplyHandler),
					pCmdInfo->GetImageKey());
			break;
		}
	case IMAGE_DESC_ROW_READ:
		{
			// cast reply to correct type
			ImageDescRecord::RqReadRow* pReply = (ImageDescRecord::RqReadRow*) pReply_;
			// make sure image was found
			if (pReply->GetRowCount()==0)
			{
				if (pMsg)
					Reply(pMsg, CTS_UPGRADE_IMAGE_NOT_FOUND);
				else
				{
					AssociateImageEvent* pEvt = new AssociateImageEvent(CTS_UPGRADE_IMAGE_NOT_FOUND,
						pCmdInfo->GetImageKey(),
						pCmdInfo->GetSlot());
					void* pData;
					U32 cbCmdInfo;
					cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
					m_pCmdServer->csrvReportCmdStatus(
						pContext->handle,
						CTS_UPGRADE_IMAGE_NOT_FOUND,
						pEvt,
						sizeof(AssociateImageEvent),
						pData,
						cbCmdInfo);
#else
					m_pCmdServer->csrvReportCmdStatus(
						pContext->handle,
						CTS_UPGRADE_IMAGE_NOT_FOUND,
						pEvt,
						pData);
#endif
					delete pEvt;			
					delete pData;
				}
				ClearAndDeleteContext(pContext);
				ProcessNextCommand();
				return CTS_UPGRADE_IMAGE_NOT_FOUND;
			}
			// make sure only one image was found
			assert(pReply->GetRowCount() == 1);
			// extract row read from the reply
			pContext->pImageDescRec = pReply->GetRowCopy();
			// check to see if we have already done the association
			if (pContext->pIOPImageRec->handle == *((RowId*)pContext->handle))
			{
				// check to see if we have already incremented the IOP Count
				if (pContext->pImageDescRec->handle == *((RowId*)pContext->handle))
				{
					if (pMsg)
						Reply(pMsg, OK);
					else
					{
						AssociateImageEvent* pEvt = new AssociateImageEvent(OK,
							pCmdInfo->GetImageKey(),
							pCmdInfo->GetSlot());
						void* pData;
						U32 cbCmdInfo;
						cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
						m_pCmdServer->csrvReportCmdStatus(
							pContext->handle,
							OK,
							pEvt,
							sizeof(AssociateImageEvent),
							pData,
							cbCmdInfo);
						m_pCmdServer->csrvReportEvent(
							EVT_IMAGE_ASSOCIATED,
							pEvt,
							sizeof(AssociateImageEvent));
#else
						m_pCmdServer->csrvReportCmdStatus(
							pContext->handle,
							OK,
							pEvt,
							pData);
						m_pCmdServer->csrvReportEvent(
							EVT_IMAGE_ASSOCIATED,
							pEvt);
#endif
						delete pEvt;	
						delete pData;
					}
				}
				else
					IncrementIopCount(pContext);
			}
			else
				AssociateWithImage(pContext);
			break;
		}
	case IOP_IMAGE_ROW_MODIFIED:
		{
			//IOPImageRecord::RqModifyRow* preqModify = (IOPImageRecord::RqModifyRow*) pReply_;
			//U32 RowsModified = preqModify->GetRowCount();
			//RowId RowIdModified = *preqModify->GetRowIdPtr();
			
			// call method to increment the iopCount
			IncrementIopCount(pContext);
			break;
		}
	case IMAGE_DESC_ROW_MODIFIED:
		{
			// reply to original associate image request
			if (pMsg)
				Reply(pMsg, OK);
			else
			{
				AssociateImageEvent* pEvt = new AssociateImageEvent(OK,
					pCmdInfo->GetImageKey(),
					pCmdInfo->GetSlot());
				void* pData;
				U32 cbCmdInfo;
				cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
				m_pCmdServer->csrvReportCmdStatus(
					pContext->handle,
					OK,
					pEvt,
					sizeof(AssociateImageEvent),
					pData,
					cbCmdInfo);
				m_pCmdServer->csrvReportEvent(
					EVT_IMAGE_ASSOCIATED,
					pEvt,
					sizeof(AssociateImageEvent));
#else
				m_pCmdServer->csrvReportCmdStatus(
					pContext->handle,
					OK,
					pEvt,
					pData);
				m_pCmdServer->csrvReportEvent(
					EVT_IMAGE_ASSOCIATED,
					pEvt);
#endif
				delete pEvt;
				delete pData;
			}
			// delete the call back context
			ClearAndDeleteContext(pContext);
			ProcessNextCommand();
			break;
		}
	default:
		assert(0);
		break;
	}

	// delete the reply
	if (pReply_)
		delete pReply_;

	// return the status
	return CTS_SUCCESS;

}
/* end DdmUpgrade::AssociateImageReplyHandler  ********************************/

//  DdmUpgrade::UnassociateImage (CONTEXT* pContext)
//
//  Description:
//		Entry point for an unassociate image request
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::UnassociateImage(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::UnassociateImage());

	// cast message to correct type
	UnassociateImageCmdInfo* pCmdInfo = (UnassociateImageCmdInfo*) pContext->pCmdInfo;

	// read IOP image row from IOP image table based on slot
	pContext->slot = pCmdInfo->GetSlot();
	status = IopImageRowRead(pContext,
		REPLYCALLBACK(DdmUpgrade, UnassociateImageReplyHandler),
		&pContext->slot);

	// return status
	return status;
}
/* end DdmUpgrade::UnassociateImage  ***********************************************/

//  DdmUpgrade::UnassociateFromImage
//
//  Description:
//		Clears either imageOne or imageTwo, depending on value of imagePosition
//		and either imageOneAccepted or imageTwoAccepted
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::UnassociateFromImage(CONTEXT *pContext)
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::UnassociateFromImage());

	// initialize call back context state
	m_State = IOP_IMAGE_ROW_MODIFIED;

	// cast message to the correct type
	UnassociateImageCmdInfo* pCmdInfo = (UnassociateImageCmdInfo*) pContext->pCmdInfo;

	// extract slot from message
	TySlot slot = pCmdInfo->GetSlot();

	if (pContext->pIOPImageRec->primaryImage == pCmdInfo->GetImageKey())
	{
		UnassociateImageEvent* pEvt = new UnassociateImageEvent(CTS_UPGRADE_IMAGE_PRIMARY,
			pCmdInfo->GetImageKey(),
			pCmdInfo->GetSlot());
		void* pData;
		U32 cbCmdInfo;
		cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
		m_pCmdServer->csrvReportCmdStatus(
			pContext->handle,
			CTS_UPGRADE_IMAGE_PRIMARY,
			pEvt,
			sizeof(UnassociateImageEvent),
			pData,
			cbCmdInfo);
#else
		m_pCmdServer->csrvReportCmdStatus(
			pContext->handle,
			CTS_UPGRADE_IMAGE_PRIMARY,
			pEvt,
			pData);
#endif
		delete pEvt;			
		delete pData;
		ClearAndDeleteContext(pContext);
		ProcessNextCommand();
		return CTS_UPGRADE_IMAGE_PRIMARY;
	}

	if (pContext->pIOPImageRec->currentImage == pCmdInfo->GetImageKey())
	{
		UnassociateImageEvent* pEvt = new UnassociateImageEvent(CTS_UPGRADE_IMAGE_CURRENT,
			pCmdInfo->GetImageKey(),
			pCmdInfo->GetSlot());
		void* pData;
		U32 cbCmdInfo;
		cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
		m_pCmdServer->csrvReportCmdStatus(
			pContext->handle,
			CTS_UPGRADE_IMAGE_CURRENT,
			pEvt,
			sizeof(UnassociateImageEvent),
			pData,
			cbCmdInfo);
#else
		m_pCmdServer->csrvReportCmdStatus(
			pContext->handle,
			CTS_UPGRADE_IMAGE_CURRENT,
			pEvt,
			pData);
#endif
		delete pEvt;			
		delete pData;
		ClearAndDeleteContext(pContext);
		ProcessNextCommand();
		return CTS_UPGRADE_IMAGE_CURRENT;
	}

	// allocate and initialize modify request
	IOPImageRecord::RqModifyRow* preqModify;
	if (pContext->pIOPImageRec->imageOne == pCmdInfo->GetImageKey())
	{
		pContext->pIOPImageRec->imageOne = 0;
		pContext->pIOPImageRec->imageOneAccepted = FALSE;
	}	
	else
	{
		if (pContext->pIOPImageRec->imageTwo == pCmdInfo->GetImageKey())
		{
			pContext->pIOPImageRec->imageTwo = 0;
			pContext->pIOPImageRec->imageTwoAccepted = FALSE;
		}
		else
		{
			UnassociateImageEvent* pEvt = 
				new UnassociateImageEvent(CTS_UPGRADE_IMAGE_NOT_ASSOCIATED_WITH_SLOT,
				pCmdInfo->GetImageKey(),
				pCmdInfo->GetSlot());
			void* pData;
			U32 cbCmdInfo;
			cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
			m_pCmdServer->csrvReportCmdStatus(
				pContext->handle,
				CTS_UPGRADE_IMAGE_NOT_ASSOCIATED_WITH_SLOT,
				pEvt,
				sizeof(UnassociateImageEvent),
				pData,
				cbCmdInfo);
#else
			m_pCmdServer->csrvReportCmdStatus(
				pContext->handle,
				CTS_UPGRADE_IMAGE_NOT_ASSOCIATED_WITH_SLOT,
				pEvt,
				pData);
#endif
			delete pEvt;			
			delete pData;
			ClearAndDeleteContext(pContext);
			ProcessNextCommand();
			return CTS_UPGRADE_IMAGE_NOT_ASSOCIATED_WITH_SLOT;
		}
	}
	preqModify = new IOPImageRecord::RqModifyRow(CT_IOP_IMAGE_TABLE_SLOT,
		&slot, sizeof(slot), *pContext->pIOPImageRec);
	assert(preqModify);

	// send modify row request
	status = Send (preqModify, pContext,
                   REPLYCALLBACK (DdmUpgrade, UnassociateImageReplyHandler));

	// make sure everything went ok and return status
    assert (status == CTS_SUCCESS);
	return status;

}
/* end DdmUpgrade::UnassociateFromImage  ***********************************************/

//  DdmUpgrade::DecrementIopCount (CONTEXT* pContext)
//
//  Description:
//		Decrements the iopCount in the image desc row associated with 
//		the image being "unassocaited"
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::DecrementIopCount(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::DecrementIopCount());

	// initialize call back state
	m_State = IMAGE_DESC_ROW_MODIFIED;

	// cast command to the correct type
	UnassociateImageCmdInfo* pCmdInfo = (UnassociateImageCmdInfo*) pContext->pCmdInfo;

	// increment the count in the image desc record
	pContext->pImageDescRec->iopCount--;
	pContext->pImageDescRec->handle = *((RowId*)pContext->handle);

	// allocate and initialize the modify field request
	ImageDescRecord::RqModifyRow* preqModify;
	preqModify = new ImageDescRecord::RqModifyRow(CT_PTS_RID_FIELD_NAME,
		&pContext->pImageDescRec->rid, sizeof(rowID),
		*pContext->pImageDescRec);
	assert(preqModify);

	// send modify field request
	status = Send (preqModify, pContext,
                   REPLYCALLBACK (DdmUpgrade, UnassociateImageReplyHandler));

	// make sure everything went ok and return status
    assert (status == CTS_SUCCESS);
	return status;

}
/* end DdmUpgrade::DecrementIopCount  ***********************************************/

//  DdmUpgrade::UnassociateImageReplyHandler
//
//  Description:
//		State machine to control unassociate image request message.
//
//  Inputs:
//		pReply_ - reply from request
//
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::UnassociateImageReplyHandler(Message* pReply_) 
{
	
	TRACE_ENTRY(DdmUpgrade::UnassociateImageReplyHandler());

	assert(pReply_->Status()==OK);

	// make sure reply is valid and extract call back context 
	assert (pReply_ != NULL);
	CONTEXT* pContext = (CONTEXT *) pReply_->GetContext();
	assert (pContext != NULL);

	UnassociateImageCmdInfo* pCmdInfo = (UnassociateImageCmdInfo*) pContext->pCmdInfo;

	// state change based on call back context state
	switch (m_State)
	{
	case IOP_IMAGE_ROW_READ:
		{
			// cast reply to correct type
			IOPImageRecord::RqReadRow* pReply = (IOPImageRecord::RqReadRow*) pReply_;
			// make sure only one row was returned
			assert(pReply->GetRowCount() == 1);
			// extract IOP image row from reply
			pContext->pIOPImageRec = pReply->GetRowCopy();
			// read image desc row from table
			ImageDescRowRead(pContext,
				REPLYCALLBACK(DdmUpgrade, UnassociateImageReplyHandler),
				pCmdInfo->GetImageKey());
			break;
		}
	case IMAGE_DESC_ROW_READ:
		{
			// cast reply to correct type
			ImageDescRecord::RqReadRow* pReply = (ImageDescRecord::RqReadRow*) pReply_;
			// make sure only one row was read
			assert(pReply->GetRowCount() == 1);
			// extract row read from reply
			pContext->pImageDescRec = pReply->GetRowCopy();
			// if this image has already been unassociated, check to see if the 
			// iopCount has been decremented and do so if necessary, else, 
			// unassociate the image
			if (pContext->pIOPImageRec->handle == *((RowId*)pContext->handle))
			{
				// check to see if the count has already been decremented.  if
				// it has, return
				if (pContext->pImageDescRec->handle == *((RowId*)pContext->handle))
				{
					UnassociateImageEvent* pEvt = new UnassociateImageEvent(OK,
						pCmdInfo->GetImageKey(),
						pCmdInfo->GetSlot());
					void* pData;
					U32 cbCmdInfo;
					cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
					m_pCmdServer->csrvReportCmdStatus(
						pContext->handle,
						OK,
						pEvt,
						sizeof(UnassociateImageEvent),
						pData,
						cbCmdInfo);
					m_pCmdServer->csrvReportEvent(
						EVT_IMAGE_UNASSOCIATED,
						pEvt,
						sizeof(UnassociateImageEvent));
#else
					m_pCmdServer->csrvReportCmdStatus(
						pContext->handle,
						OK,
						pEvt,
						pData);
					m_pCmdServer->csrvReportEvent(
						EVT_IMAGE_UNASSOCIATED,
						pEvt);
#endif
					delete pEvt;			
					delete pData;
					// delete call back context
					ClearAndDeleteContext(pContext);
					ProcessNextCommand();			
				}
				else
					DecrementIopCount(pContext);
			}
			else
				UnassociateFromImage(pContext);
			break;
		}
	case IOP_IMAGE_ROW_MODIFIED:
		{
			DecrementIopCount(pContext);
			break;
		}
	case IMAGE_DESC_ROW_MODIFIED:
		{
			// reply to original unassociate message request
			UnassociateImageEvent* pEvt = new UnassociateImageEvent(OK,
				pCmdInfo->GetImageKey(),
				pCmdInfo->GetSlot());
			void* pData;
			U32 cbCmdInfo;
			cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
			m_pCmdServer->csrvReportCmdStatus(
				pContext->handle,
				OK,
				pEvt,
				sizeof(UnassociateImageEvent),
				pData,
				cbCmdInfo);
			m_pCmdServer->csrvReportEvent(
				EVT_IMAGE_UNASSOCIATED,
				pEvt,
				sizeof(UnassociateImageEvent));
#else
			m_pCmdServer->csrvReportCmdStatus(
				pContext->handle,
				OK,
				pEvt,
				pData);
			m_pCmdServer->csrvReportEvent(
				EVT_IMAGE_UNASSOCIATED,
				pEvt);
#endif
			delete pEvt;			
			delete pData;
			// delete call back context
			ClearAndDeleteContext(pContext);
			ProcessNextCommand();
			break;
		}
	default:
		assert(0);
		break;
	}

	// delete reply request
	if (pReply_)
		delete pReply_;

	// return status
	return CTS_SUCCESS;
}
/* end DdmUpgrade::UnassociateImageReplyHandler  ********************************/

//  DdmUpgrade::QueryImages (CONTEXT* pContext)
//
//  Description:
//		Entry point for query images request
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::QueryImages(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::QueryImages());

	// initialize the call back state
	m_State = IOP_IMAGE_TABLE_ENUMERATED;

	// allocate and initialize enum table request
	IOPImageRecord::RqEnumTable* preqEnum;
	preqEnum = new IOPImageRecord::RqEnumTable();
	assert(preqEnum);

	// send enum table request
	status = Send (preqEnum, pContext,
               REPLYCALLBACK (DdmUpgrade, QueryImagesReplyHandler));

	// make sure everything went ok and return status
    assert (status == CTS_SUCCESS);
	return status;

}
/* end DdmUpgrade::QueryImages  ***********************************************/

//  DdmUpgrade::QueryDefaultImagesTable (CONTEXT* pContext)
//
//  Description:
//		Enumerate the Default Image Table
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::QueryDefaultImagesTable(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::QueryDefaultImagesTable());

	// initialize the call back state
	m_State = DEFAULT_IMAGE_TABLE_ENUMERATED;

	// allocate and initialize enum table request
	DefaultImageRecord::RqEnumTable* preqEnum;
	preqEnum = new DefaultImageRecord::RqEnumTable();
	assert(preqEnum);

	// send enum table request
	status = Send (preqEnum, pContext,
		REPLYCALLBACK (DdmUpgrade, QueryImagesReplyHandler));

	// make sure everything went ok and return status
    assert (status == CTS_SUCCESS);
	return status;

}
/* end DdmUpgrade::QueryDefaultImagesTable  ***********************************************/

//  DdmUpgrade::QueryImageDescTable (CONTEXT* pContext)
//
//  Description:
//		Query the Image Descriptor Table for images
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::QueryImageDescTable(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::QueryImageDescTable());

	// cast message to correct type
	MsgQueryImages* msg = (MsgQueryImages*) pContext->msg;

	// get type of images being queried from message request
	ImageType type = msg->GetType();

	if (type == ALL_IMAGES)
	{
		// if all images are being requested, then we enumerate
		// the image desc table

		// initialize the call back state
		m_State = IMAGE_DESC_TABLE_ENUMERATED;

		// allocate and initialize enum table request
		ImageDescRecord::RqEnumTable* preqEnum;
		preqEnum = new ImageDescRecord::RqEnumTable();
		assert(preqEnum);

		// send enum table request
		status = Send (preqEnum, pContext,
                   REPLYCALLBACK (DdmUpgrade, QueryImagesReplyHandler));

	}
	else
	{
		// if a type of image to be read is specified, then read from
		// the image desc table by the key type

		// initialize the call back state
		m_State = IMAGE_DESC_ROW_READ;

		// allocate and initialize a read row request
		ImageDescRecord::RqReadRow* preqRead;
		preqRead = new ImageDescRecord::RqReadRow(CT_IDT_TYPE, &type,
			sizeof(type));
		assert(preqRead);

		// send read row request
		status = Send (preqRead, pContext,
                   REPLYCALLBACK (DdmUpgrade, QueryImagesReplyHandler));

	}

	// make sure everything went ok and return status
    assert (status == CTS_SUCCESS);
	return status;

}
/* end DdmUpgrade::QueryImageDescTable  ***********************************************/

//  DdmUpgrade::ReadImageFile
//
//  Description:
//		Read image file from file system 
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::ReadImageFile(CONTEXT* pContext)
{

	STATUS status;

	TRACE_ENTRY(DdmUpgrade::ReadImageFile());

	// initialize call back state
	m_State = FILE_LISTED_FROM_SYS;

	// allocate and initialize message to list files from file system
	MsgListFiles* listFileMsg = new
		MsgListFiles(pContext->pImageDescRec[pContext->index].fileDescKey);

	// send request to list files
	status = Send(listFileMsg, 
		pContext, 
		REPLYCALLBACK(DdmUpgrade, QueryImagesReplyHandler));

	// make sure everything went ok and return status
    assert (status == CTS_SUCCESS);
	return status;

}
/* end DdmUpgrade::ReadImageFile  ***********************************************/

//  DdmUpgrade::QueryImagesReplyHandler
//
//  Description:
//		State machine to control query images request message.
//
//  Inputs:
//		pReply_ - reply from request
//
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::QueryImagesReplyHandler(Message* pReply_) 
{
	SystemInfo* pSysInfo;
	const Entry* pEntry;
	UnicodeString fileName;
	I64 timeLoaded, timeCreated = 0;

	TRACE_ENTRY(DdmUpgrade::QueryImagesReplyHandler());

	// make sure reply is valid and extract the call back context
	assert (pReply_ != NULL);
	CONTEXT* pContext = (CONTEXT *) pReply_->GetContext();
	assert (pContext != NULL);
	if (pReply_->Status()!=OK)
		assert(pReply_->Status()==ercKeyNotFound &&
		m_State == IMAGE_DESC_ROW_READ);

	// state change based on call back context state
	switch (m_State)
	{
	case IOP_IMAGE_TABLE_ENUMERATED:
		{
			IOPImageRecord::RqEnumTable* pReply = 
				(IOPImageRecord::RqEnumTable*)pReply_;
			pContext->numberIOPDescs = pReply->GetRowCount();
			pContext->pIOPImageRec = pReply->GetRowCopy();
			QueryDefaultImagesTable(pContext);
			break;
		}
	case DEFAULT_IMAGE_TABLE_ENUMERATED:
		{
			U32 numberOfDefaultImages;
			DefaultImageRecord::RqEnumTable* pReply = 
				(DefaultImageRecord::RqEnumTable*)pReply_;
			numberOfDefaultImages = pReply->GetRowCount();
			pContext->pDefaultImageRec = pReply->GetRowCopy();
			// add each default image into DefaultImages utility class
			for (U16 i = 0; i < numberOfDefaultImages; i++)
			{
				pContext->pDefaultImagesHead = new DefaultImages(
					(ImageType)pContext->pDefaultImageRec[i].type,
					pContext->pDefaultImageRec[i].imageKey,
					pContext->pDefaultImagesHead);
			}
			QueryImageDescTable(pContext);
			break;
		}
	case IMAGE_DESC_TABLE_ENUMERATED:
	case IMAGE_DESC_ROW_READ:
		{
			// cast reply to the correct type and extract rows 
			if (m_State == IMAGE_DESC_TABLE_ENUMERATED) 
			{
				ImageDescRecord::RqEnumTable* pReply = 
					(ImageDescRecord::RqEnumTable*) pReply_;
				pContext->numberOfImages = pReply->GetRowCount();
				pContext->pImageDescRec = pReply->GetRowCopy();
			}
			else
			{
				ImageDescRecord::RqReadRow* pReply = 
					(ImageDescRecord::RqReadRow*) pReply_;
				if (pReply->Status()==OK)
				{
					pContext->numberOfImages = pReply->GetRowCount();
					pContext->pImageDescRec = pReply->GetRowCopy();
				}
				else
					pContext->numberOfImages = 0;
			}
			// initialize index and image iterator and read image 
			// files from file system
			pContext->pII = new ImageIterator();
			assert(pContext->pII);
			pContext->index = 0;
			if (pContext->index < pContext->numberOfImages)
				ReadImageFile(pContext);
			else
			{
				// reply to original message
				MsgQueryImages* pMsg = (MsgQueryImages*)pContext->msg;
				pMsg->SetImages(pContext->pII);
				Reply(pMsg, OK);
				ClearAndDeleteContext(pContext);
				ProcessNextCommand();				
			}
			break;
		}
	case FILE_LISTED_FROM_SYS:
		{
			// cast reply to correct type
			MsgListFiles* pReply = (MsgListFiles*) pReply_;
			// extract SystemInfo from file system list reply
			pReply->GetSysInfo(&pSysInfo);
			delete pReply;
			pReply_ = NULL;
			// make sure only one entry was found
			assert(pSysInfo->GetNumberOfEntries() == 1);
			// extract entry and fileName
			pEntry = pSysInfo->GetFirst();
			pEntry->GetFileName(&fileName);
			pEntry->GetCreationDate(&timeLoaded);
			// allocate and initialize image to add to image iterator
			Image* pImage = new Image(
				pContext->pImageDescRec[pContext->index].rid,
				pContext->pImageDescRec[pContext->index].majorVersion,
				pContext->pImageDescRec[pContext->index].minorVersion,
				pEntry->GetCbFile(),
				&fileName,
				// jlo - need to build off day, time, year, etc.
				//pImageDesc->timeCreated,
				&timeCreated,
				&timeLoaded,
				pContext->pImageDescRec[pContext->index].type,
				pContext->pImageDescRec[pContext->index].iopCount);
			assert(pImage);
			rowID* rid = pContext->pDefaultImagesHead->Default(
				pContext->pImageDescRec[pContext->index].type);
			if ( *rid == pContext->pImageDescRec[pContext->index].rid)
				pImage->SetDefault();
			// add image to the image iterator
			pContext->pII->AddImage(pImage);
			// delete system info of file
			delete pSysInfo;
			// increment index
			pContext->index++;

			// if there are still images to add, read image file
			if (pContext->index < pContext->numberOfImages)		
				ReadImageFile(pContext);
			else
			{
				// add IOP Desc to reply
				IOPDesc* pIOPDesc;
				for (U16 i = 0; i < pContext->numberIOPDescs; i++)
				{
					// only append this record if it is an initialzied IOPImageRecord
					if (pContext->pIOPImageRec[i].imageState == eImageState_Initialized)
					{
						I64 timeBooted = pContext->pIOPImageRec[i].timeBooted;
						pIOPDesc = new IOPDesc(
							pContext->pIOPImageRec[i].rid,
							pContext->pIOPImageRec[i].slot,
							pContext->pIOPImageRec[i].primaryImage,
							pContext->pIOPImageRec[i].currentImage,
							&timeBooted,
							pContext->pIOPImageRec[i].imageOne,
							pContext->pIOPImageRec[i].imageTwo,
							pContext->pIOPImageRec[i].imageOneAccepted,
							pContext->pIOPImageRec[i].imageTwoAccepted);
						pContext->pII->AddIOPDesc(pIOPDesc);		
					}
				}
				// reply to original message
				MsgQueryImages* pMsg = (MsgQueryImages*)pContext->msg;
				pMsg->SetImages(pContext->pII);
				Reply(pMsg, OK);
				ClearAndDeleteContext(pContext);
				ProcessNextCommand();
			}
			break;
		}
	default:
		assert(0);
		break;
	}

	// delete reply request
	if (pReply_)
		delete pReply_;

	// return status
	return CTS_SUCCESS;

}
/* end DdmUpgrade::QueryImagesReplyHandler  ********************************/

//  DdmUpgrade::OpenImage (CONTEXT* pContext)
//
//  Description:
//		Entry point for a open image request
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::OpenImage(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::OpenImage());

	// cast message to correct type
	MsgOpenImage* msg = (MsgOpenImage*) pContext->msg;

	// read image desc row of this image from the image descriptor
	// table based on image key 
    status = ImageDescRowRead(pContext, 
		REPLYCALLBACK(DdmUpgrade, OpenImageReplyHandler),
		msg->GetImageKey());
	
	// make sure everything went ok and return status
	assert (status == CTS_SUCCESS);
	return status;

 }
/* end DdmUpgrade::OpenImage  ***********************************************/

//  DdmUpgrade::OpenImageFile (CONTEXT* pContext)
//
//  Description:
//		Makes call to File System to open image file
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::OpenImageFile(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::OpenImageFile());

	m_State = FILE_OPENED_FROM_SYS;

	MsgOpenFile* openFileMsg = 
		new MsgOpenFile(pContext->pImageDescRec->fileDescKey);
	assert(openFileMsg);
	status = Send(openFileMsg, 
		pContext, 
		REPLYCALLBACK(DdmUpgrade, OpenImageReplyHandler));	
	
	// make sure everything went ok and return status
	assert (status == CTS_SUCCESS);
	return status;

 }
/* end DdmUpgrade::OpenImageFile  ***********************************************/

//  DdmUpgrade::OpenImageReplyHandler
//
//  Description:
//		State machine to control open image request message.
//
//  Inputs:
//		pReply_ - reply from request
//
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::OpenImageReplyHandler(Message* pReply_) 
{
	
	TRACE_ENTRY(DdmUpgrade::OpenImageReplyHandler());

	// make sure reply is valid and extract call back context
	assert (pReply_ != NULL);
	CONTEXT* pContext = (CONTEXT *) pReply_->GetContext();
	assert (pContext != NULL);

	if (pReply_->Status() != OK)
	{
		if (pReply_->Status() == CTS_FILESYS_FILE_NOT_FOUND
			&& m_State == FILE_OPENED_FROM_SYS)
		{
			Reply(pContext->msg, CTS_UPGRADE_IMAGE_NOT_FOUND);
			// delete the call back context
			ClearAndDeleteContext(pContext);
			ProcessNextCommand();
			return CTS_UPGRADE_IMAGE_NOT_FOUND;
		}
		else
		{
			if (pReply_->Status() == ercKeyNotFound &&
				m_State == IMAGE_DESC_ROW_READ)
			{
				Reply(pContext->msg, CTS_UPGRADE_IMAGE_NOT_FOUND);
				// delete the call back context
				ClearAndDeleteContext(pContext);
				ProcessNextCommand();
				return CTS_UPGRADE_IMAGE_NOT_FOUND;
			}
			else
				assert(0);
		}
	}

	// state change based on call back context state
	switch (m_State)
	{
	case IMAGE_DESC_ROW_READ:
		{
			// cast reply to correct type
			ImageDescRecord::RqReadRow* pReply = 
				(ImageDescRecord::RqReadRow*) pReply_;
			assert(pReply->GetRowCount()==1);
			// extract image desc row read from reply
			pContext->pImageDescRec = pReply->GetRowCopy();
			OpenImageFile(pContext);
			break;
		}
	case FILE_OPENED_FROM_SYS:
		{
			MsgOpenFile* pReply = (MsgOpenFile*) pReply_;
			MsgOpenImage* msg = (MsgOpenImage*) pContext->msg;
			Entry* pEntry;
			void* pImage;
			U32 cbImage;
			pReply->GetFile(&pEntry);
			cbImage = pEntry->GetFile(&pImage);
			msg->SetImage(pImage, cbImage);
			Reply(msg, OK);
			// delete call back context
			ClearAndDeleteContext(pContext);
			ProcessNextCommand();
			break;
		}
	default:
		assert(0);
		break;
	}

	// delete reply 
	delete pReply_;

	// return status
	return CTS_SUCCESS;
}
/* end DdmUpgrade::OpenImageReplyHandler  ********************************/

//  DdmUpgrade::AssignDefaultImage (CONTEXT* pContext)
//
//  Description:
//		Entry point for assign default image request
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::AssignDefaultImage(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::AssignDefaultImage());

	// cast image to the correct type and extract type and imageKey
	AssignDefaultCmdInfo* pCmdInfo = (AssignDefaultCmdInfo*) pContext->pCmdInfo;

	status = ImageDescRowRead(pContext,
		REPLYCALLBACK(DdmUpgrade, AssignDefaultImageReplyHandler),
		pCmdInfo->GetImageKey());

	// make sure everything went ok and return status
    assert (status == CTS_SUCCESS);
	return status;
}
/* end DdmUpgrade::AssignDefaultImage  ***********************************************/

//  DdmUpgrade::ModifyDefault (CONTEXT* pContext)
//
//  Description:
//		Entry point for assign default image request
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::ModifyDefault(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::ModifyDefault());

	// initialize state of call back context
	m_State = DEFAULT_IMAGE_FIELD_MODIFIED;

	ImageType type = pContext->pImageDescRec->type;
	RowId imageKey = pContext->pImageDescRec->rid;

	// allocate and initialize modify field request
	DefaultImageRecord::RqModifyField* preqModify;
	preqModify = new DefaultImageRecord::RqModifyField(CT_DEF_IMAGE_TABLE_TYPE,
		&type, sizeof(type), CT_DEF_IMAGE_TABLE_IMAGEKEY, &imageKey,
		sizeof(imageKey));
	assert(preqModify);

	// send modify field request
	status = Send (preqModify, pContext,
                   REPLYCALLBACK (DdmUpgrade, AssignDefaultImageReplyHandler));

	// make sure everything went ok and return status
    assert (status == CTS_SUCCESS);
	return status;
}
/* end DdmUpgrade::ModifyDefault  ***********************************************/

//  DdmUpgrade::AssignDefaultImageReplyHandler
//
//  Description:
//		State machine to control assign default images request message.
//
//  Inputs:
//		pReply_ - reply from request
//
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::AssignDefaultImageReplyHandler(Message* pReply_) 
{
	
	TRACE_ENTRY(DdmUpgrade::AssignDefaultImageReplyHandler());

	assert(pReply_->Status()==OK||pReply_->Status()==ercKeyNotFound);

	// make sure reply is valid and extract call back context
	assert (pReply_ != NULL);
	CONTEXT* pContext = (CONTEXT *) pReply_->GetContext();
	assert (pContext != NULL);

	AssignDefaultCmdInfo* pCmdInfo = (AssignDefaultCmdInfo*) pContext->pCmdInfo;

	switch (m_State)
	{
	case IMAGE_DESC_ROW_READ:
		if (pReply_->Status()==ercKeyNotFound)
		{
			AssignDefaultEvent* pEvt = 
				new AssignDefaultEvent(CTS_UPGRADE_IMAGE_NOT_FOUND,
				pCmdInfo->GetImageKey());
			void* pData;
			U32 cbCmdInfo;
			cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef  VAR_CMD_QUEUE
			m_pCmdServer->csrvReportCmdStatus(
				pContext->handle,
				CTS_UPGRADE_IMAGE_NOT_FOUND,
				pEvt,
				sizeof(AssignDefaultEvent),
				pData,
				cbCmdInfo);
#else
			m_pCmdServer->csrvReportCmdStatus(
				pContext->handle,
				CTS_UPGRADE_IMAGE_NOT_FOUND,
				pEvt,
				pData);
#endif
			delete pEvt;			
			delete pData;
			// delete the call back context
			ClearAndDeleteContext(pContext);
			ProcessNextCommand();
			return CTS_UPGRADE_IMAGE_NOT_FOUND;
		}
		else
		{
			ImageDescRecord::RqReadRow* pReply =
				(ImageDescRecord::RqReadRow*) pReply_;
			assert(pReply->GetRowCount()==1);
			pContext->pImageDescRec = pReply->GetRowCopy();
			ModifyDefault(pContext);
		}
		break;
	case DEFAULT_IMAGE_FIELD_MODIFIED:
		// reply to original message and delete call back context
		AssignDefaultEvent* pEvt = 
			new AssignDefaultEvent(OK,
			pCmdInfo->GetImageKey());
		void* pData;
		U32 cbCmdInfo;
		cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
		m_pCmdServer->csrvReportCmdStatus(
			pContext->handle,
			OK,
			pEvt,
			sizeof(AssignDefaultEvent),
			pData,
			cbCmdInfo);
		m_pCmdServer->csrvReportEvent(
			EVT_DEFAULT_IMAGE_ASSIGNED,
			pEvt,
			sizeof(AssignDefaultEvent));
#else
		m_pCmdServer->csrvReportCmdStatus(
			pContext->handle,
			OK,
			pEvt,
			pData);
		m_pCmdServer->csrvReportEvent(
			EVT_DEFAULT_IMAGE_ASSIGNED,
			pEvt);
#endif
		delete pEvt;			
		delete pData;
		ClearAndDeleteContext(pContext);
		ProcessNextCommand();
		break;
	}

	// delete reply
	if (pReply_)
		delete pReply_;

	// return status
	return CTS_SUCCESS;	// make sure reply is valid and extract call back context from reply

}
/* end DdmUpgrade::AssignDefaultImageReplyHandler  ********************************/

//  DdmUpgrade::ModifyDefaultImage (CONTEXT* pContext)
//
//  Description:
//		Entry point for a modify default image request
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::ModifyDefaultImage(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::ModifyDefaultImage());

	// cast message to correct type
	MsgModifyDefaultImage* msg = (MsgModifyDefaultImage*) pContext->msg;

	U32 cbImage = msg->GetImage(&pContext->pData);

	img_hdr_t* pImgHdr = (img_hdr_t*)pContext->pData;
	char* pImage = (char*) pContext->pData + pImgHdr->i_imageoffset;
	RowId imageKey = msg->GetImageKey();

	m_State = DEFAULT_IMAGE_FIELD_MODIFIED;

	// allocate and initialize modify field request
	DefaultImageRecord::RqModifyRow* preqModify;
	pContext->pDefaultImageRec = new DefaultImageRecord(
		pImgHdr->i_mjr_ver, 
		pImgHdr->i_mnr_ver, 
		pImgHdr->i_day, 
		pImgHdr->i_month, 
		pImgHdr->i_year,
		pImgHdr->i_hour, 
		pImgHdr->i_min, 
		pImgHdr->i_sec, 
		(ImageType)pImgHdr->i_type, 
		imageKey);

	preqModify = new DefaultImageRecord::RqModifyRow(CT_DEF_IMAGE_TABLE_IMAGEKEY, 
		&imageKey, sizeof(rowID), *pContext->pDefaultImageRec);
	assert(preqModify);

	// send modify field request
	status = Send (preqModify, pContext,
                   REPLYCALLBACK (DdmUpgrade, ModifyDefaultImageReplyHandler));

	return status;

 }
/* end DdmUpgrade::ModifyDefaultImage  ***********************************************/

//  DdmUpgrade::ModifyImageDescRow (CONTEXT* pContext)
//
//  Description:
//		Modify the image desc row
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::ModifyImageDescRow(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::ModifyImageDescRow());

	// cast message to correct type
	MsgModifyDefaultImage* msg = (MsgModifyDefaultImage*) pContext->msg;

	// initialize the call back state
	m_State = IMAGE_DESC_ROW_MODIFIED;

	img_hdr_t* pImgHdr = (img_hdr_t*)pContext->pData;

	// allocate and initialize new Image Desc Record
	pContext->pImageDescRec->majorVersion = pImgHdr->i_mjr_ver;
	pContext->pImageDescRec->minorVersion = pImgHdr->i_mnr_ver;
	pContext->pImageDescRec->day = pImgHdr->i_day;
	pContext->pImageDescRec->month = pImgHdr->i_month;
	pContext->pImageDescRec->year = pImgHdr->i_year;
	pContext->pImageDescRec->hour = pImgHdr->i_hour;
	pContext->pImageDescRec->minute = pImgHdr->i_min;
	pContext->pImageDescRec->second = pImgHdr->i_sec;
	pContext->pImageDescRec->fileDescKey = pContext->rid;
			
	// allocate and initialize the modify row request
	ImageDescRecord::RqModifyRow* preqModify;
	preqModify = new ImageDescRecord::RqModifyRow(
		pContext->pImageDescRec->rid, 
		*pContext->pImageDescRec);
	assert(preqModify);

	// send the modify row request
    status = Send (preqModify, pContext,
                   REPLYCALLBACK (DdmUpgrade, ModifyDefaultImageReplyHandler));

	// make sure everything went ok and return the status
    assert (status == CTS_SUCCESS);
	return status;
 }
/* end DdmUpgrade::ModifyImageDescRow  ***********************************************/

//  DdmUpgrade::ModifyDefaultImageReplyHandler
//
//  Description:
//		State machine to control modify default image request message.
//
//  Inputs:
//		pReply_ - reply from request
//
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::ModifyDefaultImageReplyHandler(Message* pReply_) 
{
	
	TRACE_ENTRY(DdmUpgrade::ModifyDefaultImageReplyHandler());

	// make sure reply is valid and extract call back context
	assert (pReply_ != NULL);
	CONTEXT* pContext = (CONTEXT *) pReply_->GetContext();
	assert (pContext != NULL);

	assert(pReply_->Status()==OK || pReply_->Status()==CTS_FILESYS_OUT_OF_MEMORY);

	MsgModifyDefaultImage* msg = (MsgModifyDefaultImage*)pContext->msg;

	// state change based on call back context state
	switch (m_State)
	{
	case DEFAULT_IMAGE_FIELD_MODIFIED:
		ImageDescRowRead(pContext,
			REPLYCALLBACK(DdmUpgrade, ModifyDefaultImageReplyHandler),
			msg->GetImageKey());
		break;
	case IMAGE_DESC_ROW_READ:
		{
			ImageDescRecord::RqReadRow* pReply =
				(ImageDescRecord::RqReadRow*) pReply_;
			assert(pReply->GetRowCount() == 1);
			pContext->pImageDescRec = pReply->GetRowCopy();

			m_State = FILE_DELETED_FROM_SYS;

			// allocate and initialize file system delete file message
			MsgDeleteFile* deleteFileMsg = new MsgDeleteFile(
				pContext->pImageDescRec->fileDescKey);
			assert(deleteFileMsg);

			// send delete file request
			return Send(deleteFileMsg, 
				pContext, 
				REPLYCALLBACK(DdmUpgrade, ModifyDefaultImageReplyHandler));	

			break;
		}
	case FILE_DELETED_FROM_SYS:
		{
			m_State = FILE_ADDED_TO_SYS;

			img_hdr_t* pImgHdr = (img_hdr_t*)pContext->pData;

			UnicodeString us(StringClass((char*)&pImgHdr->i_imagename));
			UnicodeString16 imageName;
			us.CString(imageName, sizeof(imageName));

			// allocate and initialize a file system add file request message
			MsgAddFile* addFileMsg = new MsgAddFile(
				msg->GetImageSize(),
				pContext->pData,
				IMAGE_TYPE,
				imageName);
			assert(addFileMsg);
	
			// send add file request to file system master
			return Send(addFileMsg, 
				pContext, 
				REPLYCALLBACK(DdmUpgrade, ModifyDefaultImageReplyHandler));	


			break;
		}
	case FILE_ADDED_TO_SYS:
		{
			// cast add file message to correct type
			MsgAddFile* pReply = (MsgAddFile*) pReply_;	

			if (pReply->Status()==CTS_FILESYS_OUT_OF_MEMORY)
			{
				Reply(pContext->msg, CTS_FILESYS_OUT_OF_MEMORY);
				ClearAndDeleteContext(pContext);
				ProcessNextCommand();
			}
			else
			{
				pContext->rid = pReply->GetFileKey();
				ModifyImageDescRow(pContext);
			}

			break;
		}
	case IMAGE_DESC_ROW_MODIFIED:
		{
			Reply(msg, OK);
			ClearAndDeleteContext(pContext);
			ProcessNextCommand();
			break;
		}
	default:
		assert(0);
		break;
	}

	// delete reply 
	delete pReply_;

	// return status
	return CTS_SUCCESS;
}
/* end DdmUpgrade::ModifyDefaultImageReplyHandler  ********************************/

//  DdmUpgrade::GetFileSysInfo (CONTEXT* pContext)
//
//  Description:
//		Entry point for get file system info request
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::GetFileSysInfo(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::GetFileSysInfo());

	// allocate and initialize get file system info request
	MsgGetSysInfo* getSysInfoMsg = new MsgGetSysInfo();
	assert(getSysInfoMsg);

	// send file system info request to file system
	status = Send (getSysInfoMsg, pContext,
                   REPLYCALLBACK (DdmUpgrade, GetFileSysInfoReplyHandler));

	// make sure everything went ok and return status
    assert (status == CTS_SUCCESS);
	return status;

}
/* end DdmUpgrade::GetFileSysInfo  ***********************************************/

//  DdmUpgrade::GetFileSysInfoReplyHandler
//
//  Description:
//		State machine to control get file system info request message.
//
//  Inputs:
//		pReply_ - reply from request
//
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::GetFileSysInfoReplyHandler(Message* pReply_) 
{
	
	TRACE_ENTRY(DdmUpgrade::GetFileSysInfoReplyHandler());

	assert(pReply_->Status()==OK);

	// make sure reply is valid and extract call back context
	assert (pReply_ != NULL);
	MsgGetSysInfo* pReply = (MsgGetSysInfo*) pReply_;
	CONTEXT* pContext = (CONTEXT *) pReply_->GetContext();
	assert (pContext != NULL);

	// cast message to correct type and extract SystemInfo from reply
	GetFileSysCmdInfo* pCmdInfo = (GetFileSysCmdInfo*) pContext->pCmdInfo;
	SystemInfo* pSysInfo;
	pReply->GetSysInfo(&pSysInfo);

	// reply with message and delete call back context
	GetFileSysInfoEvent* pEvt = 
		new GetFileSysInfoEvent(OK,
		pSysInfo->GetUsedSpace(),
		pSysInfo->GetFileSystemSize());
	void* pData;
	U32 cbCmdInfo;
	cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
	m_pCmdServer->csrvReportCmdStatus(
		pContext->handle,
		OK,
		pEvt,
		sizeof(GetFileSysInfoEvent),
		pData,
		cbCmdInfo);
	m_pCmdServer->csrvReportEvent(
		EVT_FILESYS_INFO_GOTTEN,
		pEvt,
		sizeof(GetFileSysInfoEvent));
#else
	m_pCmdServer->csrvReportCmdStatus(
		pContext->handle,
		OK,
		pEvt,
		pData);
	m_pCmdServer->csrvReportEvent(
		EVT_FILESYS_INFO_GOTTEN,
		pEvt);
#endif
	delete pEvt;		
	delete pData;
	ClearAndDeleteContext(pContext);
	ProcessNextCommand();

	delete pSysInfo;

	// delete reply
	delete pReply;

	// return status
	return CTS_SUCCESS;

}
/* end DdmUpgrade::GetFileSysInfoReplyHandler  ********************************/

//  DdmUpgrade::BootImage (CONTEXT* pContext)
//
//  Description:
//		Entry point for boot image request
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::BootImage(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::BootImage());

	m_State = IOP_STATUS_ROW_READ;

	// cast message to correct type
	BootImageCmdInfo* pCmd = (BootImageCmdInfo*) pContext->pCmdInfo;
	pContext->slot = pCmd->GetSlot();

	// check if image is already out of service
	IOPStatusRecord::RqReadRow* preqRead;
	preqRead = new IOPStatusRecord::RqReadRow(CT_IOPST_SLOT,
		&pContext->slot, sizeof(TySlot));
	assert(preqRead);

	// send read row request
	status = Send (preqRead, pContext, 
		REPLYCALLBACK(DdmUpgrade, BootImageReplyHandler));

	// return status
	return status;

}
/* end DdmUpgrade::BootImage  ***********************************************/

//  DdmUpgrade::BootNewImage
//
//  Description:
//		Calls Boot Manager to boot image on slot
//
//  Inputs:
//		pReply_ - reply from request
//
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::BootNewImage(CONTEXT* pContext)
{
	TRACE_ENTRY(DdmUpgrade::BootImage());

	BootImageCmdInfo* pCmdInfo = (BootImageCmdInfo*) pContext->pCmdInfo;

	m_State = IMAGE_BOOTED;

	// call boot manager to boot image on board

	// return status
	return CTS_SUCCESS;
}
/* end DdmUpgrade::BootNewImage  ***********************************************/

//  DdmUpgrade::BootImageReplyHandler
//
//  Description:
//		State machine to control boot image request message.
//
//  Inputs:
//		pReply_ - reply from request
//
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::BootImageReplyHandler(Message* pReply_) 
{
	TRACE_ENTRY(DdmUpgrade::BootImageReplyHandler());

	assert(pReply_->Status()==OK);

	// make sure reply is valid and extract call back context
	assert (pReply_ != NULL);
	CONTEXT* pContext = (CONTEXT *) pReply_->GetContext();
	assert (pContext != NULL);

	BootImageCmdInfo* pCmdInfo = (BootImageCmdInfo*)pContext->pCmdInfo;

	// state change based on call back context state
	switch (m_State)
	{
	case IOP_STATUS_ROW_READ:
		{
		IOPStatusRecord::RqReadRow* pReply = 
			(IOPStatusRecord::RqReadRow*) pReply_;
		// make sure only one row was read
		assert(pReply->GetRowCount() == 1);
		pContext->pIOPStatusRec = pReply->GetRowCopy();
		IopImageRowRead(pContext,
			REPLYCALLBACK(DdmUpgrade, AssociateImageReplyHandler),
			&pContext->slot);
		break;
		}
	case IOP_IMAGE_ROW_READ:
		{
		IOPImageRecord::RqReadRow* pReply = (IOPImageRecord::RqReadRow*) pReply_;
		pContext->pIOPImageRec = pReply->GetRowCopy();
		BootImageCmdInfo* pCmdInfo = (BootImageCmdInfo*) pContext->pCmdInfo;
		if (pContext->pIOPImageRec->currentImage == pCmdInfo->GetImageKey())
		{
			// reply.  we have already booted the image
			BootImageEvent* pEvt = 
				new BootImageEvent(OK,
				pCmdInfo->GetImageKey(),
				pCmdInfo->GetSlot());
			void* pData;
			U32 cbCmdInfo;
			cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
			m_pCmdServer->csrvReportCmdStatus(
				pContext->handle,
				OK,
				pEvt,
				sizeof(BootImageEvent),
				pData,
				cbCmdInfo);
			m_pCmdServer->csrvReportEvent(
				EVT_IMAGE_BOOTED,
				pEvt,
				sizeof(BootImageEvent));
#else
			m_pCmdServer->csrvReportCmdStatus(
				pContext->handle,
				OK,
				pEvt,
				pData);
			m_pCmdServer->csrvReportEvent(
				EVT_IMAGE_BOOTED,
				pEvt);
#endif
			delete pEvt;			
			delete pData;
			ClearAndDeleteContext(pContext);
			ProcessNextCommand();
		}
		if (pContext->pIOPStatusRec->eIOPCurrentState == IOPS_POWERED_DOWN)
			BootNewImage(pContext);
		else
		{
			// power the board down here
			m_State = BOARD_FAILED_OVER;
			MsgIopOutOfService* pMsg = new MsgIopOutOfService(pCmdInfo->GetSlot());
			assert(pMsg);
			Send(pMsg, pContext, 
				REPLYCALLBACK(DdmUpgrade, BootImageReplyHandler));
		}
		break;
		}
	case BOARD_FAILED_OVER:
		BootNewImage(pContext);
		break;
	case IMAGE_BOOTED:
		{
			// reply to original message and delete call back context
			BootImageEvent* pEvt = 
				new BootImageEvent(OK,
				pCmdInfo->GetImageKey(),
				pCmdInfo->GetSlot());
			void* pData;
			U32 cbCmdInfo;
			cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
			m_pCmdServer->csrvReportCmdStatus(
				pContext->handle,
				OK,
				pEvt,
				sizeof(BootImageEvent),
				pData,
				cbCmdInfo);
			m_pCmdServer->csrvReportEvent(
				EVT_IMAGE_BOOTED,
				pEvt,
				sizeof(BootImageEvent));
#else
			m_pCmdServer->csrvReportCmdStatus(
				pContext->handle,
				OK,
				pEvt,
				pData);
			m_pCmdServer->csrvReportEvent(
				EVT_IMAGE_BOOTED,
				pEvt);
#endif
			delete pEvt;			
			delete pData;
			ClearAndDeleteContext(pContext);
			ProcessNextCommand();
			break;
		}
	default:
		assert(0);
		break;
	}

	// delete reply
	if (pReply_)
		delete pReply_;

	// return status
	return CTS_SUCCESS;

}
/* end DdmUpgrade::BootImageReplyHandler  ********************************/

//  DdmUpgrade::MakePrimary (CONTEXT* pContext)
//
//  Description:
//		Entry point for make primary request
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::MakePrimary(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::MakePrimary());

	// cast command to correct type
	if (pContext->msg)
	{
		MsgMakePrimary* pMsg = (MsgMakePrimary*)pContext->msg;
		pContext->slot = pMsg->GetSlot();
	}
	else
	{
		MakePrimaryCmdInfo* pCmdInfo = (MakePrimaryCmdInfo*) pContext->pCmdInfo;
		pContext->slot = pCmdInfo->GetSlot();
	}

	// read iop image from iop image table
	status = IopImageRowRead(pContext,
		REPLYCALLBACK(DdmUpgrade, MakePrimaryReplyHandler),
		&pContext->slot);

	// return status
	return status;

}
/* end DdmUpgrade::MakePrimary  ***********************************************/

//  DdmUpgrade::MakeImagePrimary (CONTEXT* pContext)
//
//  Description:
//		Modify Iop Image row to make image primary
//
//  Inputs:
//		pContext - call back context
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmUpgrade::MakeImagePrimary(CONTEXT *pContext) 
{
	STATUS status;

	TRACE_ENTRY(DdmUpgrade::MakeImagePrimary());

	// initialize state of call back context
	m_State = IOP_IMAGE_ROW_MODIFIED;

	// cast message to correct type and extract slot and imageKey
	TySlot slot;
	RowId imageKey;
	MsgMakePrimary* pMsg = NULL;
	MakePrimaryCmdInfo* pCmdInfo = NULL;
	if (pContext->msg)
	{
		pMsg = (MsgMakePrimary*) pContext->msg;
		slot = pMsg->GetSlot();
		imageKey = pMsg->GetImageKey();
	}
	else
	{
		pCmdInfo = (MakePrimaryCmdInfo*) pContext->pCmdInfo;
	 	slot = pCmdInfo->GetSlot();
		imageKey = pCmdInfo->GetImageKey();
	}

	// modify Iop Image row
	if (pContext->pIOPImageRec->imageOne == imageKey)
		pContext->pIOPImageRec->imageOneAccepted = TRUE;
	else
		if (pContext->pIOPImageRec->imageTwo == imageKey)
			pContext->pIOPImageRec->imageTwoAccepted = TRUE;
		else
		{
			if (pMsg)
				Reply(pMsg, CTS_UPGRADE_IMAGE_NOT_ASSOCIATED_WITH_SLOT);
			else
			{
				MakePrimaryEvent* pEvt = 
					new MakePrimaryEvent(CTS_UPGRADE_IMAGE_NOT_ASSOCIATED_WITH_SLOT,
					pCmdInfo->GetImageKey(), pCmdInfo->GetSlot());
				void* pData;
				U32 cbCmdInfo;
				cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
				m_pCmdServer->csrvReportCmdStatus(
					pContext->handle,
					CTS_UPGRADE_IMAGE_NOT_ASSOCIATED_WITH_SLOT,
					pEvt,
					sizeof(MakePrimaryEvent),
					pData,
					cbCmdInfo);
#else
				m_pCmdServer->csrvReportCmdStatus(
					pContext->handle,
					CTS_UPGRADE_IMAGE_NOT_ASSOCIATED_WITH_SLOT,
					pEvt,
					pData);
#endif
				delete pEvt;	
				delete pData;
			}
			ClearAndDeleteContext(pContext);
			ProcessNextCommand();
			return CTS_UPGRADE_IMAGE_NOT_ASSOCIATED_WITH_SLOT;
		}
	pContext->pIOPImageRec->primaryImage = imageKey;

	// allocate and initialize modify row request
	IOPImageRecord::RqModifyRow* preqModify;
	preqModify = new IOPImageRecord::RqModifyRow(CT_IOP_IMAGE_TABLE_SLOT,
		&slot, sizeof(slot), *pContext->pIOPImageRec);
	assert(preqModify);

	// send modify row request
	status = Send (preqModify, pContext,
                   REPLYCALLBACK (DdmUpgrade, MakePrimaryReplyHandler));

	// make sure everything went ok and return status
    assert (status == CTS_SUCCESS);
	return status;

}
/* end DdmUpgrade::MakeImagePrimary  ***********************************************/

//  DdmUpgrade::MakePrimaryReplyHandler
//
//  Description:
//		State machine to control make primary request message.
//
//  Inputs:
//		pReply_ - reply from request
//
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmUpgrade::MakePrimaryReplyHandler(Message* pReply_) 
{
	
	TRACE_ENTRY(DdmUpgrade::MakePrimaryReplyHandler());

	assert(pReply_->Status() == OK||pReply_->Status()==ercKeyNotFound);

	// make sure reply is valid and extract call back context
	assert (pReply_ != NULL);
	CONTEXT* pContext = (CONTEXT *) pReply_->GetContext();
	assert (pContext != NULL);

	// cast message to the correct type
	MakePrimaryCmdInfo* pCmdInfo = NULL;
	MsgMakePrimary* pMsg = NULL;
	if (pContext->msg)
		pMsg = (MsgMakePrimary*) pContext->msg;
	else
		pCmdInfo = (MakePrimaryCmdInfo*) pContext->pCmdInfo;

	// state change based on call back context state
	switch (m_State)
	{
	case IOP_IMAGE_ROW_READ:
		{
			// cast reply to the correct type and extract row read
			IOPImageRecord::RqReadRow* pReply = (IOPImageRecord::RqReadRow*) pReply_;
			// make sure only one row was read
			assert(pReply->GetRowCount() == 1);
			pContext->pIOPImageRec = pReply->GetRowCopy();
			if (pMsg)
				ImageDescRowRead(pContext,
					REPLYCALLBACK(DdmUpgrade, MakePrimaryReplyHandler),
					pMsg->GetImageKey());
			else
				ImageDescRowRead(pContext,
					REPLYCALLBACK(DdmUpgrade, MakePrimaryReplyHandler),
					pCmdInfo->GetImageKey());
			break;
		}
	case IMAGE_DESC_ROW_READ:
		{
			ImageDescRecord::RqReadRow* pReply = (ImageDescRecord::RqReadRow*)pReply_;
			if (pReply_->Status()==ercKeyNotFound)
			{
				if (pMsg)
					Reply(pMsg, CTS_UPGRADE_IMAGE_NOT_FOUND);
				else
				{
					MakePrimaryEvent* pEvt = 
						new MakePrimaryEvent(CTS_UPGRADE_IMAGE_NOT_FOUND,
						pCmdInfo->GetImageKey(), pCmdInfo->GetSlot());
					void* pData;
					U32 cbCmdInfo;
					cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
					m_pCmdServer->csrvReportCmdStatus(
						pContext->handle,
						CTS_UPGRADE_IMAGE_NOT_ASSOCIATED_WITH_SLOT,
						pEvt,
						sizeof(MakePrimaryEvent),
						pData,
						cbCmdInfo);
#else
					m_pCmdServer->csrvReportCmdStatus(
						pContext->handle,
						CTS_UPGRADE_IMAGE_NOT_ASSOCIATED_WITH_SLOT,
						pEvt,
						pData);
#endif
					delete pEvt;					
					delete pData;
				}
				// delete the call back context
				ClearAndDeleteContext(pContext);
				ProcessNextCommand();
				return CTS_UPGRADE_IMAGE_NOT_FOUND;
			}
			assert(pReply->GetRowCount()==1);
			// call method to make image primary
			MakeImagePrimary(pContext);
			break;
		}
	case IOP_IMAGE_ROW_MODIFIED:
		{
			if (pMsg)
				Reply(pMsg, OK);
			else
			{
				// reply to original message and delete call back context
				MakePrimaryEvent* pEvt = 
					new MakePrimaryEvent(OK,
					pCmdInfo->GetImageKey(), pCmdInfo->GetSlot());
				void* pData;
				U32 cbCmdInfo;
				cbCmdInfo = pCmdInfo->WriteAsStruct(&pData);
#ifdef VAR_CMD_QUEUE
				m_pCmdServer->csrvReportCmdStatus(
					pContext->handle,
					OK,
					pEvt,
					sizeof(MakePrimaryEvent),
					pData,
					cbCmdInfo);
				m_pCmdServer->csrvReportEvent(
					EVT_MADE_PRIMARY,
					pEvt,
					sizeof(MakePrimaryEvent));
#else	
				m_pCmdServer->csrvReportCmdStatus(
					pContext->handle,
					OK,
					pEvt,
					pData);
				m_pCmdServer->csrvReportEvent(
					EVT_MADE_PRIMARY,
					pEvt);
#endif	
				delete pEvt;			
				delete pData;
			}
			ClearAndDeleteContext(pContext);
			ProcessNextCommand();
			break;
		}
	default:
		assert(0);
		break;
	}

	// delete reply
	if (pReply_)
		delete pReply_;

	// return status
	return CTS_SUCCESS;

}
/* end DdmUpgrade::MakePrimaryReplyHandler  ********************************/

//  DdmUpgrade::SubmitRequest
//
//  Description:
//		This function either enqueues a new function call
//		entry or makes the call if the queue is presently
//		empty and the UpgradeMaster is not processing a command
//  Inputs:
//	  U32 functionToCall - enumerated value of function to call
//    void* pContext - information passed from the calling DdmUpgrade
//      method
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
void DdmUpgrade::SubmitRequest(U32 functionToCall, CONTEXT* pContext)
{
	if (m_pUpgradeQueue->Empty() && NotProcessingCommand())
		ExecuteFunction(functionToCall, pContext);
	else
		m_pUpgradeQueue->AddFunctionCall(functionToCall, ((void*)pContext));
}
/* end DdmUpgrade::SubmitRequest  ********************************/

//  DdmUpgrade::ExecuteFunction
//
//  Description:
//		Sets the ProcessingCommand flag in the Upgrade Master and
//		calls the appropriate function
//
//  Inputs:
//    U32 functionToCall - enumerated value which specifies 
//		function to call
//    void* _pContext - information passed from the calling DdmUpgrade
//      method
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
void DdmUpgrade::ExecuteFunction(U32 functionToCall, CONTEXT* pContext)
{
	ProcessCommand();
	switch (functionToCall)
	{
	case function_AddImage:
		AddImage(pContext);
		break;
	case function_OpenImage:
		OpenImage(pContext);
		break;
	case function_DeleteImage:
		DeleteImage(pContext);
		break;
	case function_AssociateImage:
		AssociateImage(pContext);
		break;
	case function_UnassociateImage:
		UnassociateImage(pContext);
		break;
	case function_QueryImages:
		QueryImages(pContext);
		break;
	case function_AssignDefaultImage:
		AssignDefaultImage(pContext);
		break;
	case function_GetFileSysInfo:
		GetFileSysInfo(pContext);
		break;
	case function_BootImage:
		BootImage(pContext);
		break;
	case function_MakePrimary:
		MakePrimary(pContext);
		break;
	case function_ModifyDefaultImage:
		ModifyDefaultImage(pContext);
		break;
	default:
		assert(0);
		break;
	}
}
/* end DdmUpgrade::ExecuteFunction  ********************************/

//  DdmUpgrade::ProcessNextCommand
//
//  Description:
//		Checks the queue for next function call and executes
//		or sets the Upgrade Master to a non-busy state if there
//		are no more commands to process
//
void DdmUpgrade::ProcessNextCommand()
{
	FunctionCallElement* pElement;

	pElement = m_pUpgradeQueue->PopFunctionCall();

	if (pElement)
	{
		ExecuteFunction(pElement->GetFunction(), (CONTEXT*)pElement->GetContext());
		delete pElement;
	}
	else
		FinishCommand();
}
/* end DdmUpgrade::ProcessNextCommand  ********************************/

