//************************************************************************
// FILE:		UserAccessTable.h
//
// PURPOSE:		Defines a row contents for the User Access Table
//************************************************************************

#ifndef __USER_DESCRIPTOR_TABLE_H__
#define	__USER_DESCRIPTOR_TABLE_H__



#include "CtTypes.h"
#include "Odyssey.h"
#include "PTSCommon.h"

#pragma	pack(4)

#define USER_ACCESS_TABLE_NAME		"UserAccessTable"
#define	USER_ACCESS_TABLE_VERSION	1

extern	U32	sizeOfUserAccessTableEntry;
extern	fieldDef	userAccessTableDefintion[];

struct UserAccessTableEntry {
	rowID					rid;				// rowID of this table row.
	U32 					version;			// Version of User Access Table record.
	U32						size;				// Size of User Access Table record in bytes.
	UnicodeString16			userName;			// username used for login 
	UnicodeString16			password;			// user's password
	UnicodeString32			firstName;			// user's first name
	UnicodeString32			lastName;			// user's last name
	UnicodeString32			description;		// arbitrary
	UnicodeString32			email;				// user's emal
	UnicodeString32			phoneNumber1;		// user's first phone number 
	UnicodeString32			phoneNumber2;		// user's second phone number
	UnicodeString32			department;			// the department ?!
	U32						language;			// LocalizedString -> thru message compiler
	U32						securityPolicy;		// TBD
	U32						numberOfInvalidLogins;
};	

#define	ftUATE_NUMBER_OF_WRONG_LOGINS	"numberOfInvalidLogins"

#endif	// __USER_DESCRIPTOR_TABLE_H__