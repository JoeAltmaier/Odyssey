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
// File: ScsiServerIsm.h
// 
// Description:
// This file contains the derived class for the ScsiServerIsm
// 
// Update Log 
//	$Log: /Gemini/Odyssey/IsmScsiServer/ScsiServerIsm.h $
// 
// 10    11/15/99 4:08p Mpanas
// Re-organize sources
// - Add new files: ScsiInquiry.cpp, ScsiReportLUNS.cpp, 
//   ScsiReserveRelease.cpp
// - Remove unused headers: ScsiRdWr.h, ScsiMessage.h, ScsiXfer.h
// - New Listen code
// - Use Callbacks
// - All methods part of SCSI base class
// - Remove DoWork() and replace with RequestDefault() and ReplyDefault()
// 
// 9     9/03/99 10:31a Vnguyen
// Add Support for PHS Reporter.  Start/Stop Reporter in Enable() and
// Quiesce().  Add support for PHS Request.
// 
// 8     6/06/99 4:36p Mpanas
// Implement a method to check if a Virtual Circuit is Online.
// Send a BSA_STATUS_CHECK on the first
// SCSI_SCB_EXEC message, then set a 4 second timer.
// If the status comes back with an error or the timer expires, the VC
// will be macked OFFLINE otherwise the VC is ONLINE
// 
// This version does not update the Export Table (DEFERED)
// The changes to sequence the RAC are needed for this to work.
// 
// 7     4/05/99 10:19p Mpanas
// Add Table support, read our Export table entry
// at initi time
// 
// 6     3/22/99 8:35p Mpanas
// Move config data to StsConfig.h
// 
// 06/05/98 Michael G. Panas: Create file
// 09/16/98 Michael G. Panas: Change over to new DDM model
// 02/12/99 Michael G. Panas: convert to new Oos model
/*************************************************************************/


#ifndef __ScsiServerIsm_h
#define __ScsiServerIsm_h

#include "Nucleus.h"
#include "Ddm.h"			// include base class

#include "ExportTable.h"
#include "StsConfig.h"
#include "STSData.h"
#include "ScsiSense.h"
#include "RqOsTimer.h"
#include "STSPerf.h"
#include "STSStat.h"
#include "Listen.h"

struct	SCSI_CONTEXT;

// define the derived class for the Scsi target Server
class ScsiServerIsm: public Ddm {

public:
	// Data recovered from the Persistent Table Service
	STS_CONFIG	config;
	
	ScsiServerIsm(DID did);
	VDN		MyVdn;		// my virtual device number
	
	DID		MyDid;		// for PHS Reporter
	bool	fStatusReporterActive;		// True if the Status Reporter has been started for this Did
	bool	fPerformanceReporterActive; // True if the Performance Reporter has been started for this Did
			// The reason we need these two flags is because we start the
			// reporters in enable() and stop them in quiesce().  At this
			// time I am not sure if there is a matching one to one calling
			// sequence between these two methods.

	static
	Ddm *Ctor(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);
	STATUS Quiesce(Message *pMsg);
	
	// member methods
	
	// ScsiServ.cpp
	void	ScsiInitialize();
	void	ScsiDecode(void *p_context);

	// ScsiRdWr.cpp
	U32		ScsiServerRead(SCSI_CONTEXT *p_context);
	U32		ScsiServerWrite(SCSI_CONTEXT *p_context);
	STATUS 	ScsiServerReadCallback(Message *pMsg);
	STATUS 	ScsiServerWriteCallback(Message *pMsg);
	
	// ScsiModes.cpp
	void	ScsiBuildModeSenseData(U8 *p);
	U32		ScsiServerModeSense(SCSI_CONTEXT *p_context);
	U32		ScsiServerModeSelect(SCSI_CONTEXT *p_context);

	// ScsiInquiry.cpp
	void	ScsiBuildInquiryData(void *p);
	void	ScsiServerInquiry(SCSI_CONTEXT *p_context);
	
	// ScsiReportLUNs.cpp
	void	ScsiServerReportLUNs(SCSI_CONTEXT *p_context);
	
	// ScsiReserveRelease.cpp
	void	ScsiServerReserve(SCSI_CONTEXT *p_context);
	void	ScsiServerRelease(SCSI_CONTEXT *p_context);
	
	// ScsiPassThru.cpp
	STATUS	ScsiPassThru(SCSI_CONTEXT *p_context);

	// ScsiSense.cpp
	void	ScsiBuildSense(PREQUEST_SENSE pRQ);
	void	SetStatus(U32 sense, U32 code);
	U32		GetSenseKeyStatus();
	U32		GetASCStatus();
	
	// ScsiMessage.cpp
	void	ScsiReply(SCSI_CONTEXT *p_context, U32 Length);
	void	ScsiErrReply(SCSI_CONTEXT *p_context);
	STATUS	ScsiSendMessage(SCSI_CONTEXT *p_context);
	
	// ScsiXfer.cpp
	U32		ScsiSendData(SCSI_CONTEXT *p_context, U8 *pSrc, U32 Length);
	U32		ScsiGetData(SCSI_CONTEXT *p_context, U8 *pDst, U32 Length);	
	
	// ScsiServerTable.cpp
	// Table methods for the SCSI Target Server
	STATUS	STSTableInitialize(Message *pMsg);
	STATUS 	STSTblReadExport(void *pClientContext, STATUS status);
	STATUS 	STSTblCreateStsData(void *pClientContext, STATUS status);
	STATUS 	STSTblCrStsDataReply1(void *pClientContext, STATUS status);
	STATUS 	STSTblReadDataReply1(void *pClientContext, STATUS status);
	STATUS 	STSTblReplyLast(void *pClientContext, STATUS status);
	
	// ScsiTableListen.cpp
	STATUS	StsStartListens(void );
	STATUS	StsDataListenUpdate(void *pClientContext, STATUS status);
	STATUS	StsExportListenUpdate(void *pClientContext, STATUS status);


	// Target server data
	
	StsData		*pData;					// Our Inquiry data read from PTS
	
	ExportTableEntry	*pStsExport;	// Our export table entry (for this VD)

	SS_STATUS	 m_ss;					// SCSI Check Status record
	
	Message		*m_pMsg;				// saved Init message

	U32			 m_VcStatus;			// virtual circuit state
	
#define	VC_OFFLINE		0
#define	VC_ONLINE		1
#define	VC_TIMED_OUT	2
#define	VC_NO_STATUS	3
#define	VC_STATUS_SENT	4

protected:
	STATUS ReplyDefault(Message *pMsg);
	STATUS RequestDefault(Message *pMsg);
	
	// STS Status Record
	STS_Status		m_Status;
	
	// STS Performance Record
	STS_Performance	m_Performance;
	
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
	
	RqOsTimerStart*		m_pStartTimerMsg;	// Message used to start the Timer service
											// on the BsaStatus message.
	
	// Listen on StsData table entry 
	TSListen*			m_Listen;
	U32	*				m_LType;		// Listen Type return
	U32					m_LSize;		// Size of returned row
	StsData				*m_StsData;		// return entry

	// Listen on Export Table
	TSListen*			m_Listen1;
	U32	*				m_LType1;		// Listen Type return
	U32					m_LSize1;		// Size of returned row
	ExportTableEntry	*m_Export;		// returned entry
	
};

#endif