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

// Revision History:
// $Log: /Gemini/Include/FileSystem/FileSystemMessages.h $
// 
// 6     1/26/00 2:29p Joehler
// Allocate memory from big heap and separate functions to
// FileSystemMessages.cpp.
// 
// 5     11/17/99 3:22p Joehler
// Change handling for SGLs
// 
// 4     10/18/99 11:23a Joehler
// Removed unnecessary comment
// 
// 3     10/15/99 10:57a Joehler
// Moved error codes to *.mc files to localize
// 
// 2     10/06/99 4:20p Joehler
// added error checking
// 
// 1     9/30/99 7:43a Joehler
// First cut of File System Master
// 
//			


#ifndef FileSystemMessages_h
#define FileSystemMessages_h

#include "CTTypes.h"
#include "message.h"
#include "RequestCodes.h"

#include "FileSystemInfo.h"

// for other clients,  add new types here
typedef enum {
	ALL_FILES, // used to list all file types in the system 
	GENERIC_TYPE,
	IMAGE_TYPE
} FileType;

class MsgAddFile : public Message
{
public:
	// SGL indexes for the data being sent
	enum {FILE_SGL};
	
	// Ctor: add the SGLs for the image	
	MsgAddFile(U32 cbFile_, void *pFile_, FileType type_, UnicodeString16 fileName_)
	 : Message(REQ_FILESYS_ADD_FILE), cbFile(cbFile_), type(type_)
	{
		memcpy(fileName, fileName_, sizeof(fileName));
		// need to make a copy of the file to add as a sgl
		// because this memory will be deleted when the the message
		// is destructed by CleanAllSgl()
		void* pFile = new (tBIG) char[cbFile];
		memcpy(pFile, pFile_, cbFile);
		AddSgl(FILE_SGL, pFile, cbFile);
	}

	// D'tor can't be virtual, so the message must be cast prior to deletion.
	~MsgAddFile() { if (IsLast()) CleanAllSgl(); }  // delete client allocated SGLs
	
	U32 GetFileSize() { return cbFile; }

	// allocate storage for the file, and copy the data.  The
	// caller supplies the pointer and is responsible for deleting it.
	void GetFile(void **ppFile_) 
	{
		*ppFile_ = new (tBIG) char[cbFile];
		
		CopyFromSgl(FILE_SGL, 0, *ppFile_, cbFile);
	}

	FileType GetType() { return type; }

	void GetFileName(UnicodeString16* fileName_)
	{
		memcpy(*fileName_, fileName, sizeof(fileName));
	}
			
	void SetFileKey(RowId fileKey_) { fileKey = fileKey_; }
	RowId GetFileKey() { return fileKey; }

private:
	// send data:
	FileType type;
	U32 cbFile;
	UnicodeString16 fileName;
	// reply data:
	RowId fileKey;
};

class MsgDeleteFile : public Message
{
public:	
	// Ctor
	MsgDeleteFile(RowId fileKey_)
		: Message(REQ_FILESYS_DELETE_FILE), fileKey(fileKey_) {}

	// D'tor can't be virtual, so the message must be cast prior to deletion.
	~MsgDeleteFile() { }  // delete client allocated SGLs
	
	RowId GetFileKey() { return fileKey; }

private:
	// send data:
	RowId fileKey;
};

class MsgListFiles : public Message
{
public:	
	// SGL indexes for the data being sent
	enum {SYSINFO_SGL};
		
	// Ctor
	MsgListFiles();

	// Ctor
	MsgListFiles(FileType type_);

	// Ctor
	MsgListFiles(RowId fileKey_);

	// D'tor can't be virtual, so the message must be cast prior to deletion.
	~MsgListFiles();

	FileType GetType() { return type; }

	RowId GetFileKey() { return fileKey; }
		
	void SetSysInfo(SystemInfo* pSysInfo);

	void GetSysInfo(SystemInfo** ppSysInfo);

private:
	// input data
	FileType type;
	RowId fileKey;
	// return data:
	U32 cbData;
};

class MsgOpenFile : public Message
{
public:	
	// SGL indexes for the data being sent
	enum {FILE_SGL};
		
	// Ctor
	MsgOpenFile(RowId fileKey_)
		: Message(REQ_FILESYS_OPEN_FILE), fileKey(fileKey_)
	{
		// allocate the SGLS for reply
		AddSgl(FILE_SGL, NULL, 0, SGL_DYNAMIC_REPLY);
	}

	// D'tor can't be virtual, so the message must be cast prior to deletion.
	~MsgOpenFile() { if (IsLast()) CleanAllSgl(); }  // delete client allocated SGLs

	RowId GetFileKey() { return fileKey; }
	
	void SetFile(Entry* pEntry);

	void GetFile(Entry** ppEntry);

private:
	// input data
	RowId fileKey;
	// return data:
	U32 cbData;
};

class MsgGetSysInfo : public Message
{
public:	
	// SGL indexes for the data being sent
	enum {SYSINFO_SGL};
		
	// Ctor
	MsgGetSysInfo()
		: Message(REQ_FILESYS_GET_SYSINFO)
	{
		// allocate the SGLS for reply
		AddSgl(SYSINFO_SGL, NULL, 0, SGL_DYNAMIC_REPLY);
	}

	// D'tor can't be virtual, so the message must be cast prior to deletion.
	~MsgGetSysInfo() { if (IsLast()) CleanAllSgl(); }  // delete client allocated SGLs
	
	void SetSysInfo(SystemInfo* pSysInfo)
	{
		void* pData;
		cbData = pSysInfo->GetSysInfoData(&pData);
		AllocateSgl(SYSINFO_SGL, cbData);
		CopyToSgl(SYSINFO_SGL, 0, pData, cbData);
		delete pData;
	}

	void GetSysInfo(SystemInfo** ppSysInfo)
	{
		void* pData = new (tBIG) char[cbData];
		CopyFromSgl(SYSINFO_SGL, 0, pData, cbData);
		*ppSysInfo = new SystemInfo(pData);
		assert(*ppSysInfo);
		delete pData;
	}

private:
	// return data:
	U32 cbData;
};

#endif