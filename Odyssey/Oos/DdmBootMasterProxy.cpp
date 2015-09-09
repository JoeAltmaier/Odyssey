// DdmBootMasterProxy.cpp -- The Boot Process Mgr DDM.
//
// Copyright (C) ConvergeNet Technologies, 1998 
//
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Change log located at end of file.

#define JFLsWAY // wrk - change when tom nelson merges in hid changes 

#include <String.h>
#include "OsTypes.h"
#include "Odyssey_Trace.h"
#include "DdmBootMasterProxy.h"
#include "BootMgrCmds.h"
#include "BootMgrMsgs.h"
#include "IopProxy.h"
#include "DdmCmbMsgs.h"
#include "Table.h"
#include "Rows.h"
#include "RqOsDdmManager.h"
#include "RqOsTransport.h"
#include "CTUtils.h"
#include "DefaultImageTable.h"
#include "UpgradeMasterMessages.h"


#include "Trace_Index.h"
// UpgradeMasterMessages.h defines TRACE_INDEX so I need to undef it.
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#endif
#define TRACE_INDEX TRACE_BOOT
#include "Odyssey_Trace.h" 

#include "BuildSys.h"
CLASSNAME(DdmBootMasterProxy, SINGLE);	// Class Link Name used by Buildsys.cpp

// Serve The IOP Out Of Service Request. 
SERVELOCAL(DdmBootMasterProxy,REQ_IOP_OUT_OF_SERVICE);
SERVELOCAL(DdmBootMasterProxy,REQ_IOP_OUT_OF_SERVICE_INT);	// internal msg to take Iop OffLine 

extern "C" STATUS InitBridgeFTree();	// Defined in pcimap.c
extern "C" int image_hdr( U32 block, void** pImgAdrRet, U32* pImgSizRet);	// defined in imgmgr.c


// DdmBootMasterProxy -- Constructor -------------------------------------------DdmBootMasterProxy-
//
DdmBootMasterProxy::DdmBootMasterProxy(DID did): Ddm(did)
{
	TRACE_ENTRY(DdmBootMasterProxy::DdmBootMasterProxy);
	
	SetConfigAddress(NULL,0);	// tell Ddm:: where my config area is
	m_ImgSiz = 0;
	m_pImgHdr = NULL;
	m_pMsgIopOutOfSvc = NULL;
	m_pMsgIopIntoSvc = NULL;
	m_iSlotCount = 0;
	m_iROMImgBlk = 0;
	m_fAllIopsAreUp = false;
	m_fAllIopsOnPciFlag = false;
	
	DispatchRequest(REQ_IOP_OUT_OF_SERVICE,
					REQUESTCALLBACK(DdmBootMasterProxy, HandleReqIopOutOfService));
	DispatchRequest(REQ_IOP_OUT_OF_SERVICE_INT,
					REQUESTCALLBACK(DdmBootMasterProxy, HandleReqIopOutOfServiceInt));
	#if false
	DispatchRequest(REQ_IOP_INTO_SERVICE,
					REQUESTCALLBACK(DdmBootMasterProxy, HandleReqIopIntoService));
	DispatchRequest(REQ_IOP_INTO_SERVICE_INT,
					REQUESTCALLBACK(DdmBootMasterProxy, HandleReqIopIntoServiceInt));
	#endif
	memset( &m_aIopManProxys, 0, sizeof(m_aIopManProxys) );
}
	

// Ctor -- Create ourselves --------------------------------------------DdmBootMasterProxy-
//
Ddm *DdmBootMasterProxy::Ctor(DID did)
{
	return new DdmBootMasterProxy(did);
}


// Initialize -- Do post-construction initialization -------------------DdmBootMasterProxy-
//
STATUS DdmBootMasterProxy::Initialize(Message *pMsg)
{ 
STATUS	status;

	TRACE_ENTRY(DdmBootMasterProxy::Initialize);
	m_pInitializeMsg = pMsg;
	status = DefineSystemStatusTable();
	return status;
}


// DefineSystemStatusTable -- Insure SystemStatusTable exists. 
//
STATUS DdmBootMasterProxy::DefineSystemStatusTable()
{ 
STATUS	status;

	TRACE_ENTRY(DdmBootMasterProxy::DefineSystemStatusTable);
	
	// This is the code to create the SystemStatusTable.
		  
	// Allocate an Define Table object for the System Status Table.
	TSDefineTable*	pDefineTable = new TSDefineTable;

	// Initialize the define Table object.
	status = pDefineTable->Initialize(
		this,								// DdmServices* pDdmServices,
		SYSTEM_STATUS_TABLE,				// String64 prgbTableName,
		aSystemStatusTable_FieldDefs,		// fieldDef* prgFieldDefsRet,
		cbSystemStatusTable_FieldDefs,		// U32 cbrgFieldDefs,
		1,									// U32 cEntriesRsv,
		false,								// bool* pfPersistant,
		(pTSCallback_t)&CheckSystemStatusTable,// pTSCallback_t pCallback,
		NULL								// void* pContext
	);
	
	// Initiate the define table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pDefineTable->Send();

	return status;
}


// CheckSystemStatusTable -- Check if SystemStatusTable was defined.
//
STATUS DdmBootMasterProxy::CheckSystemStatusTable(void *pClientContext, STATUS status)
{

	TRACE_ENTRY(DdmBootMasterProxy::CheckSystemStatusTable);

	if (status == ercTableExists)
	{
		TRACEF( TRACE_L8, ("\n***DdmBootMasterProxy: Found SystemStatusTable.\n"));
		// System Status Table already exists. Try to read record.
		status = ReadSystemStatusRec(pClientContext, status);
	}
	else
	if (status == OK)
	{
		// We need to insert a new SystemStatusRecord.		
		status = CreateSystemStatusRec(pClientContext, status);
	}
	else
	{
		TRACEF(TRACE_L3,("\n***DdmBootMasterProxy: DefineSystemStatusTable = 0x%x.\n", status));
		// Throw an error here!
	}
	return OK;
}

	
// ReadSystemStatusRec -- Read existing SystemStatusRecord.
//
STATUS DdmBootMasterProxy::ReadSystemStatusRec(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMasterProxy::ReadSystemStatusRec);

	TRACEF( TRACE_L8, ("\n***DdmBootMasterProxy: Reading SystemStatusRec...\n"));

	// Read the SystemStatusRecord.	
	TSReadRow*	pReadSystemStatusRec = new TSReadRow;
	
	status = pReadSystemStatusRec->Initialize
	(
		this,							// DdmServices *pDdmServices
		SYSTEM_STATUS_TABLE,			// String64	rgbTableName
		CT_PTS_ALL_ROWS_KEY_FIELD_NAME,	// String64	rgbKeyFieldName
		NULL,							// void *pKeyFieldValue
		0,								// U32 cbKeyFieldValue
		&m_SystemStatusRecord,			// void *prgbRowDataRet
		sizeof(m_SystemStatusRecord),	// U32 cbRowDataRetMax
		NULL,							// U32 *pcRowsReadRet
		(pTSCallback_t)&CheckSystemStatusRec,	// pTSCallback_t pCallback
		NULL							// void *pContext
	);
	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pReadSystemStatusRec->Send();
		
	return status;
}



// CreateSystemStatusRec - Initialize and insert a new SystemStatusRecord
//
STATUS DdmBootMasterProxy::CreateSystemStatusRec(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)
	TRACE_ENTRY(DdmBootMasterProxy::CreateSystemStatusRec);

	TRACEF( TRACE_L8, ("\n***DdmBootMasterProxy: Inserting new SystemStatusRec.\n"));
	// Init a new SystemStatusRecord.
	memset( &m_SystemStatusRecord, 0, sizeof(SystemStatusRecord));
	m_SystemStatusRecord.version = SYSTEM_STATUS_TABLE_VER;
	m_SystemStatusRecord.size = sizeof(SystemStatusRecord);
	
	// Alloc, init and send off a insert row object for the new
	// SystemStatusTable.
	TSInsertRow*	pCreateSystemStatusRec = new TSInsertRow;
	
	status = pCreateSystemStatusRec->Initialize(
		this,							// DdmServices pDdmServices,
		SYSTEM_STATUS_TABLE,			// String64 rgbTableName,
		&m_SystemStatusRecord,			// void* prgbRowData,
		sizeof(SystemStatusRecord),		// U32 cbRowData,
		&m_SystemStatusRecord.rid,		// rowID* pRowIDRet,
		(pTSCallback_t)&CheckSystemStatusRec,	// pTSCallback_t pCallback,
		NULL							// void* pContext );
	);
	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pCreateSystemStatusRec->Send();

	return status;
}



// CheckSystemStatusRec - Check that SystemStatusRecord is OK.
//
STATUS DdmBootMasterProxy::CheckSystemStatusRec(void *pClientContext, STATUS status)
{
	TRACE_ENTRY(DdmBootMasterProxy::CheckSystemStatusRec);

	if (status != OK)
	{
		TRACEF( TRACE_L3, ("\n***DdmBootMasterProxy: ERROR with SystemStatusRec: 0x%x.\n",status));
		return	CreateSystemStatusRec(pClientContext, status);
	}
	else
		TRACEF( TRACE_L8, ("\n***DdmBootMasterProxy: SystemStatusRec OK!\n"));
	
//	status = HaveCMBUpdateEVCStatusRecord(pClientContext, status);
	status = ReadEVCStatusRecord(pClientContext, status);
	return status;
}



// HaveCMBUpdateEVCStatusRecord - 
// This method sends a message to the CMB Ddm to have it populate the
// EVCStatusRecord.  
STATUS DdmBootMasterProxy::HaveCMBUpdateEVCStatusRecord(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMasterProxy::HaveCMBUpdateEVCStatusRecord);

#if false	
	m_pCmbUpdEvcSRMsg = new MsgCmbUpdateEvcStatus;
	
	// Send the message to the CMB. No Context.
	status = Send(	m_pCmbUpdEvcSRMsg, 
					NULL,
					(ReplyCallback)&ReadEVCStatusRecord
				 );
				 
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		delete m_pCmbUpdEvcSRMsg;
		m_pCmbUpdEvcSRMsg = NULL;
		;	// TODO: Throw some event here.
	}
#endif
	return status;
}


// ReadEVCStatusRecord -- Read existing EVCStatusRecord.
//
STATUS DdmBootMasterProxy::ReadEVCStatusRecord(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMasterProxy::ReadEVCStatusRecord);

	TRACEF( TRACE_L8, ("\n***DdmBootMasterProxy:Reading SystemStatusTable...\n"));

	// Read the SystemStatusRecord.	
	TSReadRow*	pReadEVCStatusRec = new TSReadRow;
	
	status = pReadEVCStatusRec->Initialize
	(
		this,							// DdmServices *pDdmServices
		SYSTEM_STATUS_TABLE,			// String64	rgbTableName
		CT_PTS_ALL_ROWS_KEY_FIELD_NAME,	// String64	rgbKeyFieldName
		NULL,							// void *pKeyFieldValue
		0,								// U32 cbKeyFieldValue
		&m_EVCStatusRecord,				// void *prgbRowDataRet
		sizeof(m_EVCStatusRecord),		// U32 cbRowDataRetMax
		NULL,							// U32 *pcRowsReadRet
		(pTSCallback_t)&CheckEVCStatusRecord,	// pTSCallback_t pCallback
		NULL							// void *pContext
	);
	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pReadEVCStatusRec->Send();
		
	return status;
}



// CheckEVCStatusRecord - 
// This method verifies the EVCStatusRecord is OK to proceed booting.  
STATUS DdmBootMasterProxy::CheckEVCStatusRecord(void *pClientContext, STATUS status)
{
	TRACE_ENTRY(DdmBootMasterProxy::CheckEVCStatusRecord);

	// Do checking of EVC data here.
	
	// status of EVC Data? 
	if (status == OK)
		TRACEF( TRACE_L3, ("\n***DdmBootMasterProxy: EVCStatusRec OK!\n"))
	else
		TRACEF( TRACE_L3, ("\n***DdmBootMasterProxy: ERROR with EVCStatusRec:0x%x.\n",status));
	
	return ReadOldIopStatusTable(pClientContext, status);
}


// ReadOldIopStatusTable -
// Read the existing IopStatusTable to verify the presence of Iops.
STATUS DdmBootMasterProxy::ReadOldIopStatusTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMasterProxy::ReadOldIopStatusTable);

	TRACEF( TRACE_L3, ("\n***DdmBootMasterProxy:Reading Old IOPStatusTable...\n"));

	// Alloc, init and send off a read row object for the new
	// IopStatusTable.
	TSReadTable *pReadOldIopStatusTable = new TSReadTable;
	
	status = pReadOldIopStatusTable->Initialize(
		this,											// DdmServices pDdmServices,
		CT_IOPST_TABLE_NAME,							// String64 rgbTableName,
		&m_pOldIopStatusTableRows,						// void* &ppTableDataRet, returned table data.
		&m_nOldIopStatusTableRows,						// U32 *pcRowsRet,			// returned # of rows read
		(pTSCallback_t)&HaveCMBUpdateIopStatusTable,	// pTSCallback_t pCallback,
		this
	);
	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pReadOldIopStatusTable->Send();

	return status;
}



