/* FileSystem.cpp
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
// $Log: /Gemini/Odyssey/FileSystem/FileSystem.cpp $  
// 
// 14    2/14/00 8:51a Joehler
// Added error code for file too large as part of DFCT13023
// 
// 13    1/31/00 4:17p Joehler
// Changes to correctly handle a persistent boot.
// 
// 12    1/26/00 2:37p Joehler
// copy creationDate to avoid alignment problem.
// 
// 11    12/09/99 1:48a Iowa
// 
// 10    11/17/99 3:24p Joehler
// Cast reply prior to deletion
// 
// 9     11/02/99 6:33p Joehler
// Added File System variable PTS field interface
// 
// 8     10/18/99 5:32p Joehler
// Variable PTS interface not checked in yet.
// 
// 7     10/15/99 2:01p Joehler
// Modified code to use Tom's GetRowCopy() from PTS reply
// 
// 6     10/15/99 11:01a Joehler
// Changed error codes from ERC_UPGRADE_* to CTS_UPGRADE_* and moved to
// *.mc files to localize
// 
// 5     10/12/99 11:23a Joehler
// Oops!  Variable interface not released yet.
// 
// 4     10/12/99 10:19a Joehler
// Modifications for variable pts table entries
// 
// 3     10/07/99 9:57a Joehler
// changes for variable sized pts rows
// 
// 2     10/06/99 4:21p Joehler
// added error checking
// 
// 1     9/30/99 7:46a Joehler
// First cut of File System Master
// 
// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF

#include "CtEvent.h"

#include "FileSystem.h"
#include "BuildSys.h"
#include "FileSystemInfo.h"

// temp vpts
#include "Rows.h"

// BuildSys linkage
	
CLASSNAME(DdmFileSystem,SINGLE);

SERVELOCAL(DdmFileSystem, REQ_FILESYS_ADD_FILE);
SERVELOCAL(DdmFileSystem, REQ_FILESYS_DELETE_FILE);
SERVELOCAL(DdmFileSystem, REQ_FILESYS_LIST_FILES);
SERVELOCAL(DdmFileSystem, REQ_FILESYS_OPEN_FILE);
SERVELOCAL(DdmFileSystem, REQ_FILESYS_GET_SYSINFO);

//  DdmFileSystem::Ctor (did)
//	  (static)
//
//  Description:
//    This routine is called by CHAOS when it wants to create
//    an instance of DdmFileSystem.
//
//  Inputs:
//    did - CHAOS "Device ID" of the new instance we are to create.
//
//  Outputs:
//    DdmFileSystem::Ctor - Returns a pointer to the new instance, or NULL.
//

Ddm *DdmFileSystem::Ctor (DID did)
{
	TRACE_ENTRY(DdmFileSystem::Ctor(DID did));
	return (new DdmFileSystem (did));
} 
/*	end DdmFileSystem::Ctor  *****************************************************/

//  DdmFileSystem::Initialize (pMsg)
//    (virtual)
//
//  Description:
//    Called for a DDM instance when the instance is being created.
//    This routine is called after the DDM's constructor, but before
//    DdmFileSystem::Enable().  Please note that this DDM is not completely
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
//    DdmFileSystem::Initialize - Returns OK if all is cool, else an error.
//

STATUS DdmFileSystem::Initialize (Message *pMsg)
{
	STATUS status;

	TRACE_ENTRY(DdmFileSystem::Initialize(Message*));

	DispatchRequest(REQ_FILESYS_ADD_FILE, REQUESTCALLBACK (DdmFileSystem, ProcessAddFile));
	DispatchRequest(REQ_FILESYS_DELETE_FILE, REQUESTCALLBACK (DdmFileSystem, ProcessDeleteFile));
	DispatchRequest(REQ_FILESYS_LIST_FILES, REQUESTCALLBACK (DdmFileSystem, ProcessListFiles));
	DispatchRequest(REQ_FILESYS_OPEN_FILE, REQUESTCALLBACK (DdmFileSystem, ProcessOpenFile));
	DispatchRequest(REQ_FILESYS_GET_SYSINFO, REQUESTCALLBACK (DdmFileSystem, ProcessGetSysInfo));

	m_Initialized = false;

	m_pFileSystemQueue = new CommandProcessingQueue;

	assert(m_pFileSystemQueue);

	processingCommand = FALSE;

	usedSpace = 0;

	// allocate CONTEXT to define tables
	CONTEXT* pContext = new (tZERO) CONTEXT;
	assert(pContext);
	pContext->msg = pMsg;
	
	status = DefineFileDescTable(pContext);

	return OK;
		
}
/*	end DdmFileSystem::Initialize  *****************************************************/

