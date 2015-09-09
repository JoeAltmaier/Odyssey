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
// File: FlashTable.h
// 
// Description:
// This file is the interface for FlashTable module. 
// 
// $Log: /Gemini/Odyssey/IsmFlashMonitor/FlashTable.h $
// 
// 1     10/11/99 4:56p Hdo
// Initial check in
// 
// 07/23/99 Huy Do: Create file
/*************************************************************************/

#if !defined(FlashTable_H)
#define FlashTable_H

#include "DdmOsServices.h"
#include "FlashFile.h"

#include "Listen.h"
#include "ReadTable.h"
#include "Rows.h"
#include "Table.h"

class FlashMonitorIsm;

class FlashTable : public DdmServices {
public:
	void FlashTable(FlashMonitorIsm *pDdm) { m_pDdm = pDdm; m_nTableRows = 0; }

	void SetDdm(FlashMonitorIsm *pDdm) { m_pDdm = pDdm; }
	//void SetFlashConfig(FF_CONFIG *pContext) { m_pConfig = pContext; }

	STATUS TableInitialize(Message *pMsg);
	STATUS RowInitialize(void *pContext, STATUS status);
	STATUS SRCInitialize(void *pContext, STATUS status);
	STATUS GetFDDef(void *pContext, STATUS status);
	STATUS ReadFDRecords(void *pContext, STATUS status);
	STATUS ReplyLast(void *pContext, STATUS status);
	STATUS TableUpdate(Message *pMsg);
	STATUS TableListen(Message *pMsg);

private:
	TSDefineTable	*m_pDefineTable;
	TSGetTableDef	*m_pGetTableDef;
	TSEnumTable		*m_pEnumTable;
	TSReadTable		*m_pReadTable;
	TSInsertRow		*m_pInsertRow;
	TSListen		*m_pListen;
	rowID			m_rid;
	U32				m_nTableRows;

	FlashMonitorIsm *m_pDdm;
	FF_CONFIG		*m_pConfig;
	typedef struct {
		Message					*pMsg;
		FlashDescriptor			*pFDRows;
		StorageRollCallRecord	*pSRCRows;
	} FD_TableContext;
} ;
#endif /* FlashTable_H  */
