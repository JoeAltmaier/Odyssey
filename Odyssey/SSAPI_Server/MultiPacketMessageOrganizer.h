//MultiPacketMessageOrganizer.h

#ifndef __MultiPacketMessageOrganizer_H
#define __MultiPacketMessageOrganizer_H

#include "SsapiRequestMessage.h"

class MultiPacketBucket {
public:
	char* m_aData;
	int m_iDataSize;
	int m_iSessionID;
	int m_iSenderID;

	MultiPacketBucket() {
		m_aData = NULL;
		m_iDataSize = 0;
		m_iSenderID = -1;
		m_iSessionID = -1;
	}

	~MultiPacketBucket() {
		if(m_aData)
			delete[] m_aData;
	}

	//returns true if the data was for this bucket... false otherwise
	BOOL AddData(char* aData, int iDataSize) {

		char* aNewData = new(tSMALL) char[m_iDataSize + iDataSize];
		if(m_aData) {
			memcpy(aNewData, m_aData, m_iDataSize);
			delete[] m_aData;
		}

		memcpy(aNewData + m_iDataSize, aData, iDataSize);
		m_iDataSize += iDataSize;
		
		m_aData = aNewData;

		return TRUE;
	}
	

};

class MultiPacketMessageOrganizer {
public:
	MultiPacketMessageOrganizer();
	~MultiPacketMessageOrganizer();

	void Clean(int iSessionID);

	SsapiRequestMessage* Place(SsapiRequestMessage* pMsg);

private:
	MultiPacketBucket** m_apMultiPacketBucket;
	int m_iMultiPacketBucketSize;
};

#endif