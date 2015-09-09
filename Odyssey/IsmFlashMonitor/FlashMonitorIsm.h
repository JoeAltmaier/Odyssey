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
// File: FlashMonitorIsm.h
// 
// Description:
// This file is the interface for FlashMonitorIsm module. 
// 
// $Log: /Gemini/Odyssey/IsmFlashMonitor/FlashMonitorIsm.h $
// 
// 2     10/22/99 11:31a Hdo
// Re-write of the table methods
// 
// 1     10/11/99 4:56p Hdo
// Initial check in
// 
// 07/13/99 Huy Do: Create file
/*************************************************************************/

#define _DEBUG

#if !defined(FlashMonitorIsm_H)
#define FlashMonitorIsm_H

#include "OsTypes.h"
#include "Odyssey.h"

#include "Ddm.h"

#include "SsdDescriptor.h"
#include "StorageRollCallTable.h"

#include "FMConfig.h"

#include "Table.h"
#include "ReadTable.h"
#include "Rows.h"
#include "Fields.h"
#include "Listen.h"

class FlashMonitorIsm : public Ddm {
public:
	FlashMonitorIsm(DID did);
	static Ddm *Ctor(DID did);
	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);
	STATUS Quiesce(Message *pMsg);

	// SSD Descriptor Table update handlers
	STATUS Desc_Table_Initialize(Message *pMsg);
	STATUS Desc_Read_Reply(void *pContext, STATUS status);

	// StorageRollCall Table update handlers
	STATUS SRC_Table_Initialize(void *pContext, STATUS status);
	STATUS SRC_Read_Reply(void *pContext, STATUS status);

	// SSD Descriptor Table listen handlers
	STATUS Desc_Start_Listen(void *pContext, STATUS status);
	STATUS Desc_Listen_Reply(void *pContext, STATUS status);

	// StorageRollCall Table listen handlers
	STATUS SRC_Start_Listen(void *pContext, STATUS status);
	STATUS SRC_Listen_Reply(void *pContext, STATUS status);

protected:
	FM_CONFIG		m_config;

	rowID			m_rid_descriptor;
	rowID			m_rid_SRC;
	U32				m_NumRowRead;
	U32				m_ListenDescSize;
	U32				m_ListenSRCSize;
	U32				*m_pListenType;

	SSD_Descriptor	*m_p_Desc_Row;
	StorageRollCallRecord *m_p_SRC_Row;

	TSReadRow		*m_pReadRow;
	TSModifyRow		*m_pModifyRow;
	TSListen		*m_pListen;

	//STATUS DoWork(Message *pMsg);
} ;
#endif /* FlashMonitorIsm_H  */
