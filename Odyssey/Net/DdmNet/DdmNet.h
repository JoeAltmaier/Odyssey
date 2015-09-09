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
// File: DdmNet.h
//
// Description:
//    CHAOS TCP/IP Network Services DDM Header
//
/*************************************************************************/

#ifndef __DdmNet_H
#define __DdmNet_H

#define PLUS

#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"
#include "externs.h"
#include "NetMsgs.h"
#include "Network.h"

void ListenThread(void* pParam);
void ReadWriteThread(void* pParam);

class Connection {

public:

	Connection() {
//		Kernel::Create_Semaphore(&m_Semaphore, "connection_semaphore",1);
		m_iWriteMsgs = 0;
		m_apWriteMsgs = NULL;
		m_iConnectionID = 0;
		m_pReadRequest = NULL;
		m_bConnected = FALSE;
	}
	
	BOOL m_bConnected;
//	CT_Semaphore m_Semaphore;
	
	NetMsgRead* m_pReadRequest;
	
	int m_iWriteMsgs;
	Message** m_apWriteMsgs;
	
	int m_sock;
	int m_iConnectionID;
};	

class DdmNet : public Ddm {

protected:
	void	*rwStack;
	void	*lStack;
	
	CT_Task	*rwTask;
	CT_Task	*lTask;	
	
	void ResetSignalSocket(int signalSocket);
	void SetSignalSocket(int signalsocket);
	void FillReadFS();	
	
public:
	static Ddm *Ctor(DID did);
	DdmNet(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);	
	STATUS Quiesce(Message *pMsg);


	// move to protected?
	
	//message handlers
	STATUS ProcessListen(Message *pMsg);
	STATUS ProcessConnect(Message *pMsg);
	STATUS ProcessKill(Message *pMsg);
	STATUS ProcessWrite(Message *pMsg);
	STATUS ProcessRead(Message *pMsg);
	STATUS ProcessChangeIP(Message *pMsg);
	STATUS ProcessChangeGateway(Message *pMsg);
	

	//signal handlers 
	STATUS HandleAccept(SIGNALCODE nSignal,void* pParam);
	STATUS HandleRead(SIGNALCODE nSignal,void* pParam);
	STATUS HandleWritten(SIGNALCODE nSignal, void* pParam);
	STATUS HandleError(SIGNALCODE nSignal, void* pParam);	

	//thread methods
	void DoListen();
	void DoReadWrite();
	
private:
	Connection* GetConnection(int iConnectionID);
	void RemoveConnection(int iConnectionID);

	int m_iConnectionSize;
	Connection* m_aConnection;

	void QueMsg(Message** & apMsgQue, int *iMsgQueSize, Message* pMsgReq);
	Message* EnqueMsg(Message** & apMsgQue, int *iMsgQueSize);
	Message* PeekMsg(Message** apMsgQue, int iMsgQueSize);

	CT_Semaphore m_ListenSemaphore;
	int m_iListenMsgs;
	Message** m_apListenMsgs;
	
	CT_Semaphore m_readSemaphore;

	int		reloadSignalSocket;
	int		writeSignalSocket;
    FD_SET	readfs;	
    
	char ip_ethernet[4];    
	char subnet_ethernet[4];    
	char ip_ppp_server[4];    
	char ip_ppp_client[4];    			
	char subnet_ppp_server[4];
	char subnet_ppp_client[4];
	char ip_gateway[4];
    	
};

BOOL ReportError(int iRet, int socketd, char *file, int line);
BOOL ReportError(int iRet, char *file, int line);
BOOL ReportError(int iRet);

#endif	// __DdmNet_H

