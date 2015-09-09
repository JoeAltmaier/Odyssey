/* Listener_T -- Template for building Listener Lists
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
//  6/29/99 Tom Nelson: Create file


#ifndef __Listener_T_h
#define __Listener_T_h

#include "DdmOsServices.h"

template <class MSG>
class Listener_T {
	Listener_T<MSG> **ppFirst;
	Listener_T<MSG> *pNext;
	Listener_T<MSG> *pPrev;
	BOOL fLinked;
	MSG *pMsg;
	
	Listener_T<MSG>(Listener_T<MSG> &);	// Not supported
public:
	
	Listener_T<MSG>(Listener_T<MSG> **_ppFirst,MSG *_pMsg) : pMsg(_pMsg),ppFirst(_ppFirst),fLinked(FALSE) {
		Link();
	}
	~Listener_T<MSG>() {
		Unlink();
	}
	// Link new listener to list.
	// Does not check for duplicates.  DON'T ADD THEM!
	void Link() {
		if (!fLinked) {
			pNext = *ppFirst;
			pPrev = NULL;
			*ppFirst = this;
			fLinked = TRUE;
		}
	}
	void Unlink() {
		if (fLinked) {
			if (*ppFirst == this)
				*ppFirst = pNext;
			else
				pPrev->pNext = pNext;
			
			if (pNext != NULL)
				pNext->pPrev = pPrev;
			
			fLinked = FALSE;
		}
	}
	Listener_T<MSG> *GetFirst() { return *ppFirst;  }
	Listener_T<MSG> *GetNext()	{ return pNext; 	}
	MSG			    *GetMsg()	{ return pMsg;  	}
	REFNUM		     GetRefNum(){ return refNum;	}
};

// Defines a Listener for a derived message type
//
// NOTE:  All templates for the same message class use 
//		  the same list!
//
template <class MSG>
class ListenerList_T {
public:
	typedef MSG ListenerMessage;
	typedef Listener_T<MSG> Listener;

private:	
	DdmServices *pDdm;
	Listener_T<MSG> *pFirst;

public:
	ListenerList_T<MSG>(DdmServices *_pDdm) : pDdm(_pDdm),pFirst(NULL) {
	}

	Listener *GetFirst() { 
		return pFirst;
	}
	Listener *Add(MSG *_pMsg) {
		return new Listener(&pFirst,_pMsg);
	}

	// Find Listener by Message RefNum
	Listener *FindListner(Message *pMsg) {
		for (Listener *pListener = GetFirst(); pListener != NULL; pListener->GetNext()) 
			if (pMsg->refNum  == pListener->pMsg->refNum)
				return pListener;
			
		return NULL;	// Not found
	}		
	// Find Message matching Message RefNum
	Message *FindMsg(Message *pMsg) {	
		Listener *pListener = FindListener(pMsg);
		
		return pListener ? pListener->pMsg : NULL;
	}
	// Delete Message matching Message RefNum
	MSG * RemoveMsg(Message *_pMsg) {
		// Search for refNum and delete it.  Does not check for duplicates!
		Listener *pListener = FindListener(_pMsg);
		if (pListener) {
			MSG *pMsg = pListener->GetMsg();
			pListener->Unlink();
			delete pListener;
			return pMsg;
		}
		return NULL;	// Not found
	}
	// Notify All Listeners
	void Notify(MSG *pReplyData,ERC erc=OK) {
		// Notify listeners 
		for (Listener *pListen = GetFirst(); pListen != NULL; pListen=pListen->GetNext()) {
			MSG *pMsg = pListen->GetMsg();
			pMsg->MakeReply(pReplyData);
			pDdm->Reply(pMsg,erc,FALSE);
		}
	}
	// Notify Last to all Listener and remove all from list.
	// ..used when shuting down service
	ERC NotifyLast(MSG *pReplyData,ERC erc=OK) {
		// Notify listeners 
		for (Listener *pListen = GetFirst(); pListen != NULL; pListen=pListen->GetNext()) {
			MSG *pMsg = pListen->GetMsg();
			pMsg->MakeReply(pReplyData);
			pDdm->Reply(pMsg,erc,TRUE);
			pListen->Unlink();
		}
		return OK;
	}
	// Notify Last to single Listener and remove from list.
	// ..used when listener stops listening.
	ERC NotifyLast(MSG *_pMsg,MSG *pReplyData,ERC erc=OK) {
		MSG *pMsg = RemoveMsg(_pMsg);
		if (pMsg) {
			pMsg->MakeReply(pReplyData);
			pDdm->Reply(pMsg,erc,TRUE);
		}
		return OK;
	}
};

#endif 	// __Listener_T_h