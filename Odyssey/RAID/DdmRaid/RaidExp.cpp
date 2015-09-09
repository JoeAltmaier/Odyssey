/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: RaidExp.cpp
//
// Description:	Raid Expansion class
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
#include "RaidUtilTable.h"
#include "Raid.h"
#include "Raid1.h"
#include "Raid0.h"
#include "RaidExp.h"
#include "FcpMessageFormats.h"
#include "OsStatus.h"


/*************************************************************************/
// ~RaidExp
// Destructor method for the Raid Expansion class
/*************************************************************************/

RaidExp::~RaidExp()
{ 
	if (pRaidOld)
		delete pRaidOld;
	if (pRaidNew)
		delete pRaidNew;
	if (pUtilQueue)
		delete pUtilQueue;
	if (pRaidLock)
		delete pRaidLock;
}

/*************************************************************************/
// Initialize
// Initialize method for the Raid Expansion class
/*************************************************************************/

STATUS	RaidExp::Initialize(DdmServices *pDdmServices,
							RAID_ARRAY_DESCRIPTOR *pArrayDesc, Ioreq *pIoreq)
{
	RAID_ROW_RECORD		*pRaidRow;

	SetParentDdm(pDdmServices);
	RaidVDN = pArrayDesc->arrayVDN;
	RaidRowId = pArrayDesc->thisRID;
	ExpansionLba = 0;					// set later from utility table
	pArrayDescriptor = pArrayDesc;		// save for callbacks during initialization

	// Allocate a Utility Queue
	if ((pUtilQueue = new UtilQueue(RAIDQ_FIFO)) == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	// Allocate a range locking class
	if ((pRaidLock = new RaidLock) == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	// read array table to get old Raid configuration
	// and new Raid configuration
	pIoreq->Status = OS_DETAIL_STATUS_SUCCESS;
	pIoreq->Count = 2;			// use to count number of table reads done
	for (U8 i = 0; i < 2; i++)
	{
		pRaidRow = new RAID_ROW_RECORD;
		// set row id in array table to read
		pRaidRow->RowData.ArrayData.thisRID = pArrayDesc->members[i];
		// set member index for callback routine
		pRaidRow->Index = i;
		// save init ioreq
		pRaidRow->pInitIor = pIoreq;
		// read array table row from PTS
		ReadArrayTableRow(pRaidRow, (pTSCallback_t)&RaidExp::ReadArrayTableDone);
	}
	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// ReadArrayTableDone
// Callback method from reading an array table row.
// New a Raid class and initilize it from this data
/*************************************************************************/

void	RaidExp::ReadArrayTableDone(RAID_ROW_RECORD *pRaidRow, STATUS Status)
{
	RAID_ARRAY_DESCRIPTOR	*pArrayDesc;
	Raid					*pRaid = NULL;
	Ioreq					*pInitIor, *pIoreq;
	U8						index;

	pInitIor = pRaidRow->pInitIor;
	pArrayDesc = &pRaidRow->RowData.ArrayData;

	if (Status != OS_DETAIL_STATUS_SUCCESS)
		pInitIor->Status = OS_DETAIL_STATUS_DEVICE_NOT_AVAILABLE;		// set error

	// Allocate a Raid class for this member in array
	else
	{
		// create instance for raid level
		switch (pArrayDesc->raidLevel)
		{
			case RAID0:
				pRaid = new (tZERO) Raid0;
				break;
			case RAID1:
				pRaid = new (tZERO) Raid1;
				break;
		}
		if (pRaid == NULL)
			pInitIor->Status = OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;
	}

	// Get an Ioreq to initialize Raid class
	if ( (pIoreq = new (tZERO) Ioreq) == NULL)
		pInitIor->Status = OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	if (pInitIor->Status != OS_DETAIL_STATUS_SUCCESS)
	{
		// Error reading table or newing a Raid class for this
		// Raid or the other Raid - Don't bother trying to initialize

		delete pRaidRow;

		if (pIoreq)
			delete pIoreq;

		pInitIor->Count--;				// done with this Raid
		if (pInitIor->Count == 0)		// Other also done?
		{
			// do callback - DDM is not enabled and should 
			// not receive any commands
			index = --pInitIor->iCall;
			( (pInitIor->Call[index].pInst)->*(pInitIor->Call[index].pCallback) )(pInitIor);
		}
		return;
	}

	if (pRaidRow->Index == 0)		// This is Read of Old Raid configuration
		pRaidOld = pRaid;
	else							// This is Read of New Raid configuration
		pRaidNew = pRaid;


	// Set callback address from initializing Raid class
	pIoreq->iCall = 1;
	pIoreq->Call[0].pInst = this;
	pIoreq->Call[0].pCallback = (pIoreqCallbackMethod)&RaidExp::RaidInitializeDone;

	// initialize the Raid class
	pRaid->Initialize(pParentDdmSvs, pArrayDesc, pIoreq);
}

/*************************************************************************/
// RaidInitializeDone
// Callback method from initializing a Raid class
/*************************************************************************/

void	RaidExp::RaidInitializeDone(Ioreq *pIoreq)
{
	Ioreq 			*pInitIor;
	RAID_ROW_RECORD *pRaidRow;
	U8				index;

	// Get RaidRow from Ioreq
	pRaidRow = (RAID_ROW_RECORD *)pIoreq->pContext;
	// Get original initialization Ioreq from RaidRow
	pInitIor = pRaidRow->pInitIor;

	pInitIor->Count--;				// done with this Raid

	if (pIoreq->Status != OS_DETAIL_STATUS_SUCCESS)
	{	// set error status for callback to DdmRaid
		pInitIor->Status = pIoreq->Status;
	}

	// done with Ioreq
	delete pIoreq;

	// done with RaidRow
	delete pRaidRow;

	if (pInitIor->Count == 0)		// Other also done?
	{
		if (pInitIor->Status != OS_DETAIL_STATUS_SUCCESS)
		{
			// error initializing this Raid class or the other
			// do callback - DDM is not enabled and should 
			// not receive any commands
			index = --pInitIor->iCall;
			( (pInitIor->Call[index].pInst)->*(pInitIor->Call[index].pCallback) )(pInitIor);
			return;
		}
		// Both Raid classes are initialized successfully
		// Now read utility table to find Expansion Lba
		ReadUtilityTable(pInitIor);
	}
}

/*************************************************************************/
// ReadUtilityTable
// Read rows from Utility Table to find Expansion LBA
/*************************************************************************/

STATUS	RaidExp::ReadUtilityTable(Ioreq *pIoreq)
{
	RAID_ROW_RECORD		*pRaidRow;
	U8					i, index;

	pIoreq->Status = OS_DETAIL_STATUS_SUCCESS;
	pIoreq->Count = 0;			// use to count number of table reads done
	if (pArrayDescriptor->numberUtilities == 0)
	{
		// do callback on pIoreq
		index = --pIoreq->iCall;
		( (pIoreq->Call[index].pInst)->*(pIoreq->Call[index].pCallback) )(pIoreq);
	}
	else
	{
		for (i = 0; i < pArrayDescriptor->numberUtilities; i++)
		{
			pRaidRow = new RAID_ROW_RECORD;
			// set row id in utility table to read
			pRaidRow->RowData.UtilityData.thisRID = pArrayDescriptor->utilities[i];
			// save init ioreq
			pRaidRow->pInitIor = pIoreq;
			// read utility table row from PTS
			ReadUtilityTableRow(pRaidRow, (pTSCallback_t)&RaidExp::ReadUtilityTableCallback);
		}
	}
	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// ReadUtilityTableCallback
// Callback method from reading a utility table row.
// Find Expand Utility and set Expansion Lba.
// Start Expand if it's state is running or suspended
/*************************************************************************/

void	RaidExp::ReadUtilityTableCallback(RAID_ROW_RECORD *pRaidRow, STATUS Status)
{
	RAID_ARRAY_UTILITY	*pArrayUtility;
	Ioreq				*pIoreq;
	U8					index;

	pIoreq = pRaidRow->pInitIor;

	if (Status != OS_DETAIL_STATUS_SUCCESS)
		pIoreq->Status = OS_DETAIL_STATUS_DEVICE_NOT_AVAILABLE;		// set error

	else
	{
		pArrayUtility = &pRaidRow->RowData.UtilityData;

		if (pArrayUtility->utilityCode == RAID_UTIL_EXPAND)
		{
			ExpansionLba = pArrayUtility->currentLBA;
			switch (pArrayUtility->status)
			{
				case RAID_UTIL_RUNNING:
				case RAID_UTIL_SUSPENDED:
					Raid::StartUtility(pArrayUtility);
			}
		}
	}

	pIoreq->Count++;										// update count of number reads done
	if (pIoreq->Count == pArrayDescriptor->numberUtilities)	// done with all utilities
	{
		// do callback on pIoreq
		index = --pIoreq->iCall;
		( (pIoreq->Call[index].pInst)->*(pIoreq->Call[index].pCallback) )(pIoreq);
	}
	delete pRaidRow;
}

/*************************************************************************/
// DoRead
// Read method for the Raid Expansion class
/*************************************************************************/

STATUS	RaidExp::DoRead(Ioreq *pIoreq)
{
	U8	index;

	// If the requested range is below the Expansion Lba,
	// just read from New Raid mapping
	if (pIoreq->Lba + pIoreq->Count < ExpansionLba)
		return (pRaidNew->DoRead(pIoreq));

	// Need to read from the Old Raid mapping. Need to lock the
	// range first

	// Set Callback from acquiring a lock for this range
	index = pIoreq->iCall;
	pIoreq->Call[index].pInst = this;
	pIoreq->Call[index].pCallback = (pIoreqCallbackMethod)&RaidExp::ReadRangeLocked;
	pIoreq->iCall++;

	// Call to get lock. Will be called back when lock is acquired
	pRaidLock->GetIoreqLockWait(pIoreq);

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// DoWrite
// Write method for the Raid Expansion class
/*************************************************************************/

STATUS	RaidExp::DoWrite(Ioreq *pIoreq)
{
	U8	index;

	// If the requested range is below the Expansion Lba,
	// just write to New Raid mapping
	if (pIoreq->Lba + pIoreq->Count < ExpansionLba)
		return (pRaidNew->DoWrite(pIoreq));

	// Need to write to the Old Raid mapping. Need to lock the
	// range first

	// Set Callback from acquiring a lock for this range
	index = pIoreq->iCall;
	pIoreq->Call[index].pInst = this;
	pIoreq->Call[index].pCallback = (pIoreqCallbackMethod)&RaidExp::WriteRangeLocked;
	pIoreq->iCall++;

	// Call to get lock. Will be called back when lock is acquired
	pRaidLock->GetIoreqLockWait(pIoreq);

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// DoReassign
// Reassign method for the Raid Expansion class
/*************************************************************************/

STATUS	RaidExp::DoReassign(Ioreq *pIoreq)
{
	U8	index;

	// If the requested range is below the Expansion Lba,
	// just send to New Raid mapping
	if (pIoreq->Lba + pIoreq->Count < ExpansionLba)
		return (pRaidNew->DoReassign(pIoreq));

	// Need to send to the Old Raid mapping. Need to lock the
	// range first

	// Set Callback from acquiring a lock for this range
	index = pIoreq->iCall;
	pIoreq->Call[index].pInst = this;
	pIoreq->Call[index].pCallback = (pIoreqCallbackMethod)&RaidExp::ReassignRangeLocked;
	pIoreq->iCall++;

	// Call to get lock. Will be called back when lock is acquired
	pRaidLock->GetIoreqLockWait(pIoreq);

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// ReadRangeLocked
// Callback from locking a range for a Read request
/*************************************************************************/

void	RaidExp::ReadRangeLocked(Ioreq *pIoreq)
{
	U8	index;

	// Set Callback from finishing the Read request
	index = pIoreq->iCall;
	pIoreq->Call[index].pInst = this;
	pIoreq->Call[index].pCallback = (pIoreqCallbackMethod)&RaidExp::RequestDone;
	pIoreq->iCall++;

	// Do the read
	pRaidOld->DoRead(pIoreq);
}

/*************************************************************************/
// WriteRangeLocked
// Callback from locking a range for a Write request
/*************************************************************************/

void	RaidExp::WriteRangeLocked(Ioreq *pIoreq)
{
	U8	index;

	// Set Callback from finishing the Write request
	index = pIoreq->iCall;
	pIoreq->Call[index].pInst = this;
	pIoreq->Call[index].pCallback = (pIoreqCallbackMethod)&RaidExp::RequestDone;
	pIoreq->iCall++;

	// Do the write
	pRaidOld->DoWrite(pIoreq);
}

/*************************************************************************/
// ReassignRangeLocked
// Callback from locking a range for a Reassign request
/*************************************************************************/

void	RaidExp::ReassignRangeLocked(Ioreq *pIoreq)
{
	U8	index;

	// Set Callback from finishing the Reassign request
	index = pIoreq->iCall;
	pIoreq->Call[index].pInst = this;
	pIoreq->Call[index].pCallback = (pIoreqCallbackMethod)&RaidExp::RequestDone;
	pIoreq->iCall++;

	// Do the Reassign
	pRaidOld->DoReassign(pIoreq);
}

/*************************************************************************/
// RequestDone
// Callback from doing a Read or Write request
/*************************************************************************/

void	RaidExp::RequestDone(Ioreq *pIoreq)
{
	U8	index;

	// Release the lock
	pRaidLock->ReleaseIoreqLock(pIoreq);

	// Do Callback to DdmRaid
	index = --pIoreq->iCall;
	( (pIoreq->Call[index].pInst)->*(pIoreq->Call[index].pCallback) )(pIoreq);
}

/*************************************************************************/
// DoExpansion
// Expand method for the Raid Expansion class
/*************************************************************************/

STATUS	RaidExp::DoExpand(Utility *pUtility)
{
// Not yet implemented
#pragma unused (pUtility)
	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// IsRaidUsable
// Used by RaidErr class. If members in Failmask were down, would
// array still be usable?
/*************************************************************************/

BOOL 	RaidExp::IsRaidUsable(U32 Failmask)
{
	if (Failmask)
		return (FALSE);
	return (TRUE);
}

/*************************************************************************/
// SuspendAllUtilities
// Flag all running utilities to be suspended
// Used when Quiescing
/*************************************************************************/

void	RaidExp::SuspendAllUtilities()
{

	// Flag all outstanding utilities to be suspended
	UtilCmd.SuspendUtilities();

	pRaidOld->SuspendAllUtilities();

	pRaidNew->SuspendAllUtilities();

	return;
}

