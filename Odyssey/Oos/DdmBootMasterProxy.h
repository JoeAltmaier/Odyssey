/* DdmBootMasterProxy.h -- Test Serial Communications Channel DDM
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
#include "IopProxy.h"
#include "imghdr.h"

#if false
#include "RqOsTimer.h"
#else
#include "TimerStatic.h"
#endif

class DdmBootMasterProxy: public Ddm
{
public:
		// Standard Ddm methods:
		DdmBootMasterProxy(DID did);
static 	Ddm* Ctor(DID did);
virtual STATUS Initialize(Message *pMsg);

		// Our Ddm specific methods:
		STATUS DefineSystemStatusTable();
		STATUS CheckSystemStatusTable(void *pClientContext, STATUS status);
		STATUS ReadSystemStatusRec(void *pClientContext, STATUS status);
		STATUS CreateSystemStatusRec(void *pClientContext, STATUS status);
		STATUS CheckSystemStatusRec(void *pClientContext, STATUS status);
		STATUS HaveCMBUpdateEVCStatusRecord(void *pClientContext, STATUS status);
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
		STATUS UpdateIopImageRecord(void *pClientContext, STATUS status);
		STATUS HandleAddNewImageReply(Message* pMsg);
#endif
#ifdef TOMsWAY		
		// DefaultImageTable initialization methods.
		STATUS ReadDefaultImageTable(Message* pMsg);
		STATUS CheckDefaultImageTable(Message* pMsg);
		STATUS DefineDefaultImageTable(Message* pMsg);
		STATUS InitDefaultImageTable(Message* pMsg);
		STATUS HandleAddNewImageReply(Message* pMsg);
		// IopImageTable initialization and reconciliation methods.
		STATUS DefineIopImageTable(Message* pMsg);
		STATUS ReadIopImageTable(Message* pMsg);
		STATUS CheckIopImageTable(Message* pMsg);
		STATUS CreateIopImageTable(Message* pMsg);
		STATUS UpdateIopImageRecord(Message* pMsg);
#endif

		int CompareVersions( img_hdr_t* pROMImgHdr, DefaultImageRecord* pDefaultImageRec);
		void DisplayImageHdr( img_hdr_t* pRomImgHdr, const char* const pString );
		void DisplayImageDesc( ImageDescRecord* pImgDscRec, const char* const pString );
		void DisplayDefaultImage( DefaultImageRecord* pDefaultImageRec, const char* const pString );
		STATUS UpdateDefaultImageTable(Message *pMsg);
		STATUS HandleDefaultImageReply(Message* pMsg);

		STATUS FinishInitialize();

		STATUS Enable(Message *pMsg);

		STATUS Boot();
		STATUS ListenOnIopStatusTable();
		STATUS HandleIopSTListenReply(void *pClientContext, STATUS status);
		STATUS InitSystemStatusRecord();
		STATUS HandleWriteSystemStatusTableReply(void *pClientContext, STATUS status);

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
		
		// Request Message handlers:
		STATUS HandleReqIopOutOfService(Message *pMsg);
		STATUS StopTransportsToIopReply(Message* pMsg);
		STATUS StartTransportsToIopReply(Message* pMsg);
		STATUS HandleReqIopOutOfServiceInt(Message *pMsg);
		STATUS HandleReqIopOutOfServiceIntReply(Message *pMsg);
		STATUS HandleReqIopIntoService(Message *pMsg);
		STATUS HandleReqIopIntoServiceInt(Message *pMsg);

		// Our instance data
private:
		MsgCmbPollAllIops*		m_pCmbPollAllIopsMsg;		// Our update Iop Status Table Message.
		TSReadTable*			m_pReadIopStatusTable;
		IopManProxy*				m_aIopManProxys[NSLOT];
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
		U32						m_nIopStatusTableRows;
		IOPImageRecord			m_IopImageTableRows[NSLOT];
		IOPImageRecord*			m_pIopImageTableRows;
		U32						m_cbIopImageTableRows;
		U32						m_nIopImageTableRows;
		DefaultImageRecord*		m_pDefaultImageTable;
		U32						m_nDefaultImageTableRows;
		DefaultImageRecord*		m_pDefImgRecToUpdate;
		U32						m_IopSTItemToStartNext;
		TSListen*				m_pListenIopStatusTable;
		U32						m_IopSTListenerId;								// U32* pListenerIDRet,
		U32*					m_pIopSTListenReplyType;
		IOPStatusRecord*		m_pIopSTModifiedRecord;
		U32						m_cbIopSTModifiedRecord;
		U32						m_iIopSTModifiedRecord;
		TSTimedListen*			m_pListen4Iops;
		U32						m_ListenerID;
		U32*					m_pListenReplyType1;
		U32*					m_pListenReplyType2;
		U32						m_fAllIopsOnPciFlag;
		U32						m_fAllIopsAreUp;
		U32						m_iROMImgBlk;
		U32						m_iSlotCount;
		U32						m_ImgSiz;
		img_hdr_t*				m_pImgHdr;
		MsgIopOutOfService*		m_pMsgIopOutOfSvc;
		MsgIopIntoService*		m_pMsgIopIntoSvc;


#if false
		RqOsTimerStart*			m_pStartTimerMsg;
#else
		TimerStatic*			m_pStartTimerMsg;
#endif
		
};


#endif	// __DdmSccTest_H

