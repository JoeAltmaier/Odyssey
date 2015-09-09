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
// File: CtTimeGuard.cpp
// 
// Description:
//  This file contains the implementation of class CCtTimedService.
//
//  For more details about this class, refer to file CtTimeGuard.h,
//  or to the function headers below.
//
//  *  Note that there are a lot of potential race conditions between
//     different message-based "virtual threads", with respect to one
//     of our instances.  We largely don't worry about this, since we
//     assume that all of our members will be executed serially on the
//     queue-servicing thread of our parent DDM's class.
//     If this becomes untrue at some point in the future (e.g., due to
//     CHAOS changes), then this class's design must be reevaluated.
// 
// $Log: /Gemini/Odyssey/Util/CtTimeGuard.cpp $
// 
// 6     1/20/00 4:00p Eric_wedel
// Simplified implementation by removing deferred ops, and strengthened
// callback type checking.
// 
// 5     12/15/99 12:38a Ewedel
// Fixed potentially unpleasant "virtual recursion" bug.
// 
// 4     11/24/99 8:14p Jlane
// [ewx]  Disabled a couple of assert()s which are firing repeatedly.
// Pointing, I believe, to the need to do away with deferred op queuing.
// 
// 3     11/24/99 7:41p Ewedel
// Changed timer REFNUM to have its own little piece of memory, due to
// instance alignment problems (see comments at REFNUM).
// 
// 2     11/24/99 3:31p Ewedel
// Made real.
// 
/*************************************************************************/

#include  "CtTimeGuard.h"

#include  "CtEvent.h"

#include  "RqOsTimer.h"



//
//  CCtTimedService::CCtTimedService (pParentDdm)
//
//  Description:
//    Our constructor.  Passes through DdmServices info, and
//    otherwise initializes us to the idle state (no timer active).
//    
//  Inputs:
//    pParent - Our parent DDM.
//
//  Outputs:
//    none
//

CCtTimedService::CCtTimedService (DdmServices *pParentDdm) :
                                          DdmServices (pParentDdm)
{


   //  our timer's not running until we're told to start it:
   m_fRunning = FALSE;

   m_cusTimeout = 0;

   m_pfnTimeoutCallback = NULL;

   m_pvCallersTimeoutCookie = NULL;

   m_ulTimeoutCookie = 1;

   //  due to alignment bugs, we have to keep a little piece of memory for REFNUM!
   //  [This 8-byte value was correctly aligned within our instance, but our
   //   instance was *not* 8-byte aligned.  *sigh*]
   m_prefTimer = new REFNUM;
   assert (m_prefTimer != NULL);
   *m_prefTimer = 0;             // (is zero a safely invalid refnum?)

   return;

}  /* end of CCtTimedService::CCtTimedService */

//
//  CCtTimedService::~CCtTimedService ()
//
//  Description:
//    Our destructor.  Exists solely to tidy up our kludgey little
//    REFNUM allocation.
//    
//  Inputs:
//    none
//
//  Outputs:
//    none
//

CCtTimedService::~CCtTimedService ()
{

   delete m_prefTimer;
   m_prefTimer = NULL;

   return;

}  /* end of CCtTimedService::~CCtTimedService */

//
//  CCtTimedService::StartWatchdogTimer (cusTimeout,
//                                       pfnTimeoutCallback,
//                                       pfnTimerOpCallback,
//                                       pvTimeoutCookie)
//
//  Description:
//    Starts our timer.  If our timer is already running, we return
//    an error.  Otherwise, we do what we're told.
//
//    *** We assume that the caller-supplied callbacks are within a
//        class derived from our own; thus our instance pointer is
//        common with our caller.
//    
//  Inputs:
//    cusTimeout - One-based count of microseconds before we call
//                the caller's pfnTimeoutCallback routine.
//                A value of zero means ??
//    pfnTimeoutCallback - Caller-supplied routine which we call
//                if our timer expires before being stopped or reset.
//    pfnTimerOpCallback - Caller-supplied routine which we call
//                when our timer is stably started.
//    pvTimeoutCookie - A cookie which we pass through to the caller's
//                timeout routine, iff we call it.  Note that this is
//                not passed through to the op completion callback.
//
//  Outputs:
//    none
//

