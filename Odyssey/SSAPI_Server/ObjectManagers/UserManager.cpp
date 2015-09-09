//************************************************************************
// FILE:		UserManager.cpp
//
// PURPOSE:		Implements class UserManager : public ObjectManager
//				whose instance will be responsible for the user 
//				management in the O2K.
//************************************************************************


#include "UserManager.h"
#include "ListenManager.h"
#include "..\utils\ShadowTable.h"
#include "SSAPIAssert.h"
#include "UserAccessTable.h"
#include "User.h"
#include "SsapiLocalResponder.h"
#include "SSAPITypes.h"
#include "SsapiAlarms.h"
#include "Event.h"

extern	U32			sizeOfUserAccessTableEntry;
extern	fieldDef	userAccessTableDefintion[];

#include "Trace_Index.h"
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#endif
#define TRACE_INDEX TRACE_SSAPI_MANAGERS


UserManager* UserManager::m_pThis	= NULL;
#define	WRONG_LOGIN_COUNT_BEFORE_ALARM		2
#define	DEFAULT_SECURE_USER_NAME			"dell\0"

//************************************************************************
// UserManager:
//
// PURPOSE:		Default constructor
//************************************************************************

UserManager::UserManager( ListenManager *pListenManager, DdmServices *pParent )
			:ObjectManager( pListenManager, DesignatorId( RowId(), SSAPI_MANAGER_CLASS_TYPE_USER_MANAGER ), pParent ) {

	m_pUserTable = new ShadowTable(	USER_ACCESS_TABLE_NAME,
									this,
									(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS( UserManager, RowInsertedEventHandler ),
									(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS( UserManager, RowDeletedEventHandler ),
									(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS( UserManager, RowModifiedEventHandler ),
									sizeof(UserAccessTableEntry));

	m_isIniting		= true;	
	m_shouldReinit	= false;
	SetIsReadyToServiceRequests( false );

	SSAPI_TRACE( TRACE_L2, "\nUserManager: Initializing....." );
	DefineTable();
}


//************************************************************************
// ~UserManager:
//
// PURPOSE:		The destructor
//************************************************************************

UserManager::~UserManager(){

	delete m_pUserTable;
}


//************************************************************************
// Dispatch:
//
// PURPOSE:		Command dispatcher.
//************************************************************************

bool 
UserManager::Dispatch( ValueSet *pRequestSet, U32 requestCode, SsapiResponder *pResponder ){

	bool			rc = false;
	User			*pUser;

	if( ObjectManager::Dispatch( pRequestSet, requestCode, pResponder ) )
		return true;

	switch(requestCode){
		case SSAPI_USER_MANAGER_LOGIN:
			pUser = new User( GetListenManager() );
			*pUser = *((ValueSet *)pRequestSet->GetValue( SSAPI_USER_MANAGER_LOGIN_OBJECT ));
			rc = Login( pUser, pResponder );
			delete pUser;
			break;
		
		case SSAPI_USER_MANAGER_LOGOUT:
			pUser = new User( GetListenManager() );
			*pUser = *((ValueSet *)pRequestSet->GetValue( SSAPI_USER_MANAGER_LOGOUT_OBJECT ));
			rc = Logout( pUser, pResponder );
			delete pUser;
			break;

		case SSAPI_USER_MANAGER_CHANGE_PASSWORD:
			rc = ChangePassword( *pRequestSet, pResponder );
			break;

		default:
			ASSERT(0);
			break;
	}

	return false;
}


//************************************************************************
// The following are standard methods used while the UserManagerr is 
// initializing.
//************************************************************************

bool 
UserManager::DefineTable(){


	STATUS	status =
	m_pUserTable->DefineTable(	userAccessTableDefintion,
								sizeOfUserAccessTableEntry,
								32,
								(pTSCallback_t)METHOD_ADDRESS( UserManager, DefineTableReplyHandler ),
								NULL );
								
	return (status == OK)? true : false;
}


STATUS
UserManager::DefineTableReplyHandler( void *p, STATUS rc){

	return InitShadowTable()? OK : !OK;
}


bool 
UserManager::InitShadowTable(){

	STATUS	status =
	m_pUserTable->Initialize( (pTSCallback_t)METHOD_ADDRESS( UserManager, InitShadowTableReplyHandler ), NULL );

	return (status == OK)? true : false;
}


STATUS 
UserManager::InitShadowTableReplyHandler( void *p, STATUS rc ){
	
	if( m_shouldReinit ){
		m_shouldReinit = false;
		return InitShadowTable()? OK : !OK;
	}
	
	return m_pUserTable->Enumerate( &m_pTemp, (pTSCallback_t)METHOD_ADDRESS( UserManager, EnumTableReplyHandler ), NULL );
}


STATUS
UserManager::EnumTableReplyHandler( void*, STATUS ){

	UserAccessTableEntry		*pRow = (UserAccessTableEntry *)m_pTemp;
	User						*pUser;
	Container					*pContainer;

	if( m_shouldReinit ){
		m_shouldReinit = false;
		delete m_pTemp;
		return m_pUserTable->Enumerate( &m_pTemp, (pTSCallback_t)METHOD_ADDRESS( UserManager, EnumTableReplyHandler ), NULL );
	}

	pContainer = new SList;
	for( U32 index = 0; index < m_pUserTable->GetNumberOfRows(); index++, pRow++ ){
		pUser = new User( GetListenManager() );
		pUser->BuildYourselfFromPtsRow( pRow );
		pUser->m_pUserManager = this;
		pContainer->Add( (CONTAINER_ELEMENT)pUser );
	}
	AddObjectsIntoManagedObjectsVector( *pContainer );
	delete m_pTemp;
	delete pContainer;

	m_isIniting = m_shouldReinit = false;

	SSAPI_TRACE( TRACE_L2, "\nUserManager: ...Done! Objects built: ", GetManagedObjectCount() );
	SetIsReadyToServiceRequests( true );

	return OK;
}


//************************************************************************
// RowInsertedEventHandler:
//
// PURPOSE:		Called by the PTS when a row is added to the UserAccess 
//				table. Adds a user object, posts an event.
//************************************************************************

STATUS 
UserManager::RowInsertedEventHandler( void *pRow, U32 rowCount, ShadowTable* ){

	Container			*pContainer;

	if( m_isIniting ){
		m_shouldReinit = true;
		return OK;
	}

	User	*pUser = new User( GetListenManager() );
	pUser->BuildYourselfFromPtsRow( (UserAccessTableEntry *)pRow );
	pUser->BuildYourValueSet();
	pUser->m_pUserManager = this;

	pContainer = new SList;
	pContainer->Add( ( CONTAINER_ELEMENT )pUser );
	AddObjectsIntoManagedObjectsVector( *pContainer );
	delete pContainer;

	SetIsReadyToServiceRequests( true );
	return OK;
}


//************************************************************************
// RowDeletedEventHandler:
//
// PURPOSE:		Called by the PTS when a row is deleted from the UserAccess 
//				table. Deletes the user object, posts an event.
//************************************************************************

STATUS
UserManager::RowDeletedEventHandler( void *pRow, U32 rowCount, ShadowTable* ){

	SList container;

	if( m_isIniting ){
		m_shouldReinit = true;
		return OK;
	}

	User	*pUser = new User( GetListenManager() );
	pUser->BuildYourselfFromPtsRow( (UserAccessTableEntry *)pRow );
	
	container.Add( (CONTAINER_ELEMENT)pUser );
	DeleteObjectsFromTheSystem( container );
	
	SetIsReadyToServiceRequests( true );
	return OK;
}


//************************************************************************
// RowModifiedEventHandler:
//
// PURPOSE:		Called by the PTS when a row is modified in the UserAccess 
//				table. Modifies the user object, posts an event.
//************************************************************************

STATUS 
UserManager::RowModifiedEventHandler( void *pRow, U32 rowCount, ShadowTable* ){

	if( m_isIniting ){
		m_shouldReinit = true;
		return OK;
	}

	User	*pUser = new User( GetListenManager() ), *pOldUser;
	pUser->BuildYourselfFromPtsRow( (UserAccessTableEntry *)pRow );
	pUser->m_pUserManager = this;
	
	DesignatorId	id = DesignatorId( pUser->GetDesignatorId()  );
	pOldUser = (User *)GetManagedObject( &id );
	pOldUser->BuildYourValueSet();
	pUser->BuildYourValueSet();
	*((ValueSet *)pOldUser) = *pUser;
	pOldUser->BuildYourselfFromYourValueSet();
	delete pUser;
	GetListenManager()->PropagateObjectModifiedEventForObjects( pOldUser );
	
	SetIsReadyToServiceRequests( true );
	return OK;
}



//************************************************************************
// AddUser:
//
// PURPOSE:		Attempts to add a new user. 
//************************************************************************

bool 
UserManager::AddUser( User *pUser, SsapiResponder *pResponder ){
	
	User					*pManagedObject;
	bool					canAddUser = true;
	ValueSet				*pReturnSet = new ValueSet();
	UserAccessTableEntry	row;

	pUser->BuildYourselfFromYourValueSet();
	// first if such username already exists
	for( U32 index = 0; index < GetManagedObjectCount(); index++ ){
		pManagedObject = (User *)GetManagedObject( index );
		if( pUser->GetUserName() == pManagedObject->GetUserName() ){
			canAddUser = false;
			break;
		}
	}

	if( canAddUser ){
		pUser->WriteYourselfIntoPtsRow( &row );
		m_pUserTable->InsertRow(&row, 
								&m_tempRowId, 
								(pTSCallback_t)METHOD_ADDRESS( UserManager, AddUserReplyHandler ), 
								pResponder );

		SetIsReadyToServiceRequests( false );
	}
	else
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_OBJECT_EXISTS_EXCEPTION );

	delete pReturnSet;
	return true;
}


//************************************************************************
// DeleteUser:
//
// PURPOSE:		Attempts to delete the user
//************************************************************************

bool 
UserManager::DeleteUser( User *pUser, SsapiResponder *pResponder ){

	User					*pManagedObject;
	bool					canDeleteUser = false;
	ValueSet				*pReturnSet = new ValueSet();

	pUser->BuildYourselfFromYourValueSet();

	// first if such username already exists
	for( U32 index = 0; index < GetManagedObjectCount(); index++ ){
		pManagedObject = (User *)GetManagedObject( index );
		if( pUser->GetUserName() == pManagedObject->GetUserName() ){
			canDeleteUser = true;
			break;
		}
	}

	// check if a user is deleting themselves
	if( canDeleteUser ){
		if( pUser->IsThisYourSession( pResponder->GetSessionID() ) ){
			pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_EXCEPTION_CANNOT_DELETE_USER_AS_LOGGED_IN );
			delete pReturnSet;
			return true;
		}
	}
	if( canDeleteUser ){
		m_pUserTable->DeleteRow(pUser->GetDesignatorId().GetRowId(), 
								(pTSCallback_t)METHOD_ADDRESS( UserManager, DeleteUserReplyHandler ), 
								pResponder );

		SetIsReadyToServiceRequests( false );
	}
	else
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_OBJECT_DOES_NOT_EXIST_EXCEPTION );

	delete pReturnSet;
	return true;
}


