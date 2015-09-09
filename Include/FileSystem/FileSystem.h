/* FileSystem.h
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
// $Log: /Gemini/Include/FileSystem/FileSystem.h $
// 
// 3     10/12/99 11:13a Joehler
// Modified for variable PTS entries
// 
// 2     10/07/99 9:55a Joehler
// put if's around delete of members of pContext
// 
// 1     9/30/99 7:43a Joehler
// First cut of File System Master
//			



#ifndef __FileSystem_H
#define __FileSystem_H

#include "Trace_Index.h"		// include the trace module numbers

#define	TRACE_INDEX		TRACE_FILESYS	// set this modules index to your index	
#include "Odyssey_Trace.h"			// include trace macros

#include "Ddm.h"
#include "CTtypes.h"
#include "CommandProcessingQueue.h"

#include "FileDescriptorTable.h"
#include "FileTable.h"
#include "FileSystemMessages.h"

class DdmFileSystem: public Ddm 
{
public:
	DdmFileSystem(DID did) : Ddm(did) {}
	static Ddm *Ctor(DID did);

	STATUS Initialize (Message *pMsg);

	STATUS Enable(Message *pMsg);
	
	STATUS ProcessAddFile(Message *pMsg);
	STATUS ProcessDeleteFile(Message *pMsg);
	STATUS ProcessListFiles(Message *pMsg);
	STATUS ProcessOpenFile(Message *pMsg);
	STATUS ProcessGetSysInfo(Message *pMsg);

private:

	// enum for state of CONTEXT
	enum {
		FILE_DESC_TABLE_DEFINED,
		FILE_TABLE_DEFINED,
		FILE_DESC_ROW_INSERTED,
		FILE_ROW_INSERTED,
		FILE_DESC_ROWS_READ,
		FILE_ROW_READ,
		FILE_DESC_ROW_DELETED,
		FILE_ROW_DELETED,
		FILE_DESC_TABLE_ENUM
	};

	typedef struct _CONTEXT {
		U32 state; // state as defined by the above enum
		Message* msg;
		FileRecord* pFileRec;
		FileDescRecord* pFileDescRec;
		FileType type;
		rowID rid;
		// temp vpts
		U32 sizeOfRowData;
		U32 numberOfRows;
	} CONTEXT;

	void ClearAndDeleteContext(CONTEXT* pContext)
	{
		if (pContext->pFileRec)
			delete pContext->pFileRec;
		pContext->pFileRec = NULL;
		if (pContext->pFileDescRec)
			delete pContext->pFileDescRec;
		pContext->pFileDescRec = NULL;
	}

	enum
	{
		function_AddFile = 1,
		function_DeleteFile,
		function_ListFiles,
		function_OpenFiles,
		function_GetSysInfo
	};

	CommandProcessingQueue* m_pFileSystemQueue;
	BOOL processingCommand;

	BOOL NotProcessingCommand() { return (processingCommand == FALSE); }
	void ProcessCommand() { processingCommand = TRUE; }
	void FinishCommand() { processingCommand = FALSE; }

	BOOL m_Initialized;

	U32 usedSpace;

	// jlo vpts
	CONTEXT *callbackContext;

private:

	// generic action methods
	STATUS AddFile(CONTEXT *pContext);
	STATUS DeleteFile(CONTEXT *pContext);
	STATUS ListFiles(CONTEXT *pContext);
	STATUS OpenFile(CONTEXT *pContext);
	STATUS GetSysInfo(CONTEXT *pContext);

	// initialization methods
	STATUS DefineFileDescTable(CONTEXT* pContext);
	STATUS EnumFileDescTable(CONTEXT* pContext);
	STATUS DefineFileTable(CONTEXT* pContext);
	STATUS InitReplyHandler(Message *pReply);

	// Add File methods
	STATUS InsertFileDescRow(CONTEXT *pContext);
	STATUS TempAddFileReplyHandler(void* pContext, STATUS status);
	STATUS AddFileReplyHandler(Message *pReply);

	// Delete File methods
	STATUS DeleteFileRow(CONTEXT *pContext);
	STATUS DeleteFileDescRow(CONTEXT *pContext);
	STATUS DeleteFileReplyHandler(Message *pReply);

	// List Files methods
	STATUS ListFilesReplyHandler(Message *pReply);

	// Open File methods
	STATUS ReadFileRow(CONTEXT* pContext);
	STATUS TempOpenFileReplyHandler(void* pContext, STATUS status);
	STATUS OpenFileReplyHandler(Message* pReply);

	void SubmitRequest(U32 functionToCall, CONTEXT* pContext);
	void ExecuteFunction(U32 functionToCall, CONTEXT* pContext);
	void ProcessNextCommand();

};

#endif	// __DdmFileSystem_H
