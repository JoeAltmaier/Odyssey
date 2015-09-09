// DdmBootMgr.cpp -- The Boot Process Mgr DDM.
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

#include <String.h>
#include "OsTypes.h"
#include "Odyssey_Trace.h"
#include "DdmBootMgr.h"
#include "BootMgrCmds.h"
#include "BootMgrMsgs.h"
#include "IopManager.h"
#include "DdmCmbMsgs.h"
#include "IopManager.h"
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
CLASSNAME(DdmBootMgr, SINGLE);	// Class Link Name used by Buildsys.cpp

// Serve The IOP Out Of Service Request. 
SERVELOCAL(DdmBootMgr,REQ_IOP_OUT_OF_SERVICE);
SERVELOCAL(DdmBootMgr,REQ_IOP_OUT_OF_SERVICE_INT);	// internal msg to take Iop OffLine 
SERVELOCAL(DdmBootMgr,REQ_IOP_POWER_ON);
SERVELOCAL(DdmBootMgr,REQ_IOP_INTO_SERVICE);
SERVELOCAL(DdmBootMgr,REQ_IOP_INTO_SERVICE_INT);	// internal msg to put Iop OnLine 
SERVELOCAL(DdmBootMgr,REQ_IOP_LOCK_UNLOCK_INT);	// internal msg to lock an Iop

extern "C" STATUS InitBridgeFTree();	// Defined in pcimap.c
extern "C" int image_hdr( U32 block, void** pImgAdrRet, U32* pImgSizRet);	// defined in imgmgr.c


// DdmBootMgr -- Constructor -------------------------------------------DdmBootMgr-
//
DdmBootMgr::DdmBootMgr(DID did): Ddm(did) {
	TRACE_ENTRY(DdmBootMgr::DdmBootMgr);
	
	SetConfigAddress(NULL,0);	// tell Ddm:: where my config area is
	m_ImgSiz = 0;
	m_pImgHdr = NULL;
	m_pMsgIopOutOfSvc = NULL;
	m_pCmdIopOutOfSvc = NULL;
	m_pHandleIopOutOfSvc = NULL;
	m_pMsgIopPowerOn = NULL;
	m_pCmdIopPowerOn = NULL;
	m_pHandleIopPowerOn = NULL;
	m_pMsgIopIntoSvc = NULL;
	m_pCmdIopIntoSvc = NULL;
	m_pHandleIopIntoSvc = NULL;
	m_pCmdIopLockUnlock = NULL;
	m_pHandleIopLockUnlock = NULL;
	m_iSlotCount = 0;
	m_nDefaultImageTableRows = 0;
	m_fHaveDefaultImageType = false;
	m_iROMImgBlk = 0;
	m_fAllIopsAreUp = false;
	m_fAllIopsOnPciFlag = false;
	m_iIopImage2Init = 0;
	m_iDefaultImageTable = 0;
	
	DispatchRequest(REQ_IOP_OUT_OF_SERVICE,
					REQUESTCALLBACK(DdmBootMgr, ProcessBootMgrMessages));
	DispatchRequest(REQ_IOP_OUT_OF_SERVICE_INT,
					REQUESTCALLBACK(DdmBootMgr, HandleReqIopOutOfServiceInt));
	DispatchRequest(REQ_IOP_POWER_ON,
					REQUESTCALLBACK(DdmBootMgr, ProcessBootMgrMessages));
	DispatchRequest(REQ_IOP_INTO_SERVICE,
					REQUESTCALLBACK(DdmBootMgr, ProcessBootMgrMessages));
	DispatchRequest(REQ_IOP_INTO_SERVICE_INT,
					REQUESTCALLBACK(DdmBootMgr, HandleReqIopIntoServiceInt));
	DispatchRequest(REQ_IOP_LOCK_UNLOCK_INT,
					REQUESTCALLBACK(DdmBootMgr, HandleReqIopLockUnlockInt));
					
	memset( &m_aIopManagers, 0, sizeof(m_aIopManagers) );
	memset( &m_IopStatusTableRows, 0, sizeof(m_IopStatusTableRows) );
}
	

// Ctor -- Create ourselves --------------------------------------------DdmBootMgr-
//
Ddm *DdmBootMgr::Ctor(DID did)
{
	return new DdmBootMgr(did);
}


// Initialize -- Do post-construction initialization -------------------DdmBootMgr-
//
STATUS DdmBootMgr::Initialize(Message *pMsg)
{ 
STATUS	status;

	TRACE_ENTRY(DdmBootMgr::Initialize);
	m_pInitializeMsg = pMsg;
	
	// create command processing queue (allows only one message to be
	// processed at one.
	m_processingCommand = FALSE;
	m_pBootMgrQueue = new CommandProcessingQueue;
	assert(m_pBootMgrQueue);
	
	status = DefineCommandServer();
	return status;
}

#pragma mark ### Commmand Queue Initialization ###

//  DdmBootMgr::DefineCommandServer()
//
//  Description:
//	  Defines the command server to serve SSAPI requests
//
//  Inputs:
//
//  Outputs:
//	  status - returns OK 
//
STATUS DdmBootMgr::DefineCommandServer()
{
	TRACE_ENTRY(DdmBootMgr::DefineCommandServer);

	m_pCmdServer = new CmdServer(
		BMGR_CONTROL_QUEUE,
		BMGR_CONTROL_COMMAND_SIZE,
		BMGR_CONTROL_STATUS_SIZE,
		this, 
		(pCmdCallback_t)&DdmBootMgr::ListenerForCommands);

	m_pCmdServer->csrvInitialize(
		INITIALIZECALLBACK(DdmBootMgr,DefineSystemStatusTable));
		
	return OK;
}

#pragma mark ### SystemStatusTable Initialization ###

//  DdmBootMgr::DefineSystemStatusTable()
//
//  Description:
//	  Insure SystemStatusTable exists. 
//
//  Inputs:
//	  status - return status from command server initialization
//
//  Outputs:
//
void DdmBootMgr::DefineSystemStatusTable(STATUS status)
{ 

	TRACE_ENTRY(DdmBootMgr::DefineSystemStatusTable);
	
	if (status!=OK)
		TRACEF(TRACE_L8, ("\n***DdmBootMgr: command server initialization failed.\n."));

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
}

//  DdmBootMgr::CheckSystemStatusTable()
//
//  Description:
//	  Check if SystemStatusTable was defined. 
//
//  Inputs:
//	  status - return status from PTS define table
//
//  Outputs:
//	  status - returns OK or highly descriptive error code
//
STATUS DdmBootMgr::CheckSystemStatusTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::CheckSystemStatusTable);

	if (status == ercTableExists)
	{
		TRACEF( TRACE_L8, ("\n***DdmBootMgr: Found SystemStatusTable.\n"));
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
		TRACEF(TRACE_L3,("\n***DdmBootMgr: DefineSystemStatusTable = 0x%x.\n", status));
		// Throw an error here!
	}
	return status;
}
	
//  DdmBootMgr::ReadSystemStatusRec()
//
//  Description:
//	  Read existing SystemStatusRecord.
//
//  Inputs:
//	  status - 
//
//  Outputs:
//	  status - returns status from PTS Read Row initialize
//
STATUS DdmBootMgr::ReadSystemStatusRec(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)
	
	TRACE_ENTRY(DdmBootMgr::ReadSystemStatusRec);

	TRACEF( TRACE_L8, ("\n***DdmBootMgr: Reading SystemStatusRec...\n"));

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

//  DdmBootMgr::CreateSystemStatusRec()
//
//  Description:
//	  Create and insert a new SystemStatusRecord.
//
//  Inputs:
//	  status -
//
//  Outputs:
//	  status - returns status from PTS Insert Row initialize
//
STATUS DdmBootMgr::CreateSystemStatusRec(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::CreateSystemStatusRec);
	
	TRACEF( TRACE_L8, ("\n***DdmBootMgr: Inserting new SystemStatusRec.\n"));
	
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


//  DdmBootMgr::CheckSystemStatusRec()
//
//  Description:
//	  Check that SystemStatusRecord is OK.
//
//  Inputs:
//	  status - returned status from PTS operation
//
//  Outputs:
//	  status - returns status from PTS Insert Row initialize
//
STATUS DdmBootMgr::CheckSystemStatusRec(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::CheckSystemStatusRec);

	if (status != OK)
	{
		TRACEF( TRACE_L3, ("\n***DdmBootMgr: ERROR with SystemStatusRec: 0x%x.\n",status));
		return	CreateSystemStatusRec(pClientContext, status);
	}
	else
		TRACEF( TRACE_L8, ("\n***DdmBootMgr: SystemStatusRec OK!\n"));
	
//	status = HaveCMBUpdateEVCStatusRecord(pClientContext, status);
	status = ReadEVCStatusRecord(pClientContext, status);
	return status;
}


#pragma mark ### EVCStatusRecord Initialization ###

/*// HaveCMBUpdateEVCStatusRecord - 
// This method sends a message to the CMB Ddm to have it populate the
// EVCStatusRecord.  
STATUS DdmBootMgr::HaveCMBUpdateEVCStatusRecord(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::HaveCMBUpdateEVCStatusRecord);

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
}*/


//  DdmBootMgr::ReadEVCStatusRecord()
//
//  Description:
//	  Read existing EVCStatusRecord.
//
//  Inputs:
//	  status - 
//
//  Outputs:
//	  status - returns status from PTS Read Row initialize
//
STATUS DdmBootMgr::ReadEVCStatusRecord(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::ReadEVCStatusRecord);

	TRACEF( TRACE_L8, ("\n***DdmBootMgr:Reading SystemStatusTable...\n"));

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
 
//  DdmBootMgr::CheckEVCStatusRecord()
//
//  Description:
//	  This method verifies the EVCStatusRecord is OK to proceed booting.  
//
//  Inputs:
//	  status - returned status from PTS Read Row operation
//
//  Outputs:
//	  status - returns status from ReadDefaultImageTable
//
STATUS DdmBootMgr::CheckEVCStatusRecord(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::CheckEVCStatusRecord);

	// Do checking of EVC data here.
	
	// status of EVC Data? 
	if (status == OK)
		TRACEF( TRACE_L3, ("\n***DdmBootMgr: EVCStatusRec OK!\n"))
	else
		TRACEF( TRACE_L3, ("\n***DdmBootMgr: ERROR with EVCStatusRec:0x%x.\n",status));
	
	return ReadDefaultImageTable(pClientContext, status);
}


#pragma mark ### DefaultImageTable Initialization ###

