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
// File: ScsiMonitorIsm.h
// 
// Description:
// This file contains the derived class for the ScsiMonitorIsm
// 
// Update Log 
//	$Log: /Gemini/Odyssey/IsmScsiMonitor/ScsiMonitorIsm.h $
// 
// 1     10/11/99 5:35p Cchan
// HSCSI (QL1040B) version of the Drive Monitor
// 
/*************************************************************************/


#ifndef __ScsiMonitorIsm_h
#define __ScsiMonitorIsm_h

#include "Ddm.h"			// include base class
#include "SmConfig.h"


typedef enum {
	SM_ACTION_INQUIRY					= 1,
	SM_ACTION_INQUIRY_SERIAL_NUMBER		= 2,
	SM_ACTION_TEST_UNIT					= 3,
	SM_ACTION_START_UNIT				= 4,
	SM_ACTION_GET_CAPACITY				= 5,
	SM_ACTION_UPDATE_PD					= 6,
	SM_ACTION_FINISH					= 7,
	SM_ACTION_DRIVE_READ_TEST			= 8,
	SM_ACTION_WRITE_VERIFY_TEST			= 9,
	SM_ACTION_READ_VERIFY_TEST			= 10,
	SM_ACTION_STOP_UNIT					= 11
} SM_ACTION;

// Scsi Monitor internal context
typedef struct _SM_Context {
	SM_ACTION		action;
	U32				drive_number;		// drive ID number
	U32				index;				// table index
	U32				last;				// last in chain flag
	U32				retries;			// number of retries allowed/left
	VDN				Vd;					// virtual device number for this drive
	void			*buffer;			// pointer to buffer (if used)
	Message			*pMsg;				// pointer to message for this drive
	Message			*pReply;			// non-zero if we need a reply
	U32 			logical_block_address; // Used by S_Sts_Read_Verify_Test
	U32 			block_count; 		// Used by S_Sts_Read_Verify_Test
} SM_CONTEXT;



// define the derived class for the HSCSI Scsi Drive Monitor ISM
class ScsiMonitorIsm: public Ddm {

public:
	// Data recovered from the Persistent Data Service
	SM_CONFIG config;
	
	ScsiMonitorIsm(DID did);
	VDN		MyVd;			// my virtual device number

	static
	Ddm *Ctor(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);
	STATUS Quiesce(Message *pMsg);
	
	// member methods to handle scan
	void SM_Scan_For_Drives(Message *pMsg);
	void SM_Do_Inquiry(SM_CONTEXT *pDmc);
	void SM_Do_Inquiry_Serial_Number(SM_CONTEXT *pDmc);
	void SM_Do_Test_Unit(SM_CONTEXT *pDmc);
	void SM_Do_Start_Unit(SM_CONTEXT *pDmc);
	void SM_Do_Stop_Unit(SM_CONTEXT *pDmc, unsigned s);
	void SM_Do_Read_Capacity(SM_CONTEXT *pDmc);
	void SM_Do_Update_PD(SM_CONTEXT *pDmc);
	void SM_Do_Finish(SM_CONTEXT *pDmc);
	void SM_Build_SCSI_Message(Message *pMsg, SCSI_COMMANDS Cmd,
				void *pData, long length, U32 drive_number);
	void SM_Send_Message(SM_CONTEXT *pDmc);
	void SM_Read_Drive(SM_CONTEXT *pDmc);
	void SM_Write_Drive_Block(SM_CONTEXT *pDmc,
		U32 logical_block_address, U32 block_count);
	void SM_Read_Drive_Block(SM_CONTEXT *pDmc,
		U32 logical_block_address, U32 block_count);
	void Verify_Read(SM_CONTEXT *pDmc);

	// Table methods for the drive monitor
	STATUS	SmTableInitialize(Message *pMsg);
	STATUS 	SmTblReply1(void *pClientContext, STATUS status);
	STATUS 	SmTblReply2(void *pClientContext, STATUS status);
	STATUS 	SmTblReply3(void *pClientContext, STATUS status);
	STATUS 	SmTblReply4(void *pClientContext, STATUS status);
	STATUS 	SmTblReplyLast(void *pClientContext, STATUS status);
	
	STATUS	SmTableUpdate(Message *pMsg, SM_CONTEXT *pDmc);
	STATUS 	SmTableUpdateEnd(void *pClientContext, STATUS status);
	STATUS	SmTblUpdReadDesc(void *pClientContext, STATUS status);
	STATUS	SmTblUpdAddDesc(void *pClientContext, STATUS status);
	STATUS 	SmTblUpdModifyDesc(void *pClientContext, STATUS status);
	STATUS	SmTblUpdAddRollCall(void *pClientContext, STATUS status);
	STATUS 	SmTblUpdRollCall(void *pClientContext, STATUS status);
	
	STATUS	SmListenUpdate(void *pClientContext, STATUS status);
	STATUS	SmListenUpdateEnd(void *pClientContext, STATUS status);

	// PTS Table Data
	DiskDescriptor		*SM_TS_Disk_Desc;		// one for each slot
	
	// Local Table Data
	DiskDescriptor		*SM_Disk_Desc;			// one for each slot
	
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
	
};

#endif // __ScsiMonitorIsm_h