// HaveCMBUpdateIopStatusTable -- 
// Have the CMB ddm update the Iop Status Table.
//
STATUS DdmBootMasterProxy::HaveCMBUpdateIopStatusTable(void *pClientContext, STATUS status)
{ 
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMasterProxy::HaveCMBUpdateIopStatusTable);
	
	if (status != OS_DETAIL_STATUS_SUCCESS)
		TRACEF( TRACE_L3, ("\n***DdmBootMasterProxy: ERROR reading Old IOPStatusTable: 0x%x.\n",status));
		
	TRACEF( TRACE_L8, ("\n***DdmBootMasterProxy: Updating IOPStatusTable...\n"));

	m_pCmbPollAllIopsMsg = new MsgCmbPollAllIops;
	
	// Send the message to the CMB. No Context.
	status = Send(	m_pCmbPollAllIopsMsg, 
					NULL,
					(ReplyCallback)&ReadNewIopStatusTable
				 );
				 
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		delete m_pCmbPollAllIopsMsg;
		m_pCmbPollAllIopsMsg = NULL;
		;	// TODO: Throw some event here.
	}

	return status;
}



// ReadNewIopStatusTable -
// Read the current IopStatusTable to verify the presence of Iops.
STATUS DdmBootMasterProxy::ReadNewIopStatusTable(Message*	pMsg)
{
STATUS status = pMsg->DetailedStatusCode;

	TRACE_ENTRY(DdmBootMasterProxy::ReadNewIopStatusTable);

	if (status != OS_DETAIL_STATUS_SUCCESS)
		TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy:  ERROR from CMB updating IOPStatusTable.  Status = 0x%x.\n", status));

	TRACEF( TRACE_L8, ("\n***DdmBootMasterProxy:Reading updated IOPStatusTable...\n"));

	// We're done with the CMB message.  We can delete it now.
	delete m_pCmbPollAllIopsMsg;
	m_pCmbPollAllIopsMsg = NULL;

	// Alloc, init and send off a read row object for the new
	// IopStatusTable.
	TSReadTable *pReadIopStatusTable = new TSReadTable;
	
	status = pReadIopStatusTable->Initialize(
		this,											// DdmServices pDdmServices,
		CT_IOPST_TABLE_NAME,							// String64 rgbTableName,
		&m_pIopStatusTableRows,							// void* &ppTableDataRet, returned table data.
		&m_nIopStatusTableRows,							// U32 *pcRowsRet,			// returned # of rows read
		(pTSCallback_t)&CheckIopStatusTable,		// pTSCallback_t pCallback,
		this
	);
	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pReadIopStatusTable->Send();

	return status;
}


// GetIopTypeCode(IOPStatusRecord* pIopStatusRec);
// This method returns the three letter code for the Iop.  
const char*	const DdmBootMasterProxy::GetIopTypeCode(IOPStatusRecord* pIopStatusRec)
{
    switch(pIopStatusRec->IOP_Type) {
    case IOPTY_HBC:	// 1
	    return("HBC");
	case IOPTY_NAC:	// 2
		return("NAC");
	case IOPTY_SSD:	// 3
		return("SSD");
	case IOPTY_NIC:	// 4
		return("NIC");
	case IOPTY_RAC:	// 5
		return("RAC");
	case IOPTY_EVC:	// 129
		return("EVC");
	case IOPTY_DDH:	// 130
		return("DDH");
	default:
		return("BAD");		
    };
}


// GetIopTypeDesc(IOPStatusRecord* pIopStatusRec);
// This method returns the type description string for the Iop.  
// Max len returned = 43
const char*	const DdmBootMasterProxy::GetIopTypeDesc(IOPStatusRecord* pIopStatusRec)
{
    switch(pIopStatusRec->IOP_Type) {
    case IOPTY_HBC:	// 1
	    return("(HBC) Host Bridge Controller.");
	case IOPTY_NAC:	// 2
		return("(NAC) Network Array Controller.");
	case IOPTY_SSD:	// 3
		return("(SSD) Solid State Device.");
	case IOPTY_NIC:	// 4
		return("(NIC) Network Interface Controller.");
	case IOPTY_RAC:	// 5
		return("(RAC) RAID Array Controller.");
	case IOPTY_EVC:	// 129
		return("(EVC) Environmental Controller (not an Iop).");
	case IOPTY_DDH:	// 130
		return("(DDH) Disk Drive FC Hub (not an Iop).");
	default:
		return("Invalid or Unknown!");		
    };
}


// GetIopStateCode(IOPStatusRecord* pIopStatusRec);
// This method returns the Iop_State code string for the Iop.  
// Max len returned = 16
const char*	const DdmBootMasterProxy::GetIopStateCode(IOPStatusRecord* pIopStatusRec)
{
    switch(pIopStatusRec->eIOPCurrentState) {
	case IOPS_UNKNOWN:			// 0
		return("UNKNOWN         ");
	case IOPS_EMPTY:			// 1
		return("EMPTY           ");
	case IOPS_BLANK:			// 2
		return("BLANK           ");
	case IOPS_POWERED_ON:		// 3
		return("POWERED_ON      ");
	case IOPS_AWAITING_BOOT:	// 4
		return("AWAITING_BOOT   ");
	case IOPS_DIAG_MODE:		// 5
		return("DIAG_MODE       ");
	case IOPS_BOOTING:			// 6
		return("BOOTING         ");
	case IOPS_LOADING:			// 7
		return("LOADING         ");
	case IOPS_OPERATING:		// 8
		return("OPERATING       ");
	case IOPS_SUSPENDED:		// 9
		return("SUSPENDED       ");
	case IOPS_FAILING:			// 10
		return("FAILING         ");
	case IOPS_FAILED:			// 11
		return("FAILED          ");
	case IOPS_QUIESCENT:		// 12
		return("QUIESCENT       ");
	case IOPS_POWERED_DOWN:		// 13
		return("POWERED_DOWN    ");
	case IOPS_UNLOCKED:			// 14
		return("UNLOCKED        ");
	case IOPS_IMAGE_CORRUPT:	// 15
		return("IMAGE_CORRUPT   ");
	case IOPS_INSUFFICIENT_RAM:	// 16
		return("INSUFFICIENT_RAM");
	default:
		return("Corrupt/Invalid!");
    }
	Tracef("\n");	
}

// GetIopStateDesc(IOPStatusRecord* pIopStatusRec);
// This method returns the Iop_State description string for tghe Iop.  
// Max returned len = 51
const char*	const DdmBootMasterProxy::GetIopStateDesc(IOPStatusRecord* pIopStatusRec)
{
    switch(pIopStatusRec->eIOPCurrentState) {
	case IOPS_UNKNOWN:			// 0
		return("Iop state has not been polled");
	case IOPS_EMPTY:			// 1
		return("Empty.  No card in slot");
	case IOPS_BLANK:			// 2
		return("Blank/Filler card in slot");
	case IOPS_POWERED_ON:		// 3
		return("Iop just powered on");
	case IOPS_AWAITING_BOOT:	// 4
		return("Iop awaiting boot");
	case IOPS_DIAG_MODE:		// 5
		return("Iop running diagnostics");
	case IOPS_BOOTING:			// 6
		return("Iop boot ROM handed off control to boot image");
	case IOPS_LOADING:			// 7
		return("Iop image running, loading system entries");
	case IOPS_OPERATING:		// 8
		return("Iop operating normally (OS / app-level code)");
	case IOPS_SUSPENDED:		// 9
		return("Iop is suspended (no PCI accesses)");
	case IOPS_FAILING:			// 10
		return("Iop failure detected, Iop is being failed over");
	case IOPS_FAILED:			// 11
		return("Iop failed over, may now be shut down or rebooted");
	case IOPS_QUIESCENT:		// 12
		return("Iop in quiesced state");
	case IOPS_POWERED_DOWN:		// 13
		return("Iop is powered down");
	case IOPS_UNLOCKED:			// 14
		return("Iop's card locking solenoid has been released");
	case IOPS_IMAGE_CORRUPT:	// 15
		return("Iop was told to boot an image, & found it corrupt");
	case IOPS_INSUFFICIENT_RAM:	// 16
		return("Iop has insufficient RAM to perform requested boot");
	default:
		return("Iop state Unknown or Corrupt!");
    };
}


// DumpIopStatusRec - 
// This method displays the IopStatusrecord.  
void DdmBootMasterProxy::DumpIopStatusRec(U32	TraceLvl, IOPStatusRecord* pIopStatusRec, const char* const pString)
{
	TRACE_ENTRY(DdmBootMasterProxy::DumpIopStatusRec);

	TRACEF(TraceLvl,("\n%s IopStatusRecord Fields:", pString));	
    TRACEF(TraceLvl,("\n    Iop_Type            = (%04d) %s.", pIopStatusRec->IOP_Type, GetIopTypeDesc(pIopStatusRec)) );
    TRACEF(TraceLvl,("\n    Slot                = %d.", pIopStatusRec->Slot));
    TRACEF(TraceLvl,("\n    Redundant Slot      = %d.", pIopStatusRec->RedundantSlot));
	TRACEF(TraceLvl,("\n    Manufacturer        = %s.", pIopStatusRec->Manufacturer));
	TRACEF(TraceLvl,("\n    ulAvrSwVersion      = 0x%x.", pIopStatusRec->ulAvrSwVersion));
	TRACEF(TraceLvl,("\n    ulAvrSwRevision     = 0x%x.", pIopStatusRec->ulAvrSwRevision));
	TRACEF(TraceLvl,("\n    strHwPartNo         = %s.", pIopStatusRec->strHwPartNo));
 	TRACEF(TraceLvl,("\n    ulHwRevision        = 0x%x.", pIopStatusRec->ulHwRevision));
	TRACEF(TraceLvl,("\n    ulAvrSwRevision     = 0x%x.", pIopStatusRec->ulAvrSwRevision));
	TRACEF(TraceLvl,("\n    SerialNumber        = %s.", pIopStatusRec->SerialNumber));
	TRACEF(TraceLvl,("\n    ChassisSerialNumber = %s.", pIopStatusRec->ChassisSerialNumber));
	TRACEF(TraceLvl,("\n    ulAvrSwRevision     = 0x%x.", pIopStatusRec->ulAvrSwRevision));
	TRACEF(TraceLvl,("\n    ulIopEpldRevision   = 0x%x.", pIopStatusRec->ulIopEpldRevision));
	TRACEF(TraceLvl,("\n    ulIopMipsSpeed      = 0x%x.", pIopStatusRec->ulIopMipsSpeed));
	TRACEF(TraceLvl,("\n    ulIopPciSpeed       = 0x%x.", pIopStatusRec->ulIopPciSpeed));
	TRACEF(TraceLvl,("\n    eIopCurrentState    = (%04d) %s.", pIopStatusRec->eIOPCurrentState, GetIopStateDesc(pIopStatusRec)));
	TRACEF(TraceLvl,("\n"));	
}