//  DdmBootMgr::ReadDefaultImageTable()
//
//  Description:
//	  Read existing DefaultImageTable.
//
//  Inputs:
//	  status - 
//
//  Outputs:
//	  status - returns status from PTS Read Table initialize
//
#ifdef JFLsWAY
STATUS DdmBootMgr::ReadDefaultImageTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::ReadDefaultImageTable);
	
	TRACEF( TRACE_L8, ("\n***DdmBootMgr: Reading DefaultImageTable...\n"));

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
STATUS DdmBootMgr::ReadDefaultImageTable(Message* pMsg)
{
TRACE_ENTRY(DdmBootMgr::ReadDefaultImageTable);
STATUS	status = pMsg->GetStatus();

	if (status != OK)
	{
		TRACEF(TRACE_L3, ("\n***DdmBootMgr:ReadDefaultImageTable received the following message with status = 0x%x.",status);
		pMsg->Dump("Message:");
	}  
	delete pMsg;
		
	// Allocate and send a Read Table msg for the Image Descriptor Table.
	DefaultImageRecord::RqEnumTable* pRqPtsReadTable = new DefaultImageRecord::RqEnumTable;
	status = Send(	pRqPtsReadTable,
					REPLYCALLBACK(DdmBootMgr, CheckDefaultImageTable)
				 );
	return status;
}
#endif


//  DdmBootMgr::CheckDefaultImageTable()
//
//  Description:
//	  Check if DefaultImageTable needs to be defined.
//
//  Inputs:
//	  status - returned status from PTS Read Table operation
//
//  Outputs:
//	  status - returns OK or a highly descriptive error code
//
#ifdef JFLsWAY
STATUS DdmBootMgr::CheckDefaultImageTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::CheckDefaultImageTable);

	if (status == ercTableNotfound)
	{
		// Default Image Table Doesn't exist define it.
		status = DefineDefaultImageTable(pClientContext, status);
	}
	else
	if (status == OK)
	{
		TRACEF( TRACE_L8, ("\n***DdmBootMgr: Found DefaultImageTable.\n"));
		// ImageDescTable already exists. Try to update it.
		m_iROMImgBlk = 0;
		m_iDefaultImageTable = 0;
		status = InitDefaultImageTable(pClientContext, status);
	}
	else
	{
		TRACEF(TRACE_L3,("\n***DdmBootMgr: ReadDefaultImageTable returned status = 0x%x.\n", status));
		// Throw an error here!
	}
	return status;
}
#else
STATUS DdmBootMgr::CheckDefaultImageTable(Message* pMsg)
{
TRACE_ENTRY(DdmBootMgr::CheckDefaultImageTable);
STATUS	status;

	if (pMsg)
	{
		status = pMsg->GetStatus()
		if (status != OK)
		{
			TRACEF(TRACE_L3, ("\n***DdmBootMgr:CheckDefaultImageTable received the following message with status = 0x%x.",status);
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
		TRACEF( TRACE_L8, ("\n***DdmBootMgr: Found ImageDescTable.\n"));
		// ImageDescTable already exists. Try to update it.
		m_iROMImgBlk = 0;
		status = InitDefaultImageTable(pMsg);
	}
	else
	{
		TRACEF(TRACE_L3,("\n***DdmBootMgr: ReadDefaultImageTable returned status = 0x%x.\n", status));
		// Throw an error here!
	}
	return status;
}
#endif

//  DdmBootMgr::DefineDefaultImageTable()
//
//  Description:
//	  Define the DefaultImageTable.
//
//  Inputs:
//	  status - 
//
//  Outputs:
//	  status - returns error code from PTS Define Table initialize
//
#ifdef JFLsWAY
STATUS DdmBootMgr::DefineDefaultImageTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)
TRACE_ENTRY(DdmBootMgr::DefineDefaultImageTable)
	
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
STATUS DdmBootMgr::DefineDefaultImageTable(Message* pMsg)
{
TRACE_ENTRY(DdmBootMgr::DefineDefaultImageTable);
STATUS	status;

	if (pMsg)
	{
		status = pMsg->GetStatus()
		if (status != OK)
		{
			TRACEF(TRACE_L3, ("\n***DdmBootMgr:DefineDefaultImageTable received the following message with status = 0x%x.",status);
			pMsg->Dump("Message:");
		}  
		delete pMsg;
	}	
	else
		status = OK;

	// Allocate and send a Define Table msg for the Default Image Table.
	DefaultImageRecord::RqDefineTable* pRqPtsDefineTable
		= new DefaultImageRecord::RqDefineTable(Persistant_PT, 4);
	status = Send(	pRqPtsDefineTable,
					REPLYCALLBACK(DdmBootMgr, ReadDefaultImageTable)
				 );
	return status;
}
#endif

//  DdmBootMgr::CompareVersions()
//
//  Description:
//	  Compares the timestamps of two versions of images.
//
//  Inputs:
//	  pROMImgHdr - image found in ROM
//	  pDefaultImageRec - current default image for this type of IOP
//
//  Outputs:
//	  return 0 if RomImgHdr version equals ImageDescriptorRecord version
//	  return 1 if RomImgHdr version is more recent than ImageDescriptorRecord version
//	  return -1 if RomImgHdr version is older than ImageDescriptorRecord version
//
int DdmBootMgr::CompareVersions( img_hdr_t* pROMImgHdr, DefaultImageRecord* pDefaultImageRec)
{

// HACK: we dont't want this image it's the wrong type.
// let's say it's older in hopes the user won't take it.
if (pROMImgHdr->i_type != pDefaultImageRec->type)
	return -1;
	
if (pROMImgHdr->i_year > pDefaultImageRec->year)
	return 1;	// ROM image yr > our image year ie ROM image is newer.
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
	return -1;	// ROM image hour > our image hour: ROM image is newer.
if (pROMImgHdr->i_hour < pDefaultImageRec->hour)
	return 1;	// ROM image hour < our image hour: ROM image is older.
if (pROMImgHdr->i_min < pDefaultImageRec->minute)
	return -1;	// ROM image minute > our image minute: ROM image is newer.
if (pROMImgHdr->i_min < pDefaultImageRec->minute)
	return 1;	// ROM image minute < our image minute: ROM image is older.
if (pROMImgHdr->i_sec < pDefaultImageRec->second)
	return -1;	// ROM image second > our image seoncd: ROM image is newer.
if (pROMImgHdr->i_sec < pDefaultImageRec->second)
	return 1;	// ROM image second < our image second: ROM image is older.
return 0;	// image timestamps are equal.
}

// DisplayImageHdr - Display interesting info from an Image header (img_hdr_t in imghdr.h)
void DdmBootMgr::DisplayImageHdr( img_hdr_t* pROMImgHdr, const char* const pString )
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
void DdmBootMgr::DisplayImageDesc( ImageDescRecord* pImgDescRec, const char* const pString )
{
TRACEF(TRACE_ALL_LVL, ("\n***DdmBootMgr: %s ImageDescRecord data:\n", pString));
TRACEF(TRACE_ALL_LVL, ("	Timestamp: %d:%d:%d\n", 
					pImgDescRec->hour, pImgDescRec->minute, pImgDescRec->second));
TRACEF(TRACE_ALL_LVL, ("	Datestamp: %d-%d-%d\n",
					pImgDescRec->month, pImgDescRec->day, pImgDescRec->year));
TRACEF(TRACE_ALL_LVL, ("	Version: %d.%d.%d\n", 
					pImgDescRec->majorVersion, pImgDescRec->minorVersion, 0 ));
}

// DisplayDefImage - Display interesting info from an DefaultImageRecord.
void DdmBootMgr::DisplayDefaultImage( DefaultImageRecord* pDefaultImageRec, const char* const pString )
{
TRACEF(TRACE_ALL_LVL, ("\n***DdmBootMgr: %s ImageDescRecord data:\n", pString));
TRACEF(TRACE_ALL_LVL, ("	Timestamp: %d:%d:%d\n", 
					pDefaultImageRec->hour, pDefaultImageRec->minute, pDefaultImageRec->second));
TRACEF(TRACE_ALL_LVL, ("	Datestamp: %d-%d-%d\n",
					pDefaultImageRec->month, pDefaultImageRec->day, pDefaultImageRec->year));
TRACEF(TRACE_ALL_LVL, ("	Version: %d.%d.%d\n", 
					pDefaultImageRec->majorVersion, pDefaultImageRec->minorVersion, 0 ));
}

//  DdmBootMgr::InitDefaultImageTable()
//
//  Description:
//	  Iniatialize the DefaultImageTable with contents of boot ROM managed 
//	  image flash.
//
//  Inputs:
//	  status
//
//  Outputs:
//	  status - return code from UpdataDefaultImageTable()
//
#ifdef JFLsWAY
STATUS DdmBootMgr::InitDefaultImageTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::InitDefaultImageTable);
	
	m_iROMImgBlk = 0;
	
	TRACEF( TRACE_L8, ("\n***BootMgr: Scanning ROM Image flash...\n"));
	
	status = UpdateDefaultImageTable(NULL);
	
	return status;
}
#else
STATUS DdmBootMgr::InitDefaultImageTable(Message* pMsg)
{
	TRACE_ENTRY(DdmBootMgr::InitDefaultImageTable);

	STATUS	status = pMsg->Status();

	delete pMsg;
	
	// Prepare to scan the image flash.
	// m_iROMImgBlk will be our loop control and index.
	m_iROMImgBlk = 0;
	
	TRACEF( TRACE_L3, ("\n***BootMgr: Scanning ROM IMage flash..."));
	
	status = UpdateDefaultImageTable(NULL);
	
	return status;
}
#endif


//  DdmBootMgr::UpdateDefaultImageTable()
//
//  Description:
//	  Iniatialize the DefaultImageTable with contents of boot ROM managed 
//	  image flash.
//
//  Inputs:
//	  pMsg - either reply from ModifyDefaultImage request or NULL
//
//  Outputs:
//	  status - 
//
STATUS DdmBootMgr::UpdateDefaultImageTable(Message *pMsg)
{

	TRACE_ENTRY(DdmBootMgr::UpdateDefaultImageTable);

	STATUS	status;

	if (pMsg)
	{
		status = pMsg->Status();
		if (status != OK)
		{
			TRACEF(TRACE_L3, ("\n***DdmBootMgr:UpdateDefaultImageTable received the following message with status = 0x%x.",status));
			pMsg->Dump("Message:");
		}
		delete pMsg;
	}	
	else
		status = OK;

	// Code to update the DefaultImageTable with boot flash images.
	// m_iROMImgBlk is the index for the bootROM image flash 'block'.
	// We're going to scan the defaultImageTable for outdated code.
	// jlo temp hack
	while (m_iROMImgBlk < 4)
	{
		// Get image addr in flash incrementing our block index.
		status = image_hdr(m_iROMImgBlk++, &m_pImgHdr, &m_ImgSiz);
		
		// HACK ALERT:  image_hdr needs event codes.  for now: status == 0 => OK.
		// The only other possible  error is invalid image header signature.
		if (status != OK)
		{
			status = CTS_IOP_BAD_IMAGE_SIGNATURE;
			TRACEF( TRACE_L3, ("\n***BootMgr: Invalid Image Signature in Flash block %d.\n",m_iROMImgBlk));
			continue;
		}
		else
		if (((ImageType)m_pImgHdr->i_type != HBC_IMAGE) &&
			((ImageType)m_pImgHdr->i_type != NAC_IMAGE) &&
			((ImageType)m_pImgHdr->i_type != SSD_IMAGE))
		{
			status = CTS_IOP_BAD_IMAGE_SIGNATURE;
			TRACEF( TRACE_L3, ("\n***BootMgr: Invalid Image type in Flash block %d.\n",m_iROMImgBlk));
			continue;
		}
		
		TRACEF( TRACE_L8, ("\n***BootMgr: Found a valid Image in Flash block %d.\n",m_iROMImgBlk));
		printf("Image Name: %-10s ", m_pImgHdr->i_imagename);
		printf("Ver:%d.%02d. ",
				m_pImgHdr->i_mjr_ver, m_pImgHdr->i_mnr_ver);
		printf("Date:%d/%d/%d Time:%d:%d:%d\n\r",
				m_pImgHdr->i_month, m_pImgHdr->i_day, m_pImgHdr->i_year,
				m_pImgHdr->i_hour, m_pImgHdr->i_min, m_pImgHdr->i_sec);
		
		m_fHaveDefaultImageType = FALSE;
		// Scan the default image table.
		for (U32 j = m_iDefaultImageTable; 
			 ((j < m_nDefaultImageTableRows) && (m_fHaveDefaultImageType==FALSE)); 
			 j++)
		{
			// If the current ROM flash image type matches the current default image 
			// and is more recent (remembering if we have an image of this type)...
			m_fHaveDefaultImageType = (m_pImgHdr->i_type == m_pDefaultImageTable[j].type);
			if (m_fHaveDefaultImageType &&
				(CompareVersions(m_pImgHdr, &m_pDefaultImageTable[j]) == 1))
			{
				// ROM has later version.  Prompt to take it?
				DisplayImageHdr( m_pImgHdr,
								"\n***DdmBootMgr: The boot ROM flash contains the image...");
				DisplayDefaultImage( &m_pDefaultImageTable[j],
								  "\nWhich is newer than the current default image, which is:" );
				TRACEF( TRACE_ALL_LVL, ("\nWould you like to update our default image with the one from the boot flash?"));
				char c = getchar();
				// If the user wants it get it.
				if (c == 'y' || c == 'Y')
				{
					// Allocate, Initialize, and send a DdmUpgradeMgr msg
					// to add the image to the PTS Image Descriptor Table.
					MsgModifyDefaultImage* pMsgModifyDefaultImage
					 = new MsgModifyDefaultImage(	m_pDefaultImageTable[j].imageKey,
					 								m_ImgSiz, m_pImgHdr);
					Send(pMsgModifyDefaultImage,
						REPLYCALLBACK(DdmBootMgr, UpdateDefaultImageTable) );
					return OK;
				}
			}	// if (...
		}	// for (j...
		
		// So, do we want the image or not...
		if (!m_fHaveDefaultImageType)
		{
			// Allocate, Initialize, and send a DdmUpgradeMgr msg
			// to add the image to the PTS Image Descriptor Table.
			MsgAddImage* pMsgAddImage
			 = new MsgAddImage(m_ImgSiz, m_pImgHdr);
			Send(	pMsgAddImage,
					REPLYCALLBACK(DdmBootMgr, HandleAddNewImageReply) );
			return OK;
		}	// if (fAddImage...
	} // while (...

	status = ReadOldIopStatusTable(NULL, status);
	return status;
}

//  DdmBootMgr::HandleAddNewImageReply()
//
//  Description:
//	  Read the default image table added by the Upgrade Master
//
//  Inputs:
//	  pMsg - reply from Add Image request to Upgrade Master
//
//  Outputs:
//	  status - returns OK
//
STATUS DdmBootMgr::HandleAddNewImageReply(Message* pMsg)
{
	TRACE_ENTRY(DdmBootMgr::HandleAddNewImageReply);

	MsgAddImage*	pMsgAddImage = (MsgAddImage*)pMsg;

	STATUS			status = pMsgAddImage->Status();
 
	if (status != OK)
	{
		TRACEF(TRACE_L3, ("\n***DdmBootMgr: ERROR Inserting image.  Status = 0x%x.\n",status));
		// TODO Throw an error here!
	}

    // read the default image row (which was added by the upgrade master)
    // from the table
    RowId imageKey = pMsgAddImage->GetImageKey();
   	DefaultImageRecord::RqReadRow* pRqReadRow 
     = new DefaultImageRecord::RqReadRow(CT_DEF_IMAGE_TABLE_IMAGEKEY, &imageKey,
		sizeof(rowID));
	Send(pRqReadRow, REQUESTCALLBACK(DdmBootMgr, HandleDefaultImageReply));
	
	return OK;

}

//  DdmBootMgr::HandleDefaultImageReply()
//
//  Description:
//	  Add the default image row to local default image table and iterate
//	  back to discover more images in the boot ROM if applicable
//
//  Inputs:
//	  pMsg - reply from Add Image request to Upgrade Master
//
//  Outputs:
//	  status - returns OK
//
STATUS DdmBootMgr::HandleDefaultImageReply(Message* pMsg)
{
	TRACE_ENTRY(DdmBootMgr::HandleDefaultImageReply);
	
	STATUS	status = pMsg->Status();
	DefaultImageRecord::RqReadRow* pReply = (DefaultImageRecord::RqReadRow*) pMsg;
 
	if (status != OK)
	{
		TRACEF(TRACE_L3, ("\n***DdmBootMgr: ERROR Reading default image.  Status = 0x%x.\n",status));
		// TODO Throw an error here!
	}
	
	// Calculate old and new default image table sizes and alloc mem for new.
	U32 cbDefaultImageTable = sizeof(DefaultImageRecord) * m_nDefaultImageTableRows++;
	U32 cbNewDefaultImageTable = sizeof(DefaultImageRecord) * m_nDefaultImageTableRows;
	DefaultImageRecord* pNewDefaultImageTable = (DefaultImageRecord*) new char[cbNewDefaultImageTable];
		
	// Copy any existing default image table data into new memory.
	if (m_pDefaultImageTable && cbDefaultImageTable)
	{
		DefaultImageRecord* pOldDefaultImageTable = m_pDefaultImageTable;
		memcpy( pNewDefaultImageTable, pOldDefaultImageTable, cbDefaultImageTable );
		CheckFreeAndClear(pOldDefaultImageTable); 
	}
		
	// Update our DefaultImageTable 
	m_pDefaultImageTable = pNewDefaultImageTable;
	m_pDefaultImageTable[m_nDefaultImageTableRows-1] = *pReply->GetRowCopy();
	delete pReply;

	// We've just finished reading the DefaultImageTable to reflect a new
	// image we found during our scan of the Boot ROM image flash. 
	// If we still have flash blocks to scan call UpdateDefaultImageTable.
	if (m_iROMImgBlk < 4)
		status = UpdateDefaultImageTable(NULL);
	else
		// Otherwise On with initialization.  Call ReadOldIopStatusTable().
		status = ReadOldIopStatusTable(NULL, status);

	return status;
}


