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
// File: TestMsgs.h
//
// Description:
//    CHAOS Test Services Messages Header
//	  Contains test messages and inline functions for buffer routines
//
/*************************************************************************/

#ifndef __TestMsgs_H
#define __TestMsgs_H

#include "RequestCodes.h"
#include "Message.h"
#include <string.h>

#define UNKNOWN_TEST -2
#define PARSE_ERROR -3

enum TestSGLs {TestName=0, TestArgs, TestWrite, TestStruct};

class TestMsg : public Message {
public:
	TestMsg(char *_pTestName, char *_pTestArgs, int _iTestHandle, REQUESTCODE msgCode) : Message(msgCode) {
			
		int iSize = strlen(_pTestName) + 1;
		pTestName = new char[iSize];
		memcpy(pTestName,_pTestName,iSize);
		AddSgl(TestName, (void *)pTestName, iSize);		
			
		iSize = strlen(_pTestArgs) + 1;
		pTestArgs = new char[iSize];
		memcpy(pTestArgs,_pTestArgs,iSize);
		AddSgl(TestArgs, (void *)pTestArgs, iSize);				
		
		iTestHandle = _iTestHandle;
	}

	TestMsg(Message* pMsg) : Message(pMsg) {
	
		pTestName = NULL;
		pTestArgs = NULL;
	
	}

	~TestMsg() {
	
		delete[] pTestName;
		delete[] pTestArgs;
	
	}
	
	void GetName(char* &pBuf, int &iBufSize) {
	
		U32 iSize = 0;
		void *pvBuf;
		GetSgl(TestName, &pvBuf, &iSize);
		pBuf = (char *)pvBuf;
		iBufSize = iSize;

	}
	
	void GetArgs(char* &pBuf, int &iBufSize) {
	
		U32 iSize = 0;
		void *pvBuf;
		GetSgl(TestArgs, &pvBuf, &iSize);
		pBuf = (char *)pvBuf;
		iBufSize = iSize;

	}

	char *pTestName;
	char *pTestArgs;
	int iTestHandle;
	
};

class TestMsgRegister : public TestMsg {
public:
	TestMsgRegister(char *_pTestName, char *_pTestArgList, DID _did) :
	 TestMsg(_pTestName, _pTestArgList, 0, REQ_TEST_REGISTER) {
		did = _did;
		slot = DeviceId::ISlot(did);			
	}
	
	DID did;
	TySlot slot;
	 
};

class TestMsgRun : public TestMsg {
public:
	TestMsgRun(char *pTestName, char *pTestArgs, int iTestHandle) : TestMsg(pTestName, pTestArgs, iTestHandle, REQ_TEST_RUN) {
		pStruct = NULL;
		AddSgl(TestWrite,0,0,SGL_DYNAMIC_REPLY);		
	}

	TestMsgRun(Message* pMsg) : TestMsg(pMsg) {	
	
		pStruct = NULL;
	
	}

	~TestMsgRun() {
	
		// free the buffer allocated by SetArgsBuffer
		// Note: this destructor is not virtual and
		// must be called as a pointer to THIS class
		// and not the base class to prevent memory leaks
		
	}
	
	void GetBuffer(char* &pBuf, int &iBufSize) {
	
		U32 iSize = 0;
		void *pvBuf;
		GetSgl(TestWrite, &pvBuf, &iSize);
		pBuf = (char *)pvBuf;
		iBufSize = iSize;

	}
	
	// Uses dynamic reply SGL to eliminate virtual deconstructor delete problem
	void SetWriteBuffer(char *pBuf, int iBufSize) {
	
		U32 iSize = iBufSize;
		void* pBufSgl;
		GetSgl(TestWrite,&pBufSgl,&iSize);

		memcpy(pBufSgl,pBuf,iBufSize);

	}

	void GetStruct(char* &pBuf, int &iBufSize) {
	
		U32 iSize = 0;
		void *pvBuf;
		GetSgl(TestStruct, &pvBuf, &iSize);
		pBuf = (char *)pvBuf;
		iBufSize = iSize;

	}
	
	// Uses non-dynamic SGLs due to old reply-only dynamic limitation
	// Should be able to convert by flagging it for the system to delete
	void SetStruct(char *pBuf, int iBufSize) {

		if(pStruct)
			delete[] pStruct;

		pStruct = new char[iBufSize];
		memcpy(pStruct,pBuf,iBufSize);
		
		AddSgl(TestStruct, (void *)pStruct, iBufSize);	
	
	}	

	void SetRunRequest(TestMsgRun* _pRunRequestMsg)
	{
	
		pRunRequestMsg = _pRunRequestMsg;
	
	}

	TestMsgRun* pRunRequestMsg;			

private:
	char *pStruct;
	
};

#endif