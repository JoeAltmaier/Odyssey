/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DdmRaid.cpp
//
// Description:	Raid DDM class
//
//
// Update Log: 
//	2/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#include "OsTypes.h"
#include "Message.h"
#include "CtTypes.h"
#include "Ddm.h"
#include "RaidStructs.h"
#include "RaidQ.h"
#include "UtilQ.h"
#include "Member.h"
#include "UtilCmd.h"
#include "RaidIorQ.h"
#include "RaidLock.h"
#include "RaidErr.h"
#include "Raid.h"
#include "Raid1.h"
#include "Raid0.h"
#include "RaidCat.h"
#include "OsStatus.h"
#include "RequestCodes.h"
#include "BuildSys.h"
#include "DdmRaid.h"
#include "Scsi.h"
#include "RqDdmReporter.h"

CLASSNAME(DdmRaid, MULTIPLE);

Raid	*pRaidPtr;

/*************************************************************************/
// DdmRaid
// Constructor method for the class DdmRaid
/*************************************************************************/

DdmRaid::DdmRaid(DID did):Ddm(did)
{ 
	TRACE_ENTRY(DdmRaid::DdmRaid);

	RaidDID = did;			// save for PHS Reporter

// temp
#if 0
ArrayDescriptor.thisRID.LoPart = 0;
SetConfigAddress(&TmpConfig, sizeof(TempConfiguration));

// temp
#endif
	SetConfigAddress(&ArrayDescriptor, sizeof(RAID_ARRAY_DESCRIPTOR));
}

/*************************************************************************/
// ~DdmRaid
// Destructor method for the class DdmRaid
/*************************************************************************/

DdmRaid::~DdmRaid()
{ 
	if (pRaid)
		delete pRaid;
}

/*************************************************************************/
// Ctor
// Create a new instance of DdmRaid
/*************************************************************************/

Ddm *DdmRaid::Ctor(DID did)
{
	TRACE_ENTRY(DdmRaid::Ctor);

	return new DdmRaid(did);
}

/*************************************************************************/
// Initialize
//
/*************************************************************************/

