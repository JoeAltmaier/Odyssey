/* DdmRedundancyMgr.cpp -- Matches Ddm Classes into redundant pairs and
 *						creates VirtualClassDescriptorTable entries.
 *
 * Copyright (C) ConvergeNet Technologies, 1998 
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 // $Log: /Gemini/Odyssey/DdmRedundancyMgr/DdmRedundancyMgr.cpp $
// 
// 11    7/24/99 5:41p Jlane
// reslove PTS changes
// 
// 10    6/21/99 3:19p Rkondapalli
// Clearing m_cbDdmCDTInsertedRecord in constructor.  If this member is
// not initialized, SGL code in Chaos wipes out.
// 
// 9     6/18/99 2:27p Jlane
// INitialize member's no longer null lafter new.
// 
// 8     5/13/99 11:36a Cwohlforth
// Edits to support conversion to TRACEF
// 
// 7     4/21/99 9:38a Jlane
// Fixed call to Listen to use the new Listen parameters.
// 
// 6     4/06/99 11:24a Jlane
// added unused pragmas for unused args.
// 
// 5     3/30/99 9:22p Jlane
// Add new parameter to CLASSNAME Macro, Fix new allocations to use [] not
// ().
 //
 * Revision History:
 *     02/22/99 Jerry Lane	Created (first thing on a Monday morning).
 *
**/

#include "DdmRedundancyMgr.h"
#include "BuildSys.h"
#include "Table.h"
#include "Rows.h"
#include "Listen.h"
#include "ReadTable.h"


#define _DEBUG
#include "Odyssey_Trace.h"

// my ruler	
//345678901234567890123456789012345678901234567890123456789012345678901234567890


CLASSNAME(DdmRedundancyMgr, SINGLE);	// Class Link Name used by Buildsys.cpp


// DdmScc -- Constructor --------------------------------------DdmRedundancyMgr-
//
DdmRedundancyMgr::DdmRedundancyMgr(DID did): Ddm(did)
{
	Tracef("DdmRedundancyMgr::DdmRedundancyMgr()\n");
	m_pDdmCDTInsertedRecord = NULL;
		
	//  we must either clear record size, or set it to our target record size.
	//  Leaving it set to random uninitialized crud causes faults in SGL code.
	//  Being lazy, we opt for setting it to zero here.  Then PTS should return
	//  the correct record size in listen replies.
	m_cbDdmCDTInsertedRecord = 0;

	// I have no config area
//	SetConfigAddress(&config,sizeof(config));	// tell Ddm:: where my config area is
}


// DdmScc -- Destructor ---------------------------------------DdmRedundancyMgr-
//
DdmRedundancyMgr::~DdmRedundancyMgr()
{
	Tracef("DdmRedundancyMgr::DdmRedundancyMgr()\n");

	if (m_pListen)
	{
		// Stop listening by sending a stop listening message to the Table Service. 
		m_pListen->Stop();
		delete m_pListen;
	}
}

// Ctor -- Create ourselves -----------------------------------DdmRedundancyMgr-
//
Ddm *DdmRedundancyMgr::Ctor(DID did)
{
	return new DdmRedundancyMgr(did);
}

// .Initialize -- Just what it says ---------------------------DdmRedundancyMgr-
//
STATUS DdmRedundancyMgr::Initialize(Message *pMsg)	// virutal
{ 
	Tracef("DdmRedundancyMgr::Initialize()\n");

	#if 0
	// If we ever want to be able to register on command we'll need the following lines:
	Serve(DDM_REGISTRAR_REPORT_DDM_CLASSES,FALSE);
	DispatchRequest(DDM_REGISTRAR_REPORT_DDM_CLASSES, pServices, (RequestCallback) &VerifyDdmCDTExists);
	#endif
	
	// Kickoff the initialization process.
	VerifyIOPFailoverMapExists( NULL, OS_DETAIL_STATUS_SUCCESS ); 
	
	return Ddm::Initialize(pMsg);
}

