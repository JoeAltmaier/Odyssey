//SsapiResponder.cpp

#include "SsapiResponder.h"

#include "..\msl\osheap.h"

SsapiResponder::SsapiResponder(DdmServices* pDdmServices, SsapiRequestMessage* pMsgResp) : DdmServices() {
	SetParentDdm(pDdmServices);
	if(pMsgResp == NULL)
		int josh = 0;
	m_pMsgRespond = pMsgResp;
}

SsapiResponder::~SsapiResponder() {
}

int SsapiResponder::GetSessionID() {
	if(m_pMsgRespond) {
		return m_pMsgRespond->m_iSessionID;
	}
	return 0;
}

void SsapiResponder::Respond(ValueSet* pVS, BOOL bIsLast, int eventObjectId , BOOL bDelete) {
	eventObjectId = 0;	// to avoid warnings

	if(bIsLast) {
		//respond to message with last reply flag set
		m_pMsgRespond->SetResponse(0, (ValueSet*)(pVS->Create()));
		DdmServices::Reply(m_pMsgRespond,OK);
		if(bDelete)
			delete this;
	}
	else {
		//send a response message
		m_pMsgRespond->SetResponse(1, (ValueSet*)(pVS->Create()));
		SsapiRequestMessage* pMsg = new SsapiRequestMessage(m_pMsgRespond);
		*pMsg = *m_pMsgRespond;

		DdmServices::Reply(pMsg, OK);
	}
}

//************************************************************************
// RespondToRequest:
//
// PURPOSE:		Responds to the requestor with a return code only
//
// RECEIVE:		rc:				the return code to respond with
//				isLastResponse:	indicates if that's the last response 
//				exceptionString:id of the message to display if needed
//
// RETURN:		true:			success
//************************************************************************

bool 
SsapiResponder::RespondToRequest( U32 rc, LocalizedString exceptionString, bool isLastResponse ){

	ValueSet		*pReturnSet = new ValueSet, *pRcSet = new ValueSet;

	pRcSet->AddInt( rc, SSAPI_RETURN_STATUS );

	if( exceptionString )
		pRcSet->AddInt( exceptionString, SSAPI_EXCEPTION_STRING_ID );

	pReturnSet->AddValue( pRcSet, SSAPI_RETURN_STATUS_SET );
	
	Respond( pReturnSet, isLastResponse, 0, FALSE);

	delete pReturnSet;
	delete pRcSet;

	if(isLastResponse)
		delete this;

	return true;
}