//  DdmFileSystem::Enable (pMsgReq)
//
//  Description:
//   Lets go!

STATUS DdmFileSystem::Enable(Message *pMsgReq)
{
	TRACE_ENTRY(DdmFileSystem::Enable());

	Reply(pMsgReq,OK);
	return OK;
		
}
/* end DdmFileSystem::Enable  *****************************************************/

//  DdmFileSystem::ProcessAddFile (pMsg)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//

STATUS DdmFileSystem::ProcessAddFile(Message *pMsgReq) 
{

	STATUS status = OK;
	TRACE_ENTRY(DdmFileSystem::ProcessAddFile());
	
	// allocate and initialize context
	CONTEXT* pContext = new(tZERO) CONTEXT;

	pContext->msg = pMsgReq;
	
	SubmitRequest(function_AddFile, pContext);

	return status;
}
/* end DdmFileSystem::ProcessAddFile  ***********************************************/

//  DdmFileSystem::ProcessDeleteFile
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::ProcessDeleteFile(Message *pMsgReq)
{

	STATUS status = OK;

	TRACE_ENTRY(DdmFileSystem::ProcessDeleteFile());

	// allocate and initialize context
	CONTEXT* pContext = new(tZERO) CONTEXT;

	pContext->msg = pMsgReq;
	
	SubmitRequest(function_DeleteFile, pContext);

	return status;
}
/* end DdmFileSystem::ProcessDeleteFile  ***********************************************/

//  DdmFileSystem::ProcessListFiles(pMsg)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::ProcessListFiles(Message *pMsgReq) 
{
	STATUS		status = OK;

	TRACE_ENTRY(DdmFileSystem::ProcessListFiles());

	// allocate and initialize context
	CONTEXT* pContext = new(tZERO) CONTEXT;

	pContext->msg = pMsgReq;
	
	SubmitRequest(function_ListFiles, pContext);

	return status;
}
/* end DdmFileSystem::ProcessListFiles  ***********************************************/

//  DdmFileSystem::ProcessOpenFile (pMsg)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::ProcessOpenFile(Message *pMsgReq) 
{
	STATUS		status = OK;

	TRACE_ENTRY(DdmFileSystem::ProcessOpenFile());

	// allocate and initialize context
	CONTEXT* pContext = new(tZERO) CONTEXT;

	pContext->msg = pMsgReq;
	
	SubmitRequest(function_OpenFiles, pContext);

	return status;
}
/* end DdmFileSystem::ProcessOpenFile  ***********************************************/

//  DdmFileSystem::ProcessGetSysInfo (pMsg)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::ProcessGetSysInfo(Message *pMsgReq) 
{
	STATUS		status = OK;

	TRACE_ENTRY(DdmFileSystem::ProcessGetSysInfo());

	// allocate and initialize context
	CONTEXT* pContext = new(tZERO) CONTEXT;

	pContext->msg = pMsgReq;
	
	SubmitRequest(function_GetSysInfo, pContext);

	return status;
}
/* end DdmFileSystem::ProcessGetSysInfo **********************************************/

//****************************
// PRIVATE METHODS BEGIN HERE
//****************************

//  DdmFileSystem::DefineFileDescTable (CONTEXT* pContext)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::DefineFileDescTable(CONTEXT* pContext)
{
	STATUS status;

	TRACE_ENTRY(DdmFileSystem::DefineFileDescTable());

	pContext->state = FILE_DESC_TABLE_DEFINED;

	FileDescRecord::RqDefineTable * preqDefTab;

	//  attempt table definition, in case we're dealing with a fresh PTS
	preqDefTab = new FileDescRecord::RqDefineTable();
	assert(preqDefTab);

	status = Send(preqDefTab, 
		pContext,
		REPLYCALLBACK (DdmFileSystem, InitReplyHandler));

	assert (status == CTS_SUCCESS);

	return status;
}
/* end DdmFileSystem::DefineFileDescTable  ******************************************/

