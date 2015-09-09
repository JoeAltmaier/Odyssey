//************************************************************************
// FILE:		User.cpp
//
// PURPOSE:		Implements class User which is a managed object and contains
//				data atributes of an O2K user
//************************************************************************



#include "User.h"
#include "..\ObjectManagers\ListenManager.h"
#include "UserAccessTable.h"
#include "SList.h"
#include "UserManager.h"

	

//************************************************************************
// User:
//
// PURPOSE:		A default constructor for the class
//************************************************************************

User::User( ListenManager *pListenManager )
	:ManagedObject( pListenManager, SSAPI_OBJECT_CLASS_TYPE_USER ){

//	m_pOpenSessions = new SList;
	m_manager		= DesignatorId( RowId(), SSAPI_MANAGER_CLASS_TYPE_USER_MANAGER );
}


//************************************************************************
// ~User:
//
// PURPOSE:		The destructor
//************************************************************************

User::~User(){

//	delete m_pOpenSessions;
}


//************************************************************************
// BuildYourselfFromPtsRow
// 
// PURPOSE:		Populates itself off a PTS row
//************************************************************************

bool 
User::BuildYourselfFromPtsRow( UserAccessTableEntry *pRow ){

	
	m_userName		= UnicodeString( (char *)&pRow->userName );
	m_password		= UnicodeString( (char *)&pRow->password );
	m_firstName		= UnicodeString( (char *)&pRow->firstName );
	m_lastName		= UnicodeString( (char *)&pRow->lastName );
	m_description	= UnicodeString( (char *)&pRow->description );
	m_email			= UnicodeString( (char *)&pRow->email );
	m_phoneNumber1	= UnicodeString( (char *)&pRow->phoneNumber1 );
	m_phoneNumber2	= UnicodeString( (char *)&pRow->phoneNumber2 );
	m_department	= UnicodeString( (char *)&pRow->department );
	m_id			= DesignatorId( RowId(pRow->rid), (U16)GetClassType() );
	m_language		= pRow->language;
	m_securityPolicy= pRow->securityPolicy;	

	m_numberOfInvalidLogins = pRow->numberOfInvalidLogins;

	return true;
}



//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		Populates its value set based on the data members
//************************************************************************

bool 
User::BuildYourValueSet(){
	ManagedObject::BuildYourValueSet();

	AddValue( &m_userName , SSAPI_USER_FID_USERNAME );
	AddValue( &m_password , SSAPI_USER_FID_PASSWORD );
	AddValue( &m_firstName , SSAPI_USER_FID_FIRST_NAME );
	AddValue( &m_lastName , SSAPI_USER_FID_LAST_NAME );
	AddValue( &m_description , SSAPI_USER_FID_DESCRIPTION );
	AddValue( &m_email , SSAPI_USER_FID_EMAIL );
	AddValue( &m_phoneNumber1 , SSAPI_USER_FID_PHONE_1 );
	AddValue( &m_phoneNumber2 , SSAPI_USER_FID_PHONE_2 );
	AddValue( &m_department , SSAPI_USER_FID_DEPARTMENT);
	AddInt(m_language, SSAPI_USER_FID_LANGUAGE );

	return true;
}


//************************************************************************
// BuildYourselfFromYourValueSet:
//
// PURPOSE:		Fetches data from its ValueSet into its data members
//************************************************************************

bool 
User::BuildYourselfFromYourValueSet(){
	
	ManagedObject::BuildYourselfFromYourValueSet();
	
	GetString( SSAPI_USER_FID_USERNAME, &m_userName );
	GetString( SSAPI_USER_FID_PASSWORD, &m_password );
	GetString( SSAPI_USER_FID_FIRST_NAME, &m_firstName );
	GetString( SSAPI_USER_FID_LAST_NAME, &m_lastName );
	GetString( SSAPI_USER_FID_DESCRIPTION, &m_description);
	GetString( SSAPI_USER_FID_EMAIL, &m_email );
	GetString( SSAPI_USER_FID_PHONE_1, &m_phoneNumber1 );
	GetString( SSAPI_USER_FID_PHONE_2, &m_phoneNumber2 );
	GetString( SSAPI_USER_FID_DEPARTMENT, &m_department );
	GetInt(SSAPI_USER_FID_LANGUAGE, (int *)&m_language);

	return true;
}


//************************************************************************
// WriteYourselfIntoPtsRow:
//
// PURPOSE:		Populates a PTS row with the information contained 
//				in its data members. The PTS row must be ready to be 
//				written after this call has returned.
//************************************************************************

bool 
User::WriteYourselfIntoPtsRow( UserAccessTableEntry *pRow ){
	
	pRow->version	= USER_ACCESS_TABLE_VERSION;
	pRow->size		= sizeof(UserAccessTableEntry);

	m_userName.CString( &pRow->userName, sizeof( pRow->userName ) );
	m_password.CString( &pRow->password, sizeof( pRow->password ) );
	m_firstName.CString( &pRow->firstName, sizeof( pRow->firstName ) );
	m_lastName.CString( &pRow->lastName, sizeof( pRow->lastName ) );
	m_description.CString( &pRow->description, sizeof( pRow->description ) );
	m_email.CString( &pRow->email, sizeof( pRow->email ) );
	m_phoneNumber1.CString( &pRow->phoneNumber1, sizeof( pRow->phoneNumber1 ) );
	m_phoneNumber2.CString( &pRow->phoneNumber2, sizeof( pRow->phoneNumber2 ) );
	m_department.CString( &pRow->department, sizeof( pRow->department ) );
	pRow->language = m_language;
	pRow->securityPolicy = m_securityPolicy;
	pRow->numberOfInvalidLogins = m_numberOfInvalidLogins;

	return true;
}