#pragma mark ### IopStatusTable Initialization ###

// 
//  DdmBootMgr::ReadOldIopStatusTable()
//
//  Description:
//	  Read the existing IopStatusTable to verify the presence of Iops.
//
//  Inputs:
//	  status - 
//
//  Outputs:
//	  status - returns return code from PTS read table initialization
//
STATUS DdmBootMgr::ReadOldIopStatusTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::ReadOldIopStatusTable);

	TRACEF( TRACE_L3, ("\n***DdmBootMgr:Reading Old IOPStatusTable...\n"));

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

//
//  DdmBootMgr::HaveCMBUpdateIopStatusTable()
//
//  Description:
//	  Have the CMB ddm update the Iop Status Table.
//
//  Inputs:
//	  status - return status from PTS read table
//
//  Outputs:
//	  status - returns OK
//
STATUS DdmBootMgr::HaveCMBUpdateIopStatusTable(void *pClientContext, STATUS status)
{ 
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::HaveCMBUpdateIopStatusTable);
	
	if (status != OS_DETAIL_STATUS_SUCCESS)
		TRACEF( TRACE_L3, ("\n***DdmBootMgr: ERROR reading Old IOPStatusTable: 0x%x.\n",status));
		
	TRACEF( TRACE_L8, ("\n***DdmBootMgr: Updating IOPStatusTable...\n"));

	m_pCmbPollAllIopsMsg = new MsgCmbPollAllIops;
	
	// Send the message to the CMB. No Context.
	Send(m_pCmbPollAllIopsMsg, (ReplyCallback)&ReadNewIopStatusTable);
				 
	return OK;
}



// ReadNewIopStatusTable -
// Read the current IopStatusTable to verify the presence of Iops.
STATUS DdmBootMgr::ReadNewIopStatusTable(Message*	pMsg)
{
STATUS status = pMsg->DetailedStatusCode;

	TRACE_ENTRY(DdmBootMgr::ReadNewIopStatusTable);

	if (status != OS_DETAIL_STATUS_SUCCESS)
		TRACEF(TRACE_L3, ("\n***DdmBootMgr:  ERROR from CMB updating IOPStatusTable.  Status = 0x%x.\n", status));

	TRACEF( TRACE_L8, ("\n***DdmBootMgr:Reading updated IOPStatusTable...\n"));

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


//
//  DdmBootMgr::GetIopTypeCode()
//
//  Description:
//	  This method returns the three letter code for the IOP.  
//
//  Inputs:
//	  pIopStatusRec - IOP Status record associated with the IOP.
//
//  Outputs:
//	  returns three letter code for the IOP
//
const char*	const DdmBootMgr::GetIopTypeCode(IOPStatusRecord* pIopStatusRec)
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

//
//  DdmBootMgr::GetIopTypeDesc()
//
//  Description:
//	  This method returns the type description string for the Iop. 
//	  max length returned = 43 
//
//  Inputs:
//	  pIopStatusRec - IOP Status record associated with the IOP.
//
//  Outputs:
//	  returns the type descriptor string for the IOP.
//
const char*	const DdmBootMgr::GetIopTypeDesc(IOPStatusRecord* pIopStatusRec)
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

//
//  DdmBootMgr::GetIopStateCode()
//
//  Description:
// 	  This method returns the Iop_State code string for the Iop.  
//	  max length returned = 16
//
//  Inputs:
//	  pIopStatusRec - IOP status record associated with the IOP.
//
//  Outputs:
//	  returns the state code string for the IOP.
//
const char*	const DdmBootMgr::GetIopStateCode(IOPStatusRecord* pIopStatusRec)
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

//
//  DdmBootMgr::GetIopStateDesc()
//
//  Description:
// 	  This method returns the Iop_State description string for the Iop.  
//	  max length returned = 51
//
//  Inputs:
//	  pIopStatusRec - IOP status record associated with the IOP.
//
//  Outputs:
//	  returns the state descriptor string for the IOP.
//
const char*	const DdmBootMgr::GetIopStateDesc(IOPStatusRecord* pIopStatusRec)
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


//
//  DdmBootMgr::DumpIopStatusRec()
//
//  Description:
// 	  This method displays the Iop Status Record 
//
//  Inputs:
//	  TraceLvl - trace level
//	  pIopStatusRec - IOP status record associated with the IOP.
//	  pString - output string
//
//  Outputs:
//
void DdmBootMgr::DumpIopStatusRec(U32	TraceLvl, IOPStatusRecord* pIopStatusRec, const char* const pString)
{
	TRACE_ENTRY(DdmBootMgr::DumpIopStatusRec);

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
 
//
//  DdmBootMgr::Dump2IopStatusRecs()
//
//  Description:
// 	  This method displays two IopStatusrecords aide by side.
//
//  Inputs:
//	  TraceLvl - trace level
//	  pIopStatusRec1 - IOP status record associated with the first IOP.
//	  pString1 - output string1
//	  pIopStatusRec2 - IOP status record associated with the second IOP.
//	  pString2 - output string2
//
//  Outputs:
//
void DdmBootMgr::Dump2IopStatusRecs(U32	TraceLvl, 
									IOPStatusRecord* pIopStatusRec1,
									char* pString1,
									IOPStatusRecord* pIopStatusRec2,
									char* pString2
									)
{
	TRACE_ENTRY(DdmBootMgr::Dump2IopStatusRecs);

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

//
//  DdmBootMgr::CheckIopStatusTable()
//
//  Description:
// 	  This method verifies the IopStatusTable is OK to proceed booting. 
//
//  Inputs:
//	  status 
//
//  Outputs:
//	  status - return status from DefineIopImageTable()
//
STATUS DdmBootMgr::CheckIopStatusTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	U32		fIopTableOK = TRUE;
	U32		m,n;

	TRACE_ENTRY(DdmBootMgr::CheckIopStatusTable);

	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACEF(TRACE_L3, ("\n***DdmBootMgr: ERROR with IOPStatusTable.  Status = 0x%x.\n",status));
		// TODO Throw an error here!
	}
	else
	{
		TRACEF( TRACE_L8, ("\n***DdmBootMgr: Reconciling old and new IOPStatusTables...\n"));

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
				DumpIopStatusRec( TRACE_L3, &m_pIopStatusTableRows[n], "***DdmBootMgr: Newly Discovered" );

				// Initialize a new Iop Image Record for the new IOP.
				// jlo - we are doing this is UpdateIopImageTable()
				/*m_IopImageTableRows[n].handle.Clear();
				m_IopImageTableRows[n].slot = m_pIopStatusTableRows[n].Slot;
				m_IopImageTableRows[n].currentImage.Clear();								// = {0,0,0};
				m_IopImageTableRows[n].primaryImage.Clear();	// Call MakePrimary() to do this
				m_IopImageTableRows[n].imageOne.Clear();		// Call AssociateImage() to set this
				m_IopImageTableRows[n].imageTwo.Clear();				// = {0,0,0};
				m_IopImageTableRows[n].imageOneAccepted = false;	
				m_IopImageTableRows[n].imageTwoAccepted = false;
				m_IopImageTableRows[n].imageState = eImageState_Initialized;
				m_IopImageTableRows[n].timeBooted = 0;*/
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
				DumpIopStatusRec( TRACE_L3, &m_pIopStatusTableRows[n], "***DdmBootMgr: Un-Discovered" );
			}
			else
			// If the old and new serial #s match...
			if (strcmp(m_pOldIopStatusTableRows[n].SerialNumber, m_pIopStatusTableRows[n].SerialNumber) == 0)
			{
				// We have the same Iop
				TRACEF( TRACE_L3, ("\n***DdmBootMgr: Rediscovered IOP Moved in Slot %d.\n", m_pOldIopStatusTableRows[n].Slot));
				DumpIopStatusRec( TRACE_L8, &m_pIopStatusTableRows[n], "***DdmBootMgr: ReDiscovered" );
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
					TRACEF( TRACE_L3, ("\n***DdmBootMgr: An IOP Moved from Slot %d to Slot %d!\n", m_pOldIopStatusTableRows[n].Slot, m_pIopStatusTableRows[m].Slot));
					Dump2IopStatusRecs( TRACE_L3,
										&m_pOldIopStatusTableRows[n], "Old",
										&m_pIopStatusTableRows[m], "New"
									  );
				}
				else
				{
					// An Iop has been replaced!
					TRACEF( TRACE_L3, ("\n***DdmBootMgr: An IOP has been replaced!\n"));
					Dump2IopStatusRecs( TRACE_L3,
										&m_pOldIopStatusTableRows[n], "Old", 
										&m_pIopStatusTableRows[n], "New"
									  );
					fIopTableOK = m_pOldIopStatusTableRows[n].IOP_Type == m_pIopStatusTableRows[n].IOP_Type;
				}						
			}
		
		
		// Status of Iop Data? 
		if (fIopTableOK)
			TRACEF( TRACE_L3, ("\n***DdmBootMgr: IopStatusTable OK!\n"))
		else
			TRACEF( TRACE_L3, ("\n***DdmBootMgr:  ERROR with IopStatusTable:0x%x.\n",status));

		// proceed to the definition of the IOP Image table
		status = DefineIopImageTable(NULL, status);

	}	
	return status;
}


#pragma mark ### IopImageTable Initialization ###

//
//  DdmBootMgr::DefineIopImageTable()
//
//  Description:
// 	  Insure IOPImageTable exists.
//
//  Inputs:
//	  status 
//
//  Outputs:
//	  status - return status from PTS Define Table initialization
//
STATUS DdmBootMgr::DefineIopImageTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)
TRACE_ENTRY(DdmBootMgr::DefineIopImageTable);
	
	// This is the code to create the IOPImageTable.
		  
	// Allocate an Define Table object for the IOPImage Table.
	TSDefineTable*	pDefineTable = new TSDefineTable;

	// Initialize the define Table object.
	status = pDefineTable->Initialize(
		this,								// DdmServices* pDdmServices,
		(char*)IOPImageRecord::TableName(),		// String64 prgbTableName,
		IOPImageRecord::FieldDefs(),		// fieldDef* prgFieldDefsRet,
		IOPImageRecord::FieldDefsSize(),	// U32 cbrgFieldDefs,
		m_nIopStatusTableRows,				// U32 cEntriesRsv,
		true,								// bool* pfPersistant,
		(pTSCallback_t)&CheckIopImageTable,// pTSCallback_t pCallback,
		NULL								// void* pContext
	);
	
	// Initiate the define table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pDefineTable->Send();

	return status;
}

//
//  DdmBootMgr::CheckIOPImageTable()
//
//  Description:
// 	  Check if IOPImageTable was defined.
//
//  Inputs:
//	  status - return status from PTS Define Table
//
//  Outputs:
//	  status - return OK or highly descriptive status code
//
STATUS DdmBootMgr::CheckIopImageTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::CheckIopImageTable);

	if (status == ercTableExists)
	{
		TRACEF( TRACE_L8, ("\n***DdmBootMgr:Found IOPImageTable."));
		// IOPImageTable already exists. Try to read it.
		status = ReadIopImageTable(pClientContext, status);
		return status;
	}
	else
	if (status != OK)
	{
		TRACEF(TRACE_L3, ("\n***DdmBootMgr:CheckIopImageTable received status = 0x%x from DefineIopImageTable.",status));
		// To Do Throw an error here.
		;
		return OK;
	}  
		
	// We need to insert a new IOPImageTable.
	m_pIopImageTableRows = &m_IopImageTableRows[0];		
	status = InitIopImageTable(pClientContext, status);
	return status;
}

//
//  DdmBootMgr::InitIopImageTable()
//
//  Description:
// 	   Write a new blank image row for each slot.
//
//  Inputs:
//	  status - 
//
//  Outputs:
//	  status - return OK or highly descriptive status code
//
STATUS DdmBootMgr::InitIopImageTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	if (m_iIopImage2Init < m_nIopStatusTableRows)
	{
		//  create a new InsertRow message, and load it up with our row stuff
		TSInsertRow* pInsertRow = new TSInsertRow;
		m_IopImageTableRows[m_iIopImage2Init].slot = m_pIopStatusTableRows[m_iIopImage2Init].Slot;
		
		//  add our new-row data to the add-row message
		status = pInsertRow -> Initialize (
			this,
			CT_IOP_IMAGE_TABLE_NAME,
			&m_IopImageTableRows[m_iIopImage2Init],				// row data
			&m_IopImageTableRows[m_iIopImage2Init].rid,			// where to stashnew row's rowId
			(pTSCallback_t)&InitIopImageTable,					// our callback
			NULL
		);
			
		//  all ready, send off the message object
		pInsertRow->Send();
		
		m_iIopImage2Init++;
	}
	else
		status = UpdateIopImageTable(pClientContext, status);
		
	return OK;
}

//
//  DdmBootMgr::ReadIOPImageTable()
//
//  Description:
// 	   Read existing IOPImageTable
//
//  Inputs:
//	  status - 
//
//  Outputs:
//	  status - return OK or highly descriptive status code
//
STATUS DdmBootMgr::ReadIopImageTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::ReadIOPImageTable);

	TRACEF( TRACE_L8, ("\n***DdmBootMgr: Reading IOPImageTable...\n"));

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

