#include "DdmPartitionMgr.h"
#include "BuildSys.h"
#include "MessageBroker.h"

#include "ChaosFileMsgs.h"
#include "PmIoContext.h"

#include <assert.h>

// Class Link Name used by Buildsys.cpp.  Must match CLASSENTRY in buildsys.cpp
CLASSNAME (DdmPartitionMgr, SINGLE);  

#ifndef _DEBUG
#define _DEBUG
#endif

#define	TRACE_INDEX		TRACE_PARTITIONMGR
#include "Odyssey_Trace.h"


//  Declare the messages served by this DDM

SERVELOCAL(DdmPartitionMgr, REQ_CHAOSFILE_CREATE);
SERVELOCAL(DdmPartitionMgr, REQ_CHAOSFILE_RENAME);
SERVELOCAL(DdmPartitionMgr, REQ_CHAOSFILE_RESIZE_MAX);
SERVELOCAL(DdmPartitionMgr, REQ_CHAOSFILE_SET_SIZE);
SERVELOCAL(DdmPartitionMgr, REQ_CHAOSFILE_DELETE);
SERVELOCAL(DdmPartitionMgr, REQ_CHAOSFILE_OPEN);
SERVELOCAL(DdmPartitionMgr, REQ_CHAOSFILE_CLOSE);
SERVELOCAL(DdmPartitionMgr, REQ_CHAOSFILE_READ);
SERVELOCAL(DdmPartitionMgr, REQ_CHAOSFILE_WRITE);

// Create the static flash device for the flash on the HBC
Flash_Device	*DdmPartitionMgr::devFSS = new HBC_Flash_Device();

inline void DdmPartitionMgr::IOQ::Push(PmIoContext *ioc_) // push onto the tail
{
	if (ioqHead == NULL || ioqTail == NULL)
		ioqHead = ioqTail = new IOQEntry;
	else
	{
		ioqTail->next = new IOQEntry;
		ioqTail = ioqTail->next;
	}
	ioqTail->ioc = ioc_;
	ioqTail->next = NULL;
}

inline	PmIoContext *DdmPartitionMgr::IOQ::Pop() // pop off the head
{
	if (ioqHead == NULL) return NULL;
	PmIoContext *ioc = ioqHead->ioc;
	ioqHead = ioqHead->next;
	if (ioqHead == NULL) ioqTail = NULL;
	return ioc;
}


MessageBroker *DdmPartitionMgr::msgBroker = NULL;
extern "C" int bzero(void *, size_t);

DdmPartitionMgr::DdmPartitionMgr (DID did_) : Ddm(did_), aOpenHandles(NULL), cOpenHandles(0), iocCurr(NULL)
{
	if (!msgBroker)
		msgBroker = new MessageBroker(this);
	memcpy(&config, Oos::GetBootData("DdmPartitionMgr"), sizeof(config));  

	// Initialize cache config.
	bzero(&config.cacheConfig, sizeof(CM_CONFIG));
	config.cacheConfig.version = CM_CONFIG_VERSION;
	config.cacheConfig.size = sizeof(CM_CONFIG);
	config.cacheConfig.p_table_memory = 0;
	config.cacheConfig.p_page_memory = 0;
	config.cacheConfig.num_pages = 0;
	config.cacheConfig.page_size = 2048;
	config.cacheConfig.page_table_size = 32768;
	config.cacheConfig.hash_table_size = 0;
	config.cacheConfig.num_reserve_pages = 64;
	config.cacheConfig.dirty_page_writeback_threshold = 60;
	config.cacheConfig.dirty_page_error_threshold = 95;
	config.cacheConfig.num_prefetch_forward = 0;
	config.cacheConfig.num_prefetch_backward = 0;

	// Initialize flash config.
	bzero(&config.flashConfig, sizeof(FF_CONFIG));
	config.flashConfig.version = FF_CONFIG_VERSION;
	config.flashConfig.size = sizeof(FF_CONFIG);
	config.flashConfig.p_device = devFSS;
	config.flashConfig.memory_size = 1048576;
	config.flashConfig.p_memory = new(tBIG | tUNCACHED) char[config.flashConfig.memory_size];
	config.flashConfig.verify_write = 1;
	config.flashConfig.verify_erase = 0;
	config.flashConfig.percentage_erased_pages = 1;
	config.flashConfig.verify_page_erased_before_write = 0;
	config.flashConfig.percentage_replacement_pages = 1;
	config.flashConfig.replacement_page_threshold = 25;
	config.flashConfig.erase_all_pages = 0;
	config.flashConfig.wear_level_threshold = 25;
	config.flashConfig.read_error_frequency_value = 0;
	config.flashConfig.erase_error_frequency_value  = 0;
	config.flashConfig.write_error_frequency_value = 0;
	config.flashConfig.test_all_static =0;
	config.flashConfig.test_all_random = 0;
	
    
    // Initialize callbacks.
	Callback_Context::Initialize(new (tBIG | tUNCACHED)char[1048576], 1048576, 140, this);

}



