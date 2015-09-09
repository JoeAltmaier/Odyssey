/*
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
 * DdmPnP.h -- DDM
 *
 * Revision History:
 *		03/19/99 Huy Do: Created
 *		03/20/99 Huy Do: change param in HandleReplies from
 *						 Message to Context-status
 *		04/20/99 Huy Do: add private intances for the DefineTable
 *		04/23/99 Huy Do: add private intances for the EnumTable
 *		05/12/99 Huy Do: change GetTableDef's private intances
 *						 to GET_TABLE_DEF_REPLY_PAYLOAD
**/

#ifndef __DdmPnP_H__
#define __DdmPnP_H__

#include "Ddm.h"
#include "PnPMsg.h"
#include "Table.h"
#include "Rows.h"
#include "Fields.h"
#include "Listen.h"

class DdmPnP: public Ddm {
protected:
	// These are private instances for DefineTable
	TSDefineTable	*m_pDefTbl;
	rowID			m_ReplyRowID;

	// These are private instances for GetTableDef
	TSGetTableDef	*m_pGetTblDef;
	GET_TABLE_DEF_REPLY_PAYLOAD m_GetTableDefReplyPayload;

	// These are private instances for EnumTable
	TSEnumTable		*m_pEnumTbl;
	ENUM_TABLE_REPLY_PAYLOAD m_EnumTableReplyPayload;

	TSInsertRow		*m_pInsertRow;
	TSDeleteTable	*m_pDeleteTable;
	TSReadRow		*m_pReadRow;
	READ_ROW_REPLY_PAYLOAD m_ReadRowReplyPayload;

	TSDeleteRow		*m_pDeleteRow;
	U32				m_cRowsDeleted;

	TSModifyRow		*m_pModifyRow;
	TSModifyField	*m_pModifyField;
	U32				m_cRowsModified;
	rowID			*m_pReturnedRowID;

	TSListen		*m_pListen;

	// These are the Message handler
	STATUS CreateTable(Message *pReqMsg);
	STATUS GetTableDef(Message *pReqMsg);
	STATUS EnumTable  (Message *pReqMsg);
	STATUS InsertRow  (Message *pReqMsg);
	STATUS DeleteTable(Message *pReqMsg);
	STATUS ModifyRow(Message *pReqMsg);
	STATUS ReadRow(Message *pReqMsg);
	STATUS DeleteRow(Message *pReqMsg);
	STATUS ModifyField(Message *pReqMsg);
	STATUS Listen(Message *pReqMsg);
	STATUS Connect(Message *pReqMsg);

	// These are the Reply handler
	STATUS HandleCreateTable(void *pContext, STATUS status);
	STATUS HandleGetTableDef(void *pContext, STATUS status);
	STATUS HandleEnumTable  (void *pContext, STATUS status);
	STATUS HandleInsertRow  (void *pContext, STATUS status);
	STATUS HandleDeleteTable(void *pContext, STATUS status);
	STATUS HandleModifyRow(void *pContext, STATUS status);
	STATUS HandleReadRow(void *pContext, STATUS status);
	STATUS HandleDeleteRow(void *pContext, STATUS status);
	STATUS HandleModifyField(void *pContext, STATUS status);
	STATUS HandleListen(void *pContext, STATUS status);

public:
	static Ddm *Ctor(DID did);
	DdmPnP(DID did);

	// Ddm Methods
	virtual STATUS Initialize(Message *pMsg);
	virtual STATUS Enable(Message *pMsg);
};

#endif	// __DdmPnP_H__
