/*
 * ErrorTrigger.cpp - Error Injection Trigger Class
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
 *     4/6/1999 Bob Butler: Moved from ErrorInjectionService.cpp
 *
*/
#include "ErrorTrigger.h"
#include "EISTriggers.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int atoi(char *);

/* Create an ErrorTrigger object from a script line having the general form:
	<phase>,<slot>,<Unique ID>,<parameter>,<iteration>
	1,2,SOME_UNIQUE_ID,3,4
*/

// Static
ErrorTrigger *ErrorTrigger::CreateTrigger(char *_pScriptLine)
{
	BOOL _bErr = true;
	U32 _aArgs[5];
	ErrorTrigger *_pET = NULL;
	
	
	if (_pScriptLine && strlen(_pScriptLine))
	{
		// strtok is destructive, so make a copy.
		char *_pTemp = new char[strlen(_pScriptLine) + 1];
		strcpy(_pTemp, _pScriptLine);
		char *_pTok = strtok(_pTemp, ", ");
		_bErr = false;
		for (U16 i = 0; !_bErr && i < sizeof(_aArgs); ++i)
		{
			_bErr = (!_pTok || !isdigit(*_pTok));  // is the token numeric?
			
			if (!_bErr)
			{
				 _aArgs[i]= (U32)atoi(_pTok);
			}
			else if (i == 2 && _pTok)
				_bErr = !FindTriggerID(_pTok, &(_aArgs[i]));
			if (!_bErr)
				_pTok = strtok(NULL, ", ");
		}
		_bErr = _pTok != NULL;	
		if (!_bErr)
			_pET =  new ErrorTrigger(_aArgs[0], _aArgs[1], _aArgs[2], _aArgs[3], _aArgs[4]);
		delete _pTemp;
	}		
	return _pET;
}

ErrorTrigger::ErrorTrigger(const ErrorTrigger &et)
{
	operator=(et);
}

ErrorTrigger &ErrorTrigger::operator =(const ErrorTrigger &et)
{
	if (this != &et)
	{
		phase = et.phase;
		slot = et.slot;
		ID = et.ID;
		iteration = et.iteration;
		parameter = et.parameter;
		triggerState = et.triggerState;
	}
	return *this;
}



ErrorTrigger::~ErrorTrigger()
{
}


BOOL ErrorTrigger::Trigger(U32 _ID, U32 _parameter)
{ 
	// see if we're armed and the right trigger. 
	if ( IsArmed() && ID == _ID && parameter == _parameter)
		{
		  	// This is the right trigger, so increment the iteration count 
		  	++triggerIteration;
		  	if (iteration == triggerIteration) // if this is the right iteration, trigger it.
			{
				SetTriggered(); 
				return true;
			}
		}
	return false;
}
