/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DdmPart.cpp
//
// Description:	Partition DDM class
//
//
// Update Log: 
//	5/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#include "OsTypes.h"
#include "Message.h"
#include "CtTypes.h"
#include "Ddm.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"
#include "Scsi.h"
#include "OsStatus.h"
#include "RequestCodes.h"
#include "BuildSys.h"
#include "PartitionTable.h"
#include "DdmPart.h"

CLASSNAME(DdmPart, MULTIPLE);

/*************************************************************************/
// DdmPart
// Constructor method for the class DdmPart
/*************************************************************************/

DdmPart::DdmPart(DID did):Ddm(did)
{ 
	SetConfigAddress(&config, sizeof(config));
}

/*************************************************************************/
// Ctor
// Create a new instance of DdmPart
/*************************************************************************/

Ddm *DdmPart::Ctor(DID did)
{
	return new DdmPart(did);
}

/*************************************************************************/
// Initialize
//
/*************************************************************************/

STATUS	DdmPart::Initialize(Message *pMsg)
{
	// Set dispatch routines for Read, Write, and Reassign Messages
	DispatchRequest(BSA_BLOCK_READ,(MessageCallback) &DdmPart::DoIO);
	DispatchRequest(BSA_BLOCK_WRITE,(MessageCallback) &DdmPart::DoIO);
	DispatchRequest(BSA_BLOCK_WRITE_VERIFY,(MessageCallback) &DdmPart::DoIO);
	DispatchRequest(BSA_BLOCK_REASSIGN,(MessageCallback) &DdmPart::DoIO);

	// All other Messages go to DispatchDefault

	// do reply
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// Enable
//
/*************************************************************************/

STATUS	DdmPart::Enable(Message *pMsg)
{ 
	// do reply
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// Quiesce
//
/*************************************************************************/
	
STATUS	DdmPart::Quiesce(Message *pMsg)
{
	// do reply
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// DoIO
// Adjust LBA and Send Message
/*************************************************************************/

STATUS	DdmPart::DoIO(Message *pMsg)
{
	BSA_RW_PAYLOAD			*pBSA;
	U32						Num, Lba;

	pBSA = (BSA_RW_PAYLOAD *)pMsg->GetPPayload();

	// convert the byte transfer count to blocks
	Num = pBSA->TransferByteCount / 512;

	Lba = pBSA->LogicalBlockAddress;

	// check valid range
	if ((Lba + Num) > config.partitionSize)
		return (OS_DETAIL_STATUS_INVALID_PARAMETER);

	// Adjust Block Number
	Lba = pBSA->LogicalBlockAddress += config.startLBA;

	// Send Message
	return (Send(config.parentVDN, pMsg, (ReplyCallback) &DdmPart::IODone));
}


/*************************************************************************/
// DefaultMessage
// All other messages come here
/*************************************************************************/

STATUS	DdmPart::DispatchDefault(Message *pMsg)
{

	// Forward Message
	return (Forward(pMsg, config.parentVDN));
}


/*************************************************************************/
// IODone
// Check for a Media Error and adjust Lba in sense data before Replying
/*************************************************************************/

void	DdmPart::IODone(MessageReply *pMsg)
{
	CNVTR				cnvtr;
	BSA_REPLY_PAYLOAD	*pPayload;

	if ((pMsg->DetailedStatusCode & FCP_SCSI_DEVICE_DSC_MASK) == FCP_SCSI_DSC_CHECK_CONDITION)
	{	// Check condition
	 	pPayload = (BSA_REPLY_PAYLOAD *) pMsg->GetPPayload();
		if ((pPayload->SenseData[2] & 0x0f) == SENSE_MEDIUM_ERROR)
		{	// Media error - get LBA from sense data
			cnvtr.charval[0] = pPayload->SenseData[3];
			cnvtr.charval[1] = pPayload->SenseData[4];
			cnvtr.charval[2] = pPayload->SenseData[5];
			cnvtr.charval[3] = pPayload->SenseData[6];
			// adjust Lba
			cnvtr.ulngval -= config.startLBA;
			// put back in sense data
			pPayload->SenseData[3] = cnvtr.charval[0];
			pPayload->SenseData[4] = cnvtr.charval[1];
			pPayload->SenseData[5] = cnvtr.charval[2];
			pPayload->SenseData[6] = cnvtr.charval[3];
		}
	}
	// do reply
	Reply(pMsg);
}

