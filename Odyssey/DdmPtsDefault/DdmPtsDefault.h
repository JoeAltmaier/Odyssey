/* DdmPtsDefault.h -- SystemEntry Ddm to load PTS defaults
 *
 * Copyright (C) ConvergeNet Technologies, 1999
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
**/

// Revision History:
//	$Log: /Gemini/Odyssey/DdmPtsDefault/DdmPtsDefault.h $
// 
// 2     7/08/99 3:28p Mpanas
// Change to match latest Oos changes
// 
// 1     6/30/99 7:38p Mpanas
// New SystemEntry DDM to initialize PTS tables before anyone executes a
// request
//
//  6/30/99 Tom Nelson: Create file


#ifndef __DdmPtsDefault_h
#define __DdmPtsDefault_h

#include "Nucleus.h"
#include "Ddm.h"
#include "RqOsDdmManager.h"

//
//  DdmPtsDefault Class
//
class DdmPtsDefault : public Ddm {

public:
	static Ddm *Ctor(DID did) 	{ return new DdmPtsDefault(did); }

	DdmPtsDefault(DID did);

	ERC Initialize(Message *pArgMsg);
	ERC Enable(Message *pArgMsg);
	
	// Table methods for the Pts Default value
	STATUS	PtsDefInitialize(Message *pMsg);
	STATUS 	PtsDefReply1(void *pClientContext, STATUS status);
	//STATUS 	PtsDefReply2(void *pClientContext, STATUS status);
	//STATUS 	PtsDefReply3(void *pClientContext, STATUS status);
	//STATUS 	PtsDefReply4(void *pClientContext, STATUS status);
	//STATUS 	PtsDefReply5(void *pClientContext, STATUS status);
	//STATUS 	PtsDefReply6(void *pClientContext, STATUS status);
	//STATUS 	PtsDefReply7(void *pClientContext, STATUS status);
	STATUS 	PtsDefReplyLast(void *pClientContext, STATUS status);
	
private:
	TSDefineTable*		m_pDefineTable;
	TSGetTableDef*		m_pTSGetTableDef;
	TSInsertRow*		m_pInsertRow;
	TSReadRow*			m_pSRCReadRow;
	TSEnumTable*		m_EnumTable;
	TSModifyRow*		m_ModifyRow;
	TSReadTable*		m_ReadTable;
	
	rowID				m_RowID1;
	rowID				m_RowID2;
	U32					m_Index1;
	U32					m_Index2;
	U32					m_nTableRows;
	U32					m_numBytesEnumed;

};

#endif // __DdmPtsDefault_h