//
//  DdmBootMgr::HandleReadIopImageTableReply()
//
//  Description:
// 	   Handle reply of Read IOPImageTable.
//
//  Inputs:
//	  status - 
//
//  Outputs:
//	  status - return OK or highly descriptive status code
//
STATUS DdmBootMgr::HandleReadIopImageTableReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::HandleReadIOPImageTableReply);

	if (status != OK)
	{
		TRACEF( TRACE_L3, ("\n***DdmBootMgr:ReadIOPImageTable returned status 0x%x.\n", status));
		return status;
	}
	else
	{
		if (m_nIopImageTableRows)
		{
			// Copy The returned IopStatusTable into our local records.
			m_cbIopImageTableRows = m_nIopImageTableRows * sizeof(IOPImageRecord);
			bcopy ((const char*)m_pIopImageTableRows, (char*)&m_IopImageTableRows, m_cbIopImageTableRows );
			m_pIopImageTableRows = m_IopImageTableRows;
		}
	}
	
	status = UpdateIopImageTable(pClientContext, status);
	return status;
}

//
//  DdmBootMgr::UpdateIopImageTable()
//
//  Description:
// 	   Updates the IOP image table
//
//  Inputs:
//	  status - 
//
//  Outputs:
//	  status - return OK or highly descriptive status code
//
STATUS DdmBootMgr::UpdateIopImageTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)
	
	int i;
	// For each record in the IopStatusTable...
	for (i = 0; i < m_nIopStatusTableRows; i++)
		// If it's a valid IOP type AND it'S IopImageRecord is uninitialized...
		if (
			((m_pIopStatusTableRows[i].IOP_Type == IOPTY_HBC) ||
			(m_pIopStatusTableRows[i].IOP_Type == IOPTY_NAC) ||
			(m_pIopStatusTableRows[i].IOP_Type == IOPTY_SSD)) &&
			(m_pIopImageTableRows[i].imageState	== eImageState_Uninitialized))
		{
			m_IopImageTableRows[i].handle.Clear();
			m_IopImageTableRows[i].slot	= m_pIopStatusTableRows[i].Slot;
			m_IopImageTableRows[i].currentImage.Clear();								// = {0,0,0};
			m_IopImageTableRows[i].primaryImage.Clear();	// call MakePrimary() to do this
			m_IopImageTableRows[i].imageOne.Clear();	// call AssociateImage() to set this
			m_IopImageTableRows[i].imageTwo.Clear();									// = {0,0,0};
			m_IopImageTableRows[i].imageOneAccepted	= false;	
			m_IopImageTableRows[i].imageTwoAccepted	= false;
			m_IopImageTableRows[i].imageState		= eImageState_Initialized;
			m_IopImageTableRows[i].timeBooted		= 0;

			//  create a new InsertRow message, and load it up with our row stuff
			// jlo - shouldn't this be a modify row request?
			/*TSInsertRow* pInsertRow = new TSInsertRow;
				
			//  add our new-row data to the add-row message
			status = pInsertRow -> Initialize (
				this,
				CT_IOP_IMAGE_TABLE_NAME,
				&m_IopImageTableRows[i],				// row data
				&m_IopImageTableRows[i].rid,			// where to stashnew row's rowId
				(pTSCallback_t)&IopImageRowModified,		// our callback
				NULL
			);
					
			//  all ready, send off the message object
			pInsertRow->Send();*/
			
			TSModifyRow* pModifyRow = new TSModifyRow;
				
			//  add our new-row data to the add-row message
			status = pModifyRow -> Initialize (
				this,
				CT_IOP_IMAGE_TABLE_NAME,
				CT_PTS_RID_FIELD_NAME,
				&m_IopImageTableRows[i].rid,
				sizeof(m_IopImageTableRows[i].rid),
				&m_IopImageTableRows[i],				// row data
				sizeof(m_IopImageTableRows[i]),
				1,
				NULL,
				NULL,
				NULL,
				(pTSCallback_t)&IopImageRowModified,		// our callback
				NULL
			);
					
			//  all ready, send off the message object
			pModifyRow->Send();

		}
	
	status = FinishInitialize();

   	return (CTS_SUCCESS);
}

//
//  DdmBootMgr::IopImageRowModified()
//
//  Description:
// 	   Call back for IOP image table modification
//
//  Inputs:
//	  status - return status from PTS modify row
//
//  Outputs:
//	  status - return status from PTS modify row
//
STATUS DdmBootMgr::IopImageRowModified(void* pClientContext, STATUS status)
{
	TRACE_ENTRY(DdmBootMgr::IopImageRowModified);

	if (status!=OK)
	{
		TRACEF( TRACE_L8, ("\n***DdmBootMgr: Iop Image Row Modify failed!\n"))
	}
	
	return status;
}

//
//  DdmBootMgr::FinishInitialize()
//
//  Description:
// 	   End point for Boot Manager initialization
//
//  Inputs:
//
//  Outputs:
//
STATUS DdmBootMgr::FinishInitialize()
{
	// We're finished with our initialization.
	return Ddm::Initialize(m_pInitializeMsg);
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


// Enable -- Start-it-up -----------------------------------------------DdmBootMgr-
//
STATUS DdmBootMgr::Enable(Message *pMsg)
{ 
STATUS	status;

	TRACE_ENTRY(DdmBootMgr::Enable);

	m_pEnableMsg = pMsg;
	status = Boot();	
	return status;
}


//
//  DdmBootMgr::Boot()
//
//  Description:
// 	  This is the entry point for the BootMgr's actual boot operation.
//
//  Inputs:
//
//  Outputs:
//	  status - return status from ListenOnIopStatusTable
//
STATUS DdmBootMgr::Boot()
{
	return ListenOnIopStatusTable();
}


//
//  DdmBootMgr::ListenOnIopStatusTable()
//
//  Description:
// 	  Listen On the IopStatusTable in preparation for bringing up the Iops.
//
//  Inputs:
//
//  Outputs:
//	  status - return status from PTS Listen initialize
//
STATUS DdmBootMgr::ListenOnIopStatusTable()
{
STATUS status;

	TRACE_ENTRY(DdmBootMgr::ListenOnIopStatusTable);	

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

//
//  DdmBootMgr::HandleIopSTListenReply()
//
//  Description:
// 	  This is the reply handler specified in ListenOnIopStatusTable.  It will
// 	  be called:
// 		- In response to the initial listen message with the original table.
// 		- When any IopStatusTable record has been modified.
// 		- When we stop the listen in preparation for shutdown.
//    In turn HandleIopSTListenReply will set up and maintain IopStateRecord 
// 	  instance data for use by the IopManager objects.
//
//  Inputs:
// 	  status - return status from PTS listen operation
//
//  Outputs:
//	  status - returns OK or a highly descriptive error code.
//
STATUS DdmBootMgr::HandleIopSTListenReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::HandleIopSTListenReply);

	TRACEF( TRACE_L3, ("\n***DdmBootMgr:ListenOnIopStatusTable returned status 0x%x.\n", status));

	// Check for reply to the initial listen message with the original table.
	if (*m_pIopSTListenReplyType == ListenInitialReply)
	{
		// Copy The returned IopStatusTable into our local records.
		bcopy ((const char*)m_pIopStatusTableRows, (char*)&m_IopStatusTableRows, m_cbIopStatusTableRows );
		m_pIopStatusTableRows = m_IopStatusTableRows;
		m_nIopStatusTableRows = m_cbIopStatusTableRows / sizeof(IOPStatusRecord);
		status = ListenOnIopImageTable();
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
			TRACEF(TRACE_L3,("\n***DdmBootMgr: Invalid IOPStatusTable Listen Reply.  Status = 0x%x.\n", status));
			DumpIopStatusRec( TRACE_L3, m_pIopSTModifiedRecord, "***DdmBootMgr: Invalid" );
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
		TRACEF(TRACE_L3,("\n***DdmBootMgr: Received IOPStatusTable StopListen Reply.  Status = 0x%x.\n", status));

		if (status != OS_DETAIL_STATUS_SUCCESS)
		;	// TODO: Throw some event here.
		
		status = ShutDown();
	}

	return status;
}

//
//  DdmBootMgr::ListenOnIopImageTable()
//
//  Description:
// 	  Listen On the IopImageTable in preparation for bringing up the Iops.
//
//  Inputs:
//
//  Outputs:
//	  status - return status from PTS Listen initialize
//
STATUS DdmBootMgr::ListenOnIopImageTable()
{
STATUS status;

	TRACE_ENTRY(DdmBootMgr::ListenOnIopImageTable);	

	// Alloc, init and send off a Listen object for the IopStatusTable.
	m_pListenIopImageTable = new TSListen;
	
	status = m_pListenIopImageTable->Initialize(
		this,											// DdmServices pDdmServices,
		ListenOnModifyAnyRowAnyField,					// U32 ListenType,
		CT_IOP_IMAGE_TABLE_NAME,							// String64 rgbTableName,
		NULL,											// String64 prgbRowKeyFieldName,
		NULL,											// void* prgbRowKeyFieldValue,
		0,												// U32 cbRowKeyFieldValue,
		NULL,											// String64 prgbFieldName,
		NULL,											// void* prgbFieldValue,
		0,												// U32 cbFieldValue,
		ReplyContinuous|ReplyFirstWithTable|ReplyWithRow,	// U32 ReplyMode,
		&m_pIopImageTableRows,							// void** ppTableDataRet,
		&m_cbIopImageTableRows,							// U32* pcbTableDataRet,
		&m_IopImageListenerId,							// U32* pListenerIDRet,
		&m_pIopImageListenReplyType,					// U32** ppListenTypeRet,
		&m_pIopImageModifiedRecord,						// void** ppModifiedRecordRet,
		&m_cbIopImageModifiedRecord,					// U32* pcbModifiedRecordRet,
		(pTSCallback_t)&HandleIopImageListenReply,		// pTSCallback_t pCallback,
		this
	);
	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		m_pListenIopImageTable->Send();

	return status;
}

//
//  DdmBootMgr::HandleIopImageListenReply()
//
//  Description:
// 	  This is the reply handler specified in ListenOnIopImageTable.  It will
// 	  be called:
// 		- In response to the initial listen message with the original table.
// 		- When any IopImageTable record has been modified.
//
//  Inputs:
// 	  status - return status from PTS listen operation
//
//  Outputs:
//	  status - returns OK or a highly descriptive error code.
//
STATUS DdmBootMgr::HandleIopImageListenReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::HandleIopImageListenReply);

	TRACEF( TRACE_L3, ("\n***DdmBootMgr:HandleIopImageListenReply returned status 0x%x.\n", status));

	// Check for reply to the initial listen message with the original table.
	if (*m_pIopImageListenReplyType == ListenInitialReply)
	{
		// Copy The returned IopImageTable into our local records.
		bcopy ((const char*)m_pIopImageTableRows, (char*)&m_IopImageTableRows, m_cbIopImageTableRows );
		m_pIopImageTableRows = m_IopImageTableRows;
		m_nIopImageTableRows = m_cbIopImageTableRows / sizeof(IOPImageRecord);
		status = InitSystemStatusRecord();
		return status;
	}
	
	// Check for reply when any IopImageTable record has been modified.
	if (*m_pIopImageListenReplyType == ListenOnModifyAnyRowAnyField)
	{
		// Find the returned record in our array.
		for (m_iIopImageModifiedRecord = 0; 
			m_iIopImageModifiedRecord < m_nIopImageTableRows; m_iIopImageModifiedRecord++)
			if (m_pIopImageTableRows[m_iIopImageModifiedRecord].slot == m_pIopImageModifiedRecord->slot)
				break;
				
		if (m_iIopImageModifiedRecord == m_nIopImageTableRows)
		{
			TRACEF(TRACE_L3,("\n***DdmBootMgr: Invalid IOPImageTable Listen Reply.  Status = 0x%x.\n", status));
		}
		else
		{
			// Update the array with the new data.
			m_IopImageTableRows[m_iIopImageModifiedRecord] = *m_pIopImageModifiedRecord;
		}
	}
		
	return status;
}


//
//  DdmBootMgr::InitSystemStatusRecord()
//
//  Description:
//	  Initialize the System Status Record's masks of Iops present and active.
//
//  Inputs:
//
//  Outputs:
//	  status - returns status from PTS Modify row initialize
//
STATUS DdmBootMgr::InitSystemStatusRecord()
{
STATUS status;;

	TRACE_ENTRY(DdmBootMgr::InitSystemStatusRecord);

	TRACEF( TRACE_L8, ("\n***DdmBootMgr: Modifying SystemStatusRecord...\n"));

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
//			(m_aIopManagers[i])IOP_Type == IOPTY_HBC))
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

//
//  DdmBootMgr::HandleWriteSystemStatusTableReply()
//
//  Description:
//	  This is the reply handler specified in UpdateSystemStatusTable.  It will
//	  be called when the System Status Table has been updated.  In turn, it sets
//	  up a listen on the System Status Table in preparation for powering up
// 	  the Iops.  It will then call StartIops to initiate the powering up of
//	  the Iops
//
//  Inputs:
//	  status - return from PTS modify row command 
//
//  Outputs:
//	  status - returns OK or highly descriptive status code
//
STATUS DdmBootMgr::HandleWriteSystemStatusTableReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::HandleWriteSystemStatusTableReply);
	
	TRACEF(TRACE_L3, ("\n***DdmBootMgr: Modify SystemStatusRecord status=0x%x.\n",status));
	
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		;	// TODO: Throw some event here.
	}

	// Call StartIops to power on and boot the Iops.
	status = InitIopMgrs();
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACEF(TRACE_L3,("\n***DdmBootMgr: ERROR Initing IOP Managers.  Status = 0x%x.\n", status ));
		// TODO: Throw some event here.
	}
	else
		status = StartIops();

	return status;
}


//
//  DdmBootMgr::InitIopMgrs()
//
//  Description:
//	  This method will loop through the IopStatusTable instantiating IopManger
//	  objects for each Iop.  Each IopManager object will then be told to power
//	  on its Iop and get it on the PCI bus by sending commands to the Iop via
//	  the Card Management Bus (CMB) Ddm.
//
//  Inputs:
//
//  Outputs:
//	  status - returns OK or highly descriptive status code
//
STATUS DdmBootMgr::InitIopMgrs()
{
	// Instantiate an IopManager for each Iop.
	for (int i = 0;  i < m_nIopStatusTableRows; i++)
		// If we found a powered down IOP or an HBC IOPStatusTable record, then...
		if ((m_pIopStatusTableRows[i].eIOPCurrentState == IOPS_POWERED_DOWN) ||
//			(m_pIopStatusTableRows[i].IOP_Type == IOPTY_HBC))
			(m_pIopStatusTableRows[i].Slot == Address::iSlotHbcMaster))
		{
			// Instantiate an IopManager object to manage the IOP for me.
			m_aIopManagers[i] = new IopManager(this, &(m_pIopStatusTableRows[i]), &(m_pIopImageTableRows[i]));
			if (m_aIopManagers[i] == NULL)
				return CTS_OUT_OF_MEMORY;
		}
		
	return OK;
}