// Dump2IopStatusRecs - 
// This method displays two IopStatusrecords aide by side.  
void DdmBootMasterProxy::Dump2IopStatusRecs(U32	TraceLvl, 
									IOPStatusRecord* pIopStatusRec1,
									char* pString1,
									IOPStatusRecord* pIopStatusRec2,
									char* pString2
									)
{
	TRACE_ENTRY(DdmBootMasterProxy::Dump2IopStatusRecs);

	TRACEF(TraceLvl,("\nIopStatusRecord           !                      !                     "));
	TRACEF(TraceLvl,("\n                          %-22s  %-22s", pString1, pString2));	
    TRACEF(TraceLvl,("\n    Iop_Type            = (%04d)  %-4s           (%04d) %-4s", pIopStatusRec1->IOP_Type, GetIopTypeCode(pIopStatusRec1),pIopStatusRec2->IOP_Type, GetIopTypeCode(pIopStatusRec2)));
    TRACEF(TraceLvl,("\n    Slot                = %-22d  %-22d", pIopStatusRec1->Slot, pIopStatusRec2->Slot));
    TRACEF(TraceLvl,("\n    Redundant Slot      = %-22d  %-22d", pIopStatusRec1->RedundantSlot, pIopStatusRec2->RedundantSlot));
	TRACEF(TraceLvl,("\n    Manufacturer        = %-22s  %-22s", pIopStatusRec1->Manufacturer, pIopStatusRec2->Manufacturer));
	TRACEF(TraceLvl,("\n    ulAvrSwVersion      = %-22x  %-22x", pIopStatusRec1->ulAvrSwVersion, pIopStatusRec2->ulAvrSwVersion));
	TRACEF(TraceLvl,("\n    ulAvrSwRevision     = %-22x  %-22x", pIopStatusRec1->ulAvrSwRevision, pIopStatusRec2->ulAvrSwRevision));
	TRACEF(TraceLvl,("\n    strHwPartNo         = %-22s  %-22s", pIopStatusRec1->strHwPartNo, pIopStatusRec2->strHwPartNo));
 	TRACEF(TraceLvl,("\n    ulHwRevision        = %-22x  %-22x", pIopStatusRec1->ulHwRevision, pIopStatusRec2->ulHwRevision));
	TRACEF(TraceLvl,("\n    ulAvrSwRevision     = %-22x  %-22x", pIopStatusRec1->ulAvrSwRevision, pIopStatusRec2->ulAvrSwRevision));
	TRACEF(TraceLvl,("\n    SerialNumber        = %-22s  %-22s", pIopStatusRec1->SerialNumber, pIopStatusRec2->SerialNumber));
	TRACEF(TraceLvl,("\n    ChassisSerialNumber = %-22s  %-22s", pIopStatusRec1->ChassisSerialNumber, pIopStatusRec2->ChassisSerialNumber));
	TRACEF(TraceLvl,("\n    ulAvrSwRevision     = %-22x  %-22x", pIopStatusRec1->ulAvrSwRevision, pIopStatusRec2->ulAvrSwRevision));
	TRACEF(TraceLvl,("\n    ulIopEpldRevision   = %-22x  %-22x", pIopStatusRec1->ulIopEpldRevision, pIopStatusRec2->ulIopEpldRevision));
	TRACEF(TraceLvl,("\n    ulIopMipsSpeed      = %-22x  %-22x", pIopStatusRec1->ulIopMipsSpeed, pIopStatusRec2->ulIopMipsSpeed));
	TRACEF(TraceLvl,("\n    ulIopPciSpeed       = %-22x  %-22x", pIopStatusRec1->ulIopPciSpeed, pIopStatusRec2->ulIopPciSpeed));
	TRACEF(TraceLvl,("\n    eIopCurrentState    = (%02d) %-16s  (%02d) %-16s", pIopStatusRec1->eIOPCurrentState, GetIopStateCode(pIopStatusRec1), pIopStatusRec2->eIOPCurrentState, GetIopStateCode(pIopStatusRec2)));
	TRACEF(TraceLvl,("\n"));	
}



// CheckIopStatusTable - 
// This method verifies the IopStatusTable is OK to proceed booting.  
STATUS DdmBootMasterProxy::CheckIopStatusTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

U32		fIopTableOK = TRUE;
U32		m,n;

	TRACE_ENTRY(DdmBootMasterProxy::CheckIopStatusTable);

	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy: ERROR with IOPStatusTable.  Status = 0x%x.\n",status));
		// TODO Throw an error here!
	}
	else
	{
		TRACEF( TRACE_L8, ("\n***DdmBootMasterProxy: Reconciling old and new IOPStatusTables...\n"));

		// Copy The returned IopStatusTable into our local records.
		m_cbIopStatusTableRows = m_nIopStatusTableRows * sizeof(IOPStatusRecord);
		bcopy ((const char*)m_pIopStatusTableRows, (char*)&m_IopStatusTableRows, m_cbIopStatusTableRows );
		m_pIopStatusTableRows = m_IopStatusTableRows;
		
		// Do checking of Iop data here.
		for (n = 0; n < m_nOldIopStatusTableRows; n++)
			if (
				// If there was no old Iop and there is a new one...
				((m_pOldIopStatusTableRows[n].eIOPCurrentState == IOPS_UNKNOWN) ||
				(m_pOldIopStatusTableRows[n].eIOPCurrentState == IOPS_EMPTY) ||
				(m_pOldIopStatusTableRows[n].eIOPCurrentState == IOPS_BLANK))  &&
				
				((m_pIopStatusTableRows[n].eIOPCurrentState != IOPS_UNKNOWN) &&
				(m_pIopStatusTableRows[n].eIOPCurrentState != IOPS_EMPTY) &&
				(m_pIopStatusTableRows[n].eIOPCurrentState != IOPS_BLANK))
			   )
			{	// We have a new Iop.
				DumpIopStatusRec( TRACE_L3, &m_pIopStatusTableRows[n], "***DdmBootMasterProxy: Newly Discovered" );
			}
			else
			if (
				// If there was an old Iop and there is NOT a new one...
				((m_pOldIopStatusTableRows[n].eIOPCurrentState != IOPS_UNKNOWN) &&
				(m_pOldIopStatusTableRows[n].eIOPCurrentState != IOPS_EMPTY) &&
				(m_pOldIopStatusTableRows[n].eIOPCurrentState != IOPS_BLANK)) &&
	
				((m_pIopStatusTableRows[n].eIOPCurrentState == IOPS_UNKNOWN) ||
				(m_pIopStatusTableRows[n].eIOPCurrentState == IOPS_EMPTY) ||
				(m_pIopStatusTableRows[n].eIOPCurrentState == IOPS_BLANK)) 
			   )
			{	// We have a missing Iop.
				DumpIopStatusRec( TRACE_L3, &m_pIopStatusTableRows[n], "***DdmBootMasterProxy: Un-Discovered" );
			}
			else
			// If the old and new serial #s match...
			if (strcmp(m_pOldIopStatusTableRows[n].SerialNumber, m_pIopStatusTableRows[n].SerialNumber) == 0)
			{
				// We have the same Iop
				TRACEF( TRACE_L3, ("\n***DdmBootMasterProxy: Rediscovered IOP Moved in Slot %d.\n", m_pOldIopStatusTableRows[n].Slot));
				DumpIopStatusRec( TRACE_L8, &m_pIopStatusTableRows[n], "***DdmBootMasterProxy: ReDiscovered" );
			}
			else
			{
				// We have an unexpected Iop.
				U32	fFound = false;
				for (m = 0; m < m_nIopStatusTableRows; m++)
					if (strcmp( m_pOldIopStatusTableRows[n].SerialNumber, m_pIopStatusTableRows[m].SerialNumber) == 0)
					{
						fFound = true;
						break;
					};
					
				if (fFound)
				{
					// The Iop has been moved!
					TRACEF( TRACE_L3, ("\n***DdmBootMasterProxy: An IOP Moved from Slot %d to Slot %d!\n", m_pOldIopStatusTableRows[n].Slot, m_pIopStatusTableRows[m].Slot));
					Dump2IopStatusRecs( TRACE_L3,
										&m_pOldIopStatusTableRows[n], "Old",
										&m_pIopStatusTableRows[m], "New"
									  );
				}
				else
				{
					// An Iop has been replaced!
					TRACEF( TRACE_L3, ("\n***DdmBootMasterProxy: An IOP has been replaced!\n"));
					Dump2IopStatusRecs( TRACE_L3,
										&m_pOldIopStatusTableRows[n], "Old", 
										&m_pIopStatusTableRows[n], "New"
									  );
					fIopTableOK = m_pOldIopStatusTableRows[n].IOP_Type == m_pIopStatusTableRows[n].IOP_Type;
				}						
			}
		
		
		// Status of Iop Data? 
		if (fIopTableOK)
			TRACEF( TRACE_L3, ("\n***DdmBootMasterProxy: IopStatusTable OK!\n"))
		else
			TRACEF( TRACE_L3, ("\n***DdmBootMasterProxy:  ERROR with IopStatusTable:0x%x.\n",status));
		
		status = FinishInitialize();
	}
		
	return status;
}



// ReadDefaultImageTable -- Read existing DefaultImageTable.
//
#ifdef JFLsWAY
STATUS DdmBootMasterProxy::ReadDefaultImageTable(void *pClientContext, STATUS status)
{
	TRACE_ENTRY(DdmBootMasterProxy::ReadDefaultImageTable);
	TRACEF( TRACE_L8, ("\n***DdmBootMasterProxy: Reading DefaultImageTable...\n"));

	// Read the ReadImageDescTable.	
	TSReadTable*	pReadDefaultImageTable = new TSReadTable;
	
	status = pReadDefaultImageTable->Initialize(
		this,									// DdmServices pDdmServices,
		(char*)DefaultImageRecord::TableName(),	// String64 prgbTableName,
		&m_pDefaultImageTable,					// void* &ppTableDataRet, returned table data.
		&m_nDefaultImageTableRows,				// U32 *pcRowsRet,			// returned # of rows read
		(pTSCallback_t)&CheckDefaultImageTable,	// pTSCallback_t pCallback,
		pClientContext
	);

	if (status == OS_DETAIL_STATUS_SUCCESS)
		pReadDefaultImageTable->Send();
		
	return status;
}
#else	// TOMSWAY
//ReadDefaultImageTable -  This is the code to read the Default Image Table.
STATUS DdmBootMasterProxy::ReadDefaultImageTable(Message* pMsg)
{
TRACE_ENTRY(DdmBootMasterProxy::ReadDefaultImageTable);
STATUS	status = pMsg->GetStatus();

	if (status != OK)
	{
		TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy:ReadDefaultImageTable received the following message with status = 0x%x.",status);
		pMsg->Dump("Message:");
	}  
	delete pMsg;
		
	// Allocate and send a Read Table msg for the Image Descriptor Table.
	DefaultImageRecord::RqEnumTable* pRqPtsReadTable = new DefaultImageRecord::RqEnumTable;
	status = Send(	pRqPtsReadTable,
					REPLYCALLBACK(DdmBootMasterProxy, CheckDefaultImageTable)
				 );
	return status;
}
#endif


// CheckDefaultImageTable -- Check if DefaultImageTable needs to be defined.
//
#ifdef JFLsWAY
STATUS DdmBootMasterProxy::CheckDefaultImageTable(void *pClientContext, STATUS status)
{

	TRACE_ENTRY(DdmBootMasterProxy::CheckDefaultImageTable);

	if (status == ercTableNotfound)
	{
		// Default Image Table Doesn't exist define it.
		status = DefineDefaultImageTable(pClientContext, status);
	}
	else
	if (status == OK)
	{
		TRACEF( TRACE_L8, ("\n***DdmBootMasterProxy: Found ImageDescTable.\n"));
		// ImageDescTable already exists. Try to update it.
		m_iROMImgBlk = 0;
		status = InitDefaultImageTable(pClientContext, status);
	}
	else
	{
		TRACEF(TRACE_L3,("\n***DdmBootMasterProxy: ReadDefaultImageTable returned status = 0x%x.\n", status));
		// Throw an error here!
	}
	return status;
}
#else
STATUS DdmBootMasterProxy::CheckDefaultImageTable(Message* pMsg)
{
TRACE_ENTRY(DdmBootMasterProxy::CheckDefaultImageTable);
STATUS	status;

	if (pMsg)
	{
		status = pMsg->GetStatus()
		if (status != OK)
		{
			TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy:CheckDefaultImageTable received the following message with status = 0x%x.",status);
			pMsg->Dump("Message:");
		}  
		delete pMsg;
	}	
	else
		status = OK;
		
	DefaultImageRecord::RqEnumTable* 	pRqPtsEnumTable;

	pRqPtsEnumTable = (DefaultImageRecord::RqEnumTable*)pMsg;
	m_pDefaultIMageTable = pRqPtsEnumTable->GetSglDataCopy( &m_nDefaultImageTableRows, sizeof(DefaultImageRecord) );
	delete pMsg;
		
	if (status == ercTableNotfound)
	{
		// Default Image Table Doesn't exist define it.
		status = DefineDefaultImageTable(pMsg);
	}
	else
	if (status == OK)
	{
		TRACEF( TRACE_L8, ("\n***DdmBootMasterProxy: Found ImageDescTable.\n"));
		// ImageDescTable already exists. Try to update it.
		m_iROMImgBlk = 0;
		status = InitDefaultImageTable(pMsg);
	}
	else
	{
		TRACEF(TRACE_L3,("\n***DdmBootMasterProxy: ReadDefaultImageTable returned status = 0x%x.\n", status));
		// Throw an error here!
	}
	return status;
}
#endif



