/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: CtTimeGuard.h
// 
// Description:
//  This file contains the definition of class CCtTimedService.
//  CCtTimedService is a base class, and is itself derived
//  from DdmServices.
//
//  The idea behind CCtTimedService is that some operation, requested
//  of a DDM, may be encapsulated by that DDM in a DdmServices-based
//  helper class.  By basing the DDM's helper on CCtTimedService instead
//  of directly on DdmServices, the helper gains simple access to timer
//  services, oriented towards maintaining watchdog-style timeouts on
//  arbitrary operations.
//
//  In its present incarnation, CCtTimedService supports only a single
//  logical timer.  This timer may be started, stopped, and reset (its
//  watchdog countdown interval reset).
//
//  We have some interesting requirements, detailed more fully in the
//  function headers in our companion .cpp file.  Briefly, we guarantee
//  that once a caller has requested that we either shut off or reset
//  a timer, we will never send them a timeout message for that timer
//  (until they set a new timer and/or the timer expires).
//
//  Since we are based on a message-interfaced timer service, our
//  control methods themselves respond via callbacks.  The implication
//  of the prior paragraph is that, once the caller has transferred
//  control to one of our members, our guarantee is in effect.
//
//  However, the caller in turn must guarantee that they will not
//  destroy our instance until we have called their callback routine
//  [they may destroy our instance within that callback, however].
//  The caller further must guarantee that they will *not* destroy
//  our instance while our timer is active.  That is, they may only
//  destroy our instance from within 1) their timeout callback, or
//  2) some state in which our timer is known to not be enabled (e.g.,
//  the callback from our stop operation).
// 
// $Log: /Gemini/Include/CtTimeGuard.h $
// 
// 5     1/20/00 4:00p Eric_wedel
// Whoops, removed obsolete internal member func.
// 
// 4     1/20/00 3:49p Eric_wedel
// Simplified implementation by removing deferred ops, and strengthened
// callback type checking.
// 
// 3     11/24/99 7:42p Ewedel
// Changed timer REFNUM to have its own little piece of memory, due to
// instance alignment problems (see comments at REFNUM).
// 
// 2     11/24/99 3:30p Ewedel
// Made real.
// 
/*************************************************************************/

#ifndef _CtTimeGuard_h_
#define _CtTimeGuard_h_


#ifndef __DdmOsServices_h
# include  "DdmOsServices.h"
#endif


class CCtTimedService : public DdmServices
{
public:

   //  our only constructor, simply passes through DdmServices info
   CCtTimedService (DdmServices *pParentDdm);

   //  our destructor, just to tidy up our kludge memory
   ~CCtTimedService ();
   
   //  define the callback used when we detect a timeout
   //  (watchdog timer expiration).
   //  Use macro WDTIMEOUTCALLBACK() to cast a member to this type.
   //  *** keep in sync with CtWdTimeoutCallbackCast<>() below ***
   typedef void (DdmServices::*WdTimeoutCallback) (void *pvTimeoutCookie);

   //  define the callback used to report completion of a timer operation.
   //  Use macro WDTIMEROPCALLBACK() to cast a member to this type.
   //  *** keep in sync with CtWdTimerOpCallbackCast<>() below ***
   typedef void (DdmServices::*WdTimerOpCallback) (STATUS sResult);

   //  start a watchdog timer running
   void  StartWatchdogTimer (U32 cusTimeout,
                             WdTimeoutCallback pfnTimeoutCallback,
                             WdTimerOpCallback pfnTimerOpCallback,
                             void *pvTimeoutCookie = NULL);

   //  restart watchdog timer's timeout interval
   void  ResetWatchdogTimer (WdTimerOpCallback pfnTimerOpCallback,
                             void *pvTimeoutCookie = NULL);

   //  stop a running watchdog timer (once this routine has run,
   //  we guarantee that no timeout callback will happen, unless
   //  a new timer is started)
   void  StopWatchdogTimer (WdTimerOpCallback pfnTimerOpCallback);

