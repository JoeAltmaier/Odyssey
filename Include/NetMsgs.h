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
// File: NetMsgs.h
//
// Description:
//    CHAOS TCP/IP Network Service Messages Header
//	  Contains network messages and inline functions for buffer routines
//
/*************************************************************************/

#ifndef __NetMsgs_H
#define __NetMsgs_H

#include "RequestCodes.h"
#include "Message.h"
#include <string.h>
#include "Oos\Services.h"

class NetMsg : public Message {
public:
	NetMsg(REQUESTCODE msgCode) : Message(msgCode) {
		m_iConnectionID = 0;
	}

	NetMsg(Message* pMsg) : Message(pMsg) {}

	~NetMsg() {}

	void SetConnectionID(int iID) {
		m_iConnectionID = iID;
	}
	
	int m_iConnectionID;
};

class NetMsgChangeIP : public Message {
public:
	NetMsgChangeIP(char cIPa, char cIPb, char cIPc, char cIPd) : Message(REQ_NET_CHANGEIP) {	
		m_acIPAddress[0] = cIPa;
		m_acIPAddress[1] = cIPb;
		m_acIPAddress[2] = cIPc;
		m_acIPAddress[3] = cIPd;
		
		m_acSubnet[0] = 0;		
		m_acSubnet[1] = 0;		
		m_acSubnet[2] = 0;		
		m_acSubnet[3] = 0;								
	}

	NetMsgChangeIP(char cIPa, char cIPb, char cIPc, char cIPd, char cSUBa, char cSUBb, char cSUBc, char cSUBd) : Message(REQ_NET_CHANGEIP) {	
		m_acIPAddress[0] = cIPa;
		m_acIPAddress[1] = cIPb;
		m_acIPAddress[2] = cIPc;
		m_acIPAddress[3] = cIPd;
		
		m_acSubnet[0] = cSUBa;		
		m_acSubnet[1] = cSUBb;		
		m_acSubnet[2] = cSUBc;		
		m_acSubnet[3] = cSUBd;								
	}

	char m_acIPAddress[4];
	char m_acSubnet[4];

};

class NetMsgChangeGateway : public Message {
public:
	NetMsgChangeGateway(char cIPa, char cIPb, char cIPc, char cIPd) : Message(REQ_NET_CHANGEGATEWAY) {	
		m_acIPAddress[0] = cIPa;
		m_acIPAddress[1] = cIPb;
		m_acIPAddress[2] = cIPc;
		m_acIPAddress[3] = cIPd;
	}

	char m_acIPAddress[4];

};

class NetMsgConnectionOriented : public NetMsg {
public:
	NetMsgConnectionOriented(REQUESTCODE msgCode, char cIPa, char cIPb, char cIPc, char cIPd, int iPort) : NetMsg(msgCode) {
		m_acIPAddress[0] = cIPa;
		m_acIPAddress[1] = cIPb;
		m_acIPAddress[2] = cIPc;
		m_acIPAddress[3] = cIPd;
		m_iPort = iPort;
	}

	char m_acIPAddress[4];
	int m_iPort;
};

class NetMsgListen : public NetMsgConnectionOriented {
public:
	NetMsgListen(char cIPa, char cIPb, char cIPc, char cIPd, int iPort) : NetMsgConnectionOriented(REQ_NET_LISTEN,cIPa, cIPb, cIPc, cIPd, iPort) {
	}
};

class NetMsgConnect : public NetMsgConnectionOriented {
public:
	NetMsgConnect(char cIPa, char cIPb, char cIPc, char cIPd, int iPort) : NetMsgConnectionOriented(REQ_NET_CONNECT,cIPa, cIPb, cIPc, cIPd, iPort) {
	}
};

// Kills a connection being managed by DdmNet
// IMPORTANT:  The client must not issue the kill request until he is sure that
//  all of his writes have been completed.  If this does not occur, any outstanding
//  writes will be lost before the connection is closed
class NetMsgKill : public NetMsg {
public: 
	NetMsgKill(int iConnID) : NetMsg(REQ_NET_KILL) {
		SetConnectionID(iConnID);
	}
};

class BufferedNetMsg : public NetMsg {
public:
	BufferedNetMsg(int iConnID, int iSessionID,REQUESTCODE msgCode) : NetMsg(msgCode) {
		SetConnectionID (iConnID);
		SetSessionID (iSessionID);
		m_iProcessPos = 0;
	}
	
	BufferedNetMsg(Message* pMsg) : NetMsg(pMsg) {}

	~BufferedNetMsg() {}

	void SetSessionID(int iSessionID) {
		m_iSessionID = iSessionID;
	}

	// generic inline getbuffer routine
	//  - sets your buffer pointer and size to the SGL's pointer and size
	void GetBuffer(char* &pBuf, int &iBufSize) {
	
		U32 iSize = 0;
		void *pvBuf;
		GetSgl(0, &pvBuf, &iSize);
		pBuf = (char *)pvBuf;
		iBufSize = iSize;
	
	}

	int m_iProcessPos;
	int m_iSessionID;

};

class NetMsgWrite : public BufferedNetMsg {
public:
	NetMsgWrite(int iConnID, int iSessionID) : BufferedNetMsg(iConnID, iSessionID, REQ_NET_WRITE) {	
		pBufWrite = NULL;
	}
	
	~NetMsgWrite() {
	
		// free the buffer allocated by SetBuffer
		// Note: this destructor is not virtual and
		// must be called as a pointer to THIS class
		// and not the base class to prevent memory leaks
//		Tracef("Deleting buffer\n");
		if(pBufWrite)
			delete[] pBufWrite;
		
	}
	
	// write message specific set buffer
	// copies data from the user buffer to a message buffer.
	// this buffer is then added to the SGLs
	// Note: convert to a dynamic send SGL ASAP to
	// prevent above virtual destructor limitation
	void SetBuffer(char *pBuf, int iBufSize) {
		
//		Tracef("Write->SetBuffer\n");

		if(pBufWrite)
			delete[] pBufWrite;

		pBufWrite = new(tZERO) char[iBufSize];
		memcpy(pBufWrite,pBuf,iBufSize);
		
		AddSgl(0, (void *)pBufWrite, iBufSize);
	
	}		
	
private:
	char *pBufWrite;
};

class NetMsgRead : public BufferedNetMsg {
public:
	NetMsgRead(int iConnID, int iSessionID) : BufferedNetMsg(iConnID,iSessionID,REQ_NET_READ) {
		AddSgl(0,0,0,SGL_DYNAMIC_REPLY);
	}

	NetMsgRead(Message* pMsg) : BufferedNetMsg(pMsg) {
	}

	// read specific set buffer
	// utilizes dynamic reply SGLs since read buffers
	// will be in reply - eliminates buffer freeing issues
	// with message and client
	void SetBuffer(char* pBuf, int iBufSize) {

//		Tracef("Read->SetBuffer\n");
		U32 iSize = iBufSize;
		void* pBufSgl;
		GetSgl(0,&pBufSgl,&iSize);

		memcpy(pBufSgl,pBuf,iBufSize);
	}	
};

#endif