// DefineDefaultImageTable - Define The DefaultImageTable.
//
#ifdef JFLsWAY
STATUS DdmBootMasterProxy::DefineDefaultImageTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)
TRACE_ENTRY(DdmBootMasterProxy::DefineDefaultImageTable)
	
	// This is the code to create the DefaultImageTable.
	// Allocate an Define Table object for the DefaultImageTable.
	TSDefineTable*	pDefineTable = new TSDefineTable;
	
	// Initialize the define Table object.
	status = pDefineTable->Initialize(
		this,									// DdmServices* pDdmServices,
		(char*)DefaultImageRecord::TableName(),	// String64 prgbTableName,
		DefaultImageRecord::FieldDefs(),		// fieldDef* prgFieldDefsRet,
		DefaultImageRecord::FieldDefsSize(),	// U32 cbrgFieldDefs,
		4,										// U32 cEntriesRsv,
		true,									// bool* pfPersistant,
		(pTSCallback_t)&ReadDefaultImageTable,	// pTSCallback_t pCallback,
		NULL									// void* pContext
	);
	
	// Initiate the define table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pDefineTable->Send();
	return status;
}
#else
STATUS DdmBootMasterProxy::DefineDefaultImageTable(Message* pMsg)
{
TRACE_ENTRY(DdmBootMasterProxy::DefineDefaultImageTable);
STATUS	status;

	if (pMsg)
	{
		status = pMsg->GetStatus()
		if (status != OK)
		{
			TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy:DefineDefaultImageTable received the following message with status = 0x%x.",status);
			pMsg->Dump("Message:");
		}  
		delete pMsg;
	}	
	else
		status = OK;

	// This is the code to create the Default ImageDescTable.
	// Allocate and send a Define Table msg for the Image Descriptor Table.
	// TOM HOW DO I SET THE DEFAULT NUMBER OF RECORDS?
	DefaultImageRecord::RqDefineTable* pRqPtsDefineTable = new DefaultImageRecord::RqDefineTable;
	status = Send(	pRqPtsDefineTable,
					REPLYCALLBACK(DdmBootMasterProxy, ReadDefaultImageTable)
				 );
	return status;
}
#endif


// Return 0 if RomImgHdr version equals ImageDescriptorRecord version.
// Return 1 if RomImgHdr version is more recent than ImageDescriptorRecord version.
// Return -1 if RomImgHdr version is older than ImageDescriptorRecord version.
int DdmBootMasterProxy::CompareVersions( img_hdr_t* pROMImgHdr, DefaultImageRecord* pDefaultImageRec)
{
// HACK: we dont't want thhis image it's the wrong type.
// let's say it's older in hopes the user won't take it.
if (pROMImgHdr->i_type != pDefaultImageRec->type)
	return -1;
	
if (pROMImgHdr->i_year > pDefaultImageRec->year)
	return 1;	// ROM image yr > our imagee year ie ROM image is newer.
if (pROMImgHdr->i_year < pDefaultImageRec->year)
	return -1;	// ROM image yr < our image year ie ROM image is older.
if (pROMImgHdr->i_month > pDefaultImageRec->month)
	return 1;	// ROM image month > our image month: ROM image is newer.
if (pROMImgHdr->i_month < pDefaultImageRec->month)
	return -1;	// ROM image month < our image month: ROM image is older.
if (pROMImgHdr->i_day < pDefaultImageRec->day)
	return -1;	// ROM image day > our image day: ROM image is newer.
if (pROMImgHdr->i_day < pDefaultImageRec->day)
	return 1;	// ROM image day < our image day: ROM image is older.
if (pROMImgHdr->i_hour < pDefaultImageRec->hour)
	return -1;	// ROM image day > our image day: ROM image is newer.
if (pROMImgHdr->i_hour < pDefaultImageRec->hour)
	return 1;	// ROM image hour < our image hour: ROM image is older.
if (pROMImgHdr->i_min < pDefaultImageRec->minute)
	return -1;	// ROM image day > our image day: ROM image is newer.
if (pROMImgHdr->i_min < pDefaultImageRec->minute)
	return 1;	// ROM image day < our image day: ROM image is older.
if (pROMImgHdr->i_sec < pDefaultImageRec->second)
	return -1;	// ROM image day > our image day: ROM image is newer.
if (pROMImgHdr->i_sec < pDefaultImageRec->second)
	return 1;	// ROM image hour < our image hour: ROM image is older.
return 0;	// image timestamps are equal.
}


// DisplayImageHdr - Display interesting info from an Image header (img_hdr_t in imghdr.h)
void DdmBootMasterProxy::DisplayImageHdr( img_hdr_t* pROMImgHdr, const char* const pString )
{
TRACEF(TRACE_ALL_LVL, ("\n%s\n", pString));
TRACEF(TRACE_ALL_LVL, ("	Timestamp: %d:%d:%d\n", 
					pROMImgHdr->i_hour, pROMImgHdr->i_min, pROMImgHdr->i_sec));
TRACEF(TRACE_ALL_LVL, ("	Datestamp: %d-%d-%d\n", 
					pROMImgHdr->i_month, pROMImgHdr->i_day, pROMImgHdr->i_year));
TRACEF(TRACE_ALL_LVL, ("	Version: %d.%d.%d\n", 
					pROMImgHdr->i_mjr_ver, pROMImgHdr->i_mnr_ver, 0 ));
}

// DisplayImageDesc - Display interesting info from an ImageDescRecord.
void DdmBootMasterProxy::DisplayImageDesc( ImageDescRecord* pImgDescRec, const char* const pString )
{
TRACEF(TRACE_ALL_LVL, ("\n***DdmBootMasterProxy: %s ImageDescRecord data:\n", pString));
TRACEF(TRACE_ALL_LVL, ("	Timestamp: %d:%d:%d\n", 
					pImgDescRec->hour, pImgDescRec->minute, pImgDescRec->second));
TRACEF(TRACE_ALL_LVL, ("	Datestamp: %d-%d-%d\n",
					pImgDescRec->month, pImgDescRec->day, pImgDescRec->year));
TRACEF(TRACE_ALL_LVL, ("	Version: %d.%d.%d\n", 
					pImgDescRec->majorVersion, pImgDescRec->minorVersion, 0 ));
}

// DisplayDefImage - Display interesting info from an DefaultImageRecord.
void DdmBootMasterProxy::DisplayDefaultImage( DefaultImageRecord* pDefaultImageRec, const char* const pString )
{
TRACEF(TRACE_ALL_LVL, ("\n***DdmBootMasterProxy: %s ImageDescRecord data:\n", pString));
TRACEF(TRACE_ALL_LVL, ("	Timestamp: %d:%d:%d\n", 
					pDefaultImageRec->hour, pDefaultImageRec->minute, pDefaultImageRec->second));
TRACEF(TRACE_ALL_LVL, ("	Datestamp: %d-%d-%d\n",
					pDefaultImageRec->month, pDefaultImageRec->day, pDefaultImageRec->year));
TRACEF(TRACE_ALL_LVL, ("	Version: %d.%d.%d\n", 
					pDefaultImageRec->majorVersion, pDefaultImageRec->minorVersion, 0 ));
}




// InitDefaultImageTable -  Iniatialize the DefaultImageTable with contents
// of boot ROM managed image flash.
#ifdef JFLsWAY
STATUS DdmBootMasterProxy::InitDefaultImageTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)
TRACE_ENTRY(DdmBootMasterProxy::InitDefaultImageTable)
	m_iROMImgBlk = 0;
	TRACEF( TRACE_L8, ("\n***BootMgr: Scanning ROM IMage flash..."));
	status = UpdateDefaultImageTable(NULL);
	return status;
}
#else
STATUS DdmBootMasterProxy::InitDefaultImageTable(Message* pMsg)
{
TRACE_ENTRY(DdmBootMasterProxy::InitDefaultImageTable);
STATUS	status = pMsg->Status();

	// 
	delete pMsg;
	
	// Prepare to scan the image flash.
	// m_iROMImgBlk will be our loop control and index.
	m_iROMImgBlk = 0;
	TRACEF( TRACE_L3, ("\n***BootMgr: Scanning ROM IMage flash..."));
	status = UpdateDefaultImageTable(NULL);
	return status;
}
#endif


// UpdateDefaultImageTable -  Iniatialize the DefaultImageTable with contents
// of boot ROM managed image flash.
STATUS DdmBootMasterProxy::UpdateDefaultImageTable(Message *pMsg)
{
TRACE_ENTRY(DdmBootMasterProxy::UpdateDefaultImageTable);
STATUS	status;

	if (pMsg)
	{
		status = pMsg->Status();
		if (status != OK)
		{
			TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy:UpdateDefaultImageTable received the following message with status = 0x%x.",status));
			pMsg->Dump("Message:");
		}
		delete pMsg;
	}	
	else
		status = OK;

	// Code to update the DefaultImageTable with boot flash images.
	// m_iROMImgBlk is the index for the bootROM image flash 'block'.
	// We're going to scan the defaultImageTable for outdated code.
	while (m_iROMImgBlk < 3)
	{
		// Get image addr in flash incrementing our block index.
		status = image_hdr(m_iROMImgBlk++, &m_pImgHdr, &m_ImgSiz);
		
		// HACK ALERT:  image_hdr needs event codes.  for now: status == 0 => OK.
		// The only other possible  error is invalid image header signature.
		if (status != OK)
		{
			status = CTS_IOP_BAD_IMAGE_SIGNATURE;
			TRACEF( TRACE_L3, ("\***bootMgr: Invalid Image Signature in Flash block %d.",m_iROMImgBlk));
			continue;
		}
		
		TRACEF( TRACE_L8, ("\***bootMgr: Found a valid Image in Flash block %d.",m_iROMImgBlk));
		
		// Initialize a few object state members.
		m_pDefImgRecToUpdate = NULL;	// so I know to add new default image.
		
		// initialize a few local flags
		U32	fWantImage = false;	// flags if user wants to update to newer image.
		U32	fAddImage = false;	// flags if image should be added after loop.
		U32	fHaveImageType = false;	// flags new image types we don't have.

		// Scan the default image table.
		for (U32 j = 0; j < m_nDefaultImageTableRows; j++)
		{
			// If the current ROM flash image type matches the current default image 
			// and is more recent (remembering if we have an image of this type)...
			if ((fHaveImageType != (m_pImgHdr->i_type == m_pDefaultImageTable[j].type)) &&
				(CompareVersions(m_pImgHdr, &m_pDefaultImageTable[j]) == 1))
			{
				// ROM has later version.  Prompt to take it?
				DisplayImageHdr( m_pImgHdr,
								"\n***DdmBootMasterProxy: The boot ROM flash contains the image...");
				DisplayDefaultImage( &m_pDefaultImageTable[j],
								  "\nWhich is newer than the current default image, which is:" );
				TRACEF( TRACE_ALL_LVL, ("\nWould you like to update our default image with the one from the boot flash?"));
				char c = getchar();
				// If the user wants it get it.
				if (c == 'y' || c == 'Y')
				{
					m_pDefImgRecToUpdate = &m_pDefaultImageTable[j];
					fWantImage = fAddImage = true;
					break;
				}
			}	// if (...
		}	// for (j...
		
		// If we don't have this image type and it's one of the following...
		if (!fHaveImageType &&
			((ImageType)m_pImgHdr->i_type == HBC_IMAGE) ||
			((ImageType)m_pImgHdr->i_type == NAC_IMAGE) ||
			((ImageType)m_pImgHdr->i_type == SSD_IMAGE))
			// make sure we get it.  This should be the first execution path.
			fAddImage = true;
			
		// So, do we want the image or not...
		if (fAddImage)
		{
			// Allocate, Initialize, and send a DdmUpgradeMgr msg
			// to add the image to the PTS Image Descriptor Table.
			MsgAddImage* pMsgAddImage
			 = new MsgAddImage(m_ImgSiz, m_pImgHdr);
			status = Send(	pMsgAddImage,
							REPLYCALLBACK(DdmBootMasterProxy, HandleAddNewImageReply) );
			if (status == OK)
			{
				// Todo throw error here.
				;
			}
		}	// if (fAddImage...
	} // while (...

	status = OK;
	return status;
}


