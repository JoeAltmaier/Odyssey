/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: RdIsm.h
//
// Description:
// This class is a BSA Ram Disk ISM 
// 
// Update Log: 
// 03/11/99 Michael G. Panas: Create file from BsaIsm.h
/*************************************************************************/

#ifndef __RdIsm_h
#define __RdIsm_h

#include "Nucleus.h"
#include "Ddm.h"
#include "RDCONFIG.h"


class RdIsm: public Ddm {
public:

	RD_CONFIG config;
	
	VDN		myVd;
	
	RdIsm(DID did);

	static
	Ddm *Ctor(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);
	STATUS Quiesce(Message *pMsg);
	
	unsigned char	*pRam;				// points to ram area for new
	unsigned char	*pEnd;				// points to end of ram data area
	unsigned		num_sectors;		// last sector number

	// Update StorageRollcall table entry
	STATUS	RdTableInitialize(Message *pMsg);
	STATUS 	RdTblUpdReadRollCall(void *pClientContext, STATUS status);
	STATUS	RdTblUpdAddRollCall(void *pClientContext, STATUS status);
	STATUS 	RdTblUpdModifyRollCall(void *pClientContext, STATUS status);
	STATUS 	RdTableUpdateRCEnd(void *pClientContext, STATUS status);
	
	
protected:

	STATUS DoWork(Message *pMsg);
	
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
	U32					m_nTableRowsRead;
	U32					m_numBytesEnumed;

};

#endif