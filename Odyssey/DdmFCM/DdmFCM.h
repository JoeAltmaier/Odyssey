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
// File: DdmFCM.h
// 
// Description:
// DdmFCM.h
// 
// $Log: /Gemini/Odyssey/DdmFCM.h $
// 
// 1     1/05/00 5:17p Dpatel
// Initial creation
// 
//
/*************************************************************************/
#ifndef _DdmFCM_h
#define _DdmFCM_h


#include "CtTypes.h"
#include "DdmMaster.h"
#include "Message.h"

// Cmd queue files
#include "CmdServer.h"
#include "CmdSender.h"


// Loop Monitor interface
#include "LoopMessages.h"

// Tables
#include "StorageRollCallTable.h"
#include "LoopDescriptor.h"
#include "WWNTable.h"

#include "DdmClassDescriptorTable.h"
#include "DiskDescriptor.h"
#include "VirtualClassDescriptorTable.h"
#include "VirtualDeviceTable.h"

// PTS
#include "PtsCommon.h"
#include "ReadTable.h"
#include "Listen.h"
#include "Fields.h"
#include "Rows.h"
#include "Listen.h"
#include "Table.h"


#include "stdio.h"
#include "odyssey_trace.h"
#include "Trace_Index.h"
#ifdef	TRACE_INDEX
#undef	TRACE_INDEX
#endif	// TRACE_INDEX
#define TRACE_INDEX			TRACE_RMSTR


#include "UnicodeString.h"
#include "StringResourceManager.h"

// FCM interface files
#include "DdmFCMCmnds.h"
#include "DdmFCMMsgs.h"

#define UNUSED(p) p=p


#include "CTEvent.h"


#pragma	pack(4)

/********************************************************************
*
*	Local use for DdmFCM
*
********************************************************************/
#define FCM_TBL_RSRV_ENTRIES		20

/********************************************************************
*
*
*
********************************************************************/

class DdmFCM: public DdmMaster
{
public:
	DdmFCM(DID did);
	~DdmFCM();
	static Ddm *Ctor(DID MyDID);
	STATUS Enable(Message *pMsg);
	STATUS Quiesce(Message *pMsg);
	STATUS Initialize(Message *pMsg);
	STATUS BuildVirtualDeviceList();
	STATUS DispatchDefault(Message *pMsg);

protected:

private:

	struct ALARM_CONTEXT{
		U32			eventCode;			// event code for alarm
		rowID		alarmSourceRowId;	// src on which alarm raised
	};

	struct CONTEXT
	{
		U32						state;
		HANDLE					cmdHandle;
		U32						numProcessed;
		U32						value;		// some value
		U32						value1;		// some value
		void					*pData;		// general purpose ptr
		void					*pData1;	// general purpose ptr
		void					*pData2;	// general purpose ptr
		rowID					newRowId;	// for inserts
		struct CONTEXT			*pParentContext;
		pTSCallback_t			pCallback;
		CONTEXT()
		{
			memset(this,0,sizeof(CONTEXT));
		};
		
		~CONTEXT(){
			if (pData){
				delete pData;
				pData = NULL;
			}
			if (pData1){
				delete pData1;
				pData1 = NULL;
			}
			if (pData2){
				delete pData2;
				pData2 = NULL;
			}
		}
	};

private:
	DID				m_FCMDID;
	BOOL			m_DescriptorTablesDefined;	// all FCM related tables defined
	Message			*m_pInitializeMessage;
	Message			*m_pQuiesceMessage;

	I64				m_chassisWWN_NAA2;
	I64				m_chassisWWN_NAA5;

	// FCM Cmd Server
	CmdServer		*m_pCmdServer;			

	// Internal Cmd Sender
	CmdSender		*m_pInternalCmdSender;			


	// Table services

	BOOL					m_CommandInProgress;
	BOOL					m_failoverInProgress;
	BOOL					m_isQuiesced;

	StringResourceManager	*m_pStringResourceManager;

	// Initialization Prototypes
	void InitializeCommandQueues();
	STATUS FCMStringResourceManagerInitializedReply(
					void			*pContext,
					STATUS			status);

	// FCM CMD QUEUE
	void FCMCommandReceiver(HANDLE handle, void* pClientContext);
	STATUS FCMEventHandler(void* pModifiedSQRecord, STATUS status);
	void FCMCmdServerInitializedReply(STATUS status);

	// INTERNAL CMD SENDER
	void FCMInternalCmdSenderInitializedReply(STATUS status);


	// INTERNAL CMNDS
	STATUS StartInternalLoopDown(U32 loopInstanceNumber);
	void FCMInternalCmdCompletionReply (
			STATUS			completionCode,
			void			*pResultData,
			void			*pCmdData,
			void			*pCmdContext);



	// DEFINE TABLES
	STATUS FCMDefineDescriptorTables();
	STATUS FCMDefineLoopDescriptorTable(CONTEXT *_pContext);
	STATUS FCMDefineWWNTable(CONTEXT *pContext);
	STATUS FCMProcessDefineDescriptorTablesReply(void *pClientContext, STATUS status);

	// LOOP CONTROL 
	STATUS LoopControlCommandValidation(HANDLE h, FCM_CMND_INFO *_pCmdInfo);
	STATUS LoopControlValdiationReply(void *_pContext, STATUS status);
	STATUS LoopControlCommand(
			HANDLE						handle,
			FCM_CMND_INFO				*_pCmdInfo,
			LoopDescriptorRecord		*_pLoopDescriptorRecord);
	STATUS ProcessLoopControlReply(Message* pMsg);
	STATUS LoopControlEventReply(void *_pContext, STATUS status);


	// NAC SHUTDOWN
	STATUS ProcessNacShutdown(HANDLE h, FCM_CMND_INFO *_pCmdInfo);
	STATUS ProcessNacShutdownReply(void *_pContext, STATUS status);

	// WWN
	STATUS FCMInitializeChassisWWN(pTSCallback_t cb, CONTEXT *pContext);
	STATUS FCMInitializeChassisWWNReply(void *_pContext, STATUS status);
	STATUS ReadChassisWWN(CONTEXT *pContext, void *pKeyValue);
	STATUS InsertChassisWWN(void *pRowData, pTSCallback_t cb, CONTEXT *pContext);
	STATUS GetNextWWN(HANDLE h, FCM_CMND_INFO *_pCmdInfo);
	STATUS GetNextWWNReply(void *_pContext, STATUS status);
	STATUS GetChassisWWN(HANDLE h, FCM_CMND_INFO *_pCmdInfo);



	// Message based interface
	STATUS ProcessLoopControlMsg(Message *_pLoopControlMsg);
	STATUS ProcessNacShutdownMsg(Message *_pNacShutdownMsg);
	STATUS ProcessGetWWNMsg(Message *_pMsgFCMGetWWN);
	STATUS ProcessGetChassisWWNMsg(Message *_pMsgGetChassisWWN);



};

#endif