STATUS DdmPartitionMgr::Initialize(Message *pMsg_)
{
	TRACE_PROC(DdmPtsLoader::Initialize);

	DispatchRequest(REQ_CHAOSFILE_CREATE, REQUESTCALLBACK(DdmPartitionMgr, Create));
	DispatchRequest(REQ_CHAOSFILE_RENAME, REQUESTCALLBACK(DdmPartitionMgr, Rename));
	DispatchRequest(REQ_CHAOSFILE_RESIZE_MAX, REQUESTCALLBACK(DdmPartitionMgr, ResizeMax));
	DispatchRequest(REQ_CHAOSFILE_SET_SIZE, REQUESTCALLBACK(DdmPartitionMgr, SetSize));
	DispatchRequest(REQ_CHAOSFILE_DELETE, REQUESTCALLBACK(DdmPartitionMgr, Delete));
	DispatchRequest(REQ_CHAOSFILE_OPEN, REQUESTCALLBACK(DdmPartitionMgr, Open));
	DispatchRequest(REQ_CHAOSFILE_CLOSE, REQUESTCALLBACK(DdmPartitionMgr, Close));
	DispatchRequest(REQ_CHAOSFILE_READ, REQUESTCALLBACK(DdmPartitionMgr, Read));
	DispatchRequest(REQ_CHAOSFILE_WRITE, REQUESTCALLBACK(DdmPartitionMgr, Write));


	Reply(pMsg_, OK);
	return OK;
}



STATUS DdmPartitionMgr::Quiesce(Message *pMsg_)
{
	PmIoContext *ioc = new PmIoContext(this, ACTIONCALLBACK(DdmPartitionMgr, CloseFlash), 
							NULL, 0, 0, pMsg_, ACTIONCALLBACK(DdmPartitionMgr, SendReply));
	
	ioc->cPages = 0;
	ioc->length = 0;
	ioQueue.Push(ioc);
	ProcessIO();
	return OK;
}

STATUS DdmPartitionMgr::Enable(Message *pMsg_)
{
	TRACE_PROC(DdmPartitionMgr::Enable);
	ffContext.Initialize();
	ffContext.Set_Status(OK);
	ffContext.pData = pMsg_;
	ffContext.pDDM = this;
	
	cOpenHandles = 0;
	bSecondOpenAttempt = false; // try an open first, if it fails set this flag true and try again.
	ffContext.Set_Callback(&CbOpenFlash);
	
	TRACEF(TRACE_L2, ("DdmPartitionMgr::Enable(): Opening the Flash Storage System\n"));
	// Try to open the FSS.  If this fails, we'll create and format.
	STATUS stat = FF_Open(&config.flashConfig, &config.cacheConfig, (Callback_Context *)&ffContext, &hFlash);
	if (OK != stat)
	{
		Tracef("FF_Open failed with error code: %d.  Failing the board.\n", stat);
		Fail(); // fail the board if it didn't work.
	}
	return OK;
}

// static callback from the FSS
void DdmPartitionMgr::CbOpenFlash(void *pContext_, Status stat_)
{
	PmFfContext *pFfContext = (PmFfContext *)pContext_;
	DdmPartitionMgr *pDdm = pFfContext->pDDM;

	TRACEF(TRACE_L2, ("DdmPartitionMgr::CbOpenFlash(): Callback (%s) from FF_Open.\n", pDdm->bSecondOpenAttempt ? "Second Attempt" : "First Attempt"));
	
	// Check to see if the flash file system was opened.
	if (stat_ == OK)
		pDdm->LoadDirectory();
	else if (stat_ == CTS_FLASH_BAD_BLOCK_TABLE_DOES_NOT_EXIST) // failed we need to create the FSS.  This is temporary
	{		
		if (pDdm->bSecondOpenAttempt || !pDdm->config.allowCreate)
		{
			TRACEF(TRACE_L2, ("DdmPartitionMgr::CbOpenFlash(): The call to FF_Open failed.\n"));
			if (!pDdm->config.allowCreate)				
				TRACEF(TRACE_L2, ("  *** The Flash Storage System has not been created on this HBC.\n"));
			TRACEF(TRACE_L2, ("There was an unrecoverable error while attempting to open the Flash Storage System.  The HBC will now fail.\n"));
			pDdm->Fail(); // fail the HBC.  We've already tried creating the FSS, and the open failed again
			return;
		}
		else
		{
			pDdm->bSecondOpenAttempt = true;
			pFfContext->Initialize();
			pFfContext->Set_Callback(&CbCreateFlash);
			pFfContext->Set_Status(OK);
			STATUS stat = FF_Create(&pDdm->config.flashConfig, &pDdm->config.cacheConfig, pFfContext);
			Tracef("\n*** The Flash Storage System will now be created.  This can take a long, long time.  Be patient.\n");
	    	if (OK != stat)
	    	{
	 			Tracef("The call to FF_Create failed with error code: %d.  Failing the board.\n", stat);
	   			pDdm->Fail();	// Fail the board if it didn't work.
	    	}
	    }
	}
	else  // Format the flash, there is a problem (probably didn't close)
	{
		TRACEF(TRACE_L2, ("DdmPartitionMgr::CbOpenFlash(): An error occured when opening the Flash Storage System (probably due to a power off without closing the FSS)\n*** The FSS will now be reformatted, this may take a couple of minutes.\n"));
		//LogEvent
		pDdm->config.reformat = false;	
		STATUS stat = FF_Format(pDdm->hFlash, &pDdm->config.flashConfig, pFfContext, CbFormatFlash);
		if (OK != stat)
    	{
 			Tracef("FF_Format failed with error code: %d.  Failing the board.\n", stat);
    		pDdm->Fail();	// Fail the board if it didn't work.
    	}
	}
}

// static callback from the FSS
void DdmPartitionMgr::CbCreateFlash(void *pContext_, Status stat_)
{
	TRACEF(TRACE_L2, ("DdmPartitionMgr::CbCreateFlash(): Callback from FF_Create.\n"));

	PmFfContext *pFfContext = (PmFfContext *)pContext_;
	DdmPartitionMgr *pDdm = pFfContext->pDDM;	
	
	pFfContext->Set_Callback(&CbOpenFlash);

	if (stat_ != OK)
	{
		Tracef("FF_Create failed with error code: %d.  Failing the board.\n", stat_);
		pDdm->Fail(); // fail the HBC, there is nothing we can do from here
	}

	// Check to see if the flash file system was created
	// Try again to open the newly created FSS.  
	TRACEF(TRACE_L2, ("DdmPartitionMgr::CbCreateFlash(): Create was successful, trying open again.\n"));
	
	STATUS stat = FF_Open(&pDdm->config.flashConfig, &pDdm->config.cacheConfig, (Callback_Context *)pFfContext, &pDdm->hFlash);
	if (OK != stat)
	{
		Tracef("FF_Open failed with error code: %d.  Failing the board.\n", stat);
		pDdm->Fail(); // fail the board if it didn't work.
	}
}

