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
// File: FlashTable.cpp
// 
// Description:
// This file is the implementation for FlashTable module. 
// 
// $Log: /Gemini/Odyssey/IsmFlashMonitor/FlashTable.cpp $
// 
// 2     10/22/99 11:31a Hdo
// Re-write of the table methods
// 
// 1     10/11/99 4:56p Hdo
// Initial check in
// 
// 07/23/99 Huy Do: Create file
/*************************************************************************/

#include "Trace_Index.h"
#define TRACE_INDEX TRACE_FLASH_MONITOR
#include "Odyssey_Trace.h"

#include "FlashMonitorIsm.h"

#define FM_SRC_LISTEN_MASK	(ListenOnTableChange)

/*************************************************************************/
// Desc_Table_Initialize
// Start of the table handlers, and listen on these tables
//  SSD_Descriptor
//  StorageRollCall
/*************************************************************************/
STATUS FlashMonitorIsm::Desc_Table_Initialize(Message *pMsg)
{
	TRACE_ENTRY(FlashMonitorIsm::Desc_Table_Initialize);

	STATUS			status;

	// Allocate a ReadRow object for the SSD Descriptor table.
	m_pReadRow = new(tUNCACHED) TSReadRow();

	// Initialize the read row operation.
	status = m_pReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		SSD_DESCRIPTOR_TABLE,				// rgbTableName,
		"SerialNumber",						// rgbKeyFieldName,
		&m_config.SerialNumber,				// pKeyFieldValue,
		sizeof(String32),					// cbKeyFieldValue,
		m_p_Desc_Row,						// prgbRowDataRet,
		sizeof(SSD_Descriptor),				// cbRowDataRetMax,
		&m_NumRowRead,						// pcRowsReadRet,
		(pTSCallback_t)&Desc_Read_Reply,	// pTSCallback_t pCallback,
		(void*)pMsg							// pContext
	);

	if( status == ercOK )
		m_pReadRow->Send();
	
	return status;
	
} // Desc_Table_Initialize

/*************************************************************************/
// Desc_Read_Reply
// 
// 
/*************************************************************************/
STATUS FlashMonitorIsm::Desc_Read_Reply(void *pContext, STATUS status)
{
	TRACE_ENTRY(FlashMonitorIsm::Desc_Read_Reply);

	if( status == ercOK )
	{
		// TODO: if more than one entry match, then ......???
		//if( m_NumRowRead > 1 )
		//	;

		// Save data for local use
		m_rid_descriptor = m_p_Desc_Row->rid;
		status = SRC_Table_Initialize(pContext, ercOK);
	}
	else if( status == ercKeyNotFound )
		status = Desc_Start_Listen(pContext, ercOK);

	return status;

} // Desc_Read_Reply


/*************************************************************************/
// SRC_Table_Initialize
// Read the SRC row.
/*************************************************************************/
STATUS FlashMonitorIsm::SRC_Table_Initialize(void *pContext, STATUS status)
{
	TRACE_ENTRY(FlashMonitorIsm::SRC_Table_Initialize);

	// Allocate a ReadRow object for the DiskStatusTable.
	m_pReadRow = new(tUNCACHED) TSReadRow();

	// Initialize the read row operation.
	status = m_pReadRow->Initialize(
		this,								// pDdmServices,
		STORAGE_ROLL_CALL_TABLE,			// rgbTableName,
		fdSRC_DESC_RID,						// rgbKeyFieldName,
		&m_rid_descriptor,					// pKeyFieldValue,
		sizeof(rowID),						// cbKeyFieldValue,
		m_p_SRC_Row,						// prgbRowDataRet,
		sizeof(StorageRollCallRecord),		// cbRowDataRetMax,
		&m_NumRowRead,						// pcRowsReadRet,
		(pTSCallback_t)&SRC_Read_Reply,		// pCallback,
		pContext							// pContext
	);

	if (status == ercOK)
		m_pReadRow->Send();
	
	return status;
	
} // SRC_Table_Initialize


/*************************************************************************/
// SRC_Read_Reply
// Reply handler from the SRC_Table_Initialize
// 
/*************************************************************************/
STATUS FlashMonitorIsm::SRC_Read_Reply(void *pContext, STATUS status)
{
	TRACE_ENTRY(FlashMonitorIsm::SRC_Read_Reply);

	if( status == ercOK )
	{
		// TODO: if more than one entry match, then ......???
		//if( m_NumRowRead > 1 )
		//	;

		// Save data for local use
		m_rid_SRC = m_p_Desc_Row->rid;
		// Reply to the Initialized pMsg
		Reply((Message *)pContext);
	}
	else if( status == ercKeyNotFound )
		status = SRC_Start_Listen(pContext, ercOK);

	return status;
	
} // SRC_Read_Reply

