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
// File: DdmHotSwap.h
// 
// Description:
//    Hot Swap Master DDM definitions.
//
// 
// $Log: /Gemini/Odyssey/DdmHotSwap/DdmHotSwap.h $
// 
// 1     10/11/99 7:51p Ewedel
// First cut, really just a shell now.
// 
/*************************************************************************/

#ifndef _DdmHotSwap_h_
#define _DdmHotSwap_h_

#include "Ddm.h"


#ifndef __QUEUE_OPERATIONS_H__
# include  "CmdServer.h"      // control interface to our DDM
#endif



class CDdmHotSwap : public Ddm
{

public:

   //  our constructor, preserves our DID for posterity
   CDdmHotSwap (DID did);

   //  how to make an instance of us (used deep in the bowels of CHAOS)
   static Ddm *Ctor (DID did);

   //  initialize, called the first time we are loaded (faulted-in, or whatever)
   virtual STATUS Initialize (Message *pMsg);

   //  called when our DDM instance is supposed to quiesce
   virtual STATUS Quiesce (Message *pMsg);

   //  called after we initialize, and each time we are unquiesced
   virtual STATUS Enable (Message *pMsg);


private:

   //  here's what other DDMs use to talk to us (mostly..)
   CmdServer   m_CmdServer;


   //  the routines used to carry on Initialize() processing:
   void  Initialize2 (STATUS sStatus);


   //  our extended message processing handlers




   //  members in CmbDdmCmdProc.cpp

   //  routine for handling incoming command requests (via class CmdServer)
   void  CmdReady (HANDLE hRequest, void *pvRequest);

   //  process an IOP power control command
//   void  ControlIopPower (CDdmCmdCarrier *pCmd);


//   //  common response callback used by most of our Control*() members (above)
//   void  ReportControlResult (CDdmCmdCarrier *pCmd, STATUS ulStatus,
//                              const CmbPacket *pReply);


//   //  helper for parsing Odyssey-internal FC drive IDs
//   STATUS  ParseDriveBayId (U32 ulRawDriveBay, U32& iDdhBay,
//                            TySlot& eDdhSlot);


};  /* end of class CDdmHotSwap */


#endif   // #ifndef _DdmHotSwap_h_