//  DdmFileSystem::EnumFileDescTable (CONTEXT* pContext)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::EnumFileDescTable(CONTEXT* pContext)
{
	STATUS status;

	TRACE_ENTRY(DdmFileSystem::EnumFileDescTable());

	// enumerate rows in File Desc Table
	pContext->state = FILE_DESC_TABLE_ENUM;

	FileDescRecord::RqEnumTable* preqEnum;
	preqEnum = new FileDescRecord::RqEnumTable();
	assert(preqEnum);

	status = Send (preqEnum, pContext,
               REPLYCALLBACK (DdmFileSystem, InitReplyHandler));

	assert (status == CTS_SUCCESS);

	return status;
}
/* end DdmFileSystem::EnumFileDescTable  ******************************************/

//  DdmFileSystem::DefineFileTable (CONTEXT* pContext)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::DefineFileTable(CONTEXT* pContext)
{
	STATUS status;
	
	TRACE_ENTRY(DdmFileSystem::DefineFileTable());

	pContext->state = FILE_TABLE_DEFINED;

	FileRecord::RqDefineTable * preqDefTab;

	//  attempt table definition, in case we're dealing with a fresh PTS
	preqDefTab = new FileRecord::RqDefineTable();
	assert(preqDefTab);

	status = Send(preqDefTab, 
		pContext,
		REPLYCALLBACK (DdmFileSystem, InitReplyHandler));

	assert (status == CTS_SUCCESS);

	return status;
}
/* end DdmFileSystem::DefineFileTable  ******************************************/

//  DdmFileSystem::InitReplyHandler (Message* pReply)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::InitReplyHandler(Message *pReply_)
{

	TRACE_ENTRY(DdmFileSystem::InitReplyHandler());

	assert (pReply_ != NULL);
	assert(pReply_->Status()==OK || pReply_->Status() == ercTableExists);
	CONTEXT* pContext = (CONTEXT *) pReply_->GetContext();
	assert (pContext != NULL);

	switch (pContext->state)
	{
	case FILE_DESC_TABLE_DEFINED:
		EnumFileDescTable(pContext);
		break;
	case FILE_DESC_TABLE_ENUM:
		{
			FileDescRecord::RqEnumTable* pReply = (FileDescRecord::RqEnumTable*) pReply_;
			FileDescRecord* pFileDescRecords = pReply->GetRowPtr();
			for (U16 i = 0; i < pReply->GetRowCount(); i++)
				usedSpace += pFileDescRecords[i].cbFile;
			DefineFileTable(pContext);
			break;
		}
	case FILE_TABLE_DEFINED:
		Reply(pContext->msg, OK);
		ClearAndDeleteContext(pContext);
		ProcessNextCommand();
		break;
	default:
		assert(0);
		break;
	}

	delete pReply_;

	return CTS_SUCCESS;
}
/* end DdmFileSystem::InitReplyHandler  ******************************************/

//  DdmFileSystem::AddFile (CONTEXT* pContext)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::AddFile(CONTEXT *pContext) 
{
	STATUS		status;

	TRACE_ENTRY(DdmFileSystem::AddFile());

	pContext->state = FILE_ROW_INSERTED;

	MsgAddFile* msg = (MsgAddFile*) pContext->msg;
	U8* pFile;
	U32 cbFile = msg->GetFileSize();

	msg->GetFile((void**)&pFile);

	pContext->pFileRec = new FileRecord(pFile, cbFile);
	assert(pContext->pFileRec);

	if (usedSpace + cbFile > TOTAL_FILE_SYSTEM_SIZE)
	{
		Reply(pContext->msg, CTS_FILESYS_OUT_OF_MEMORY);
		ClearAndDeleteContext(pContext);
		ProcessNextCommand();
		return CTS_FILESYS_OUT_OF_MEMORY;
	}
	else 
		usedSpace += cbFile;
// temp vpts

	/*FileRecord::RqInsertRow* preqInsert;

	preqInsert = new FileRecord::RqInsertRow(pContext->pFileRec, 1);
	assert(preqInsert);

    status = Send (preqInsert, pContext,
                   REPLYCALLBACK (DdmFileSystem, AddFileReplyHandler));*/

	TSInsertVLRow			*pInsertRow = NULL;

	pInsertRow = new TSInsertVLRow;

	status = pInsertRow->Initialize(
		this,
		CT_FILE_TABLE_NAME,
		pContext->pFileRec,
		&pContext->rid,
		(pTSCallback_t)&DdmFileSystem::TempAddFileReplyHandler,
		pContext);

	if (status == OS_DETAIL_STATUS_SUCCESS)
		pInsertRow->Send();

    assert (status == CTS_SUCCESS);
 
	return status;
}
/* end DdmFileSystem::AddFile  ***********************************************/

