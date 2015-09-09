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
// File: DdmCMB.h
// 
// Description:
//    Card Management Bus interface DDM definitions.
// 
// $Log: /Gemini/Odyssey/DdmCmb/DdmCMB.h $
// 
// 29    1/20/00 4:29p Eric_wedel
// Added ReqEvcPowerControl(), a little debug info, and fixed several
// callback target signatures.
// 
// 28    12/13/99 7:26p Jlane
// [ewx]  Fixed battery fuse drops cache to be signed (it was unsigned,
// oops).
// 
// 27    12/13/99 2:26p Ewedel
// Added env param cache for asynchronously updated values (keyswitch and
// battery fuse drops).
// 
// 26    11/19/99 6:36p Ewedel
// Added cache of current % fan speed settings.
// 
// 25    11/17/99 11:41a Ewedel
// Removed last vestiges of PHS support.
// 
// 24    11/16/99 6:25p Ewedel
// Cleaned up CEnvPollContext for PHS->CMB env poll message support.
// 
// 23    11/16/99 6:12p Ewedel
// Added DDH control command, and changed env reporting from PHS message
// to special CMB message.
// 
// 22    10/27/99 12:17p Ewedel
// Updated for EVC and IOP control message changes.
// 
// 21    10/08/99 12:03p Ewedel
// Added ReqSetMipsState(), and a tweak to ReportOsRunningToCma().
// 
// 18    9/03/99 5:30p Ewedel
// Lots of tweaks to reflect real EVC param reading/update, and PHS
// Reporter support.
// 
// 17    8/24/99 8:13p Ewedel
// Removed EVC Status query support (request obsolete).  Substantial
// revision of initialization to support failover, and operation in slave
// mode on IOPs and non-master HBCs.  Misc other cleanup too.
// 
// 16    8/11/99 7:48p Ewedel
// Updated for new members (better IOP Status checking, etc.).
// 
// 15    8/02/99 7:30p Ewedel
// Added environmental polling handlers.
// 
// 14    7/16/99 10:13a Ewedel
// Added flag to track when slot status polling is enabled.
// 
// 13    7/15/99 4:20p Ewedel
// Various updates for file rearrangement, context carrier cleanup and
// unsolicited input support.
// 
// 12    7/08/99 5:10p Ewedel
// Removed Initialize0a(), which was a patch to overcome temporary
// problems with "system start" DDM access to PTS.
// 
// 11    6/30/99 5:02p Ewedel
// Added new member Initialize0a() to handle PTS wakeup call (since we're
// system entry).
// 
// 10    6/18/99 1:13p Ewedel
// Added ControlIopPower3(), and made SendCmbMsg() param count optional
// (it defaults to zero).
// 
// 9     6/16/99 8:25p Ewedel
// Added support for "get board info" and "set boot params".
// 
// 8     6/15/99 7:08p Ewedel
// Updated for latest command queue interface.
// 
// 7     6/15/99 12:31a Ewedel
// Updated to current queue interface level.
// 
// 6     6/14/99 11:59p Ewedel
// Various updates, cleanup, support added for listen mode and command
// queue interface.
// 
// 5     6/03/99 7:16p Ewedel
// Moved CT_IOPST_MAX_IOP_SLOTS here, added Initialize5() for first
// exercise of CMB Hw Intf class.
// 
// 4     5/12/99 4:08p Ewedel
// Beginnings of hardware interface support.  (Intermediate checkin due to
// trace.h -> Odyssey_Trace.h change.)
// 
// 3     4/07/99 6:29p Ewedel
// Added helper routine MakeSlotMessage() for displaying our temp kludge
// slot action prompts in a more intelligible way.
// 
// 2     4/05/99 5:38p Ewedel
// Changed m_aeContigToSlot to use standard slot count definition.
// 
// 1     3/19/99 7:37p Ewedel
// Initial checkin.
// 
/*************************************************************************/

#ifndef _DdmCMB_h_
#define _DdmCMB_h_

#include "Ddm.h"

#ifndef _IOPStatusRecord_h
# include  "IOPStatusTable.h"
#endif

#ifndef _EVCStatusRecord_h
# include  "EVCStatusRecord.h"
#endif

#ifndef __EnvStatusReport_h
# include  "EnvStatusReport.h"
#endif

#ifndef _EnvControlTable_h
# include  "EnvControlTable.h"
#endif

#ifndef __Table_h__
# include  "Table.h"       // PTS table defs
#endif

