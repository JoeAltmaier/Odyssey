//SsapiRequestMessage.h

#ifndef __SsapiRequestMessage_H
#define __SsapiRequestMessage_H

#include "Message.h"
#include "RequestCodes.h"
#include "valueset.h"

#define SSAPI_SESSION_TERMINATED -1

class SsapiRequestMessage : public Message {
public:
	SsapiRequestMessage(REQUESTCODE reqCode = REQ_SSAPI_REQ);

	SsapiRequestMessage(Message* pMsg);

	void InternalInit();

	~SsapiRequestMessage();
	
	void Clear();
	SsapiRequestMessage& operator =(SsapiRequestMessage& pMsg);

	int m_iSessionID;
	char m_acSenderID[4];
	char m_cPriority;
	short int m_iObjectCode;
	short int m_iRequestCode;

	short int m_iRequestSequence;

	int m_iDataSize;
	
	ValueSet* m_pValueSet;
	
	//response related
	void SetResponse(int iSequence, ValueSet* pResponseValueSet);
	ValueSet* m_pResponseValueSet;
	int m_iResponseSequence;
};

#endif