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
// This class is the Fcp Target device HDM.
// 
// Update Log:
//	$Log: /Gemini/Odyssey/HdmFcpTarget/HdmFcpTarget.h $
// 
// 4     1/09/00 8:14p Mpanas
// Support for Read of Export table using the vdNext field
// as the key (Claim VC support)
// 
// 3     9/13/99 5:34p Vnguyen
// Add PHS Reporter performance and status counters.
// 
// 2     8/20/99 7:50p Mpanas
// Changes to support Export Table states
// Re-organize sources
// 
// 1     7/15/99 11:48p Mpanas
// Changes to support Multiple FC Instances
// and support for NAC
// -New Target DDM project
// 
// 07/02/99 Michael G. Panas: Create file from HdmFcpNic.h
/*************************************************************************/

#ifndef __HdmFcpTarget_h
#define __HdmFcpTarget_h

#include "Ddm.h"
#include "FcpConfig.h"
#include "ExportTable.h"
#include "HdmFcp.h"

#define		HASH_TABLE_LEN				16384		// 16k to start with

#include "FCPTargetStatus.h"
#include "FCPTargetPerformance.h"

class HdmFcpTarget: public HdmFcp {
public:

typedef U32 (HdmFcpTarget::*pHandler_t)(void *, void *);
public:
typedef STATUS (HdmFcpTarget::*pTgtCallback_t)(void *, STATUS);

	// configuration data stored in the PDS
	FCP_CONFIG		config;
	
	HdmFcpTarget(DID did);
	static
	Ddm *Ctor(DID did);

	VDN				MyVd;		// my virtual device number
	DID				MyDid;		// for PHS Reporter
	bool	fStatusReporterActive;		// True if the Status Reporter has been started for this Did
	bool	fPerformanceReporterActive; // True if the Performance Reporter has been started for this Did
			// The reason we need these two flags is because we start the
			// reporters in enable() and stop them in quiesce().  At this
			// time I am not sure if there is a matching one to one calling
			// sequence between these two methods.


	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);
	STATUS Quiesce(Message *pMsg);
	
	void Target_Build_Tables();
	
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
	// Table methods for the FCP Target HDM
	STATUS	TarTableInitialize(Message *pMsg);
	STATUS 	TarTblReply2(void *pClientContext, STATUS status);
	STATUS 	TarTblReply3(void *pClientContext, STATUS status);
	STATUS 	TarTblReplyLast(void *pClientContext, STATUS status);
	// Export table state update
	STATUS 	TarTblModifyExportState(rowID *pRow, CTReadyState *pState);
	STATUS 	TarTblModifyExportStateCallback(void *pClientContext, STATUS status);
	
	// Export Table read using vdNext as key
	STATUS	TarTblMatchVdn(void *pContext, pTgtCallback_t Callback,
								void *buffer, U32 length, VDN vdn);
	STATUS 	TarTblMatchVdnReplyLast(void *pClientContext, STATUS status);

	// TargetTableListen.cpp
	STATUS 	TarStartListen(U32 LoopInstance);
	STATUS	TarExportListenUpdate(void *pClientContext, STATUS status);
		
	// PTS Table Data
	ExportTableEntry		*Tar_TS_Export;		// one for each slot
	
	// Local Table Data
	ExportTableEntry		*Tar_Export;		// one for each slot
	
	// pointer to HASH table for LUN/ID pairs
	struct _LunXlate 		**pXlt;
	
	U32						num_xport_entrys;
	U32						num_valid_exports;
	
protected:

	STATUS DoWork(Message *pMsg);

	// FCP Target Status Record
	FCPT_Status			*m_pStatus;
	
	// FCP Target Performance Record
	FCPT_Performance	*m_pPerformance;
	
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
	HdmFcpTarget::pHandler_t	 Handler;			// saved Callback address
	rowID						 Row;				// Export table row ID
} Xlt;


#endif