//  DdmFileSystem::InsertFileDescRow (CONTEXT* pContext)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::InsertFileDescRow(CONTEXT *pContext) 
{
	STATUS		status;

	TRACE_ENTRY(DdmFileSystem::InsertFileDescRow());

	pContext->state = FILE_DESC_ROW_INSERTED;

	MsgAddFile* msg = (MsgAddFile*) pContext->msg;
	UnicodeString16 fileName;

	msg->GetFileName(&fileName);

	pContext->pFileDescRec = new FileDescRecord(msg->GetType(),
		msg->GetFileSize(), fileName, pContext->rid);
	assert(pContext->pFileDescRec);

	FileDescRecord::RqInsertRow* preqInsert;

	preqInsert = new FileDescRecord::RqInsertRow(pContext->pFileDescRec, 1);
	assert(preqInsert);

    status = Send (preqInsert, pContext,
                   REPLYCALLBACK (DdmFileSystem, AddFileReplyHandler));

    assert (status == CTS_SUCCESS);
 
	return status;
}
/* end DdmFileSystem::InsertFileDescRow  ******************************************/

// temp vpts
//  DdmFileSystem::TempAddFileReplyHandler 
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::TempAddFileReplyHandler(void* pContext_, STATUS status)
{
	TRACE_ENTRY(DdmFileSystem::TempAddFileReplyHandler());
	
	CONTEXT* pContext = (CONTEXT*)pContext_;
	
	if (status==ercFieldSizeTooLarge)
	{
		Reply(pContext->msg, CTS_FILESYS_FILE_TOO_LARGE);
		ClearAndDeleteContext(pContext);
		ProcessNextCommand();	
	}

	assert(status == OK);

	callbackContext = (CONTEXT*)pContext_;

	AddFileReplyHandler(NULL);

	return CTS_SUCCESS;
}
/* end DdmFileSystem::TempAddFileReplyHandler  ******************************************/
// temp vpts

//  DdmFileSystem::AddFileReplyHandler (Message* pReply)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::AddFileReplyHandler(Message *pReply_)
{
	TRACE_ENTRY(DdmFileSystem::AddFileReplyHandler());

	// temp vpts
	if (pReply_==NULL)
	{
		InsertFileDescRow(callbackContext);
		return OK;
	}
	// temp vpts

	assert (pReply_ != NULL);
	assert(pReply_->Status()==OK);
	CONTEXT* pContext = (CONTEXT *) pReply_->GetContext();
	assert (pContext != NULL);

	MsgAddFile* msg = (MsgAddFile*) pContext->msg;
	RqPtsInsertRow* pReply = (RqPtsInsertRow*) pReply_;

	switch (pContext->state)
	{
	case FILE_ROW_INSERTED:
		// vpts temp assert(pReply->GetRowIdDataCount()==1);
		// vpts temp pContext->rid = *pReply->GetRowIdDataPtr();
		InsertFileDescRow(pContext);
		break;
	case FILE_DESC_ROW_INSERTED:
		assert(pReply->GetRowIdDataCount()==1);
		msg->SetFileKey(*pReply->GetRowIdDataPtr());
		Reply(msg, OK);
		ClearAndDeleteContext(pContext);
		ProcessNextCommand();
		break;
	default:
		assert(0);
		break;
	}

	delete pReply;

	return CTS_SUCCESS;
}
/* end DdmFileSystem::AddFileReplyHandler  ******************************************/

//  DdmFileSystem::DeleteFile (CONTEXT* pContext)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::DeleteFile(CONTEXT *pContext) 
{
	STATUS		status;

	TRACE_ENTRY(DdmFileSystem::DeleteFile());

	pContext->state = FILE_DESC_ROWS_READ;

	MsgDeleteFile* msg = (MsgDeleteFile*) pContext->msg;

	FileDescRecord::RqReadRow* preqRead;

	preqRead = new FileDescRecord::RqReadRow(msg->GetFileKey());
	assert(preqRead);

    status = Send (preqRead, pContext,
                   REPLYCALLBACK (DdmFileSystem, DeleteFileReplyHandler));

    assert (status == CTS_SUCCESS);
 
	return status;
}
/* end DdmFileSystem::DeleteFile  ***********************************************/

