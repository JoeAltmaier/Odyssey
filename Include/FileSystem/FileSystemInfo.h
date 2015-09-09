//*************************************************************************
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
//
// 
// Update Log: 
// $Log: /Gemini/Include/FileSystem/FileSystemInfo.h $
// 
// 5     1/26/00 2:29p Joehler
// Allocate memory from big heap and memcpy creationDate to ei.
// 
// 4     10/21/99 3:32p Joehler
// Change for Metrowerks
// 
// 3     10/12/99 11:13a Joehler
// Modified for variable PTS entries
// 
// 2     10/06/99 4:19p Joehler
// added error checking
// 
// 1     9/30/99 7:43a Joehler
// First cut of File System Master
//*************************************************************************

#ifndef _FileSystemInfo_h_
#define _FileSystemInfo_h_

#include <string.h>

#include "CTTypes.h"
#include "UnicodeString.h"
#include "assert.h"


// jlo copy
template <class T>
void CopyRowDataFromSgl(T** resultRow,
				   T* rowPtr,
				   U32 numberOfRows,
				   U32 cbRow)
{
	*resultRow = (T*)new char[numberOfRows*cbRow];
	assert(*resultRow);
	for(U32 i = 0; i < numberOfRows; i++)
	{
		memcpy(&((*resultRow)[i]), &rowPtr[i], cbRow);
	}
}

class Entry 
{
public:
	Entry(RowId key_, U32 type_, UnicodeString* fileName_, 
			I64* creationDate_, U32 cbFile_ = 0);

	Entry(RowId key_, U32 type_, 
		UnicodeString* fileName_, 
			I64* creationDate_,
			U32 cbFile_, 
			void* pFile_);

	Entry(void* pData);

	~Entry();

	rowID GetKey() const { return ei.key; }

	U32 GetType() const { return ei.type; }

	U32 GetCbFile() const { return ei.cbFile; }

	void GetCreationDate(I64* date) const 
	{  
		memcpy(date, ei.creationDate, sizeof(ei.creationDate));
	} 

	void GetFileName(UnicodeString* fileName) const
	{
		UnicodeString us(ei.fileName);
		*fileName = us;
	}

	void AddFile(void* pFile_, U32 cbFile_)
	{
		ei.cbFile = cbFile_;
		pFile = new (tBIG) char[cbFile_];
		assert(pFile);
		memcpy(pFile, pFile_, ei.cbFile);
	}

	U32 GetFile(void** pFile_) const
	{
		*pFile_ = new (tBIG) char[ei.cbFile];
		assert(*pFile_);
		memcpy(*pFile_, pFile, ei.cbFile);
		return ei.cbFile;
	}

	// pack up the data into a blob to be sent as a message
	U32 GetEntryInfoData(void **pData_) const;

private:

	void Set(void* pData);

private:
	// entry information
	struct 
	{
		rowID key;
		U32 type;
		U32 cbFile;
		UnicodeString16 fileName;
		I64 creationDate;
	} ei;

	void* pFile;
	U32 pad;
};

// file system initially at 5 Megs
#define ONEK 1024
//#define TOTAL_FILE_SYSTEM_SIZE  (5 * 1000 * ONEK)
//#define TOTAL_FILE_SYSTEM_SIZE (2.5 * ONEK)
#define TOTAL_FILE_SYSTEM_SIZE 4000000

class SystemInfo
{
public:

	SystemInfo(U32 totalSize_);

	SystemInfo(U32 usedSpace_, U32 totalSize_);

	// construct an SystemInfo struct from binary blob.
	SystemInfo(void *pData);	

	~SystemInfo();

	U32 GetNumberOfEntries() { return si.numberOfEntries; }
	void SetNumberOfEntries(U32 number) { si.numberOfEntries = number; }

	U32 GetFileSystemSize() { return si.totalSize; }

	U32 GetUsedSpace() { return si.usedSpace; }
	void SetUsedSpace(U32 space) { si.usedSpace = space; }

	U32 GetAvailableSpace() { return (si.totalSize-si.usedSpace); }
	
	const Entry* GetFirst() 
	{
		placeHolder = 0;
		return GetEntry(placeHolder);
	}

	const Entry* GetNext() 
	{
		return GetEntry(++placeHolder);
	}

	void AddEntry(Entry *entry_)
	{
		Append(entry_);
	}

	// pack up the data into a blob to be sent as a message
	U32 GetSysInfoData(void **pData_) const;
	
private:

	void Set(void *pData);

	const Entry *GetEntry(U16 i) const
	{ 
		if (i < si.numberOfEntries)
			return pEntries[i]; 
		else 
			return NULL;
	}
	
private:
	// system information
	struct 
	{
		U32 numberOfEntries;
		U32	usedSpace;
		U32 totalSize;
	} si;

	// don't allow copy ctor or equals operator.  Probably add later.
	SystemInfo(const SystemInfo &);
	SystemInfo &operator=(const SystemInfo &);

	U32 placeHolder;
	Entry** pEntries;  // array of pointers to the file system entries	
	U32 pad;
	
	// append an entry to the SystemInfo entries
	void Append(Entry *pEntry);	

};


#endif	// _FileSystemInfo_h_