// 
STATUS DdmBootMasterProxy::HandleAddNewImageReply(Message* pMsg)
{
TRACE_ENTRY(DdmBootMasterProxy::HandleAddNewImageReply);
MsgAddImage*	pMsgAddImage = (MsgAddImage*)pMsg;
STATUS			status = pMsgAddImage->Status();
 
	if (status != OK)
	{
		TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy: ERROR Inserting image.  Status = 0x%x.\n",status));
		// TODO Throw an error here!
	}

	// If we are not updating an existing default image...
	if (!m_pDefImgRecToUpdate)
	{
		// Try to find an empty place in the default image table for it.
		// ALL_IMAGES (0 in enum ImageType in UpgradeMasterMsgs.h)) is
		// an invalid image type and handily will indicate a default
		// image table row that has not been initialized with an image.
		U32 i = 0;
		for (i = 0; i < m_nDefaultImageTableRows; i++)
			if (m_pDefaultImageTable[i].type == ALL_IMAGES)
			{
				m_pDefImgRecToUpdate = &m_pDefaultImageTable[i];
				break;
			}
	}	
	
	// If we are not updating an existing default image record....
	if (!m_pDefImgRecToUpdate)
	{	// We're adding a new default image record.

		// Calculate old and new default image table sizes and alloc mem for new.
		U32 cbDefaultImageTable = sizeof(DefaultImageRecord) * m_nDefaultImageTableRows++;
		U32 cbNewDefaultImageTable = sizeof(DefaultImageRecord) * m_nDefaultImageTableRows;
		DefaultImageRecord* pNewDefaultImageTable = (DefaultImageRecord*) new char[cbNewDefaultImageTable];
		
		// Copy any existing default image table data into new memory.
		if (m_pDefaultImageTable && cbDefaultImageTable)
		{
			DefaultImageRecord* pOldDefaultImageTable = m_pDefaultImageTable;
			memcpy( pNewDefaultImageTable, m_pDefaultImageTable, cbDefaultImageTable );
			CheckFreeAndClear(pOldDefaultImageTable); 
		}
		
		// Update our DefaultImageTable Pointer and record count and
		// set the pointer to the new record in the newly allocated table.
		m_pDefaultImageTable = pNewDefaultImageTable;
		m_pDefImgRecToUpdate = &m_pDefaultImageTable[m_nDefaultImageTableRows++];
		m_pDefImgRecToUpdate->rid.Clear();	
	}

	// Update the DefaultImageRecord with the new image data.
	m_pDefImgRecToUpdate->majorVersion	= m_pImgHdr->i_mjr_ver;
	m_pDefImgRecToUpdate->minorVersion	= m_pImgHdr->i_mnr_ver;
	m_pDefImgRecToUpdate->day			= m_pImgHdr->i_day;
	m_pDefImgRecToUpdate->month			= m_pImgHdr->i_month;
	m_pDefImgRecToUpdate->year			= m_pImgHdr->i_year;
	m_pDefImgRecToUpdate->hour			= m_pImgHdr->i_hour;
	m_pDefImgRecToUpdate->minute		= m_pImgHdr->i_min;
	m_pDefImgRecToUpdate->second		= m_pImgHdr->i_sec;
	m_pDefImgRecToUpdate->type			= (ImageType)m_pImgHdr->i_type;
	m_pDefImgRecToUpdate->imageKey = pMsgAddImage->GetImageKey();
	
	// Insert or Modify the new DefaultImageRecord as appropriate. 
	if (m_pDefImgRecToUpdate->rid.IsClear())
	{
		// Alloc, Init, and send a message to the PTS to Insert the new row.
		DefaultImageRecord::RqInsertRow* pRqInsertRow
		 = new DefaultImageRecord::RqInsertRow(m_pDefImgRecToUpdate, 1);
		status = Send( pRqInsertRow, REQUESTCALLBACK( DdmBootMasterProxy, HandleDefaultImageReply));
	}
	else
	{
		// Allocate, initialize and send a modify msg for the DefaultImageRecord.	
		DefaultImageRecord::RqModifyRow* pRqModifyRow;
		pRqModifyRow = new DefaultImageRecord::RqModifyRow(
			CT_PTS_RID_FIELD_NAME,				// const char *_psKeyFieldName, 
			&m_pDefImgRecToUpdate->rid,			// const void *_pKeyFieldValue,
			sizeof(m_pDefImgRecToUpdate->rid),	// U32 _cbKeyFieldValue,
			*m_pDefImgRecToUpdate				// const REC *_prgbRowData,
			// sizeof(DefaultImageRecord)			// U32 _cbRowData,
		 );
		status = Send( pRqModifyRow, REQUESTCALLBACK( DdmBootMasterProxy, HandleDefaultImageReply));
	}

	// make sure everything went ok and return status
    assert (status == CTS_SUCCESS);
	return status;

}


STATUS DdmBootMasterProxy::HandleDefaultImageReply(Message* pMsg)
{
TRACE_ENTRY(DdmBootMasterProxy::HandleDefaultImageReply);
STATUS	status = pMsg->Status();
 
	if (status != OK)
	{
		TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy: ERROR Inserting default image.  Status = 0x%x.\n",status));
		// TODO Throw an error here!
	}
	delete pMsg;
	
	// We've just finished updating the DefaultImageTable to reflect a new
	// image we found during our scan of the Boot ROM image flash and added. 
	// If we still have flash blocks to scan call UpdateDefaultImageTable.
	if (m_iROMImgBlk < 3)
		status = UpdateDefaultImageTable(NULL);
	else
	// Otherwise On with initialization.  Call DefineIopImageTable().
		status = DefineIopImageTable(NULL, status);
	return status;
}


#ifdef TOMsWAY
STATUS DdmBootMasterProxy::DefineIopImageTable(Message pMsg)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMasterProxy::DefineIopImageTable);

	TRACEF( TRACE_L3, ("\n***DdmBootMasterProxy:DefineIopImageTable returned status 0x%x.\n", status));
	IOPImageRecord::RqDefineTable	*pRqPtsefineIOPImageTable = new IOPImageRecord::RqDefineTable(Persistant_PT, NSLOT);
	Send(pRqPtsDefineIOPImageTable, REPLYCALLBACK(DdmBootMasterProxy,InitializeIopImageTable));
	return status;
}
#endif


// DefineIopImageTable -- Insure IOPImageTable exists. 
//
STATUS DdmBootMasterProxy::DefineIopImageTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)
TRACE_ENTRY(DdmBootMasterProxy::DefineIopImageTable);
	
	// This is the code to create the IOPImageTable.
		  
	// Allocate an Define Table object for the IOPImage Table.
	TSDefineTable*	pDefineTable = new TSDefineTable;

	// Initialize the define Table object.
	status = pDefineTable->Initialize(
		this,								// DdmServices* pDdmServices,
		(char*)IOPImageRecord::TableName(),		// String64 prgbTableName,
		IOPImageRecord::FieldDefs(),		// fieldDef* prgFieldDefsRet,
		IOPImageRecord::FieldDefsSize(),	// U32 cbrgFieldDefs,
		NSLOT,								// U32 cEntriesRsv,
		true,								// bool* pfPersistant,
		(pTSCallback_t)&CheckIopImageTable,// pTSCallback_t pCallback,
		NULL								// void* pContext
	);
	
	// Initiate the define table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pDefineTable->Send();

	return status;
}


// CheckIOPImageTable -- Check if IOPImageTable was defined.
//
STATUS DdmBootMasterProxy::CheckIopImageTable(void *pClientContext, STATUS status)
{

	TRACE_ENTRY(DdmBootMasterProxy::CheckIOPImageTable);

	if (status == OK)
	{
		TRACEF( TRACE_L8, ("\n***DdmBootMasterProxy:Found IOPImageTable."));
		// IOPImageTable already exists. Try to read it.
		return ReadIopImageTable(pClientContext, status);
	}
		
	// We need to insert a new IOPImageTable.		
	return CreateIopImageTable(pClientContext, status);

}

	
// ReadIOPImageTable -- Read existing IOPImageTable.
//
STATUS DdmBootMasterProxy::ReadIopImageTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMasterProxy::ReadIOPImageTable);

	TRACEF( TRACE_L8, ("\n***DdmBootMasterProxy: Reading IOPImageTable...\n"));

	// Read the IOPImageTable.	
	TSReadTable*	pReadIOPImageTable = new TSReadTable;
	
	status = pReadIOPImageTable->Initialize(
		this,											// DdmServices pDdmServices,
		(char*)IOPImageRecord::TableName(),					// String64 prgbTableName,
		&m_pIopImageTableRows,							// void* &ppTableDataRet, returned table data.
		&m_nIopImageTableRows,							// U32 *pcRowsRet,			// returned # of rows read
		(pTSCallback_t)&HandleReadIopImageTableReply,	// pTSCallback_t pCallback,
		pClientContext
	);

	if (status == OS_DETAIL_STATUS_SUCCESS)
		pReadIOPImageTable->Send();
		
	return status;
}

// HandleReadIopImageTableReply -- Handle reply of Read IOPImageTable.
//
STATUS DdmBootMasterProxy::HandleReadIopImageTableReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMasterProxy::HandleReadIOPImageTableReply);

	TRACEF( TRACE_L3, ("\n***DdmBootMasterProxy:ReadIOPImageTable returned status 0x%x.\n", status));

	if (status == ercTableExists)
		return UpdateIopImageTable(pClientContext, status);
	else
	if (status == OK)
		return CreateIopImageTable(pClientContext, status); 
	else
		return status;
}

STATUS DdmBootMasterProxy::UpdateIopImageTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)
		
	return status;
}


// Boot Image
// There are three distinct situations that require an image to be booted 
// on a board.  These include
// - Initial boot
// - Upgrade Master requested boot
// - Boot at failure
// The actions taken for each of these situations, as it relates to the
// Upgrade Master and the IOP Image Table maintenance, are outlined below.
//
// Initial Boot
// During the initial boot process, the Boot Manager will take the following
// steps:
// - Requests that CMB populate the IOP Status Table 
// - Define the IOP Image Table - this is persistent, so it may already exist
// - For each slot which the Boot Manager is going to power up
// - If a row doesn't exist for this slot
// - insert and initialize as defined in Section 1
// - Boot image with primaryImage
// - Update currentImage and timeBooted in IOP Image Table
// The primaryImage is retrieved from the image repository using the message
// MsgOpenImage, defined in UpgradeMasterMessages.h.
//
// Upgrade Master Requested Reboot
// The process of upgrading an image, involves:
// - quiescing a board and then
// - rebooting it. The Boot Manager can assume, upon receipt of a request to
// boot an image from the Upgrade Master, that the board is in a QUIESCED
// state. 
// The arguments to the boot request message include an image key and a slot.
// - The image key is used to extract the image from the image repository 
// using the message MsgOpenImage. 
// - Set currentImage to imageKey in IOP Image Table
// - Set timeBooted in IOP Image Table
// - Remit any outstanding "Running Trial Image" alarms for this board
// - Throw an alarm "Running Trial Image"
// - Boot with imageKey
//
// Failover Reboot
// At failure, the Boot Manager will take the following actions:
// - If currentImage is not equal to primaryImage
// - Remit "Running Trial Image" Alarm
// - Reboot with primaryImage
// - Update currentImage and timeBooted in IOP Image Table
// - else
// - According to policy, either power down or reboot with primaryImage
// - Update currentImage and timeBooted in IOP Image Table, either to NULL
//   or to the appropriate value if rebooting
//************************************************************************//
STATUS DdmBootMasterProxy::FinishInitialize()
{
	// We're finished with our initialization.
	return Ddm::Initialize(m_pInitializeMsg);
}



// Enable -- Start-it-up -----------------------------------------------DdmBootMasterProxy-
//
STATUS DdmBootMasterProxy::Enable(Message *pMsg)
{ 
STATUS	status;

	TRACE_ENTRY(DdmBootMasterProxy::Enable);

	m_pEnableMsg = pMsg;
	status = Boot();	
	return status;
}


// Boot -
// This is the entry point for the BootMgr's actual boot operation.
STATUS DdmBootMasterProxy::Boot()
{
	return ListenOnIopStatusTable();
}


// ListenOnIopStatusTable -
// Listen On the IopStatusTable in preparation for bringing up the Iops.
STATUS DdmBootMasterProxy::ListenOnIopStatusTable()
{
STATUS status;

	TRACE_ENTRY(DdmBootMasterProxy::ListenOnIopStatusTable);	

	// Alloc, init and send off a Listen object for the IopStatusTable.
	m_pListenIopStatusTable = new TSListen;
	
	status = m_pListenIopStatusTable->Initialize(
		this,											// DdmServices pDdmServices,
		ListenOnModifyAnyRowAnyField,					// U32 ListenType,
		CT_IOPST_TABLE_NAME,							// String64 rgbTableName,
		NULL,											// String64 prgbRowKeyFieldName,
		NULL,											// void* prgbRowKeyFieldValue,
		0,												// U32 cbRowKeyFieldValue,
		NULL,											// String64 prgbFieldName,
		NULL,											// void* prgbFieldValue,
		0,												// U32 cbFieldValue,
		ReplyContinuous|ReplyFirstWithTable|ReplyWithRow,	// U32 ReplyMode,
		&m_pIopStatusTableRows,							// void** ppTableDataRet,
		&m_cbIopStatusTableRows,						// U32* pcbTableDataRet,
		&m_IopSTListenerId,								// U32* pListenerIDRet,
		&m_pIopSTListenReplyType,						// U32** ppListenTypeRet,
		&m_pIopSTModifiedRecord,						// void** ppModifiedRecordRet,
		&m_cbIopSTModifiedRecord,						// U32* pcbModifiedRecordRet,
		(pTSCallback_t)&HandleIopSTListenReply,			// pTSCallback_t pCallback,
		this
	);
	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		m_pListenIopStatusTable->Send();

	return status;
}


