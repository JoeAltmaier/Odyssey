//************************************************************************
// FILE:		UserManager.h
//
// PURPOSE:		Defines class UserManager : public ObjectManager
//				whose instance will be responsible for the user 
//				management in the O2K.
//************************************************************************

#ifndef	__USER_MANAGER_H__
#define	__USER_MANAGER_H__

#include "ObjectManager.h"
#include "Listener.h"
#include "SList.h"

class ListenManager;
class ShadowTable;
class SsapiResponder;
class User;
class StringClass;

#define		USER_MANAGER_NAME			"UserManager"

#ifdef WIN32
#pragma pack(4)
#endif


class UserManager : public ObjectManager{

	ShadowTable			*m_pUserTable;
	bool				m_isIniting, m_shouldReinit;
	void				*m_pTemp;
	rowID				m_tempRowId;
	SList				m_submittedAlarms;	// ptrs to ssapi alarm copntexts.
	static UserManager	*m_pThis;

friend class User;


//************************************************************************
// UserManager:
//
// PURPOSE:		Default constructor
//************************************************************************

UserManager( ListenManager *pListenManager, DdmServices *pParent );

public:


//************************************************************************
// ~UserManager:
//
// PURPOSE:		The destructor
//************************************************************************

~UserManager();


//************************************************************************
// GetName:
//
// PURPOSE:		Returns the name of the manager
//************************************************************************

virtual StringClass GetName() { return USER_MANAGER_NAME; }


//************************************************************************
// Ctor:
//
// PURPOSE:		Creates the manager
//************************************************************************

static ObjectManager* Ctor(	ListenManager			*pLManager, 
							DdmServices				*pParent, 
							StringResourceManager	*pSRManager ){

	return m_pThis? m_pThis : m_pThis = new UserManager( pLManager, pParent );
}

//************************************************************************
// Dispatch:
//
// PURPOSE:		Command dispatcher.
//************************************************************************

virtual bool Dispatch( ValueSet *pRequestSet, U32 requestCode, SsapiResponder *pResponder );


//************************************************************************
// IsValidRequest:
//
// PURPOSE:		Checks if the given request is allowed for this user.
//				This method only checks for security breaches, not 
//				the validity of the request or the capabilities of the system
//
// RETURN:		true:		carry on!
//				false:		security check failed
//************************************************************************

bool IsValidRequest( U32 requestId, SsapiResponder *pResponder );


//************************************************************************
// GetUserNameBySessionId:
//
// PURPOSE:		Performs a look up of currently logged in users and
//				returns the username of the one who is logged in with
//				the session id specified
//************************************************************************

UnicodeString GetUserNameBySessionId( SESSION_ID sessionID );


//************************************************************************
// CleanUpForThisSession:
//
// PURPOSE:		Called by the DdmSSAPI when a session is terminated (or 
//				terminates). The manager finds the user who was logged
//				in under this session and removed it from the user's
//				list of sessions
//************************************************************************

void CleanUpForThisSession( SESSION_ID sessionId );


//************************************************************************
// CheckAndAddWellknownAccount:
//
// PURPOSE:		Checks if the secure account exists. If it does not, creates
//				it.
//************************************************************************

void CheckAndAddWellknownAccount( StringClass &chassisSerialNumber  );

protected:

//************************************************************************
// HandleAlarmRecovered:
//
// PURPOSE:		Called by the SSAPI Ddm to inform the manager that a 
//				previously submitted alarm has been recovered. The default
//				implementation does nothing.
//************************************************************************

virtual void HandleAlarmRecovered( SsapiAlarmContext *pAlarmContext );



private:


//************************************************************************
// The following are standard methods used while the UserManagerr is 
// initializing.
//************************************************************************

bool DefineTable();
STATUS DefineTableReplyHandler( void *, STATUS );

bool InitShadowTable();
STATUS InitShadowTableReplyHandler( void *, STATUS );
STATUS EnumTableReplyHandler( void*, STATUS );


//************************************************************************
// RowInsertedEventHandler:
//
// PURPOSE:		Called by the PTS when a row is added to the UserAccess 
//				table. Adds a user object, posts an event.
//************************************************************************

STATUS RowInsertedEventHandler( void *pRow, U32 rowCount, ShadowTable* );


//************************************************************************
// RowDeletedEventHandler:
//
// PURPOSE:		Called by the PTS when a row is deleted from the UserAccess 
//				table. Deletes the user object, posts an event.
//************************************************************************

STATUS RowDeletedEventHandler( void *pRow, U32 rowCount, ShadowTable* );


//************************************************************************
// RowModifiedEventHandler:
//
// PURPOSE:		Called by the PTS when a row is modified in the UserAccess 
//				table. Modifies the user object, posts an event.
//************************************************************************

STATUS RowModifiedEventHandler( void *pRow, U32 rowCount, ShadowTable* );


//************************************************************************
// AddUser:
//
// PURPOSE:		Attempts to add a new user. 
//************************************************************************

bool AddUser( User *pUser, SsapiResponder *pResponder );


//************************************************************************
// DeleteUser:
//
// PURPOSE:		Attempts to delete the user
//************************************************************************

bool DeleteUser( User *pUser, SsapiResponder *pResponder );


//************************************************************************
// ModifyUser:
//
// PURPOSE:		Attempts to modify a user
//************************************************************************

bool ModifyUser( User *pUser, SsapiResponder *pResponder );


//************************************************************************
// Login:
//
// PURPOSE:		Logs in a user
//************************************************************************

bool Login( User *pUser, SsapiResponder *pResponder );


//************************************************************************
// Logout:
//
// PURPOSE:		Logs out a user
//************************************************************************

bool Logout( User *pUser, SsapiResponder *pResponder );


//************************************************************************
// AddUserReplyHandler:
//
// PURPOSE:		Responsible for handling callback for ADD USER
//				pContext points to SsapiResponder object
//************************************************************************

STATUS AddUserReplyHandler( void *pContext, STATUS rc );


//************************************************************************
// DeleteUserReplyHandler:
//
// PURPOSE:		Responsible for handling callback for DELETE USER
//				pContext points to SsapiResponder object
//************************************************************************

STATUS DeleteUserReplyHandler( void *pContext, STATUS rc );


//************************************************************************
// ModifyUserReplyHandler:
//
// PURPOSE:		Responsible for handling callback for MODIFY USER
//				pContext points to SsapiResponder object
//************************************************************************

STATUS ModifyUserReplyHandler( void *pContext, STATUS rc );


//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

virtual void ObjectDeletedCallbackHandler( SSAPIEvent *pEvent , bool isLast ) {}


//************************************************************************
// AddObject:
//
// PURPOSE:		Adds an object to the system
//
// NOTE:		Must be overridden by object managers that can add objects
//************************************************************************

virtual bool AddObject( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// ChangePassword:
//
// PURPOSE:		Attempts to change password
//************************************************************************

bool ChangePassword( ValueSet &requestParms, SsapiResponder *pResponder );


//************************************************************************
// UpdateWrongLoginCount:
//
// PURPOSE:		Updates the number of invalid logins
//************************************************************************

void UpdateWrongLoginCount( User *pUser, SsapiResponder *pResponder, bool wrongLogin );
STATUS UpdateWrongLoginCountCallback( void *pContext, STATUS status );


void DummyCallback( ValueSet *, bool , int  ) {};
};


struct UPDATE_INVALID_LOGIN_CELL{

	SsapiResponder		*pResponder;
	DesignatorId		userId;
	U32					numberOfWrongLogins;
};

#endif