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
*	This file contains the declaration of the Partition Manager DDM
* 
* Update Log: 
* 06/07/99 Bob Butler: Create file
* 08/02/99 Bob Butler: Removed STL classes, since Mwerks doesn't work with them
* 08/03/99 Bob Butler: Moved PmIoContext into PmIoContext.h, removed in-memory test code
*
*************************************************************************/


#ifndef DdmPartionMgr_h
#define DdmPartionMgr_h

#include "ddm.h"
#include "MessageBroker.h"
#include "HbcFlashDevice.h"
#include "FlashStorage.h"
#include "Callback.h"
#include "ChaosFile.h"

struct PmConfig
{
	U32 cPages;
	U16 bytesPerPage;
	FF_CONFIG flashConfig;
	CM_CONFIG cacheConfig;
	U32 reformat; // temp for testing
	U32 allowCreate;
};
 
class PmIoContext;  // fwd ref of the IO context object

class DdmPartitionMgr : public Ddm
{

public:

   //  Constructor
	DdmPartitionMgr (DID did_);

   //  Create an instance of this DDM
	static Ddm *Ctor (DID did_) { return new DdmPartitionMgr(did_); }

	virtual STATUS Initialize (Message *pMsg_);
	virtual STATUS Quiesce (Message *pMsg_);
	virtual STATUS Enable (Message *pMsg_);

	// send and wait for a reply using MessageBroker.  
	static STATUS SendWait(Message *pMsg_) { return msgBroker->SendWait(pMsg_); }

protected:
	enum PartitionTypes { ptPltType, ptPtType, ptFplType, ptSingleFilePartition, ptLast = -1 };
	enum PartitionNumbers {ptPartition = 0, pltPartition = 1, fplPartition = 2, userPartitionStart = 3 };


	STATUS Create(Message *pMsg_);
	STATUS Delete(Message *pMsg_);

	STATUS Open(Message *pMsg_);
	STATUS Close(Message *pMsg_);

	STATUS Read(Message *pMsg_);
	STATUS Write(Message *pMsg_);

	STATUS Rename(Message *pMsg_);

	STATUS ResizeMax(Message *pMsg_);
	STATUS SetSize(Message *pMsg_);

private:

	PmIoContext *UpdateSystemTables(Message *pMsg_);
	void 	ProcessIO();
	void 	NextIO(PmIoContext *ioc_);

	STATUS 	GrowPartition(U16 pNum_, U32 addlPages);

	STATUS 	LoadDirectory();
	STATUS 	BootStrap(void *payload_);
	
	STATUS 	SendReply(void *payload_);
	STATUS 	WritePages(void *payload_);
	STATUS 	ReadPages(void *payload_);
	STATUS 	CloseFlash(void *payload_);

	bool 	FindHandle(U32 handle_);
	void 	AddHandle(U32 handle_);
	void 	DeleteHandle(U32 handle_);
	void 	Cleanup(PmIoContext *);
		
	static Flash_Device	*devFSS;
	static MessageBroker *msgBroker;
	static void CbOpenFlash(void *pContext_, Status stat_);
	static void CbCloseFlash(void *pContext_, Status stat_);
	static void CbCreateFlash(void *pContext_, Status stat_);
	static void CbFormatFlash(U32 cBytes_, I64 lbaFail_, STATUS stat_, void *pContext_);
	static void CbReadFlash(U32 cBytes_, I64 lbaFail_, STATUS stat_, void *pContext_);
	static void CbWriteFlash(U32 cBytes_, I64 lbaFail_, STATUS stat_, void *pContext_);
	

	// Define the structure of the partition record
	struct ChaosPartitionEntry
	{
		U16 pNum;
		U16 pType;
		char name[ChaosFile::cfNameLen];
		U32 cMaxBlocks;
		U32 cBytesUsed;
		U32 headLBA;
		U32 tailLBA;
	} *pCPE;

	// extend the callback class for use with the Flash Storage System.
	class PmFfContext : public Callback_Context
	{
	public:
		void *pData;
		DdmPartitionMgr *pDDM;
	} ffContext;

	// Queue of IO operations.  Note that the PmIoContext also has a next pointer. The list
	// within the PmIoContext is a list of sub-operations.  This is the list of primary operations
	// Implement a simplistic queue, since I can't use the STL queue
	struct IOQ {
		struct IOQEntry {
			PmIoContext *ioc;
			IOQEntry	*next;
		} *ioqHead, *ioqTail;
		IOQ() : ioqHead(NULL), ioqTail(NULL) { }
		void Push(PmIoContext *ioc_); // push onto the tail
		PmIoContext *Pop(); // pop off the head
	} ioQueue;
	
	bool 	*aOpenHandles; 
	U32 	cOpenHandles;
	U32 	cHandle;
	char 	*pPartitionTable;
	U16 	cCPE;

	char 	*pPageMap;
	void 	*pCbContexts, *pFssMemory;
	FF_HANDLE hFlash;
	bool 	bSecondOpenAttempt;
	
	PmIoContext *iocCurr;

	U32 *pPLT;

	PmConfig config;
	enum {HANDLE_ARRAY_GROW = 20}; 
	enum {sPtHdr = 4};  // number of bytes for the partition table header tag ("CPT<ver>")

};
#endif