// HandleIopSTListenReply -
// This is the reply handler specified in ListenOnIopStatusTable.  It will
// be called:
// - In response to the initial listen message with the original table.
// - When any IopStatusTable record has been modified.
// - When we stop the listen in preparation for shutdown.
//  In turn HandleIopSTListenReply will set up and maintain IopStateRecord 
// instance data for use by the IopManProxy objects.
STATUS DdmBootMasterProxy::HandleIopSTListenReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMasterProxy::HandleIopSTListenReply);

	TRACEF( TRACE_L3, ("\n***DdmBootMasterProxy:ListenOnIopStatusTable returned status 0x%x.\n", status));

	// Check for reply to the initial listen message with the original table.
	if (*m_pIopSTListenReplyType == ListenInitialReply)
	{
		// Copy The returned IopStatusTable into our local records.
		bcopy ((const char*)m_pIopStatusTableRows, (char*)&m_IopStatusTableRows, m_cbIopStatusTableRows );
		delete m_pIopStatusTableRows;
		m_pIopStatusTableRows = m_IopStatusTableRows;
		m_nIopStatusTableRows = m_cbIopStatusTableRows / sizeof(IOPStatusRecord);
		status = InitSystemStatusRecord();
		return status;
	}
	
	// Check for reply when any IopStatusTable record has been modified.
	if (*m_pIopSTListenReplyType == ListenOnModifyAnyRowAnyField)
	{
		// Find the returned record in our array.
		for (m_iIopSTModifiedRecord = 0; m_iIopSTModifiedRecord < m_nIopStatusTableRows; m_iIopSTModifiedRecord++)
			if (m_pIopStatusTableRows[m_iIopSTModifiedRecord].Slot == m_pIopSTModifiedRecord->Slot)
				break;
				
		if (m_iIopSTModifiedRecord == m_nIopStatusTableRows)
		{
			TRACEF(TRACE_L3,("\n***DdmBootMasterProxy: Invalid IOPStatusTable Listen Reply.  Status = 0x%x.\n", status));
			DumpIopStatusRec( TRACE_L3, m_pIopSTModifiedRecord, "***DdmBootMasterProxy: Invalid" );
		}
		else
		{
			// Update the array with the new data.
			m_IopStatusTableRows[m_iIopSTModifiedRecord] = *m_pIopSTModifiedRecord;
		}
	}
		
	// Check for when we stop the listen in preparation for shutdown.
	if (*m_pIopSTListenReplyType == ListenReturnedOnStopListen)
	{
		TRACEF(TRACE_L3,("\n***DdmBootMasterProxy: Received IOPStatusTable StopListen Reply.  Status = 0x%x.\n", status));

		if (status != OS_DETAIL_STATUS_SUCCESS)
		;	// TODO: Throw some event here.
		
		status = ShutDown();
	}

	return status;
}


// InitSystemStatusRecord -
// Initialize the System Status Record's masks of Iops present and active.
STATUS DdmBootMasterProxy::InitSystemStatusRecord()
{
STATUS status;;

	TRACE_ENTRY(DdmBootMasterProxy::InitSystemStatusRecord);

	TRACEF( TRACE_L8, ("\n***DdmBootMasterProxy: Modifying SystemStatusRecord...\n"));

	// Clear the flags first.
	m_SystemStatusRecord.ActiveIOPsMask = 0;
	m_SystemStatusRecord.PresentIOPsMask = 0;
	m_SystemStatusRecord.IOPsOnPCIMask = 0;
	
	// Reset the System Status Record's masks of Iops present and active.
	m_SystemStatusRecord.ActiveIOPsMask |= (1 << Address::iSlotMe);
	
	// Turn this HBC's bit on in the Iops present mask.
	m_SystemStatusRecord.IOPsOnPCIMask |= (1 << Address::iSlotMe);

	// Turn this HBC's bit on in the Iops present mask.
	m_SystemStatusRecord.PresentIOPsMask |= (1 << Address::iSlotMe);

	// Setup m_IopSTItemToStartNext for use in StartIops.
	int i;
	for (i = 0; i < m_nIopStatusTableRows; i++)
		// If we found a powered down IOP or our own IOPStatusTable record, then...
		if ((m_pIopStatusTableRows[i].eIOPCurrentState == IOPS_POWERED_DOWN) ||
//			(m_aIopManProxys[i])IOP_Type == IOPTY_HBC))
			(m_pIopStatusTableRows[i].Slot == Address::iSlotHbcMaster))
		{
			// Turn the slot's bit on in the Iops present mask.
			m_SystemStatusRecord.PresentIOPsMask |= (1 << m_pIopStatusTableRows[i].Slot);
		}

	TSModifyRow*	pModifySystemStatusRecord = new TSModifyRow;
	
	status = pModifySystemStatusRecord->Initialize(	
		this,												// DdmServices* pDdmServices,
		SYSTEM_STATUS_TABLE,								// String64 rgbTableName,
		CT_PTS_RID_FIELD_NAME,								// String64 rgbKeyFieldName,
		&m_SystemStatusRecord.rid,							// void* pKeyFieldValue,
		sizeof(rowID),										// U32 cbKeyFieldValue,
		&m_SystemStatusRecord,								// void* prgbRowData,
		sizeof(SystemStatusRecord),							// U32 cbRowData,
		1,													// U32 cRowsToModify,
		NULL,												// U32* pcRowsModifiedRet,
		&m_SystemStatusRecord.rid,							// rowID* pRowIDRet,
		sizeof(rowID),										// U32 cbMaxRowID,
		(pTSCallback_t)&HandleWriteSystemStatusTableReply,	// pTSCallback_t pCallback,
		NULL												// void* pContext
	);

	if (status == OS_DETAIL_STATUS_SUCCESS)
		pModifySystemStatusRecord->Send();

	return status;
}


// HandleWriteSystemStatusTableReply -
// This is the reply handler specified in UpdateSystemStatusTable.  It will
// be called when System Status Table has been updated.  In turn it sets up 
// a listen on the SystemStatus Table in preparation for powering up the Iops.
// It will then call StartIops to initiate the powering up of the Iops.
STATUS DdmBootMasterProxy::HandleWriteSystemStatusTableReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMasterProxy::HandleWriteSystemStatusTableReply);
	
	TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy: Modify SystemStatusRecord status=0x%x.\n",status));
	
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		;	// TODO: Throw some event here.
	}

	// Call StartIops to power on and boot the Iops.
	status = InitIopMgrs();
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACEF(TRACE_L3,("\n***DdmBootMasterProxy: ERROR Initing IOP Managers.  Status = 0x%x.\n", status ));
		// TODO: Throw some event here.
	}
	else
		status = StartIops();

	return status;
}


// InitIopMgrs - 
// This method will loop through the IopStatusTable Instantiating IopManProxy
// objects for each Iop.  
STATUS DdmBootMasterProxy::InitIopMgrs()
{
	// Find an Iop to power up.
	for (int i = 0;  i < m_nIopStatusTableRows; i++)
		// If we found a powered down IOP or an HBC IOPStatusTable record, then...
		if ((m_pIopStatusTableRows[i].eIOPCurrentState == IOPS_POWERED_DOWN) ||
//			(m_pIopStatusTableRows[i].IOP_Type == IOPTY_HBC))
			(m_pIopStatusTableRows[i].Slot == Address::iSlotHbcMaster))
		{
			// Instantiate an IopManProxy object to manage the IOP for me.
			m_aIopManProxys[i] = new IopManProxy(this, &(m_pIopStatusTableRows[i]), &(m_pIopImageTableRows[i]));
			if (m_aIopManProxys[i] == NULL)
				return CTS_OUT_OF_MEMORY;
		}
		
	return OK;
}


// StartIops - 
// This method will loop through the IopStatusTable Instantiating IopManProxy
// objects for each Iop.  Each IopManProxy object will then be told to power
// on its Iop and get it on the PCI bus by sending commands to the Iop via
// the Card Management Bus Ddm.
STATUS DdmBootMasterProxy::StartIops()
{
STATUS status;

U32	i = 0;		// Skip the HBCs which will always come first.

	// Find an Iop to power up.
	for (;  i < m_nIopStatusTableRows; i++)
		if ((m_pIopStatusTableRows[i].eIOPCurrentState == IOPS_POWERED_DOWN) &&
			(m_pIopStatusTableRows[i].IOP_Type != IOPTY_HBC))
		{
			status = m_aIopManProxys[i]->StartIop();
			if (status != OS_DETAIL_STATUS_SUCCESS)
			{
				TRACEF(TRACE_L3,("***DdmBootMasterProxy: ERROR Starting IOP in Slot %d.  Status = 0x%x.\n",
				m_pIopStatusTableRows[i].Slot, status));
				// TODO: Throw some event here.
			}
		}
	
	// Clear flag indicating all Iops are up.
	m_fAllIopsOnPciFlag = false;
	
	// Wait for all IOPs to come on to PCI.
	status =  Listen4IopsOnPci(status);

	return status;
}


//  DdmBootMasterProxy::Listen4IopsOnPci(void *pClientContext, STATUS status)
//
//  Description:
//    This method is called to Listen on the IOPsOnPCIMask mask in the 
//	system status table.
//
//  Inputs:
//    pClientContext - unused
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmBootMasterProxy::Listen4IopsOnPci(STATUS status)
{
	TRACE_ENTRY(DdmBootMasterProxy::Listen4IopsOnPci);
	TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy: Listen4IopsOnPCI() status = 0x%x.\n",status));

	m_pListen4Iops = new TSTimedListen;
	if (!m_pListen4Iops)
		status = CTS_OUT_OF_MEMORY;
	else			
		// We are going to listen for the ActiveIopsMask to equal the PresentIopsMask.
		// PresentIopsMask will have a one in bits corresponding to all present Iops.
		status = m_pListen4Iops->Initialize( 
			this,											// DdmServices* pDdmServices
			ListenOnModifyOneRowOneField,					// U32 ListenType
			SYSTEM_STATUS_TABLE,							// String64 prgbTableName
			CT_PTS_RID_FIELD_NAME,							// String64 prgbRowKeyFieldName
			&m_SystemStatusRecord.rid,						// void* prgbRowKeyFieldValue
			sizeof(rowID),									// U32 cbRowKeyFieldValue
			CT_SYSST_IOPSONPCIMASK,							// String64 prgbFieldName
			&m_SystemStatusRecord.PresentIOPsMask,			// void* prgbFieldValue
			sizeof(m_SystemStatusRecord.PresentIOPsMask),	// U32 cbFieldValue
			ReplyOnceOnly | ReplyWithRow,					// U32 ReplyMode
			NULL,											// void** ppTableDataRet,
			NULL,											// U32* pcbTableDataRet,
			&m_ListenerID,									// U32* pListenerIDRet,
			&m_pListenReplyType1,							// U32** ppListenTypeRet,
			&m_pNewSystemStatusRecord,						// void** ppModifiedRecordRet,
			&m_cbNewSystemStatusRecord,						// U32* pcbModifiedRecordRet,
			TSCALLBACK(DdmBootMasterProxy, IopsOnPciListenReply),	// pTSCallback_t pCallback,
			60000000,										// U32 iTimeOut, 60 secs.
			NULL											// void* pContext
		);
	
	if (status == OK)
			m_pListen4Iops->Send();
	else
	{
		;	// TODO:  Throew error here.
	}
	
	return OK;
}


