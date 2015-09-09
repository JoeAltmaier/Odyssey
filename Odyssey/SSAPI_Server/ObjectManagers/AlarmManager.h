//************************************************************************
// FILE:		AlarmManager.h
//
// PURPOSE:		DEfines the manager that manages SSAPI alarm objects
//************************************************************************

#ifndef __ALARM_MANAGER_H__
#define	__ALARM_MANAGER_H__

#include "ObjectManager.h"
#include "CmdSender.h"
#include "SSAPIAssert.h"

#define	ALARM_MANAGER_NAME	"AlarmManager"

class AlarmManager : public ObjectManager {
	
	static	AlarmManager	*m_pThis;
	bool					m_isInited;
	CmdSender				*m_pCmdSender;


//************************************************************************
// AlarmManager:
//
// PURPOSE:		Default constructor
//************************************************************************

AlarmManager( ListenManager *pListenManager, DdmServices *pParent );


public:


//************************************************************************
// ~AlarmManager:
//
// PURPOSE:		Default destructor
//************************************************************************

~AlarmManager();


//************************************************************************
// GetName:
//
// PURPOSE:		Returns the name of the manager
//************************************************************************

virtual StringClass GetName() { return StringClass(ALARM_MANAGER_NAME); }


//************************************************************************
// Ctor:
//
// PURPOSE:		Creates the manager
//************************************************************************

static ObjectManager* Ctor(	ListenManager			*pLManager, 
							DdmServices				*pParent, 
							StringResourceManager	*pSRManager ){

	return m_pThis? m_pThis : m_pThis = new AlarmManager( pLManager, pParent );
}


//************************************************************************
// Dispatch:
//
// PURPOSE:		Dispatches a request to whoever should service it.
//				
// RECEIVE:		requestParms:		value set with request parms
//				reuestCode:			code of the request
//				pResponder:			a wrapper object that knows how to 
//									respond
//
// NOTE:		All sub-classes that override this method MUST call it on
//				their superclass before processing!!! If this method 
//				returned false, only then should they tray to handle 
//				a request.
//************************************************************************

virtual bool Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder);


protected:

//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

virtual void ObjectDeletedCallbackHandler( SSAPIEvent *pEvent , bool isLast );


//************************************************************************
// InitAMCommandQueueCallback:
//
// PURPOSE:		Indicates that Alarm Master's command q is ready
//************************************************************************

void InitAMCommandQueueCallback( STATUS rc ){ ASSERT( rc == OK ); }


//************************************************************************
// QueryAMReplyCallback:
//
// PURPOSE:		Callback for a alarm query. We need to check for 
//				the context. If it's not null, it's the ptr to RowID of the
//				new/modified alarm which we need to add/modify
//************************************************************************

STATUS QueryAMReplyCallback( Message *pMsg_ );


//************************************************************************
// AlarmEventHandler:
//
// PURPOSE:		Receives and handles events from the alarm master
//************************************************************************

void AlarmEventHandler( STATUS eventCode, void *pEventData );


//************************************************************************
// SendCommandToAM:
//
// PURPOSE:		Builds and sends a command to the Alarm Master.
//				Always responds.
//************************************************************************

void SendCommandToAM( U32 command, DesignatorId alarmId, UnicodeString&, SsapiResponder *pResponder );
void SendCommandToAMCallback(	STATUS			completionCode,
								void			*pResultData,
								void			*pCmdData,
								void			*pCmdContext );


//************************************************************************
// LocalResponderDummyCallback:
//
// PURPOSE:		Callback for local responder objects. Completely dummy
//************************************************************************

void LocalResponderDummyCallback( ValueSet*, bool , int ){}


//************************************************************************
//************************************************************************
//************************************************************************
};

#endif	// __ALARM_MANAGER_H__