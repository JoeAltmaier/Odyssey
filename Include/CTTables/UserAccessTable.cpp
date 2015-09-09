//************************************************************************
// FILE:		UserAccessTable.cpp
//
// PURPOSE:		Declares a var describing field defintions in a 
//				UserAccessTable entry.
//************************************************************************

#include "OsTypes.h"
#include "PTSCommon.h"
#include "UserAccessTable.h"

	fieldDef	userAccessTableDefintion[] = {

//		"rowID",									8,	ROWID_FT,		Persistant_PT,
		"version",									4,	U32_FT,			Persistant_PT,
		"size",										4,	U32_FT,			Persistant_PT,
		"userName",									32,	USTRING16_FT,	Persistant_PT,
		"password",									32, USTRING16_FT,	Persistant_PT,
		"firstName",								64,	USTRING32_FT,	Persistant_PT,
		"lastName",									64,	USTRING32_FT,	Persistant_PT,
		"description",								64,	USTRING32_FT,	Persistant_PT,							
		"email",									64,	USTRING32_FT,	Persistant_PT,
		"phoneNumber1",								64,	USTRING32_FT,	Persistant_PT,
		"phoneNumber2",								64,	USTRING32_FT,	Persistant_PT,
		"department",								64,	USTRING32_FT,	Persistant_PT,
		"language",									4,	U32_FT,			Persistant_PT,
		"securityPolicy",							4,	BINARY_FT,		Persistant_PT,
		ftUATE_NUMBER_OF_WRONG_LOGINS,				4,	U32_FT,			Persistant_PT
	};

U32	sizeOfUserAccessTableEntry = sizeof(userAccessTableDefintion);