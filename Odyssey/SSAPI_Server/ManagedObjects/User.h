//************************************************************************
// FILE:		User.h
//
// PURPOSE:		Declares class User which is a managed object and contains
//				data atributes of an O2K user
//************************************************************************

#ifndef __USER_MO_H__
#define __USER_MO_H__

#include "ManagedObject.h"
#include "..\SSAPI_Codes.h"
#include "UnicodeString.h"
#include "SList.h"
#include "Listener.h"

struct UserAccessTableEntry;
class ListenManager;
class UserManager;

#ifdef WIN32
#pragma pack(4)
#endif



class User : public ManagedObject {
	
	SList				m_openSessions;	// contains open sessions for this user

	UnicodeString		m_userName;
	UnicodeString		m_password;
	UnicodeString		m_firstName;
	UnicodeString		m_lastName;
	UnicodeString		m_description;
	UnicodeString		m_email;
	UnicodeString		m_phoneNumber1;
	UnicodeString		m_phoneNumber2;
	UnicodeString		m_department;
	U32					m_language;
	U32					m_securityPolicy;
	U32					m_numberOfInvalidLogins;

	
public:
	UserManager			*m_pUserManager;	// not owned here, do not delete

//************************************************************************
// User:
//
// PURPOSE:		A default constructor for the class
//************************************************************************

User( ListenManager *pListenManager );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new User( GetListenManager() ); }


//************************************************************************
// ~User:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~User();


//************************************************************************
// BuildYourselfFromPtsRow
// 
// PURPOSE:		Populates itself off a PTS row
//************************************************************************

bool BuildYourselfFromPtsRow( UserAccessTableEntry *pRow );



//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		Populates its value set based on the data members
//************************************************************************

virtual bool BuildYourValueSet();


//************************************************************************
// Data Member Accessors
//
// RETURN:	copies of corresponding data members
//************************************************************************

UnicodeString	GetUserName() const		{ return m_userName; }
UnicodeString	GetPassword() const		{ return m_password; }
U32				GetNumberOfWrongLogins() const { return m_numberOfInvalidLogins; }
void			SetNumberOfWrongLogins( U32 num ) { m_numberOfInvalidLogins = num; }


void			SetUserName( UnicodeString &name ) { m_userName = name; }
void			SetPassword( UnicodeString &pass ) { m_password = pass; }

//************************************************************************
// WriteYourselfIntoPtsRow:
//
// PURPOSE:		Populates a PTS row with the information contained 
//				in its data members. The PTS row must be ready to be 
//				written after this call has returned.
//************************************************************************

bool WriteYourselfIntoPtsRow( UserAccessTableEntry *pRow ); 


//************************************************************************
// BuildYourselfFromYourValueSet:
//
// PURPOSE:		Fetches data from its ValueSet into its data members
//************************************************************************

bool BuildYourselfFromYourValueSet();


//************************************************************************
// AddOpenSession:
//
// PURPOSE:		Adds a new open session id into open session container
//************************************************************************

bool AddOpenSession( SESSION_ID sessionId );


//************************************************************************
// DeleteOpenSession:
//
// PURPOSE:		Removes session id from open session container
//************************************************************************

bool DeleteOpenSession( SESSION_ID sessionId );


//************************************************************************
// Assignment operator overloaded
//************************************************************************

const ValueSet& operator=(const ValueSet& obj ){ *(ValueSet *)this = obj; return obj; }


//************************************************************************
// AreThereAnyTooLongStrings
//
// PURPOSE:		Checks every string's size against the maximum in the PTS 
//				row. Will raise an exception if something's wrong.
//				Checks strings in the value set, not the members. 
//
// RETURN:		true:	all strings are OK, may proceed
//				false:	an exception was rased, terminate normal execution
//************************************************************************

bool AreThereAnyTooLongStrings( SsapiResponder *pResponder );


//************************************************************************
// IsThisYourSession:
//
// PURPOSE:		Checks if the user is logged with the session id specified
//
// RETURN:		true:		yes
//************************************************************************

bool IsThisYourSession( SESSION_ID sessionId );



protected:

//************************************************************************
// ModifyObject:
//
// PURPOSE:		Modifes contents of the object
//
// NOTE:		Must be overridden by objects that can be modified
//************************************************************************

virtual bool ModifyObject( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

virtual bool DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder );


private:





};

#endif // _USER_MO_H__