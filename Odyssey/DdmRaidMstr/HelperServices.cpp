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
// File: HelperServices.cpp
// 
// Description:
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/HelperServices.cpp $
// 
// 12    12/09/99 4:47p Agusev
// use autostart = true for vd create (till false works..) - Dipam
// 
// 11    11/09/99 3:49p Dpatel
// fix for win32 .. vd create..
// 
// 10    11/09/99 2:50p Dpatel
// win32 compilation etc.
// 
// 9     11/08/99 9:36a Jlane
// Made changes to utilize new VirtualMaster LoadVirtualDevice message.
// 
// 8     11/04/99 3:58p Jlane
// Use VDT_FIOPHASDID_FIELD.
// 
// 7     10/21/99 12:18p Dpatel
// added ifdef win32 to make create vd work for windows..
// 
// 6     10/11/99 6:43p Dpatel
// changes for integration
// 
// 5     10/07/99 1:27p Dpatel
// fixed some bugs in the VD create..
// 
// 3     9/17/99 1:56p Dpatel
// 
// 2     9/16/99 5:42p Dpatel
// 
// 1     9/15/99 4:00p Dpatel
// Initial creation
//
/*************************************************************************/

#include "HelperServices.h"
#include "CtUtils.h"
#include "RqOsVirtualMaster.h"


//************************************************************************
//	CONSTRUCTOR
//		Store the parent ddm services pointer. This will be used to call
//		the callback.
//************************************************************************
HelperServices::HelperServices(DdmServices *pParentDdm)
				: DdmServices( pParentDdm )
{
	m_pParentDdm = pParentDdm;
	m_pTableServices			= NULL;
	m_pStringResourceManager	= NULL;

	m_pTableServices = new TableServices(this);

	// create the string resource manager
	m_pStringResourceManager = new StringResourceManager (
										m_pParentDdm,
										TSCALLBACK(HelperServices,StringResourceManagerInitializedReply));
}


//************************************************************************
//	DESTRUCTOR
//
//************************************************************************
HelperServices::~HelperServices()
{
	if (m_pTableServices){
		delete m_pTableServices;
	}
	if (m_pStringResourceManager){
		delete m_pStringResourceManager;
	}
}



//**************************************************************************
//
//	String Resource Manager Initialized reply
//
//**************************************************************************
STATUS HelperServices
::StringResourceManagerInitializedReply(
			void			*pContext,
			STATUS			status)
{
	assert(status == 0);
	return status;
}



//************************************************************************
//	ReadStorageElementName
//		This method reads the name of a storage element. If the name
//		is not found, then it attempts to fill in a unique id
//		if the Storage element is of the type disk.
//
//	Modified values:
//		pContext->ucName - filled if name is found
//		if no name is found then the following may be filled
//			pContext->FCInstance
//			pContext->SlotID	
//			pContext->FCTargetID;
//
//************************************************************************
void HelperServices::
ReadStorageElementName(
		UnicodeString				*pUnicodeString,
		U32							*pSlotIdRet,
		rowID						*pSRCTRID,
		pTSCallback_t				cb,
		void						*pContext)
{	
	HELPER_CONTEXT		*pReadNameContext = new HELPER_CONTEXT;

	pReadNameContext->pParentContext = pContext;
	pReadNameContext->pCallback = cb;
	pReadNameContext->pData		= (void *)new(tZERO) StorageRollCallRecord;
	pReadNameContext->pData1	= (void *)pUnicodeString;
	pReadNameContext->pData2	= (U32 *)pSlotIdRet;
	// first read the src
	STATUS status = m_pTableServices->TableServiceReadRow(
						STORAGE_ROLL_CALL_TABLE,
						pSRCTRID,
						pReadNameContext->pData,
						sizeof(StorageRollCallRecord),
						TSCALLBACK(HelperServices,ReadStorageElementNameSRCReadReply),
						pReadNameContext);
}


