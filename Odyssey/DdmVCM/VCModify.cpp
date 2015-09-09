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
// File: VCModify.cpp
//
// Description:
//    Virtual Circuit Manager DDM Delete Command methods.
//
// $Log: /Gemini/Odyssey/DdmVCM/VCModify.cpp $
// 
// 2     12/17/99 5:30p Dpatel
// added vc modify
// 
// 1     12/17/99 3:44p Dpatel
// Initial checkin
// 
// 
//
/*************************************************************************/

#include  <assert.h>          // debug stuff

#include "DdmVCM.h"
#include "CTEvent.h"
#include "CTUtils.h"
#include "Odyssey_Trace.h" 

#include "DdmVCMMsgs.h"
#define TRACE_INDEX TRACE_VCM


/***************************************************************
*
*	Resolve:
*		1) Quiesce code, enable code
*		2) Error codes change from Delete to Modify
*
*
*
***************************************************************/

STATUS DdmVCM::VirtualCircuitModify(Message *_pModifyMsg)
{
	MsgVCMModifyVC *pModifyMsg = (MsgVCMModifyVC *)_pModifyMsg;
	
	STATUS		status;

	TRACEF(TRACE_L1, ("DdmVCM: VCModify Enter msg received\n"));
	VCRequest	*pRequest;
	pModifyMsg->GetVCRequest((void **)&pRequest);
	status = VirtualCircuitModify(pModifyMsg,pRequest);
	return status;
}


//  DdmVCM::VirtualCircuitModify(HANDLE hRequest, VCRequest *pRequest)
//
//  Description:
//    Called when our DDM is supposed to modify a virtual Circuit.
//    Begin the process by quiescing the VCID
//
//  Inputs:
//    hRequest - Handle of request we're call for.  We must return this
//                when we report request status.
//    pRequest - Request which we're to process.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VirtualCircuitModify(HANDLE hRequest, VCRequest *pRequest)
{
VCCommandContext_t*	pCmdInfo;
STATUS 				status = OK;

	// Fill out a command context structure to keep the command info for all the callbacks.
	pCmdInfo = new(tZERO) VCCommandContext_t;
	memcpy(&pRequest->rid, hRequest, sizeof(rowID));
	pCmdInfo->hRequest = hRequest;
	pCmdInfo->pRequest = pRequest;

#ifdef WIN32
	status = VCModify_ModifySTSCfg( pCmdInfo, status );
#else
	// Resolve: add the code to quiesce the virtual circuit
	status = VCModify_ModifySTSCfg( pCmdInfo, status );
#endif
	return status;
}




//  DdmVCM::VCModify_ModifySTSCfg (void *pClientContext, STATUS status)
//
//  Description:
//    Modify the vdNext (to point to new vd) of the STS Config Record with vcid
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
STATUS DdmVCM::VCModify_ModifySTSCfg(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	if (status != OK) {
		TRACEF(TRACE_L1, ("DdmVCM: VCModify ModifySTS Config Error\n"));
		return CleanUpVCModify(pCmdInfo, &status, CTS_VCM_RECORD_DELETE_ERROR);
	}

	TRACEF(TRACE_L1, ("DdmVCM: VCModify ModifySTS Config Enter\n"));
	// Try to modify the SCSI Target Server config record flagged
	// by our current Virtual Circuit command's ID.
	TSModifyField *pVCModifySTSCfgRec = new TSModifyField;
	
	status = pVCModifySTSCfgRec->Initialize( 
		this,										// DdmServices *pDdmServices,
		STS_CONFIG_TABLE_NAME,						// String64 rgbTableName,
		RID_VC_ID,									// String64 rgbKeyFieldName,
		&pCmdInfo->pRequest->u.VCModifyParms.ridVcId,	// void *pKeyFieldValue,
		sizeof(rowID),								// U32 cbKeyFieldValue,
		"vdNext",									// field name for vd next
		&pCmdInfo->pRequest->u.VCModifyParms.vdNext,// the new vdNext
		sizeof(pCmdInfo->pRequest->u.VCModifyParms.vdNext),
		0,												// U32 cRowsToModify (0 => all)
		NULL,											// U32* pcRowsModifiedRet,
		NULL,											// rowID *prowIDRet,
		0,												// U32 cbMaxRowID,
		TSCALLBACK(DdmVCM,VCModify_ModifyExportRecs),	// pTSCallback_t pCallback,
		pCmdInfo										// void* pContext
	);
	
	if (status == OK)
		pVCModifySTSCfgRec->Send();
	else
		status = CleanUpVCModify(pCmdInfo, NULL, status);
	
	return status;
}	// VCModify_ModifySTSCfg