//************************************************************************
// ModifyUser:
//
// PURPOSE:		Attempts to modify a user
//************************************************************************

bool 
UserManager::ModifyUser( User *pUser, SsapiResponder *pResponder ){
		
	pUser->BuildYourselfFromYourValueSet();

	DesignatorId			id = pUser->GetDesignatorId();
	User					*pManagedObject = (User *) GetManagedObject( &id );
	ValueSet				*pReturnSet = new ValueSet();
	UserAccessTableEntry	row;

	if( pUser->AreThereAnyTooLongStrings( pResponder ) )
		return true;

	if( pManagedObject ){
		User	*p = new User( NULL );
		Value	*pValue;
		pManagedObject->BuildYourValueSet();
		*((ValueSet *)p) = *pManagedObject;
		for( int valuesFound = 0, index = 0; valuesFound < pUser->GetCount(); index++ ){
			if( (pValue = pUser->GetValue( index ) ) != 0 ){
				p->AddValue( pValue, index );
				valuesFound++;
			}
		}
		p->BuildYourselfFromYourValueSet();
		p->WriteYourselfIntoPtsRow( &row );
		delete p;
		m_pUserTable->ModifyRow (pUser->GetDesignatorId().GetRowId(), 
								 &row,
								 (pTSCallback_t)METHOD_ADDRESS( UserManager, ModifyUserReplyHandler ), 
								 pResponder );

		SetIsReadyToServiceRequests( false );
	}
	else
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_OBJECT_DOES_NOT_EXIST_EXCEPTION );

	delete pReturnSet;
	return true;
}