//
//  DdmBootMgr::StartIops()
//
//  Description:
// 	  Each IopManager object powers on its Iop and gets it on the PCI bus by
//	  sending command to the Iop via the CMB.
//
//  Inputs:
//
//  Outputs:
//	  status - returns OK or highly descriptive status code
//
STATUS DdmBootMgr::StartIops()
{
STATUS status;

	U32	i = 0;		// Skip the HBCs which will always come first.

	// Find an Iop to power up.
	for (;  i < m_nIopStatusTableRows; i++)
		if ((m_pIopStatusTableRows[i].eIOPCurrentState == IOPS_POWERED_DOWN) &&
			(m_pIopStatusTableRows[i].IOP_Type != IOPTY_HBC))
		{
			status = m_aIopManagers[i]->StartIop();
			if (status != OS_DETAIL_STATUS_SUCCESS)
			{
				TRACEF(TRACE_L3,("***DdmBootMgr: ERROR Starting IOP in Slot %d.  Status = 0x%x.\n",
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


//  DdmBootMgr::Listen4IopsOnPci()
//
//  Description:
//    This method is called to Listen on the IOPsOnPCIMask mask in the 
//	  system status table.
//
//  Inputs:
////    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmBootMgr::Listen4IopsOnPci(STATUS status)
{
	TRACE_ENTRY(DdmBootMgr::Listen4IopsOnPci);
	TRACEF(TRACE_L3, ("\n***DdmBootMgr: Listen4IopsOnPCI() status = 0x%x.\n",status));

	m_pListen4Iops = new TSTimedListen;
	if (!m_pListen4Iops)
		status = CTS_OUT_OF_MEMORY;
	else			
		// We are going to listen for the IopsOnPciMask to equal the PresentIopsMask.
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
			&m_pListenReplyType,							// U32** ppListenTypeRet,
			&m_pNewSystemStatusRecord,						// void** ppModifiedRecordRet,
			&m_cbNewSystemStatusRecord,						// U32* pcbModifiedRecordRet,
			TSCALLBACK(DdmBootMgr, IopsOnPciListenReply),	// pTSCallback_t pCallback,
			60000000,										// U32 iTimeOut, 60 secs.
			NULL											// void* pContext
		);
	
	if (status == OK)
			m_pListen4Iops->Send();
	else
	{
		;	// TODO:  Throw error here.
	}
	
	return OK;
}

//  DdmBootMgr::IopsOnPciListenReply()
//
//  Description:
//    This method is called back by Listen when the System Status Table's
//	IopsOnPciMask comes equal to the PresentIopsMask.
//
//  Inputs:
//    status - The returned status of the PTS listen operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmBootMgr::IopsOnPciListenReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::IopsOnPciListenReply);

	// If it is the initial reply ignore it.
	if (m_pListenReplyType && (*m_pListenReplyType == ListenInitialReply))
	{
		TRACEF(TRACE_L3,("\n***BootMgr: Received Initial Listen Reply for IOPsOnPCIMask.  Status = 0x%x.\n",
				status ));
		return status;
	}	

	// If the Timer we set has already gone off then we have already aborted
	// The only thing we have to do is free the reply type buffer returned to us.  
	if (m_pListenReplyType && (*m_pListenReplyType == ListenReturnedOnStopListen))
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
			
		;	// TODO:  Throw error here.
		return status;
	}
	else
	{
		TRACEF(TRACE_L3,("\n***BootMgr: Received Listen for IOPsOnPCIMask.  Status = 0x%x.\n", status));
		m_fAllIopsOnPciFlag = TRUE;
		
		// We need to reinit the bridges after powering on and connecting all the IOPs.
		// commented out because it breaks the net
		// status = InitBridgeFTree();
		// TRACEF(TRACE_L3,("\n***BootMgr: InitBridgeFTree() returned Status = 0x%x.\n", status ));	
	
		// We need to replace our SystemStatusRecord with the updated one returned by the Listen.
		m_SystemStatusRecord = *m_pNewSystemStatusRecord;

		// Now finish the rest of the boot process.
		status = BootIops();
	}
				
	return status;
}
		
//
//  DdmBootMgr::BootIops()
//
//  Description:
// 	  This method will loop through the IopManager objects cbooting each IOP.
//
//  Inputs:
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmBootMgr::BootIops()
{
STATUS status;

	// Find an Iop to power up.  Skip the HBCs which will always come first.
	for (int i = 0;  i < m_nIopStatusTableRows; i++)
		if ((m_pIopStatusTableRows[i].eIOPCurrentState == IOPS_AWAITING_BOOT) &&
			(m_pIopStatusTableRows[i].IOP_Type != IOPTY_HBC))
		{
			status = m_aIopManagers[i]->BootIop();
			if (status != OS_DETAIL_STATUS_SUCCESS)
			{
				TRACEF(TRACE_L3,("***DdmBootMgr: ERROR Booting IOP in Slot %d.  Status = 0x%x.\n",
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

//  DdmBootMgr::Listen4ActiveIops(STATUS status)
//
//  Description:
//    This method is called to Listen on the ActiveIops mask in the 
//	  system status table.
//
//  Inputs:
//
//    status - 
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmBootMgr::Listen4ActiveIops(STATUS status)
{
	TRACE_ENTRY(DdmBootMgr::Listen4ActiveIops);
	TRACEF(TRACE_L3, ("DdmBootMgr::Listen4ActiveIops() status=0x%x.\n",status));

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
			&m_pListenReplyType,							// U32** ppListenTypeRet,
			&m_pNewSystemStatusRecord,						// void** ppModifiedRecordRet,
			&m_cbNewSystemStatusRecord,						// U32* pcbModifiedRecordRet,
			TSCALLBACK(DdmBootMgr, IopsActiveListenReply),	// pTSCallback_t pCallback,
			300000000,										// U32 iTimeOut,
			NULL											// void* pContext
		);
	
	if (status == OK)
			m_pListen4Iops->Send();
	else
	{
		;	// TODO:  Throw error here.
	}
	
	return status;
}

//
//  DdmBootMgr::IopsActiveListenReply()
//
//  Description:
//    This method is called back by Listen when the System Status Table's
//	ActiveIopsMask comes equal to the PresentIopsMask.
//
//  Inputs:
//    status - The returned status of the PTS listen operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmBootMgr::IopsActiveListenReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::IopsActiveListenReply);
	TRACEF(TRACE_L3, ("\n***DdmBootMgr::Listen4ActiveIops() returned status = 0x%x.\n",status));

	// Before we do anything check if this is the reply we're waiting for 
	// ie the Iop's state changed to AWAITING_BOOT as opposed to initial
	// reply.
	if (m_pListenReplyType && (*m_pListenReplyType == ListenInitialReply))
	{
		TRACEF(TRACE_L8,("\n***BootMgr: Received Initial Listen Reply for ActiveIopsMask.  Status = 0x%x.\n",
				status ));
		// Allow the rest of the system to come up.
		status = FinishEnable();
		return status;
	}	

	// If the Timer we set has already gone off then we have already aborted
	// The only thing we have to do is free the reply type buffer returned to us.  
	if (m_pListenReplyType && (*m_pListenReplyType == ListenReturnedOnStopListen))
	{
		TRACEF(TRACE_L8,("\n***BootMgr: Received Stop Listen Reply for ActiveIopsMask.  Status = 0x%x.\n",
				status ));
		
	};
	
	if (status == CTS_PTS_LISTEN_TIMED_OUT)
	{
		TRACEF(TRACE_L3,("\n***BootMgr: TIMED OUT Listening for ActiveIopsMask.  Status = 0x%x.\n", status ));
		// Now that we're as present, up, and active as we're gonn be, Start the LED heartbeat. 
		status = StartLEDs();
	}
	else
	if (status != OK)
	{
		TRACEF(TRACE_L3,("\n***BootMgr: Received BAD Listen Reply for ActiveIopsMask.  Status = 0x%x.\n",
				status ));
			
		;	// TODO:  Throw error here.
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
		
//
//  DdmBootMgr::FinishEnable()
//
//  Description:
// 	  Return our enable message.
//
//  Inputs:
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmBootMgr::FinishEnable()
{
	STATUS status;

	// For Environment Ddm
	Message *pStartEnvironmentDdmMsg;

	pStartEnvironmentDdmMsg = new Message(ENV_ACTIVATE);
	if (Send(pStartEnvironmentDdmMsg, &DiscardReply))
   	{
     	Tracef("DdmBootMgr::.  Error when activate Environment Ddm.\n");
     	delete pStartEnvironmentDdmMsg;
   	}

	status = Ddm::Enable(m_pEnableMsg);
	if (status != OK)
		TRACEF(TRACE_L3,("\n***BootMgr: ERROR replying to Enable.  Status = 0x%x.\n", status ))		
	return status;
}

//
//  DdmBootMgr::StartLEDs()
//
//  Description:
// 	  This method will start a timer for the LED blinking heartbeat to the Iops.
//
//  Inputs:
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmBootMgr::StartLEDs()
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
		status = Send(m_pStartTimerMsg, NULL, REPLYCALLBACK(DdmBootMgr, BlinkLEDs));
#else
		// Allocate timer for LEDs
		m_pStartTimerMsg = new TimerStatic(DdmBootMgr::BlinkLEDs, this);
		
		// Start the new LED timer.
		m_pStartTimerMsg->Enable(1000000, 1000000);


#endif

	return status;
}


//
//  DdmBootMgr::BlinkLEDs()
//
//  Description:
// 	  This method will loop through the IopStatusTable issuing BlinkLED Msgs
//	  to the Iops
//
//  Inputs:
//
//  Outputs:
//
#if false

STATUS DdmBootMgr::BlinkLEDs(Message* pMsg)
{
STATUS status = pMsg ? pMsg->DetailedStatusCode : OK;

	// Delete the timer reply message.
	delete pMsg;
	
	if (status != OK)
		TRACEF(TRACE_L3, ("\n***DdmBootMgr::BlinkLEDs(). Status = 0x%x.\n",status));

	// Find an Iop to ping.
	for (int i = 0; i < m_nIopStatusTableRows; i++)
		if ((pBootMgr->m_pIopStatusTableRows[i].eIOPCurrentState == IOPS_OPERATING))
			if (m_aIopManagers[i])
				m_aIopManagers[i]->PingIop();
	return OK;
}
#else
void DdmBootMgr::BlinkLEDs(void *pContext)
{
	DdmBootMgr*	pBootMgr = (DdmBootMgr*)pContext;

//	TRACEF(TRACE_L8, ("\n***DdmBootMgr::BlinkLEDs().\n"));

	// Find an Iop to ping.
	for (int i = 0; i < pBootMgr->m_nIopStatusTableRows; i++)

		if (pBootMgr->m_pIopStatusTableRows[i].eIOPCurrentState == IOPS_OPERATING)
			#if false
			if ((pBootMgr->m_SystemStatusRecord.PresentIOPsMask & (1 << pBootMgr->m_pIopStatusTableRows[i].Slot)) &&
			 	(pBootMgr->m_SystemStatusRecord.ActiveIOPsMask & (1 << pBootMgr->m_pIopStatusTableRows[i].Slot)))
			#endif
				if (pBootMgr->m_aIopManagers[i])
					 pBootMgr->m_aIopManagers[i]->PingIop();
}
#endif

//
//  DdmBootMgr::ShutDown()
//
//  Description:
// 	  Shuts down an Iop.
//
//  Inputs:
//
//  Outputs:
//	  status - returns OK
//
STATUS DdmBootMgr::ShutDown()
{
	if (m_pStartTimerMsg)
		/* Disable the timer */
		m_pStartTimerMsg->Disable();

	return OK;
}

#pragma mark ### Listener for Command Queue ###

//  DdmBootMgr::ListenerForCommands ()
//
//  Description:
//     This is called when any CmdSender objects calls its Execute method.
//     This allows the BootManager to receive commands from SSAPI and 
//     act upon them
//
//  Inputs:
//	  HANDLE handle - handle to indentify request to command sender
//    void* CmdData - command inputs
//
//  Outputs:
//
void DdmBootMgr::ListenerForCommands(HANDLE handle, void* pCmdData_)
{
	TRACE_ENTRY(DdmBootMgr::ListenerForCommands(HANDLE, void*));

	STATUS status = OK;
	
	BMGRRequest* pCmdData = (BMGRRequest*)pCmdData_;

	// allocate and initialize context
	CONTEXT* pContext = new (tZERO) CONTEXT;
	pContext->handle = handle;
	pContext->pCmdInfo =  pCmdData;

	switch (pCmdData->eCommand) {
	case k_eIOPTakeOutOfService:
		SubmitRequest(function_TakeIopOutOfSvc, pContext);
		break;
	case k_eIOPPowerOn:
		SubmitRequest(function_PowerIopOn, pContext);
		break;
	case k_eIOPIntoService:
		SubmitRequest(function_PutIopIntoSvc, pContext);
		break;
	case k_eIOPLock:
		SubmitRequest(function_LockIop, pContext);
		break;
	case k_eIOPUnlock:
		SubmitRequest(function_UnlockIop, pContext);
		break;
	default:
		assert(0);
		break;
	}
}
/* end DdmBootMgr::ListenerForCommands  ************************************/

#pragma mark ### Message Handler ###

//  DdmBootMgr::ProcessBootMgrMessages ()
//
//  Description:
//		This adds the message to the command processing queue if a message
//		is currently being processed, or it ispatched it to the appropriate
//		method if the upgrade master is idle.
//
//  Inputs:
//		pMsgReq - incoming message
//
//  Outputs:
//    Returns OK 
//

STATUS DdmBootMgr::ProcessBootMgrMessages(Message *pMsgReq) 
{

	TRACE_ENTRY(DdmBootMgr::ProcessBootMgrMessages());

	// allocate and initialize context
	CONTEXT* pContext = new (tZERO) CONTEXT;
	pContext->pMsg = pMsgReq;
	
	// queue request based upon the message code
	switch (pContext->pMsg->reqCode)
	{
	case REQ_IOP_OUT_OF_SERVICE:
		SubmitRequest(function_TakeIopOutOfSvc, pContext);
		break;
	case REQ_IOP_POWER_ON:
		SubmitRequest(function_PowerIopOn, pContext);
		break;
	case REQ_IOP_INTO_SERVICE:
		SubmitRequest(function_PutIopIntoSvc, pContext);
		break;
	default:
		assert(0);
		break;
	}

	return OK;
}
/* end DdmBootMgr::ProcessBootMgrMessages  ***********************************************/

#pragma mark ### Internal Command Queue Processing ###

//
//  DdmBootMgr::SubmitRequest
//
//  Description:
//		This function either enqueues a new function call
//		entry or makes the call if the queue is presently
//		empty and the Boot Manager is not processing a command
//
//  Inputs:
//	  U32 functionToCall - enumerated value of function to call
//    void* pContext - information passed from the calling DdmUpgrade
//      method
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
void DdmBootMgr::SubmitRequest(U32 functionToCall, CONTEXT* pContext)
{
	if (m_pBootMgrQueue->Empty() && NotProcessingCommand())
		ExecuteFunction(functionToCall, pContext);
	else
		m_pBootMgrQueue->AddFunctionCall(functionToCall, ((void*)pContext));
}
/* end DdmBootMgr::SubmitRequest  ********************************/

//
//  DdmBootMgr::ExecuteFunction
//
//  Description:
//		Sets the ProcessingCommand flag in the Boot Manager and
//		calls the appropriate function
//
//  Inputs:
//    U32 functionToCall - enumerated value which specifies 
//		function to call
//    void* _pContext - information passed from the calling DdmUpgrade
//      method
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
void DdmBootMgr::ExecuteFunction(U32 functionToCall, CONTEXT* pContext)
{
	ProcessCommand();
	switch (functionToCall)
	{
	case function_TakeIopOutOfSvc:
		HandleTakeIopOutOfService(pContext);
		break;
	case function_PutIopIntoSvc:
		HandlePutIopIntoService(pContext);
		break;
	case function_PowerIopOn:
		HandlePowerOnIop(pContext);
		break;
	case function_LockIop:
		HandleLockUnlockIop(pContext);
		break;
	case function_UnlockIop:
		HandleLockUnlockIop(pContext);
		break;
	default:
		assert(0);
		break;
	}
}
/* end DdmBootMgr::ExecuteFunction  ********************************/

//
//  DdmBootMgr::ProcessNextCommand
//
//  Description:
//		Checks the queue for next function call and executes
//		or sets the Boot Manager to a non-busy state if there
//		are no more commands to process
//
//	Inputs:
//
//	Outputs:
//
void DdmBootMgr::ProcessNextCommand()
{
	FunctionCallElement* pElement;

	pElement = m_pBootMgrQueue->PopFunctionCall();

	if (pElement)
	{
		ExecuteFunction(pElement->GetFunction(), (CONTEXT*)pElement->GetContext());
		delete pElement;
	}
	else
		FinishCommand();
}
/* end DdmBootMgr::ProcessNextCommand  ********************************/

#pragma mark ### Take Iop Out Of Service ###

//
//  DdmBootMgr::HandleTakeIopOutOfService
//
//  Description:
//	  Handles a received message or command requesting an Iop be taken
//	  out of service.  Tells all other Iops not to transport to the Iop going
// 	  out of service and then sends a messgae to ourselves to tell the 
//	  IopManger to take the Iop down.  The IopManager will reply to 
//	  HandleReqiopOutOfServiceIntReply when the Iop is offline at
//	  which point we will tell the other Iops to try any outstanding msgs
//	  for the downed Iop with it's failover partner
//
//  Inputs:
//	  pContext - holds message or command
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmBootMgr::HandleTakeIopOutOfService(CONTEXT* pContext)
{
	TRACE_ENTRY(DdmBootMgr::HandleTakeIopOutOfService);

	IOPStatusRecord* pIopStatusRecord = NULL;
	
	// assign current message to appropriate members
	m_pMsgIopOutOfSvc = (MsgIopOutOfService*) pContext->pMsg;
	m_pCmdIopOutOfSvc = (BMGRRequest*)pContext->pCmdInfo;
	m_pHandleIopOutOfSvc = pContext->handle;

	// extract slot from request
	TySlot slot;
	if (m_pMsgIopOutOfSvc)
		slot = m_pMsgIopOutOfSvc->GetSlot();
	else
	{
		assert(m_pCmdIopOutOfSvc);
		slot = m_pCmdIopOutOfSvc->GetSlot();
	}
	
	// find the IopStatusTable row associated with this slot
	for (int i = 0; i < m_nIopStatusTableRows; i++)
	{
		if (m_pIopStatusTableRows[i].Slot == slot)
		{
			pIopStatusRecord = &m_pIopStatusTableRows[i];
			break;
		}
	}
	
	// If we don't find an IOP Status record associated with the
	// requested slot, something is wrong.  Return error.
	if (pIopStatusRecord==NULL)
	{
		if (m_pMsgIopOutOfSvc)
		{
			Reply(m_pMsgIopOutOfSvc,CTS_IOP_NO_SUCH_SLOT);
			m_pMsgIopOutOfSvc = NULL;
		}
		else
		{
			m_pCmdServer->csrvReportCmdStatus(
				m_pHandleIopOutOfSvc,
				CTS_IOP_NO_SUCH_SLOT,
				0,
				0,
				m_pCmdIopOutOfSvc,
				BMGR_CONTROL_COMMAND_SIZE);
			m_pHandleIopOutOfSvc = NULL;
			m_pCmdIopOutOfSvc = NULL;
		}
		return CTS_IOP_NO_SUCH_SLOT;

	}
	
	// I'm thinking that the IOP Manager can't be in the middle of any
	// state sequence or else it may undo what we're gonna ask it to do 
	// in the shutdown sequence.  Sooo...
	
	// Disallow the Out Of Service operation from any state except...
	// IOPS_OPERATIING
	if (pIopStatusRecord->eIOPCurrentState != IOPS_OPERATING)
	{
		if (m_pMsgIopOutOfSvc != NULL)
		{
			Reply(m_pMsgIopOutOfSvc, CTS_IOP_OOS_STATE_NONO);
			m_pMsgIopOutOfSvc = NULL;
		}
		else
		{
			m_pCmdServer->csrvReportCmdStatus(
				m_pHandleIopOutOfSvc,
				CTS_IOP_OOS_STATE_NONO,
				0,
				0,
				m_pCmdIopOutOfSvc,
				BMGR_CONTROL_COMMAND_SIZE);
			m_pHandleIopOutOfSvc = NULL;
			m_pCmdIopOutOfSvc = NULL;
		}
		return CTS_IOP_OOS_STATE_NONO;
	}
			
	// Setup a count of quiesced transports.
	m_iSlotCount = 0;

	// Send other IOP's Transport Service a msg to stop Transporting to
	// the IOP going out of service and keep track of how many we sent.  
	// As replies are handled in StopTransportsToIopReply we'll decrement
	// the count and when it's zero all IOPs have stopped Transport to the
	// IOP going out of service.  Then and only then will we take the IOP
	// out of service.
	int i;
	for (i = 0; i < m_nIopStatusTableRows; i++)
		if ((m_pIopStatusTableRows[i].Slot != slot) &&
			(m_pIopStatusTableRows[i].eIOPCurrentState == IOPS_OPERATING))
		{
			m_iSlotCount++;
			RqOsTransportIopStop* prqStopTransport = new RqOsTransportIopStop(slot);
		    Send(m_pIopStatusTableRows[i].Slot, prqStopTransport, 
		    	REPLYCALLBACK(DdmBootMgr, StopTransportsToIopReply));
		}
	
	return OK;	
}

//
//  DdmBootMgr::StopTransportsToIopReply
//
//  Description:
//	  Handles replies from Transport services on Iops to our msg 
//	  telling them to stop transport to the Iop going out of service.
//
//  Inputs:
//	  pReply - reply from RqOsTransportIopStop
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmBootMgr::StopTransportsToIopReply(Message* pReply)
{
	TRACE_ENTRY(DdmBootMgr::StopTransportsToIopReply);

	assert (pReply != NULL);
	
	STATUS status = pReply->Status();
	
	if (status !=OK)
	{
		TRACEF(TRACE_L3, ("\n***DdmBootMgr:StopTransportsToIopReply received the following message with status = 0x%x.",status));
		pReply->Dump("Message:");
		delete pReply;
		if (m_pMsgIopOutOfSvc != NULL)
		{
			Reply(m_pMsgIopOutOfSvc, CTS_IOP_UNABLE_TO_STOP_TRANSPORT);
			m_pMsgIopOutOfSvc = NULL;
		}
		else
		{
			assert(m_pHandleIopOutOfSvc);
			m_pCmdServer->csrvReportCmdStatus(
				m_pHandleIopOutOfSvc,
				CTS_IOP_UNABLE_TO_STOP_TRANSPORT,
				0,
				0,
				m_pCmdIopOutOfSvc,
				BMGR_CONTROL_COMMAND_SIZE);
			m_pHandleIopOutOfSvc = NULL;
			m_pCmdIopOutOfSvc = NULL;
		}
		return CTS_IOP_UNABLE_TO_STOP_TRANSPORT;
	}  
	
	// if m_iSlotCount == 0, something has gone wrong with our logic.
	assert (m_iSlotCount > 0);

	// Decrement the count of outstanding Transport Stop Msgs.  When it's
	// zero all IOPs have stopped Transport to the IOP going out of service.
	// Then and only then can we actually take the IOP out of service.
	if (--m_iSlotCount == 0)
	{
		// Send ourselves a msg to take the IOP out of service.  We do this 
		// so that we get a reply when the operation is complete.  If we just
		// invoke the IopManager directly we would never get the reply.  
		TySlot slot;
		if (m_pMsgIopOutOfSvc)
			slot = m_pMsgIopOutOfSvc->GetSlot();
		else
		{
			assert(m_pCmdIopOutOfSvc);
			slot = m_pCmdIopOutOfSvc->GetSlot();
		}
			
		MsgIopOutOfServiceInt* pMsgIopOutOfServiceInt = 
    		new MsgIopOutOfServiceInt(slot);
    	Send(pMsgIopOutOfServiceInt,
    		REPLYCALLBACK(DdmBootMgr, HandleReqIopOutOfServiceIntReply));
	}
		
    delete pReply;
	return status;
}

//
//  DdmBootMgr::HandleReqIopOutOfServiceInt
//
//  Description:
//	  Our own internal method to take an IOP out of service.
//
//  Inputs:
//	  pMsg - internal out of service message
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmBootMgr::HandleReqIopOutOfServiceInt(Message *pMsg)
{
	TRACE_ENTRY(DdmBootMgr::HandleReqIopOutOfServiceInt);

	MsgIopOutOfServiceInt*	pRqIopOOSInt = (MsgIopOutOfServiceInt*)pMsg;
	TySlot					slot = pRqIopOOSInt->GetSlot();
	
	// Disable the IOPs LED Ping
	// status = m_aIopManagers[i]->DisablePingIop();
	// Unclear such a task is necessary.

	// Start Out Of Service state sequence processing by the IOPManager.
	// need to find the appropriate IopManger for this slot
	IopManager* pIopManager = NULL;
	for (int i = 0; i < m_nIopStatusTableRows; i++)
	{
		if (m_pIopStatusTableRows[i].Slot == slot)
		{
			pIopManager = m_aIopManagers[i];
			break;
		}
	}
	assert (pIopManager);

	return pIopManager->HandleReqIopOutOfService(pRqIopOOSInt);
};

//
//  DdmBootMgr::HandleReqIopOutOfServiceIntReply
//
//  Description:
//	  Our reply method called back after our own internal message to take
//	  an Iop out of service has completed.  This method will restart the
//	  transports previously quiesced on other Iops.
//
//  Inputs:
//	  pMsg - internal out of service message reply
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmBootMgr::HandleReqIopOutOfServiceIntReply(Message *pReply)
{	
	TRACE_ENTRY(DdmBootMgr::HandleReqIopOutOfServiceIntReply);
	
	STATUS	status = pReply->Status();

	if (status != OK)
	{
		TRACEF(TRACE_L3, ("\n***DdmBootMgr::HandleReqIopOutOfServiceIntReply() received a message with status 0x%x.\n",status));
		delete pReply;
		if (m_pMsgIopOutOfSvc != NULL)
		{
			Reply(m_pMsgIopOutOfSvc, status);
			m_pMsgIopOutOfSvc = NULL;
		}
		else
		{
			assert(m_pHandleIopOutOfSvc);
			m_pCmdServer->csrvReportCmdStatus(
				m_pHandleIopOutOfSvc,
				status,
				0,
				0,
				m_pCmdIopOutOfSvc,
				BMGR_CONTROL_COMMAND_SIZE);
			m_pHandleIopOutOfSvc = NULL;
			m_pCmdIopOutOfSvc = NULL;
		}
		return status;
	}

	// extract slot we took out of service from internal message
	MsgIopOutOfServiceInt*	pRqIopOOSInt = (MsgIopOutOfServiceInt*)pReply;
	TySlot					slot = pRqIopOOSInt->GetSlot();
	delete pRqIopOOSInt;

	// Send other IOP's Transport Servicse a msg to restart Transport to
	// the IOP out of service and keep track of how many we sent.  
	// As replies are handled in StartransportsToIopReply we'll decrement
	// the count and when it's zero all IOPs have restarted Transport to the
	// IOP.
	int i;
	for (i = 0; i < m_nIopStatusTableRows; i++)
		if ((m_pIopStatusTableRows[i].Slot != slot) &&
			(m_pIopStatusTableRows[i].eIOPCurrentState == IOPS_OPERATING))
		{
			m_iSlotCount++;
			RqOsTransportIopStart* prqStartTransport = 
		    	new RqOsTransportIopStart(slot);
		    Send(m_pIopStatusTableRows[i].Slot, prqStartTransport, 
		    	REPLYCALLBACK(DdmBootMgr, StartTransportsToIopReply));
		}
		    
	return status;	
};

//
//  DdmBootMgr::StartTransportsToIopReply
//
//  Description:
//	  Handles replies from transport services on Iops to our msg
//	  telling them to retry transports to the Iop gone out of services's
//	  failover.
//
//  Inputs:
//	  pMsg - reply from transport
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmBootMgr::StartTransportsToIopReply(Message* pReply)
{
	TRACE_ENTRY(DdmBootMgr::StartTransportsToIopReply);	
	
	STATUS	status = pReply->Status();

	if (status != OK)
	{
		TRACEF(TRACE_L3, ("\n***DdmBootMgr:StartTransportsToIopReply received the following message with status = 0x%x.",status));
		pReply->Dump("Message:");
		delete pReply;
		if (m_pMsgIopOutOfSvc != NULL)
		{
			Reply(m_pMsgIopOutOfSvc, CTS_IOP_UNABLE_TO_RESTART_TRANSPORT);
			m_pMsgIopOutOfSvc = NULL;
		}
		else
		{
			assert(m_pHandleIopOutOfSvc);
			m_pCmdServer->csrvReportCmdStatus(
				m_pHandleIopOutOfSvc,
				CTS_IOP_UNABLE_TO_RESTART_TRANSPORT,
				0,
				0,
				m_pCmdIopOutOfSvc,
				BMGR_CONTROL_COMMAND_SIZE);
			m_pHandleIopOutOfSvc = NULL;
			m_pCmdIopOutOfSvc = NULL;
		}
		return CTS_IOP_UNABLE_TO_RESTART_TRANSPORT;
	}  
	
	delete pReply;

	// if m_iSlotCount == 0, then something has logically gone wrong
	assert (m_iSlotCount > 0);

	// Decrement the count of outstanding Transport Start Msgs.  When it's
	// zero all IOPs have retried Transports to the IOP going out of service.
	// Then and only then can we reply to the IOP out of service message.
	if (--m_iSlotCount == 0)
	{
		// Finally,we can reply to the original message or command.
		if (m_pMsgIopOutOfSvc != NULL)
		{
			Reply(m_pMsgIopOutOfSvc, OK);
			m_pMsgIopOutOfSvc = NULL;
		}
		else
		{
			assert(m_pHandleIopOutOfSvc);
			m_pCmdServer->csrvReportCmdStatus(
				m_pHandleIopOutOfSvc,
				OK,
				0,
				0,
				m_pCmdIopOutOfSvc,
				BMGR_CONTROL_COMMAND_SIZE);
			BMGREvent* pEvt = new BMGREvent(k_eIOPTakeOutOfService, OK,
				m_pCmdIopOutOfSvc->GetSlot());
			m_pCmdServer->csrvReportEvent(
					k_eIOPTakeOutOfService,
					pEvt,
					BMGR_CONTROL_STATUS_SIZE);
			m_pHandleIopOutOfSvc = NULL;
			m_pCmdIopOutOfSvc = NULL;
		}		
	}
			
	return status;
}

#pragma mark ### Power On Iop ###

//
//  DdmBootMgr::HandlePowerOnIop
//
//  Description:
//	  Handle a received message or command requesting an Iop be
//	  powered on.
//
//  Inputs:
//	  pContext - context containing requesting message or command
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmBootMgr::HandlePowerOnIop(CONTEXT* pContext)
{
	TRACE_ENTRY(DdmBootMgr::HandlePowerOnIop);

	IOPStatusRecord* pIopStatusRecord = NULL;
	IopManager* pIopManager = NULL;
	STATUS status;
	
	// assign current message to appropriate members
	m_pMsgIopPowerOn = (MsgIopPowerOn*) pContext->pMsg;
	m_pCmdIopPowerOn = (BMGRRequest*)pContext->pCmdInfo;
	m_pHandleIopPowerOn = pContext->handle;

	// extract slot from message or command
	TySlot slot;
	if (m_pMsgIopPowerOn)
		slot = m_pMsgIopPowerOn->GetSlot();
	else
	{
		assert(m_pCmdIopPowerOn);
		slot = m_pCmdIopPowerOn->GetSlot();
	}

	// need to find the appropriate IopStatusTable row associated with
	// this slot
	for (int i = 0; i < m_nIopStatusTableRows; i++)
	{
		if (m_pIopStatusTableRows[i].Slot == slot)
		{
			pIopStatusRecord = &m_pIopStatusTableRows[i];
			pIopManager = m_aIopManagers[i];
			break;
		}
	}

	// If we don't find an IOP Status record associated with the
	// requested slot, something is wrong.  Return error.
	if (pIopStatusRecord==NULL)
	{
		if (m_pMsgIopPowerOn)
		{
			Reply(m_pMsgIopPowerOn,CTS_IOP_NO_SUCH_SLOT);
			m_pMsgIopPowerOn = NULL;
		}
		else
		{
			m_pCmdServer->csrvReportCmdStatus(
				m_pHandleIopPowerOn,
				CTS_IOP_NO_SUCH_SLOT,
				0,
				0,
				m_pCmdIopPowerOn,
				BMGR_CONTROL_COMMAND_SIZE);
			m_pHandleIopPowerOn = NULL;
			m_pCmdIopPowerOn = NULL;
		}
		return CTS_IOP_NO_SUCH_SLOT;
	}
	
	// Disallow the Power On operation from any state except...
	// IOPS_POWERED_DOWN
	if (pIopStatusRecord->eIOPCurrentState != IOPS_POWERED_DOWN)
	{
		if (m_pMsgIopPowerOn != NULL)
		{
			Reply(m_pMsgIopPowerOn, CTS_IOP_POWERON_STATE_NONO);
			m_pMsgIopPowerOn = NULL;
			}
		else
		{
			assert(m_pHandleIopOutOfSvc);
			m_pCmdServer->csrvReportCmdStatus(
				m_pHandleIopPowerOn,
				CTS_IOP_POWERON_STATE_NONO,
				0,
				0,
				m_pCmdIopPowerOn,
				BMGR_CONTROL_COMMAND_SIZE);
			m_pHandleIopPowerOn = NULL;
			m_pCmdIopPowerOn = NULL;
		}
		return CTS_IOP_POWERON_STATE_NONO;
	}
			
	// call Iop Manager to Power on Iop
	status = pIopManager->StartIop();
	
	status = Listen4IopOnPci(status, slot);
	
	return status;

}

//  DdmBootMgr::Listen4IopOnPci()
//
//  Description:
//    This method is called to Listen on the IOPOnPCIMask mask in the 
//	  system status table.
//
//  Inputs:
//    status - The returned status of the PTS operation.
//
//	  slot - the slot of the IOP to listen on
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmBootMgr::Listen4IopOnPci(STATUS status, TySlot slot)
{
	TRACE_ENTRY(DdmBootMgr::Listen4IopOnPci);
	TRACEF(TRACE_L3, ("\n***DdmBootMgr: Listen4IopOnPCI() status = 0x%x.\n",status));

	m_pListen4Iop = new TSTimedListen;

	U32 desiredIOPsOnPCIMask = ((m_SystemStatusRecord.IOPsOnPCIMask) | (1 << slot));

	if (!m_pListen4Iop)
		status = CTS_OUT_OF_MEMORY;
	else			
		// we are going to listen for the above slot to become present on the PCIMask
		status = m_pListen4Iop->Initialize( 
			this,											// DdmServices* pDdmServices
			ListenOnModifyOneRowOneField,					// U32 ListenType
			SYSTEM_STATUS_TABLE,							// String64 prgbTableName
			CT_PTS_RID_FIELD_NAME,							// String64 prgbRowKeyFieldName
			&m_SystemStatusRecord.rid,						// void* prgbRowKeyFieldValue
			sizeof(rowID),									// U32 cbRowKeyFieldValue
			CT_SYSST_IOPSONPCIMASK,							// String64 prgbFieldName
			&desiredIOPsOnPCIMask,							// void* prgbFieldValue
			sizeof(m_SystemStatusRecord.IOPsOnPCIMask),		// U32 cbFieldValue
			ReplyOnceOnly | ReplyWithRow,					// U32 ReplyMode
			NULL,											// void** ppTableDataRet,
			NULL,											// U32* pcbTableDataRet,
			&m_ListenerID,									// U32* pListenerIDRet,
			&m_pListenReplyType,							// U32** ppListenTypeRet,
			&m_pNewSystemStatusRecord,						// void** ppModifiedRecordRet,
			&m_cbNewSystemStatusRecord,						// U32* pcbModifiedRecordRet,
			TSCALLBACK(DdmBootMgr, IopsOnPciListenReply),	// pTSCallback_t pCallback,
			60000000,										// U32 iTimeOut, 60 secs.
			NULL											// void* pContext
		);
	
	if (status == OK)
		m_pListen4Iop->Send();
	else
	{
		if (m_pMsgIopPowerOn)
		{
			Reply(m_pMsgIopPowerOn,status);
			m_pMsgIopPowerOn = NULL;
		}
		else
		{
			m_pCmdServer->csrvReportCmdStatus(
				m_pHandleIopPowerOn,
				status,
				0,
				0,
				m_pCmdIopPowerOn,
				BMGR_CONTROL_COMMAND_SIZE);
			m_pHandleIopPowerOn = NULL;
			m_pCmdIopPowerOn = NULL;
		}
		return status;
	}
	
	return OK;
}


//  DdmBootMgr::IopOnPciListenReply()
//
//  Description:
//    This method is called back by Listen when the System Status Table's
//	  IopsOnPciMask comes equal to the PresentIopsMask.
//
//  Inputs:
//    pClientContext - unused.
//
//    status - The returned status of the PTS listen operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmBootMgr::IopOnPciListenReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmBootMgr::IopOnPciListenReply);

	// If it is the initial reply ignore it.
	if (m_pListenReplyType && (*m_pListenReplyType == ListenInitialReply))
	{
		TRACEF(TRACE_L3,("\n***BootMgr: Received Initial Listen Reply for IOPOnPCIMask.  Status = 0x%x.\n",
				status ));
		return status;
	}	

	// If the Timer we set has already gone off then we have already aborted
	// The only thing we have to do is free the reply type buffer returned to us.  
	if (m_pListenReplyType && (*m_pListenReplyType == ListenReturnedOnStopListen))
	{
		TRACEF( TRACE_L3, ("\n***BootMgr: Received Stop Listen Reply for IOPOnPCIMask.  Status = 0x%x.\n", status ));
		return OK;		
	};
	
	if (status == CTS_PTS_LISTEN_TIMED_OUT)
	{
		TRACEF(TRACE_L3, ("\n***BootMgr: TIMED OUT Listening on IOPOnPCIMask.  Status = 0x%x.\n", status));
		if (m_pMsgIopPowerOn)
		{
			Reply(m_pMsgIopPowerOn,CTS_IOP_POWERON_TIMED_OUT);
			m_pMsgIopPowerOn = NULL;
		}
		else
		{
			m_pCmdServer->csrvReportCmdStatus(
				m_pHandleIopPowerOn,
				CTS_IOP_POWERON_TIMED_OUT,
				0,
				0,
				m_pCmdIopPowerOn,
				BMGR_CONTROL_COMMAND_SIZE);
			m_pHandleIopPowerOn = NULL;
			m_pCmdIopPowerOn = NULL;
		}
		return CTS_IOP_POWERON_TIMED_OUT;	
	}
	else
	if (status != OK)
	{
		TRACEF(TRACE_L3,("\n***BootMgr: Received BAD Listen Reply for IOPOnPCIMask.  Status = 0x%x.\n",
				status ));		
		if (m_pMsgIopPowerOn)
		{
			Reply(m_pMsgIopPowerOn,status);
			m_pMsgIopPowerOn = NULL;
		}
		else
		{
			m_pCmdServer->csrvReportCmdStatus(
				m_pHandleIopPowerOn,
				status,
				0,
				0,
				m_pCmdIopPowerOn,
				BMGR_CONTROL_COMMAND_SIZE);
			m_pHandleIopPowerOn = NULL;
			m_pCmdIopPowerOn = NULL;
		}
		return status;
	}
	else
	{
		TRACEF(TRACE_L3,("\n***BootMgr: Received Listen for IOPOnPCIMask.  Status = 0x%x.\n", status));
		m_fAllIopsOnPciFlag = TRUE;
		
		// We need to reinit the bridges after powering on and connecting all the IOPs.
		// commented out because it breaks the net
		// status = InitBridgeFTree();
		// TRACEF(TRACE_L3,("\n***BootMgr: InitBridgeFTree() returned Status = 0x%x.\n", status ));	
	
		// We need to replace our SystemStatusRecord with the updated one returned by the Listen.
		m_SystemStatusRecord = *m_pNewSystemStatusRecord;
	
		// Finally,we can reply to the original message or command.
		if (m_pMsgIopPowerOn != NULL)
		{
			Reply( m_pMsgIopPowerOn, OK );
			m_pMsgIopPowerOn = NULL;
		}
		else
		{
			assert(m_pHandleIopPowerOn);
			m_pCmdServer->csrvReportCmdStatus(
				m_pHandleIopPowerOn,
				OK,
				0,
				0,
				m_pCmdIopPowerOn,
				BMGR_CONTROL_COMMAND_SIZE);
			BMGREvent* pEvt = new BMGREvent(k_eIOPPowerOn, OK,
				m_pCmdIopPowerOn->GetSlot());
			m_pCmdServer->csrvReportEvent(
					k_eIOPTakeOutOfService,
					pEvt,
					BMGR_CONTROL_STATUS_SIZE);
			m_pHandleIopPowerOn = NULL;
			m_pCmdIopPowerOn = NULL;
		}	
		return OK;	
	}
}
		
#pragma mark ### Put Iop Into Service ###

//
//  DdmBootMgr::HandlePutIopIntoService
//
//  Description:
//	  Handle a received message or command requesting an Iop be
//	  put into service.
//
//  Inputs:
//	  pContext - context containing requesting message or command
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmBootMgr::HandlePutIopIntoService(CONTEXT* pContext)
{
	TRACE_ENTRY(DdmBootMgr::HandlePutIopIntoService);
	
	IOPStatusRecord* pIopStatusRecord = NULL;
	
	// assign current message to local members
	m_pMsgIopIntoSvc = (MsgIopIntoService*) pContext->pMsg;
	m_pCmdIopIntoSvc = (BMGRRequest*)pContext->pCmdInfo;
	m_pHandleIopIntoSvc = pContext->handle;

	// extract slot and image key from the current message or command
	TySlot slot;
	RowId imageKey;
	if (m_pMsgIopIntoSvc)
	{
		slot = m_pMsgIopIntoSvc->GetSlot();
		imageKey = m_pMsgIopIntoSvc->GetImageKey();
	}
	else
	{
		assert(m_pCmdIopIntoSvc);
		slot = m_pCmdIopIntoSvc->GetSlot();
		imageKey = 0;
	}

	// need to find the appropriate IopStatusTable row associated with
	// this slot
	for (int i = 0; i < m_nIopStatusTableRows; i++)
	{
		if (m_pIopStatusTableRows[i].Slot == slot)		{
			pIopStatusRecord = &m_pIopStatusTableRows[i];
			break;
		}
	}

	// If we don't find an IOP Status record associated with the
	// requested slot, something is wrong.  Return error.
	if (pIopStatusRecord==NULL)
	{
		if (m_pMsgIopPowerOn)
		{
			Reply(m_pMsgIopIntoSvc,CTS_IOP_NO_SUCH_SLOT);
			m_pMsgIopPowerOn = NULL;
		}
		else
		{
			m_pCmdServer->csrvReportCmdStatus(
				m_pHandleIopIntoSvc,
				CTS_IOP_NO_SUCH_SLOT,
				0,
				0,
				m_pCmdIopIntoSvc,
				BMGR_CONTROL_COMMAND_SIZE);
			m_pHandleIopIntoSvc = NULL;
			m_pCmdIopIntoSvc = NULL;
		}
		return CTS_IOP_NO_SUCH_SLOT;
	}
	
	// Disallow the Into Service operation from any state except...
	// IOPS_POWERED_ON
	if (pIopStatusRecord->eIOPCurrentState != IOPS_POWERED_ON)
	{
		if (m_pMsgIopIntoSvc != NULL)
		{
			Reply(m_pMsgIopIntoSvc, CTS_IOP_ITS_STATE_NONO);
			m_pMsgIopIntoSvc = NULL;
		}
		else
		{
			assert(m_pHandleIopIntoSvc);
			m_pCmdServer->csrvReportCmdStatus(
				m_pHandleIopIntoSvc,
				CTS_IOP_ITS_STATE_NONO,
				0,
				0,
				m_pCmdIopIntoSvc,
				BMGR_CONTROL_COMMAND_SIZE);
			m_pHandleIopIntoSvc = NULL;
			m_pCmdIopIntoSvc = NULL;
		}
		return CTS_IOP_ITS_STATE_NONO;
	}
			

	// send internal message and then call Iop Manager to boot
	MsgIopIntoServiceInt* pMsg = new MsgIopIntoServiceInt(slot, imageKey);
	
	Send(pMsg, REPLYCALLBACK(DdmBootMgr, HandleReqIopIntoServiceIntReply));
		
	return OK;	
}


//
//  DdmBootMgr::HandleReqIopIntoServiceInt
//
//  Description:
//	  Handles our own internal method to put an IOP into service
//
//  Inputs:
//	  pMsg - internal message
//    
//  Outputs:
//    STATUS - returns status from IopManager request
//
STATUS DdmBootMgr::HandleReqIopIntoServiceInt(Message *pMsg)
{
	TRACE_ENTRY(DdmBootMgr::HandleReqIopIntoServiceInt);

	MsgIopIntoServiceInt*	pRqIopIntoSvcInt = (MsgIopIntoServiceInt*)pMsg;
	TySlot					slot = pRqIopIntoSvcInt->GetSlot();
	
	// Start Out Of Service state sequence processing by the IOPManager.
	IopManager* pIopManager = NULL;
	for (int i = 0; i < m_nIopStatusTableRows; i++)
	{
		if (m_pIopStatusTableRows[i].Slot == slot)
		{
			pIopManager = m_aIopManagers[i];
			break;
		}
	}
	assert (pIopManager);

	return pIopManager->HandleReqIopIntoService(pRqIopIntoSvcInt);
}

//
//  DdmBootMgr::HandleReqIopIntoServiceIntReply
//
//  Description:
//	  Call back for our own internal method to put an IOP into service.
//	  Replies to the originating message or command.
//
//  Inputs:
//	  pReply - reply to internal message from IopManger
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmBootMgr::HandleReqIopIntoServiceIntReply(Message *pReply)
{
	TRACE_ENTRY(DdmBootMgr::HandleReqIopIntoServiceIntReply);

	STATUS status = pReply->Status();

	if (status != OK)
		TRACEF(TRACE_L3, ("\n***DdmBootMgr::HandleReqIopIntoServiceIntReply() received a message with status 0x%x.\n",status));
			
	// Finally,we can reply to the original message or command.
	if (m_pMsgIopIntoSvc != NULL)
	{
		Reply( m_pMsgIopIntoSvc, status );
		m_pMsgIopIntoSvc = NULL;
	}
	else
	{
		assert(m_pHandleIopIntoSvc);
		m_pCmdServer->csrvReportCmdStatus(
			m_pHandleIopIntoSvc,
			status,
			0,
			0,
			m_pCmdIopIntoSvc,				
			BMGR_CONTROL_COMMAND_SIZE);
		BMGREvent* pEvt = new BMGREvent(k_eIOPIntoService, OK,
			m_pCmdIopIntoSvc->GetSlot());
		m_pCmdServer->csrvReportEvent(
				k_eIOPTakeOutOfService,
				pEvt,
				BMGR_CONTROL_STATUS_SIZE);
		m_pHandleIopIntoSvc = NULL;
		m_pCmdIopIntoSvc = NULL;
	}	
	return status;	
}

#pragma mark ### Lock Iop ###

//
//  DdmBootMgr::HandleLockUnlockIop
//
//  Description:
//	  Handle a received command requesting an IOP be locked or 
//	  unlocked.
//
//  Inputs:
//	  pContext - contains requesting command and handle
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmBootMgr::HandleLockUnlockIop(CONTEXT* pContext)
{
	TRACE_ENTRY(DdmBootMgr::HandleLockUnlockIop);

	IOPStatusRecord* pIopStatusRecord = NULL;

	// assign current command to local members
	m_pCmdIopLockUnlock = (BMGRRequest*)pContext->pCmdInfo;
	m_pHandleIopLockUnlock = pContext->handle;

	// extract the slot from the cmd info
	TySlot slot;
	assert(m_pCmdIopLockUnlock);
	slot = m_pCmdIopLockUnlock->GetSlot();

	// need to find the appropriate IopStatusTable row associated with
	// this slot
	for (int i = 0; i < m_nIopStatusTableRows; i++)
	{
		if (m_pIopStatusTableRows[i].Slot == slot)		{
			pIopStatusRecord = &m_pIopStatusTableRows[i];
			break;
		}
	}

	// If we don't find an IOP Status record associated with the
	// requested slot, something is wrong.  Return error.
	if (pIopStatusRecord==NULL)
	{
		m_pCmdServer->csrvReportCmdStatus(
			m_pHandleIopLockUnlock,
			CTS_IOP_NO_SUCH_SLOT,
			0,
			0,
			m_pCmdIopLockUnlock,
			BMGR_CONTROL_COMMAND_SIZE);
		m_pHandleIopLockUnlock = NULL;
		m_pCmdIopLockUnlock = NULL;
		return CTS_IOP_NO_SUCH_SLOT;
	}
		
	// Disallow the lock operation from any state except...
	// IOPS_UNLOCKED and disallow the unlock operation from
	// any state except...  IOPS_POWERED_DOWN
	if (m_pCmdIopLockUnlock->eCommand==k_eIOPLock)
	{
		if (pIopStatusRecord->eIOPCurrentState != IOPS_UNLOCKED)
		{
			m_pCmdServer->csrvReportCmdStatus(
				m_pHandleIopLockUnlock,
				CTS_IOP_LOCK_STATE_NONO,
				0,
				0,
				m_pCmdIopLockUnlock,
				BMGR_CONTROL_COMMAND_SIZE);
			m_pHandleIopLockUnlock = NULL;
			m_pCmdIopLockUnlock = NULL;
			return CTS_IOP_LOCK_STATE_NONO;
		}
	}
	else
	{
		assert(m_pCmdIopLockUnlock->eCommand==k_eIOPUnlock);
		if (pIopStatusRecord->eIOPCurrentState != IOPS_POWERED_DOWN)
		{
			m_pCmdServer->csrvReportCmdStatus(
				m_pHandleIopLockUnlock,
				CTS_IOP_UNLOCK_STATE_NONO,
				0,
				0,
				m_pCmdIopLockUnlock,
				BMGR_CONTROL_COMMAND_SIZE);
			m_pHandleIopLockUnlock = NULL;
			m_pCmdIopLockUnlock = NULL;
			return CTS_IOP_UNLOCK_STATE_NONO;
		}
	}
				
	// send internal message and then call Iop Manager to lock
	MsgIopLockUnlockInt* pMsg = new MsgIopLockUnlockInt(slot,
		(m_pCmdIopLockUnlock->eCommand==k_eIOPLock));
	
	Send(pMsg, REPLYCALLBACK(DdmBootMgr, HandleReqIopLockUnlockIntReply));
		
	return OK;	
}


//
//  DdmBootMgr::HandleReqIopLockUnlockInt
//
//  Description:
//	  Our own internal method to lock or unlock an Iop
//
//  Inputs:
//	  pMsg - internal message
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmBootMgr::HandleReqIopLockUnlockInt(Message *pMsg)
{
	TRACE_ENTRY(DdmBootMgr::HandleReqIopLockUnlockInt);

	MsgIopLockUnlockInt*	pRqIopLockUnlockInt = (MsgIopLockUnlockInt*)pMsg;
	TySlot					slot = pRqIopLockUnlockInt->GetSlot();
	
	// call appropriate IopManager to lock or unlock an IOP
	IopManager* pIopManager;
	for (int i = 0; i < m_nIopStatusTableRows; i++)
	{
		if (m_pIopStatusTableRows[i].Slot == slot)
		{
			pIopManager = m_aIopManagers[i];
			break;
		}
	}
	assert (pIopManager);
	
	if (pRqIopLockUnlockInt->Lock())
		return pIopManager->HandleReqIopLock(pRqIopLockUnlockInt);
	else
		return pIopManager->HandleReqIopUnlock(pRqIopLockUnlockInt);
	
}

//
//  DdmBootMgr::HandleReqIopLockUnlockIntReply
//
//  Description:
//	  Call back for our own internal method to lock or unlock an Iop
//
//  Inputs:
//	  pReply - reply to our internal message from the IopManager
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmBootMgr::HandleReqIopLockUnlockIntReply(Message *pReply)
{
	TRACE_ENTRY(DdmBootMgr::HandleReqIopLockUnlockIntReply);
	STATUS				status = pReply->Status();
	
	MsgIopLockUnlockInt* pRqIopLockUnlockInt = (MsgIopLockUnlockInt*) pReply;

	if (status != OK)
		TRACEF(TRACE_L3, ("\n***DdmBootMgr::HandleReqIopLockUnlockIntReply() received a message with status 0x%x.\n",status));
			
	// Finally,we can reply to the original message or command.
	assert(m_pHandleIopLockUnlock);
	m_pCmdServer->csrvReportCmdStatus(
		m_pHandleIopLockUnlock,
		status,
		0,
		0,
		m_pCmdIopLockUnlock,				
		BMGR_CONTROL_COMMAND_SIZE);
	BMGREvent* pEvt;
	if (pRqIopLockUnlockInt->Lock())
	{
		pEvt = new BMGREvent(k_eIOPLock, OK, m_pCmdIopLockUnlock->GetSlot());
		m_pCmdServer->csrvReportEvent(
			k_eIOPLock,
			pEvt,
			BMGR_CONTROL_STATUS_SIZE);
	}

	else
	{
		pEvt = new BMGREvent(k_eIOPUnlock, OK, m_pCmdIopLockUnlock->GetSlot());	
		m_pCmdServer->csrvReportEvent(
			k_eIOPUnlock,
			pEvt,
			BMGR_CONTROL_STATUS_SIZE);
	}
	m_pHandleIopLockUnlock = NULL;
	m_pCmdIopLockUnlock = NULL;
	
	return status;
}

// $Log: /Gemini/Odyssey/Hbc/DdmBootMgr.cpp $
// 
// 47    2/08/00 4:23p Jlane
// Increase ActiveIOP timeout to 5 minutes.
// 
// 46    1/31/00 4:17p Joehler
// Changes to correctly handle a persistent boot.
// 
// 45    1/26/00 2:40p Joehler
// Added Boot Manager Command Queue and Power On, Into Service, Lock and
// Unlock functionality.  Also added functionality to support Upgrade
// Master.
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
// Comment out InitBridgeFTree for now it breaks the net.
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
// E2 CMB Boot changes: Move Init_Iop into iopManager.
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
// Don't return in loop starting IopManagers.
// 
// 14    6/21/99 1:53p Ewedel
// Various changes to straighten out (I hope!) usage of member pointers to
// various message types.  Also, code now carefully NULLs such pointers
// after their respective messages have been deleted.
// 
// 13    6/21/99 1:29p Rkondapalli
// Changed "update Iop status" CMB DDM message to correct "poll all Iops."
// [jl/ew]
// Disabled init_hbc() call in DdmBootMgr::StartIops(). [jl?]
// 
// 12    6/16/99 5:46p Jlane
// Enhanced to work with IopManager class for CMB Boot.
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
