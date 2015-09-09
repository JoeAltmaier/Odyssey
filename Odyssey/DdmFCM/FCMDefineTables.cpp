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
// File: FCMDefineTables.h
// 
// Description:
// Defines the PTS tables required by FCM
// 
// $Log: /Gemini/Odyssey/FCMDefineTables.cpp $
// 
// 1     1/05/00 5:17p Dpatel
// Initial creation
// 
//
/*************************************************************************/


#include "DdmFCM.h"


// Define Tables and Listener Registration states
typedef enum
{
	FCM_DEFINE_LOOP_DESCRIPTOR_TABLE = 1,
	FCM_DEFINE_WWN_TABLE,
	FCM_INITIALIZE_CHASSIS_WWN,
	FCM_DEFINE_DESCRIPTOR_TABLES_DONE
} FCM_DEFINE_DESCRIPTOR_TABLES;




//************************************************************************
//	FCMDefineDescriptorTables
//		Define all the tables which FCM uses, if they are not
//		already defined.
//
//************************************************************************
STATUS DdmFCM::
FCMDefineDescriptorTables()
{
	STATUS			status;
	CONTEXT			*pContext = new CONTEXT;

	memset((void *)pContext,0,sizeof(CONTEXT));
	
	pContext->state = FCM_DEFINE_LOOP_DESCRIPTOR_TABLE;
	status = FCMDefineLoopDescriptorTable(pContext);
	return status;
}	


//************************************************************************
//	FCMProcessDefineDescriptorTablesReply
//		Process the replies for the definition of the FCM tables
//		When all tables are defined, initialize the CmdQueues.
//
//************************************************************************
STATUS DdmFCM::
FCMProcessDefineDescriptorTablesReply(void *_pContext, STATUS status)
{
	CONTEXT		*pContext = (CONTEXT *)_pContext;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		// if table exists, then also continue to try for other tables
		if (status == ercTableExists){
			status = 0;
		}
	} 
	switch(pContext->state){
		case FCM_DEFINE_LOOP_DESCRIPTOR_TABLE:
			pContext->state = FCM_DEFINE_WWN_TABLE;
			status = FCMDefineWWNTable(pContext);			
			break;

		case FCM_DEFINE_WWN_TABLE:
			pContext->state = FCM_INITIALIZE_CHASSIS_WWN;
			status = FCMInitializeChassisWWN(
							TSCALLBACK(DdmFCM,FCMProcessDefineDescriptorTablesReply),
							pContext);			
			break;

		case FCM_INITIALIZE_CHASSIS_WWN:
			m_DescriptorTablesDefined = 1;
			break;

		default:
			m_DescriptorTablesDefined = 1;
			break;
	}
	if (m_DescriptorTablesDefined) {
		InitializeCommandQueues();
		// delete the context now.
		delete pContext;
		pContext = NULL;
	}
	return status;
}



//************************************************************************
//
//	DEFINE LOOP DESCRIPTOR TABLE
//	
//************************************************************************
STATUS DdmFCM::
FCMDefineLoopDescriptorTable(CONTEXT *pContext)
{
	STATUS		status;

	// Allocate an Define Table object
	TSDefineTable *pDefineTable = new TSDefineTable;

	// Initialize the define Table object.
	status = pDefineTable->Initialize(
		this,									// DdmServices* pDdmServices,
		LOOP_DESCRIPTOR_TABLE,					// String64 prgbTableName,
		(fieldDef*)Loop_Descriptor_Table_FieldDefs,			// fieldDef* prgFieldDefsRet,
		cbLoop_Descriptor_Table_FieldDefs,		// U32 cbrgFieldDefs,
		FCM_TBL_RSRV_ENTRIES,					// U32 cEntriesRsv,
		true,									// bool* pfPersistant,
		(pTSCallback_t)&DdmFCM::FCMProcessDefineDescriptorTablesReply,	// pTSCallback_t pCallback,
		pContext								// void* pContext
	);
	
	// Initiate the define table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pDefineTable->Send();

	return status;
}



//************************************************************************
//
//	DEFINE WWN TABLE
//	
//************************************************************************
STATUS DdmFCM::
FCMDefineWWNTable(CONTEXT *pContext)
{
	STATUS		status;

	// Allocate an Define Table object
	TSDefineTable *pDefineTable = new TSDefineTable;

	// Initialize the define Table object.
	status = pDefineTable->Initialize(
		this,									// DdmServices* pDdmServices,
		WWN_TABLE,								// String64 prgbTableName,
		(fieldDef*)WWNTable_FieldDefs,			// fieldDef* prgFieldDefsRet,
		sizeofWWNTable_FieldDefs,				// U32 cbrgFieldDefs,
		FCM_TBL_RSRV_ENTRIES,					// U32 cEntriesRsv,
		true,									// bool* pfPersistant,
		(pTSCallback_t)&DdmFCM::FCMProcessDefineDescriptorTablesReply,	// pTSCallback_t pCallback,
		pContext								// void* pContext
	);
	
	// Initiate the define table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pDefineTable->Send();

	return status;
}



