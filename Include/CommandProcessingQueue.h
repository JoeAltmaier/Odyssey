/* CommandProcessingQueue.h
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
**/

// Revision History:
// $Log: /Gemini/Include/CommandProcessingQueue.h $
// 
// 1     9/12/99 10:34p Joehler
// Replaced AlarmQueue.h
// 

#ifndef __CommandProcessingQueue_H
#define __CommandProcessingQueue_H

#include "CTTypes.h"

class FunctionCallElement {
public:
	FunctionCallElement(U32 function, void* context) : 
	  functionToCall(function), pContext(context), pNext(NULL) {}
 
	FunctionCallElement* GetNext() { return pNext; }
	void SetNext(FunctionCallElement* next) { pNext = next; }

	U32 GetFunction() { return functionToCall; }
	void* GetContext() { return pContext; }

private:
	U32 functionToCall;
	void* pContext;
	FunctionCallElement* pNext;
};

class CommandProcessingQueue
{
public:
	CommandProcessingQueue() : pHead(NULL) {}

	BOOL Empty() { return pHead == NULL; }

	void AddFunctionCall(U32 functionToCall, void* pContext)
	{
		FunctionCallElement* pElement = new FunctionCallElement(
			functionToCall,
			pContext);
		FunctionCallElement* position;

		position = GetHead();
		if (position)
		{
			while (position->GetNext())
				position = position->GetNext();
			position->SetNext(pElement);
		}
		else
			SetHead(pElement);
	}
	
	FunctionCallElement* PopFunctionCall()
	{
		FunctionCallElement* pElement;

		pElement = GetHead();
		if (pElement)
		{
			SetHead(pElement->GetNext());
			return pElement;
		}
		else
			return NULL;
	}

private:
	FunctionCallElement* pHead;
	FunctionCallElement* GetHead() { return pHead; }
	void SetHead(FunctionCallElement* head) { pHead = head; }

};

#endif	// __CommandProcessingQueue_H