//MultiPacketMessageOrganizer.cpp

#include "MultiPacketMessageOrganizer.h"
#include "..\msl\OsHeap.h"

MultiPacketMessageOrganizer::MultiPacketMessageOrganizer() {
	m_apMultiPacketBucket = NULL;
	m_iMultiPacketBucketSize = 0;
}

MultiPacketMessageOrganizer::~MultiPacketMessageOrganizer() {
	if(m_apMultiPacketBucket) {
		for(int i=0;i<m_iMultiPacketBucketSize;i++) {
			delete m_apMultiPacketBucket[i];
		}
		delete[] m_apMultiPacketBucket;
	}
}

void MultiPacketMessageOrganizer::Clean(int iSessionID) {
	for(int i=0;i<m_iMultiPacketBucketSize;i++) {
		if(m_apMultiPacketBucket[i]->m_iSessionID == iSessionID) {
			delete m_apMultiPacketBucket[i];
			for(int j=i + 1;j<m_iMultiPacketBucketSize;j++) {
				m_apMultiPacketBucket[j - 1] = m_apMultiPacketBucket[j];
			}
			m_iMultiPacketBucketSize--;
		}
	}
}

SsapiRequestMessage* MultiPacketMessageOrganizer::Place(SsapiRequestMessage* pMsg) {

	int	i=0;
	if(pMsg->m_iRequestSequence == 0)
		OsHeap::heapSmall.ReportDeltaMemoryUsage("message complete");
	else
		OsHeap::heapSmall.ReportDeltaMemoryUsage("message part recieved");

	int iIndex = -1;

	int iSenderID;
	memcpy(&iSenderID, pMsg->m_acSenderID, sizeof(int));

	for(i=0;i<m_iMultiPacketBucketSize;i++) {
		if(m_apMultiPacketBucket[i]->m_iSenderID == iSenderID) {
			iIndex = i;
			break;
		}
	}
	
	if(iIndex == -1) {
		MultiPacketBucket** aNew = new MultiPacketBucket*[m_iMultiPacketBucketSize + 1];
		if(m_apMultiPacketBucket) {
			memcpy(aNew, m_apMultiPacketBucket, sizeof(MultiPacketBucket*) * m_iMultiPacketBucketSize);
			delete[] m_apMultiPacketBucket;
		}
		m_apMultiPacketBucket = aNew;
		m_apMultiPacketBucket[m_iMultiPacketBucketSize] = new MultiPacketBucket();
		m_apMultiPacketBucket[m_iMultiPacketBucketSize]->m_iSessionID = pMsg->m_iSessionID;
		m_apMultiPacketBucket[m_iMultiPacketBucketSize++]->m_iSenderID = iSenderID;
		iIndex = m_iMultiPacketBucketSize - 1;
	}

	//now, get the data from the message and add it to the bucket at iIndex
	Value* pValue = pMsg->m_pValueSet->GetValue(0);
	char* pData = new char[pValue->GetSize()];
	pValue->GetGenericValue(pData, pValue->GetSize());
	
	m_apMultiPacketBucket[iIndex]->AddData(pData, pValue->GetSize() - SSAPI_INTERNAL_VALUE_HEADER_SIZE - sizeof(int));

	delete[] pData;

	//check to see if this message is complete
	if(pMsg->m_iRequestSequence == 0) {
		//build the new message and clean up this bucket

		//make a generic value data and put it in the value set at ID 0
		pMsg->m_pValueSet->AddValue(SSAPI_TYPE_GENERIC, m_apMultiPacketBucket[iIndex]->m_aData, m_apMultiPacketBucket[i]->m_iDataSize, 0);

		//clean up this one
		delete m_apMultiPacketBucket[iIndex];
		for(int j=iIndex + 1;j<m_iMultiPacketBucketSize;j++) {
			m_apMultiPacketBucket[j - 1] = m_apMultiPacketBucket[j];
		}
		m_iMultiPacketBucketSize--;

		//this message now has all the data in it!
		return pMsg;
	}

	return NULL;
}