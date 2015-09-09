/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: RmstrHotCopy.cpp
// 
// Description:
// Implementation for the raid hot copy
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrHotCopy.cpp $
// 
// 1     12/17/99 3:55p Dpatel
// Initial creation
// 
//
/*************************************************************************/

#include "DdmRaidMgmt.h"


// Start Util Validation states
enum
{
	HOT_COPY_SRC_MEMBER_EXPORT_RECORD_READ =1,
	HOT_COPY_VC_MODIFIED
};




//************************************************************************
//	StartHotCopy
//	A routine, which does all the Virtual Circuit related operations
//	for the Hot Copy 
//		Get the ridVcId of the src member by reading export table
//		Modify the VC (ridVcId, newVd) - includes Quiescing VC
//		Insert a new util descriptor record for regenerate
//		The state m/c will then proceed as a regular start utility operation
//
//
//	_pADTRecord	- the hot copy array
//
//************************************************************************
STATUS DdmRAIDMstr::
StartHotCopy(
			RAID_ARRAY_DESCRIPTOR	*_pADTRecord,
			pTSCallback_t			_pCallback,
			CONTEXT					*_pParentContext)
{
	CONTEXT							*pHotCopyContext;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	STATUS							status = OK;
	rowID							sourceMemberRowId;
	RAID_ARRAY_MEMBER				*pSourceMember;


	// pHotCopyContext->pData = ADT Record

	pHotCopyContext = new CONTEXT;
	pHotCopyContext->pParentContext = _pParentContext;
	pHotCopyContext->pCallback = _pCallback;

	pHotCopyContext->state = HOT_COPY_SRC_MEMBER_EXPORT_RECORD_READ;
	pADTRecord = new (tZERO) RAID_ARRAY_DESCRIPTOR;
	pHotCopyContext->pData = pADTRecord;
	*pADTRecord = *_pADTRecord;

	// Obtain the src members, src row id
	sourceMemberRowId = pADTRecord->members[pADTRecord->sourceMemberIndex];
	GetRmstrData(
		RAID_MEMBER,
		&sourceMemberRowId,
		(void **)&pSourceMember);

	// allocate data for export record to be read
	pHotCopyContext->pData1 = new (tZERO) ExportTableEntry;
	// Read the export row with the SRC of the array's src member
	TSReadRow *pReadExportRec = new TSReadRow;
	if (!pReadExportRec)
		status = CTS_OUT_OF_MEMORY;
	else		
		status = pReadExportRec->Initialize( 
				this,										// DdmServices *pDdmServices,
				EXPORT_TABLE,								// String64 rgbTableName,
				"ridSRC",									// String64 rgbKeyFieldName,
				&pSourceMember->memberRID,					// void *pKeyFieldValue,
				sizeof(rowID),								// U32 cbKeyFieldValue,
				pHotCopyContext->pData1,					// void *prgbRowDataRet,
				sizeof(ExportTableEntry),					// U32 cbRowDataRetMax,
				NULL,										// U32 *pcRowsReadRet,
				TSCALLBACK(DdmRAIDMstr,ProcessStartHotCopyReply),	// pTSCallback_t pCallback,
				pHotCopyContext								// void* pContext
			);
		
	if (status == OK)
		pReadExportRec->Send();
	else
		ProcessStartHotCopyReply(pHotCopyContext, status);
	return status;
}