#ifndef __TSRows_h__
# include  "Rows.h"        // PTS row defs
#endif

#ifndef _ReadTable_h_
# include  "ReadTable.h"   // more PTS table defs
#endif

#ifndef _Listen_h_
# include  "Listen.h"      // PTS listen interface
#endif

#ifndef CTtypes_H
# include  "CtTypes.h"     // for rowID, etc.
#endif

#ifndef _CmbHwIntf_h_
# include  "CmbHwIntf.h"   // CMB hardware interface class
#endif

#ifndef _CmbMsgSendWrapper_h_
# include  "CmbMsgSendWrapper.h"
#endif

#ifndef _CmbEventRecv_h_
# include  "CmbEventRecv.h"   // for event log entry reassembly
#endif

#ifndef __QUEUE_OPERATIONS_H__
# include  "CmdServer.h"      // control interface to our DDM
#endif

#ifndef _DdmCMBMsgs_h
# include  "DdmCmbMsgs.h"     // for calling IOP poll support code, etc.
#endif

#ifndef _CmbDdmCommands_h_
# include  "CmbDdmCommands.h"    // our custom control interface stuff
#endif

#ifndef _Event_h_
# include  "Event.h"          // event log entry object class
#endif


//  just for fun, we also define the maximum slots which we support.
//  This def might (eventually?) better belong somewhere closer to the
//  IOP slot enumerator.
#define  CT_IOPST_MAX_IOP_SLOTS  (24)



class CDdmCMB : public Ddm
{

class SlotContext;      // forward ref for friendliness

friend SlotContext;

public:

   //  our constructor, preserves our DID for posterity
   CDdmCMB (DID did);

   //  how to make an instance of us (used deep in the bowels of CHAOS)
   static Ddm *Ctor (DID did);

   //  initialize, called the first time we are loaded (faulted-in, or whatever)
   virtual STATUS Initialize (Message *pMsg);

   //  called when our DDM instance is supposed to quiesce
   virtual STATUS Quiesce (Message *pMsg);

   //  called after we initialize, and each time we are unquiesced
   virtual STATUS Enable (Message *pMsg);

   //  send an event log entry off to the current master HBC, so it can be logged
   static STATUS  ForwardEventLogEntry (Event *pEventEntry);

   //  these values are used by some of our helper classes, so we
   //  break with convention and make them public:

   //  a little map for going from contiguous zero-based slot index
   //  to our actual TySlot IOP slot enum values
   static const TySlot  m_aeContigToSlot [CT_IOPST_MAX_IOP_SLOTS];

   //  just so all our members don't need to know how many contig slots we do:
   static const U32     m_cContigSlots;

   //  a little kludge so that our helper routine EvcQHFanSpeeds() in
   //  CtlPollEnvironment.cpp can access our fan speed setting cache.
   //  [Maybe this means helper routines should be static members?]
   static U32  FanSpeedSet (CmbFanSpeedSetFanSelect eSetFanPair)
         {  return (m_pLocalCmbDdm->m_aulFanSpeedSet [eSetFanPair]);  };

   //  another little kludge, so that our helper routine EvcQHKeyPos()
   //  in CtlPollEnvironment.cpp can update(!) our env params cache
   static void  SetKeyPos (TySlot eEvcId, CT_EVC_KEYPOS eKeyPos)
      {
      m_pLocalCmbDdm->m_EvcEnvCache[eEvcId - CMB_EVC0].eKeyPosition = eKeyPos;
      };

   //  yet another kludge, for helper routine EvcQHBattFuseDrops() in
   //  CtlPollEnvironment.cpp, which needs to update our env params cache
   static void  SetFuseDrops (TySlot eEvcId, const CmbPacket& pkt)
      {
      m_pLocalCmbDdm->m_EvcEnvCache[eEvcId - CMB_EVC0].SetFuseDrops(pkt);
      };


private:

   //  here's what other DDMs use to talk to us (sometimes..)
   CmdServer   m_CmdServer;

   //  here's what we use to talk to the local board's CMA (Atmel part)
   CCmbHwIntf  m_CmbHwIntf;

   //  we use this on the HBC to reassemble incoming event log fragments
   CCmbEventRecv  m_EventAssembler;

   //  do we want to listen to unsolicited input from the local CMA?
   BOOL        m_fAcceptUnsolicitedCmbInput;

   //  here's the slot number of our own IOP board
   TySlot      m_eMyIopSlot;