//  DdmFileSystem::DeleteFileRow (CONTEXT* pContext)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::DeleteFileRow(CONTEXT *pContext) 
{
	STATUS		status;

	TRACE_ENTRY(DdmFileSystem::DeleteFileRow());

	pContext->state = FILE_ROW_DELETED;

	FileRecord::RqDeleteRow* preqDelete;

	preqDelete = new FileRecord::RqDeleteRow(pContext->pFileDescRec->fileKey);
	assert(preqDelete);

    status = Send (preqDelete, pContext,
                   REPLYCALLBACK (DdmFileSystem, DeleteFileReplyHandler));

    assert (status == CTS_SUCCESS);
 
	return status;
}
/* end DdmFileSystem::DeleteFileRow  ******************************************/

//  DdmFileSystem::DeleteFileDescRow (CONTEXT* pContext)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::DeleteFileDescRow(CONTEXT *pContext) 
{
	STATUS		status;

	TRACE_ENTRY(DdmFileSystem::DeleteFileDescRow());

	pContext->state = FILE_DESC_ROW_DELETED;

	FileDescRecord::RqDeleteRow* preqDelete;

	preqDelete = new FileDescRecord::RqDeleteRow(pContext->pFileDescRec->rid);
	assert(preqDelete);

    status = Send (preqDelete, pContext,
                   REPLYCALLBACK (DdmFileSystem, DeleteFileReplyHandler));

    assert (status == CTS_SUCCESS);
 
	return status;
}
/* end DdmFileSystem::DeleteFileDescRow  ******************************************/

//  DdmFileSystem::DeleteFileReplyHandler (Message* pReply)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::DeleteFileReplyHandler(Message *pReply_)
{
	TRACE_ENTRY(DdmFileSystem::DeleteFileReplyHandler());

	assert (pReply_ != NULL);
	CONTEXT* pContext = (CONTEXT *) pReply_->GetContext();
	assert (pContext != NULL);
	if (pReply_->Status()!=OK)
	{
		// temp vpts
		//assert(pReply_->Status()==ercKeyNotFound &&
		//	pContext->state == FILE_DESC_ROWS_READ);
		if (pReply_->Status()!= ercKeyNotFound)
			assert(pContext->state==FILE_DESC_ROWS_READ ||
			pContext->state==FILE_ROW_DELETED);
	}
	
	switch (pContext->state)
	{
	case FILE_DESC_ROWS_READ:
		{
			FileDescRecord::RqReadRow* pReply = (FileDescRecord::RqReadRow*) pReply_;
			if (pReply->Status()==ercKeyNotFound)
			{
				Reply(pContext->msg, CTS_FILESYS_FILE_NOT_FOUND);
				ClearAndDeleteContext(pContext);
				ProcessNextCommand();
				return CTS_FILESYS_FILE_NOT_FOUND;
			}
			assert(pReply->GetRowCount()==1);
			pContext->pFileDescRec = pReply->GetRowCopy();
			usedSpace -= pContext->pFileDescRec->cbFile;
			DeleteFileRow(pContext);
			break;
		}
	case FILE_ROW_DELETED:
		DeleteFileDescRow(pContext);
		break;
	case FILE_DESC_ROW_DELETED:
		Reply(pContext->msg, OK);
		ClearAndDeleteContext(pContext);
		ProcessNextCommand();
		break;
	default:
		assert(0);
		break;
	}
	delete pReply_;

	return CTS_SUCCESS;
}
/* end DdmFileSystem::DeleteFileReplyHandler  ******************************************/

