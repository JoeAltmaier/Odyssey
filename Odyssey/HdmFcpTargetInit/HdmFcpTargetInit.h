/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This class is the combined Fcp Target and Initiator device HDM.
// 
// Update Log:
//	$Log: /Gemini/Odyssey/HdmFcpTargetInit/HdmFcpTargetInit.h $
// 
// 4     2/03/00 7:19p Jlane
// Add ContinueInitialize to redirect port 2 in slots 24 & 28 to DDHs only
// if DDHs exist.
// 
// 3     1/09/00 8:14p Mpanas
// Support for Read of Export table using the vdNext field
// as the key (Claim VC support)
// 
// 2     8/23/99 2:20p Mpanas
// Changes to support Export Table States
// 
// 1     7/15/99 11:48p Mpanas
// Changes to support Multiple FC Instances
// and support for NAC
// -New Target and Initiator DDM project
// 
// 07/06/99 Michael G. Panas: Create file from HdmFcpNic.h
/*************************************************************************/

#ifndef __HdmFcpTarget_h
#define __HdmFcpTarget_h

#include "Ddm.h"
#include "FcpConfig.h"
#include "ExportTable.h"
#include "HdmFcp.h"

#define		HASH_TABLE_LEN				16384		// 16k to start with


class HdmFcpTargetInit: public HdmFcp {
public:

typedef U32 (HdmFcpTargetInit::*pHandler_t)(void *, void *);
public:
typedef STATUS (HdmFcpTargetInit::*pTgtCallback_t)(void *, STATUS);

	// configuration data stored in the PDS
	FCP_CONFIG		config;
	
	HdmFcpTargetInit(DID did);
	static
	Ddm *Ctor(DID did);

	VDN				MyVd;		// my virtual device number

	STATUS Initialize(Message *pMsg);
	STATUS ContinueInitialize(Message *pMsg);
	STATUS Enable(Message *pMsg);
	STATUS Quiesce(Message *pMsg);
	
	void TI_Build_Tables();
	
	// TargetHash.cpp
	virtual VDN FCP_Find_Next_Virtual_Device(void *pCtx, U32 key);
	// Target Hash Table Methods
	U32		 Hash(U32 key);
	U32		 Add_Hash_Entry(U32 key, void *entry);
	void	*Remove_Hash_Entry(U32 key);
	void	*Find_Hash_Entry(U32 key);
	void	 Set_Handler(void *p);
	// Handler methods
	U32 	Handle_Busy_Resp(void *pContext, void *pX);
	U32 	Handle_Error_Resp(void *pContext, void *pX);
	U32 	Handle_Cmd_Resp(void *pContext, void *pX);
	U32 	Handle_Trigger_Resp(void *pContext, void *pX);

	// TargetTable.cpp
	// Table methods for the Target/Initiator HDM
	STATUS	TI_TableInitialize(Message *pMsg);
	STATUS 	TI_TblReply2(void *pClientContext, STATUS status);
	STATUS 	TI_TblReply3(void *pClientContext, STATUS status);
	STATUS 	TI_TblReplyLast(void *pClientContext, STATUS status);
	// Export table state update
	STATUS 	TI_TblModifyExportState(rowID *pRow, CTReadyState *pState);
	STATUS 	TI_TblModifyExportStateCallback(void *pClientContext, STATUS status);
	
	// Export Table read using vdNext as key
	STATUS	TI_TblMatchVdn(void *pContext, pTgtCallback_t Callback,
								void *buffer, U32 length, VDN vdn);
	STATUS 	TI_TblMatchVdnReplyLast(void *pClientContext, STATUS status);

	// TargetTableListen.cpp
	STATUS 	TI_StartListen(U32 LoopInstance);
	STATUS	TI_ExportListenUpdate(void *pClientContext, STATUS status);
		
	// PTS Table Data
	ExportTableEntry		*TI_TS_Export;		// one for each slot
	
	// Local Table Data
	ExportTableEntry		*TI_Export;		// one for each slot
	
	// pointer to HASH table for LUN/ID pairs
	struct _LunXlate 		**pXlt;
	
	U32						num_xport_entrys;
	U32						num_valid_exports;
	
	// array index of instance
	UNSIGNED	instance;
	
	// saved enable message cookie
	Message		*pEnableMsg;
		
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
	U32					m_numBytesEnumed;
	
	// Listen on Export Table
	TSListen*			m_Listen;
	U32	*				m_LType;		// Listen Type return
	U32					m_LSize;		// Size of returned row
	ExportTableEntry	*m_Export;		// returned entry
	
};


// Local Translation table indexed by ID/LUN
typedef struct _LunXlate {
	_LunXlate					*pNext;				// next table entry
	union {
		IDLUN					 idlun;				// key
		long					 l;
	} key;
	VDN							 vd;				// next vitual device number
	CTReadyState				 state;				// current state
	HdmFcpTargetInit::pHandler_t Handler;			// saved Callback address
	rowID						 Row;				// Export table row ID
} Xlt;



#endif