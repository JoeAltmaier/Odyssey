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
*	This file contains the declaration of the LogMaster DDM
* 
*  Trace Levels:
*		0: Errors
*		1: Ctor, Dtor, Create, Delete, Open, Close, Rename, Resize
*		3: Read, Write
*		5: Detailed file information output during file operations
*
* Update Log: 
* 06/07/99 Bob Butler: Create file
* 08/18/99 Bob Butler: Add TraceF code.
*
*************************************************************************/
#include "CtTypes.h"
#include "RequestCodes.h"
#include "ChaosFile.h"
#include "ChaosFileMsgs.h"
#include "DdmPartitionMgr.h"

#define	TRACE_INDEX		TRACE_CHAOSFILE
#include "odyssey_trace.h"

#define _DEBUG
#ifdef _DEBUG
char *CfErrStr[11] = {"Success", "File Not Found", "File Already Open",
	"EOF", "Out Of Space", "Duplicate Name", "Invalid Name", "Invalid Handle", 
	"Resize Too Small", "Write Past End", "Read Past End"};

#define CF_ERRSTR(i) CfErrStr[(i)]
#else
#define CF_ERRSTR(i)
#endif

ChaosFile::ChaosFile(char *name_)
{
	TRACEF(TRACE_L1, ("ChaosFile::ChaosFile(%s): Open file.\n", name_));
	name = new char[strlen(name_) + 1];
	strcpy(name, name_);
	
	if (strlen(name_) <= cfNameLen)
	{
		ReqChaosFileOpen *pMsg = new ReqChaosFileOpen(name);
		DdmPartitionMgr::SendWait(pMsg);
		lastErr = (CfError)pMsg->resultCode;
		isValid = lastErr == cfSuccess;
		handle = pMsg->handle;
		cBytes = pMsg->size;
		cMaxBytes = pMsg->maxSize;
		delete pMsg;
	}
	else
	{
		lastErr = cfInvalidName;
		isValid = false;
		handle = CF_INVALIDHANDLE;
	}
	TRACEF(TRACE_L5, ("ChaosFile::ChaosFile(%s): handle=%d, size=%d, maxSize=%d", name_, handle, cBytes, cMaxBytes));
	if (!isValid)
		Tracef("ChaosFile::ChaosFile(%s): open error: %s\n", name_, CF_ERRSTR(lastErr));
}

ChaosFile::ChaosFile (char *name_, U32 maxBytes_)
{
	TRACEF(TRACE_L1, ("ChaosFile::ChaosFile(%s, %d): Create file.\n", name_, maxBytes_));
	name = new char[strlen(name_) + 1];
	strcpy(name, name_);
	if (strlen(name_) <= cfNameLen)
	{
		ReqChaosFileCreate *pMsg = new ReqChaosFileCreate(name_, maxBytes_);
		DdmPartitionMgr::SendWait(pMsg);
		lastErr = (CfError)pMsg->resultCode;
		isValid = lastErr == cfSuccess;
		handle = pMsg->handle;
		cBytes = pMsg->size;
		cMaxBytes = pMsg->maxSize;
		delete pMsg;
	}
	else
	{
		lastErr = cfInvalidName;
		isValid = false;
		handle = CF_INVALIDHANDLE;
	}
	TRACEF(TRACE_L5, ("ChaosFile::ChaosFile(%s, %d): handle=%d, size=%d, maxSize=%d\n", name_, maxBytes_, handle, cBytes, cMaxBytes));
	if (!isValid)
		Tracef("ChaosFile::ChaosFile(%s, %d): open error: %s\n", name, maxBytes_, CF_ERRSTR(lastErr));
}

ChaosFile::~ChaosFile()
{
	TRACEF(TRACE_L1, ("ChaosFile::~ChaosFile(): name=%s, handle=%d, size=%d, maxSize=%d\n", name, handle, cBytes, cMaxBytes));
	Close();
	delete name;
}

CfError ChaosFile::Rename(char *name_)
{
	Tracef("ChaosFile::Rename(%s): Not Yet Implemented\n");
	return cfSuccess;
}