//  DdmFileSystem::ListFiles (CONTEXT* pContext)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::ListFiles(CONTEXT *pContext) 
{
	STATUS		status = OK;

	TRACE_ENTRY(DdmFileSystem::ListFiles());

	MsgListFiles* msg = (MsgListFiles*) pContext->msg;
	pContext->type = msg->GetType();

	if (pContext->type == ALL_FILES)
	{
		if (msg->GetFileKey() == 0)
		{
			// enumerate rows in File Desc Table
			pContext->state = FILE_DESC_TABLE_ENUM;

			FileDescRecord::RqEnumTable* preqEnum;
			preqEnum = new FileDescRecord::RqEnumTable();
			assert(preqEnum);

			status = Send (preqEnum, pContext,
					   REPLYCALLBACK (DdmFileSystem, ListFilesReplyHandler));

			assert (status == CTS_SUCCESS);
		}
		else
		{
			// read by file key
			pContext->state = FILE_DESC_ROWS_READ;
			FileDescRecord::RqReadRow* preqRead;
			
			preqRead = new FileDescRecord::RqReadRow(msg->GetFileKey());
			assert(preqRead);

			status = Send (preqRead, pContext,
					   REPLYCALLBACK (DdmFileSystem, ListFilesReplyHandler));

			assert (status == CTS_SUCCESS);
		}
	}
	else
	{
		// read rows by key type
		pContext->state = FILE_DESC_ROWS_READ;
		FileDescRecord::RqReadRow* preqRead;
		
		preqRead = new FileDescRecord::RqReadRow(CT_FDT_TYPE,
			&pContext->type,
			sizeof(pContext->type));
		assert(preqRead);

		status = Send (preqRead, pContext,
                   REPLYCALLBACK (DdmFileSystem, ListFilesReplyHandler));

		assert (status == CTS_SUCCESS);

	}

	return status;
}
/* end DdmFileSystem::ListFiles  ***********************************************/

//  DdmFileSystem::ListFilesReplyHandler (Message* pReply)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::ListFilesReplyHandler(Message *pReply_)
{
	U32 numberOfRecords;
	FileDescRecord* pFileDescRecords;
	TRACE_ENTRY(DdmFileSystem::ListFilesReplyHandler());

	assert (pReply_ != NULL);
	CONTEXT* pContext = (CONTEXT *) pReply_->GetContext();
	assert (pContext != NULL);
	if (pReply_->Status()!=OK)
	{
		assert(pReply_->Status()==ercKeyNotFound &&
			pContext->state == FILE_DESC_ROWS_READ);
	}

	MsgListFiles* msg = (MsgListFiles*) pContext->msg;
	
	switch (pContext->state)
	{
	case FILE_DESC_ROWS_READ:
		{
			if (pReply_->Status()==OK)
			{
				FileDescRecord::RqReadRow* pReply = (FileDescRecord::RqReadRow*) pReply_;
				pFileDescRecords = pReply->GetRowPtr();
				numberOfRecords = pReply->GetRowCount();
			}
			else 
				numberOfRecords=0;
			break;
		}
		case FILE_DESC_TABLE_ENUM:
		{
			FileDescRecord::RqEnumTable* pReply = (FileDescRecord::RqEnumTable*) pReply_;
			pFileDescRecords = pReply->GetRowPtr();
			numberOfRecords = pReply->GetRowCount();
 			break;
		}
	default:
		assert(0);
		break;
	}

	SystemInfo* pSysInfo = new SystemInfo(usedSpace, TOTAL_FILE_SYSTEM_SIZE);
	assert(pSysInfo);
	Entry *pEntry;
	for (U16 i = 0; i < numberOfRecords; i++)
	{
		I64 creationDate = pFileDescRecords[i].creationDate;
		UnicodeString file(pFileDescRecords[i].fileName);
		pEntry = new Entry(pFileDescRecords[i].rid,
			pFileDescRecords[i].type,
			&file,
			&creationDate,
			pFileDescRecords[i].cbFile);
		assert(pEntry);
		pSysInfo->AddEntry(pEntry);
	}
	msg->SetSysInfo(pSysInfo);

	delete pSysInfo;

	delete pReply_;
	
	Reply(pContext->msg, OK);
	ClearAndDeleteContext(pContext);
	ProcessNextCommand();

	return CTS_SUCCESS;
}
/* end DdmFileSystem::ListFilesReplyHandler  ******************************************/

//  DdmFileSystem::OpenFile (CONTEXT* pContext)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::OpenFile(CONTEXT *pContext) 
{
	STATUS		status = OK;

	TRACE_ENTRY(DdmFileSystem::OpenFile());

	pContext->state = FILE_DESC_ROWS_READ;

	MsgOpenFile* msg = (MsgOpenFile*) pContext->msg;

	FileDescRecord::RqReadRow* preqRead;

	preqRead = new FileDescRecord::RqReadRow(msg->GetFileKey());
	assert(preqRead);

    status = Send (preqRead, pContext,
                   REPLYCALLBACK (DdmFileSystem, OpenFileReplyHandler));

    assert (status == CTS_SUCCESS);
 
	return status;
}
/* end DdmFileSystem::OpenFile  ***********************************************/