   //  a flag saying whether we're HBC master or not
   BOOL        m_WeAreMasterHbc;

   //  and a flag indicating whether we've been asked to do a slot poll yet.
   //  Before the first slot poll, we drop slot status updates on the floor,
   //  so they don't disturb the prior state of PTS.
   //  (Valid only in HBC master instance of our DDM.)
   BOOL        m_fSlotPollBegun;


   //  we keep some local copies of IOP Status table data, for both
   //  access and listen purposes.  This table is indexed by ContigSlot.
   typedef struct {
      rowID    rid;        // row ID for current slot

      IopState eState;     // IOP's current state (as we last wrote it to PTS)
//      IopDesiredState eIOPDesiredState;   // IOP's desired / target state
      U32             TempHiThreshold;    // Threshold value for above to turn fans up.
      U32             TempNormThreshold;  // Threshold value for above to restore fans.
      String16        ChassisSerialNumber;// Board's last known chassis serial #.
   }  IopStatusImage;

   IopStatusImage m_aIopStatusImage [CT_IOPST_MAX_IOP_SLOTS];

   //  for convenience, we keep static copies of the contiguous indices
   //  for the two EVCs, so we don't have to translate them in order to
   //  check current state.
   //  (Note:  these should be const, but ValidIopSlot() doesn't work then.)

   U32         m_aiContigEvc[2];

   //  here's some stuff for tracking the Env Control table

   //  RowId of table's only (we hope) record
   RowId       m_ridEnvCtl;

   //  cached copies of the EVC temperature settings (we should probably
   //  keep the whole EnvControl record, but these will do for now):

   U32         m_aulFanSpeedSet[2];

   //  and here are where we keep track of our outstanding PTS listen requests

   //  and some data provided to our IOP Status table listen callbacks
   IOPStatusRecord * m_pUpdatedIopRow;    // ptr to changed row
   U32               m_cbUpdatedIopRow;   // byte size of changed row


   //  here's a little struct for tracking IOP slots during our initial
   //  IOP Status table construction phase:
   class IopInitCtx : private DdmServices
   {
   public:
      U32      iContigSlot;   // current slot we're working on (contig index)
      TySlot   eSlot;         // CMB slot we're working on
      Message *pInitMsg;      // Initialize message which started it all

      //  stuff for PTS reads, etc.
      U32      cRowsRead;     // how many rows read by read-row

      //  dummy rid used to catch data returned for EVC row inserts:
      rowID    ridDummy;

      union {
         IOPStatusRecord         aIopRec[2];    // two, to catch duplicate recs
         CtEVCRawParameterRecord aEvcRaw[2];    // ditto
      } u;

      CtEnvControlRecord   EnvCtlRec;     // (duplicate check works different way)

      //  our constructor
      IopInitCtx (Message *pInitMsg)
      {
         iContigSlot = 0;
         eSlot = IOP_HBC0;       // (has iContigSlot = 0)
         this->pInitMsg = pInitMsg;
      };
   };  /* end of class IopInitCtx */


   //  here's a little struct for tracking IOP slot info across phases
   //  of request message handling
   class SlotContext
   {
   public:
      TySlot   eSlot;         // IOP slot we're working on
      U32      iContigSlot;   // eSlot, in contiguously-indexed form
      Message *pReq;          // request message which we're processing
      IOPStatusRecord * pIopRow; // data comprising new/modified IOP row
      rowID    ridNewRow;     // fed back to us by PTS modify operator
      U32      cRowsRead;     // when reading table, PTS reports rows read here

      SlotContext (void)  {  pReq = NULL;  pIopRow = NULL;  };

      SlotContext (U32 iContigSlot, IOPStatusRecord * pIopRow)
      {
         eSlot = CDdmCMB::m_aeContigToSlot [iContigSlot];
         this->iContigSlot = iContigSlot;
         pReq = NULL;
         this->pIopRow = pIopRow;
         cRowsRead = 0;
      };
   };  /* end of class SlotContext */

   //  here's a similar struct, but for tracking Env Control updates
   typedef struct {
      STATUS               sResult; // our reportable result
      MsgCmbEvcControl   * pReq;    // request message which we're processing
      CtEnvControlRecord   EnvRec;
   } EnvCtlContext;

   //  here's a context for tracking multi-EVC updates
   class CEvcSendContext {
   public:

      CEvcSendContext (CmbHwCmdCode eCommand, U8 cParams,
                       U8 bParam1, U8 bParam2,
                       CCmbMsgSender::CmbCallback pCmdCallback,
                       void *pvCookie) :
                              m_eCommand (eCommand),
                              m_cParams (cParams),
                              m_bParam1 (bParam1),
                              m_bParam2 (bParam2),
                              m_pCmdCallback (pCmdCallback),
                              m_pvCookie (pvCookie)
      {  m_sEvcResult = CTS_SUCCESS;  };

      //  caller's command data
      CmbHwCmdCode   m_eCommand;
      U8             m_cParams;
      U8             m_bParam1;
      U8             m_bParam2;

      //  caller's callback address
      CCmbMsgSender::CmbCallback m_pCmdCallback;

      //  caller's cookie
      void   * m_pvCookie;

      //  result of send to first EVC
      STATUS      m_sEvcResult;

      CmbPacket   m_pktEvcResult;
      
   };  /* end of class CEvcSendContext */

   //  here's yet another context, this one for tracking IOP polling
   class PollContext
   {
   public:
      Message *pReq;          // original poll request which we're processing
      U32      cRowsPolled;   // how many rows polled yet

      PollContext (void)
            {  pReq = NULL;  cRowsPolled = 0;  };
   };  /* end of class PollContext */

   //  a context for tracking a simple IOP State update PTS request
   class ModifyStateContext
   {
   public:
      IopState eState;
      rowID    ridNewRow;     // fed back to us by PTS modifiy operator

      ModifyStateContext (IopState eNewState)
               {  eState = eNewState;  };
   };  /* end of class ModifyStateContext */


   //  a little helper struct for carting around current request info
   //  as a cookie

   class CDdmCmdCarrier
   {
   public:
      U32            m_iContigSlot;    // request's target IOP slot
      HANDLE         m_hRequest;       // handle of req we're processing
      CmbCtlRequest  m_Req;            // our local copy of request

      //  helper for when we're updating IOP's PTS state
      IopState       m_eIopState;

      CDdmCmdCarrier (U32 iContigSlot, HANDLE hRequest,
                      const CmbCtlRequest& Src)
      {  m_iContigSlot = iContigSlot;
         m_hRequest = hRequest;
         m_Req = Src;
      };

   };  /* end of CDdmCmdCarrier */


   typedef void (* pfnEvcQueryHandler) (CtEVCRawParameterRecord& EvcRec,
                                        const CmbPacket *pPkt);

   //  table entry type for driving environmental polling of EVC(s)
   class CEvcQueryEntry
   {
   public:
//      BOOL  fEachEvc
      TySlot               eTarget;          // which EVC do we ask?
      CmbHwParamType       eInfoSubCode;     // type of info we're asking for
      U32                  cbResponse;       // how big response should be
      pfnEvcQueryHandler   pfnHandler;       // response handler routine
                                             //  (NULL signals end of query list)
   };  /* end of CEvcQueryEntry */

   //  (table itself is statically declared in PollEnvironment.cpp, at present)

   //  a helper struct for carting around environment poll context
   //  as a cookie

   class CEnvPollContext
   {
   public:
      MsgCmbPollEnvironment * m_pReqMsg;
      MsgCmbPollEnvironment::CPayload m_EnvStatus; // where we accrue status info
      CEvcQueryEntry  * m_pCurEvcQuery;   // where we are in EVC query seq.
      rowID             m_ridNewRow;      // scratch for PTS row modify
      U32               m_iContigSlot;    // tracks IOPs as we read temperatures

//BUGBUG - debug hack
      U32   m_iRingMyReq;    // debug ring index for our poll request
      I64   m_i64ReqStartTime;   // Time() at start of request proc
//BUGBUG - endit

      CEnvPollContext (MsgCmbPollEnvironment *pEnvPollRequest)
      {  m_pReqMsg = pEnvPollRequest;
         m_EnvStatus.EVCRawParameters[0].EvcSlotId = CMB_EVC0;
         m_EnvStatus.EVCRawParameters[1].EvcSlotId = CMB_EVC1;
         m_pCurEvcQuery = NULL;
         m_iContigSlot = -1;     // set up for pre-increment use

      };

      //  a little helper for accessing our current CMB slot struct
      inline MsgCmbPollEnvironment::CPayload::CMB_SLOT_RECORD& CurSlot (void)
      {
         return (m_EnvStatus.CmbSlotInfo[
                       CDdmCMB::m_aeContigToSlot [m_iContigSlot]]);
      };

   };  /* end of CEnvPollContext */