//************************************************************************
//
//	ReadStorageElementNameSRCReadReply
//
//************************************************************************
STATUS HelperServices::
ReadStorageElementNameSRCReadReply(void *_pContext, STATUS status)
{
	HELPER_CONTEXT *pReadNameContext = (HELPER_CONTEXT *)_pContext;
	void	*pOriginalContext = pReadNameContext->pParentContext;
	pTSCallback_t	cb = pReadNameContext->pCallback; 

	if (status != OS_DETAIL_STATUS_SUCCESS){
		pReadNameContext->pData1 = NULL; // make sure we dont delete users mem
		pReadNameContext->pData2 = NULL; // make sure we dont delete users mem
		delete pReadNameContext;
		(m_pParentDdm->*cb)(pOriginalContext,OK);
		return status;
	}

	StorageRollCallRecord *pSRCRecord = 
		(StorageRollCallRecord *)pReadNameContext->pData;
	UnicodeString *pUnicodeString = 
		(UnicodeString *)pReadNameContext->pData1;

	// try to read the rid in the SRC Record
	status = m_pStringResourceManager->ReadString(
							pUnicodeString,
							RowId(pSRCRecord->ridName),
							cb,
							pOriginalContext);
	if (status == false){
		HELPER_CONTEXT		*pDiskDescriptorContext = NULL;
		// no name exists, so we better make sure that this is
		// a disk and get its unique identifier and fill it in the
		// our context
		switch(pSRCRecord->storageclass){
		case SRCTypeFCDisk:
		case SRCTypeSSD:
		case SRCTypeRamDisk:
			pDiskDescriptorContext = new HELPER_CONTEXT;
			pDiskDescriptorContext->pParentContext = pOriginalContext;
			pDiskDescriptorContext->pCallback = cb;
			pDiskDescriptorContext->pData = new DiskDescriptor;
			pDiskDescriptorContext->pData1 = pReadNameContext->pData2;
			status = m_pTableServices->TableServiceReadRow(
						DISK_DESC_TABLE,
						&pSRCRecord->ridDescriptorRecord,
						pDiskDescriptorContext->pData,
						sizeof(DiskDescriptor),
						TSCALLBACK(HelperServices,ReadStorageElementNameDiskDescriptorReadReply),
						pDiskDescriptorContext);
			break;
		default:
			(m_pParentDdm->*cb)(pOriginalContext,OK);
		}
	}
	pReadNameContext->pData1 = NULL; // make sure we dont delete users mem
	pReadNameContext->pData2 = NULL; // make sure we dont delete users mem
	delete pReadNameContext;
	return status;
}

//************************************************************************
//
//	ReadStorageElementNameDiskDescriptorReadReply
//
//************************************************************************
STATUS HelperServices::
ReadStorageElementNameDiskDescriptorReadReply(void *_pContext, STATUS status)
{
	HELPER_CONTEXT	*pDiskDescriptorContext = (HELPER_CONTEXT *)_pContext;
	void			*pOriginalContext = pDiskDescriptorContext->pParentContext;

	DiskDescriptor	*pDiskDescriptor = (DiskDescriptor *)pDiskDescriptorContext->pData;
	U32	*pSlotIDRet = (U32 *)pDiskDescriptorContext->pData1;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		*pSlotIDRet = 0;
	} else{
		*pSlotIDRet = pDiskDescriptor->SlotID;
	}
	pTSCallback_t	cb = pDiskDescriptorContext->pCallback; 
	(m_pParentDdm->*cb)(pOriginalContext,OK);
	pDiskDescriptorContext->pData1 = NULL;
	delete pDiskDescriptorContext;
	return status;
}



//************************************************************************
//	DeleteStorageElementName
//
//************************************************************************
void HelperServices::
DeleteStorageElementName(
		rowID						*pSRCTRID,
		pTSCallback_t				cb,
		void						*pContext)
{	
	HELPER_CONTEXT		*pDeleteNameContext = new HELPER_CONTEXT;

	pDeleteNameContext->pParentContext = pContext;
	pDeleteNameContext->pCallback = cb;
	pDeleteNameContext->pData = new StorageRollCallRecord;

	// first read the src
	STATUS status = m_pTableServices->TableServiceReadRow(
						STORAGE_ROLL_CALL_TABLE,
						pSRCTRID,
						pDeleteNameContext->pData,
						sizeof(StorageRollCallRecord),
						TSCALLBACK(HelperServices,DeleteStorageElementNameSRCReadReply),
						pDeleteNameContext);
}


//************************************************************************
//
//	DeleteStorageElementNameSRCReadReply
//
//************************************************************************
STATUS HelperServices::
DeleteStorageElementNameSRCReadReply(void *_pContext, STATUS status)
{
	HELPER_CONTEXT	*pDeleteNameContext = (HELPER_CONTEXT *)_pContext;
	void	*pOriginalContext	= pDeleteNameContext->pParentContext;
	pTSCallback_t cb			= pDeleteNameContext->pCallback; 

	if (status != OS_DETAIL_STATUS_SUCCESS){
		delete pDeleteNameContext;
		(m_pParentDdm->*cb)(pOriginalContext,OK);
		return status;
	}

	StorageRollCallRecord *pSRCRecord = 
		(StorageRollCallRecord *)pDeleteNameContext->pData;

	// try to read the rid in the SRC Record
	status = m_pStringResourceManager->DeleteString(
							RowId(pSRCRecord->ridName),
							cb,
							pOriginalContext);
	delete pDeleteNameContext;
	return status;
}



