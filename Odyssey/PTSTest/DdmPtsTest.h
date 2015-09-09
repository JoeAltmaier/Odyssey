/* DdmPtsTest.h -- Test Serial Communications Channel DDM
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
 * Revision History:
 *     10/19/98 Tom Nelson: Created
 *
**/

#ifndef __DdmPtsTest_H
#define __DdmPtsTest_H

#include "OsTypes.h"
#include "CtTypes.h"
#include "Message.h"
#include "Ddm.h"
#include "DiskStatusTable.h"
#include "ReadTable.h"
#include "Listen.h"
#include "Table.h"
#include "Rows.h"

class TestServices : public DdmServices {

	U16 id;
	VDN vdn;
	char *pBuf;
	Ddm *m_pDdm;
	U32	m_EnumCount;

public:
	TestServices(Ddm *pDdm) : DdmServices(pDdm), m_pDdm(pDdm) {};
	STATUS Start(U16 _id,VDN _vdn);
	STATUS Reply1(void *pClientContext, STATUS status);
	STATUS Reply2(void *pClientContext, STATUS status);
	STATUS Reply3(void *pClientContext, STATUS status);
	STATUS Reply4(void *pClientContext, STATUS status);
	STATUS Reply5(void *pClientContext, STATUS status);
	STATUS Reply6(void *pClientContext, STATUS status);
	STATUS Reply7(void *pClientContext, STATUS status);
	STATUS Reply8(void *pClientContext, STATUS status);
	STATUS Reply9(void *pClientContext, STATUS status);
	STATUS ReplyA(void *pClientContext, STATUS status);
	STATUS ReplyB(void *pClientContext, STATUS status);            	//added two new message handlers to
	STATUS ReplyC(void *pClientContext, STATUS status);		//test TSDeleteRow functionality  LWE 5/13/99
	STATUS ListenReply(void *pClientContext, STATUS status);

private:
	TSDefineTable		*m_pDefineTable;
	TSInsertRow			*m_pInsertRow;
	TSReadRow			*m_pSRCReadRow;
	TSEnumTable			*m_pEnumTable;
	TSModifyRow			*m_pModifyRow;
	TSReadTable			*m_pReadTable;
	TSDeleteRow			*m_pDeleteRow;				//added delete row functionality LWE 5/13/99
	TSListen			*m_pListen;	
	rowID				m_RowID1;
	rowID				m_RowID2;
	DiskStatusRecord	*m_pTableRows;
	U32					m_nTableRows;
	DiskStatusRecord	m_DiskStatusRecord1;
	DiskStatusRecord	m_DiskStatusRecord2;
	DiskStatusRecord	m_DiskStatusTable[10];
	U32					m_numBytesEnumed;
	DiskStatusRecord	*m_pDiskStatusTable;
	U32					m_cbDiskStatusTable;
	U32					m_RowsDel;
	U32					m_ListenerID;
	U32					*m_pListenReplyType;
	DiskStatusRecord	*m_pModifiedDSTRecord;
	U32					m_cbModifiedDSTRecord;
	fieldDef			*m_pciDiskStatusTable_FieldDefs;
	char				*m_pcinewDiskStatusRecord;
	char				*m_pcinewDiskStatusRecord2;	
};

class DdmPtsTest: public Ddm {
	struct {
		VDN vd;
		U16 id;
	} config;

	char fill[6];	// Fixup Metrowerks alignment bug (v3.3 and earlier).  Member classes fail to
					// align properly and faults (or locks up) during the parent classes constructor.
					// Better to allocate member classes with new and avoid this problem since you
					// will never know how big the Ddm base class is/will be.
							
	TestServices services;
	
public:
	DdmPtsTest(DID did);
	static Ddm *Ctor(DID did);
	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);	
};
#endif	// __DdmSccTest_H