void  CCtTimedService::StartWatchdogTimer (U32 cusTimeout,
                                           WdTimeoutCallback pfnTimeoutCallback,
                                           WdTimerOpCallback pfnTimerOpCallback,
                                           void *pvTimeoutCookie /* = NULL */ )
{

RqOsTimerStart  * pmsgStart;


   assert ((pfnTimeoutCallback != NULL) && (pfnTimerOpCallback != NULL));
   if ((pfnTimeoutCallback == NULL) || (pfnTimerOpCallback == NULL))
      {
      //  illegal params, do nothing.
      return;
      }

   //  if our timer is already running, report an error
   if (m_fRunning)
      {
      assert (! m_fRunning);

      //  let caller's callback know that they blew it
      FinishOp (pfnTimerOpCallback, CTS_UTIL_WATCHDOG_ALREADY_RUNNING);
      }
   else
      {
      //  not running, so do the startup

      //  mark our timer as active before handing off to any callbacks
      m_fRunning = TRUE;

      //  save caller's timeout callback & timeout interval
      m_cusTimeout = cusTimeout;
      m_pfnTimeoutCallback = pfnTimeoutCallback;
      m_pvCallersTimeoutCookie = pvTimeoutCookie;

      //  define our currently valid timeout cookie:  for initial start,
      //  move it past any old stale values:
      m_ulTimeoutCookie ++;

      //  create our underlying timer.  Note that we use the timer in
      //  repeating mode (non one-shot), with the first interval being
      //  our timeout, and the second being a Really Big Value.  The
      //  The purpose of the second interval is to keep the timer
      //  intact indefinitely with a minimal number of extra expirations
      //  until we either reset or stop it.

      pmsgStart = new RqOsTimerStart (cusTimeout,
                                      -1,    // make second interval very long
                                      (void *) m_ulTimeoutCookie);

      //  save away start message's refnum, as this is our key to the
      //  new timer for its lifetime:
      *m_prefTimer = pmsgStart->refnum;

      //  send off message to create & start timer:
      Send (pmsgStart, REPLYCALLBACK(CCtTimedService, MyTimerCallback));

      //  since we don't get any timer-create ack message, simply
      //  call caller's callback ourself:
      FinishOp (pfnTimerOpCallback, CTS_SUCCESS);
      }

   return;

}  /* end of CCtTimedService::StartWatchdogTimer */

//
//  CCtTimedService::ResetWatchdogTimer (pfnTimerOpCallback,
//                                       pvTimeoutCookie)
//
//  Description:
//    Resets our timer.  From the time this routine is first entered,
//    we assure the caller that we will not call their timeout
//    callback until our originally-specified timeout has elapsed
//    all over again.
//
//    We require that our timer is either running or expired; we will
//    not start a stopped timer.
//    
//  Inputs:
//    pfnTimerOpCallback - Caller-supplied routine which we call
//                when our timer has been stably reset.
//    pvTimeoutCookie - A cookie which we pass through to the caller's
//                timeout routine, iff we call it.  Note that this is
//                not passed through to the op completion callback, but
//                only to the timeout notification callback (if called).
//                This value replaces that supplied to StartWatchdogTimer().
//
//  Outputs:
//    none
//

void  CCtTimedService::ResetWatchdogTimer (WdTimerOpCallback pfnTimerOpCallback,
                                           void *pvTimeoutCookie /* = NULL */ )
{

RqOsTimerReset  * pmsgReset;


   assert (pfnTimerOpCallback != NULL);
   if (pfnTimerOpCallback == NULL)
      {
      //  invalid parameter, do nothing.
      return;
      }

   //  if our timer isn't already running, report an error
   if (! m_fRunning)
      {
//* BUGBUG - this one is also firing too much.  We need to get rid of
//*          deferred op queuing -- it's screwing up timing.  ewx 11/24
      assert (m_fRunning);

      //  let caller's callback know that they blew it
      FinishOp (pfnTimerOpCallback, CTS_UTIL_WATCHDOG_NOT_RUNNING);
      }
   else
      {
      //  cool, do the reset.

      //  first, bump our cookie, to assure that we ignore any now-obsolete ticks
      m_ulTimeoutCookie ++;

      //  update our copy of caller's timeout cookie too
      m_pvCallersTimeoutCookie = pvTimeoutCookie;

      //  timeout messages which may be pending.
      pmsgReset = new RqOsTimerReset (*m_prefTimer,    // key of our timer
                                      m_cusTimeout,   // restore watchdog value
                                      -1,             // standard large 2nd
                                      (void *) m_ulTimeoutCookie);

      //  send message, and our part is finished
      Send (pmsgReset, REPLYCALLBACK(DdmOsServices, DiscardOkReply));

      //  fake it, and assume that operation is successful
      FinishOp (pfnTimerOpCallback, CTS_SUCCESS);
      }

   return;

}  /* end of CCtTimedService::ResetWatchdogTimer */

//
//  CCtTimedService::StopWatchdogTimer (pfnTimerOpCallback)
//
//  Description:
//    Stops our timer.  From the time this routine is first entered,
//    we assure the caller that we will not call their timeout
//    callback unless they call StartWatchdogTimer() again.
//
//    Of course, we require that our timer already be running.  :-)
//    
//  Inputs:
//    pfnTimerOpCallback - Caller-supplied routine which we call
//                when our timer has been stably reset.
//
//  Outputs:
//    none
//

