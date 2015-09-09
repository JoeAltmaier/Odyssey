/*************************************************************************/
// 
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: LoopMonitorIsm.h
// 
// Description:
// This file contains the derived class for the LoopMonitorIsm
// 
// Update Log 
//	$Log: /Gemini/Odyssey/IsmLoopMonitor/LoopMonitorIsm.h $
// 
// 6     2/07/00 9:22p Cchan
// Added VDN reference in the LM_ET_STATE container class, and a global
// reset counter per loop chip variable, to support LIP reset handling.
// 
// 5     9/15/99 6:43p Mpanas
// Fix race condition, AE comes in before Initialize() is complete
// 
// 4     8/14/99 9:28p Mpanas
// LoopMonitor version with most functionality
// implemented
// - Creates/Updates LoopDescriptor records
// - Creates/Updates FCPortDatabase records
// - Reads Export Table entries
// Note: This version uses the Linked List Container types
//           in the SSAPI Util code (SList.cpp)
// 
// 3     7/23/99 1:40p Mpanas
// Add latest versions of the Loop code
// so we can use the fix in FCP Lib
// for Immediate Notify handling
// 
// 1     6/18/99 6:46p Mpanas
// Initial version of the LoopMonitor
// for ISP2200 support
// 
// 06/11/98 Michael G. Panas: Create file
/*************************************************************************/


#ifndef __LoopMonitorIsm_h
#define __LoopMonitorIsm_h

#include "Ddm.h"			// include base class
#include "LmConfig.h"
#include "FcpData.h"
#include "FcpEvent.h"
#include "FcpLoop.h"

#include "Slist.h"			// includes Container.h (from SSAPI Util)


// FCPortDatabase state structure
// Keyed internally by FC ID
struct LM_FCDB_STATE {
	U32						 state;		// Entry state
	U32						 num_LUNs;	// number of exported LUNs on this ID
	FCPortDatabaseRecord	*pDBR;		// FC Port Database record
	rowID					 ridDBR;	// row ID of this PTS entry
};

// LM_FCDB_STATE state values
#define	FCDB_STATE_ADD		0			// need to add record
#define	FCDB_STATE_UPDATE	1			// row exists, update only
#define	FCDB_STATE_DELETE	2			// remove row
#define	FCDB_STATE_INACTIVE	3			// ID is inactive

// Export Table state/info structure
// Keyed by lower 32 bits (LoPart) of the row ID
struct LM_ET_STATE {
	CTReadyState			 state;		// last state
	U32						 flags;		// internal flags
	IDLUN					 idlun;		// Host ID, Target ID, LUN
	rowID					 ridET;		// row ID of this PTS entry
	VDN						 vdNext;	// Next Virual Device #
};

// define the derived class for the FC Loop Monitor ISM
class LoopMonitorIsm: public Ddm {
public:

typedef STATUS (LoopMonitorIsm::*pLMCallback_t)(void *, STATUS);

	// Data recovered from the Persistent Data Service
	LM_CONFIG config;
	
	LoopMonitorIsm(DID did);
	VDN		MyVd;			// my virtual device number

	static
	Ddm *Ctor(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);
	STATUS Quiesce(Message *pMsg);
	
	void LM_Do_Callback(void *ptr);

	void LM_LIP(U32 loop);
	void LM_Get_Pos_Map(U32 loop);
	U32	 LM_Get_State(U32 loop, U16 *state);
	
	// member methods to handle scan - LoopScan.cpp
	void LM_Scan_Loops(Message *pMsg);
	void LM_Scan_A_Loop(U32 FCinstance);
	void LM_Send_DM_SCAN(U32 chip );
	void LM_Scan_Callback(void *pClientContext, STATUS status);
	
	// member methods to handle Virtual Ports - LoopMultiTarget.cpp
	U32  LM_MT_OK(U32 chip);
	void LM_Expose_IDs(U32 loop);
	void LM_Get_VP_DB(U32 loop);
	void LM_Enable_Loop_ID(void *pLmc, U16 VP_Count, U8 *VP_Indexes);
	void LM_Disable_Loop_ID(void *pLmc, U16 VP_Count, U8 *VP_Indexes);
	void LM_Modify_VP_Config(void *pLmc, U16 VP_Count, U8 *VP_Indexes,
				VP_CONFIG *vp_config, U8 command);
	void LM_Build_VP_Config(VP_CONFIG *pVp, U32 id, U32 LoopInstance);
	void LM_Do_Expose_Callback_Modify_VP(void *ptr, STATUS status);
	void LM_Do_Expose_Callback_Enable(void *ptr, STATUS status);
	void LM_Do_Expose_Callback_Last(void *ptr, STATUS status);

	// member methods to handle Async Events - LoopAEHandler.cpp
	void LM_Handle_Loop_Up(PINSTANCE_DATA Id);
	void LM_Handle_Loop_Down(PINSTANCE_DATA	Id);
	void LM_Handle_Loop_LIP(PINSTANCE_DATA	Id);
	void LM_Handle_Loop_LIP_Reset(PINSTANCE_DATA Id);

	// member method to handle the FC_Master interface
	STATUS	LM_Handle_FC_Master(PINSTANCE_DATA	Id, U32 command);

	// Table methods for the loop monitor
	
