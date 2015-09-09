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
// File: DdmVCM.h
// 
// Description:
//    Virtual Circuit Manager DDM definitions.
// 
// $Log: /Gemini/Odyssey/DdmVCM/DdmVCM.h $
// 
// 15    1/26/00 9:43a Szhang
// 
// 14    12/15/99 2:16p Dpatel
// VC delete functionality.
// 
// 13    11/12/99 9:49a Dpatel
// changes for win32 port of VCM
// 
// 12    11/06/99 2:16p Jlane
// Cleanup after using new VD creation message.  Remove timer and listen
// stuff.
// 
// 11    11/01/99 5:06p Dpatel
// Export, unexport event generated after we hear the listen reply on the
// ready
// state field. 
// 
// 10    10/27/99 5:09p Dpatel
// vccreate needed to set fUsed to used in SRC table if create
// successful..
// 
// 9     10/05/99 11:28a Agusev
// Only use 1 export record for now.
// 
// 7     9/06/99 8:03p Jlane
// Yet another interim checkin.
// 
// 6     9/05/99 4:40p Jlane
// Compiles and is theoretically ready to try.
// 
// 5     8/08/99 12:20p Jlane
// Interim checkin upon compiling
// 
// 4     8/05/99 11:36a Jlane
// Interim checkin to share the code.
// 
// 3     7/26/99 1:22p Jlane
// INterim checkin before rewrite supporting new HostConnectionDescriptor.
// 
// 2     7/26/99 1:21p Jlane
// INterim checkin before rewrite supporting new HostConnectionDescriptor.
// 
// 1     7/21/99 5:52p Jlane
// Initial Checkin of as yet unfinished work.
// 
// 
/*************************************************************************/

#ifndef _DdmVCM_h_
#define _DdmVCM_h_

#include "Ddm.h"

#ifndef __Table_h__
#include  "Table.h"       // PTS table defs
#endif

#ifndef __TSFields_h__
#include  "Fields.h"       // PTS Field defs
#endif

#ifndef __TSRows_h__
#include  "Rows.h"        // PTS row defs
#endif

#ifndef _SystemStatusTable_h
#include  "SystemStatusTable.h"
#endif

#ifndef _ReadTable_h_
#include  "ReadTable.h"   // more PTS table defs
#endif

#ifndef _Listen_h_
#include  "Listen.h"      // PTS listen interface
#endif

#ifndef __QUEUE_OPERATIONS_H__
#include  "CmdServer.h"      // control interface to our DDM
#endif

#ifndef _DdmVcmCommands_h_
#include "DdmVCMCommands.h"  // command/status queue interface defs
#endif

#ifndef __VirtualDeviceTable_h
#include "VirtualDeviceTable.h"
#endif

#ifndef __RqOsTimer_h
#include "RqOsTimer.h"  // Timer service messages
#endif


#include "FCPortDatabaseTable.h"
#include "StorageRollCallTable.h"
#include "HostConnectionDescriptorTable.h"
#include "LoopDescriptor.h"
#include "ExportTable.h"
#include "STSConfig.h"



class DdmVCM : public Ddm
{
public:
	
	//  our constructor, preserves our DID for posterity
	DdmVCM (DID did);
	
	//  how to make an instance of us (used deep in the bowels of CHAOS)
	static Ddm *Ctor (DID did);
	
	//  initialize, called the first time we are loaded (faulted-in, or whatever)
	virtual STATUS Initialize (Message *pMsg);
	//  our callbacks continuing the work of Initialize()
	// These first four are all PTSCallback_t prototypes.
	STATUS	DefineSTSConfig (void *pClientContext, STATUS status);
	STATUS	DefineSRCTable (void *pClientContext, STATUS status);
	STATUS	DefineExportTable (void *pClientContext, STATUS status);
	STATUS 	InitializeCmdSvr (void *pClientContext, STATUS status);
	//   InitializeLast is pInitializeCallback_t callback
	void	InitializeLast (STATUS status);

	//  called when our DDM instance is supposed to quiesce
	virtual STATUS Quiesce (Message *pMsg);
	
	//  called after we initialize, and each time we are unquiesced
	virtual STATUS Enable (Message *pMsg);
	