//************************************************************************
// Login:
//
// PURPOSE:		Logs in a user
//************************************************************************

bool 
UserManager::Login( User *pUser, SsapiResponder *pResponder ){

	User				*pManagedObject;
	bool				isAthenticated = false;
	bool				isCorrectUserName = false;

	pUser->BuildYourselfFromYourValueSet();
	for( U32 index = 0; index < GetManagedObjectCount(); index++ ){
		pManagedObject = (User *)GetManagedObject( index );
		if( pManagedObject->GetUserName() == pUser->GetUserName() ){
			isCorrectUserName = true;
			if( pManagedObject->GetPassword() == pUser->GetPassword() ){
				isAthenticated = true;
			}
			else {
				isAthenticated = false;
			}
			break;
		}
	}

	if( !isCorrectUserName )
		pResponder->RespondToRequest( SSAPI_EXCEPTION_SECURITY, CTS_SSAPI_USERNAME_PASSWORD_INVALID );
	else {
		if( isAthenticated ){
			// username as a parameter
			LogEvent( CTS_SSAPI_EVENT_USER_LOGGED_IN, pManagedObject->GetUserName() );
			UpdateWrongLoginCount( pManagedObject, pResponder, false );
		}
		else
			UpdateWrongLoginCount( pManagedObject, pResponder, true );
	}
	return true;
}



