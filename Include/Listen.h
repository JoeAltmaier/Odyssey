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
// This file is the declaration of an object interface to the GetTableDef 
// API of the Table Service.
// 
// $Log: /Gemini/Odyssey/DdmPTS/Listen.h $
// 
// 13    9/06/99 7:07p Jlane
// Added TSTimedListen.
// 
// 12    7/22/99 3:11p Hdo
// 
// 11    5/11/99 10:54a Jhatwich
// win32
// 
// 10    4/21/99 9:23a Jlane
// Added new seperate parameters for key field name and value pb/cb
// distinct from modified field name and pb/cb.
//
// Update Log: 
// 11/17/98	JFL	Created.
// 1/18/99	JFL	Compiled (except for calls to send pending new DdmServices class)
//				and checked in.
// 1/26/99	JFL	Compiled using the new DdmService base class and checked in.
// 02/27/99 JFL	Added support for returned records.
//
/*************************************************************************/
#ifndef _Listen_h_
#define _Listen_h_

#include "CTTypes.h"
#include "Ddm.h"
#include "Message.h"
#include "TableMsgs.h"

#ifndef __RqOsTimer_h
#include "RqOsTimer.h"  // Timer service messages
#endif


// The User's callback is (currently) of the form:
// STATUS Callback(STATUS status).
// 
typedef STATUS (DdmServices::*pTSCallback_t)(void *, STATUS);

#ifdef WIN32
#define TSCALLBACK(clas,method)	(pTSCallback_t) method
#elif defined(__ghs__)  // Green Hills
#define TSCALLBACK(clas,method)	(pTSCallback_t) &clas::method
#else	// MetroWerks
#define TSCALLBACK(clas,method)	(pTSCallback_t)&method
#endif

class TSListen : public DdmServices 
{
public:
		
		// Initialize all parameters necessary to Listen.
		STATUS Initialize(	DdmServices*		pDdmServices,
							U32					ListenType,
							String64			prgbTableName,
							String64			prgbRowKeyFieldName,
							void*				prgbRowKeyFieldValue,
							U32					cbRowKeyFieldValue,
							String64			prgbFieldName,
							void*				prgbFieldValue,
							U32					cbFieldValue,
							U32					ReplyMode,
							void**				ppTableDataRet,
							U32*				pcbTableDataRet,
							U32*				pListenerIDRet,
							U32**				ppListenTypeRet,
							void**				ppModifiedRecordRet,
							U32*				pcbModifiedRecordRet,
							pTSCallback_t		pCallback,
							void*				pContext
						  );
		
		// Start listening by sending a Listen message to the Table Service.
		void Send();
		
		// Stop listening by sending a stop listening message to the Table Service. 
		STATUS Stop();
		
		// This object's listen message reply handler.
		STATUS HandleListenReply( MessageReply* pMessage );
		
		// This object's stop listening message reply handler.
		STATUS HandleStopListenReply( MessageReply* pMessage );
		
private:
	DdmServices*		m_pDdmServices;
	U32					m_ListenType;
	String64			m_rgbTableName;
	String64			m_rgbRowKeyFieldName;
	void*				m_prgbRowKeyFieldValue;
	U32					m_cbRowKeyFieldValue;
	String64			m_rgbFieldName;
	void*				m_prgbFieldValue;
	U32					m_cbFieldValue;
	U32					m_ReplyMode;
	void*				m_pTableDataRet;
	void**				m_ppTableDataRet;
	U32					m_cbTableDataRet;
	U32*				m_pcbTableDataRet;
	U32					m_ListenerIDRet;
	U32*				m_pListenerIDRet;
	U32**				m_ppListenTypeRet;
	U32					m_cbListenTypeRet;
	void**				m_ppModifiedRecordRet;
	U32					m_cbModifiedRecordRet;
	U32*				m_pcbModifiedRecordRet;
	pTSCallback_t		m_pCallback;
	void*				m_pContext;
	U32					m_fFirstReply;
	
};


class TSTimedListen : public DdmServices 
{
public:
		
		// Initialize all parameters necessary to Time a Listen.
		STATUS Initialize(	DdmServices*		pDdmServices,
							U32					ListenType,
							String64			prgbTableName,
							String64			prgbRowKeyFieldName,
							void*				prgbRowKeyFieldValue,
							U32					cbRowKeyFieldValue,
							String64			prgbFieldName,
							void*				prgbFieldValue,
							U32					cbFieldValue,
							U32					ReplyMode,
							void**				ppTableDataRet,
							U32*				pcbTableDataRet,
							U32*				pListenerIDRet,
							U32**				ppListenTypeRet,
							void**				ppModifiedRecordRet,
							U32*				pcbModifiedRecordRet,
							pTSCallback_t		pCallback,
							U32					iTimeOut,
							void*				pContext
						  );
		
		// Start listening by sending a Listen message to the Table Service.
		void Send();
		
		// Stop listening by sending a stop listening message to the Table Service. 
		STATUS Stop();
		
		STATUS ListenCallback(void *pClientContext, STATUS status);
		STATUS ListenTimerCallback(Message *pTimerMsgReply);
		STATUS StopTimerCallback(Message *pStopTimerReply);
	
private:
	TSListen*			m_pListen;
	DdmServices*		m_pDdmServices;
	RqOsTimerStart*		m_pStartTimerMsg;
	RqOsTimerStop*		m_pStopTimerMsg;
	pTSCallback_t		m_pUserCallback;
	void*				m_pContext;	
	U32					m_iTimeOut;
	U32					m_fListenWasDetected;
	U32					m_fListenTimerExpired;
	U32					m_fSendWasCalled;
	U32					m_fStopWasCalled;
	U32*				m_pListenReplyType;
	U32**				m_ppListenReplyType;
	U32					m_ReplyMode;
};


#endif // _Listen_h_