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
// File: RmstrDefineTables.h
// 
// Description:
// Defines the PTS tables required by RMSTR
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrDefineTables.cpp $
// 
// 10    8/30/99 8:35a Jhatwich
// added casts for win32 build
// 
// 9     8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 8     8/03/99 10:15a Jtaylor
// added RMSTR_RAID_DDM test ifdefs
// 
// 7     8/02/99 3:18p Jtaylor
// fixed warnings
// 
// 6     7/28/99 6:35p Dpatel
// Added capability code, table services, add/remove members, preferred
// member and source member, hot copy etc...
// 
// 5     7/23/99 5:47p Dpatel
// Added internal cmds, hotcopy, changed commit spare etc.
// 
// 4     7/17/99 3:09p Dpatel
// added reading of PTS tables during init.
// 
// 3     6/28/99 5:18p Dpatel
// Changed the header.
// 
//
// 06/11/99 Dipam Patel: Create file
//
/*************************************************************************/


#include "DdmRaidMgmt.h"


// Define Tables and Listener Registration states
typedef enum
{
	rmstrDEFINE_ARRAY_DESCRIPTOR_TABLE = 1,
	rmstrDEFINE_MEMBER_DESCRIPTOR_TABLE,
	rmstrDEFINE_SPARE_DESCRIPTOR_TABLE,
	rmstrDEFINE_UTIL_DESCRIPTOR_TABLE,
	rmstrDEFINE_CAPABILITY_TABLE,
	rmstrCAPABILITY_TABLE_POPULATED,
	rmstrDEFINE_DESCRIPTOR_TABLES_DONE
} rmstrDEFINE_DESCRIPTOR_TABLES;




//************************************************************************
//	rmstrDefineDescriptorTables
//		Define all the tables which RMSTR uses, if they are not
//		already defined.
//
//************************************************************************
STATUS DdmRAIDMstr::
rmstrDefineDescriptorTables()
{
	STATUS			status;
	CONTEXT			*pContext = new CONTEXT;

	memset((void *)pContext,0,sizeof(CONTEXT));
	
	pContext->state = rmstrDEFINE_ARRAY_DESCRIPTOR_TABLE;
	status = rmstrDefineArrayDescriptorTable(pContext);
	return status;
}	


//************************************************************************
//	rmstrProcessDefineDescriptorTablesReply
//		Process the replies for the definition of the ADT, MDT, SDT and UDT
//		When all tables are defined, initialize the RmstrData.
//
//************************************************************************
STATUS DdmRAIDMstr::
rmstrProcessDefineDescriptorTablesReply(void *_pContext, STATUS status)
{
	CONTEXT		*pContext = (CONTEXT *)_pContext;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		// if table exists, then also continue to try for other tables
		if (status != ercTableExists)
			return status;
	}

	switch(pContext->state){
	case rmstrDEFINE_ARRAY_DESCRIPTOR_TABLE:
		pContext->state = rmstrDEFINE_MEMBER_DESCRIPTOR_TABLE;
		rmstrDefineMemberDescriptorTable(pContext);
		break;
	case rmstrDEFINE_MEMBER_DESCRIPTOR_TABLE:
		pContext->state = rmstrDEFINE_SPARE_DESCRIPTOR_TABLE;
		rmstrDefineSpareDescriptorTable(pContext);
		break;
	case rmstrDEFINE_SPARE_DESCRIPTOR_TABLE:
		pContext->state = rmstrDEFINE_UTIL_DESCRIPTOR_TABLE;
		rmstrDefineUtilDescriptorTable(pContext);
		break;
	case rmstrDEFINE_UTIL_DESCRIPTOR_TABLE:
		pContext->state = rmstrDEFINE_CAPABILITY_TABLE;
		rmstrDefineCapabiliyTable(pContext);
		break;

	case rmstrDEFINE_CAPABILITY_TABLE:
		if (status == ercTableExists){
			// Now all the tables we need are created
			m_DescriptorTablesDefined = 1;
		} else {
			// We need to populate the capability records
			pContext->state = rmstrCAPABILITY_TABLE_POPULATED;
			rmstrPopulateCapabilitiesTable(
				(pTSCallback_t)&DdmRAIDMstr::rmstrProcessDefineDescriptorTablesReply,
				pContext);
		}
		break;

	case rmstrCAPABILITY_TABLE_POPULATED:
		m_DescriptorTablesDefined = 1;
		break;

	default:
		break;
	}
	if (m_DescriptorTablesDefined) {
		// Read any existing data from the tables and store it
		// in our local copy
#ifdef RMSTR_RAID_DDM_TEST
		InitializeCommandQueues();
#else		
		InitializeRmstrData();
#endif		
		// delete the context now.
		delete pContext;
		pContext = NULL;
	}
	return status;
}