//************************************************************************
// Logout:
//
// PURPOSE:		Logs out a user
//************************************************************************

bool 
UserManager::Logout( User *pUser, SsapiResponder *pResponder ){
	
	pUser->BuildYourselfFromYourValueSet();

	DesignatorId		id =  pUser->GetDesignatorId();
	User				*pManagedObject = (User *)GetManagedObject( &id );
	U32					rc;
	ValueSet			*pReturnSet = new ValueSet;

	if( pManagedObject ){
		rc = SSAPI_RC_SUCCESS;
		pManagedObject->DeleteOpenSession( pResponder->GetSessionID() );
		//TBDGAI if we want to show who is present, we need an event here
	}
	else
		rc = SSAPI_EXCEPTION_SECURITY;

	ValueSet	*pRc = new ValueSet;
	pRc->AddInt(rc, SSAPI_RETURN_STATUS );
	if( rc != SSAPI_RC_SUCCESS )
		pRc->AddInt( CTS_SSAPI_USER_WAS_NOT_LOGGED_IN, SSAPI_EXCEPTION_STRING_ID );
	pReturnSet->AddValue( pRc, SSAPI_RETURN_STATUS_SET );

	pResponder->Respond( pReturnSet, TRUE );

	// username as a parameter
	LogEvent( CTS_SSAPI_EVENT_USER_LOGGED_OUT, pManagedObject->GetUserName() );

	delete pReturnSet;
	delete pRc;
	return true;
}


//************************************************************************
// AddUserReplyHandler:
//
// PURPOSE:		Responsible for handling callback for ADD USER
//				pContext points to SsapiResponder object
//************************************************************************

