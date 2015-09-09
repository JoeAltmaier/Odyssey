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
// File: VCDelete.cpp
//
// Description:
//    Virtual Circuit Manager DDM Delete Command methods.
//
// $Log: /Gemini/Odyssey/DdmVCM/VCDelete.cpp $
// 
// 6     1/26/00 9:45a Szhang
// Add codes to send message to delete VC 
// 
// 5     12/15/99 2:16p Dpatel
// added functionality for delete vc
// 
// 4     11/04/99 4:27p Jlane
// Use VDT_FIOPHASDID_FIELD and in general make compile with Tom's new VDT
// code.
// 
// 3     9/06/99 8:03p Jlane
// Yet another interim checkin.
// 
// 2     9/05/99 4:40p Jlane
// Compiles and is theoretically ready to try.
// 
// 1     9/05/99 4:38p Jlane
// INitial checkin.
// 
//
/*************************************************************************/

#include  <assert.h>          // debug stuff

#include "DdmVCM.h"
#include "RqOsVirtualMaster.h"
#include "CTEvent.h"
#include "CTUtils.h"
#include "Odyssey_Trace.h" 
#define TRACE_INDEX TRACE_VCM


//  DdmVCM::VirtualCircuitDelete (HANDLE hRequest, VCRequest *pRequest)
//
//  Description:
//    Called when our DDM is supposed to create a virtual Circuit.
//    Begin the process by reading the Specified Storage Roll Call Record.
//
//  Inputs:
//    hRequest - Handle of request we're call for.  We must return this
//                when we report request status.
//    pRequest - Request which we're to process.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VirtualCircuitDelete(HANDLE hRequest, VCRequest *pRequest)
{
VCCommandContext_t*	pCmdInfo;
STATUS 				status = OK;

	// Fill out a command context structure to keep the command info for all the callbacks.
	TRACEF(TRACE_L1, ("DdmVCM - VCDelete: Enter\n"));	
	pCmdInfo = new(tZERO) VCCommandContext_t;
	memcpy(&pRequest->rid, hRequest, sizeof(rowID));
	pCmdInfo->hRequest = hRequest;
	pCmdInfo->pRequest = pRequest;

#ifdef WIN32
	status = VCDelete_ReadExportRecord( pCmdInfo, status );
#else
	// Resolve: add the code to send message for VD Delete
	status = VCDelete_ReadExportRecord( pCmdInfo, status );
#endif
	return status;
}


//  DdmVCM::VCDelete_ReadExportRecord (void *pClientContext, STATUS status)
//
//  Description:
//    Read the export record to obtain the src entry so that we can
//	  mark it as free
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the previous PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCDelete_ReadExportRecord(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

		if (status){
			TRACEF(TRACE_L1, ("DdmVCM - VCDelete: Delete STS failed!\n"));	
		} else {
			TRACEF(TRACE_L2, ("DdmVCM - VCDelete: Delete STS Done!\n"));	
		}
		TSReadRow *pReadExportRecord = new TSReadRow;
		if (!pReadExportRecord)
			status = CTS_OUT_OF_MEMORY;
		else
			pCmdInfo->pData = new (tZERO) ExportTableEntry;
			status = pReadExportRecord->Initialize( 
				this,										// DdmServices *pDdmServices,
				EXPORT_TABLE,								// String64 rgbTableName,
				RID_VC_ID,									// String64 rgbKeyFieldName,
				&pCmdInfo->pRequest->u.VCDeleteParms.ridVcId,// void *pKeyFieldValue,
				sizeof(rowID),								// U32 cbKeyFieldValue,
				pCmdInfo->pData,							// void *prgbRowDataRet,
				sizeof(ExportTableEntry),					// U32 cbRowDataRetMax,
				NULL,										// U32 *pcRowsReadRet,
				TSCALLBACK(DdmVCM,VCDelete_DeleteStsVD),// pTSCallback_t pCallback,
				pCmdInfo									// void* pContext
			);
		
		if (status == OK)
			pReadExportRecord->Send();
		else
			status = CleanUpVCDelete(pCmdInfo, NULL, status);
		return status;
}


//  DdmVCM::VCDelete_DeleteStsVD (void *pClientContext, STATUS status)
//
//  Description:
//    Continue deleting a virtual Circuit. Deinstantiate the SCSI Target
//    Server Virtual Device.
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the previous PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCDelete_DeleteStsVD(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;
ExportTableEntry	*pExportTableEntry = (ExportTableEntry *)pCmdInfo->pData;

	if (status != OK){
		TRACEF(TRACE_L1, ("DdmVCM - VCDelete: Read Export Record failed!\n"));	
		return CleanUpVCDelete(pCmdInfo, &status, CTS_VCM_RECORD_DELETE_ERROR);
	} else {
		TRACEF(TRACE_L2, ("DdmVCM - VCDelete: Read Export Record successful!\n"));		
	
#ifndef WIN32
		RqOsVirtualMasterDeleteVirtualDevice *pMsg;
		pMsg = new  RqOsVirtualMasterDeleteVirtualDevice(pExportTableEntry->vdNext);
		status = Send(
			pMsg,
			pCmdInfo,
			REPLYCALLBACK(DdmVCM,VCDelete_DeleteSTSCfg));		
#endif
	}
	return status;
}


