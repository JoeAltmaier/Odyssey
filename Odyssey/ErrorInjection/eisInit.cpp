/*
 * EisInit.cpp - Error Injection initialization
 *
 *  This file is responsible for building the mappings between string names
 *  of error triggers and the numeric IDs.  It also contains functions for 
 *  finding an ID based on a name, and vice-versa.
 *
 * Copyright (C) ConvergeNet Technologies, 1999
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Revision History:
 *     4/7/1999 Bob Butler: Created
 *
*/

#include "EISTriggers.h"


#define _BuildTriggerIDTable

#include "EISTriggers.h"
#include <string.h>

BOOL FindTriggerID(char *_pName, U32 *_ID)
{
	for (U32 i = 0; aEISClasses[i].iClCode != EIS_ERROR_CLASS_LAST; ++i)
		if (FindTriggerID(i, _pName, _ID))
			return true;
	return false;
}

BOOL FindTriggerID(U32 _classID, char *_pName, U32 *_ID)
{
	for (U32 i = 0; aEISClasses[_classID].paCodes[i].sEISCode; ++i)
		if (strcmp(_pName, aEISClasses[_classID].paCodes[i].sEISCode) == 0)
		{
			*_ID = aEISClasses[_classID].paCodes[i].iIDCode;
			return true;
		}
	return false;
}

char *FindTriggerName(U32 _ID)
{
	return NULL;
}

char *FindTriggerClassName(U32 _classID)
{
	return NULL;
}