STATUS 
UserManager::AddUserReplyHandler( void *pContext, STATUS rc ){

	U32				status;
	SsapiResponder	*pResponder = (SsapiResponder *)pContext;

	if( rc == OK )
		status = SSAPI_RC_SUCCESS;
	else 
		status = SSAPI_EXCEPTION_INTERNAL;
	
	pResponder->RespondToRequest( status, rc == SSAPI_RC_SUCCESS? 0 : CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );

	return OK;
}


//************************************************************************
// DeleteUserReplyHandler:
//
// PURPOSE:		Responsible for handling callback for DELETE USER
//				pContext points to SsapiResponder object
//************************************************************************

STATUS 
UserManager::DeleteUserReplyHandler( void *pContext, STATUS rc ){

	U32				status;
	SsapiResponder	*pResponder = (SsapiResponder *)pContext;

	if( rc == OK )
		status = SSAPI_RC_SUCCESS;
	else 
		status = SSAPI_EXCEPTION_INTERNAL;

	
	pResponder->RespondToRequest( status, rc == SSAPI_RC_SUCCESS? 0 : CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );
	return OK;
}


//************************************************************************
// ModifyUserReplyHandler:
//
// PURPOSE:		Responsible for handling callback for MODIFY USER
//				pContext points to SsapiResponder object
//************************************************************************

STATUS 
UserManager::ModifyUserReplyHandler( void *pContext, STATUS rc ){

	U32				status;
	SsapiResponder	*pResponder = (SsapiResponder *)pContext;

	if( rc == OK )
		status = SSAPI_RC_SUCCESS;
	else 
		status = SSAPI_EXCEPTION_INTERNAL;

	
	pResponder->RespondToRequest( status, rc == SSAPI_RC_SUCCESS? 0 : CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );
	return OK;
}

//************************************************************************
// AddObject:
//
// PURPOSE:		Adds an object to the system
//
// NOTE:		Must be overridden by object managers that can add objects
//************************************************************************

bool 
UserManager::AddObject( ValueSet &objectValues, SsapiResponder *pResponder ){
	
	bool rc = true;		
	User *pUser = new User( GetListenManager() );

	*pUser = objectValues;
	if( !pUser->AreThereAnyTooLongStrings( pResponder ) )
		rc= AddUser( pUser, pResponder );

	delete pUser;
	return rc;
}


//************************************************************************
// ChangePassword:
//
// PURPOSE:		Attempts to change password
//************************************************************************

bool 
UserManager::ChangePassword( ValueSet &requestParms, SsapiResponder *pResponder ){

	DesignatorId				id;
	UnicodeString				oldPassword, newPassword, password;
	bool						rc = true;
	ManagedObject				*pManagedObject;
	
	if( !requestParms.GetGenericValue( (char *)&id, sizeof(id), SSAPI_USER_MANAGER_CHANGE_PASSWORD_OBJECT_ID ) )
		rc = false;
	else if( !requestParms.GetString( SSAPI_USER_MANAGER_CHANGE_PASSWORD_OLD_PASSWORD, &oldPassword ) )
		rc = false;
	else if(!requestParms.GetString( SSAPI_USER_MANAGER_CHANGE_PASSWORD_NEW_PASSWORD, &newPassword ) )
		rc = false;

	if( !rc )
		return pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );

	if( !( pManagedObject = GetManagedObject( &id ) ) )
		return pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_OBJECT_DOES_NOT_EXIST_EXCEPTION );
	
	pManagedObject->BuildYourValueSet();
	pManagedObject->GetString( SSAPI_USER_FID_PASSWORD, &password );
	pManagedObject->Clear();
	if( (password == oldPassword) == false )
		return pResponder->RespondToRequest( SSAPI_EXCEPTION_SECURITY, CTS_SSAPI_SECURITY_INVALID_PASSWORD );

	User *pUser = new User(0);
	pUser->AddString( &newPassword, SSAPI_USER_FID_PASSWORD );
	pUser->AddGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );
	rc = ModifyUser( pUser, pResponder );
	delete pUser;

	return rc;
}