// Enable -- Start-it-up --------------------------------------DdmRedundancyMgr-
//
STATUS DdmRedundancyMgr::Enable(Message *pMsg)	// virtual
{ 
	Tracef("DdmRedundancyMgr::Enable()\n");
	
	return Ddm::Enable(pMsg);
}


// VerifyIOPFailoverMapExists - Attempts to define the IOPFailoverMapTable.
// Called from Initialize, This is the beginning of the actual work of this Ddm.
// This method is the primary entry point for the IOPFailoverMap creation process.
// This method insures the existance of the IOPFailoverMapTable.  This is 
// done by simply defining the table.  If the table already exists, no harm will
// be done and if the table doesn't already exist then it will be defined.
// When initiating the DefineTable Operation this method specifies the method
// InitIOPFailoverMapTable as the callback function that method will continue the 
// rest of the IOP Failover Map Table creation process.  
STATUS DdmRedundancyMgr::VerifyIOPFailoverMapExists(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	// This is the code to create the IOPFailoverMapTable.
	
	// Allocate an Define Table object for the Virtual Class Descriptor Table.
	m_pDefineTable = new TSDefineTable;

	// Initialize the define Table object.
	status = m_pDefineTable->Initialize(
		this,											// DdmServices* pDdmServices,
		CT_IOPFM_TABLE_NAME,							// String64 prgbTableName,
		aIopFailoverMapTable_FieldDefs,					// fieldDef* prgFieldDefsRet,
		cbIopFailoverMapTable_FieldDefs,				// U32 cbrgFieldDefs,
		20,												// U32 cEntriesRsv,
		false,											// bool* pfPersistant,
		(pTSCallback_t)&InitIOPFailoverMapTable,		// pTSCallback_t pCallback,
		NULL											// void* pContext
	);
	
	// Initiate the define table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		m_pDefineTable->Send();

	return status;
}


// InitIOPFailoverMapTable -
// This method writes the IOP Failover Map.  
// When initiating the InsertRows Operation this method specifies the method
// VerifyVirtualCDTExists as the callback function that method will continue 
// with the Virtual Class Table creation process.  
STATUS DdmRedundancyMgr::InitIOPFailoverMapTable(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	// Declare and initialize a IOPFailoverMapTable with default values. 
	IOPFailoverMapRecord	newIOPFailoverMapTable[] = {
	//	rid.Tablew, rid.LoPart, rid.HiPart, Ver.,	Size,							PrimaryIOP, FailoverIOP
		0,			0,			0,			1,		sizeof(IOPFailoverMapRecord),	IOP_HBC0,	IOP_HBC1,
		0,			0,			0,			1,		sizeof(IOPFailoverMapRecord),	IOP_HBC1,	IOP_HBC0,
		0,			0,			0,			1,		sizeof(IOPFailoverMapRecord),	IOP_SSDU0,	IOP_SSDL0,
		0,			0,			0,			1,		sizeof(IOPFailoverMapRecord),	IOP_SSDL0,	IOP_SSDU0,
		0,			0,			0,			1,		sizeof(IOPFailoverMapRecord),	IOP_SSDU1,	IOP_SSDL1,
		0,			0,			0,			1,		sizeof(IOPFailoverMapRecord),	IOP_SSDL1,	IOP_SSDU1,
		0,			0,			0,			1,		sizeof(IOPFailoverMapRecord),	IOP_SSDU2,	IOP_SSDL2,
		0,			0,			0,			1,		sizeof(IOPFailoverMapRecord),	IOP_SSDL2,	IOP_SSDU2,
		0,			0,			0,			1,		sizeof(IOPFailoverMapRecord),	IOP_SSDU3,	IOP_SSDL3,
		0,			0,			0,			1,		sizeof(IOPFailoverMapRecord),	IOP_SSDL3,	IOP_SSDU3,
		0,			0,			0,			1,		sizeof(IOPFailoverMapRecord),	IOP_RAC0,	IOP_RAC1,
		0,			0,			0,			1,		sizeof(IOPFailoverMapRecord),	IOP_RAC1,	IOP_RAC0,
		0,			0,			0,			1,		sizeof(IOPFailoverMapRecord),	IOP_APP0,	IOP_APP2,
		0,			0,			0,			1,		sizeof(IOPFailoverMapRecord),	IOP_APP2,	IOP_APP0,
		0,			0,			0,			1,		sizeof(IOPFailoverMapRecord),	IOP_APP1,	IOP_APP3,
		0,			0,			0,			1,		sizeof(IOPFailoverMapRecord),	IOP_APP3,	IOP_APP1,
		0,			0,			0,			1,		sizeof(IOPFailoverMapRecord),	IOP_NIC0,	IOP_NIC1,
		0,			0,			0,			1,		sizeof(IOPFailoverMapRecord),	IOP_NIC1,	IOP_NIC0
	};
	
	
	m_nIOPFailoverMapRecords = 18;
	m_pIOPFailoverMap = (IOPFailoverMapRecord*)new(tPCI) char[sizeof(newIOPFailoverMapTable)];
	memcpy( (char*)m_pIOPFailoverMap,
			(char*)&newIOPFailoverMapTable,
			sizeof(newIOPFailoverMapTable)
		  ); 

	// Create a new InsertRow Object, Initialize it with our pramaeters
	// and send it off to the the table service.  This will insert
	// the new record initialized above into the DiskStatusTable.
	m_pInsertRow = new TSInsertRow;
	
	m_pInsertRow->Initialize(
		this,									// Ddm* ClientDdm
		CT_IOPFM_TABLE_NAME,					// prgbTableName
		m_pIOPFailoverMap,						// prgbRowData
		sizeof(newIOPFailoverMapTable),			// cbRowData
		NULL,									// *pRowIDRet
		(pTSCallback_t)&VerifyVirtualCDTExists,	// pTSCallback_t pCallback,
		this									// pContext
	);
	
	// Initiate the insert operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		m_pInsertRow->Send();
		
	return status;
}