   //  here is a little cache of those environmental poll items which
   //  can have value changes announced by the EVC.  We use this set
   //  of cached values as the master for all notifications / replies.
   //  Thus, if a value change is announced after it's already been
   //  read by an env poll "thread", but before that thread has replied,
   //  the thread will pick up the revised value and send that as part
   //  of its reply, rather than the value which it read itself.
   //  This should ensure consistency between env poll replies and
   //  value change notification messages.
   struct EvcEnvCache {
      S16   ausBattFuseDrops[2];    // each EVC reads both battery fuses
      CT_EVC_KEYPOS  eKeyPosition;

      //  extract a signed 12-bit payload value
      inline S16 Adc12ToS16 (const CmbPacket& pkt, U32 ibFirstByte)
         {
         S16   sMyRet;

            //  concatenate bytes, using CMB-standard big endianess
            sMyRet = pkt.abTail[ibFirstByte] << 8;
            sMyRet |= pkt.abTail[ibFirstByte + 1];

            //  treat number as 12-bit, and sign-extend it to 16 bits
            if (sMyRet & 0x800)
               {
               sMyRet |= 0xF000;
               }

            return (sMyRet);
         };

      //  set fuse drop values for this EVC record
      void  SetFuseDrops (const CmbPacket& pkt)
         {
            //BUGBUG - we need a ADC fuse voltage-current conversion here
            ausBattFuseDrops[0] = Adc12ToS16 (pkt, 0);
            ausBattFuseDrops[1] = Adc12ToS16 (pkt, 2);
         };

      //  copy our contents to the proper spots in a RawParams record
      void  CopyToRawRec (CtEVCRawParameterRecord& EvcRawRec) const
         {
            EvcRawRec.BatteryCurrent[0] = ausBattFuseDrops[0];
            EvcRawRec.BatteryCurrent[1] = ausBattFuseDrops[1];
            EvcRawRec.KeyPosition = eKeyPosition;
         };

   } m_EvcEnvCache[2];     // one instance for each EVC

   //  we keep a copy of our local instance pointer (only one per board)
   static CDdmCMB  * m_pLocalCmbDdm;


   //  and here are our DDM's CHAOS signal codes
   enum SignalDefs {
      eSigSendEventLogEntry = 1, // send event log entry attached as context
      eSigAtmelMsgWaiting,       // message ready to read from Atmel interface
      eSigAtmelReserved          // reserved for use by Atmel interface object
   };


   //*** message upcaster template function
   template <class CDerivedMsg>
      inline BOOL MsgUpcast (Message *pMsg, CDerivedMsg *&pDerivedMsg)
   {


      if (pMsg->reqCode == CDerivedMsg::MyRequestCode())
         {
         pDerivedMsg = (CDerivedMsg *) pMsg;
         return (TRUE);
         }
      else
         {
         return (FALSE);
         }

   };  /* end of MsgUpcast <> () */


   //  our extended message processing handlers

   //  callbacks to continue work of Initialize()
   void  Initialize2 (void *pvCookie, STATUS status, const CmbPacket *pReply);
   void  Initialize3 (STATUS ulStatus);

   //  the IOP Status table-building phase of initialization
   void   InitializeIopPts (Message *pInitMsg);
   STATUS InitializeIopPts2 (Message *pInitMsg, STATUS status);
   STATUS InitializeIopPts3 (IopInitCtx *pInitCtx, STATUS status);
   STATUS InitializeIopPts4 (IopInitCtx *pInitCtx, STATUS status);

   //  the EVC Raw Params table-building phase of initialization
   void   InitializeEvcPts (IopInitCtx *pInitCtx);
   STATUS InitializeEvcPts2 (IopInitCtx *pInitCtx, STATUS status);
   STATUS InitializeEvcPts3 (IopInitCtx *pInitCtx, STATUS status);
   STATUS InitializeEvcPts4 (IopInitCtx *pInitCtx, STATUS status);

   //  the Env Control Table-building phase of initialization
   void   InitializeEnvCtlPts (IopInitCtx *pInitCtx);
   STATUS InitializeEnvCtlPts2 (Message *pReply);
   STATUS InitializeEnvCtlPts3 (Message *pmsgReply);
   STATUS InitializeEnvCtlPts4 (Message *pmsgReply);