STATUS	DdmRaid::Initialize(Message *pMsg)
{
	TRACE_ENTRY(DdmRaid::Initialize);

	// Set dispatch routines for BSA Messages
	DispatchRequest(BSA_BLOCK_READ,(RequestCallback) &DdmRaid::BlockRead);
	DispatchRequest(BSA_BLOCK_WRITE,(RequestCallback) &DdmRaid::BlockWrite);
	DispatchRequest(BSA_BLOCK_WRITE_VERIFY,(RequestCallback) &DdmRaid::BlockWrite);
	DispatchRequest(BSA_BLOCK_REASSIGN,(RequestCallback) &DdmRaid::BlockReassign);
	DispatchRequest(BSA_STATUS_CHECK,(RequestCallback) &DdmRaid::StatusCheck);
	DispatchRequest(BSA_POWER_MANAGEMENT,(RequestCallback) &DdmRaid::PowerManagement);

	// Set dispatch routines for PHS Messages
	DispatchRequest(PHS_RESET_STATUS,(RequestCallback) &DdmRaid::ResetStatus);
	DispatchRequest(PHS_RETURN_STATUS,(RequestCallback) &DdmRaid::ReturnStatus);
	DispatchRequest(PHS_RESET_PERFORMANCE,(RequestCallback) &DdmRaid::ResetPerformance);
	DispatchRequest(PHS_RETURN_PERFORMANCE,(RequestCallback) &DdmRaid::ReturnPerformance);
	DispatchRequest(PHS_RETURN_RESET_PERFORMANCE,(RequestCallback) &DdmRaid::ReturnResetPerformance);

	// All other Messages go to DispatchDefault

	// Do reply
	Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// HandleReporterReply
// Callback from sending a Msg to PHS
/*************************************************************************/
	
void	DdmRaid::HandleReporterReply(Message *pMsg)
{
	delete pMsg;
}

/*************************************************************************/
// Enable
// Configuration may change between Quiesce and Enable, so Enable news
// and initializes a Raid class, and Quiesce destroys the Raid class
/*************************************************************************/

// temp - this should be Enable
STATUS	DdmRaid::Enable(Message *pMsg)
{ 
	STATUS	Status;
	Ioreq	*pIoreq;
	
	TRACE_ENTRY(DdmRaid::Enable);

	// Create instance for raid level
	switch (ArrayDescriptor.raidLevel)
	{
		case RAID0:
			pRaid = new (tZERO) Raid0;
			break;
		case RAID1:
			pRaid = new (tZERO) Raid1;
			break;
		case RAIDCAT:
			pRaid = new (tZERO) RaidCat;
			break;
		default:
			return OS_DETAIL_STATUS_UNSUPPORTED_FUNCTION;
	}

pRaidPtr = pRaid;
	if (!pRaid)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	// Gets an Ioreq and fills in
	if ( (pIoreq = new (tZERO) Ioreq(pMsg) ) == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	// Set callback address
	pIoreq->iCall = 1;
	pIoreq->Call[0].pInst = this;
	pIoreq->Call[0].pCallback = (pIoreqCallbackMethod)&DdmRaid::EnableDone;

	// Initialize the Raid class
	if ((Status = pRaid->Initialize(this, &ArrayDescriptor, pIoreq)) != OS_DETAIL_STATUS_SUCCESS)
	{
		delete pRaid;
		pRaid = NULL;
		delete pIoreq;
		return (Status);
	}

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// EnableDone
// Done initializing Raid class, Reply to Enable Message
/*************************************************************************/

void	DdmRaid::EnableDone(Ioreq *pIoreq)
{
	Message 		*pMsg;
	RqDdmReporter	*pReporterMsg;
	STATUS			Status;

	TRACE_ENTRY(DdmRaid::EnableDone);

	pMsg = pIoreq->pMessage;
	Status = pIoreq->Status;

	if (Status == OS_DETAIL_STATUS_SUCCESS)
	{
		// Start PHS Reporters

		// Start Performance Reporter
		//pReporterMsg = new RqDdmReporter(PHS_START, PHS_ARRAY_PERFORMANCE, RaidDID,
		//							ArrayDescriptor.arrayVDN);
									
		pReporterMsg = new RqDdmReporter(PHS_START, PHS_ARRAY_PERFORMANCE, RaidDID,
									GetVdn());
		Send(pReporterMsg, (ReplyCallback) &DdmRaid::HandleReporterReply);

		// Start Status Reporter
		//pReporterMsg = new RqDdmReporter(PHS_START, PHS_ARRAY_STATUS, RaidDID,
		//							ArrayDescriptor.arrayVDN);
		pReporterMsg = new RqDdmReporter(PHS_START, PHS_ARRAY_STATUS, RaidDID,
									GetVdn());
		Send(pReporterMsg, (ReplyCallback) &DdmRaid::HandleReporterReply);
	}
	delete pIoreq;

	// Do reply
	Reply(pMsg, Status);
}

/*************************************************************************/
// Quiesce
// Configuration may change between Quiesce and Enable, so Enable news
// and initializes a Raid class, and Quiesce destroys the Raid class
/*************************************************************************/
	
STATUS	DdmRaid::Quiesce(Message *pMsg)
{
	Ioreq			*pIoreq;
	RqDdmReporter	*pReporterMsg;

	TRACE_ENTRY(DdmRaid::Quiesce);

	// Stop PHS Reporters

	// Stop Performance Reporter
	pReporterMsg = new RqDdmReporter(PHS_STOP, PHS_ARRAY_PERFORMANCE, RaidDID,
								ArrayDescriptor.arrayVDN);
	Send(pReporterMsg, (ReplyCallback) &DdmRaid::HandleReporterReply);

	// Stop Status Reporter
	pReporterMsg = new RqDdmReporter(PHS_STOP, PHS_ARRAY_STATUS, RaidDID,
								ArrayDescriptor.arrayVDN);
	Send(pReporterMsg, (ReplyCallback) &DdmRaid::HandleReporterReply);


	// Gets an Ioreq and fills it in
	if ( (pIoreq = new (tZERO) Ioreq(pMsg) ) == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	// Set callback address
	pIoreq->iCall = 1;
	pIoreq->Call[0].pInst = this;
	pIoreq->Call[0].pCallback = (pIoreqCallbackMethod)&DdmRaid::QuiesceDone;
	pRaid->Quiesce(pIoreq);

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// QuiesceDone
// Raid is Quiesced. Destroy the Raid class and Reply to Quiesce Message
/*************************************************************************/
	
void	DdmRaid::QuiesceDone(Ioreq *pIoreq)
{
	Message *pMsg;

	TRACE_ENTRY(DdmRaid::QuiesceDone);

	pMsg = pIoreq->pMessage;

	delete pIoreq;
	delete pRaid;
	pRaid = NULL;

	// Do reply
	Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);
}

/*************************************************************************/
// BlockRead
// BSA_BLOCK_READ Messages come here
/*************************************************************************/

STATUS	DdmRaid::BlockRead(Message *pMsg)
{
	BSA_RW_PAYLOAD			*pBSA;
	Ioreq					*pIoreq;
	STATUS					Status;
	U32						Num, Lba;

	TRACE_ENTRY(DdmRaid::BlockRead);

	pBSA = (BSA_RW_PAYLOAD *)pMsg->GetPPayload();

	if (pRaid->IsRaidOffline())
		return OS_DETAIL_STATUS_DEVICE_NOT_AVAILABLE;

	// Convert the byte transfer count to blocks
	Num = pBSA->TransferByteCount / 512;

	Lba = pBSA->LogicalBlockAddress;

	// Check valid range
	if ( (Status = CheckLBA(Lba, Num) ) != OS_DETAIL_STATUS_SUCCESS)
		return Status;

	// Gets an Ioreq and fills it in
	if ( (pIoreq = new (tZERO) Ioreq(pMsg) ) == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	if (pIoreq->Status != OS_DETAIL_STATUS_SUCCESS)
	{
		Status = pIoreq->Status;
		delete pIoreq;
		return (Status);
	}

	// Set callback address
	pIoreq->iCall = 1;
	pIoreq->Call[0].pInst = this;
	pIoreq->Call[0].pCallback = (pIoreqCallbackMethod)&DdmRaid::BlockReadDone;

	// Do the Read
	if ( (Status = pRaid->DoRead(pIoreq)) != OS_DETAIL_STATUS_SUCCESS)
	{
		if (pIoreq->NumSGLs)
			delete []pIoreq->pSGList;
		delete pIoreq;
		return (Status);
	}

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// BlockWrite
// BSA_BLOCK_WRITE and BSA_BLOCK_WRITE_VERIFY Messages come here
/*************************************************************************/

STATUS	DdmRaid::BlockWrite(Message *pMsg)
{
	BSA_RW_PAYLOAD			*pBSA;
	Ioreq					*pIoreq;
	STATUS					Status;
	U32						Num, Lba;

	TRACE_ENTRY(DdmRaid::BlockWrite);

	if (pRaid->IsRaidOffline())
		return OS_DETAIL_STATUS_DEVICE_NOT_AVAILABLE;

	pBSA = (BSA_RW_PAYLOAD *)pMsg->GetPPayload();

	// Convert the byte transfer count to blocks
	Num = pBSA->TransferByteCount / 512;

	Lba = pBSA->LogicalBlockAddress;

	// Check valid range
	if ( (Status = CheckLBA(Lba, Num) ) != OS_DETAIL_STATUS_SUCCESS)
		return Status;

	// Gets an Ioreq and fills it in
	if ( (pIoreq = new (tZERO) Ioreq(pMsg) ) == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	if (pIoreq->Status != OS_DETAIL_STATUS_SUCCESS)
	{
		Status = pIoreq->Status;
		delete pIoreq;
		return (Status);
	}

	// Set the callback address
	pIoreq->iCall = 1;
	pIoreq->Call[0].pInst = this;
	pIoreq->Call[0].pCallback = (pIoreqCallbackMethod)&DdmRaid::BlockWriteDone;

	// Do the Write
	if ( (Status = pRaid->DoWrite(pIoreq)) != OS_DETAIL_STATUS_SUCCESS)
	{
		if (pIoreq->NumSGLs)
			delete []pIoreq->pSGList;
		delete pIoreq;
		return (Status);
	}

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// BlockReassign
// BSA_BLOCK_REASSIGN Messages come here
/*************************************************************************/

STATUS	DdmRaid::BlockReassign(Message *pMsg)
{
	BSA_RW_PAYLOAD			*pBSA;
	Ioreq					*pIoreq;
	STATUS					Status;
	U32						Num, Lba;

	TRACE_ENTRY(DdmRaid::BlockReassign);

	pBSA = (BSA_RW_PAYLOAD *)pMsg->GetPPayload();

	if (pRaid->IsRaidOffline())
		return OS_DETAIL_STATUS_DEVICE_NOT_AVAILABLE;

	Num = pBSA->TransferByteCount / 512;

	Lba = pBSA->LogicalBlockAddress;

	// Check valid range
	if ( (Status = CheckLBA(Lba, Num) ) != OS_DETAIL_STATUS_SUCCESS)
		return Status;

	// Gets an Ioreq and fills it in
	if ( (pIoreq = new (tZERO) Ioreq(pMsg) ) == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	if (pIoreq->Status != OS_DETAIL_STATUS_SUCCESS)
	{
		Status = pIoreq->Status;
		delete pIoreq;
		return (Status);
	}

	// Set the callback address
	pIoreq->iCall = 1;
	pIoreq->Call[0].pInst = this;
	pIoreq->Call[0].pCallback = (pIoreqCallbackMethod)&DdmRaid::BlockReassignDone;

	// Do the Reassign Block command
	if ( (Status = pRaid->DoReassign(pIoreq)) != OS_DETAIL_STATUS_SUCCESS)
	{
		if (pIoreq->NumSGLs)
			delete []pIoreq->pSGList;
		delete pIoreq;
		return (Status);
	}

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// StatusCheck
// BSA_STATUS_CHECK Messages come here
/*************************************************************************/

STATUS	DdmRaid::StatusCheck(Message *pMsg)
{
	Ioreq		 *pIoreq;
	STATUS		 Status;

	TRACE_ENTRY(DdmRaid::StatusCheck);

	// Gets an Ioreq and fills it in
	if ( (pIoreq = new (tZERO) Ioreq(pMsg) ) == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	if (pIoreq->Status != OS_DETAIL_STATUS_SUCCESS)
	{
		Status = pIoreq->Status;
		delete pIoreq;
		return (Status);
	}

	// Set the callback address
	pIoreq->iCall = 1;
	pIoreq->Call[0].pInst = this;
	pIoreq->Call[0].pCallback = (pIoreqCallbackMethod)&DdmRaid::IODone;

	// Do the Status Check command
	if ( (Status = pRaid->DoStatusCheck(pIoreq)) != OS_DETAIL_STATUS_SUCCESS)
	{
		delete pIoreq;
		return (Status);
	}

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// PowerManagement
// BSA_POER_MANAGEMENT Messages come here
/*************************************************************************/

STATUS	DdmRaid::PowerManagement(Message *pMsg)
{
	Ioreq		 *pIoreq;
	STATUS		 Status;

	TRACE_ENTRY(DdmRaid::PowerManagement);

	// Gets an Ioreq and fills it in
	if ( (pIoreq = new (tZERO) Ioreq(pMsg) ) == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	if (pIoreq->Status != OS_DETAIL_STATUS_SUCCESS)
	{
		Status = pIoreq->Status;
		delete pIoreq;
		return (Status);
	}

	// Set the callback address
	pIoreq->iCall = 1;
	pIoreq->Call[0].pInst = this;
	pIoreq->Call[0].pCallback = (pIoreqCallbackMethod)&DdmRaid::IODone;

	// Do the Power Management command
	if ( (Status = pRaid->DoPowerManagement(pIoreq)) != OS_DETAIL_STATUS_SUCCESS)
	{
		delete pIoreq;
		return (Status);
	}

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// ResetStatus
// PHS Reset Status Message
/*************************************************************************/

STATUS	DdmRaid::ResetStatus(Message *pMsg)
{
	TRACE_ENTRY(DdmRaid::ResetStatus);

	// Zero the Status structure
	pRaid->ResetStatus();

	// Reply to Message
	Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// ReturnStatus
// PHS Return Status Message
/*************************************************************************/

STATUS	DdmRaid::ReturnStatus(Message *pMsg)
{
	U8		*ptr;
	U32		Count = sizeof(RAID_STATUS);

	TRACE_ENTRY(DdmRaid::ReturnStatus);

	// Get address to copy Status structure to
	pMsg->GetSgl(DDM_REPLY_DATA_SGI, &ptr, &Count);

	// Copy the Status structure
	pRaid->ReturnStatus(ptr);

	// Reply to the Message
	Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// ResetPerformance
// PHS Reset Performance Message
/*************************************************************************/

STATUS	DdmRaid::ResetPerformance(Message *pMsg)
{
	TRACE_ENTRY(DdmRaid::ResetPerformance);

	// Zero the Performance structure
	pRaid->ResetPerformance();

	// Reply to Message
	Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// ReturnPerformance
// PHS Return Performance Message
/*************************************************************************/

STATUS	DdmRaid::ReturnPerformance(Message *pMsg)
{
	U8		*ptr;
	U32		Count = sizeof(RAID_PERFORMANCE);

	TRACE_ENTRY(DdmRaid::ReturnPerformance);

	// Get address to copy Performance structure to
	pMsg->GetSgl(DDM_REPLY_DATA_SGI, &ptr, &Count);

	// Copy the Performance structure
	pRaid->ReturnPerformance(ptr);

	// Reply to the Message
	Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// ReturnResetPerformance
// PHS Return Performance and Reset Performance Message
/*************************************************************************/

STATUS	DdmRaid::ReturnResetPerformance(Message *pMsg)
{
	U8		*ptr;
	U32		Count = sizeof(RAID_PERFORMANCE);

	TRACE_ENTRY(DdmRaid::ReturnResetPerformance);

	// Get address to copy Performance structure to
	pMsg->GetSgl(DDM_REPLY_DATA_SGI, &ptr, &Count);

	// Copy the Performance structure
	pRaid->ReturnPerformance(ptr);

	// Zero the Performance structure
	pRaid->ResetPerformance();

	// Reply to the Message
	Reply(pMsg, OS_DETAIL_STATUS_SUCCESS);

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// DispathDefault
// All other messages come here
/*************************************************************************/

STATUS	DdmRaid::DispatchDefault(Message *pMsg)
{

	TRACE_ENTRY(DdmRaid::DispatchDefault);

	if (pRaid->IsRaidOffline())
		return OS_DETAIL_STATUS_DEVICE_NOT_AVAILABLE;

	// New service message
	switch(pMsg->reqCode)
	{
		case BSA_CACHE_FLUSH:
		case BSA_DEVICE_RESET:
		default:
			return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
	}
	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// BlockReadDone
// Callback for Block Read
/*************************************************************************/

void	DdmRaid::BlockReadDone(Ioreq *pIoreq)
{
	U32					NumBlocks;
	BSA_RW_PAYLOAD		*pBSA;
	Message 			*pMsg;

	TRACE_ENTRY(DdmRaid::BlockReadDone);

	pMsg = pIoreq->pMessage;

	pBSA = (BSA_RW_PAYLOAD *)pMsg->GetPPayload();

	// Convert the byte transfer count to blocks
	NumBlocks = pBSA->TransferByteCount / 512;

	// Set completion status
	SetMsgReplyStatus(pMsg, pIoreq);

	// Do reply
	Reply(pMsg);

	// Update Stats
	pRaid->UpdatePerfReads(NumBlocks);

	if (pIoreq->NumSGLs)
		delete []pIoreq->pSGList;

	delete pIoreq;
}

/*************************************************************************/
// BlockWriteDone
// Callback for Block Write
/*************************************************************************/

void	DdmRaid::BlockWriteDone(Ioreq *pIoreq)
{
	U32					NumBlocks;
	BSA_RW_PAYLOAD		*pBSA;
	Message 			*pMsg;

	TRACE_ENTRY(DdmRaid::BlockWriteDone);

	pMsg = pIoreq->pMessage;

	pBSA = (BSA_RW_PAYLOAD *)pMsg->GetPPayload();

	// Convert the byte transfer count to blocks
	NumBlocks = pBSA->TransferByteCount / 512;

	// Set completion status
	SetMsgReplyStatus(pMsg, pIoreq);

	// Do reply
	Reply(pMsg);

	// Update Stats
	pRaid->UpdatePerfWrites(NumBlocks);

	if (pIoreq->NumSGLs)
		delete []pIoreq->pSGList;

	delete pIoreq;
}

/*************************************************************************/
// BlockReassignDone
// Callback for Block Reassign
/*************************************************************************/

void	DdmRaid::BlockReassignDone(Ioreq *pIoreq)
{
	Message *pMsg;

	TRACE_ENTRY(DdmRaid::BlockReassignDone);

	pMsg = pIoreq->pMessage;

	// Set completion status
	SetMsgReplyStatus(pMsg, pIoreq);

	// Do reply
	Reply(pMsg);

	// Update Stats
	pRaid->UpdateRaidReassignments(pIoreq->Status);

	if (pIoreq->NumSGLs)
		delete []pIoreq->pSGList;

	delete pIoreq;
}

/*************************************************************************/
// IODone
// Callback for Status Check or Power Management
/*************************************************************************/

void	DdmRaid::IODone(Ioreq *pIoreq)
{
	Message *pMsg;

	TRACE_ENTRY(DdmRaid::IODone);

	pMsg = pIoreq->pMessage;

	// Set completion status
	SetMsgReplyStatus(pMsg, pIoreq);

	// Do reply
	Reply(pMsg);

	if (pIoreq->NumSGLs)
		delete []pIoreq->pSGList;

	delete pIoreq;
}

/*************************************************************************/	 
// SetMsgReplyStatus
// Set Msg Status and Reply Payload from Ioreq
/*************************************************************************/

void	DdmRaid::SetMsgReplyStatus(Message *pMsg, Ioreq *pIoreq)
{
	BSA_REPLY_PAYLOAD	ReplyPayload;
	CNVTR				cnvtr;
	 
	// Set completion status
	pMsg->DetailedStatusCode = pIoreq->Status;

	// Clear the payload 
	memset(&ReplyPayload, 0, sizeof(BSA_REPLY_PAYLOAD));

	if ((pIoreq->Status & FCP_SCSI_DEVICE_DSC_MASK) == FCP_SCSI_DSC_CHECK_CONDITION)
	{
		// Check Condition - fill in Sense Data
		ReplyPayload.TransferCount = 0;
		ReplyPayload.LogicalBlockAddress = pIoreq->ErrorLba;
		ReplyPayload.AutoSenseTransferCount = 8;
		ReplyPayload.SenseData[0] = 0xf0;
		ReplyPayload.SenseData[2] = pIoreq->SenseKey;
		if (pIoreq->SenseKey == SENSE_MEDIUM_ERROR)
		{
			// Media Error - Fill in LBA in Sense Data
			cnvtr.ulngval = pIoreq->ErrorLba;
			ReplyPayload.SenseData[3] = cnvtr.charval[0];
			ReplyPayload.SenseData[4] = cnvtr.charval[1];
			ReplyPayload.SenseData[5] = cnvtr.charval[2];
			ReplyPayload.SenseData[6] = cnvtr.charval[3];
		}
		// Add Reply Payload to Message
		pMsg->AddReplyPayload(&ReplyPayload, sizeof(BSA_REPLY_PAYLOAD));
	}
}


/*************************************************************************/
// Ioreq
// Constructor method for the class Ioreq
/*************************************************************************/

Ioreq::Ioreq(Message *pMsg)
{
	BSA_RW_PAYLOAD	*pBSA;
	BOOL			isPayload = TRUE;

	Status = FCP_SCSI_DSC_SUCCESS;
	Flags = 0;
	pReq = NULL;
	switch(pMsg->reqCode)
	{
		case BSA_BLOCK_WRITE_VERIFY:
			Flags |= WRITE_VERIFY_BIT;
			// fall through
		case BSA_BLOCK_WRITE:
			Type = RAID_WRITE;
			break;
		case BSA_BLOCK_READ:
			Type = RAID_READ;
			break;
		case BSA_BLOCK_REASSIGN:
			Type = RAID_REASSIGN;
			break;
		case BSA_STATUS_CHECK:
			Type = RAID_STATUS_CHECK;
			isPayload = FALSE;		// no payload
			break;
		case BSA_POWER_MANAGEMENT:
			Type = RAID_POWER_MANAGEMENT;
			isPayload = FALSE;		// no payload
			break;
		case REQ_OS_DDM_QUIESCE:
			Type = RAID_QUIESCE;
			isPayload = FALSE;		// no payload
			break;

	}
	pMessage = pMsg;
	if (isPayload)
	{	// Message has a payload
		pBSA = (BSA_RW_PAYLOAD *)pMsg->GetPPayload();

		Count = pBSA->TransferByteCount / 512;

		Lba = pBSA->LogicalBlockAddress;

		NumSGLs = pMsg->GetCSgl();
		if (NumSGLs)
		{
			pSGList = new SGLIST[NumSGLs];
			pCurSGList = pSGList;
			if (!pSGList)
				Status = OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;
			for (int i = 0; i < NumSGLs; i++)
			{
				pMsg->GetSgl(i, (void **)&pCurSGList->Address, &pCurSGList->Length);
				pCurSGList++;
			}
			pCurSGList = pSGList;	// restore Current Ptr
		}
	}
}


//
// Temporary!!!
// Create configuration records in PTS
//

#if 0
STATUS	DdmRaid::Enable(Message *pMsg)
{
	TempCreateConfiguration(pMsg);
	return (OS_DETAIL_STATUS_SUCCESS);
}
#endif

void	DdmRaid::TempCreateConfiguration(Message *pMsg)
{
	RAID_ROW_RECORD		*pRaidRow;

	if (ArrayDescriptor.thisRID.LoPart != 0)
	{
		// not first time -
		// tables should exist in PTS
		// and may have been changed

		// read array descriptor table
		pRaidRow = new (tZERO | tUNCACHED) RAID_ROW_RECORD;
		// set row id in array table to read
		pRaidRow->RowData.MemberData.thisRID = ArrayDescriptor.thisRID;
		// save pMsg
		pRaidRow->pInitIor = (Ioreq *)pMsg;

		// read array table row from PTS
		ReadArrayTableRow(pRaidRow,
			   	(pTSCallback_t)&DdmRaid::ReadArrayTableDone);
	}
	else
	{
		TempCreateTables(pMsg);
	}
}

void	DdmRaid::TempCreateTables(Message *pMsg)
{
 	// copy buildsys config into array descriptor

	ArrayDescriptor.arrayVDN = TmpConfig.AD.arrayVDN;	
	ArrayDescriptor.totalCapacity = TmpConfig.AD.totalCapacity;		
	ArrayDescriptor.memberCapacity = TmpConfig.AD.memberCapacity;		
	ArrayDescriptor.dataBlockSize = TmpConfig.AD.dataBlockSize;
	ArrayDescriptor.parityBlockSize = TmpConfig.AD.parityBlockSize;
	ArrayDescriptor.raidLevel = TmpConfig.AD.raidLevel;			
	ArrayDescriptor.health = TmpConfig.AD.health;				
	ArrayDescriptor.initStatus = TmpConfig.AD.initStatus;			
	ArrayDescriptor.peckingOrder = NEVER_PECK;
	ArrayDescriptor.numberMembers = TmpConfig.AD.numberMembers;
	ArrayDescriptor.numberUtilities = TmpConfig.AD.numberUtilities;	
	ArrayDescriptor.numberSpares = 0;		
	ArrayDescriptor.serialNumber = 0;		
	ArrayDescriptor.creationDate = 0;		
	ArrayDescriptor.timeStamp = 0;			
		

	DefineMemberTable(pMsg);
//	CreateMemberTableEntries(pMsg);
}

STATUS DdmRaid::DefineMemberTable(Message *pMsg)
{
	STATUS		status;

	// Allocate an Define Table object
	TSDefineTable *pMDDefineTable = new TSDefineTable;

	// Initialize the define Table object.
	status = pMDDefineTable->Initialize(
		this,									// DdmServices* pDdmServices,
		RAID_MEMBER_DESCRIPTOR_TABLE,			// String64 prgbTableName,
		MemberDescriptorTable_FieldDefs,		// fieldDef* prgFieldDefsRet,
		sizeofMemberDescriptorTable_FieldDefs,	// U32 cbrgFieldDefs,
		20,					// U32 cEntriesRsv,
		TRUE,									// bool* pfPersistant,
		(pTSCallback_t)&DdmRaid::CreateMemberTableEntries,	// pTSCallback_t pCallback,
		pMsg									// void* pContext
	);
	
	// Initiate the define table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pMDDefineTable->Send();

	return status;
}

void	DdmRaid::CreateMemberTableEntries(Message *pMsg)
{
	TSInsertRow				*pInsertRow;
	Ioreq					*pIoreq;
	RAID_ROW_RECORD			*pRaidRow;
	RAID_ARRAY_MEMBER		*pRowMember;

	pIoreq = new (tZERO) Ioreq;
	pIoreq->pMessage = pMsg;
	pIoreq->Count = ArrayDescriptor.numberMembers;
	for (U8 i = 0; i < ArrayDescriptor.numberMembers; i++)
	{

		pRaidRow = new (tZERO | tUNCACHED) RAID_ROW_RECORD;
		pRowMember = &pRaidRow->RowData.MemberData;
		// copy data
	pRowMember->arrayRID.Table = 8;
	pRowMember->arrayRID.HiPart = 0;
	pRowMember->arrayRID.LoPart = 1;
	pRowMember->memberRID.Table = 2;
	pRowMember->memberRID.HiPart = 0;
	pRowMember->memberRID.LoPart = i+1;
		pRowMember->memberVD = TmpConfig.MD[i].memberVD;
		pRowMember->memberHealth = TmpConfig.MD[i].memberHealth;
		pRowMember->memberIndex = TmpConfig.MD[i].memberIndex;
		pRowMember->maxRetryCnt = 3;
		pRowMember->queueMethod = RAID_QUEUE_ELEVATOR;
		pRowMember->startLBA = TmpConfig.MD[i].startLBA;
		pRowMember->endLBA = TmpConfig.MD[i].endLBA;
		pRowMember->maxOutstanding = 1;

		// set member index for callback routine
		pRaidRow->Index = i;
		// save init ioreq
		pRaidRow->pInitIor = pIoreq;

		pInsertRow = new TSInsertRow;
		pInsertRow->Initialize(
			this,
			RAID_MEMBER_DESCRIPTOR_TABLE,
			pRowMember,
			sizeof(RAID_ARRAY_MEMBER),
			&pRowMember->thisRID,
			(pTSCallback_t)&DdmRaid::MemberTableDone,
			pRaidRow
		);
		pInsertRow->Send();
	}
}


void	DdmRaid::MemberTableDone(RAID_ROW_RECORD *pRaidRow, STATUS Status)
{

	Message	*pMsg;
	Ioreq	*pIoreq;

	if (Status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACE_HEX(TRACE_ALL_LVL, "\n\rDdmRaid::MemberTableDone Status = %x ", Status);
	}

	pIoreq = pRaidRow->pInitIor;
	pMsg = pIoreq->pMessage;
	ArrayDescriptor.members[pRaidRow->Index] = pRaidRow->RowData.MemberData.thisRID;
	pIoreq->Count--;
	if (pIoreq->Count == 0)
	{	// done with all members
		delete pIoreq;

		CreateArrayTableEntry(pMsg);
	}
	delete pRaidRow;
}

void	DdmRaid::CreateArrayTableEntry(Message *pMsg)
{
	TSInsertRow				*pInsertRow;
	Ioreq					*pIoreq;
	RAID_ROW_RECORD			*pRaidRow;

	pIoreq = new (tZERO) Ioreq;
	pIoreq->pMessage = pMsg;

	pRaidRow = new (tZERO | tUNCACHED) RAID_ROW_RECORD;
	// copy data
	RAID_ARRAY_DESCRIPTOR	*ptr1, *ptr2;
	ptr2 = &ArrayDescriptor;
	ptr1 = &pRaidRow->RowData.ArrayData;
	*ptr1 = *ptr2;	// copy config data

	pRaidRow->pInitIor = pIoreq;

	pInsertRow = new TSInsertRow;
	pInsertRow->Initialize(
		this,
		RAID_ARRAY_DESCRIPTOR_TABLE,
		ptr1,
		sizeof(RAID_ARRAY_DESCRIPTOR),
		&ptr1->thisRID,
		(pTSCallback_t)&DdmRaid::ArrayTableDone,
		pRaidRow
	);
	pInsertRow->Send();
}

void	DdmRaid::ArrayTableDone(RAID_ROW_RECORD *pRaidRow, STATUS Status)
{

	Message	*pMsg;
	Ioreq	*pIoreq;

	if (Status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACE_HEX(TRACE_ALL_LVL, "\n\rDdmRaid::ArrayTableDone Status = %x ", Status);
	}

	pIoreq = pRaidRow->pInitIor;
	pMsg = pIoreq->pMessage;
	ArrayDescriptor.thisRID = pRaidRow->RowData.ArrayData.thisRID;
	delete pIoreq;
	delete pRaidRow;

	// do the real Enable
	RealEnable(pMsg);
}

void	DdmRaid::ReadArrayTableDone(RAID_ROW_RECORD *pRaidRow, STATUS Status)
{

	RAID_ARRAY_DESCRIPTOR	*ptr1, *ptr2;
	Message					*pMsg;

	if (Status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACE_HEX(TRACE_ALL_LVL, "\n\rDdmRaid::ReadArrayTableDone Status = %x ", Status);
	}

	pMsg = (Message *)pRaidRow->pInitIor;

	// copy read table to ArrayDescriptor
	ptr1 = &ArrayDescriptor;
	ptr2 = &pRaidRow->RowData.ArrayData;
	*ptr1 = *ptr2;	// copy config data

	// member configuration should
	// also already be written in PTS

	// do the real Enable
	RealEnable(pMsg);
}


/*************************************************************************/
// ReadArrayTableRow
// Read specified row from Array Table
/*************************************************************************/

STATUS	DdmRaid::ReadArrayTableRow(RAID_ROW_RECORD *pRaidRow, pTSCallback_t pCallback)
{
	STATUS		Status;

	// allocate the ReadRow class
	TSReadRow	*pTSReadRow = new TSReadRow;

	// row id to read in array decriptor table
	rowID		*pRowId = &pRaidRow->RowData.ArrayData.thisRID;
		
	// Initialize the ReadRow operation.
	Status = pTSReadRow->Initialize(
			this,								// DdmServices pDdmServices,
			RAID_ARRAY_DESCRIPTOR_TABLE,		// String64 rgbTableName,
			CT_PTS_RID_FIELD_NAME,				// String64 prgbKeyFieldName,
			pRowId,  			  				// void* pKeyFieldValue,
			sizeof(rowID),						// U32 cbKeyFieldValue,
			&pRaidRow->RowData.ArrayData,		// void* prgbRowData,
			sizeof(RAID_ARRAY_DESCRIPTOR),		// U32 cbRowData,
			NULL,								// rowID *pRowIDRet,
			(pTSCallback_t)pCallback,  			// pTSCallback_t pCallback,
			(void*)pRaidRow						// void* pContext
	);

	if (Status != OS_DETAIL_STATUS_SUCCESS)
		return (Status);

	// Initiate the ReadRow table operation.
	pTSReadRow->Send();

	return OS_DETAIL_STATUS_SUCCESS;
}

