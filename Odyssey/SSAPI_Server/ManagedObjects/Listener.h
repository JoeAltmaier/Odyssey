//******************************************************************************
// FILE:		Listener.h
//
// PURPOSE:		Defines class Listener which is the datastructure used to 
//				store the Listen requests from external clients.
//******************************************************************************

#ifndef __SSAPI_LISTER_H__
#define	__SSAPI_LISTER_H__

#include	"CtTypes.h"
#include	"..\utils\DesignatorId.h"
#include	"..\SSAPIResponder.h"

typedef 	U32					SESSION_ID;

struct CALLBACK_METHOD{
	SsapiResponder		*pResponder;
	int					eventObjectCode;

	CALLBACK_METHOD( SsapiResponder* pR = NULL, int eOC = 0 ) { pResponder = pR; eventObjectCode = eOC; }
};

class Listener{
	
	SESSION_ID			m_sessionId;
	CALLBACK_METHOD		m_callbackMethod;
	DesignatorId		m_owner;				// who to notify
	DesignatorId		m_objectInQuestion;		// who to listen at
	bool				m_shouldNotifyManager;

public:
//******************************************************************************
// Listener
//
// PURPOSE:		The default constructor
//******************************************************************************

Listener(	DesignatorId owner_,	DesignatorId	objectInQuestion_,	 
			SESSION_ID sessionId_,	CALLBACK_METHOD	callbackMethod_,
			bool shouldNotifyParent_ = false );


//******************************************************************************
// ~Listener:
//
// PURPOSE:		The destructor
//******************************************************************************

~Listener();


//******************************************************************************
// GetOwner:
//
// PUEPOSE:		An accessor
//
// RETURN:		the owner's id
//******************************************************************************

DesignatorId GetOwner() const { return m_owner; }


//******************************************************************************
// GetObjectInQuestion:
//
// PURPOSE:		An accessor
//
// RETURN:		the id of the object in question
//******************************************************************************

DesignatorId GetObjectInQuestion() const { return m_objectInQuestion; }


//******************************************************************************
// GetSessionId:
//
// PRUPOSE:		An accessor
//
// RETURN:		session id 
//******************************************************************************

SESSION_ID	GetSessionId() const { return m_sessionId; }
CALLBACK_METHOD GetCallback() const { return m_callbackMethod; }
bool ShouldNotifyManager() const { return m_shouldNotifyManager; }

};

#endif	//__SSAPI_LISTER_H__