   //  is our timer presently running?
   inline BOOL  WatchdogIsRunning (void)  const
            {  return (m_fRunning);  };

private:

   //  is our timer presently running (started, but not expired)
   BOOL     m_fRunning;

   //  one-based count of microsends to allow before watchdog expiration
   U32      m_cusTimeout;

   //  what to call if our watchdog timer expires
   WdTimeoutCallback m_pfnTimeoutCallback;

   //  caller-supplied cookie which we pass through to *m_pfnTimeoutCallback:
   void   * m_pvCallersTimeoutCookie;

   //  "cookie" used to track whether a given timer reply is live or stale
   U32      m_ulTimeoutCookie;

   //  ref num of timer's start message, which is timer's "key":
   //  due to alignment bugs, we have to keep a little piece of memory for REFNUM!
   //  [This 8-byte value was correctly aligned within our instance, but our
   //   instance was *not* 8-byte aligned.  *sigh*]
   REFNUM * m_prefTimer;

   //  continuation routine for StopWatchdogTimer()
   STATUS  StopWatchdogTimer2 (Message *pReply);

   //  what we give to DdmTimer as a callback point.  This handles
   //  timer expirations & kills.  It is a ReplyCallback.
   STATUS  MyTimerCallback (Message *pReply);


   //  Called by an operation once it has completed.  This routine
   //  notifies the client of op completion (it used to do more,
   //  but we simplified away deferred ops).
   void  FinishOp (WdTimerOpCallback pfnTimerOpCallback, STATUS sStatus);

};  /* end of class CCtTimedService */



//  here are some callback type checkers / helpers:

//  We introduce a template function which can be used to verify the
//  signature of member function parameters passed to our
//  WDTIMEOUTCALLBACK macro.  The type-checking is done by passing
//  the caller's raw member function pointer to this routine; if it
//  can accept it, then the member's signature is ok.
//  The key is that we require the signature of this function's
//  pfn parameter to be exactly the same as WdTimeoutCallback, except
//  for substitution of the template argument as the class qualifier.
template<class CCaller> inline CCtTimedService::WdTimeoutCallback  CtWdTimeoutCallbackCast
            (void (CCaller::*TimeoutCallback) (void *pvTimeoutCookie))
      {  return ((CCtTimedService::WdTimeoutCallback) TimeoutCallback);  };

#ifdef WIN32
#define WDTIMEOUTCALLBACK(clas,method)	 \
         (CtWdTimeoutCallbackCast<clas> (method))
#elif defined(__ghs__)  // Green Hills
#define WDTIMEOUTCALLBACK(clas,method)	 \
         (CtWdTimeoutCallbackCast<clas> (&clas::method))
#else	// MetroWerks
#define WDTIMEOUTCALLBACK(clas,method)	 \
         (CtWdTimeoutCallbackCast<clas> (&method))
#endif


//  watchdog timer operation callback cast helper
template<class CCaller> inline CCtTimedService::WdTimerOpCallback  CtWdTimerOpCallbackCast
            (void (CCaller::*TimerOpCallback) (STATUS sResult))
      {  return ((CCtTimedService::WdTimerOpCallback) TimerOpCallback);  };

//  watchdog timer operation callback parameter (portable)
#ifdef WIN32
#define WDTIMEROPCALLBACK(clas,method)	 \
         (CtWdTimerOpCallbackCast<clas> (method))
#elif defined(__ghs__)  // Green Hills
#define WDTIMEROPCALLBACK(clas,method)	 \
         (CtWdTimerOpCallbackCast<clas> (&clas::method))
#else	// MetroWerks
#define WDTIMEROPCALLBACK(clas,method)	 \
         (CtWdTimerOpCallbackCast<clas> (&method))
#endif


#endif  // #ifndef _CtTimeGuard_h_