//************************************************************************
//	ProcessStartHotCopyReply
//		
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessStartHotCopyReply(void *_pContext, STATUS status)
{
	CONTEXT							*pHotCopyContext = (CONTEXT *)_pContext;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	ExportTableEntry				*pExportRec = NULL;
	BOOL							cmdComplete = false;
	CONTEXT							*pOriginalContext = pHotCopyContext->pParentContext;

	// VC Modify stuff
	VCRequest						*pVCRequest = NULL;
	VCCommand						*pVcCmd;
	VCModifyCtlCommand				*pVCModifyCtl;
	MsgVCMModifyVC					*pMsg;


	// Regenerate stuff, extracted out of original context
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_START_UTIL_INFO			*pStartUtilityInfo = NULL;
	RAID_ARRAY_UTILITY				*pUtility;
	RMSTR_CMND_INFO					*pCmdInfo = NULL;
	RAID_ARRAY_MEMBER				*pMember = NULL;
	rowID							*pSrcRowIds = NULL;
	rowID							*pDestRowIds = NULL;
	U32								numberSourceMembers = 0;
	U32								numberDestinationMembers = 0;
	STATE_IDENTIFIER				stateIdentifier;

	// pHotCopyContext->pData = ADT Record
	// pHotCopyContext->pData1 = Export Record
	pADTRecord	= (RAID_ARRAY_DESCRIPTOR *)pHotCopyContext->pData;
	pExportRec	= (ExportTableEntry *)pHotCopyContext->pData1;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		TRACEF(TRACE_L1, ("Start Hot Copy Reply: Error: Invalid command\n"));
		cmdComplete = true;
	} else {
		switch(pHotCopyContext->state){
		case 	HOT_COPY_SRC_MEMBER_EXPORT_RECORD_READ:
			TRACEF(TRACE_L1, ("Start Hot Copy: Export record read\n"));

			pHotCopyContext->state = HOT_COPY_VC_MODIFIED;
			// Issue a VC Modify for the VC Id in the export record
			pVCRequest = new (tZERO) VCRequest;
			pHotCopyContext->pData2 = pVCRequest;

			pVcCmd = (VCCommand *)&pVCRequest->eCommand;
			pVCModifyCtl = (VCModifyCtlCommand *)&pVCRequest->u.VCModifyParms;

			*pVcCmd = k_eModifyVC;
			pVCModifyCtl->vdNext = pADTRecord->arrayVDN;
			pVCModifyCtl->ridVcId = pExportRec->ridVcId;

			pVCModifyCtl->srcNext = pADTRecord->SRCTRID;

			pMsg = new MsgVCMModifyVC(pVCRequest);
			status = Send(
						pMsg,
						pHotCopyContext,
						REPLYCALLBACK(DdmRAIDMstr,ProcessModifyVCReply));
			break;

		case 	HOT_COPY_VC_MODIFIED:
			// VC is modified successfully, now start the
			// actual regenerate operation (thru the start util state m/c)
			pCmdInfo		= (RMSTR_CMND_INFO *)pOriginalContext->pData;
			pCmdParams		= &pCmdInfo->cmdParams;
			pStartUtilityInfo = 
				(RMSTR_START_UTIL_INFO *)&pCmdParams->startUtilInfo;
			pUtility = (RAID_ARRAY_UTILITY *)pOriginalContext->pData2;

			PrepareRegenerateMembers(
						pADTRecord,
						&pSrcRowIds,
						&pDestRowIds,
						&numberSourceMembers,
						&numberDestinationMembers);

			stateIdentifier.cmdOpcode	= pCmdInfo->opcode;
			stateIdentifier.cmdRowId	= *(rowID *)pOriginalContext->cmdHandle; 
			stateIdentifier.cmdState	= pOriginalContext->state;
			stateIdentifier.index		= 0;
			status = rmstrInsertUtilDescriptor(
					&stateIdentifier,
					pStartUtilityInfo->utilityName,	// util name
					&pADTRecord->thisRID,			// target array
					pStartUtilityInfo->priority,	// priority
					pStartUtilityInfo->updateRate,	// update rate
					pStartUtilityInfo->policy,		// policies
					pSrcRowIds,						
					numberSourceMembers,								
					pDestRowIds,			
					numberDestinationMembers,
					pADTRecord->memberCapacity,
					TSCALLBACK(DdmRAIDMstr,rmstrProcessStartUtilityReply),
					pOriginalContext);
			if (pSrcRowIds){
				delete pSrcRowIds;
			}
			if (pDestRowIds){
				delete pDestRowIds;
			}
			cmdComplete = true;
			break;

		default:
			status = RMSTR_ERR_INVALID_COMMAND;
			cmdComplete = true;
			break;
		}
	}
	if (cmdComplete){
		if (status){
			TRACEF(TRACE_L1, ("ERROR: Start Hot Copy Preparation failed!!\n")); 
			pTSCallback_t	cb = pHotCopyContext->pCallback; 
			(this->*cb)(pOriginalContext,status);
		} else {
			TRACEF(TRACE_L1, ("Start Hot Copy Preparation complete: starting regenerate\n")); 
		}
		delete pHotCopyContext;
	}
	return status;
}


//************************************************************************
//	ProcessModifyVCReply
//		
//
//************************************************************************
STATUS DdmRAIDMstr::
ProcessModifyVCReply(Message* pMsg)
{
	STATUS	status = pMsg->Status();
	CONTEXT	*pContext = (CONTEXT *)pMsg->GetContext();
	ProcessStartHotCopyReply(pContext, status);
	delete pMsg;
	return status;
}