// static callback from the FSS
void DdmPartitionMgr::CbFormatFlash(U32 /* cBytes_ */, I64 /* lbaFail_ */, STATUS stat_, void *pContext_)
{
	TRACEF(TRACE_L2, ("DdmPartitionMgr::CbFormatFlash(): Callback from FF_Format.\n"));
	PmFfContext *pFfContext = (PmFfContext *)pContext_;
	DdmPartitionMgr *pDdm = pFfContext->pDDM;	

	if (stat_ != OK)
	{
		Tracef("FF_Format failed with error code: %d.  Failing the board.\n", stat_);
		pDdm->Fail(); // fail the HBC, there is nothing we can do from here
	}
	pDdm->LoadDirectory();
}


STATUS DdmPartitionMgr::LoadDirectory()
{
	// We need to get the first page of the partition table to
	// figure out whether we need to create the system partitions.
	TRACEF(TRACE_L2, ("DdmPartitionMgr::LoadDirectory(): Checking for the existance of the partition table.\n"));
	
	PmIoContext *ioc = new PmIoContext(this, ACTIONCALLBACK(DdmPartitionMgr, ReadPages), 
							new char[config.bytesPerPage], 0, config.bytesPerPage, 
							(Message *)ffContext.pData, ACTIONCALLBACK(DdmPartitionMgr, BootStrap), TRUE);
	
	ioc->cPages = 1;
	ioc->aPgNums = new U32[ioc->cPages];
	ioc->aPgNums[0] = 0;
	ioc->length = config.bytesPerPage;
	ioQueue.Push(ioc);
	cHandle = 0;
	ProcessIO();
	return OK;
}

STATUS DdmPartitionMgr::Create(Message *pMsg_)
{
	assert(pMsg_->reqCode == REQ_CHAOSFILE_CREATE);
	ReqChaosFileCreate *pMsg = (ReqChaosFileCreate *)pMsg_;
	TRACEF(TRACE_L1, ("DdmPartitionMgr::Create(%s): \n", pMsg->name));

	int i;

	for (i = userPartitionStart; i < cCPE; ++i)
	{
		if (strncmp((pCPE + i)->name, pMsg->name, ChaosFile::cfNameLen) == 0)
		{
			Tracef("DdmPartitionMgr::Create(): Duplicate Name\n");
			pMsg->handle = CF_INVALIDHANDLE;
			pMsg->bytesLastIO = 0;
			pMsg->resultCode = cfDuplicateName;
			Reply(pMsg, pMsg->resultCode);
			return OK;
		}
	}

	if (strlen(pMsg->name) > ChaosFile::cfNameLen || strlen(pMsg->name) == 0)
	{
		Tracef("DdmPartitionMgr::Create(): Invalid Name\n");
		pMsg->handle = CF_INVALIDHANDLE;
		pMsg->bytesLastIO = 0;
		pMsg->resultCode = cfInvalidName;
		Reply(pMsg, pMsg->resultCode);
		return OK;
	}


	BOOL growPT = (pCPE + ptPartition)->cBytesUsed + sizeof(ChaosPartitionEntry) > (pCPE + ptPartition)->cMaxBlocks * config.bytesPerPage;

	// also need to search for deleted entries we can use.  TBD

	U32 pagesNeeded = (pMsg->maxBytes / config.bytesPerPage) + ((pMsg->maxBytes % config.bytesPerPage) ? 1 : 0);
	if (growPT) ++pagesNeeded;
	if (pagesNeeded > (pCPE + fplPartition)->cMaxBlocks)
	{
		Tracef("DdmPartitionMgr::Create(): Invalid Name\n");
		pMsg->handle = CF_INVALIDHANDLE;
		pMsg->bytesLastIO = 0;
		pMsg->resultCode = cfOutOfSpace;
		Reply(pMsg, pMsg->resultCode);
		return OK;
	}


	if (growPT)
	{
		TRACEF(TRACE_L1, ("DdmPartitionMgr::Create(): Partition table must be expanded\n"));
	
		// allocate another page to the partition table partition
		STATUS rc = GrowPartition(ptPartition, 1);
		assert (rc == cfSuccess);  // this is bad, we should have had enough room
		
		char *pTemp = new char[(pCPE + ptPartition)->cMaxBlocks * config.bytesPerPage];
		memcpy(pTemp, pPartitionTable, (pCPE + ptPartition)->cMaxBlocks * config.bytesPerPage);
		delete pPartitionTable;

		pPartitionTable = pTemp;
		pCPE = (ChaosPartitionEntry *)(pPartitionTable + sPtHdr);

		--pagesNeeded;
	}
	// we now know the new partition entry will fit, so move the last marker down and add the new entry
	
	memcpy(pCPE + cCPE, pCPE + cCPE - 1, sizeof(ChaosPartitionEntry));
	ChaosPartitionEntry *pPE = pCPE + cCPE - 1;
	strcpy (pPE->name, pMsg->name);
	pPE->cMaxBlocks = 0;
	pPE->cBytesUsed = 0;
	pPE->pNum = cCPE - 1;
	pPE->pType = ptSingleFilePartition;
	pPE->headLBA = pPE->tailLBA = 0;
	STATUS  rc = GrowPartition(cCPE - 1, pagesNeeded);
	assert (rc == cfSuccess);  // this is bad, we should have had enough room

	// Everything is now as it should be, we're ready to move on.
	AddHandle(pPE->pNum);

	pMsg->handle = cCPE - 1;
	pMsg->bytesLastIO = pPE->cMaxBlocks * config.bytesPerPage;
	pMsg->resultCode = cfSuccess;

	++cCPE;
	pPE = pCPE + ptPartition;  // point at the Partition table and update it
	pPE->cBytesUsed += sizeof(ChaosPartitionEntry);
	
	//TRACEF(TRACE_L2, ("DdmPartitionMgr::LoadDirectory()\n%s", DumpPartitionTable()));
	

	ioQueue.Push(UpdateSystemTables(pMsg));
	ProcessIO();

	return OK;
}