void  CCtTimedService::StopWatchdogTimer (WdTimerOpCallback pfnTimerOpCallback)
{

RqOsTimerStop      * pmsgStop;
WdTimerOpCallback  * pOpCallback;


   assert (pfnTimerOpCallback != NULL);
   if (pfnTimerOpCallback == NULL)
      {
      //  invalid parameter, do nothing.
      return;
      }

   //  if our timer isn't already running, report an error
   if (! m_fRunning)
      {
//*BUGBUG - for now, just patch this out.  It's firing a lot.
//*         --ewx 11/24
      assert (m_fRunning);

      //  let caller's callback know that they blew it
      FinishOp (pfnTimerOpCallback, CTS_UTIL_WATCHDOG_NOT_RUNNING);
      }
   else
      {
      //  cool, do the stop

      //  bump our timer callback cookie, to invalidate any potential
      //  pending timeout notifies
      m_ulTimeoutCookie ++;

      //  send stop notification to our timer.
      pmsgStop = new RqOsTimerStop (*m_prefTimer);

      //  create a dummy carrier for callback pointer, since it won't
      //  directly fit in a (void *):
      pOpCallback = new WdTimerOpCallback (pfnTimerOpCallback);

      //  send stop and have it carry along callback info, since we might
      //  (somehow!) have multiple stop requests outstanding at once.
      Send (pmsgStop, pOpCallback,
            REPLYCALLBACK(CCtTimedService, StopWatchdogTimer2));

      //  since we bumped our cookie, we can legitimately reset our instance.
      //  Of course, it's not safe to destroy us until after
      //  StopWatchdogTimer2() has said so, by calling the callback.
      m_fRunning = FALSE;
      m_pfnTimeoutCallback = NULL;
      }

   return;

}  /* end of CCtTimedService::StopWatchdogTimer */


//  CCtTimedService::StopWatchdogTimer2
//
//    Called back from DdmTimer when it has finished processing
//    our timer stop message.

STATUS  CCtTimedService::StopWatchdogTimer2 (Message *pReply)
{

WdTimerOpCallback  * pOpCallback;


   assert (pReply->Status() == CTS_SUCCESS);

   //  per the design of DdmTimer::ProcessTimerStop(), we should always
   //  receive the final tick message prior to receiving the reply to
   //  our stop request.  Since we're processing our stop request reply,
   //  we can now do final instance cleanup.

   //  recover callback pointer
   pOpCallback = (WdTimerOpCallback *) pReply->GetContext();
   assert ((pOpCallback != NULL) && (*pOpCallback != NULL));

   //  let client know that stop is complete.  (And so we're now safe
   //  to destroy.)
   FinishOp (*pOpCallback, CTS_SUCCESS);

   //NOTE:  we can't touch our instance data after this point, since
   //       the callback might have destroyed us.

   //  tidy up after message & callback data
   delete pOpCallback;
   delete pReply;

   return (CTS_SUCCESS);      // what reply handlers always say

}  /* end of CCtTimedService::StopWatchdogTimer2 */

//
//  CCtTimedService::MyTimerCallback (pReply)
//
//  Description:
//    Reply handler for tick messages sent from our timer.
//
//    These messages are by definition asynchronous with respect to
//    any operation which might be being performed on our timer.
//    
//  Inputs:
//    pReply - Reply to our original "start timer" message.  This reply
//                only contains a status of CTS_SUCCESS when it is the
//                final tick from our timer (i.e., once we have sent
//                a stop request to the timer).
//
//  Outputs:
//    CCtTimedService::MyTimerCallback - Returns CTS_SUCCESS, always.
//

STATUS  CCtTimedService::MyTimerCallback (Message *pReply)
{

RqOsTimerStart  * pmsgStart = (RqOsTimerStart *) pReply;
U32               ulTimerCookie;


   //  what we really care about is the timeout cookie.
   ulTimerCookie = (U32) pmsgStart->pCookie;

   //BUGBUG - should we also gate on (pReply->Status() != CTS_SUCCESS) here?
   if (ulTimerCookie == m_ulTimeoutCookie)
      {
      //  we have a winner!  This tick message appears to represent
      //  a valid timeout.  So, we pass it on along to our client.
      (((DdmServices *) this)->*m_pfnTimeoutCallback)
                                    (m_pvCallersTimeoutCookie);

      //  note that we do *not* change the value of m_ulTimeoutCookie,
      //  as this may interfere with current/pending watchdog operations.
      //  But we are careful to always specify a very large "2nd tick
      //  interval" when communicating with DdmTimer.  So we should never
      //  see a second tick, unless we have sent a Reset request.
      }

   //  always dispose of tick reply
   delete pReply;

   //  and say what reply handlers are always supposed to say:
   return (CTS_SUCCESS);

}  /* end of CCtTimedService::MyTimerCallback */

//
//  CCtTimedService::FinishOp (pfnTimerOpCallback, sStatus)
//
//  Description:
//    Called when an operation has completed.  This routine
//    is responsible for notifying the client of the completion.
//
//    This is the standard routine which should be used by
//    all operation code to signal completion to the client,
//    whether successful or otherwise.
//
//  Inputs:
//    pfnTimerOpCallback - Client's callback, which we call to
//                   notify client of completion.
//    sStatus - Final result status of operation.  Passed on
//                   to client's callback.
//
//  Outputs:
//    none
//

void  CCtTimedService::FinishOp (WdTimerOpCallback pfnTimerOpCallback,
                                 STATUS sStatus)
{


   //  first off, invoke client's callback.
   (((DdmServices *) this)->*pfnTimerOpCallback) (sStatus);

   //  that's it.  Things are rather simpler since we disposed of
   //  deferred-op queuing.  :-)

   //  all done.
   return;

}  /* end of CCtTimedService::FinishOp */