// VerifyVirtualCDTExists - Attempts to define the VirtualClassDescriptorTable.
// This method is the primary entry point for the Virtual Class creation process.
// This method insures the existance of the VirtualClassDescriptorTable.  This is 
// done by simply defining the table.  If the table already exists, no harm will
// be done and if the table doesn't already exist then it will be defined.
// When initiating the DefineTable Operation this method specifies the method
// VerifyDdmCDTExists as the callback function that method will continue the 
// rest of the Class Table creation process.  
STATUS DdmRedundancyMgr::VerifyVirtualCDTExists(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	// This is the code to create the VirtualClassDescriptorTable.
		  
	// Allocate an Define Table object for the Virtual Class Descriptor Table.
	m_pDefineTable = new TSDefineTable;

	// Initialize the define Table object.
	status = m_pDefineTable->Initialize(
		this,											// DdmServices* pDdmServices,
		CT_VCDT_TABLE_NAME,								// String64 prgbTableName,
		aVirtualClassDescriptorTable_FieldDefs,			// fieldDef* prgFieldDefsRet,
		cbVirtualClassDescriptorTable_FieldDefs,		// U32 cbrgFieldDefs,
		20,												// U32 cEntriesRsv,
		false,											// bool* pfPersistant,
		(pTSCallback_t)&ReadVirtualCDT,				// pTSCallback_t pCallback,
		NULL											// void* pContext
	);
	
	// Initiate the define table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		m_pDefineTable->Send();

	return status;
}


// ReadVirtualCDT - This method reads the VirtualClassDescriptorTable
STATUS DdmRedundancyMgr::ReadVirtualCDT(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	// Allocate an ReadTable object for the DdmClassDescriptorTable.
	m_pReadTable = new TSReadTable;

	m_pVCDT = NULL;
	m_nRowsVCDT = 0;

	// Initialize the modify row operation.
	status = m_pReadTable->Initialize(	
		this,									// DdmServices pDdmServices.
		CT_VCDT_TABLE_NAME,						// String64 rgbTableName.
		&m_pVCDT,								// void* &ppTableDataRet.
		&m_nRowsVCDT,							// U32 *pcRowsRet,
		(pTSCallback_t)&VerifyDdmCDTExists,		// pTSCallback_t pCallback,
		this
	);

	// Initiate the enumerate table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		m_pReadTable->Send();
	else
		; // ???
	
	return status;
}