/*************************************************************************/
// Desc_Start_Listen
// Start a Listen on insert a SSD Descriptor record
// 
/*************************************************************************/
STATUS FlashMonitorIsm::Desc_Start_Listen(void *pContext, STATUS status)
{
	TRACE_ENTRY(FlashMonitorIsm::Desc_Start_Listen);

	m_ListenDescSize = sizeof(SSD_Descriptor);

	// Allocate a Listen object for the SSD_Descriptor table.
	m_pListen = new(tUNCACHED) TSListen();

	// Initialize the Listen operation.
	status = m_pListen->Initialize(
		this,								// DdmServices pDdmServices,
		ListenOnTableChange,				// U32		ListenType,
		SSD_DESCRIPTOR_TABLE,				// String64	prgbTableName,
		"SerialNumber",						// String64	prgbRowKeyFieldName,
		&m_config.SerialNumber,				// void*	prgbRowKeyFieldValue,
		sizeof(String32),					// U32		cbRowKeyFieldValue,
		NULL,								// String64	prgbFieldName,
		NULL,								// void*	prgbFieldValue,
		0,									// U32		cbFieldValue,
		(ReplyWithRow|ReplyContinuous),		// U32		ReplyMode, (1=once)
		NULL,								// void**	ppTableDataRet,
		0,									// U32*		pcbTableDataRet,
		NULL,								// U32*		pListenerIDRet,
		&m_pListenType,						// U32**	ppListenTypeRet,
		&m_p_Desc_Row,						// void**	ppModifiedRecordRet,
		&m_ListenDescSize,					// U32*		pcbModifiedRecordRet,
		(pTSCallback_t)&Desc_Listen_Reply,	// pTSCallback_t pCallback,
		pContext							// void*	pContext
	);

	if (status == ercOK)
		m_pListen->Send();

	return status;

} // Desc_Start_Listen

/*************************************************************************/
// Desc_Listen_Reply
// Reply from the Listen on SSD Descriptor
// 
/*************************************************************************/
STATUS FlashMonitorIsm::Desc_Listen_Reply(void *pContext, STATUS status)
{
	TRACE_ENTRY(FlashMonitorIsm::Desc_Listen_Reply);

	// no work on first reply
	if (*m_pListenType & ListenInitialReply)
	{
		// Save the data
		m_rid_descriptor = m_p_Desc_Row->rid;

		// Call the SRC table handler
		status = SRC_Table_Initialize(pContext, ercOK);
	}
	else
	{
	}

	return status;

} // Desc_Listen_Reply

/*************************************************************************/
// SRC_Start_Listen
// Start a Listen on insert a StorageRollCall record
// 
/*************************************************************************/
STATUS FlashMonitorIsm::SRC_Start_Listen(void *pContext, STATUS status)
{
	TRACE_ENTRY(FlashMonitorIsm::SRC_Start_Listen);

	m_ListenSRCSize = sizeof(StorageRollCallRecord);

	// Allocate a Listen object for the StorageRollCall table.
	m_pListen = new(tUNCACHED) TSListen();

	// Initialize the Listen operation.
	status = m_pListen->Initialize(
		this,								// DdmServices pDdmServices,
		ListenOnTableChange,				// U32		ListenType,
		STORAGE_ROLL_CALL_TABLE,			// String64	prgbTableName,
		fdSRC_DESC_RID,						// String64	prgbRowKeyFieldName,
		(void*)&m_rid_descriptor,			// void*	prgbRowKeyFieldValue,
		sizeof(rowID),						// U32		cbRowKeyFieldValue,
		NULL,								// String64	prgbFieldName,
		NULL,								// void*	prgbFieldValue,
		0,									// U32		cbFieldValue,
		(ReplyWithRow|ReplyContinuous),		// U32		ReplyMode, (1=once)
		NULL,								// void**	ppTableDataRet,
		0,									// U32*		pcbTableDataRet,
		NULL,								// U32*		pListenerIDRet,
		&m_pListenType,						// U32**	ppListenTypeRet,
		&m_p_SRC_Row,						// void**	ppModifiedRecordRet,
		&m_ListenSRCSize,					// U32*		pcbModifiedRecordRet,
		(pTSCallback_t)&SRC_Listen_Reply,	// pTSCallback_t pCallback,
		pContext							// void*	pContext
	);

	if (status == ercOK)
		m_pListen->Send();

	return status;

} // SRC_Start_Listen

/*************************************************************************/
// SRC_Listen_Reply
// Reply from the Listen on StorageRollCall record
// 
/*************************************************************************/
STATUS FlashMonitorIsm::SRC_Listen_Reply(void *pContext, STATUS status)
{
	TRACE_ENTRY(FlashMonitorIsm::SRC_Listen_Reply);

	// no work on first reply
	if (*m_pListenType & ListenInitialReply)
	{
		// Save the data
		m_rid_SRC = m_p_SRC_Row->rid;

		// Initialize a ModifyRow object to update the vdnMonitor field
		m_pModifyRow = new TSModifyRow();
		m_p_SRC_Row->vdnMonitor = GetVdn();

		status = m_pModifyRow->Initialize(
			this,
			STORAGE_ROLL_CALL_TABLE,
			"rid",
			&m_rid_SRC,
			sizeof(rowID),
			m_p_SRC_Row,
			sizeof(StorageRollCallRecord),
			1,
			NULL,
			NULL,
			0,
			NULL,
			NULL
		);

		if( status == ercOK )
			m_pModifyRow->Send();

		// Reply to the Initialized pMsg
		Reply((Message *)pContext);
	}
	else
	{
		// TODO: Update record
	}

	return status;

} // SRC_Listen_Reply