//************************************************************************
//
//	DEFINE ARRAY DESCRIPTOR TABLE
//	
//************************************************************************
STATUS DdmRAIDMstr::
rmstrDefineArrayDescriptorTable(CONTEXT *pContext)
{
	STATUS		status;

	// Allocate an Define Table object
	TSDefineTable *pADDefineTable = new TSDefineTable;

	// Initialize the define Table object.
	status = pADDefineTable->Initialize(
		this,									// DdmServices* pDdmServices,
		RAID_ARRAY_DESCRIPTOR_TABLE,			// String64 prgbTableName,
		(fieldDef*)ArrayDescriptorTable_FieldDefs,			// fieldDef* prgFieldDefsRet,
		sizeofArrayDescriptorTable_FieldDefs,	// U32 cbrgFieldDefs,
		RMSTR_TBL_RSRV_ENTRIES,					// U32 cEntriesRsv,
		true,									// bool* pfPersistant,
		(pTSCallback_t)&DdmRAIDMstr::rmstrProcessDefineDescriptorTablesReply,	// pTSCallback_t pCallback,
		pContext								// void* pContext
	);
	
	// Initiate the define table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pADDefineTable->Send();

	return status;
}


//************************************************************************
//
//	DEFINE MEMBER DESCRIPTOR TABLE
//	
//************************************************************************
STATUS DdmRAIDMstr::
rmstrDefineMemberDescriptorTable(CONTEXT *pContext)
{
	STATUS		status;

	// Allocate an Define Table object
	TSDefineTable *pMDDefineTable = new TSDefineTable;

	// Initialize the define Table object.
	status = pMDDefineTable->Initialize(
		this,									// DdmServices* pDdmServices,
		RAID_MEMBER_DESCRIPTOR_TABLE,			// String64 prgbTableName,
		(fieldDef*)MemberDescriptorTable_FieldDefs,		// fieldDef* prgFieldDefsRet,
		sizeofMemberDescriptorTable_FieldDefs,	// U32 cbrgFieldDefs,
		RMSTR_TBL_RSRV_ENTRIES,					// U32 cEntriesRsv,
		true,									// bool* pfPersistant,
		(pTSCallback_t)&DdmRAIDMstr::rmstrProcessDefineDescriptorTablesReply,	// pTSCallback_t pCallback,
		pContext								// void* pContext
	);
	
	// Initiate the define table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pMDDefineTable->Send();

	return status;
}


//************************************************************************
//
//	DEFINE SPARE DESCRIPTOR TABLE
//	
//************************************************************************
STATUS DdmRAIDMstr::
rmstrDefineSpareDescriptorTable(CONTEXT *pContext)
{
	STATUS		status;

	// Allocate an Define Table object
	TSDefineTable *pSDTDefineTable = new TSDefineTable;

	// Initialize the define Table object.
	status = pSDTDefineTable->Initialize(
		this,									// DdmServices* pDdmServices,
		RAID_SPARE_DESCRIPTOR_TABLE,			// String64 prgbTableName,
		(fieldDef*)SpareDescriptorTable_FieldDefs,			// fieldDef* prgFieldDefsRet,
		sizeofSpareDescriptorTable_FieldDefs,	// U32 cbrgFieldDefs,
		RMSTR_TBL_RSRV_ENTRIES,					// U32 cEntriesRsv,
		true,									// bool* pfPersistant,
		(pTSCallback_t)&DdmRAIDMstr::rmstrProcessDefineDescriptorTablesReply,	// pTSCallback_t pCallback,
		pContext								// void* pContext
	);
	
	// Initiate the define table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pSDTDefineTable->Send();

	return status;
}


//************************************************************************
//
//	DEFINE UTIL DESCRIPTOR TABLE
//	
//************************************************************************

STATUS DdmRAIDMstr::
rmstrDefineUtilDescriptorTable(CONTEXT *pContext)
{
	STATUS		status;

	// Allocate an Define Table object
	TSDefineTable *pUDTDefineTable = new TSDefineTable;

	// Initialize the define Table object.
	status = pUDTDefineTable->Initialize(
		this,									// DdmServices* pDdmServices,
		RAID_UTIL_DESCRIPTOR_TABLE,				// String64 prgbTableName,
		(fieldDef*)UtilDescriptorTable_FieldDefs,			// fieldDef* prgFieldDefsRet,
		sizeofUtilDescriptorTable_FieldDefs,	// U32 cbrgFieldDefs,
		RMSTR_TBL_RSRV_ENTRIES,					// U32 cEntriesRsv,
		true,									// bool* pfPersistant,
		(pTSCallback_t)&DdmRAIDMstr::rmstrProcessDefineDescriptorTablesReply,	// pTSCallback_t pCallback,
		pContext								// void* pContext
	);
	
	// Initiate the define table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pUDTDefineTable->Send();

	return status;
}



//************************************************************************
//
//	DEFINE CAPABILITY TABLE
//	
//************************************************************************