PmIoContext *DdmPartitionMgr::UpdateSystemTables(Message *pMsg_)
{
	TRACEF(TRACE_L2, ("DdmPartitionMgr::UpdateSystemTables()\n"));
	PmIoContext *iocPT = new PmIoContext(this, ACTIONCALLBACK(DdmPartitionMgr, WritePages), 
							pPartitionTable, 0, (pCPE + ptPartition)->cBytesUsed, 
							pMsg_, ACTIONCALLBACK(DdmPartitionMgr, SendReply));
	iocPT->SetPageLinks(config.bytesPerPage, pPLT, (pCPE + ptPartition)->headLBA);
	PmIoContext *iocPLT = new PmIoContext(this, ACTIONCALLBACK(DdmPartitionMgr, WritePages), 
							pPageMap, 0, (pCPE + pltPartition)->cBytesUsed, 
							pMsg_, ACTIONCALLBACK(DdmPartitionMgr, SendReply));
	iocPLT->SetPageLinks(config.bytesPerPage, pPLT, (pCPE + pltPartition)->headLBA);
	iocPT->next = iocPLT;

	return iocPT;
}

bool DdmPartitionMgr::FindHandle(U32 handle_)
{
	return aOpenHandles[handle_];
}

void DdmPartitionMgr::AddHandle(U32 handle_)
{
	TRACEF(TRACE_L2, ("DdmPartitionMgr::AddHandle(%d): mark handle as open\n", handle_));
	if (handle_ >= cHandle)
	{
		bool *aTemp = new(tZERO) bool[cHandle + HANDLE_ARRAY_GROW];
		if (cHandle)
			memcpy(aTemp, aOpenHandles , sizeof(bool) * cHandle);
		cHandle += HANDLE_ARRAY_GROW;
		delete []aOpenHandles;
		aOpenHandles = aTemp;
	}
	if (!aOpenHandles[handle_])
		++cOpenHandles;
	
	aOpenHandles[handle_] = true;  // mark as open

}

void DdmPartitionMgr::DeleteHandle(U32 handle_)
{
	TRACEF(TRACE_L2, ("DdmPartitionMgr::DeleteHandle(%d): mark handle as closed\n", handle_));
	if (handle_ <= cHandle)
	{
		if (aOpenHandles[handle_])
			--cOpenHandles;
		aOpenHandles[handle_] = false;  // mark as closed
	}
}


// update the partition table so that partition pNum_ has addlPages_ more pages.  Does
// not do I/O.

STATUS DdmPartitionMgr::GrowPartition(U16 pNum_, U32 addlPages_)
{
	TRACEF(TRACE_L2, ("DdmPartitionMgr::GrowPartition(%d (partition), %d (pages)): \n", pNum_, addlPages_));
	if (addlPages_ == 0)
		return OK;

	// move pages from the free list to the specified partition
	if ((pCPE + fplPartition)->cMaxBlocks < addlPages_)
		return cfOutOfSpace;

	ChaosPartitionEntry *pPE = pCPE + pNum_;
	U32 next = (pCPE + fplPartition)->headLBA;

	if (pPE->cMaxBlocks)  // do we already have a chain?
		*(pPLT + pPE->tailLBA) = next; // link to the start of the free chain
	else
		pPE->headLBA = next; // point the head to the start of the free chain

	for (int i = 1; i < addlPages_; ++i)
	{
		assert(next != 0);
		next = *(pPLT + next);
	}

	(pCPE + fplPartition)->headLBA = *(pPLT + next);  // move the start of the free chain
	(pCPE + fplPartition)->cMaxBlocks -= addlPages_;
	*(pPLT + next) = 0;  // snip off the chain on the resized partition
	(pCPE + pNum_)->tailLBA = next;  
	(pCPE + pNum_)->cMaxBlocks += addlPages_;
	return OK;
}


void DdmPartitionMgr::ProcessIO()
{
	if (!iocCurr)
	{
		iocCurr = ioQueue.Pop();
		if (iocCurr)
		{
			TRACEF(TRACE_L3, ("DdmPartitionMgr::ProcessIO(): Popped the next I/O operation off the queue\n"));
			Action(iocCurr->acHandler, iocCurr);
		}
		else
		{		
			TRACEF(TRACE_L3, ("DdmPartitionMgr::ProcessIO(): IO Queue is empty\n"));
		}
	}
}

void DdmPartitionMgr::NextIO(PmIoContext *ioc_)
{
	assert (ioc_);
	TRACEF(TRACE_L3, ("DdmPartitionMgr::NextIO(): Checking for I/O sub-operation: "));
	iocCurr = ioc_->next;
	if (!iocCurr)
	{
		TRACEF(TRACE_L3, ("I/O operation is complete\n"));
		Action(ioc_->acComplete, ioc_);
		ProcessIO();
	}
	else
	{
		delete ioc_;
		TRACEF(TRACE_L3, ("I/O operation is continuing\n"));
		Action(iocCurr->acHandler, iocCurr);
	}
}


