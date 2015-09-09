//PacketParser.h

#ifndef __PacketParser_H
#define __PacketParser_H

#include "SsapiRequestMessage.h"
#include "NetMsgs.h"

class PacketParser {
public:
	PacketParser();
	~PacketParser();
	
	void Package(NetMsgWrite* pWriteMsg, SsapiRequestMessage* pSsapiMsg);

	//returns a valid pointer if a complete message was parsed
	void AddData(char* pData, int iDataSize);
	SsapiRequestMessage* GetCompleteMessage();
private:
	void Encrypt(char* pData, int iDataSize, int iEncryptionKey);
	void Colapse(int iStart);
	char* m_pReadBuffer;
	int m_iReadBufferSize;
	int m_iReadBufferAvail;
	
};

#endif