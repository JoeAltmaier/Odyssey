//SsapiRequestMessage.cpp

#include "SsapiRequestMessage.h"

#define RESPONSE_MINIMUM_SIZE 12
//responses have [senderID = 4][object code = 2][request code = 2][sequence = 4] = 12

SsapiRequestMessage::SsapiRequestMessage(REQUESTCODE reqCode) : Message(reqCode) {
	InternalInit();
}

SsapiRequestMessage::SsapiRequestMessage(Message* pMsg) : Message(pMsg) {
	InternalInit();
}

void SsapiRequestMessage::InternalInit() {
	m_iSessionID = 0;
	m_acSenderID[0] = 0;
	m_cPriority = 0;//request only
	m_iObjectCode = 0;
	m_iRequestCode = 0;
	m_pValueSet = new ValueSet();
	
	//response space
	m_iDataSize = 0;

	m_iRequestSequence = 0;

	m_iResponseSequence = 0;
	m_pResponseValueSet = NULL;
}

SsapiRequestMessage::~SsapiRequestMessage() {
	Clear();
}

void SsapiRequestMessage::Clear() {
	if(m_pValueSet)
		delete m_pValueSet;
	if(m_pResponseValueSet)
		delete m_pResponseValueSet;	
	m_pValueSet = NULL;
	m_pResponseValueSet = NULL;
	m_iSessionID = 0;
	m_acSenderID[0] = 0;
	m_iObjectCode = 0;
	m_iRequestCode = 0;
	m_iDataSize = 0;
	m_iResponseSequence = 0;
}

SsapiRequestMessage& SsapiRequestMessage::operator =(SsapiRequestMessage& msg) {
	Clear();
	m_iSessionID = msg.m_iSessionID;
	memcpy(m_acSenderID, msg.m_acSenderID, 4);
	m_cPriority = msg.m_cPriority;
	m_iObjectCode = msg.m_iObjectCode;
	m_iRequestCode = msg.m_iRequestCode;
	m_pValueSet = (ValueSet*)msg.m_pValueSet->Create();
	m_pResponseValueSet = (ValueSet*)msg.m_pResponseValueSet->Create();
	m_iDataSize = msg.m_iDataSize;
	m_iResponseSequence = msg.m_iResponseSequence;
	return msg;
}

void SsapiRequestMessage::SetResponse(int iSequence, ValueSet* pResponseValueSet) {
	if(m_pResponseValueSet)
		delete m_pResponseValueSet;
	m_pResponseValueSet = pResponseValueSet;
	m_iDataSize = RESPONSE_MINIMUM_SIZE + m_pResponseValueSet->GetSize();
	m_iResponseSequence = iSequence;
}