STATUS DdmPartitionMgr::Open(Message *pMsg_)
{
	ChaosPartitionEntry *pPE;

	ReqChaosFileOpen *pMsg = (ReqChaosFileOpen *) pMsg_;
	TRACEF(TRACE_L1, ("DdmPartitionMgr::Open(%s)\n", pMsg->name));

	for (int i = userPartitionStart; i < cCPE; ++i)
	{
		pPE = pCPE + i;
		if (strcmp(pPE->name, pMsg->name) == 0)
		{
			if (!FindHandle(pPE->pNum)) // not already in the set
			{
				AddHandle(pPE->pNum);  // add to the set
				pMsg->handle = pPE->pNum;
				pMsg->bytesLastIO = 0;
				pMsg->resultCode = cfSuccess;
				pMsg->size = (pCPE + pMsg->handle)->cBytesUsed;
				pMsg->maxSize = (pCPE + pMsg->handle)->cMaxBlocks * config.bytesPerPage;
				Reply(pMsg, pMsg->resultCode);
			}
			else
			{
				Tracef("DdmPartitionMgr::Open(): Already Open\n");
			
				pMsg->handle = CF_INVALIDHANDLE;
				pMsg->bytesLastIO = 0;
				pMsg->resultCode = cfFileAlreadyOpen;
				Reply(pMsg, pMsg->resultCode);
			}

			return OK;
		}
	}
	pMsg->handle = CF_INVALIDHANDLE;
	pMsg->bytesLastIO = 0;
	pMsg->resultCode = cfFileNotFound;
	Tracef("DdmPartitionMgr::Open(): Not found\n");
	Reply(pMsg, pMsg->resultCode);
	return OK;
}

STATUS DdmPartitionMgr::Rename(Message *pMsg_)
{
	TRACEF(TRACE_L1, ("DdmPartitionMgr::Rename(): Not yet implemented\n"));
	Reply(pMsg_, OK);
	return OK;
}

STATUS DdmPartitionMgr::Delete(Message *pMsg_)
{
	TRACEF(TRACE_L1, ("DdmPartitionMgr::Delete(): Not yet implemented\n"));
	Reply(pMsg_, OK);
	return OK;
}

STATUS DdmPartitionMgr::Close(Message *pMsg_)
{
	ReqChaosFileClose *pMsg = (ReqChaosFileClose *)pMsg_;
	TRACEF(TRACE_L1, ("DdmPartitionMgr::Close(%d)\n", pMsg->handle));

	if (!FindHandle(pMsg->handle)) // not open !
	{
		TRACEF(TRACE_L1, ("DdmPartitionMgr::Close(%d): Not open!\n", pMsg->handle));

		pMsg->resultCode = cfInvalidHandle;
	}
	else
	{
		pMsg->resultCode = cfSuccess;
		DeleteHandle(pMsg->handle);
	}

	if (cOpenHandles == userPartitionStart)  // no more open handles..  Quiesce ourselves so that the flash gets flushed
	{
		TRACEF(TRACE_L1, ("DdmPartitionMgr::Close(%d): Last handle has closed, Quiescing\n", pMsg->handle));
		Message *pMsgQuiesce = new Message(REQ_OS_DDM_QUIESCE);
		Send(GetDid(), pMsgQuiesce, REPLYCALLBACK(DdmServices, DiscardReply));
	}

	pMsg->bytesLastIO = 0;
	pMsg->size = (pCPE + pMsg->handle)->cBytesUsed;
	pMsg->maxSize = (pCPE + pMsg->handle)->cMaxBlocks * config.bytesPerPage;
	Reply(pMsg, pMsg->resultCode);
	

	return OK;
}

STATUS DdmPartitionMgr::ResizeMax(Message *pMsg_)
{
	TRACEF(TRACE_L1, ("DdmPartitionMgr::ResizeMax(): Not yet implemented\n"));
	Reply(pMsg_, OK);
	return OK;
}

STATUS DdmPartitionMgr::SetSize(Message *pMsg_)
{
	TRACEF(TRACE_L3, ("DdmPartitionMgr::SetSize()\n"));

	ReqChaosFileSetSize *pMsg = (ReqChaosFileSetSize *)pMsg_;
	TRACEF(TRACE_L3, ("DdmPartitionMgr::DdmPartitionMgr::SetSize(): handle=%d, size=%d\n", pMsg->handle, pMsg->size));

	// check for bad handle - TBD
	ChaosPartitionEntry *pPE = pCPE + pMsg->handle;


	if (pMsg->size >= pPE->cMaxBlocks * config.bytesPerPage)
	{
		pMsg->resultCode = cfOutOfSpace;
		Reply(pMsg, OK);
	}
	else
	{
		pMsg->resultCode = cfSuccess;
		pPE->cBytesUsed = pMsg->size;
		ioQueue.Push(UpdateSystemTables(pMsg));
		ProcessIO();
	}
		
		
	return OK;
}

