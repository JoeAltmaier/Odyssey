//SsapiResponder.h

#ifndef __SsapiResponder_H
#define __SsapiResponder_H

#include "valueset.h"
#include "SsapiRequestMessage.h"
#include "Ddm.h"
#include "SSAPITypes.h"

class SsapiResponder : public DdmServices {
public:
	SsapiResponder(DdmServices* pDdmServices, SsapiRequestMessage* pMsgResp);
	~SsapiResponder();

	virtual int GetSessionID();

	virtual void Respond(ValueSet* pVS, BOOL bIsLast, int iEventObjectID = 0, BOOL bDelete=TRUE);

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

bool RespondToRequest( U32 rc, LocalizedString exceptionString = 0, bool isLastResponse = true );

private:
	SsapiRequestMessage* m_pMsgRespond;
};

#endif