	// LoopTableInit.cpp
	STATUS	LmTableInitialize(Message *pMsg);
	STATUS 	LmTblReply1(void *pClientContext, STATUS status);
	STATUS 	LmTblReply2(void *pClientContext, STATUS status);
	STATUS 	LmTblReply3(void *pClientContext, STATUS status);
	STATUS 	LmTblReply4(void *pClientContext, STATUS status);
	STATUS 	LmTblReply5(void *pClientContext, STATUS status);
	STATUS 	LmTblReply6(void *pClientContext, STATUS status);
	STATUS 	LmTblReply7(void *pClientContext, STATUS status);
	STATUS 	LmTblReplyLast(void *pClientContext, STATUS status);
	void 	LmTblReplyCheckExports(U32 loop);
	
	// LoopTable.cpp
	// Update LoopDescriptor Table
	STATUS	LmLpTableUpdate(Message *pMsg, void *pContext, U32 LoopId, pLMCallback_t Callback);
	STATUS	LmTblUpdReadLpDesc(void *pClientContext, STATUS status);
	STATUS	LmTblUpdAddLpDesc(void *pClientContext, STATUS status);
	STATUS 	LmTblUpdModifyLpDesc(void *pClientContext, STATUS status);
	STATUS 	LmLpTableUpdateEnd(void *pClientContext, STATUS status);
	// Update FCPortDatabase Table
	STATUS	LmDBTableUpdate(LM_FCDB_STATE *pDBState,
					void *pContext,  pLMCallback_t Callback);
	STATUS	LmTblUpdAddDBEntry(void *pClientContext, STATUS status);
	STATUS 	LmTblUpdModifyDBEntry(void *pClientContext, STATUS status);
	STATUS 	LmDBTableUpdateEnd(void *pClientContext, STATUS status);
	// Update ReadyState of an Export Table entry
	STATUS 	LmTblModifyExportState(rowID *pRow, CTReadyState *pState);
	STATUS 	LmTblModifyAllExportState(U32 *pLoopInstance, CTReadyState *pState);
	// Update portStatus of all FCPortDatabase Table entries on al loop
	STATUS 	LmTblModifyPortDBStatus(rowID *pLoopDesc, U32 *pStatus);

	// LoopTableListen.cpp
	void	LmListenInitialize(void);
	STATUS 	LmStartListens(U32 LoopInstance, U32 Index);
	STATUS	LmExportListenUpdate(void *pClientContext, STATUS status);
	STATUS	LmExportListenUpdateEnd(void *pClientContext, STATUS status);
	STATUS	LmLoopListenUpdate(void *pClientContext, STATUS status);
		
	// Init counter
	U32		m_next;
	
	// configured flag 0 = yes, 1 = no
	U32		m_config_done;
	
	// define the LoopFlags
#define	LM_STS_LOOP_UNDEF			0
#define	LM_STS_LOOP_UP				1
#define	LM_STS_LOOP_DOWN			2
#define	LM_STS_LOOP_LIP				3
#define	LM_STS_LOOP_DM_SCAN_REQ		4	// next state = LM_STS_LOOP_UP
#define	LM_STS_LOOP_ERRORS			5	// too many LIPS or LIP_RESET

	// Loop Monitor Data, one per chip
	U32		LoopFlags[MAX_FC_CHIPS];
	U32		num_IDs[MAX_FC_CHIPS];
	U32		num_LIPs[MAX_FC_CHIPS];
	U32		num_resets[MAX_FC_CHIPS];
	
	// Multiple Target ID info
	U32		m_num_IDs[MAX_FC_CHIPS];
	U8		*m_ids[MAX_FC_CHIPS];
	U8		*m_num_luns[MAX_FC_CHIPS];
	
	// PTS Table Data
	LoopDescriptorEntry		*LM_TS_Loop_Desc[MAX_FC_CHIPS];		// one for each loop
	
	// Local Table Data
	LoopDescriptorEntry		*LM_Loop_Desc[MAX_FC_CHIPS];		// one for each loop configed
	
	// PTS Table Data
	ExportTableEntry		*Lm_TS_Export[MAX_FC_CHIPS];		// one for each slot
	U32						 nExportRows[MAX_FC_CHIPS];
	
	Container				*pPDB[MAX_FC_CHIPS];
	Container				*pETS[MAX_FC_CHIPS];
	
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
	
	// Listen on LoopDescriptor 
	TSListen*			m_Listen;
	U32	*				m_LType;		// Listen Type return
	U32					m_LSize;		// Size of returned row
	LoopDescriptorEntry	*m_Loop_Desc;	// return entry

	// Listen on Export Table
	TSListen*			m_Listen1;
	U32	*				m_LType1;		// Listen Type return
	U32					m_LSize1;		// Size of returned row
	ExportTableEntry	*m_Export;		// returned entry
	
};


// Loop Monitor internal context
typedef struct _LM_Context {
	LoopMonitorIsm::pLMCallback_t	 Callback;			// saved Callback address
	U32				loop_number;		// FC Loop instance on this board
	U32				index;				// table index
	U32				last;				// last index
	U8				idx;				// the current index value
	U8				start_index;		// staring index
	U8				num_indexes;		// number of vp indexes to work on
	U8				spare;
	void			*buffer;			// pointer to buffer (if used)
	void			*ptr;				// general purpose pointer
	Message			*pMsg;				// pointer to message
	Message			*pReply;			// non-zero if we need a reply
} LM_CONTEXT;


#endif // __LoopMonitorIsm_h