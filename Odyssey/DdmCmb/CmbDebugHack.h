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
// File: CmbDebugHack.h
// 
// Description:
//    Some dummy debug-only code for tracking strange CMB buffer loss
//    which both Lee & Joe seem to be seeing.
// 
// $Log: /Gemini/Odyssey/DdmCmb/CmbDebugHack.h $
// 
// 3     2/15/00 11:28a Eric_wedel
// Added new debug class CCmbDebugHwIntfLog for tracking low-level AVR
// interface traffic.
// 
// 2     2/08/00 7:06p Eric_wedel
// Added yet more debug info.
// 
// 1     1/20/00 4:36p Eric_wedel
// Initial revision.
// 
/*************************************************************************/

#ifndef _CmbDebugHack_h_
#define _CmbDebugHack_h_


#ifndef Simple_H
# include  "Simple.h"
#endif

#ifndef __Kernel_h
# include  "Kernel.h"      // for Kernel::Time_Stamp()
#endif

#ifndef _Critical_h
# include  "Critical.h"
#endif

#include  <string.h>

#ifndef assert
# include  <assert.h>
#endif



//  simple forward ref for dirty param passage
class CCmbMsgSender;    // (defined in CmbMsgSendWrapper.h)


class CCmbDebugInfo {
public:

   //  count of ReqPollEnvironment() "threads" outstanding
   S32   cEnvPollsActive;

   //  count of CCmbMsgSender wrapper instances outstanding
   S32   cMsgSendWrappersExtant;

   //  count of CSendContext hardware intf packets in use
   S32   cCSendContextsInUse;

   //  count of CUnsolicitedPkt rx packets in use
   S32   cCUnsolicitedPktsInUse;

   //  count of cmbhwintf watchdog timeout callbacks
   U32   cCmbHwIntfMatchedTimeoutCallbacks;     // "live" timeouts
   U32   cCmbHwIntfUnmatchedTimeoutCallbacks;   // "stale" timeouts

   //  count of tick-induced send timeouts
   U32   cCmbHwIntfTickInducedTimeouts;

   //  time of last received tick
   I64   ulTimeLastTickReceived;

   //  count of total 1sec timer tick messages received
   I64   cTotalTicksReceived;

};  /* end of class CCmbDebugInfo */


//  our one instance of this struct.  Instance is found in CmbHwIntf.cpp.
extern CCmbDebugInfo CmbDebugInfo;


//  here's a whole 'nother class for tracking low-level AVR interface details.
//  This roughly replaces the "Sent byte =..." console messages and their
//  receive counterparts with a ring buffer that doesn't impact performance
//  nearly as much as all that console output would.  The ring can be dumped
//  to the console, using messages equivalent to those used by the raw
//  TRACEF()s, but with timestamps added.  The dump is triggered by a call
//  placed at some interesting point in the code.
class CCmbDebugHwIntfLog
{
public:

   CCmbDebugHwIntfLog (void) : m_cRingEntries (512)
      {
         m_pRing = new CRingEntry [m_cRingEntries];
         assert (m_pRing != NULL);
         m_iNextWrite = 0;
         memset (m_pRing, 0, sizeof (CRingEntry) * m_cRingEntries);
      }

   //  record a single byte about to be written to the AVR hardware
   inline void  RecordByteTx (U8 bAvrTxNibble)
      {
         Critical section;    // we might be called @ ISR time
         m_pRing[m_iNextWrite].Set (bAvrTxNibble, TRUE);
         BumpRingPtr (m_iNextWrite);
      };

   //  record a single byte just read from the AVR interface
   inline void  RecordByteRx (U8 bAvrRxNibble)
      {
         Critical section;    // we might be called @ ISR time
         m_pRing[m_iNextWrite].Set (bAvrRxNibble, FALSE);
         BumpRingPtr (m_iNextWrite);
      };

   //  dump contents of our ring to the console.  Does *not* clear ring.
   void  DumpRing (void);

private:

   //  helper for incrementing our next-write ring index
   inline void  BumpRingPtr (int& iRingPtr)
      {
     
         iRingPtr ++;
     
         if (iRingPtr >= m_cRingEntries)
            {
            iRingPtr = 0;
            }
     
      };

   //  here's what one entry in our ring looks like:
   class CRingEntry
   {
   public:
      TIMESTAMP   i64TimeStamp;     // when entry was written
      U8          bNibble;          // AVR data nibble (& ctl flags)
      U8          fIsTx;            // TRUE if bNibble is TX value, FALSE if RX
      U8          abReserved[6];    // keep entry size rounded to i64 multiple

      //  helper for setting a value
      inline void  Set (U8 bAvrNibble, U8 fIsTx)
         {
            bNibble = bAvrNibble;
            this->fIsTx = fIsTx;
            i64TimeStamp = Kernel::Time_Stamp();
         };

   };  /* end of class CRingEntry */

   //  since we can't guarantee our instance data alignment,
   //  we keep a dynamically allocated ring buffer
   CRingEntry   * m_pRing;

   //  count of entries our ring has room for
   const int   m_cRingEntries;

   //  zero-based index of next entry to be written
   int         m_iNextWrite;

};  /* end of class CCmbDebugHwIntfLog */


//  there is only one instance, also placed in CmbHwIntf.cpp
extern CCmbDebugHwIntfLog   CmbDebugHwIntfLog;


#endif  // #ifndef _CmbDebugHack_h_