//  DdmVCM::VCDelete_DeleteSTSCfg (Message* pMsg)
//
//  Description:
//    Continue deleteing a virtual Circuit. Delete the SCSI Target Server
//    config record for the VC.
//
//  Inputs:
//    pMsg - reply to our VD delete msg.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCDelete_DeleteSTSCfg(Message *pMsg)
{
RqOsVirtualMasterDeleteVirtualDevice *pDeleteStsVDMsg = (RqOsVirtualMasterDeleteVirtualDevice *)pMsg;
STATUS	status = pDeleteStsVDMsg->Status();
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pDeleteStsVDMsg->GetContext();

	if (status != OK)
		;;	// todo log error???!!!???
		
	// delete the reply.
	delete pMsg;

	// Try to delete the SCSI Target Server config record flagged
	// by our current Virtual Circuit command's ID.
	m_pVCDDeleteSTSCfgRec = new TSDeleteRow;
	
	status = m_pVCDDeleteSTSCfgRec->Initialize( 
		this,										// DdmServices *pDdmServices,
		STS_CONFIG_TABLE_NAME,						// String64 rgbTableName,
		RID_VC_ID,									// String64 rgbKeyFieldName,
		&pCmdInfo->pRequest->u.VCDeleteParms.ridVcId,	// void *pKeyFieldValue,
		sizeof(rowID),								// U32 cbKeyFieldValue,
		0,											// U32 cRowsToDelete, 0 = all.
		NULL,										// U32 *pcRowsDelRet,
		TSCALLBACK(DdmVCM,VCDelete_ModifySRCRecord),	// pTSCallback_t pCallback,
		pCmdInfo									// void* pContext
	);
	
	if (status == OK)
		m_pVCDDeleteSTSCfgRec->Send();
	else
		status = CleanUpVCDelete(pCmdInfo, NULL, status);
	
	return status;
}	// VCDelete_DeleteSTSCfg


STATUS DdmVCM::VCDelete_ModifySRCRecord(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;
ExportTableEntry	*pExportTableEntry = (ExportTableEntry *)pCmdInfo->pData;

	if (status != OK){
		TRACEF(TRACE_L1, ("DdmVCM - VCDelete: Read Export Record failed!\n"));	
		return CleanUpVCDelete(pCmdInfo, &status, CTS_VCM_RECORD_DELETE_ERROR);
	} else {
		TRACEF(TRACE_L2, ("DdmVCM - VCDelete: Read Export Record successful!\n"));		
		status = VCM_SetSRCUsed(
					&pExportTableEntry->ridSRC,
					FALSE,
					TSCALLBACK(DdmVCM,VCDelete_DelExpRecs),
					pCmdInfo);
	}
	if (status != OK)
		status = CleanUpVCDelete(pCmdInfo, NULL, status);
	return status;
}



//  DdmVCM::VCDelete_DelExpRecs (void *pClientContext, STATUS status)
//
//  Description:
//    Continue deleting a virtual Circuit. Attempt to delete the Export
//    record(s).
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the previous PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCDelete_DelExpRecs(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	if (status != OK)
		return CleanUpVCDelete(pCmdInfo, &status, CTS_VCM_RECORD_DELETE_ERROR);
	
	// Try to delete all export records identified by our ridVcId parameter. 
	// Note that ridVcId is the VC's Create Virtual Circuit command's Row ID.
	m_pVCDDeleteExpRecs = new TSDeleteRow;
	if (!m_pVCDDeleteExpRecs)
		status = CTS_OUT_OF_MEMORY;
	else		
		status = m_pVCDDeleteExpRecs->Initialize( 
			this,									// DdmServices *pDdmServices,
			EXPORT_TABLE,							// String64 rgbTableName,
			RID_VC_ID,								// String64 rgbKeyFieldName,
			&pCmdInfo->pRequest->u.VCDeleteParms.ridVcId,	// void *pKeyFieldValue,
			sizeof(rowID),							// U32 cbKeyFieldValue,
			0,										// U32 cRowsToDelete, 0 = all.
			NULL,									// U32 *pcRowsDelRet,
			TSCALLBACK(DdmVCM,VCDelete_Finish),		// pTSCallback_t pCallback,
			pCmdInfo								// void* pContext
		);
	
	if (status == OK)
		m_pVCDDeleteExpRecs->Send();
	else
		status = CleanUpVCDelete(pCmdInfo, NULL, status);

	return status;
}	// VCDelete_DelExpRecs
		


