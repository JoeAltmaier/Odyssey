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
// File: DdmFCMTest.cpp
// 
// Description:
// Test DDM for FCM master
// 
// $Log: /Gemini/Odyssey/DdmFCM/DdmFCMTest/DdmFCMTest.h $
// 
// 1     1/05/00 5:20p Dpatel
// Initial creation
// 
//
/*************************************************************************/
#ifndef _DdmFCMTest_h
#define _DdmFCMTest_h


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

#include "DdmClassDescriptorTable.h"
#include "DiskDescriptor.h"
#include "VirtualClassDescriptorTable.h"
#include "VirtualDeviceTable.h"

// PTS
#include "Rows.h"
#include "Listen.h"
#include "ReadTable.h"
#include "Table.h"
#include "PTSCommon.h"

#ifdef	TRACE_INDEX
#undef	TRACE_INDEX
#endif	// TRACE_INDEX
#define TRACE_INDEX			TRACE_RMSTR_1

#include "UnicodeString.h"
#include "StringResourceManager.h"

// FCM interface files
#include "DdmFCMCmnds.h"


#define UNUSED(p) p=p



#pragma	pack(4)


/********************************************************************
*
*
*
********************************************************************/

class DdmFCMTest: public Ddm
{
	struct CONTEXT
	{
		U32						cmnd;
		U32						state;
		U32						numProcessed;
		U32						value;		// some value
		U32						value1;		// some value
		U32						value2;		// some value
		void					*pData;		// general purpose ptr
		void					*pData1;	// general purpose ptr
		rowID					newRowId;	// for inserts
		CommandQueueRecord		*pCQRecord;
		struct CONTEXT			*pParentContext;
		pTSCallback_t			pCallback;
		CONTEXT()
		{
			cmnd = 0;
			state = 0;
			numProcessed = 0;
			value = 0;
			value1 = 0;
			value2 = 0;
			pData = NULL;
			pData1 = NULL;
			pCQRecord = NULL;
			pParentContext = NULL;
			pCallback = NULL;
		};
	};
public:
	DdmFCMTest(DID did);
	~DdmFCMTest();
	static Ddm *Ctor(DID MyDID);
	STATUS Enable(Message *pMsg);
	STATUS Initialize(Message *pMsg);
	STATUS DispatchDefault(Message *pMsg);

protected:


private:
	DID			MyDID;

	CmdSender			*m_pCmdSender;
	HANDLE				m_CmdHandle;

	StringResourceManager	*m_pStringResourceManager;

	STATUS StringResourceManagerInitializedReply(
			void			*pContext,
			STATUS			status);


	// TEST METHODS
	STATUS TestLoopUp();
	STATUS TestNacShutdown();
	STATUS TestGetNextWWN();
	STATUS TestGetChassisWWN();

	// Message based requests
	STATUS FCM_LoopControlMsgReply(Message *pMsg);
	STATUS FCM_NacShutdownMsgReply(Message *pMsg);
	STATUS FCM_GetWWNMsgReply(Message *pMsg);
	STATUS FCM_GetChassisWWNMsgReply(Message *pMsg);


	// GENERAL
	void FCMObjectInitializedReply(STATUS status);
	void FCMCommandCompletionReply(
				STATUS				statusCode,
				void				*pStatusData,
				void				*pCmdData,
				void				*pCmdContext);
	void FCMEventHandler(
				STATUS			eventCode,
				void			*pEventData);
};


#endif