//  DdmBootMasterProxy::IopsOnPciListenReply(void *pClientContext, STATUS status)
//
//  Description:
//    This method is called back by Listen when the System Status Table's
//	IopsOnPciMask comes equal to the PresentIopsMask.
//
//  Inputs:
//    pClientContext - unused.
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmBootMasterProxy::IopsOnPciListenReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMasterProxy::IopsOnPciListenReply);
//	TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy: IopsOnPciListenReply() status = 0x%x.\n",status));

	// If it is the initial reply ignore it.
	if (m_pListenReplyType1 && (*m_pListenReplyType1 == ListenInitialReply))
	{
		TRACEF(TRACE_L3,("\n***BootMgr: Received Initial Listen Reply for IOPsOnPCIMask.  Status = 0x%x.\n",
				status ));
		return status;
	}	

	// If the Timer we set has already gone off then we have already aborted
	// The only thing we have to do is free the reply type buffer returned to us.  
	if (m_pListenReplyType1 && (*m_pListenReplyType1 == ListenReturnedOnStopListen))
	{
		TRACEF( TRACE_L3, ("\n***BootMgr: Received Stop Listen Reply for IOPsOnPCIMask.  Status = 0x%x.\n", status ));
		return OK;		
	};
	
	if (status == CTS_PTS_LISTEN_TIMED_OUT)
	{
		TRACEF(TRACE_L3, ("\n***BootMgr: TIMED OUT Listening on IOPsOnPCIMask.  Status = 0x%x.\n", status));
		//		return Cleanup( CTS_Iop_BOOT_TIMEOUT);
		return 999;
	}
	else
	if (status != OK)
	{
		TRACEF(TRACE_L3,("\n***BootMgr: Received BAD Listen Reply for IOPsOnPCIMask.  Status = 0x%x.\n",
				status ));
			
		;	// TODO:  Throew error here.
		return status;
	}
	else
	{
		TRACEF(TRACE_L3,("\n***BootMgr: Received Listen for IOPsOnPCIMask.  Status = 0x%x.\n", status));
		m_fAllIopsOnPciFlag = TRUE;
		
		// We need to reinit the bridges after powering on and connecting all the IOPs.
//		status = InitBridgeFTree();
//		TRACEF(TRACE_L3,("\n***BootMgr: InitBridgeFTree() returned Status = 0x%x.\n", status ));	
	
		// We need to replace our SystemStatusRecord with the updated one returned by the Listen.
		m_SystemStatusRecord = *m_pNewSystemStatusRecord;

		// Now finish the rest of the boot process.
		status = BootIops();
	}
				
	return status;
}
		

// BootIops - 
// This method will loop through the IopManProxy objects cbooting each IOP.
STATUS DdmBootMasterProxy::BootIops()
{
STATUS status;

	// Find an Iop to power up.  Skip the HBCs which will always come first.
	for (int i = 0;  i < m_nIopStatusTableRows; i++)
		if ((m_pIopStatusTableRows[i].eIOPCurrentState == IOPS_AWAITING_BOOT) &&
			(m_pIopStatusTableRows[i].IOP_Type != IOPTY_HBC))
		{
			status = m_aIopManProxys[i]->BootIop();
			if (status != OS_DETAIL_STATUS_SUCCESS)
			{
				TRACEF(TRACE_L3,("***DdmBootMasterProxy: ERROR Booting IOP in Slot %d.  Status = 0x%x.\n",
				m_pIopStatusTableRows[i].Slot, status));
				// TODO: Throw some event here.
			}
		}

	// Clear flag indicating all Iops are up.
	m_fAllIopsAreUp = false;
	
	if (status == OK)
		// Wait for all IOPs to finish their boot-up process.
		status =  Listen4ActiveIops(status);
	
	return status;
}

// FinishEnable - Return our Enable Message.
STATUS DdmBootMasterProxy::FinishEnable()
{
STATUS status;

// For Environment Ddm
Message *pStartEnvironmentDdmMsg;

	pStartEnvironmentDdmMsg = new Message(ENV_ACTIVATE);
	if (Send(pStartEnvironmentDdmMsg, &DiscardReply))
   	{
     	Tracef("DdmBootMasterProxy::.  Error when activate Environment Ddm.\n");
     	delete pStartEnvironmentDdmMsg;
   	}



	status = Ddm::Enable(m_pEnableMsg);
	if (status != OK)
		TRACEF(TRACE_L3,("\n***BootMgr: ERROR replying to Enable.  Status = 0x%x.\n", status ))		
	return status;
}


//  DdmBootMasterProxy::Listen4ActiveIops(STATUS status)
//
//  Description:
//    This method is called to Listen on the ActiveIops mask in the 
//	system status table.
//
//  Inputs:
//    pClientContext - unused
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmBootMasterProxy::Listen4ActiveIops(STATUS status)
{
	TRACE_ENTRY(DdmBootMasterProxy::Listen4ActiveIops);
	TRACEF(TRACE_L3, ("DdmBootMasterProxy::Listen4ActiveIops() status=0x%x.\n",status));

	m_pListen4Iops = new TSTimedListen;
	if (!m_pListen4Iops)
		status = CTS_OUT_OF_MEMORY;
	else			
		// We are going to listen for the ActiveIopsMask to equal the PresentIopsMask.
		// PresentIopsMask will have a one in bits corresponding to all present Iops.
		status = m_pListen4Iops->Initialize( 
			this,											// DdmServices* pDdmServices
			ListenOnModifyOneRowOneField,					// U32 ListenType
			SYSTEM_STATUS_TABLE,							// String64 prgbTableName
			CT_PTS_RID_FIELD_NAME,							// String64 prgbRowKeyFieldName
			&m_SystemStatusRecord.rid,						// void* prgbRowKeyFieldValue
			sizeof(rowID),									// U32 cbRowKeyFieldValue
			CT_SYSST_ACTIVEIOPSMASK,						// String64 prgbFieldName
			&m_SystemStatusRecord.PresentIOPsMask,			// void* prgbFieldValue
			sizeof(m_SystemStatusRecord.PresentIOPsMask),	// U32 cbFieldValue
			ReplyOnceOnly | ReplyWithRow,					// U32 ReplyMode
			NULL,											// void** ppTableDataRet,
			NULL,											// U32* pcbTableDataRet,
			&m_ListenerID,									// U32* pListenerIDRet,
			&m_pListenReplyType2,							// U32** ppListenTypeRet,
			&m_pNewSystemStatusRecord,						// void** ppModifiedRecordRet,
			&m_cbNewSystemStatusRecord,						// U32* pcbModifiedRecordRet,
			TSCALLBACK(DdmBootMasterProxy, IopsActiveListenReply),	// pTSCallback_t pCallback,
			120000000,										// U32 iTimeOut,
			NULL											// void* pContext
		);
	
	if (status == OK)
			m_pListen4Iops->Send();
	else
	{
		;	// TODO:  Throew error here.
	}
	
	return status;
}


//  DdmBootMasterProxy::IopsActiveListenReply(void *pClientContext, STATUS status)
//
//  Description:
//    This method is called back by Listen when the System Status Table's
//	ActiveIopsMask comes equal to the PresentIopsMask.
//
//  Inputs:
//    pClientContext - unused.
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmBootMasterProxy::IopsActiveListenReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMasterProxy::IopsActiveListenReply);
	TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy::IopsActiveListenReply() returned status = 0x%x. \n",status));

	// Before we do anything check if this is the reply we're waiting for 
	// ie the Iop's state changed to AWAITING_BOOT as opposed to initial
	// reply.
	if (m_pListenReplyType2 && (*m_pListenReplyType2 == ListenInitialReply))
	{
		TRACEF(TRACE_L8,("\n***BootMgr: Received Initial Listen Reply for ActiveIopsMask.  Status = 0x%x.\n",
				status ));
		// Allow the rest of the system to come up.
		status = FinishEnable();
		return status;
	}	

	// If the Timer we set has already gone off then we have already aborted
	// The only thing we have to do is free the reply type buffer returned to us.  
	if (m_pListenReplyType2 && (*m_pListenReplyType2 == ListenReturnedOnStopListen))
	{
		TRACEF(TRACE_L8,("\n***BootMgr: Received Stop Listen Reply for ActiveIopsMask.  Status = 0x%x.\n",
				status ));
		
	};
	
	if (status == CTS_PTS_LISTEN_TIMED_OUT)
	{
		TRACEF(TRACE_L3,("\n***BootMgr: TIMED OUT Listening for ActiveIopsMask.  Status = 0x%x.\n", status ));
//		return Cleanup( CTS_Iop_BOOT_TIMEOUT);
		// Now that we're as present, up, and active as we're gonn be, Start the LED heartbeat. 
		status = StartLEDs();
	}
	else
	if (status != OK)
	{
		TRACEF(TRACE_L3,("\n***BootMgr: Received BAD Listen Reply for ActiveIopsMask.  Status = 0x%x.\n",
				status ));
			
		;	// TODO:  Throew error here.
		return status;
	}
	else
	{
		TRACEF(TRACE_L8,("\n***BootMgr: Received Listen for ActiveIopsMask.  Status = 0x%x.\n", status ));
		m_fAllIopsAreUp = TRUE;
	
		// We need to replace our SystemStatusRecord with the updated one returned by the Listen.
		m_SystemStatusRecord = *m_pNewSystemStatusRecord;
	
		// Now that we're all present, up, and active, Start the LED heartbeat. 
		status = StartLEDs();
	}

	return status;
}
		

// StartLEDs - 
// This method will start a timer for the LED blinking heartbeat to the Iops.
STATUS DdmBootMasterProxy::StartLEDs()
{
STATUS status = OK;

#if false
	// Allocate a message we'll send to the timer service to start a timer.
	// Initialize the payload structure for a repeating timer message with 
	// a 1 second delay.
	m_pStartTimerMsg = new RqOsTimerStart(1000000,1000000);
	if (!m_pStartTimerMsg)
		status = CTS_OUT_OF_MEMORY;
	else
		// Send the message off to the timer service.
		status = Send(m_pStartTimerMsg, NULL, REPLYCALLBACK(DdmBootMasterProxy, BlinkLEDs));
#else
		// Allocate timer for LEDs
		m_pStartTimerMsg = new TimerStatic(DdmBootMasterProxy::BlinkLEDs, this);
		
		// Start the new LED timer.
		m_pStartTimerMsg->Enable(1000000, 1000000);


#endif

	return status;
}


// BlinkLEDs - 
// This method will loop through the IopStatusTable issueing BlinkLED Msgs
// to the Iops.
#if false

STATUS DdmBootMasterProxy::BlinkLEDs(Message* pMsg)
{
STATUS status = pMsg ? pMsg->DetailedStatusCode : OK;

	// Delete the timer reply message.
	delete pMsg;
	
	if (status != OK)
		TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy::BlinkLEDs(). Status = 0x%x.\n",status));

	// Find an Iop to ping.
	for (int i = 0; i < m_nIopStatusTableRows; i++)
		if ((pBootMgr->m_pIopStatusTableRows[i].eIOPCurrentState == IOPS_OPERATING))
			if (m_aIopManProxys[i])
			{
				status = m_aIopManProxys[i]->PingIop();
	
				if (status != OS_DETAIL_STATUS_SUCCESS)
				{
					TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy::BlinkLEDs() Error Pinging slot %d.  Status=0x%x.\n",m_pIopStatusTableRows[i].Slot, status));
					;	// TODO: Throw some event here.
				}
			}
	return OK;
}
#else

void DdmBootMasterProxy::BlinkLEDs(void *pContext)
{
DdmBootMasterProxy*	pBootMgr = (DdmBootMasterProxy*)pContext;
STATUS status;

//	TRACEF(TRACE_L8, ("\n***DdmBootMasterProxy::BlinkLEDs().\n"));

	// Find an Iop to ping.
	for (int i = 0; i < pBootMgr->m_nIopStatusTableRows; i++)

		if (pBootMgr->m_pIopStatusTableRows[i].eIOPCurrentState == IOPS_OPERATING)
			#if false
			if ((pBootMgr->m_SystemStatusRecord.PresentIOPsMask & (1 << pBootMgr->m_pIopStatusTableRows[i].Slot)) &&
			 	(pBootMgr->m_SystemStatusRecord.ActiveIOPsMask & (1 << pBootMgr->m_pIopStatusTableRows[i].Slot)))
			#endif
				if (pBootMgr->m_aIopManProxys[i])
				{
					status = pBootMgr->m_aIopManProxys[i]->PingIop();
					if (status != OS_DETAIL_STATUS_SUCCESS && status != OS_DETAIL_STATUS_TIMEOUT)
					{
						TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy::BlinkLEDs() Error Pinging slot %d.  Status=0x%x.\n",pBootMgr->m_pIopStatusTableRows[i].Slot, status));
					;	// TODO: Throw some event here.
					}
				}
}
#endif


// ShutDown() - 
STATUS DdmBootMasterProxy::ShutDown()
{
	if (m_pStartTimerMsg)
		/* Disable the timer */
		m_pStartTimerMsg->Disable();

	return OK;
}