//************************************************************************
//	CreateVirtualDevice
//	Create Virtual Device
//
//************************************************************************
STATUS HelperServices
::CreateVirtualDevice(
		VirtualDeviceRecord			*pVDRecord,
		pTSCallback_t				cb,
		void						*pClientContext)
{	
	TSReadTable *pReadSystemStatusRec = new TSReadTable;

	HELPER_CONTEXT		*pHelperContext = new HELPER_CONTEXT;

	pHelperContext->pParentContext = pClientContext;
	pHelperContext->pCallback = cb;


	pHelperContext->pData = new(tZERO) SystemStatusRecord;
	// save the VD record to be inserted
	pHelperContext->pData1 = pVDRecord;

#ifndef WIN32
	STATUS status = pReadSystemStatusRec->Initialize( 
			this,										// DdmServices *pDdmServices,
			SYSTEM_STATUS_TABLE,						// String64 rgbTableName,
			&pHelperContext->pData,						// void* *ppTableDataRet
			NULL,										// U32 *pcRowsReadRet,
			(pTSCallback_t) &HelperServices::CheckExistingVirtualDeviceRecord,	// pTSCallback_t pCallback,
			pHelperContext									// void* pContext
		);
	if (status == OK)
		pReadSystemStatusRec->Send();
	else 
		CleanVDCreate(pHelperContext,status);
#else 
	// for our testing just define the VD table
	TSDefineTable *pVDDefineTable = new TSDefineTable;

	// Initialize the define Table object.
	STATUS status = pVDDefineTable->Initialize(
		this,									// DdmServices* pDdmServices,
		(char *)pVDRecord->TableName(),
		(fieldDef *)pVDRecord->FieldDefs(),
		pVDRecord->FieldDefsSize(),									// U32 cbrgFieldDefs,
		5,											// U32 cEntriesRsv,
		true,										// bool* pfPersistant,
		TSCALLBACK(HelperServices,CheckExistingVirtualDeviceRecord),
		pHelperContext								// void* pContext
	);
	
	// Initiate the define table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pVDDefineTable->Send();
	else 
		CleanVDCreate(pHelperContext,status);
#endif
	return status;	
}


//************************************************************************
//	CreateVirtualDevice
//
//	Check Existing Virtual Device record
//
//************************************************************************
STATUS HelperServices
::CheckExistingVirtualDeviceRecord(
		void						*_pHelperContext,
		STATUS						status)
{
	HELPER_CONTEXT		*pHelperContext = (HELPER_CONTEXT *)_pHelperContext;

	VirtualDeviceRecord	*pVDRecord	= (VirtualDeviceRecord *)pHelperContext->pData1;

	// If the table exists try to read the VD record.
	if (status == ercTableExists)
	{
		TSReadRow *pReadVDTRec = new TSReadRow;
		status = pReadVDTRec->Initialize( 
				this,									// DdmServices *pDdmServices,
				PTS_VIRTUAL_DEVICE_TABLE,				// String64 rgbTableName,
				VDT_RID_VDOWNERUSE_FIELD,				// String64 rgbKeyFieldName,
				&pVDRecord->ridVDOwnerUse,				// For internal use by VD Owner/Creator. ** KEY FIELD **
				sizeof(rowID),							// U32 cbKeyFieldValue,
				pVDRecord,								// void *prgbRowDataRet,
				sizeof(VirtualDeviceRecord),			// U32 cbRowDataRetMax,
				&pHelperContext->value,					// U32 *pcRowsReadRet,
				(pTSCallback_t) &HelperServices::LoadVirtualDevice,	// pTSCallback_t pCallback,
				pHelperContext							// void* pContext
			);
		if (status == OK) 
			pReadVDTRec->Send();
	}	
	else
		if (status == OK)
			// We created the VD table so continue with the Load.
			status = LoadVirtualDevice(_pHelperContext, !status);

	if (status != OK)
		CleanVDCreate(pHelperContext,status);
		
	return status;
}


