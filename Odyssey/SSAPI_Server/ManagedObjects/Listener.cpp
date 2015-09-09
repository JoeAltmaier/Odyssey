//******************************************************************************
// FILE:		Listener.cpp
//
// PURPOSE:		Implements class Listener which is the datastructure used to 
//				store the Listen requests from external clients.
//******************************************************************************

#include	"Listener.h"



//******************************************************************************
// Listener
//
// PURPOSE:		The default constructor
//******************************************************************************

Listener::Listener(DesignatorId owner_,		DesignatorId objectInQuestion_, 
				   SESSION_ID sessionId_,	CALLBACK_METHOD callbackMethod_, 
				   bool shouldNotifyParent_ ){

	m_owner					= owner_;
	m_objectInQuestion		= objectInQuestion_;
	m_sessionId				= sessionId_;
	m_callbackMethod		= callbackMethod_;
	m_shouldNotifyManager	= shouldNotifyParent_;
}


//******************************************************************************
// ~Listener:
//
// PURPOSE:		The destructor
//******************************************************************************

Listener::~Listener(){
}
