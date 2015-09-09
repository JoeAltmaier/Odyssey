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
*	This file contains the definition of the File System messages.
*************************************************************************/

#include "FileSystemMessages.h"


	// Ctor
	MsgListFiles::MsgListFiles()
		: Message(REQ_FILESYS_LIST_FILES), type(ALL_FILES), fileKey(0)
	{
		// allocate the SGLS for reply
		AddSgl(SYSINFO_SGL, NULL, 0, SGL_DYNAMIC_REPLY);
	}

	// Ctor
	MsgListFiles::MsgListFiles(FileType type_)
		: Message(REQ_FILESYS_LIST_FILES), type(type_), fileKey(0)
	{
		// allocate the SGLS for reply
		AddSgl(SYSINFO_SGL, NULL, 0, SGL_DYNAMIC_REPLY);
	}

	// Ctor
	MsgListFiles::MsgListFiles(RowId fileKey_)
		: Message(REQ_FILESYS_LIST_FILES), type(ALL_FILES), fileKey(fileKey_)
	{
		// allocate the SGLS for reply
		AddSgl(SYSINFO_SGL, NULL, 0, SGL_DYNAMIC_REPLY);
	}

	// D'tor can't be virtual, so the message must be cast prior to deletion.
	MsgListFiles::~MsgListFiles() { if (IsLast()) CleanAllSgl(); }  // delete client allocated SGLs
	
	void MsgListFiles::SetSysInfo(SystemInfo* pSysInfo)
	{
		void *pData;
		cbData = pSysInfo->GetSysInfoData(&pData);
		AllocateSgl(SYSINFO_SGL, cbData);
		CopyToSgl(SYSINFO_SGL, 0, pData, cbData);
		delete pData;
	}

	void MsgListFiles::GetSysInfo(SystemInfo** ppSysInfo)
	{
		void* pData = new (tBIG) char[cbData];
		CopyFromSgl(SYSINFO_SGL, 0, pData, cbData);
		*ppSysInfo = new SystemInfo(pData);
		delete pData;
		assert(*ppSysInfo);
	}

	void MsgOpenFile::SetFile(Entry* pEntry)
	{
		void* pData;
		cbData = pEntry->GetEntryInfoData(&pData);
		AllocateSgl(FILE_SGL, cbData);
		CopyToSgl(FILE_SGL, 0, pData, cbData);
		delete pData;
	}

	void MsgOpenFile::GetFile(Entry** ppEntry)
	{
		void *pData = new (tBIG) char[cbData];
		CopyFromSgl(FILE_SGL, 0, pData, cbData);
		*ppEntry = new Entry(pData);
		delete pData;
		assert(*ppEntry);
	}