//************************************************************************
// IsValidRequest:
//
// PURPOSE:		Checks if the given request is allowed for this user.
//				This method only checks for security breaches, not 
//				the validity of the request or the capabilities of the system
//
// RETURN:		true:		carry on!
//				false:		security check failed,
//************************************************************************

bool 
UserManager::IsValidRequest( U32 requestId, SsapiResponder *pResponder ){

	User			*pUser;

	for( U32 i = 0; i < GetManagedObjectCount(); i++ ){
		pUser = (User *)GetManagedObject( i );
		if( pUser->IsThisYourSession( pResponder->GetSessionID() ) ){
			// currently, no other checks are needed, but to be sure the user's logged in
			return true;
		}
	}
	return false;
}


//************************************************************************
// GetUserNameBySessionId:
//
// PURPOSE:		Performs a look up of currently logged in users and
//				returns the username of the one who is logged in with
//				the session id specified
//************************************************************************

UnicodeString 
UserManager::GetUserNameBySessionId( SESSION_ID sessionID ){

	User			*pUser;
	UnicodeString	us;

	if( sessionID == SSAPI_LOCAL_SESSION_ID ){
			// for right now --- no localization. TBDGAI
		return UnicodeString( StringClass( "System" ) );
	}
	else {	// check logged in users
		for( U32 i = 0; i < GetManagedObjectCount(); i++ ){
			pUser = (User *)GetManagedObject( i );
			if( pUser->IsThisYourSession( sessionID ) )
				return pUser->GetUserName();
		}
	}

	ASSERT(0);
	return us;
}


//************************************************************************
// CleanUpForThisSession:
//
// PURPOSE:		Called by the DdmSSAPI when a session is terminated (or 
//				terminates). The manager finds the user who was logged
//				in under this session and removed it from the user's
//				list of sessions
//************************************************************************

void 
UserManager::CleanUpForThisSession( SESSION_ID sessionId ){

	User			*pUser;

	for( U32 i = 0; i < GetManagedObjectCount(); i++ ){
		pUser = (User *)GetManagedObject( i );
		if( pUser->IsThisYourSession( sessionId ) ){
			pUser->DeleteOpenSession( sessionId );
			break;
		}
	}
}


//************************************************************************
// HandleAlarmRecovered:
//
// PURPOSE:		Called by the SSAPI Ddm to inform the manager that a 
//				previously submitted alarm has been recovered. The default
//				implementation does nothing.
//************************************************************************

void 
UserManager::HandleAlarmRecovered( SsapiAlarmContext *pAlarmContext ){

	char *p = new char[pAlarmContext->GetSize()];

	memcpy( p, pAlarmContext, pAlarmContext->GetSize() );
	
	m_submittedAlarms.Add( (CONTAINER_ELEMENT)p );
}


//************************************************************************
// UpdateWrongLoginCount:
//
// PURPOSE:		Updates the number of invalid logins
//************************************************************************

void 
UserManager::UpdateWrongLoginCount( User *pUser, SsapiResponder *pResponder, bool wrongLogin ){

	UPDATE_INVALID_LOGIN_CELL	*pCell = new UPDATE_INVALID_LOGIN_CELL;
	U32							number = wrongLogin? pUser->GetNumberOfWrongLogins() + 1 : 0;

	pCell->pResponder	= pResponder;
	pCell->userId		= pUser->GetDesignatorId();
	pCell->numberOfWrongLogins = number;

	m_pUserTable->ModifyField(	pUser->GetDesignatorId().GetRowId(),
								ftUATE_NUMBER_OF_WRONG_LOGINS,
								&number,
								sizeof(number),
								(pTSCallback_t)METHOD_ADDRESS(UserManager, UpdateWrongLoginCountCallback),
								pCell );

	SetIsReadyToServiceRequests( false );

	if( number - 1 == WRONG_LOGIN_COUNT_BEFORE_ALARM ){
		Event		*pEvent = new Event( CTS_SSAPI_ALARM_TOO_MANY_WRONG_LOGINS );
		SsapiAlarmContextTooManyWrongLogins *pContext = new SsapiAlarmContextTooManyWrongLogins( pUser->GetDesignatorId() );

		// username as a parameter
		pEvent->AddEventParameter( pUser->GetUserName() );
	
		SubmitAlarm( (const Event*)pEvent, pContext, true );
	
		delete pContext;
		delete pEvent;
	}
}