	// A global we use to make sure we're only instantiated once.
	static DdmVCM	*pDdmVCM;
	
private:
	
	// A structure passed from callback to callback during command execution.
	struct VCCommandContext_t
	{
		HANDLE		hRequest;
		VCRequest*	pRequest;
		VCStatus	Results;
		void		*pData;			// general purpose ptr
	};

	struct VCListenContext_t {	
		U32					exportListenerID;
		U32*				pExportListenReplyType;
		rowID				ridVcId;
		CTReadyState		readyState;
	};
	
	
	// Our command dispatcher method.
	void  CmdReady (HANDLE hRequest, void *pRequest);

	// Our VirtualCircuitCreate command handler entry point method:
	STATUS	VirtualCircuitCreate(HANDLE hRequest, VCRequest *pRequest);

	// The rest of the VirtualCircuitCreate Handlers:
	STATUS VCCreate_ReadEVCRecord(void *pClientContext, STATUS status);
	STATUS VCCreate_ReadSRCRecord(void *pClientContext, STATUS status);
	STATUS VCCreate_ReadFCPDBT(void *pClientContext, STATUS status);
	STATUS VCCreate_ReadHCDR(void *pClientContext, STATUS status);
	STATUS VCCreate_ReadLDR(void *pClientContext, STATUS status);
	STATUS VCCreate_Validate(void *pClientContext, STATUS status);
	STATUS VCCreate_Check4STSCfg(void *pClientContext, STATUS status);
	STATUS VCCreate_InsertSTSCfg(void *pClientContext, STATUS status);
	STATUS VCCreate_Check4STSVD(void *pClientContext, STATUS status);
	STATUS VCCreate_InstSTSVD(void *pClientContext, STATUS status);
	STATUS VCCreate_ProcessVDCreateReply(Message* pMsg);
	STATUS VCCreate_DelExpRecs(void *pClientContext, STATUS status);
	STATUS VCCreate_InsExpRecs(void *pClientContext, STATUS status);
	STATUS VCCreate_ExportVC(void *pClientContext, STATUS status);
	STATUS VCCreate_ModifySRCTable(void *pClientContext, STATUS status);
	STATUS VCCreate_SRCTableModifiedReply(void *pClientContext, STATUS status);
	int	VCCreate_FCPDBIndex( rowID	rid );

#ifdef WIN32
	STATUS SimulateExportTableState(U32 fExportRowId, rowID *pExportTableRowId, CTReadyState newState);
#endif

	// VirtualCircuitCreate cleanup handlers:
	STATUS CleanUpVCCreate(VCCommandContext_t* pCmdInfo, void* pReplyData, STATUS status);
	STATUS VCCreate_Cleanup_DelSTSCfgRec(void *pClientContext, STATUS status);
	STATUS VCCreate_Cleanup_DelExpRecs(void *pClientContext, STATUS status);
	STATUS VCCreate_Cleanup_Last(void *pClientContext, STATUS status);

	// VirtualCircuitCreate instance data
	TSDefineTable*					m_pDefineTable;
	TSReadTable*					m_pReadSystemStatusRec;
	TSReadRow*						m_pReadSRCRow;
	TSReadTable*					m_pReadFCPDBTable;
	TSReadTable*					m_pReadHCCTable;
	TSReadRow*						m_pReadHCDRRow;
	TSReadRow*						m_pReadHBADRRow;
	TSReadRow*						m_pReadLDRow;
	TSReadRow*						m_pReadSTSCfgRec;
	TSReadRow*						m_pReadVDTRec;
	TSInsertRow*					m_pInsertSTSConfigRec;
	TSDeleteRow*					m_pDeleteSTSCfgRec;
	TSInsertRow*					m_pInsertExportRecs;
	TSModifyRow*					m_pModifyExportRec;
	TSDeleteRow*					m_pDeleteExpRecs;
	SystemStatusRecord*				m_pSystemStatusRecord;
	StorageRollCallRecord			m_SRCRecord;
	HostConnectionDescriptorRecord	m_HostConnDescRec;
	FCPortDatabaseRecord*			m_pFCPortDatabaseTable;
	U32								m_cRowsFCPortDatabaseTable;
	LoopDescriptorRecord*			m_pLoopDescriptorRecs;
	U32								m_CurrLoopDescRecord;
	U32								m_CurrEIPArrayEntry;
	VCCreateResult					m_VCCreateResult;
	STS_CONFIG						m_SCSITargetConfigRec;
	ExportTableRecord				m_ExportTableRecs[1];
	rowID							m_ExportRecsRowIDs[1];
	U32								m_iExportRecToUpdate;
	VDN								m_vdnSTS;
	VirtualDeviceRecord				m_NewVDTRecord;



