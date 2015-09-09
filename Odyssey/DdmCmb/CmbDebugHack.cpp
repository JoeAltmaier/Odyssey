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
// File: CmbDebugHack.cpp
//
// Description:
//    Helper code, strictly for debug use.  This code is checked in because
//    it's being used to chase after the sorts of bugs that take hours or
//    days to reproduce, in trees built by QA.  I.e., it's easier this way.  :-)
//
// $Log: /Gemini/Odyssey/DdmCmb/CmbDebugHack.cpp $
// 
// 1     2/15/00 11:36a Eric_wedel
// More debug tracking, nominally for DFCT12059.
// 
/*************************************************************************/

#include  "CmbDebugHack.h"


#include  <assert.h>

#include  "Odyssey_Trace.h"



//
//  CCmbDebugHwIntfLog::DumpRing ()
//
//  Description:
//    Dumps all entries presently in our ring buffer to the console.
//
//    We are careful to emulate the text message form used in prior
//    TRACEF() calls made from CmbIsr.cpp.  This maintains compatibility
//    with the trace digest tool, CmbPkt.exe.
//    But we also add a timestamp to the end of each line.  These values
//    should not break CmbPkt, but may prove very useful in figuring out
//    what is really going on here.
//
//  Inputs:
//    none
//
//  Outputs:
//    none
//

void  CCmbDebugHwIntfLog::DumpRing ()
{

int         iNextRead;
int         cEntriesDumped;
I64         i64Last;          // time of last entry, for delta display


   //  grab a local copy of ring's high-water mark, in case
   //  somebody tries to move it while we're dumping.
   //  [This also happens to be the oldest entry in the ring,
   //   which is where we want to start our dump.]
   iNextRead = m_iNextWrite;

   cEntriesDumped = 0;
   i64Last = 0;

   while (cEntriesDumped < m_cRingEntries)
      {
      if (m_pRing[iNextRead].fIsTx)
         {
         //  do TX nibble message
         Tracef ("Sent byte = 0x%02X \t\t%hu\n", 
                 m_pRing[iNextRead].bNibble,
                 m_pRing[iNextRead].i64TimeStamp - i64Last);
         }
      else
         {
         //  do RX nibble message
         Tracef ("CMB_ISR_DataReady: data = 0x%02X \t%hu\n",
                 m_pRing[iNextRead].bNibble,
                 m_pRing[iNextRead].i64TimeStamp - i64Last);
         }

      //  update timestamp basis for next nibble's delta computation
      i64Last = m_pRing[iNextRead].i64TimeStamp;

      //  advance to next nibble in ring
      BumpRingPtr (iNextRead);
      cEntriesDumped ++;
      }

   return;

}  /* end of CCmbDebugHwIntfLog::DumpRing */