// HandleReqIopOutOfService - Handle a recived message requesting an IOP
// be taken out of service.
// Tell all other IOPs not to Transport to the IOP going out of service
// and then send ourselves a msg to tell the IopManProxy to take the IOP
// down.  The IopManProxy will reply to HandleReqIopOutOfServiceIntReply
// when the IOP is offline  at wqhich point we will tell the other IOPs
// to retry any outstanding msgs for the downed IOP with it's failover. 
STATUS DdmBootMasterProxy::HandleReqIopOutOfService(Message *pMsg)
{
TRACE_ENTRY(DdmBootMasterProxy::HandleReqIopOutOfService);
STATUS				status = pMsg->DetailedStatusCode;

	if (status != OK)
	{
		TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy::HandleReqIopOutOfService() received a message with status 0x%x.\n",status));
		return status;
	}
	else
	if (m_pMsgIopOutOfSvc != NULL)
	{
		status = -1;//CTS_ERROR_TRY_AGAIN_LATER;
		TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy::HandleReqIopOutOfService(): One at a time please.\n",status));
		return status;
	}
	
	m_pMsgIopOutOfSvc = (MsgIopOutOfService*)pMsg;
	TySlot				slot = m_pMsgIopOutOfSvc->GetSlot();

	// I'm thinking that the IOP Manager can't be in the middle of any
	// state sequence or else it may undo what we're gonna ask it to do 
	// in the shutdown sequence.  Sooo...
	
	// Disallow the Out Of Service operation from any state except...
	// IOPS_OPERATIING
	if (m_pIopStatusTableRows[slot].eIOPCurrentState != IOPS_OPERATING)
		Reply(pMsg, CTS_IOP_OOS_STATE_NONO);
	
	// Setup a count of quiesced transports.
	m_iSlotCount = 0;

	// Send other IOP's Transport Servicse a msg to stop Transporting to
	// the IOP going out of service and keep track of how many we sent.  
	// As replies are handled in StopTransportsToIopReply we'll decrement
	// the count and when it's zero all IOPs have stopped Transport to the
	// IOP going out of service.  Then and only then will we take the IOP
	// out of service.
	int i;
	for (i = 0; i < m_nIopStatusTableRows; i++)
		if ((i != slot) &&
			(m_pIopStatusTableRows[i].eIOPCurrentState == IOPS_OPERATING))
		{
			m_iSlotCount++;
			RqOsTransportIopStop* prqStopTransport = new RqOsTransportIopStop(slot);
		    Send(prqStopTransport, REPLYCALLBACK(DdmBootMasterProxy, StopTransportsToIopReply));
		}
	
	return status;	
};


//	STATUS DdmBootMasterProxy::StopTransportsToIopReply(Message* pMsg);
// 
// Handle replies from Transport services on IOPs to our msg telling
// them to stop Transport to the IOP going out of service.
STATUS DdmBootMasterProxy::StopTransportsToIopReply(Message* pMsg)
{
TRACE_ENTRY(DdmBootMasterProxy::StopTransportsToIopReply);
STATUS	status;

	if (pMsg)
	{
		status = pMsg->Status();
		if (status != OK)
		{
			TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy:StopTransportsToIopReply received the following message with status = 0x%x.",status));
			pMsg->Dump("Message:");
		}  
		delete pMsg;
	}	
	else
		status = OK;
	
	// Decrement the count of outstanding Transport Stop Msgs.  When it's
	// zero all IOPs have stopped Transport to the IOP going out of service.
	// Then and only then can we actually take the IOP out of service.
	if (m_iSlotCount == 0)
	{
		TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy:StopTransportsToIopReply received the following unexpected reply:",status));
		pMsg->Dump("Message:");
	}
	else
	if (--m_iSlotCount == 0)
	{
		// Send ourselves a msg to take the IOP out of service.  We do this 
		// so that we get a reply when the operation is complete.  If we just
		// invoke the IopManProxy directly we would never get the reply.  
		MsgIopOutOfServiceInt* pMsgIopOutOfServiceInt = 
	    	new MsgIopOutOfServiceInt(m_pMsgIopOutOfSvc->GetSlot());
	    Send(pMsgIopOutOfServiceInt, REPLYCALLBACK(DdmBootMasterProxy, HandleReqIopOutOfServiceIntReply));
	}
	return status;
}


// STATUS DdmBootMasterProxy::HandleReqIopOutOfServiceInt(Message *pMsg)
// Our own internal method to take an IOP out of service.
STATUS DdmBootMasterProxy::HandleReqIopOutOfServiceInt(Message *pMsg)
{
TRACE_ENTRY(DdmBootMasterProxy::HandleReqIopOutOfService);
STATUS				status = pMsg->DetailedStatusCode;

	if (status != OK)
		TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy::HandleReqIopOutOfService() received a message with status 0x%x.\n",status));
		return status;

	MsgIopOutOfServiceInt*	pRqIopOOSInt = (MsgIopOutOfServiceInt*)pMsg;
	TySlot					slot = pRqIopOOSInt->GetSlot();

	// Disallow the Out Of Service operation from any state except...
	// IOPS_OPERATIING
	if (m_pIopStatusTableRows[slot].eIOPCurrentState == IOPS_OPERATING)
		Reply(pMsg, CTS_IOP_OOS_STATE_NONO);
	
	// Disable the IOPs LED Ping
//	status = m_aIopManProxys[i]->DisablePingIop();
	// Unclear such a task is necessary.

	// Start Out Of Service state sequence processing by the IopManProxy.
	status = m_aIopManProxys[slot]->HandleReqIopOutOfService(pRqIopOOSInt);

	return status;	
};

	
// STATUS DdmBootMasterProxy::HandleReqIopOutOfServiceIntReply(Message *pMsg)
// Our reply method called back after our own internal message to take an
// IOP out of service has completed.
// This method will restart the Transports previously quiesced on other
// IOPs 
STATUS DdmBootMasterProxy::HandleReqIopOutOfServiceIntReply(Message *pMsg)
{
TRACE_ENTRY(DdmBootMasterProxy::HandleReqIopOutOfService);
STATUS				status = pMsg->DetailedStatusCode;

	if (status != OK)
		TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy::HandleReqIopOutOfServiceIntReply() received a message with status 0x%x.\n",status));
		return status;

	MsgIopOutOfServiceInt*	pRqIopOOSInt = (MsgIopOutOfServiceInt*)pMsg;
	TySlot					slot = pRqIopOOSInt->GetSlot();

	// Disallow the Out Of Service operation from any state except...
	// IOPS_OPERATIING
	if (m_pIopStatusTableRows[slot].eIOPCurrentState == IOPS_OPERATING)
		Reply(pMsg, CTS_IOP_OOS_STATE_NONO);
	
	// Disable the IOPs LED Ping
//	status = m_aIopManProxys[i]->DisablePingIop();
	// Unclear such a task is necessary.

	// Send other IOP's Transport Servicse a msg to stop Transporting to
	// the IOP going out of service and keep track of how many we sent.  
	// As replies are handled in StopTransportsToIopReply we'll decrement
	// the count and when it's zero all IOPs have stopped Transport to the
	// IOP going out of service.  Then and only then will we take the IOP
	// out of service.
	int i;
	for (i = 0; i < m_nIopStatusTableRows; i++)
		if ((i != slot) &&
			(m_pIopStatusTableRows[i].eIOPCurrentState == IOPS_OPERATING))
		{
			m_iSlotCount++;
			RqOsTransportIopStart* prqStartTransport = 
		    	new RqOsTransportIopStart(pRqIopOOSInt->GetSlot());
		    Send(prqStartTransport, REPLYCALLBACK(DdmBootMasterProxy, StartTransportsToIopReply));
		}
	

	return status;	
};


//	STATUS DdmBootMasterProxy::StartTransportsToIopReply(Message* pMsg);
// 
// Handle replies from Transport services on IOPs to our msg telling
// them to retry Transports to the IOP gone out of service's failover.
STATUS DdmBootMasterProxy::StartTransportsToIopReply(Message* pMsg)
{
TRACE_ENTRY(DdmBootMasterProxy::StartTransportsToIopReply);
STATUS	status;

	if (pMsg)
	{
		status = pMsg->Status();
		if (status != OK)
		{
			TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy:StartTransportsToIopReply received the following message with status = 0x%x.",status));
			pMsg->Dump("Message:");
		}  
		delete pMsg;
	}	
	else
		status = OK;
	
	// Decrement the count of outstanding Transport Start Msgs.  When it's
	// zero all IOPs have retried Transports to the IOP going out of service.
	// Then and only then can we reply to the IOP out of service message.
	if (m_iSlotCount == 0)
	{
		TRACEF(TRACE_L3, ("\n***DdmBootMasterProxy:StartTransportsToIopReply received the following unexpected reply:",status));
		pMsg->Dump("Message:");
	}
	else
	if (--m_iSlotCount == 0)
	{
		// Finally,we can reply to the original message.  
		Reply( m_pMsgIopOutOfSvc, OK );
		m_pMsgIopOutOfSvc = NULL;
	}
	return status;
}


	
// $Log: /Gemini/Odyssey/Oos/DdmBootMasterProxy.cpp $
// 
// 1     2/15/00 6:13p Tnelson
// Proxy services for testing
// 
// 44    12/14/99 4:40p Vnguyen
// Add call to activate Environment Ddm in FinishEnable routine.
// 
// 43    12/09/99 1:50a Iowa
// 
// 42    11/22/99 5:32p Jlane
// Add code to inform all Transport svcs of an IOP going out of service.
// 
// 41    11/21/99 11:38p Jlane
// more of the same.
// 
// 40    11/21/99 9:52p Jlane
// Don't enable until IOPs are in a known state.
// 
// 39    11/21/99 8:11p Jlane
// Fix TRACE_INDEX hack.
// 
// 38    11/21/99 7:33p Jlane
// Can't ServeVirtual as SystemEntry...Hmmm Tom!?!
// 
// 37    11/21/99 4:54p Jlane
// IopOutOfService support and initial checkin of ImageDescTable, and
// Default/Iop-ImageTable handling (mostly commented out).
// 
// 36    11/15/99 2:27p Jlane
// Ignore other HBC for now.
// 
// 35    11/11/99 3:40p Jlane
// Comment out INitBridgeFTree for now it breaks the net.
// 
// 34    10/30/99 4:09p Sgavarre
// Initialize Status.
// 
// 33    10/28/99 9:55a Jlane
// Major changes to support Persistance and Initializing the bridges after
// IOPs are on the PCI.  
// 
// 32    10/14/99 4:48p Jlane
// Multiple fixes to get LED Blinking working.
// 
// 31    10/05/99 1:59p Jlane
// comment out timer start.
// 
// 29    9/01/99 8:05p Iowa
// 
// 28    8/31/99 11:41a Jlane
// Don't delete m_pReadIopStatusTable in its own callback.  BAD BAD
// 
// 27    8/30/99 12:19p Jlane
// Make sure IopStatusTable is updated before Enable is completed.
// 
// 25    8/27/99 8:55a Cwohlforth
// Temporarily comment out VirtualStateTable stuff.
// 
// 24    8/26/99 8:48a Jlane
// Remove CMBUpdEvcStatusTable message stuff.
// 
// 23    8/25/99 7:05p Jlane
// Multiple changes for blinking LEDs.
// 
// 22    8/24/99 10:02a Jlane
// Multiple changes to support blinking LEDs and confirmation of Iops
// coming active.
// 
// 21    8/06/99 5:53p Jlane
// Readd FinishEnable call to Enable so our enable message gets replied
// to.
// 
// 20    7/24/99 5:41p Jlane
// reslove PTS changes
// 
// 19    7/20/99 6:46p Rkondapalli
// E2 CMB Boot changes: Move Init_Iop into IopManProxy.
// 
// 18    7/09/99 11:19a Jlane
// Remove slot ping during initialization.
// 
// 17    6/30/99 3:01p Jlane
// [trn/ewx] Changed OS "ping" message to official class name (still comes
// from wrong header, this will change later).
// 
// 16    6/29/99 4:12p Jlane
// Added the FinishEnable method and pinged the virtual manager to make
// sure he's alive before we begin our work.
// 
// 15    6/28/99 12:40p Jlane
// Don't return in loop starting IopManProxys.
// 
// 14    6/21/99 1:53p Ewedel
// Various changes to straighten out (I hope!) usage of member pointers to
// various message types.  Also, code now carefully NULLs such pointers
// after their respective messages have been deleted.
// 
// 13    6/21/99 1:29p Rkondapalli
// Changed "update Iop status" CMB DDM message to correct "poll all Iops."
// [jl/ew]
// Disabled init_hbc() call in DdmBootMasterProxy::StartIops(). [jl?]
// 
// 12    6/16/99 5:46p Jlane
// Enhanced to work with IopManProxy class for CMB Boot.
// 
// 11    6/15/99 9:39a Cwohlforth
// Change Iop state in line 241 for new states per CMB.  Jerry will need
// to merge this change with current work.
// 
// 10    5/13/99 12:44p Cwohlforth
// Field name change to   eIopCurrentState
// 
// 9     5/13/99 11:36a Cwohlforth
// Edits to support conversion to TRACEF
// 
// 8     4/21/99 9:53a Jlane
// Fix file duplication.
// 
// 7     4/21/99 9:48a Jlane
// Rewrote the loop to start Iops.
//
// Revision History:
//     02/01/99 JFL	Created.
//