// VerifyDdmCDTExists -
// This method insures the existance of the DdmClassDescriptorTable.  This is 
// done by simply defining the table.  If the table already exists, no harm will
// be done and if the table doesn't already exist then it will be defined.
// When initiating the DefineTable Operation this method specifies the method
// ReadDdmCDT as the callback function that method will continue the 
// rest of the Class Table creation process.  
STATUS DdmRedundancyMgr::VerifyDdmCDTExists(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	// This is the code to create the DdmClassDescriptorTable.
	m_pDefineTable = new TSDefineTable;

	m_pDefineTable->Initialize
	(
		this,									// DdmServices* pDdmServices,
		CT_DCDT_TABLE_NAME,						// String64 prgbTableName,
		aDdmClassDescriptorTable_FieldDefs,		// fieldDef* prgFieldDefsRet,
		cbDdmClassDescriptorTable_FieldDefs,	// U32 cbrgFieldDefs,
		20,										// U32 cEntriesRsv,
		false,									// bool* pfPersistant,
		(pTSCallback_t)&ReadDdmCDT,				// pTSCallback_t pCallback,
		NULL									// void* pContext
	);
	
	// Initiate the define table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		m_pDefineTable->Send();

	return status;
}


// ReadDdmCDT - This method reads the DdmClassDescriptorTable
STATUS DdmRedundancyMgr::ReadDdmCDT(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	// Allocate an ReadTable object for the DdmClassDescriptorTable.
	m_pReadTable = new TSReadTable;

	m_pDdmCDT = NULL;
	m_nRowsDdmCDT = 0;

	// Initialize the modify row operation.
	status = m_pReadTable->Initialize(	
		this,									// DdmServices pDdmServices.
		CT_DCDT_TABLE_NAME,						// String64 rgbTableName.
		&m_pDdmCDT,								// void* &ppTableDataRet.
		&m_nRowsDdmCDT,							// U32 *pcRowsRet,
		(pTSCallback_t)&ListenOnDdmCDTInserts,	// pTSCallback_t pCallback,
		this
	);

	// Initiate the enumerate table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		m_pReadTable->Send();
	else
		; // ???
	
	return status;
}


// ListenOnDdmCDTInserts - Listens for inserts into the DdmClassDescriptorTable.
STATUS DdmRedundancyMgr::ListenOnDdmCDTInserts(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	// Allocate an Listen object for the Ddm ClassDescriptorTable.
	m_pListen = new TSListen;
	
	// Initialize the Listen operation to reply to HandleDdmCDTInsertListenReply
	// whenever rows are inserted into the DdmClassDescriptorTable.
	status =  m_pListen->Initialize(	
		this,											// DdmServices* pDdmServices,
		ListenOnInsertRow,								// ListenTypeEnum ListenType,
		CT_DCDT_TABLE_NAME,								// String64 prgbTableName,
		NULL,											// String64 prgbRowKeyFieldName,
		NULL,											// void* prgbRowKeyFieldValue,
		0,												// U32 cbRowKeyFieldValue,
		NULL,											// String64 prgbFieldName,
		NULL,											// void* prgbFieldValue,
		0,												// U32 cbFieldValue,
		ReplyContinuous,								// ReplyModeEnum ReplyMode,
		NULL,											// void** ppTableDataRet,
		NULL,											// U32* pcbTableDataRet,
		&m_DdmCDTInsertionListenerID,					// U32 *pListenerIDRet,
		NULL,											// U32 *pListenerTypeRet,
		&m_pDdmCDTInsertedRecord,						// void* pModifiedRecordRet,
		&m_cbDdmCDTInsertedRecord,						// U32	cbModifiedrecordRet,
		(pTSCallback_t)&HandleDdmCDTInsertListenReply,	// pTSCallback_t pCallback,
		NULL											// void *pContext
	);
		
	// Start listening by sending the Listen message to the Table Service.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		m_pListen->Send();
		
	return status;
}


