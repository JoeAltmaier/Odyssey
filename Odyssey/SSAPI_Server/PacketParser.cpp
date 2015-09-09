//PacketParser.cpp

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"
#include "stdio.h"
#include "string.h"
#include "OsTypes.h"
#include "PacketParser.h"
#include "byte_array_utils.h"

#include "..\msl\OsHeap.h"

PacketParser::PacketParser() {
	m_pReadBuffer = NULL;
	m_iReadBufferSize = 0;
	m_iReadBufferAvail = 0;
}

PacketParser::~PacketParser() {
	if(m_pReadBuffer)
		delete[] m_pReadBuffer;
}

void PacketParser::Package(NetMsgWrite* pWriteMsg, SsapiRequestMessage* pSsapiMsg) {
	char* pBuffer = new char[pSsapiMsg->m_iDataSize + 10];//10 is for packet overhead
	int iOffset = 0;

	pBuffer[iOffset++] = '<';
	iOffset += byte_array_utils::Write(pBuffer + iOffset, (char*)&(pSsapiMsg->m_iDataSize),sizeof(int));
	iOffset += byte_array_utils::Write(pBuffer + iOffset, (char*)&(pSsapiMsg->m_iSessionID), sizeof(int));

	int iEncryptStart = iOffset;
	memcpy(pBuffer + iOffset, pSsapiMsg->m_acSenderID, 4);
	iOffset += 4;
	iOffset += byte_array_utils::Write(pBuffer + iOffset, (char*)&(pSsapiMsg->m_iObjectCode), sizeof(short));
	iOffset += byte_array_utils::Write(pBuffer + iOffset, (char*)&(pSsapiMsg->m_iRequestCode), sizeof(short));
	iOffset += byte_array_utils::Write(pBuffer + iOffset, (char*)&(pSsapiMsg->m_iResponseSequence), sizeof(int));

	//add all the arguments here
	pSsapiMsg->m_pResponseValueSet->m_iCode = 0;
	iOffset += pSsapiMsg->m_pResponseValueSet->Write(pBuffer + iOffset);

	Encrypt(pBuffer + iEncryptStart, iOffset - iEncryptStart, pSsapiMsg->m_iSessionID);

	pBuffer[iOffset++] = '>';

	pWriteMsg->SetBuffer(pBuffer, iOffset);
	delete[] pBuffer;
}

void PacketParser::Encrypt(char* pData, int iDataSize, int iEncryptionKey) {
	char aKey[4];
#ifdef WIN32
	aKey[0] = ((char*)&iEncryptionKey)[1] + 28;
	aKey[1] = ((char*)&iEncryptionKey)[0] + 1;
	aKey[2] = ((char*)&iEncryptionKey)[2];
	aKey[3] = ((char*)&iEncryptionKey)[3] - 6;
#else
	aKey[0] = ((char*)&iEncryptionKey)[2] + 28;
	aKey[1] = ((char*)&iEncryptionKey)[3] + 1;
	aKey[2] = ((char*)&iEncryptionKey)[1];
	aKey[3] = ((char*)&iEncryptionKey)[0] - 6;
#endif

	for(int i=0;i<iDataSize;i++) {
		pData[i] = pData[i] ^ aKey[i % 4];
	}
}

void PacketParser::AddData(char* pData, int iDataSize) {
	//place data in our buffer
	if(m_iReadBufferAvail - m_iReadBufferSize <= iDataSize) {
		//grow the buffer
		char* pTemp = new char[m_iReadBufferAvail + iDataSize];
		if(m_pReadBuffer) {
			memcpy(pTemp, m_pReadBuffer, m_iReadBufferSize);
			delete[] m_pReadBuffer;
		}
		m_iReadBufferAvail += iDataSize;
		m_pReadBuffer = pTemp;
	}
	
	//copy the data
	memcpy(m_pReadBuffer + m_iReadBufferSize, pData, iDataSize);
	m_iReadBufferSize += iDataSize;
}

SsapiRequestMessage* PacketParser::GetCompleteMessage()  {

	//check for complete message
	if(!m_iReadBufferSize)
		return NULL;	
	
	int iStart = 0;
	while(iStart < m_iReadBufferSize && m_pReadBuffer[iStart] != '<')
		iStart++;
	
	if(m_iReadBufferSize - iStart <= sizeof(int) * 2 + 1) {
		return NULL;
	}
	
	iStart++;
	
	int iSize;
	iStart += byte_array_utils::Write((char*)&iSize, m_pReadBuffer + iStart, sizeof(int));
	
	int iSessionID;
	iStart += byte_array_utils::Write((char*)&iSessionID, m_pReadBuffer + iStart, sizeof(int));

	if(iSize + 1 > m_iReadBufferSize - iStart) {
		return NULL;
	}
	
	if(m_pReadBuffer[iStart + iSize] != '>') {
		printf("Packet Parser found message to be corrupt, bad close token\n");
		//corrupt message, colapse
		Colapse(iStart + iSize + 1);
		return NULL;
	}
	
	Encrypt(m_pReadBuffer + iStart, iSize, iSessionID);

	iSize += 10;//added for the packet overhead

	//populate SsapiRequestMessage
	SsapiRequestMessage* pMsg = new SsapiRequestMessage();
	memcpy((char*)(pMsg->m_acSenderID),m_pReadBuffer + iStart, 4);
	iStart += 4;

	iStart += byte_array_utils::Write((char*)&(pMsg->m_iRequestSequence), m_pReadBuffer + iStart, sizeof(short));

	pMsg->m_cPriority = m_pReadBuffer[iStart++];

	iStart += byte_array_utils::Write((char*)&(pMsg->m_iObjectCode),m_pReadBuffer + iStart, sizeof(short int));
	
	iStart += byte_array_utils::Write((char*)&(pMsg->m_iRequestCode), m_pReadBuffer + iStart, sizeof(short int));

	int iCode;
	iStart += byte_array_utils::Write((char*)&iCode, m_pReadBuffer + iStart, sizeof(int));

	iStart++;

	pMsg->m_pValueSet->Parse(m_pReadBuffer + iStart, iSize - iStart - 1);

	Colapse(iSize);
	
	return pMsg;
}

void PacketParser::Colapse(int iStart) {
	memmove(m_pReadBuffer, m_pReadBuffer + iStart, m_iReadBufferSize - iStart);
	m_iReadBufferSize -= iStart;
}