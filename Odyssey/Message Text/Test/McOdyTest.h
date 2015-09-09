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
// File: McOdyTest.h
// 
// Description:
//    Defines a test DDM for exercising the message compiler emitted
//    language database and rendering code in a MIPS (Eval) environment.
// 
// $Log: /Gemini/Odyssey/Message Text/Test/McOdyTest.h $
// 
// 1     7/12/99 6:10p Ewedel
// Initial revision.
//
/*************************************************************************/

#ifndef _McOdyTest_h_
#define _McOdyTest_h_

#ifndef __Ddm_h
# include "Ddm.h"
#endif


class CMcOdyTest: public Ddm
{
public:

   //  hook to let CHAOS call our constructor -- this is standard form
   static Ddm *Ctor(DID did);

   //  our constructor -- standard form as needed by Ctor() above
   CMcOdyTest(DID did);

   //  what CHAOS calls when our DDM is first activated, and after each unquiesce
   virtual STATUS  Enable (Message *pMsg);

private:

   //  helper which runs our test routines
   void  DoTests (void);

};  /* end of class CMcOdyTest */


#endif   // #ifndef _McOdyTest_h_

