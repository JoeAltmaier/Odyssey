/*************************************************************************
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
*************************************************************************/
#include "FileSystemInfo.h"

Entry::Entry(RowId key_, U32 type_, UnicodeString* fileName_, 
			I64* creationDate_, U32 cbFile_)
{
	ei.key = key_;
	ei.type = type_;
	ei.creationDate = *creationDate_;
	ei.cbFile = cbFile_;
	fileName_->CString(ei.fileName, sizeof(ei.fileName));
	pFile = NULL;
}

Entry::Entry(RowId key_, U32 type_, UnicodeString* fileName_, 
			I64* creationDate_,
			U32 cbFile_, 
			void* pFile_) 
{
	ei.key = key_;
	ei.type = type_;
	ei.creationDate = *creationDate_;
	ei.cbFile = cbFile_;
	fileName_->CString(ei.fileName, sizeof(ei.fileName));
	pFile = new (tBIG) char[ei.cbFile];
	assert(pFile);
	memcpy(pFile, pFile_, ei.cbFile);
}

Entry::Entry(void* pData)
{
	Set(pData);
}

void Entry::Set(void *pData)
{
	U32 cb = 0;
	
	char *p = (char *)pData;
	memcpy(&cb, p, sizeof(cb));	// total bytes
	p += sizeof(cb);
	memcpy(&ei, p, sizeof(ei));
	p += sizeof(ei);
	
	if (cb != (sizeof(cb) + sizeof(ei)))
	{
		pFile = new (tBIG) char[ei.cbFile];
		assert(pFile);
		memcpy(pFile, p, ei.cbFile);
	}
	else
		pFile = 0;
}

Entry::~Entry()
{
	if (pFile != NULL)
		delete pFile;
}

U32 Entry::GetEntryInfoData(void **pData_) const
{
   /*
	* build a blob containing the entry data.  
	* the blob is formatted as follows:
	* <cbEntry><ei><file>
	*/

	U32 cb = 0;
	if (pFile != NULL)
		cb = sizeof(cb) + sizeof(ei) + ei.cbFile;
	else
		cb = sizeof(cb) + sizeof(ei);

	*pData_ = new (tBIG) char[cb];  // allocate the blob.  
	assert(*pData_);

	char *p = (char *)*pData_;

	memcpy(p, &cb, sizeof(cb));	// total bytes
	p += sizeof(cb);	

	memcpy(p, &ei, sizeof(ei));	// Entry Info data
	p += sizeof(ei);
	
	if (pFile!=NULL)
	{
		memcpy(p, pFile, ei.cbFile);
		p += ei.cbFile;
	}

	return cb;

}

const U16 FILE_SYSTEM_ENTRY_GROW_SIZE = 2;

SystemInfo::SystemInfo(U32 usedSpace_, U32 totalSize_)
{
	si.usedSpace = usedSpace_;
	si.totalSize = totalSize_;
	si.numberOfEntries = 0;
	pEntries = NULL;
}

SystemInfo::SystemInfo(U32 totalSize_)
{
	si.usedSpace = 0;
	si.totalSize = totalSize_;
	si.numberOfEntries = 0;
	pEntries = NULL;
}

SystemInfo::SystemInfo(void *pData) 		// a blob containing everything about the event
{
	Set(pData);
}

void SystemInfo::Set(void *pData)
{
	/*<total bytes><si><entryBlob1>...<entryBlobN>*/

	U32 cb = 0;
	U16 i;
	
	char *p = (char *)pData;
	memcpy(&cb, p, sizeof(cb));	// total bytes
	p += sizeof(cb);
	memcpy(&si, p, sizeof(si));
	p += sizeof(si);
	
	pEntries = new Entry *[(si.numberOfEntries)];
	assert(pEntries);
	U32 sizeEntryBlob;

	if (si.numberOfEntries > 0)
	{
		for (i=0; i < si.numberOfEntries; ++i)			
		{
			/* <cbEntry><ei><file> */
			pEntries[i] = new Entry(p);
			assert(pEntries[i]);
			memcpy(&sizeEntryBlob, p, sizeof(sizeEntryBlob));
			p += sizeEntryBlob;
		}
	}
	else
		pEntries = NULL;

}

SystemInfo::~SystemInfo()
{
	U16 i;

	for (i=0; i < si.numberOfEntries; ++i)
	{
		delete pEntries[i];
	}
	delete []pEntries;
}

U32 SystemInfo::GetSysInfoData(void **pData_) const
{
   /*
	* build a blob containing the entry data.  
	* the blob is formatted as follows:
	* <total bytes><si><entryBlob1>...<entryBlobN>
	*/

	U32 cb = 0, i;

	cb = sizeof(cb) + sizeof(si);

	void **pEntryBlobs;
	U32 *pSizeBlobs;

	// first make the entry blobs
	pEntryBlobs = new void *[si.numberOfEntries];
	assert(pEntryBlobs);
	pSizeBlobs = new U32 [si.numberOfEntries];
	assert(pSizeBlobs);
	for (i=0; i < si.numberOfEntries; ++i)
	{
		pSizeBlobs[i] = pEntries[i]->GetEntryInfoData(&pEntryBlobs[i]);
		cb += pSizeBlobs[i];
	}

	*pData_ = new (tBIG) char[cb];  // allocate the blob.  
	assert(*pData_);

	char *p = (char *)*pData_;

	memcpy(p, &cb, sizeof(cb));	// total bytes
	p += sizeof(cb);	

	memcpy(p, &si, sizeof(si));	// System Info data
	p += sizeof(si);

	for (i=0; i < si.numberOfEntries; ++i)
	{
		memcpy(p, pEntryBlobs[i], pSizeBlobs[i]);
		delete pEntryBlobs[i];
		p += pSizeBlobs[i];
	}
	delete []pSizeBlobs;
	delete []pEntryBlobs;
	return cb;
}

// append an entry to the entry array
void SystemInfo::Append(Entry *pEntry)
{
	if (si.numberOfEntries % FILE_SYSTEM_ENTRY_GROW_SIZE == 0)
	{
		Entry** pTemp = pEntries;
		if (si.numberOfEntries == 0)		
			pEntries = new Entry *[FILE_SYSTEM_ENTRY_GROW_SIZE];
		else
			pEntries = new Entry *[si.numberOfEntries + FILE_SYSTEM_ENTRY_GROW_SIZE];
		assert(pEntries);
		for (U16 i = 0; i < si.numberOfEntries; i++)
			pEntries[i] = pTemp[i];
		delete pTemp;
	}
	pEntries[si.numberOfEntries++] = pEntry;
}


