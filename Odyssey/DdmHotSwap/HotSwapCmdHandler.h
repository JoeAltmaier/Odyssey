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
// File: HotSwapCmdHandler.h
// 
// Description:
//    Defines a "smart cookie" class for handling drive Hot Swap
//    command processing.  One instance is created for each disk
//    drive command received by the Hot Swap DDM.
//
// 
// $Log: /Gemini/Odyssey/DdmHotSwap/HotSwapCmdHandler.h $
// 
// 2     12/22/99 6:37p Ewedel
// Added host DDM instance ptr param to our constructor, as this must be
// passed through to our DdmServices base class.
// 
// 1     12/16/99 1:13a Ewedel
// Initial revision.
// 
/*************************************************************************/

#ifndef _HotSwapCmdHandler_h_
#define _HotSwapCmdHandler_h_

#ifndef __DdmOsServices_h
# include  "DdmOsServices.h"
#endif

#ifndef __QUEUE_OPERATIONS_H__
# include  "CmdServer.h"      // control interface to our DDM
#endif

#ifndef _DdmHotSwapCommands_h_
# include  "DdmHotSwapCommands.h"   // the request struct we receive
#endif

#ifndef _DiskDescriptor_h
# include  "DiskDescriptor.h"
#endif

#ifndef _DdmCMBMsgs_h
# include  "DdmCmbMsgs.h"     // what we use for DDH control messages
#endif


class CHotSwapCmdHandler : public DdmServices
{
public:

   //  our only constructor
   CHotSwapCmdHandler (DdmServices *pHostDdm, const CHotSwapIntf& req,
                       HANDLE hReq, CmdServer& HotSwapCmdQueue);

private:

   //  we keep a reference to the request we're processing
   const CHotSwapIntf&  m_req;

   //  and the command queue handle of the request:
   HANDLE   m_hReq;

   //  and the Hot Swap DDM's command queue interface critter:
   CmdServer&  m_HotSwapCmdQueue;

   //  we also keep our disk descriptor query reply around for a while
   //  (we'll delete it in our destructor)
   DiskDescriptor::RqReadRow   * m_pDiskDescrReply;

   //  actual record, inside DiskDescrReply:
   DiskDescriptor              * m_pDiskDescr;

   //  our destructor - tidies up loose ends, as usual
   ~CHotSwapCmdHandler ()
         {  delete m_pDiskDescrReply;
            m_pDiskDescrReply = NULL;  };

   //  handle a disk class of operation request
   STATUS  HandleDiskCommand (Message *pmsgRawReply);

   //  handle a disk release operation
   STATUS  HandleDiskRelease (Message *pmsgRawReply);

   //  handle a disk restore operation
   STATUS  HandleDiskRestore (Message *pmsgRawReply);

   //  report results back to requestor & clean up
   STATUS  ReportResults (Message *pmsgReply);

   //  a helper for reporting error status to requestor
   void  ReportErrorAndSelfDestruct (STATUS sRet);

   //  a little helper for setting up DDH control messages
   void  SetDdhDriveId (MsgCmbDdhControl * pDdhCtl)  const;

};  /* end of class CHotSwapCmdHandler */


#endif   // #ifndef _DdmHotSwap_h_

