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
// File: HotSwapTest.h
// 
// Description:
//    Defines a test DDM for exercising the Hot Swap Master DDM.
// 
// $Log: /Gemini/Odyssey/DdmHotSwap/TestDdm/HotSwapTest.h $
// 
// 1     10/11/99 7:53p Ewedel
// First cut.
//
/*************************************************************************/

#ifndef _HotSwapTest_h_
#define _HotSwapTest_h_

#ifndef __Ddm_h
# include "Ddm.h"
#endif

#ifndef __CMD_SENDER_H__
# include  "CmdSender.h"
#endif

#ifndef _DdmHotSwapCommands_h_
# include  "DdmHotSwapCommands.h"
#endif


class CHotSwapTest: public Ddm
{
public:

   //  hook to let CHAOS call our constructor -- this is standard form
   static Ddm *Ctor(DID did);

   //  our constructor -- standard form as needed by Ctor
   CHotSwapTest(DID did);

   //  what CHAOS calls when our DDM is first activated, and after each unquiesce
   virtual STATUS  Enable (Message *pMsg);

private:
   
   //  we don't actually *do* anything yet, we're more of a link test case.
   CmdSender   m_HotSwapQueueIntf;

   //  helper for actually exercising the Hot Swap Master (eventually)
   void  TestHotSwap (void);

   void  TestHotSwap2 (STATUS sStatus);


};  /* end of class CHotSwapTest */


#endif   // #ifndef _HotSwapTest_h_

