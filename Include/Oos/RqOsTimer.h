/* RqOsTimer.h -- Request Interface to DdmTimer.
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99
 * Copyright (C) Dell Computer, 2000
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
//  8/17/98 Joe Altmaier: Create file
//  ** Log at end-of-file **

// 100 columns
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#ifndef __RqOsTimer_h
#define __RqOsTimer_h

#include "Ddm.h"
#include "RequestCodes.h"
#include "Message.h"

// This is the payload structure for the REQ_OS_TIMER_START message.
// NOTE: The time units are microseconds.
// time0: is the time until the timer replies.  
// timeN: is the time for all subsequent replies.
// For a one-shot timer timeN should be 0.
class RqOsTimerStart : public Message {
public:
	enum { RequestCode = REQ_OS_TIMER_START };

	I64	time0;		// The time until the first reply
	I64	timeN;		// The time for all subsequent replies. 0 for one shot.
	void *pCookie;	// Cookie returned by timer until RqOsTimerReset sent
	
	RqOsTimerStart(I64 _time0, I64 _timeN, void *_pCookie=NULL) : Message(RequestCode) {
		time0 = _time0; timeN = _timeN; 
		pCookie = _pCookie;
	}
};
	
// This is the payload structure for the REQ_OS_TIMER_RESET message.
// NOTE: The time units are microseconds.
// pTimerMsg is the address of the message used to start the timer.
//           it is used to identify the timer to reset.
// time0: is the time until the timer replies.  
// timeN: is the time for all subsequent replies.
// For a one-shot timer timeN should be 0.
class RqOsTimerReset : public Message {
public:
	enum { RequestCode = REQ_OS_TIMER_RESET };

	REFNUM refNum;		// Reference Number of RqOsTimerStart
	U32		time0;		// The time until the first reply
	U32		timeN;		// The time for all subsequent replies. 0 for one shot.
	void *pCookie;	    // replaces RqOsTimerStart cookie in *future* replies

	RqOsTimerReset(const RqOsTimerStart *_pMsg, I64 _time0,I64 _timeN, void *_pCookie=NULL) : Message(RequestCode) {
		refNum = _pMsg->refnum; time0 = _time0; timeN = _timeN;
		pCookie = _pCookie;
	}
	
	RqOsTimerReset(REFNUM _refNum, I64 _time0,I64 _timeN, void *_pCookie=NULL) : Message(RequestCode) {
		refNum = _refNum; time0 = _time0; timeN = _timeN;
		pCookie = _pCookie;
	}
};

// This is the payload structure for the REQ_OS_TIMER_STOP message.
// pTimerMsg is the address of the message used to start the timer.
//           it is used to identify the timer to stop.
class RqOsTimerStop : public Message {
public:
	enum { RequestCode = REQ_OS_TIMER_STOP };

	REFNUM refNum;		// Unique Timer Reference Number
	void *pCookie;	    // sets cookie seen in final "last" timer reply


	RqOsTimerStop(const RqOsTimerStart *_pMsg, void *_pCookie=NULL) : Message(RequestCode) {
		refNum = _pMsg->refnum;
		pCookie = _pCookie;
	}
	
	RqOsTimerStop(REFNUM _refNum, void *_pCookie=NULL) : Message(RequestCode) {
		refNum = _refNum;
		pCookie = _pCookie;
	}
};

#endif	// __RqOsTimer_h

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Include/Oos/RqOsTimer.h $
// 
// 9     2/06/00 2:52p Tnelson
// Update comments
// 
// 8     1/24/00 11:04a Jlane
// VD Delete support changes.  Checked in by JFL for TN.
// 
// 7     11/10/99 11:20a Ewedel
// Added tick reply cookie control param to stop message.
// 
// 6     11/09/99 5:38p Ewedel
// Rolled TRN's changes into SJ vss.  [Adds a timer-specific cookie value,
// and message constructors which accept direct REFNUM values instead of
// messages.]
// 
//  6/27/99 Tom Nelson:	  Added RequestCode enums
//  3/27/99 Tom Nelson:   Support derived timer request classes
//  3/18/99 Tom Nelson:   Changed to use Timer.h and SERVE macros
//  3/04/99 Tom Nelson:	  Change NU_Create_Timer to Kernel::Create_Timer
//  8/17/98 Joe Altmaier: Create file