   void   InitializeEnvCtlPtsDone (IopInitCtx *pInitCtx);
   void   InitializeEnvCtlPtsDone2 (void *pvCookie, STATUS sStatus,
                                    const CmbPacket *pReply);
   void   InitializeEnvCtlPtsDone3 (void *pvCookie, STATUS sStatus,
                                    const CmbPacket *pReply);


   //  helper for signalling readiness for unsolicited packet receipt
   void   ReportOsRunningToCma (Message *pMsg);

   //BUGBUG ...1 is a hack, and should be removed!  (AVR state mismatch only)
   void   ReportOsRunningToCma1 (void *pvCookie, STATUS status,
                                 const CmbPacket *pReply);
   void   ReportOsRunningToCma2 (void *pvCookie, STATUS status,
                                 const CmbPacket *pReply);

   STATUS ReportOsRunningToCma3 (Message *pReply);


   //  handlers for our "served" request codes

   //  update a given IOP's status in the IOP status table
   STATUS  ReqUpdateIopStatus (Message *pReqMsg);

   STATUS  ReqUpdateIopStatus2 (void *pClientContext, STATUS status);

   void    ReqUpdateIopStatus3 (void *pvCookie, STATUS status,
                                const CmbPacket *pReply);

   void    ReqUpdateIopStatus4 (void *pvCookie, STATUS status,
                                const CmbPacket *pReply);

   void    ReqUpdateIopStatus5 (void *pvCookie, STATUS status,
                                const CmbPacket *pReply);

   void    ReqUpdateIopStatus6 (void *pvCookie, STATUS status,
                                const CmbPacket *pReply);

   void    ReqUpdateIopStatus7 (void *pvCookie, STATUS status,
                                const CmbPacket *pReply);

   void    ReqUpdateIopStatus8 (void *pvCookie, STATUS status,
                                const CmbPacket *pReply);

   void    ReqUpdateIopStatus9 (void *pvCookie, STATUS status,
                                const CmbPacket *pReply);

   void    ReqUpdateIopStatus10 (void *pvCookie, STATUS status,
                                 const CmbPacket *pReply);

   //  do something to a particular IOP
   STATUS  ReqIopControl (Message *pReqMsg);

   void    ReqIopControl2 (void *pvReqMsg, STATUS sStatus,
                           const CmbPacket *pReply);

   //  do something to a particular EVC
   STATUS  ReqEvcControl (Message *pReqMsg);

   STATUS  ReqEvcControl2 (Message *pRaw);

   void    ReqEvcControl3 (void *pCtx, STATUS sStatus,
                           const CmbPacket *pReply);

   STATUS  ReqEvcControl4 (Message *pReply);

   //  tell EVCs to do something to system power
   STATUS  ReqEvcPowerControl (Message *pReqMsg);

   void    ReqEvcPowerControl2 (void *pvReqMsg, STATUS sStatus,
                                const CmbPacket *pReply);

   //  do something to a particular DDH
   STATUS  ReqDdhControl (Message *pReqMsg);

   void    ReqDdhControl2 (void *pvReqMsg, STATUS sStatus,
                           const CmbPacket *pReply);

   //  enumerate all known IOPs, and add them to the IOP status table
   STATUS  ReqPollAllIops (Message *pReqMsg);

   STATUS  ReqPollAllIops2 (Message *pReply);

   //  send a small request code-based message to another IOP
   STATUS  ReqSendSmallMsg (Message *pReqMsg);

   void    ReqSendSmallMsg2 (void *pvSendSmallMsg,
                             STATUS sStatus, const CmbPacket *pReply);

   //  process a "set mips state" request message, by doing what it says :-)
   STATUS  ReqSetMipsState (Message *pReqMsg);

   void    ReqSetMipsState2 (void *pmsgReq,
                             STATUS sStatus, const CmbPacket *pReply);

   STATUS  ReqSetMipsState3 (Message *pmsgReply);



   //  here are some helper routines

   //  send a message to the CMB interface
   void  SendCmbMsg (void *pvCookie, CmbHwCmdCode eCommand, U8 bDest,
                     CCmbMsgSender::CmbCallback pCmbCallback,
                     U8 cParams = 0, U8 Param1 = 0, U8 Param2 = 0);

   //  send a message via the CMB interface to all present EVCs
   void  SendEvcMsg (void *pvCookie, CmbHwCmdCode eCommand, 
                     CCmbMsgSender::CmbCallback pCmbCallback,
                     U8 cParams = 0, U8 Param1 = 0, U8 Param2 = 0);