STATUS DdmPartitionMgr::Read(Message *pMsg_)
{
	ReqChaosFileRead *pMsg = (ReqChaosFileRead *)pMsg_;
	TRACEF(TRACE_L3, ("DdmPartitionMgr::Read(): handle=%d, offset=%d, length=%d\n", pMsg->handle, pMsg->offset, pMsg->length));

	void *buffer;
	U32 size;

	// check for bad handle - TBD
	ChaosPartitionEntry *pPE = pCPE + pMsg->handle;

	if (pMsg->length == 0)
	{
		pMsg->size = pPE->cBytesUsed;
		pMsg->maxSize = pPE->cMaxBlocks * config.bytesPerPage;

		Reply(pMsg, OK);
		return OK;
	}

	if (pMsg->offset + pMsg->length > pPE->cBytesUsed)
	{
		assert(false);		// Cannot read that much.
	}

	// this should be replaced with calls to CopyFromSgl at some point - TBD
	pMsg->GetSgl(0, &buffer, &size);
	assert(size == pMsg->length);
	PmIoContext *ioc = new PmIoContext(this, ACTIONCALLBACK(DdmPartitionMgr, ReadPages), 
							buffer, pMsg->offset, pMsg->length, 
							pMsg_, ACTIONCALLBACK(DdmPartitionMgr, SendReply));
	ioc->SetPageLinks(config.bytesPerPage, pPLT, pPE->headLBA);
	ioQueue.Push(ioc);
	ProcessIO();

	return OK;
}

STATUS DdmPartitionMgr::Write(Message *pMsg_)
{
	ReqChaosFileWrite *pMsg = (ReqChaosFileWrite *)pMsg_;
	TRACEF(TRACE_L3, ("DdmPartitionMgr::Write(): handle=%d, offset=%d, length=%d\n", pMsg->handle, pMsg->offset, pMsg->length));

	// check for bad handle - TBD
	ChaosPartitionEntry *pPE = pCPE + pMsg->handle;

	if (pMsg->length == 0)
	{
		pMsg->size = pPE->cBytesUsed;
		pMsg->maxSize = pPE->cMaxBlocks * config.bytesPerPage;

		Reply(pMsg, OK);
		return OK;
	}
	
	

	void *buffer;
	U32 size;

	
	// this should be replaced with calls to CopyFromSgl at some point - TBD
	pMsg->GetSgl(0, &buffer, &size);
	assert(size == pMsg->length);

	if (pMsg->offset + pMsg->length > pPE->cMaxBlocks * config.bytesPerPage)
	{
			assert(false);		// Partition Full
	}

	PmIoContext *ioc = new PmIoContext(this, ACTIONCALLBACK(DdmPartitionMgr, WritePages), 
							buffer, pMsg->offset, pMsg->length, 
							pMsg_, ACTIONCALLBACK(DdmPartitionMgr, SendReply));
	ioc->SetPageLinks(config.bytesPerPage, pPLT, pPE->headLBA); 

	if (pMsg->offset + pMsg->length > pPE->cBytesUsed)
	{
		pPE->cBytesUsed = pMsg->offset + pMsg->length;
		// update the system tables before replying.
		ioc->next = UpdateSystemTables(pMsg);
	}
	ioQueue.Push(ioc);
	ProcessIO();
	return OK;
}


// ActionCallbacks