	// Our VirtualCircuitExport command handler entry point method:
	STATUS	VirtualCircuitExportCtl(HANDLE hRequest, VCRequest *pRequest);
	// The rest of the VirtualCircuitExport Handlers:
	STATUS 	VCExport_ExportVC(void *pClientContext, STATUS status);
	STATUS 	VCExport_Finish(void *pClientContext, STATUS status);
	STATUS	RegisterExportTableListener (rowID *pRidVcId, CTReadyState _readyState);
	STATUS	ExportListenReply(void *pClientContext, STATUS status);
	// VirtualCircuitExport cleanup handlers:
	STATUS 	CleanUpVCExport (VCCommandContext_t* pCmdInfo, void* pReplyData, STATUS status);


	// Our VirtualCircuitDelete command handler entry point method:
	STATUS	VirtualCircuitDelete(HANDLE hRequest, VCRequest *pRequest);
	// The rest of the VirtualCircuitDelete Handlers:
	STATUS	VCDelete_ReadExportRecord(void *pClientContext, STATUS status);
	STATUS	VCDelete_DeleteStsVD(void *pClientContext, STATUS status);
	STATUS	VCDelete_DeleteSTSCfg(Message *pMsg);
	STATUS	VCDelete_ModifySRCRecord(void *pClientContext, STATUS status);
	STATUS 	VCDelete_DelExpRecs(void *pClientContext, STATUS status);
	STATUS 	VCDelete_Finish(void *pClientContext, STATUS status);

	// VirtualCircuitDelete cleanup handlers:
	STATUS	CleanUpVCDelete (VCCommandContext_t* pCmdInfo, void* pReplyData, STATUS status);

	// VirtualCircuitDelete instance data
	//TSTimedListen*					m_pVCDListen4VD;
	//U32*							m_pVCDListenReplyType;
	TSModifyField*					m_pVCDModifyVDStateField;
	TSDeleteRow*					m_pVCDDeleteSTSCfgRec;
	TSDeleteRow*					m_pVCDDeleteExpRecs;


	// Our VirtualCircuitQuiesce command handler entry point method:
    STATUS	VirtualCircuitQuiesceCtl(HANDLE hRequest, VCRequest *pRequest);

	// The rest of the VirtualCircuitQuiesce Handlers:
	STATUS	VCQuiesce_Listen4VD(void *pClientContext, STATUS status);

	// VirtualCircuitQuiesce cleanup handlers:
	STATUS 	CleanUpVCQuiesce (VCCommandContext_t* pCmdInfo, void* pReplyData, STATUS status);


	// VCModify
	STATUS VirtualCircuitModify(Message *pRequestMsg);
	STATUS VirtualCircuitModify(HANDLE hRequest, VCRequest *pRequest);
	STATUS VCModify_ModifySTSCfg(void *pClientContext, STATUS status);
	STATUS VCModify_ModifyExportRecs(void *pClientContext, STATUS status);
	STATUS VCModify_ModifySRCTable(void *pClientContext, STATUS status);
	STATUS VCModify_ModifySTSCfgReply(void *pClientContext, STATUS status);
	STATUS CleanUpVCModify (VCCommandContext_t* pCmdInfo, void* pReplyData, STATUS status);
	// VCModify END

	STATUS VCM_SetSRCUsed(
			rowID					*pRowId, 
			BOOL					isUsed, 
			pTSCallback_t			pCallback,
			void					*pContext);


	// Miscellaneous instance data
	CmdServer   					m_CmdServer;
	VCCommandContext_t				m_CmdInfo;
	Message*						m_pInitMsg;
	
};  /* end of class DdmVCM */


#endif   // #ifndef _DdmVCM_h_