//  DdmVCM::VCModify_ModifyExportRecs (void *pClientContext, STATUS status)
//
//  Description:
//    Modify the export recs, src entry
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
STATUS DdmVCM::VCModify_ModifyExportRecs(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	if (status != OK){
		TRACEF(TRACE_L1, ("DdmVCM: VCModify ModifySTS Config Error\n"));
		return 	CleanUpVCModify(pCmdInfo, NULL, status);
	}

	TRACEF(TRACE_L1, ("DdmVCM: VCModify ModifySTS Config successful\n"));
	TSModifyField*	pModifyExportRecFields = new TSModifyField;
	
	if (!pModifyExportRecFields)
		status = CTS_OUT_OF_MEMORY;
	else		
		status = pModifyExportRecFields->Initialize(
			this,											// DdmServices *pDdmServices,
			EXPORT_TABLE,									// String64 rgbTableName,
			RID_VC_ID,								
			&pCmdInfo->pRequest->u.VCModifyParms.ridVcId,	// key 
			sizeof(rowID),
			"ridSRC",										// String64 rgbFieldName,
			&pCmdInfo->pRequest->u.VCModifyParms.srcNext,	// void* pFieldValue,
			sizeof(pCmdInfo->pRequest->u.VCModifyParms.srcNext),	// U32 cbFieldValue,
			0,												// U32 cRowsToModify (0 => all)
			NULL,											// U32* pcRowsModifiedRet,
			NULL,											// rowID *prowIDRet,
			0,												// U32 cbMaxRowID,
			TSCALLBACK(DdmVCM,VCModify_ModifySRCTable),		// pTSCallback_t pCallback,
			pCmdInfo										// void* pContext
		);

	if (status == OK)
		pModifyExportRecFields->Send();
	else
		status = CleanUpVCModify(pCmdInfo, NULL, status);

	return status;
}


//  DdmVCM::VCModify_ModifySRCTable (void *pClientContext, STATUS status)
//
//  Description:
//    Modify the SRC entry to used 
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
STATUS DdmVCM::VCModify_ModifySRCTable(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;
	if (status) {
		TRACEF(TRACE_L1, ("DdmVCM: VCModify Modify Export recs Config Error\n"));
	} else {
		TRACEF(TRACE_L1, ("DdmVCM: VCModify Export Recs successful\n"));	
	}
	
	status = VCM_SetSRCUsed(
					&pCmdInfo->pRequest->u.VCModifyParms.srcNext,
					TRUE,
					TSCALLBACK(DdmVCM,VCModify_ModifySTSCfgReply),
					pCmdInfo);
	if (status != OK)
		status = CleanUpVCModify(pCmdInfo, NULL, status);
	return status;			
}



//  DdmVCM::VCModify_ModifySTSCfgReply (void *pClientContext, STATUS status)
//
//  Description:
//		Now enable the virtual circuit again.
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
STATUS DdmVCM::VCModify_ModifySTSCfgReply(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	if (status != OK){
		TRACEF(TRACE_L1, ("DdmVCM: VCModify Set SRC used Error\n"));	
		return CleanUpVCModify(pCmdInfo, &status, CTS_VCM_RECORD_DELETE_ERROR);
	}

	TRACEF(TRACE_L1, ("DdmVCM: VCModify Set SRC used successful\n"));	
#ifdef WIN32	
		status = CleanUpVCModify(pCmdInfo, NULL, status);
#else
		// Resolve: add code to enable virtual circuit
		status = CleanUpVCModify(pCmdInfo, NULL, status);		
#endif

	return status;
}	
		

//  DdmVCM::CleanUpVCModify (VCCommandContext_t* pCmdInfo, void* pReplyData, STATUS status)
//
//  Description:
//    Finish processing a Modify Virtual Circuit command. Reply to the 
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
STATUS DdmVCM::CleanUpVCModify (VCCommandContext_t* pCmdInfo, void* pReplyData, STATUS status)
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
		myStatus.ridVcId = pCmdInfo->pRequest->u.VCModifyParms.ridVcId;
		myStatus.eVCEvent = VCModified;
		myStatus.status = status;
		if (pReplyData){
			memcpy( &myStatus.u, pReplyData, sizeof(STATUS));
		} else {
			myStatus.u.detailedstatus = status;
		}
#if 0
		// Resolve : support cmd queue interface??
			m_CmdServer.csrvReportCmdStatus(pCmdInfo->hRequest,
										status,
										&myStatus,
										pCmdInfo->pRequest);
	
			m_CmdServer.csrvReportEvent( status, &myStatus);
#endif
		// we got a message based request
		((Message *)pCmdInfo->hRequest)->DetailedStatusCode = status;
		Reply((Message *)pCmdInfo->hRequest);
		TRACEF(TRACE_L1, ("DdmVCM: VCModify successful, sending reply!\n"));	
	}
	
	CheckFreeAndClear(pCmdInfo);

	return OK;
}	// VCModify_Cleanup_Last
