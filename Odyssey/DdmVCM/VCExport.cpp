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
// File: VCExport.cpp
//
// Description:
//    Virtual Circuit Manager DDM Export Command methods.
//
// $Log: /Gemini/Odyssey/DdmVCM/VCExport.cpp $
// 
// 6     11/12/99 9:49a Dpatel
// changes for win32 port of VCM
// 
// 5     11/01/99 5:07p Dpatel
// Export, unexport event generated after we hear the listen reply on the
// ready
// state field. 
// 
// 4     10/15/99 3:58p Dpatel
// export event was not being properly sent
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
#include "CTEvent.h"
#include "Fields.h"

#include "Odyssey_Trace.h" 
#define TRACE_INDEX TRACE_VCM


//  DdmVCM::VirtualCircuitExportCtl(VCExportCtlCommand &VCExportCtlParms)
//
//  Description:
//    Called when our DDM is supposed to Qiesce Virtual Circuit(s).
//
//  Inputs:
//    hRequest - Handle of request we're call for.  We must return this
//                when we report request status.
//    pRequest - Request which we're to process.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VirtualCircuitExportCtl(HANDLE hRequest, VCRequest *pRequest)
{
VCCommandContext_t*	pCmdInfo;
STATUS 				status = OK;

	// Fill out a command context structure to keep the command info for all the callbacks.
	pCmdInfo = new(tZERO) VCCommandContext_t;
	memcpy(&pRequest->rid, hRequest, sizeof(rowID));
	pCmdInfo->hRequest = hRequest;
	pCmdInfo->pRequest = pRequest;

	status = VCExport_ExportVC( pCmdInfo, status );

	return status;
}


//  DdmVCM::VCExport_ExportVC(void *pClientContext, STATUS status)
//
//  Description:
//    Finish exporting a virtual Circuit. Export the Virtual Circuit
//    by modifying the Export Records ReadyState field to contian the
//    "configured" or "ConfiguredNotExported" as specified by the command.
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCExport_ExportVC(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;
CTReadyState		ready_state;
int					fAllVCs = pCmdInfo->pRequest->u.VCExportParms.fAllVCs;

	// Determine table modification parameters according to our command.
	if (pCmdInfo->pRequest->u.VCExportParms.fExport){
		ready_state = StateConfigured;				// Desired Ready state
		RegisterExportTableListener(&pCmdInfo->pRequest->u.VCExportParms.ridVcId, StateConfiguredAndExported);
	} else {
		ready_state = StateConfiguredNotExported;	// Desired Ready state
		RegisterExportTableListener(&pCmdInfo->pRequest->u.VCExportParms.ridVcId, StateConfiguredNotExported);		
	}
		
	
	TSModifyField*	pModifyExportRecFields = new TSModifyField;
	
	if (!pModifyExportRecFields)
		status = CTS_OUT_OF_MEMORY;
	else		
		status = pModifyExportRecFields->Initialize(
			this,											// DdmServices *pDdmServices,
			EXPORT_TABLE,									// String64 rgbTableName,
			fAllVCs 										// String64 rgbKeyFieldName,
				? CT_PTS_ALL_ROWS_KEY_FIELD_NAME 
				: RID_VC_ID,								
			fAllVCs											// void* pKeyFieldValue,
				? NULL
				: &pCmdInfo->pRequest->u.VCExportParms.ridVcId,
			fAllVCs											// U32 cbKeyFieldValue,
				? 0
				: sizeof(rowID),
			READY_STATE,									// String64 rgbFieldName,
			&ready_state,									// void* pFieldValue,
			sizeof(CTReadyState),							// U32 cbFieldValue,
			0,												// U32 cRowsToModify (0 => all)
			NULL,											// U32* pcRowsModifiedRet,
			NULL,											// rowID *prowIDRet,
			0,												// U32 cbMaxRowID,
			TSCALLBACK(DdmVCM,VCExport_Finish),				// pTSCallback_t pCallback,
			pCmdInfo										// void* pContext
		);

	if (status == OK)
		pModifyExportRecFields->Send();
	else
		status = CleanUpVCExport(pCmdInfo, NULL, status);

	return status;
}	// end of VCExport_ExportVC




//  DdmVCM::VCExport_Finish(void *pClientContext, STATUS status)
//
//  Description:
//    Finish exporting a virtual Circuit. Export the Virtual Circuit
//    by modifying the Export Records ReadyState field to contian the
//    "configured" or "ConfiguredNotExported" as specified by the command.
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCExport_Finish(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	status = CleanUpVCExport(pCmdInfo, NULL, status);

	return OK;
}	// end of VCExport_Finish