STATUS DdmRAIDMstr::
rmstrDefineCapabiliyTable(CONTEXT *pContext)
{
	STATUS		status;

	// Allocate an Define Table object
	TSDefineTable *pDefineTable = new TSDefineTable;

	// Initialize the define Table object.
	status = pDefineTable->Initialize(
		this,									// DdmServices* pDdmServices,
		RMSTR_CAPABILITY_TABLE,					// String64 prgbTableName,
		(fieldDef*)RmstrCapabilityTable_FieldDefs,			// fieldDef* prgFieldDefsRet,
		sizeofRmstrCapabilityTable_FieldDefs,	// U32 cbrgFieldDefs,
		RMSTR_TBL_RSRV_ENTRIES,					// U32 cEntriesRsv,
		true,									// bool* pfPersistant,
		(pTSCallback_t)&DdmRAIDMstr::rmstrProcessDefineDescriptorTablesReply,	// pTSCallback_t pCallback,
		pContext								// void* pContext
	);
	
	// Initiate the define table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pDefineTable->Send();

	return status;
}




enum {
	READ_CAPABILITY_DATA = 1,
	CAPABILITY_DATA_INSERTED
};
//************************************************************************
//
//	POPULATE THE CAPABILITIES TABLE
//	
//************************************************************************
STATUS DdmRAIDMstr::
rmstrPopulateCapabilitiesTable(
				pTSCallback_t	cb,
				CONTEXT			*pContext)
{
	STATUS						status;
	CONTEXT						*pCapabilityContext = NULL;

	pCapabilityContext = new CONTEXT;

	pCapabilityContext->state = READ_CAPABILITY_DATA;
	pCapabilityContext->numProcessed = 0;
	pCapabilityContext->pCallback = cb;
	pCapabilityContext->pParentContext = pContext;
	status = rmstrPopulateCapabilitiesTableReply(pCapabilityContext,OK);
	return status;
}


//************************************************************************
//	rmstrPopulateCapabilitiesTableReply
//	
//	
//************************************************************************
STATUS DdmRAIDMstr::
rmstrPopulateCapabilitiesTableReply(
				void			*_pContext,
				STATUS			status)
{
	CONTEXT						*pCapabilityContext = (CONTEXT *)_pContext;
	char						*pCapabilityData = NULL;
	RMSTR_CAPABILITY_RAID_LEVEL *pRaidLevelCapabilities = NULL;
	RMSTR_CAPABILITY_DESCRIPTOR *pRaidCapabilityDescriptor = NULL;

	U32					i = 0;
	BOOL				cmdComplete = false;

	i = pCapabilityContext->numProcessed;

	switch(pCapabilityContext->state){
	case READ_CAPABILITY_DATA:
		if (RaidCapabilities[i].capabilityCode != RMSTR_CAPABILITY_INVALID){
			switch(RaidCapabilities[i].capabilityCode){
			case RMSTR_CAPABILITY_RAID0:
				pCapabilityData = (char *)&Raid0Capabilities; 
				break;
			case RMSTR_CAPABILITY_RAID1:
				pCapabilityData = (char *)&Raid1Capabilities; 
				break;
			case RMSTR_CAPABILITY_RAID5:
				pCapabilityData = (char *)&Raid5Capabilities; 
				break;
			}

			if (pCapabilityContext->pData){
				delete pCapabilityContext->pData;
				pCapabilityContext->pData = NULL;
			}
			pCapabilityContext->pData = new RMSTR_CAPABILITY_DESCRIPTOR;
			memset(pCapabilityContext->pData, 0, sizeof(RMSTR_CAPABILITY_DESCRIPTOR));

			pRaidCapabilityDescriptor = (RMSTR_CAPABILITY_DESCRIPTOR *)pCapabilityContext->pData;
			memcpy(
				pRaidCapabilityDescriptor,
				&RaidCapabilities[i],
				sizeof(RMSTR_CAPABILITY_DESCRIPTOR));

			memcpy(
				pRaidCapabilityDescriptor->capabilities,
				pCapabilityData,
				sizeof(pRaidCapabilityDescriptor->capabilities));
			pRaidLevelCapabilities = 
				(RMSTR_CAPABILITY_RAID_LEVEL *)pRaidCapabilityDescriptor->capabilities;

			pCapabilityContext->state = CAPABILITY_DATA_INSERTED;
			status = m_pTableServices->TableServiceInsertRow(
						RMSTR_CAPABILITY_TABLE,
						pRaidCapabilityDescriptor,
						sizeof(RMSTR_CAPABILITY_DESCRIPTOR),
						&pRaidCapabilityDescriptor->thisRID,
						(pTSCallback_t)&DdmRAIDMstr::rmstrPopulateCapabilitiesTableReply,
						pCapabilityContext);
		} else {
			cmdComplete = true;
		}
		break;

		case CAPABILITY_DATA_INSERTED:
			pRaidCapabilityDescriptor = (RMSTR_CAPABILITY_DESCRIPTOR *)pCapabilityContext->pData;
			pCapabilityContext->numProcessed++;
			pCapabilityContext->state = READ_CAPABILITY_DATA;
			rmstrPopulateCapabilitiesTableReply(pCapabilityContext,OK);
			break;
	}
	if (cmdComplete){
		pTSCallback_t		cb = pCapabilityContext->pCallback;
		(this->*cb)(
				pCapabilityContext->pParentContext,	
				status);		
		delete pCapabilityContext;
	}
	return status;
}