CfError ChaosFile::Delete()
{
	Tracef("ChaosFile::Delete(): Not Yet Implemented\n");
	return cfSuccess;
}

CfError ChaosFile::Close()
{
	TRACEF(TRACE_L5, ("ChaosFile::Close():: name=%s, handle=%d, size=%d, maxSize=%d\n", name, handle, cBytes, cMaxBytes));
	if (isValid)
	{
		ReqChaosFileClose *pMsg = new ReqChaosFileClose(handle);
		DdmPartitionMgr::SendWait(pMsg);
		lastErr = (CfError)pMsg->resultCode;
		isValid = false;
		handle = CF_INVALIDHANDLE;
		cBytes = pMsg->size;
		cMaxBytes = pMsg->maxSize;
		delete pMsg;
		TRACEF(TRACE_L1, ("ChaosFile::Close(): Closed file.\n"));
	}
	else
	{
		lastErr = cfInvalidHandle;
		handle = CF_INVALIDHANDLE;
	}
	if (lastErr != cfSuccess)
		Tracef("ChaosFile::Close(): Close error: %s\n", CF_ERRSTR(lastErr));
	return lastErr;
}

CfError ChaosFile::SetMaxSize(U32 maxBytes_)
{
	Tracef("ChaosFile::SetMaxSize(%d): Not Yet Implemented\n", maxBytes_);
	return cfSuccess;
}

CfError ChaosFile::SetSize(U32 cBytes_)
{
	if (isValid)
	{
		ReqChaosFileSetSize *pMsg = new ReqChaosFileSetSize(handle, cBytes_);
		DdmPartitionMgr::SendWait(pMsg);
		lastErr = (CfError)pMsg->resultCode;
		cBytes = pMsg->size;
		cMaxBytes = pMsg->maxSize;
		delete pMsg;
	}
	else
		lastErr = cfInvalidHandle;

	if (lastErr != cfSuccess)
		Tracef("ChaosFile::SetSize(): Error: %s\n", CF_ERRSTR(lastErr));

	return lastErr;
}


CfError ChaosFile::Read(void *buffer_, U32 offset_, U32 length_)
{
	TRACEF(TRACE_L3, ("ChaosFile::Read():: name=%s, offset=%d, length=%d\n", name, offset_, length_));
	TRACEF(TRACE_L5, ("ChaosFile::Read():: name=%s, handle=%d, size=%d, maxSize=%d\n", name, handle, cBytes, cMaxBytes));
	if (isValid)
	{
		ReqChaosFileRead *pMsg = new ReqChaosFileRead(handle, buffer_, offset_, length_);
		DdmPartitionMgr::SendWait(pMsg);
		lastErr = (CfError)pMsg->resultCode;
		cBytes = pMsg->size;
		cMaxBytes = pMsg->maxSize;
		delete pMsg;
	}
	else
		lastErr = cfInvalidHandle;

	if (lastErr != cfSuccess)
		Tracef("ChaosFile::Read(): Read error: %s\n", CF_ERRSTR(lastErr));

	return lastErr;
}

CfError ChaosFile::Write(void *buffer_, U32 offset_, U32 length_, BOOL markEOF_)
{
	TRACEF(TRACE_L3, ("ChaosFile::Write():: name=%s, offset=%d, length=%d, markEOF=%s\n", name, offset_, length_, markEOF_ ? "ture" : "false"));
	TRACEF(TRACE_L5, ("ChaosFile::Write():: name=%s, handle=%d, size=%d, maxSize=%d\n", name, handle, cBytes, cMaxBytes));
	if (isValid)
	{
		ReqChaosFileWrite *pMsg = new ReqChaosFileWrite(handle, buffer_, offset_, length_, markEOF_);
		DdmPartitionMgr::SendWait(pMsg);
		lastErr = (CfError)pMsg->resultCode;
		cBytes = pMsg->size;
		cMaxBytes = pMsg->maxSize;
		delete pMsg;
	}
	else
		lastErr = cfInvalidHandle;

	if (lastErr != cfSuccess)
		Tracef("ChaosFile::Write(): Write error: %s\n", CF_ERRSTR(lastErr));
	return lastErr;
}