//  DdmFileSystem::ReadFileRow (CONTEXT* pContext)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::ReadFileRow(CONTEXT *pContext) 
{
	STATUS		status = OK;

	TRACE_ENTRY(DdmFileSystem::ReadFileRow());

	pContext->state = FILE_ROW_READ;

	// temp vpts this needs to be a variable read row

	/*FileRecord::RqReadRow* preqRead;

	preqRead = new FileRecord::RqReadRow(pContext->pFileDescRec->fileKey);
	assert(preqRead);

    status = Send (preqRead, pContext,
                   REPLYCALLBACK (DdmFileSystem, OpenFileReplyHandler));
	
	assert (status == CTS_SUCCESS);*/

	TSReadVLRow		*pReadRow;

	pReadRow = new TSReadVLRow;

	status = pReadRow->Initialize(
		this,
		CT_FILE_TABLE_NAME,
		CT_PTS_RID_FIELD_NAME,
		&pContext->pFileDescRec->fileKey,
		sizeof(rowID),
		(CPtsRecordBase**)&pContext->pFileRec,
		&pContext->sizeOfRowData,
		&pContext->numberOfRows,
		(pTSCallback_t)&DdmFileSystem::TempOpenFileReplyHandler,
		pContext);

	if (status == OS_DETAIL_STATUS_SUCCESS)
		pReadRow->Send();

 
	return status;
}
/* end DdmFileSystem::ReadFileRow  ***********************************************/

// temp vpts
//  DdmFileSystem::TempOpenFileReplyHandler 
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::TempOpenFileReplyHandler(void* pContext_, STATUS status)
{
	TRACE_ENTRY(DdmFileSystem::TempOpenFileReplyHandler());

	assert(status == OK);

	callbackContext = (CONTEXT*)pContext_;

	OpenFileReplyHandler(NULL);

	return CTS_SUCCESS;
}
/* end DdmFileSystem::TempOpenFileReplyHandler  ******************************************/
// temp vpts

//  DdmFileSystem::OpenFileReplyHandler (Message* pReply)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::OpenFileReplyHandler(Message *pReply_)
{
	TRACE_ENTRY(DdmFileSystem::OpenFileReplyHandler());

	// temp vpts
	if (pReply_==NULL)
	{
		UnicodeString file(callbackContext->pFileDescRec->fileName);
		Entry* pEntry = new Entry(
			callbackContext->pFileDescRec->rid,
			callbackContext->pFileDescRec->type,
			&file,
			&callbackContext->pFileDescRec->creationDate,
			callbackContext->pFileDescRec->cbFile,
			callbackContext->pFileRec->vfFile.Ptr());
		assert(pEntry);
		MsgOpenFile* msg = (MsgOpenFile*) callbackContext->msg;
		msg->SetFile(pEntry);
		Reply(msg, OK);
		ClearAndDeleteContext(callbackContext);
		ProcessNextCommand();
		return OK;
	}
	// temp vpts

	assert (pReply_ != NULL);
	CONTEXT* pContext = (CONTEXT *) pReply_->GetContext();
	assert (pContext != NULL);
	if (pReply_->Status()!=OK)
	{
		assert(pReply_->Status()==ercKeyNotFound &&
			pContext->state == FILE_DESC_ROWS_READ);
	}

	MsgOpenFile* msg = (MsgOpenFile*) pContext->msg;
	
	switch (pContext->state)
	{
	case FILE_DESC_ROWS_READ:
		{
			FileDescRecord::RqReadRow* pReply = (FileDescRecord::RqReadRow*) pReply_;
			if (pReply->Status()==ercKeyNotFound)
			{
				Reply(pContext->msg, CTS_FILESYS_FILE_NOT_FOUND);
				ClearAndDeleteContext(pContext);
				ProcessNextCommand();
				return CTS_FILESYS_FILE_NOT_FOUND;
			}
			assert(pReply->GetRowCount()==1);
			pContext->pFileDescRec = pReply->GetRowCopy();
			ReadFileRow(pContext);
			break;
		}
	case FILE_ROW_READ:
		{
			FileRecord::RqReadRow *pReply = 
				(FileRecord::RqReadRow*) pReply_;
			// jlo copy variable sized row
			assert(pReply->GetRowCount()==1);
			CopyRowDataFromSgl(&pContext->pFileRec,
				pReply->GetRowPtr(),
				pReply->GetRowCount(),
				sizeof(FileRecord));
			UnicodeString file(pContext->pFileDescRec->fileName);
			Entry* pEntry = new Entry(
				pContext->pFileDescRec->rid,
				pContext->pFileDescRec->type,
				&file,
				&pContext->pFileDescRec->creationDate,
				pContext->pFileDescRec->cbFile,
				pContext->pFileRec->vfFile.Ptr());
			assert(pEntry);
			msg->SetFile(pEntry);
			Reply(msg, OK);
			ClearAndDeleteContext(pContext);
			ProcessNextCommand();
			break;
		}
	default:
		assert(0);
		break;
	}
	delete pReply_;

	return CTS_SUCCESS;
}
/* end DdmFileSystem::OpenFileReplyHandler  ******************************************/