// HandleDdmCDTInsertListenReply -
// This is the reply handler for the Listen on inserts into the Ddm Class Descriptor Table.  
// This method begins the process of handling the newly inserted DdmClassDescriptorTable
// record 
STATUS DdmRedundancyMgr::HandleDdmCDTInsertListenReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	// Extend our copy of the DdmClassdescriptorTable with the newly inserted record returned by Listen.
	ExtendTable(m_pDdmCDT,							// pointer to the table to extend.
				m_nRowsDdmCDT,						// # of records currently in table.
				sizeof(DdmClassDescriptorRecord),	// the size of records  in the table.
				m_pDdmCDTInsertedRecord,			// Pointer to new record(s) to append.
				1									// # of new records to extend table by.
			   );
	
	// Get a pointer to the last record which is also the new record.
	#if true
	// We shouldn't need this anymore - on the other hand it shouldn't hurt.
	m_pDdmCDTInsertedRecord = m_pDdmCDT + (m_nRowsDdmCDT * sizeof(DdmClassDescriptorRecord));
	#endif
	
	// Increment the number of rows in the DdmClassdescriptorTable.
	m_nRowsDdmCDT++;

	// Loop through the DdmClassDescriptorTable looking for a redundant
	// pair to the newly inserted DdmClassDescriptorRecord. 
	for (int i = 0; i < m_nRowsDdmCDT; i++)
		// If we found a redundant pair...
		if (IsRedundantPair(&m_pDdmCDT[i], m_pDdmCDTInsertedRecord, m_VCDTNewRecs))
		{
			// ... and if it's not already in the VirtualClassDescriptorTable...
			if (!FindMatchingVCDTRecords(m_VCDTNewRecs))
			{
				// Allocate an insertRow object, init it and send it off.
				m_pInsertRow = new TSInsertRow;
				
				status = m_pInsertRow->Initialize(this,										// DdmServices *pDdmServices,
												  CT_VCDT_TABLE_NAME,			// String64 rgbTableName,
												  &m_VCDTNewRecs[0],						// void *prgbRowData,
												  sizeof(m_VCDTNewRecs),					// U32 cbRowData,
												  &m_rgridNewVCDTRecords[0],				// rowID *pRowIDRet,
												  (pTSCallback_t)&HandleVCDTInsertReply,	// pTSCallback_t pCallback,
												  NULL										// void *pContext
											      );
											   
				if (status == OS_DETAIL_STATUS_SUCCESS)							   
					m_pInsertRow->Send();
					
			}
			break;
		}
		
	return status;
}


// HandleVCDTInsertReply - Handles the reply from the insdert operation.
STATUS DdmRedundancyMgr::HandleVCDTInsertReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	// If all went OK bump the # of rows in the VirtualClassDescriptorTable
	// also store the returned row IDs in our local copy of the 
	// VirtualClassDescriptorTable.
	if (status == OS_DETAIL_STATUS_SUCCESS)							   
	{
		m_pVCDT[m_nRowsVCDT++].rid = m_rgridNewVCDTRecords[1];
		m_pVCDT[m_nRowsVCDT++].rid = m_rgridNewVCDTRecords[2];
	}
}
	


// ExtendTable - Malloc more memory, copy the table into it and append the new record.
void DdmRedundancyMgr::ExtendTable(	void*	&pTable,		// pointer to the table to extend.
									U32		NumRecords,		// # of records currently in table.
									U32		cbRecords,		// the size of records  in the table.
									void*	pNewRecords,	// Pointer to new record to append.
									U32		nNewRecords		// # of new records to extend table by.
							 	  )
{
	// Save the original table pointer.
	void*	pOldTable = pTable;
	
	// Calculate the size of the existing table, the new records and the new table.
	U32		cbOldTable = NumRecords * cbRecords;
	U32		cbNewRecords = nNewRecords * cbRecords;
	U32		cbNewTable = cbOldTable + cbNewRecords;
	
	// Allocate a new chunk of memory to hold the extended table.
	pTable = new(tPCI) char[cbNewTable];
	
	if (!pTable)
		Tracef("Can't allocate memory in DdmRedundancyMgr!!!");
		
	// Copy the table to the existing records to the new table. 
	memcpy((char*)pTable, pOldTable, cbOldTable);
	
	// Append the new record.
	memcpy((char*)pTable + cbOldTable, (char*)pNewRecords, cbNewRecords);
	
	// Delete the old memory;
	delete pOldTable;
}