//  DdmVCM::CleanUpVCExport (VCCommandContext_t* pCmdInfo, void* pReplyData, STATUS status)
//
//  Description:
//    Finish processing a Export Virtual Circuit command. Reply to the 
// command with the specified status and data and free all resources.  
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
STATUS DdmVCM::CleanUpVCExport (VCCommandContext_t* pCmdInfo, void* pReplyData, STATUS status)
{
	pReplyData = pReplyData;
	if (pCmdInfo)
	{
		VCStatus	myStatus;
		
		// Finish filling out the reply structure of the create command.
		myStatus.rid.HiPart = 0;
		myStatus.rid.LoPart = 0;
		myStatus.rid.Table = 0;
		myStatus.version = 1;
		myStatus.size = sizeof(VCStatus);
		// The Virtual Circuit's ID that was exported
		myStatus.ridVcId = pCmdInfo->pRequest->u.VCExportParms.ridVcId;
		if (pCmdInfo->pRequest->u.VCExportParms.fExport){
			myStatus.eVCEvent = VCExported;
#ifdef WIN32
			// fake the modify field in the export table
			SimulateExportTableState(
				0, 
				&pCmdInfo->pRequest->u.VCExportParms.ridVcId,
				StateConfiguredAndExported);
#endif
		} else {
			myStatus.eVCEvent = VCUnExported;
#ifdef WIN32
			// fake the modify field in the export table
			SimulateExportTableState(
				0, 
				&pCmdInfo->pRequest->u.VCExportParms.ridVcId,
				StateConfiguredNotExported);
#endif
		}
		myStatus.status = status;
		myStatus.u.detailedstatus = status;
		m_CmdServer.csrvReportCmdStatus(pCmdInfo->hRequest,
										status,
										&myStatus,
										pCmdInfo->pRequest);
		// event is generated when we hear the listen reply on the export table										
	}
	
	return OK;
}	// end of DdmVCM::CleanUpVCCreate


//  DdmVCM::RegisterExportTableListener (VCCommandContext_t* pCmdInfo)
//
//  Description:
//    	Register a listener on the export table's ready state field. When it is set to 
//		StateConfiguredAndExported, generate an "Exported" event and when it is set to
//		StateConfiguredNotExported, generate an "Unexported event.
//
//  Inputs:
//    pClientContext - A pointer to our command info structure which
//    contains a handle we return to report status and the request
//    we're processing.
//
//
//  Outputs:
//    returns the passed in status.
//

STATUS DdmVCM::RegisterExportTableListener (rowID *pRidVcId, CTReadyState _readyState)
{
STATUS				status = 0;

	VCListenContext_t *pListenContext = new (tZERO) VCListenContext_t;
	
	pListenContext->ridVcId = *pRidVcId;
	pListenContext->readyState = _readyState;

	TSListen *pExportTableListener = new TSListen;
	if (!pExportTableListener)
		status = CTS_OUT_OF_MEMORY;
	else			
		// We are going to listen once only for the export table entry to 
		// change to exported...
		status = pExportTableListener->Initialize( 
			this,										// DdmServices* pDdmServices
			ListenOnModifyOneRowOneField,				// U32 ListenType
			EXPORT_TABLE,								// String64 prgbTableName
			RID_VC_ID,									
			&pListenContext->ridVcId,					// void* prgbRowKeyFieldValue
			sizeof(rowID),								// U32 cbRowKeyFieldValue
			READY_STATE,								// String64 prgbFieldName
			&pListenContext->readyState,				// void* prgbFieldValue
			sizeof(CTReadyState),						// U32 cbFieldValue
			ReplyOnceOnly,								// U32 ReplyMode
			NULL,										// void** ppTableDataRet,
			NULL,										// U32* pcbTableDataRet,
			&pListenContext->exportListenerID,			// U32* pListenerIDRet,
			&pListenContext->pExportListenReplyType,	// U32** ppListenTypeRet,
			NULL,										// void** ppModifiedRecordRet,
			NULL,										// U32* pcbModifiedRecordRet,
			TSCALLBACK(DdmVCM,ExportListenReply),		// pTSCallback_t pCallback,
			pListenContext								// void* pContext
		);
	
	if (status == OK)
		pExportTableListener->Send();
	return status;
}



STATUS DdmVCM::ExportListenReply(void *pClientContext, STATUS status)
{
VCListenContext_t*	pListenContext = (VCListenContext_t*)pClientContext;

	if (status == OK) {
		if (*pListenContext->pExportListenReplyType == ListenOnModifyOneRowOneField){	
			VCStatus	myStatus;		
			// Finish filling out the reply structure of the event (export/unexport)
			myStatus.rid.HiPart = 0;
			myStatus.rid.LoPart = 0;
			myStatus.rid.Table = 0;
			myStatus.version = 1;
			myStatus.size = sizeof(VCStatus);
			// The Virtual Circuit's ID that was exported/unexported
			myStatus.ridVcId = pListenContext->ridVcId;
			if (pListenContext->readyState == StateConfiguredAndExported)
				myStatus.eVCEvent = VCExported;
			if (pListenContext->readyState == StateConfiguredNotExported)
				myStatus.eVCEvent = VCUnExported;				
			myStatus.status = status;
			myStatus.u.detailedstatus = status;
			m_CmdServer.csrvReportEvent( status, &myStatus );
			delete pListenContext;
		}
	}
	return status;
}