//  DdmFileSystem::GetSysInfo (CONTEXT* pContext)
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmFileSystem::GetSysInfo(CONTEXT *pContext) 
{
	TRACE_ENTRY(DdmFileSystem::GetSysInfo());

	SystemInfo* pSysInfo = new SystemInfo(usedSpace, TOTAL_FILE_SYSTEM_SIZE);
	assert(pSysInfo);

	MsgGetSysInfo* msg = (MsgGetSysInfo*) pContext->msg;

	msg->SetSysInfo(pSysInfo);

	delete pSysInfo;

	Reply(msg, OK);
	ClearAndDeleteContext(pContext);
	ProcessNextCommand();

	return CTS_SUCCESS;
}
/* end DdmFileSystem::GetSysInfo  ***********************************************/


//  DdmFileSystem::SubmitRequest
//
//  Description:
//		This function either enqueues a new function call
//		entry or makes the call if the queue is presently
//		empty and the AlarmMaster is not processing a command
//  Inputs:
//	  U32 functionToCall - enumerated value of function to call
//    void* pContext - information passed from the calling DdmFileSystem
//      method
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
void DdmFileSystem::SubmitRequest(U32 functionToCall, CONTEXT* pContext)
{
	if (m_pFileSystemQueue->Empty() && NotProcessingCommand())
		ExecuteFunction(functionToCall, pContext);
	else
		m_pFileSystemQueue->AddFunctionCall(functionToCall, ((void*)pContext));
}
/* end DdmFileSystem::SubmitRequest  ********************************/

//  DdmFileSystem::ExecuteFunction
//
//  Description:
//		Sets the ProcessingCommand flag in the File System Master and
//		calls the appropriate function
//
//  Inputs:
//    U32 functionToCall - enumerated value which specifies 
//		function to call
//    void* _pContext - information passed from the calling DdmFileSystem
//      method
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
void DdmFileSystem::ExecuteFunction(U32 functionToCall, CONTEXT* pContext)
{
	ProcessCommand();
	switch (functionToCall)
	{
	case function_AddFile:
		AddFile(pContext);
		break;
	case function_DeleteFile:
		DeleteFile(pContext);
		break;
	case function_ListFiles:
		ListFiles(pContext);
		break;
	case function_OpenFiles:
		OpenFile(pContext);
		break;
	case function_GetSysInfo:
		GetSysInfo(pContext);
		break;
	default:
		assert(0);
		break;
	}
}
/* end DdmFileSystem::ExecuteFunction  ********************************/

//  DdmFileSystem::ProcessNextCommand
//
//  Description:
//		Checks the queue for next function call and executes
//		or sets the Alarm Master to a non-busy state if there
//		are no more commands to process
//
void DdmFileSystem::ProcessNextCommand()
{
	FunctionCallElement* pElement;

	pElement = m_pFileSystemQueue->PopFunctionCall();

	if (pElement)
	{
		ExecuteFunction(pElement->GetFunction(), (CONTEXT*)pElement->GetContext());
		delete pElement;
	}
	else
		FinishCommand();
}
/* end DdmFileSystem::ProcessNextCommand  ********************************/