   void  SendEvcMsg2 (void *pvSendCtx, STATUS sStatus,
                      const CmbPacket *pReply);

   void  SendEvcMsg3 (void *pvSendCtx, STATUS sStatus,
                      const CmbPacket *pReply);

   //  make sure that an IOP slot parameter is valid
   BOOL  ValidIopSlot (TySlot eSlot, U32& iContigSlot)  const;

   //  allocate & fill in an IOP Status record for a given slot
   IOPStatusRecord *BuildCurrentIopStatus (U32 iContigSlot,
                                           U32 ulPlacement = tSMALL);

   //  allocate & fill in an EVC status record
   EVCStatusRecord *BuildCurrentEvcStatus (U32 ulPlacement = tSMALL);

   //  a little helper for generating user prompts (*temporary*)
   void  MakeSlotMessage (char *pchMsgBuf, const char *pszMsgPrefix,
                          int iContigSlot);


   //  members in CmbPtsUtil.cpp

   //  helper routine for doing a modify of an IOP slot row in PTS
   void  UpdatePtsIopRow (SlotContext *pSlotCtx);

   //  common callback for IOP Status table row-modify requests
   STATUS  FinishIopRowModify (void *pClientContext, STATUS status);

   //  helper routine for doing update of an IOP slot's state value in PTS
   void  UpdatePtsIopState (U32 iContigSlot, IopState eNewState);

   STATUS  UpdatePtsIopState2 (ModifyStateContext *pModifyStateCtx,
                               STATUS sStatus);


   //  members in CmbUnsolicited.cpp

   //  routine for handling incoming (unsolicited) messages from the Atmel
   STATUS  CmbMsgReceived (SIGNALCODE nSignal, void *pPayload);

   //  process a "cmb slot status" update message (response)
   void  HandleSlotStatusUpdate (const CmbPacket& pkt);

   STATUS  HandleSlotStatusUpdate2 (Message *pmsgReply);

   //  process a "cmb slot status" response whose state is "att. needed"
   void  HandleAttentionState (const CmbPacket& pkt);

   void  HandleAttentionState2 (void *pvCookie, STATUS sStatus,
                                const CmbPacket *pReply);

   //  process an "attention needed" flagset from an EVC
   void  HandleAttentionEvc (const CmbPacket& pktReply);

   void  HandleAttentionEvcKey (void * /* pvCookie */, STATUS sStatus,
                                const CmbPacket *pReply);

   void  HandleAttentionEvcBattery (void * /* pvCookie */, STATUS sStatus,
                                    const CmbPacket *pReply);

   //  process an "attention needed" flagset from a DDH
   void  HandleAttentionDdh (const CmbPacket& pktReply);

   void  HandleAttentionDdh2 (void * /* pvCookie */, STATUS sStatus,
                              const CmbPacket *pReply);

   //  process an incoming packet containing a DDM-level message
   void  HandleDdmMessage (const CmbPacket& pkt);

   //  return a NAK of some unrecognized command
   //BUGBUG - must bypass normal send queue, since CMB is stalled
   //         awaiting ack/nak!!!
   void  SendCmbNak (const CmbPacket& pkt);

   //  members in CmbSendLogEntry.cpp

   STATUS  SendEventLogEntry (SIGNALCODE nSignal, void *pvEvent);


   //  members in CmbPtsListenIop.cpp

   //  PTS listen callback for IOP Status table row modifications
   STATUS  PtsIopListenCallback (void *pvCookie, STATUS sListenResult);

   //  process a change in an IOP's low (green / yellow) temperature threshold
   void  ChangeIopTempLowThreshold (U32 iContigSlot, U32 ulTempLowThreshold);

   //  process a change in an IOP's high (yellow / red) temperature threshold
   void  ChangeIopTempHiThreshold (U32 iContigSlot, U32 ulTempHiThreshold);

   //  process a change in an IOP's chassis serial number
   void  ChangeIopChassisSerNum (U32 iContigSlot,
                                 String16 strChassisSerialNumber);

   //  update an IOP's current state in PTS
   void  SetIopPtsStateAndReply (CDdmCmdCarrier *pCmd, IopState eIopState);

   void  SetIopPtsStateAndReply2 (void *pClientContext, STATUS status);

   //  an alternate form of IOP Status update, for use with request messages
   void  SetIopPtsStateAndReply (TySlot eIopSlot, IopState eIopState,
                                 Message *pRequest);

   STATUS  SetIopPtsStateAndReply2a (Message *pmsgReply);