STATUS 
UserManager::UpdateWrongLoginCountCallback( void *pContext, STATUS status ){

	UPDATE_INVALID_LOGIN_CELL	*pCell = (UPDATE_INVALID_LOGIN_CELL *)pContext;
	User						*pUser = (User *)GetManagedObject( &pCell->userId );
	ValueSet					*pRc = new ValueSet, *pReturnSet = new ValueSet;

	if( pUser ){
		if( !pCell->numberOfWrongLogins ){
			pUser->AddOpenSession( pCell->pResponder->GetSessionID() );
			pUser->BuildYourValueSet();
			pRc->AddInt( SSAPI_RC_SUCCESS, SSAPI_RETURN_STATUS );
			pReturnSet->AddValue( pUser, SSAPI_USER_MANAGER_LOGIN_USER_OBJECT );
			pReturnSet->AddValue( pRc, SSAPI_RETURN_STATUS_SET );
			pCell->pResponder->Respond( pReturnSet, TRUE );
		}
		else
			pCell->pResponder->RespondToRequest( SSAPI_EXCEPTION_SECURITY, CTS_SSAPI_USERNAME_PASSWORD_INVALID );
		
		pUser->SetNumberOfWrongLogins( pCell->numberOfWrongLogins );

		delete pRc;
		delete pReturnSet;
	}

	SetIsReadyToServiceRequests( true );
	delete pCell;
	return OK;
}



//************************************************************************
// CheckAndAddWellknownAccount:
//
// PURPOSE:		Checks if the secure account exists. If it does not, creates
//				it.
//************************************************************************

void 
UserManager::CheckAndAddWellknownAccount( StringClass &chassisSerialNumber  ){

	User			*pUser, *pNewUser = NULL;
	U32				i;

	for( i = 0; i < GetManagedObjectCount(); i++ ){
		pUser = (User *)GetManagedObject( i );
		if( pUser->GetUserName() == StringClass(DEFAULT_SECURE_USER_NAME) ){
			pNewUser = pUser;
			break;
		}
	}

	if( pNewUser && (pNewUser->GetPassword() != chassisSerialNumber ) ){
		ValueSet		*pVs = new ValueSet();
		DesignatorId	id = pNewUser->GetDesignatorId();
		UnicodeString	s = pNewUser->GetPassword();
		
		pVs->AddGenericValue( (char *)&id, sizeof(id), SSAPI_USER_MANAGER_CHANGE_PASSWORD_OBJECT_ID );
		pVs->AddString( &s, SSAPI_USER_MANAGER_CHANGE_PASSWORD_OLD_PASSWORD );
		s = UnicodeString(StringClass(chassisSerialNumber));
		pVs->AddString( &s,SSAPI_USER_MANAGER_CHANGE_PASSWORD_NEW_PASSWORD );
		ChangePassword(	*pVs,
						new SsapiLocalResponder(this,
												(LOCAL_EVENT_CALLBACK)METHOD_ADDRESS(UserManager, DummyCallback) ) );
		delete pVs;
	}
	else if( !pNewUser ){
		// create a new dude
		pNewUser = new User( GetListenManager() );
		pNewUser->SetUserName( UnicodeString(StringClass(DEFAULT_SECURE_USER_NAME))  );
		pNewUser->SetPassword( UnicodeString(StringClass(chassisSerialNumber))  );
		AddUser(pNewUser,
				new SsapiLocalResponder(this,
										(LOCAL_EVENT_CALLBACK)METHOD_ADDRESS(UserManager, DummyCallback) ) );
		delete pNewUser;
	}
}