//  DdmVCM::VCDelete_Finish (void *pClientContext, STATUS status)
//
//  Description:
//    Continue deleting a virtual Circuit. Reply to our caller.
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the previous PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCDelete_Finish(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	if (status == OK)
		status = CleanUpVCDelete(pCmdInfo, NULL, OK);
	else
		status = CleanUpVCDelete(pCmdInfo, &status, CTS_VCM_RECORD_DELETE_ERROR);
	
	return status;
}	// VCDelete_Finish
		


//  DdmVCM::CleanUpVCDelete (VCCommandContext_t* pCmdInfo, void* pReplyData, STATUS status)
//
//  Description:
//    Finish processing a Create Virtual Circuit command. Reply to the 
// command with the speciofied status and data and free all resources.  
//
//  Inputs:
//    pClientContext - A pointer to our command info structure which
//    contains a handle we return to report status and the request
//    we're processing.
//
//    pReplyData - Data returned along with the reply to the requestor
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    returns the passed in status.
//
STATUS DdmVCM::CleanUpVCDelete (VCCommandContext_t* pCmdInfo, void* pReplyData, STATUS status)
{
	if (pCmdInfo)
	{
		VCStatus	myStatus;
		
		// Finish filling out the reply structure of the create command.
		myStatus.rid.HiPart = 0;
		myStatus.rid.LoPart = 0;
		myStatus.rid.Table = 0;
		myStatus.version = 1;
		myStatus.size = sizeof(VCStatus);
		// The deleted Virtual Circuit's ID
		myStatus.ridVcId = pCmdInfo->pRequest->u.VCDeleteParms.ridVcId;
		myStatus.eVCEvent = VCDeleted;
		myStatus.status = status;
		if (pReplyData){
			memcpy( &myStatus.u, pReplyData, sizeof(STATUS));
		} else {
			myStatus.u.detailedstatus = status;
		}

		m_CmdServer.csrvReportCmdStatus(pCmdInfo->hRequest,
										status,
										&myStatus,
										pCmdInfo->pRequest);
		m_CmdServer.csrvReportEvent( status, &myStatus);
		if (pCmdInfo->pData)
			delete pCmdInfo->pData;
	}
	
#if false
	CheckFreeAndClear(m_pVCDListen4VD);
	CheckFreeAndClear(m_pVCDListenReplyType);
	CheckFreeAndClear(m_pVCDModifyVDStateField);
	CheckFreeAndClear(m_pVCDDeleteSTSCfgRec);
	CheckFreeAndClear(m_pVCDDeleteExpRecs);
#endif

	CheckFreeAndClear(pCmdInfo);

	return OK;
}	// VCDelete_Cleanup_Last


//  DdmVCM::VCMSetSRCUsed(rowID	*pRowId, BOOL isUsed, pTSCallback_t	pCallback, void	*pContext)
//
//  Description:
//    Set the SRC's fUsed field to either used or unused.
//
//  Inputs:
//    pRowId 	- row id of the src to change
//	  isUsed	- TRUE to set it to used, else false	
//    pCallback	- the callback address
//	  pContext	- the context data	
//
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    returns the passed in status.
//
STATUS DdmVCM::VCM_SetSRCUsed(
			rowID					*pRowId, 
			BOOL					isUsed, 
			pTSCallback_t			pCallback,
			void					*pContext)
{
		STATUS		status;
		U32	fUsed = isUsed;
		
		TSModifyField *pModifySRCRecord = new TSModifyField;
		if (!pModifySRCRecord)
			status = CTS_OUT_OF_MEMORY;
		else		
			status = pModifySRCRecord->Initialize(
				this,										// DdmServices *pDdmServices,
				STORAGE_ROLL_CALL_TABLE,					// String64 rgbTableName,
				CT_PTS_RID_FIELD_NAME,						// String64 rgbKeyFieldName,
				pRowId,										// void *pKeyFieldValue,
				sizeof(rowID),
				fdSRC_FUSED,
				&fUsed,
				sizeof(fUsed),
				0,											// U32 cRowsToModify,
				NULL,										// U32* pcRowsModifiedRet,
				NULL,										// rowID *prowIDRet,
				0,											// U32 cbMaxRowID,
				pCallback,									// pTSCallback_t pCallback,
				pContext									// void* pContext
		);

		if (status == OK)
			pModifySRCRecord->Send();
		return status;			
}