//************************************************************************
//	LoadVirtualDevice
//
//	Load Virtual Device
//
//************************************************************************
STATUS HelperServices
::LoadVirtualDevice(
		void						*_pHelperContext,
		STATUS						status)
{
	HELPER_CONTEXT		*pHelperContext = (HELPER_CONTEXT *)_pHelperContext;

	VirtualDeviceRecord	*pVDRecord	= (VirtualDeviceRecord *)pHelperContext->pData1;

	RqOsVirtualMasterLoadVirtualDevice *pCreateVDMsg = NULL;

#ifdef WIN32
	// just fake the vd create and return a dummy vd number
	status = CleanVDCreate(pHelperContext,status);
	return status;
#else 
	// If we could not find our VD in the VDT, then load the VD.
	if (status != OK)
	{
		// Alloocate and construct a VirtualMasterLoadVirtualDevice message.
		// Mark the VD with the rowID of the DiskDescriptor that is creating it.
		pCreateVDMsg = new RqOsVirtualMasterLoadVirtualDevice(
			pVDRecord->szClassName,				// Class Name of VD.
			pVDRecord->slotPrimary,				// Primary Slot.
			pVDRecord->slotSecondary,			// Secondary Slot
			true,								// fAutoStart
			RowId(pVDRecord->ridDdmCfgRec),		// rid of VD's Config Record
			RowId(pVDRecord->ridVDOwnerUse)		// Owner unique ID rid
		);
	
		// Check the pointer and...
		if (!pCreateVDMsg)
			// Set an error if null.
			status = CTS_OUT_OF_MEMORY;
		else
			// Send the message off to the Virtual Master.
			status = Send(pCreateVDMsg, pHelperContext,
						  REPLYCALLBACK(HelperServices, LoadVirtualDeviceReply)
						 );
	}
	
	// If the VD already exists, or we couldn't allocate msg, or
	// send returned a bad status, we are done with creation.
	if (status != OK)
	{
		CheckFreeAndClear(pCreateVDMsg);
		CleanVDCreate(pHelperContext,status);
	}			
	return status;
#endif
}


//************************************************************************
//	LoadVirtualDeviceReply
//
//	Handle Reply after loading of new VD 
//
//************************************************************************
STATUS HelperServices
::LoadVirtualDeviceReply(
		Message*					pMsgRet)
{
RqOsVirtualMasterLoadVirtualDevice*	pCreateVDMsg;
STATUS								status;
HELPER_CONTEXT*						pHelperContext;
VirtualDeviceRecord*				pVDRecord;

	pCreateVDMsg = (RqOsVirtualMasterLoadVirtualDevice *)pMsgRet;
	status = pCreateVDMsg->Status();
	pHelperContext = (HELPER_CONTEXT *)pCreateVDMsg->GetContext();
	pVDRecord	= (VirtualDeviceRecord *)pHelperContext->pData1;
	
	// delete the msg.
	delete pCreateVDMsg;

	if (status == OK)
	{
		// Attempt to read the newly created VD record.
		TSReadRow *pReadVDTRec = new TSReadRow;
		status = pReadVDTRec->Initialize( 
				this,									// DdmServices *pDdmServices,
				PTS_VIRTUAL_DEVICE_TABLE,				// String64 rgbTableName,
				VDT_RID_VDOWNERUSE_FIELD,				// String64 rgbKeyFieldName,
				&pVDRecord->ridVDOwnerUse,				// For internal use by VD Owner/Creator. ** KEY FIELD **
				sizeof(rowID),							// U32 cbKeyFieldValue,
				pVDRecord,								// void *prgbRowDataRet,
				sizeof(VirtualDeviceRecord),			// U32 cbRowDataRetMax,
				&pHelperContext->value,					// U32 *pcRowsReadRet,
				TSCALLBACK(HelperServices,CleanVDCreate),	// pTSCallback_t pCallback,
				pHelperContext							// void* pContext
			);
		if (status == OK) 
			pReadVDTRec->Send();
	}
	
	// If we got an error cleanup.
	if (status != OK)
		CleanVDCreate(pHelperContext,status);		
			
	return status;
}


//************************************************************************
//	CreateVirtualDevice
//
//	CleanVDCreate
//
//************************************************************************
STATUS HelperServices
::CleanVDCreate(void *_pHelperContext, STATUS status)
{
	HELPER_CONTEXT *pHelperContext	= (HELPER_CONTEXT*)_pHelperContext;
	void *pOriginalContext			= pHelperContext->pParentContext;
	pTSCallback_t cb				= pHelperContext->pCallback; 

	pHelperContext->pData1 = NULL;	// vd record, user memory - dont delete
	delete pHelperContext;
#ifdef WIN32
	(m_pParentDdm->*cb)(pOriginalContext,OK);
#else
	(m_pParentDdm->*cb)(pOriginalContext,status);
#endif
	return status;
}