STATUS DdmPartitionMgr::BootStrap(void *payload_)
{
	// We've read page zero, we now need to determine whether there is
	// a partition table.  If not, we create one.  If there is, we read the
	// appropriate info.

	TRACEF(TRACE_L2, ("DdmPartitionMgr::BootStrap(): Checking the first page of data\n"));

	assert(payload_ != NULL);

	// number of pages in the page link table
	U16 pltPages = config.cPages * sizeof(U32) / config.bytesPerPage;
	if (config.cPages * sizeof(U32) % config.bytesPerPage != 0)
		++pltPages;

	pPageMap = new char[config.bytesPerPage * pltPages];
	pPLT = (U32 *)pPageMap;


	PmIoContext *ioc = (PmIoContext *)payload_; 

	if (config.reformat)
		Tracef("DdmPartitionMgr::BootStrap():  The reformat flag is set, erasing any existing partitions.\n");
	if (!config.reformat && memcmp(ioc->buffer, "CPT\0x01", 4) == 0)  // partition table exists
	{
		config.reformat = false;
		TRACEF(TRACE_L2, ("DdmPartitionMgr::BootStrap(): Partition table exists, read the whole thing\n"));
		// point to the first partition record in the page we read
		pCPE = (ChaosPartitionEntry *)((char *)ioc->buffer + sPtHdr);
		// allocate memory for the partition table		
		pPartitionTable = new char[config.bytesPerPage * (pCPE + ptPartition)->cMaxBlocks];		
		// copy the part of the PT that we know about
		memcpy(pPartitionTable, (char *)ioc->buffer, config.bytesPerPage);
		// point to where the first partition record is going to be
		pCPE = (ChaosPartitionEntry *)(pPartitionTable + sPtHdr);
		// open the handles for the system partitions
		AddHandle(pltPartition);
		AddHandle(ptPartition);
		AddHandle(fplPartition);				

		// This will fail if the Partition Table ever exceeds a page.  For now, just use the page
		// we got on the initial read.  This is a quick hack to  get Sherri up and running, 
		// I will need to fix this to read the Page Link Table and then the Partition Table ASAP.  RJB						
//		PmIoContext *iocPT = new PmIoContext(this, ACTIONCALLBACK(DdmPartitionMgr, ReadPages), 
//								pPartitionTable, 0, (pCPE + ptPartition)->cBytesUsed, 
//								ioc->pReplyMsg, ACTIONCALLBACK(DdmPartitionMgr, SendReply));
//		iocPT->SetPageLinks(config.bytesPerPage, pPLT, (pCPE + ptPartition)->headLBA);

		PmIoContext *iocPLT = new PmIoContext(this, ACTIONCALLBACK(DdmPartitionMgr, ReadPages), 
								pPageMap, 0, (pCPE + pltPartition)->cBytesUsed, 
								ioc->pReplyMsg, ACTIONCALLBACK(DdmPartitionMgr, SendReply));
//		iocPLT->SetPageLinks(config.bytesPerPage, pPLT, (pCPE + pltPartition)->headLBA);
		
//		iocPT->next = iocPLT;
		iocPLT->cPages = pltPages;
		iocPLT->aPgNums = new U32[iocPLT->cPages];
		for (int i=0; i< pltPages; ++i)
			iocPLT->aPgNums[i] = i + 1;
		
		// All set now, read the the partition and page tables:
		ioQueue.Push(iocPLT);
		ProcessIO();
	}
	else  // create the partition table
	{
		TRACEF(TRACE_L2, ("DdmPartitionMgr::BootStrap(): Create the partition table\n"));

		// allocate memory for the partition table
		pPartitionTable = new char[config.bytesPerPage];
		// point to the first partition record
		pCPE = (ChaosPartitionEntry *)(pPartitionTable + sPtHdr);

		
		// tag the partition table
		memcpy(pPartitionTable, "CPT\0x01", sPtHdr);

		ChaosPartitionEntry *pPE;
		// build partition 0, the PageLinkTable
		pPE = pCPE + pltPartition;
		pPE->pNum = 0;
		pPE->pType = ptPltType;
		pPE->cMaxBlocks = pltPages; 
		pPE->headLBA = 1;
		pPE->tailLBA = pPE->headLBA + pPE->cMaxBlocks - 1;
		pPE->cBytesUsed = pltPages * config.bytesPerPage;
		strcpy(pPE->name, "ChaosPLT");
		AddHandle(pPE->pNum);

		// build partition 1, the Partition Table 
		pPE = pCPE + ptPartition;
		pPE->pNum = 1;
		pPE->pType = ptPtType;
		pPE->cMaxBlocks = 1; 
		pPE->headLBA = 0;
		pPE->tailLBA = 0;
		pPE->cBytesUsed = 4 + 4 * sizeof(ChaosPartitionEntry);
		strcpy(pPE->name, "ChaosPT");
		AddHandle(pPE->pNum);

		// build partition 2, the Free Page List 
		pPE = pCPE + fplPartition;
		pPE->pNum = 2;
		pPE->pType = ptFplType;
		pPE->cMaxBlocks = config.cPages - ((pCPE + ptPartition)->cMaxBlocks + (pCPE + pltPartition)->cMaxBlocks);
		pPE->headLBA = (pCPE + pltPartition)->tailLBA + 1;
		pPE->tailLBA = config.cPages - 1;
		pPE->cBytesUsed = 0;
		strcpy(pPE->name, "ChaosFPL");
		AddHandle(pPE->pNum);

		// build partition 3, the "end" marker 
		pPE = pCPE + userPartitionStart;
		pPE->pNum = -1;
		pPE->pType = ptLast;
		pPE->cMaxBlocks = pPE->headLBA = pPE->tailLBA = pPE->cBytesUsed = 0;
		strcpy(pPE->name, "ChaosLast");

		cCPE = 4;
		// build the page link table

//		pPLT = (U32 *)pPageMap;
		int i;
		
		*pPLT = 0; // the partition table page, a single page initially
		pPE = pCPE + pltPartition;

		// the PLT partition itself
		for (i = 1; i <= pPE->cMaxBlocks; ++ i)
			*(pPLT + i) = i + 1;  // point to i+1, the next in the list
		*(pPLT + pPE->cMaxBlocks) = 0; // last page in PLT

		// the free page list
		for (i = pPE->cMaxBlocks + 1; i < config.cPages - 1; ++ i)
			*(pPLT + i) = i + 1; // point to i+1, the next in the list
		*(pPLT + config.cPages - 1) = 0;

		ioQueue.Push(UpdateSystemTables(ioc->pReplyMsg));
		ProcessIO();
	}

	delete ioc;
	ProcessIO();
	
	return OK;
}

STATUS DdmPartitionMgr::SendReply(void *payload_)
{
	assert(payload_ != NULL);
	TRACEF(TRACE_L2, ("DdmPartitionMgr::SendReply()\n"));

	PmIoContext *ioc = (PmIoContext *)payload_;

	if (MASK_REQUEST_RANGE(ioc->pReplyMsg->reqCode) == REQ_CHAOSFILE)
	{
		U16 handle = ((ReqChaosFile *)ioc->pReplyMsg)->handle;
		((ReqChaosFile *)ioc->pReplyMsg)->size = (pCPE + handle)->cBytesUsed;
		((ReqChaosFile *)ioc->pReplyMsg)->maxSize = (pCPE + handle)->cMaxBlocks * config.bytesPerPage;
	}

	Reply(ioc->pReplyMsg, OK);

	delete ioc;
	ProcessIO();

	return OK;
}

