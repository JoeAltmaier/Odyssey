/* DdmBootMgr.h -- Test Serial Communications Channel DDM
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

#define JFLsWAY
#undef TOMsWay

#include "BootMgrMsgs.h"
#include "BootMgrCmds.h"
#include "OsTypes.h"
#include "CtTypes.h"
#include "DdmCmbMsgs.h"
#include "CmbDdmCommands.h"
#include "SglLinkedList.h"
#include "Message.h"
#include "CmdSender.h"
#include "Ddm.h"
#include "DiskStatusTable.h"
#include "EVCStatusRecord.h"
#include "IOPStatusTable.h"
#include "IOPImageTable.h"
#include "DefaultImageTable.h"
#include "ImageDescriptorTable.h"
#include "SystemStatusTable.h"
#include "ReadTable.h"
#include "Listen.h"
#include "Table.h"
#include "Rows.h"
#include "IopManager.h"
#include "imghdr.h"
#include "CmdSender.h"
#include "CmdServer.h"
#include "CommandProcessingQueue.h"

#if false
#include "RqOsTimer.h"
#else
#include "TimerStatic.h"
#endif

class DdmBootMgr: public Ddm
{
public:
		// Standard Ddm methods:
		DdmBootMgr(DID did);
		
static 	Ddm* Ctor(DID did);

virtual STATUS Initialize(Message *pMsg);
		
private:
		// Our Ddm specific methods:
		STATUS DefineCommandServer();
		void DefineSystemStatusTable(STATUS status);
		STATUS CheckSystemStatusTable(void *pClientContext, STATUS status);
		STATUS ReadSystemStatusRec(void *pClientContext, STATUS status);
		STATUS CreateSystemStatusRec(void *pClientContext, STATUS status);
		STATUS CheckSystemStatusRec(void *pClientContext, STATUS status);
		//STATUS HaveCMBUpdateEVCStatusRecord(void *pClientContext, STATUS status);
		STATUS ReadEVCStatusRecord(void *pClientContext, STATUS status);
		STATUS CheckEVCStatusRecord(void *pClientContext, STATUS status);
		
		// IopStatusTable initialization and / or reconciliation methods.
		STATUS ReadOldIopStatusTable(void *pClientContext, STATUS status);
		STATUS HaveCMBUpdateIopStatusTable(void *pClientContext, STATUS status);
		STATUS ReadNewIopStatusTable(Message*	pMsg);
		const char* const GetIopTypeCode(IOPStatusRecord* pIopStatusRec);
		const char*	const GetIopTypeDesc(IOPStatusRecord* pIopStatusRec);
		const char*	const GetIopStateCode(IOPStatusRecord* pIopStatusRec);
		const char*	const GetIopStateDesc(IOPStatusRecord* pIopStatusRec);
		void DumpIopStatusRec(U32	TraceLvl, IOPStatusRecord* pIopStatusRec, const char* const pString);
		void Dump2IopStatusRecs(
								U32	TraceLvl, 
								IOPStatusRecord* pIopStatusRec1,
								char* pString1,
								IOPStatusRecord* pIopStatusRec2,
								char* pString2
								);
		STATUS CheckIopStatusTable(void *pClientContext, STATUS status);

#ifdef JFLsWAY
		// DefaultImageTable initialization methods.
		STATUS ReadDefaultImageTable(void *pClientContext, STATUS status);
		STATUS CheckDefaultImageTable(void *pClientContext, STATUS status);
		STATUS DefineDefaultImageTable(void *pClientContext, STATUS status);
		STATUS InitDefaultImageTable(void *pClientContext, STATUS status);

		// IopImageTable initialization and reconciliation methods.
		STATUS DefineIopImageTable(void *pClientContext, STATUS status);
		STATUS ReadIopImageTable(void *pClientContext, STATUS status);
		STATUS HandleReadIopImageTableReply(void *pClientContext, STATUS status);
		STATUS UpdateIopImageTable(void *pClientContext, STATUS status);
		STATUS CheckIopImageTable(void *pClientContext, STATUS status);
		STATUS CreateIopImageTable(void *pClientContext, STATUS status);
		STATUS InitIopImageTable(void *pClientContext, STATUS status);
		STATUS UpdateIopImageRecord(void *pClientContext, STATUS status);
		STATUS IopImageRowModified(void* pClientContext, STATUS status);
		STATUS HandleAddNewImageReply(Message* pMsg);
#endif
#ifdef TOMsWAY		
		// DefaultImageTable initialization methods.
		STATUS ReadDefaultImageTable(Message* pMsg);
		STATUS CheckDefaultImageTable(Message* pMsg);
		STATUS DefineDefaultImageTable(Message* pMsg);
		STATUS InitDefaultImageTable(Message* pMsg);
		STATUS HandleAddNewImageReply(Message* pMsg);
#endif

		int CompareVersions( img_hdr_t* pROMImgHdr, DefaultImageRecord* pDefaultImageRec);
		void DisplayImageHdr( img_hdr_t* pRomImgHdr, const char* const pString );
		void DisplayImageDesc( ImageDescRecord* pImgDscRec, const char* const pString );
		void DisplayDefaultImage( DefaultImageRecord* pDefaultImageRec, const char* const pString );
		STATUS UpdateDefaultImageTable(Message *pMsg);
		STATUS HandleDefaultImageReply(Message* pMsg);

		STATUS FinishInitialize();
	
public:

		STATUS Enable(Message *pMsg);

private:

		// command processing queue functionality
		// allows commands and messages to enter a common queue from
		// which requests are handled one at a time.
		typedef struct _CONTEXT {
			Message* pMsg;
			HANDLE handle;
			BMGRRequest* pCmdInfo;
		} CONTEXT;
		
		enum
		{
			function_TakeIopOutOfSvc = 1,
			function_PutIopIntoSvc,
			function_PowerIopOn,
			function_LockIop,
			function_UnlockIop
		};
		
		CommandProcessingQueue* m_pBootMgrQueue;
		BOOL m_processingCommand;
		
		BOOL NotProcessingCommand() { return (m_processingCommand == FALSE); }
		void ProcessCommand() { m_processingCommand = TRUE; }
		void FinishCommand() { m_processingCommand = FALSE; }

		// command processing queue methods
		void SubmitRequest(U32 functionToCall, CONTEXT* pContext);
		void ExecuteFunction(U32 functionToCall, CONTEXT* pContext);
		void ProcessNextCommand();

public:
		// Listener for SSAPI Command Server
		void ListenerForCommands(HANDLE handle, void* pCmdData);
		
		// Processes Boot Mgr Messages
		STATUS ProcessBootMgrMessages(Message *pMsg);

private:

		STATUS Boot();
		STATUS ListenOnIopStatusTable();
		STATUS HandleIopSTListenReply(void *pClientContext, STATUS status);
		STATUS ListenOnIopImageTable();
		STATUS HandleIopImageListenReply(void *pClientContext, STATUS status);
		STATUS InitSystemStatusRecord();
		STATUS HandleWriteSystemStatusTableReply(void *pClientContext, STATUS status);

		STATUS InitIopImageRecord(void *pClientContext, STATUS status);

		STATUS InitIopMgrs();		

		STATUS StartIops();
		STATUS Listen4IopsOnPci(STATUS status);
		STATUS IopsOnPciListenReply(void *pClientContext, STATUS status);
		STATUS BootIops();
		STATUS Listen4ActiveIops(STATUS status);
		STATUS IopsActiveListenReply(void *pClientContext, STATUS status);
		STATUS FinishEnable();

		STATUS StartLEDs();
#if false
		STATUS BlinkLEDs(Message* pMsg);
#else
		static void BlinkLEDs(void *pContext);
#endif
		STATUS ShutDown();
			
		// Take IOP out of Service
		STATUS HandleTakeIopOutOfService(CONTEXT* pContext);
		STATUS StopTransportsToIopReply(Message* pMsg);
		STATUS HandleReqIopOutOfServiceInt(Message *pMsg);
		STATUS HandleReqIopOutOfServiceIntReply(Message *pMsg);
		STATUS StartTransportsToIopReply(Message* pMsg);
		
		// Power on IOP
		STATUS HandlePowerOnIop(CONTEXT* pContext);
		STATUS Listen4IopOnPci(STATUS status, TySlot slot);
		STATUS IopOnPciListenReply(void *pClientContext, STATUS status);

		// Put IOP into Service
		STATUS HandlePutIopIntoService(CONTEXT* pContext);
		STATUS HandleReqIopIntoServiceInt(Message *pMsg);
		STATUS HandleReqIopIntoServiceIntReply(Message *pMsg);

		// Lock or Unlock an IOP
		STATUS HandleLockUnlockIop(CONTEXT* pContext);
		STATUS HandleReqIopLockUnlockInt(Message *pMsg);
		STATUS HandleReqIopLockUnlockIntReply(Message *pMsg);

private:
		// Our instance data
		MsgCmbPollAllIops*		m_pCmbPollAllIopsMsg;		// Our update Iop Status Table Message.
		TSReadTable*			m_pReadIopStatusTable;
		IopManager*				m_aIopManagers[NSLOT];
		Message*				m_pInitializeMsg;
		Message*				m_pEnableMsg;
		SystemStatusRecord		m_SystemStatusRecord;
		SystemStatusRecord*		m_pSystemStatusRecord;
		SystemStatusRecord*		m_pNewSystemStatusRecord;
		U32						m_cbNewSystemStatusRecord;
		U32						m_nSystemStatusRecords;
		#if false
		MsgCmbUpdateEvcStatus*	m_pCmbUpdEvcSRMsg;
		#endif
		EVCStatusRecord			m_EVCStatusRecord;
		IOPStatusRecord*		m_pOldIopStatusTableRows;
		U32						m_nOldIopStatusTableRows;
		IOPStatusRecord			m_IopStatusTableRows[NSLOT];
		IOPStatusRecord*		m_pIopStatusTableRows;
		U32						m_cbIopStatusTableRows;
		U8						m_pad; // jlo
		IOPImageRecord			m_IopImageTableRows[NSLOT];
		U32						m_nIopStatusTableRows;
		IOPImageRecord*			m_pIopImageTableRows;
		U32						m_cbIopImageTableRows;
		U32						m_nIopImageTableRows;
		DefaultImageRecord*		m_pDefaultImageTable;
		U32						m_nDefaultImageTableRows;
		DefaultImageRecord*		m_pDefImgRecToUpdate;
		U32						m_IopSTItemToStartNext;
		TSListen*				m_pListenIopStatusTable;
		TSListen*				m_pListenIopImageTable;
		U32						m_IopSTListenerId;								// U32* pListenerIDRet,
		U32						m_IopImageListenerId;								// U32* pListenerIDRet,
		U32*					m_pIopSTListenReplyType;
		U32*					m_pIopImageListenReplyType;
		IOPStatusRecord*		m_pIopSTModifiedRecord;
		IOPImageRecord*			m_pIopImageModifiedRecord;
		U32						m_cbIopSTModifiedRecord;
		U32						m_cbIopImageModifiedRecord;
		U32						m_iIopSTModifiedRecord;
		U32						m_iIopImageModifiedRecord;
		TSTimedListen*			m_pListen4Iops;
		TSTimedListen*			m_pListen4Iop;
		U32						m_ListenerID;
		U32*					m_pListenReplyType;
		U32						m_fAllIopsOnPciFlag;
		U32						m_fAllIopsAreUp;
		U32						m_iROMImgBlk;
		U32						m_iDefaultImageTable;
		U32						m_iSlotCount;
		U32						m_ImgSiz;
		img_hdr_t*				m_pImgHdr;
		// take out of service message and command
		MsgIopOutOfService*		m_pMsgIopOutOfSvc;
		BMGRRequest*			m_pCmdIopOutOfSvc;
		HANDLE					m_pHandleIopOutOfSvc;
		// put into service message and command
		MsgIopIntoService*		m_pMsgIopIntoSvc;
		BMGRRequest*			m_pCmdIopIntoSvc;
		HANDLE					m_pHandleIopIntoSvc;
		// power on message and command
		MsgIopPowerOn*			m_pMsgIopPowerOn;
		BMGRRequest*			m_pCmdIopPowerOn;
		HANDLE					m_pHandleIopPowerOn;
		// lock and unlock command
		BMGRRequest*			m_pCmdIopLockUnlock;
		HANDLE					m_pHandleIopLockUnlock;
		U32						m_iIopImage2Init;
		U32						m_fHaveDefaultImageType;
		U32						m_iDefaultmageTable;
#if false
		RqOsTimerStart*			m_pStartTimerMsg;
#else
		TimerStatic*			m_pStartTimerMsg;
#endif
		CmdServer*				m_pCmdServer;
		
};


#endif	// __DdmSccTest_H