//************************************************************************
// AddOpenSession:
//
// PURPOSE:		Adds a new open session id into open session container
//************************************************************************

bool 
User::AddOpenSession( SESSION_ID sessionId ){
	
	return 	m_openSessions.Add( (CONTAINER_ELEMENT)sessionId, (CONTAINER_KEY)sessionId )
			? true : false;
}


//************************************************************************
// DeleteOpenSession:
//
// PURPOSE:		Removes session id from open session container
//************************************************************************

bool 
User::DeleteOpenSession( SESSION_ID sessionId ){

	return m_openSessions.Remove( (CONTAINER_KEY)sessionId ) ? true : false;
}


//************************************************************************
// IsThisYourSession:
//
// PURPOSE:		Checks if the user is logged with the session id specified
//
// RETURN:		true:		yes
//************************************************************************

bool 
User::IsThisYourSession( U32 sessionId ){

	CONTAINER_ELEMENT		temp;

	return m_openSessions.Get( temp, (CONTAINER_KEY)sessionId )? true : false;
}


//************************************************************************
// ModifyObject:
//
// PURPOSE:		Modifes contents of the object
//
// NOTE:		Must be overridden by objects that can be modified
//************************************************************************

bool 
User::ModifyObject( ValueSet &objectValues, SsapiResponder *pResponder ){
	
	User *pUser = new User( 0 );
	*pUser = objectValues;
#if 0 
	This is not needed now!
	UnicodeString	tempString;
	if( pUser->GetString( SSAPI_USER_FID_PASSWORD,  &tempString ) )
		return pResponder->RespondToRequest( SSAPI_EXCEPTION_SECURITY,CTS_SSAPI_INVALID_PARM_EXCEPTION_INVALID_WAY_TO_CHANGE_PASSWORD );
#endif	
	bool rc = m_pUserManager->ModifyUser( pUser, pResponder );
	delete pUser;

	return rc; 
}


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

bool 
User::DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	return m_pUserManager->DeleteUser( this, pResponder );
}


//************************************************************************
// AreThereAnyTooLongStrings
//
// PURPOSE:		Checks every string's size against the maximum in the PTS 
//				row. Will raise an exception if something's wrong.
//				Checks strings in the value set, not the members. 
//
// RETURN:		false:	all strings are OK, may proceed
//				true:	an exception was rased, terminate normal execution
//************************************************************************

bool 
User::AreThereAnyTooLongStrings( SsapiResponder *pResponder ){

	UnicodeString				us;
	UserAccessTableEntry		*pRow = new UserAccessTableEntry;
	U32							exception = SSAPI_RC_SUCCESS;
	
	if( GetString( SSAPI_USER_FID_USERNAME, &us ) )
		if( us.GetSize() > sizeof( pRow->userName ) )
			exception = CTS_SSAPI_EXCEPTION_USERNAME_TOO_LONG;

	if( GetString( SSAPI_USER_FID_PASSWORD, &us ) )
		if( us.GetSize() > sizeof( pRow->password ) ) 
			exception = CTS_SSAPI_EXCEPTION_PASSWORD_TOO_LONG;

	if( GetString( SSAPI_USER_FID_FIRST_NAME, &us ) )
		if( us.GetSize() > sizeof( pRow->firstName ) )
			exception = CTS_SSAPI_EXCEPTION_FIRST_NAME_TOO_LONG;

	if( GetString( SSAPI_USER_FID_LAST_NAME, &us ) )
		if( us.GetSize() > sizeof( pRow->lastName ) )
			exception = CTS_SSAPI_EXCEPTION_LAST_NAME_TOO_LONG;

	if( GetString( SSAPI_USER_FID_DESCRIPTION, &us ) )
		if( us.GetSize() > sizeof( pRow->description ) )
			exception = CTS_SSAPI_EXCEPTION_USER_DESCRIPTION_TOO_LONG;

	if( GetString( SSAPI_USER_FID_EMAIL, &us ) )
		if( us.GetSize() > sizeof( pRow->email ) )
			exception = CTS_SSAPI_EXCEPTION_EMAIL_TOO_LONG;

	if( GetString( SSAPI_USER_FID_PHONE_1, &us ) )
		if( us.GetSize() > sizeof( pRow->phoneNumber1 ) )
			exception = CTS_SSAPI_EXCEPTION_PHONE1_TOO_LONG;

	if( GetString( SSAPI_USER_FID_PHONE_2, &us ) )
		if( us.GetSize() > sizeof( pRow->phoneNumber2 ) )
			exception = CTS_SSAPI_EXCEPTION_PHONE2_TOO_LONG;

	if( GetString( SSAPI_USER_FID_DEPARTMENT, &us ) )
		if( us.GetSize() > sizeof( pRow->department ) )
			exception = CTS_SSAPI_EXCEPTION_DEPARTMENT_TOO_LONG;
	
	delete pRow;

	if( exception == SSAPI_RC_SUCCESS )
		return false;

	pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, exception );
	return true;
}