// IsRedundantPair - Determine if the two DdmClassDescriptorRecords
// form a redundant pair; return two new VirtualClassDescriptorRecords
// if they do.
bool DdmRedundancyMgr::IsRedundantPair(
	DdmClassDescriptorRecord 		*pDdmCDTRec1,
	DdmClassDescriptorRecord		*pDdmCDTRec2,
	VirtualClassDescriptorRecord	VCDTRecs[2]
	)
{
	if ((strcmp(pDdmCDTRec1->ClassName, pDdmCDTRec2->ClassName) == 0) &&	// Names are Equal?
		(pDdmCDTRec1->ClassVersion == pDdmCDTRec2->ClassVersion) &&		// Versions are Equal?
		(pDdmCDTRec1->ClassRevision == pDdmCDTRec2->ClassRevision) &&		// Revisions are Equal?
		SlotsAreRedundant(pDdmCDTRec1->iSlot, pDdmCDTRec2->iSlot)
	   )
	{
		// Populate the VirtualClassDescriptorRecords
		VCDTRecs[0].rid.HiPart = 0;
		VCDTRecs[0].rid.LoPart = 0;
		VCDTRecs[0].version = 1;
		VCDTRecs[0].size = sizeof(VirtualClassDescriptorRecord);
		strcpy(VCDTRecs[0].ClassName, pDdmCDTRec1->ClassName);
		VCDTRecs[0].ClassVersion = pDdmCDTRec1->ClassVersion;
		VCDTRecs[0].ClassRevision = pDdmCDTRec1->ClassRevision;
		VCDTRecs[0].ridPropertySheet = pDdmCDTRec1->ridPropertySheet;
		VCDTRecs[0].sStack = pDdmCDTRec1->sStack;
		VCDTRecs[0].sQueue = pDdmCDTRec1->sQueue;
		VCDTRecs[0].iPrimaryCabinet = pDdmCDTRec2->iCabinet;
		VCDTRecs[0].iPrimarySlot = pDdmCDTRec2->iSlot;
		VCDTRecs[0].iFailoverCabinet = pDdmCDTRec1->iCabinet;
		VCDTRecs[0].iFailoverSlot = pDdmCDTRec1->iSlot;

		VCDTRecs[1].rid.HiPart = 0;
		VCDTRecs[1].rid.LoPart = 0;
		VCDTRecs[1].version = 1;
		VCDTRecs[1].size = sizeof(VirtualClassDescriptorRecord);
		strcpy(VCDTRecs[1].ClassName, pDdmCDTRec1->ClassName);
		VCDTRecs[1].ClassVersion = pDdmCDTRec1->ClassVersion;
		VCDTRecs[1].ClassRevision = pDdmCDTRec1->ClassRevision;
		VCDTRecs[1].ridPropertySheet = pDdmCDTRec1->ridPropertySheet;
		VCDTRecs[1].sStack = pDdmCDTRec1->sStack;
		VCDTRecs[1].sQueue = pDdmCDTRec1->sQueue;
		VCDTRecs[1].iPrimaryCabinet = pDdmCDTRec1->iCabinet;
		VCDTRecs[1].iPrimarySlot = pDdmCDTRec1->iSlot;
		VCDTRecs[1].iFailoverCabinet = pDdmCDTRec2->iCabinet;
		VCDTRecs[1].iFailoverSlot = pDdmCDTRec2->iSlot;
		
		return true;
	}
	else
		return false;
}


// SlotsAreRedundant - Checks the IOFailoverMapTable to see
// if the specified slots are redundant counterparts.
bool DdmRedundancyMgr::SlotsAreRedundant( TySlot slot1, TySlot slot2)
{
IOPFailoverMapRecord*	pFailoverMapRecord1 = FindIOPFailoverMapEntry( slot1 );
IOPFailoverMapRecord*	pFailoverMapRecord2 = FindIOPFailoverMapEntry( slot2 );

	if ((pFailoverMapRecord1 && pFailoverMapRecord2) &&
		(pFailoverMapRecord1->FailoverSlotNum == slot2) &&
		(pFailoverMapRecord2->FailoverSlotNum == slot1))
		return true;
	else
		return false;			
}


