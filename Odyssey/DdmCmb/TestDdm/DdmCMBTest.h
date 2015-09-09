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
// File: DdmCMBTest.h
// 
// Description:
//    Defines a test DDM for exercising the CMB DDM.
// 
// $Log: /Gemini/Odyssey/DdmCmb/TestDdm/DdmCMBTest.h $
// 
// 10    9/03/99 5:36p Ewedel
// Added PHS interface exercise support.
// 
// 9     7/16/99 7:23p Ewedel
// A few tweaks for doing a listen on the IOP Status table.
// 
// 8     6/24/99 7:18p Ewedel
// Cleaned out old cruft, updated to latest CmdQueues interface.
// 
// 7     6/16/99 8:19p Ewedel
// Added support for dynamically determining which IOP slot to play
// power-on games with (it picks the first which has initial state
// "powered down").
// 
// 6     6/15/99 7:09p Ewedel
// Updated for latest command queue interface.
// 
// 5     6/15/99 12:28a Ewedel
// Updated to current queue interface level.
// 
// 4     6/15/99 12:02a Ewedel
// Added command queue interface exercise stuff.
// 
// 3     5/12/99 4:06p Ewedel
// Removed obsolete member.
// 
// 2     3/24/99 1:47p Ewedel
// Added test of MsgCmbIopControl message.
// 
// 1     3/19/99 7:37p Ewedel
// Initial checkin.
//
/*************************************************************************/

#ifndef _DdmCMBTest_h_
#define _DdmCMBTest_h_

#ifndef __Ddm_h
# include "Ddm.h"
#endif

#ifndef __Table_h__
# include  "Table.h"
#endif

#ifndef _IOPStatusRecord_h
# include  "IOPStatusTable.h"
#endif

#ifndef __CMD_SENDER_H__
# include  "CmdSender.h"
#endif

#ifndef _CmbDdmCommands_h_
# include  "CmbDdmCommands.h"
#endif

#ifndef __RqDdmReporter_h
# include  "RqDdmReporter.h"
#endif


class CDdmCMBTest: public Ddm
{
public:

   //  hook to let CHAOS call our constructor -- this is standard form
   static Ddm *Ctor(DID did);

   //  our constructor -- standard form as needed by Ctor
   CDdmCMBTest(DID did);

   //  what CHAOS calls when our DDM is first activated, and after each unquiesce
   virtual STATUS  Enable (Message *pMsg);

private:
   
   //  a helper for doing our initial "poll" call to the CMB DDM
   STATUS  WakeUpCmbDdm (void);

   void    WakeUpCmbDdm1 (STATUS ulStatus);

   void    WakeUpCmbDdm1a (void *pvCookie, STATUS sStatus);

   STATUS  WakeUpCmbDdm2 (MessageReply *pReply);

   //  sends "command queue" request to power up an IOP
   STATUS  WakeUpCmbDdm3 (void *pClientContext, STATUS ulResult);

   // conforms to (pCmdCompletionCallback_t):
   void  WakeUpCmbDdm4 (STATUS ulStatus, void *pResultData,
                        CmbCtlRequest *pCmdData, void *pvCookie);

   //  where we route PHS_START request messages to:
   STATUS  ReqPhsStart (RqDdmReporter *pReqMsg);

   //  where replies to our PHS_CURRENT_STATUS request messages go
   STATUS  ShowCmbPhsReply (Message *pReply);

   //  buffer of stuff allocated (?) by TSReadTable et al
   IOPStatusRecord * m_pIopRowsRead;

   //  count of rows read by TSReadTable
   U32               m_cIopRowsRead;

   //  pointer to our outstanding listen on IOP Status state field changes
   TSListen        * m_pStateListen;

   //  pointer to "listen reply type" field returned at listen callback time
   U32             * m_pStateListenType;

   //  pointer to IOP Status row returned by listen
   IOPStatusRecord * m_pStateUpdRow;

   //  count of bytes in *m_pStateUpdRow, as returned by listen
   U32               m_cbStateUpdRow;


   //  helper for sending control requests to the CMB DDM
   CmdSender         m_CmbDdmIntf;

   //  the magic slot we pick to do power / boot things to:
   TySlot            m_eFirstPoweredDownSlot;

   //  DID of the CMB DDM, as reported to us via PHS_START
   DID               m_didCmbDdm;

};  /* end of class CDdmCMBTest */


#endif   // #ifndef _DdmCMBTest_h_