STATUS DdmPartitionMgr::WritePages(void *payload_)
{
	assert(payload_ != NULL);

	PmIoContext *ioc = (PmIoContext *)payload_;

	TRACEF(TRACE_L8, ("DdmPartitionMgr::WritePages()\n"));

	if (ioc->workBuffer)  // we've already done the read, ioc->workBuffer points to the result.  Otherwise drop thru to the read
	{
		TRACEF(TRACE_L8, ("DdmPartitionMgr::WritePages(): Writing page %d (LBA %d)\n", ioc->currPage, ioc->aPgNums[ioc->currPage]));
		
		U32 offsPage = 0;
		if (ioc->currPage == 0)
			offsPage = ioc->offset % config.bytesPerPage;
		U32 lenPage = config.bytesPerPage;
		if (ioc->currPage == ioc->cPages - 1 && (ioc->offset + ioc->length) % config.bytesPerPage != 0)
			lenPage = (ioc->offset + ioc->length) % config.bytesPerPage;
		
		if (offsPage != 0 || lenPage != config.bytesPerPage)
			TRACEF(TRACE_L8, ("DdmPartitionMgr::WritePages(): Writing partial page from %d to %d (write offset = %d, write length = %d)\n", offsPage, lenPage, ioc->offset, ioc->length));
		if (memcmp((char *)ioc->buffer + (config.bytesPerPage * ioc->currPage) + offsPage, (char *)ioc->workBuffer + offsPage, lenPage) != 0)
	 		FF_Write(hFlash, (char *)ioc->buffer + (config.bytesPerPage * ioc->currPage) + offsPage, 
	 			lenPage, ioc->aPgNums[ioc->currPage] * config.bytesPerPage + offsPage, ioc, CbWriteFlash);
		else
		{
			TRACEF(TRACE_L8, ("DdmPartitionMgr::WritePages(): data unchanged, skipping the write.\n"));
	//		NextIO(ioc);
		}
		++ioc->currPage;  // now we'll queue up a simultaneous read.
	}
	// Queue up a read for the current page, unless we just issued the last write operation
	if (ioc->currPage < ioc->cPages) 
	{
		TRACEF(TRACE_L8, ("DdmPartitionMgr::WritePages(): Queueing a read for page %d\n", ioc->aPgNums[ioc->currPage]));
	
		if (!ioc->workBuffer) // allocate if not already done
			ioc->workBuffer = new char[config.bytesPerPage];  // allocate a page for the read
		// read this page so we can compare before writing
		PmIoContext *iocRead = new PmIoContext(this, ACTIONCALLBACK(DdmPartitionManager, ReadPages), 
			ioc->workBuffer, 0, config.bytesPerPage, ioc->pReplyMsg, ioc->acComplete, false);
		iocRead->aPgNums = new U32[1];
		iocRead->aPgNums[0] = ioc->aPgNums[ioc->currPage];
		iocRead->length = config.bytesPerPage;
		iocRead->cPages = 1;
		iocRead->next = ioc; // call back here once the read is done
		iocCurr = iocRead;
		Action(iocRead->acHandler, iocRead);  // Do the read.  Don't requeue it on the ioQueue, since this is a subset of the current IO operation
	}
	else
		NextIO(ioc);  // done with this I/O, continue on
	
	return OK;
}


// static callback from the FSS
void DdmPartitionMgr::CbWriteFlash(U32 /* cBytes_ */, I64 /* lbaFail_ */, STATUS stat_, void *pContext_)
{
	TRACEF(TRACE_L8, ("DdmPartitionMgr::CbWriteFlash(): Status= %d\n", stat_));
	// error handling here.
	
	// Don't actually do anything.  If there are more pages, there will be a parallel read operation going on that will cause the next write
//	PmIoContext *ioc = (PmIoContext *)pContext_;

	assert(pContext_ != NULL);

//	++ioc->currPage;
//	if (ioc->currPage < ioc->cPages) 
//		WritePages(ioc);
//	else
//		ioc->pDdm->NextIO(ioc);  // this should close out the operation
}

STATUS DdmPartitionMgr::ReadPages(void *payload_)
{
	assert(payload_ != NULL);

	PmIoContext *ioc = (PmIoContext *)payload_;
	TRACEF(TRACE_L8, ("DdmPartitionMgr::ReadPages(): %d pages\n", ioc->cPages));

	ioc->cPending = ioc->cPages;
	U32 cPages = ioc->cPages; // avoid race cond.
	for (int i=0; i < cPages; ++i)
 		FF_Read(hFlash, (char *)ioc->buffer + (config.bytesPerPage * i), config.bytesPerPage, ioc->aPgNums[i] * config.bytesPerPage, ioc, CbReadFlash);
 	return OK;
}

// static callback from the FSS
void DdmPartitionMgr::CbReadFlash(U32 /* cBytes_ */, I64 /* lbaFail_ */, STATUS stat_, void *pContext_)
{
	// error handling here.

	PmIoContext *ioc = (PmIoContext *)pContext_;
	TRACEF(TRACE_L8, ("DdmPartitionMgr::CbReadFlash(): Status=%d, pending=%d\n", stat_, ioc->cPending-1));

	assert (pContext_ != NULL);
	
	if (--ioc->cPending == 0)
		ioc->pDdm->NextIO(ioc);
}


STATUS DdmPartitionMgr::CloseFlash(void *payload_)
{
	assert(payload_ != NULL);
	TRACEF(TRACE_L2, ("DdmPartitionMgr::CloseFlash()\n"));
	

	PmIoContext *ioc = (PmIoContext *)payload_;
	
	ffContext.Initialize();
	ffContext.Set_Status(OK);
	ffContext.pData = ioc;
	ffContext.pDDM = this;
	
	ffContext.Set_Callback(&CbCloseFlash);

	FF_Close(hFlash, &ffContext);
	return OK;
}

// static callback from the FSS
void DdmPartitionMgr::CbCloseFlash(void *pContext_, Status /*stat_*/)
{
	// error handling here.  What shoudl we do?
	TRACEF(TRACE_L2, ("DdmPartitionMgr::CbCloseFlash()\n"));

	assert (pContext_ != NULL);

	PmFfContext *pFfContext = (PmFfContext *)pContext_;
	pFfContext->pDDM->Cleanup((PmIoContext *)pFfContext->pData);
}
	
void DdmPartitionMgr::Cleanup(PmIoContext *ioc_)
{
	TRACEF(TRACE_L2, ("DdmPartitionMgr::Cleanup()\n"));

	NextIO(ioc_);  // this should cause the reply to be sent and clean up

	delete pPageMap;
	delete pPartitionTable;
	delete []aOpenHandles;
	pFssMemory = NULL;
	pPageMap = NULL;
	pPartitionTable = NULL;
	cOpenHandles = 0;
	aOpenHandles = NULL;
}