// FindIOPFailoverMapEntry -
// This method will return the IOPFailoverMapEntry for the specified IOP slot.
IOPFailoverMapRecord*	DdmRedundancyMgr::FindIOPFailoverMapEntry(TySlot IOPSlotNum)
{
int	i = 0;

	while (i < m_nIOPFailoverMapRecords)
		if (m_pIOPFailoverMap[i].PrimarySlotNum == IOPSlotNum)
			return &m_pIOPFailoverMap[i];
		else
			i++;
		
	Tracef("AHHHH! COULDN'T FIND SLOT # %d IN IOP FAILOVER MAP TABLE!!!\n", IOPSlotNum);	
	return NULL;
}
 
 
// FindMatchingVCDTRecords -
// This method searches our local copy of the VirtualClassDescriptorTable
// for records matching the specified records.  
bool DdmRedundancyMgr::FindMatchingVCDTRecords( VirtualClassDescriptorRecord	VCDTRecs[2] )
{
	// Loop through the VirtualClassDescriptorTable looking for the VCDTRecs[0]. 
	for (int i = 0; i < m_nRowsVCDT; i++)
		if ((strcmp(VCDTRecs[0].ClassName, m_pVCDT[i].ClassName) == 0) &&	// Names are Equal?
			(VCDTRecs[0].ClassVersion, m_pVCDT[i].ClassVersion) &&			// Versions are Equal?
			(VCDTRecs[0].ClassRevision, m_pVCDT[i].ClassRevision) &&		// Revisions are Equal?
			(VCDTRecs[0].iPrimarySlot, m_pVCDT[i].iPrimarySlot))							// Slots Are Equal?
			return true;
			
	return false;
}


#if 0
// EnumerateVirtualCDT - Enumerates the VirtualClassDescriptorTable.
STATUS DdmRedundancyMgr::EnumerateVirtualCDT(void *pClientContext, STATUS status)
{
	// Allocate an Enumerate Table object for the DiskStatusTable.
	m_pReadTable = new TSReadTable;

	m_pTableRows = NULL;
	m_nTableRows = 0;

	// Initialize the modify row operation.
	status = m_pReadTable->Initialize(	
		this,									// DdmServices pDdmServices,
		VIRTUAL_CLASS_DESCRIPTOR_TABLE,			// String64 rgbTableName,
		&m_pVCDT,								// void* &ppTableDataRet, returned table data.
		&m_nRowsVCDT,							// U32 *pcRowsRet,			// returned # of rows read
		(pTSCallback_t)&ListenOnDdmCDTInserts,	// pTSCallback_t pCallback,
		this
	);

	// Initiate the enumerate table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		status = m_pReadTable->Send()
	else
		; // ???
	
	return status;
}
#endif

#if 0
// FindNewDdmCDTRecord -
// This method will search the DdmClassDescriptorTable for the newly inserted
// record whose rowID was returned to us by our outstanding listen operation.  
STATUS DdmRedundancyMgr::FindNewDdmCDTRecord(void *pClientContext, STATUS status)
{
	m_pDdmCDTInsertedRec = NULL;
	
	// Loop through the DdmClassDescriptorTable looking for the newly inserted row ID. 
	for (int i = 0; i < m_nRowsDdmCDT; i++)
		if (m_pDdmCDT[i].rid == m_ridDdmCDTInsertedRow)
		{
			m_pDdmCDTInsertedRec = &m_pDdmCDT[i];
			break;
		}
		
	// If we didn't find the new record that's a BIG problem...we're insane.
	if (m_pDdmCDTInsertedRec == NULL)
		Tracef("AHHHH! COULDN'T FIND NEWLY INSERTED RECORD IN DDM CLASS DESCRIPTOR TABLE!!!");
}	
#endif