   //  members in CmbDdmCmdProc.cpp

   //  routine for handling incoming command requests (via class CmdServer)
   void  CmdReady (HANDLE hRequest, CmbCtlRequest *pRequest);

   //  process an IOP power control command
   void  ControlIopPower (CDdmCmdCarrier *pCmd);

   void  ControlIopPower2 (void *pvCmd, STATUS ulStatus,
                           const CmbPacket *pReply);

   void  ControlIopPower3 (void *pvCmd, STATUS ulStatus,
                           const CmbPacket *pReply);

   //  process an IOP PCI window set command
   void  ControlIopPciWindow (CDdmCmdCarrier *pCmd);

   //  process an IOP boot control command
   void  ControlIopBoot (CDdmCmdCarrier *pCmd);

   //  process an IOP lock (solenoid) control command
   void  ControlIopLock (CDdmCmdCarrier *pCmd);

   //  process a disk drive bay lock (solenoid) control command
   void  ControlDriveBayLock (CDdmCmdCarrier *pCmd);

   //  process a disk drive FC port bypass control command
   void  ControlDriveBypass (CDdmCmdCarrier *pCmd);

   //  process a "set fan speed" control command
   void  ControlFanSpeed (CDdmCmdCarrier *pReqHolder);

   void  ControlFanSpeed2 (void *pvCmd, STATUS ulStatus,
                           const CmbPacket *pReply);

   void  ControlFanSpeed3 (void *pvCmd, STATUS ulStatus,
                           const CmbPacket *pReply);

   void  ControlFanSpeed4 (void *pvCmd, STATUS ulStatus,
                           const CmbPacket *pReply);

   //  update one CMA's record of the chassis serial number
   void  ControlOneChassisSN (CDdmCmdCarrier *pCmd);

   //  set one IOP's PCI bus access enable flag (on or off)
   void  ControlPciBusAccess (CDdmCmdCarrier *pCmd);

   //  control calling HBC's access to SPI reset lines
   void  ControlSpiResetEnable (CDdmCmdCarrier *pCmd);

   //  change HBC master selection (an EVC command)
   void  ControlHbcMaster (CDdmCmdCarrier *pCmd);





   //  common response callback used by most of our Control*() members (above)
   void  ReportControlResult (void *pvCmd, STATUS ulStatus,
                              const CmbPacket *pReply);

   //  process a "poll environment" request message
   STATUS  ReqPollEnvironment (Message *pReqMsg);

   void  ReqPollEnvironment2 (CEnvPollContext *pContext);

   void  ReqPollEnvironment3 (void *pvContext, STATUS status,
                              const CmbPacket *pReply);

   void  ReqPollEnvironment4 (CEnvPollContext *pContext, STATUS sStatus);

   void  ReqPollEnvironment5 (CEnvPollContext *pContext, STATUS sStatus);

   void  ReqPollEnvironment6 (CEnvPollContext *pContext);

   void  ReqPollEnvironment7 (void *pvContext, STATUS status,
                              const CmbPacket *pReply);

   void  ReqPollEnvironment8 (CEnvPollContext *pContext, STATUS sStatus);

   STATUS  ReqPollEnvironment9 (Message *pmsgReadRow);

   //  helper routine for reporting status back to requester
   void  ReportCmdStatus (CDdmCmdCarrier *pCmd, STATUS ulStatus);

   //  helper for parsing Odyssey-internal FC drive IDs
   STATUS  ParseDriveBayId (U32 ulRawDriveBay, U32& iDdhBay,
                            TySlot& eDdhSlot, U32& iDdhContigSlot);

   //  helpers for mapping from CMB "MIPS State" value to IopState enum
   void  CmbStateToIopStatus (const CmbPacket *pCmbPkt, IopState& eState);
   void  CmbStateToIopStatus (U8 bCmbState, IopState& eState);

public:
   //  help for mapping from a CMB response packet to a STATUS event code
   static void  CmbReplyToStatus (const CmbPacket *pReply, STATUS& sStatus);

private:
   //  a couple of network byte order conversion helpers
   // (MIPS uses big-endian, which *is* network byte order,
   //  so this is easy :-).

   inline S16  ntohs (S16 sNetOrder)
                     {  return (sNetOrder);  };

   inline S32  ntohl (S32 lNetOrder)
                     {  return (lNetOrder);  };

};  /* end of class CDdmCMB */


#endif   // #ifndef _DdmCMB_h_

