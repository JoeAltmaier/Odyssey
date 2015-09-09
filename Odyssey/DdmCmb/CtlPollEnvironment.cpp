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
// File: CtlPollEnvironment.cpp
//
// Description:
//    CMB DDM module.  Contains member routines used for performing
//    a poll of all environmental sensors (power, temperature, etc.)
//    within the Odyssey.
//    See CDdmCMB::ControlPollEnvironment() below for more info.
//
// $Log: /Gemini/Odyssey/DdmCmb/CtlPollEnvironment.cpp $
// 
// 16    2/11/00 4:22p Eric_wedel
// [DFCT13005] Fixed reply leak in ReqPollEnvironment9().  Per JoeA, this
// was causing HBC memory exhaustion.  Thanks for the find, Joe!
// 
// 15    2/08/00 6:59p Eric_wedel
// Fixed a couple of env data conversion bugs found by Huy & Vuong.
// 
// 14    1/20/00 4:23p Eric_wedel
// Fixed callback target signatures.
// 
// 13    12/13/99 2:24p Ewedel
// Took away EVC master support, and added cache of env params (keyswitch
// and fuse drop) which are reported asynchronously.  [VN]
// 
// 12    11/19/99 6:35p Ewedel
// Modified to report fan speed as set speed or zero, depending on
// returned pulse count.
// 
// 11    11/16/99 6:27p Ewedel
// Changed over from PHS query support to proprietary CMB message.
// 
// 10    11/08/99 7:52p Ewedel
// Fixed battery temp scaling & lookup bugs, and added battery presence
// detection.
// 
// 9     11/03/99 12:54a Ewedel
// Added conversions for aux supply current, DC-to-DC temperatures and
// battery temperature.
// 
// 8     10/21/99 6:57p Ewedel
// Added update of EVC Master slot param, and made some of environmental
// parameters real.
// 
// 7     9/03/99 6:40p Ewedel
// Updated for PHS header file change.
// 
// 6     9/03/99 5:24p Ewedel
// Substantial updates to better support env polling, and changed to work
// with PHS interface, rather than cmd queues.
// 
// 5     8/25/99 11:23a Ewedel
// Fixed bug which Joe A found.  Thanks, Joe!
// 
// 4     8/24/99 8:08p Ewedel
// Fixed m_aIopStatusImage[] EVC indexing bug.  Also fixed various jfl TS
// interface bugs.
// 
// 3     8/15/99 12:05p Jlane
// Added parameters for new TS interface changes.
//
// 2     8/11/99 7:49p Ewedel
// Changed over to work with EVC Raw Parameters table, instead of
// consolidated view.  Also stripped some query items which are handled by
// the IOP Status table sweep.
// 
// 1     8/02/99 7:30p Ewedel
// 
/*************************************************************************/

#include  "DdmCMB.h"

#include  "CmbDdmCommands.h"

#include  "CtEvent.h"         // standard status codes

#include  "Rows.h"
#include  "Fields.h"

#include  "RqDdmReporter.h"   // for PHS' SGL ID

#include  "Odyssey_Trace.h"

#include  <assert.h>



//  EVC Query handler helper routines
//  (must match prototype CDdmCMB::pfnEvcQueryHandler())
static void  EvcQHPrimStatus    (CtEVCRawParameterRecord& EvcRec, const CmbPacket *pPkt);
static void  EvcQHPrimVoltage   (CtEVCRawParameterRecord& EvcRec, const CmbPacket *pPkt);
static void  EvcQHAuxCurrents   (CtEVCRawParameterRecord& EvcRec, const CmbPacket *pPkt);
static void  EvcQHAuxTemps      (CtEVCRawParameterRecord& EvcRec, const CmbPacket *pPkt);
static void  EvcQHAuxVoltages   (CtEVCRawParameterRecord& EvcRec, const CmbPacket *pPkt);
static void  EvcQHAuxEnables    (CtEVCRawParameterRecord& EvcRec, const CmbPacket *pPkt);
static void  EvcQHBattFuseDrops (CtEVCRawParameterRecord& EvcRec, const CmbPacket *pPkt);
static void  EvcQHBattTemps     (CtEVCRawParameterRecord& EvcRec, const CmbPacket *pPkt);
static void  EvcQHFanSpeeds     (CtEVCRawParameterRecord& EvcRec, const CmbPacket *pPkt);
static void  EvcQHKeyPos        (CtEVCRawParameterRecord& EvcRec, const CmbPacket *pPkt);


static CDdmCMB::CEvcQueryEntry   aEvcQueryList[]  =  {
   { CMB_EVC0, k_eCmbParamEvcPrimStatus,        4,    EvcQHPrimStatus      },
   { CMB_EVC0, k_eCmbParamEvcPrimVoltage,       2,    EvcQHPrimVoltage     },
   { CMB_EVC0, k_eCmbParamEvcAuxCurrents,       0xA,  EvcQHAuxCurrents     },
   { CMB_EVC0, k_eCmbParamEvcAuxTemperatures,   0xA,  EvcQHAuxTemps        },
   { CMB_EVC0, k_eCmbParamEvcAuxVoltages,       6,    EvcQHAuxVoltages     },
   { CMB_EVC0, k_eCmbParamEvcAuxEnables,        1,    EvcQHAuxEnables      },
   { CMB_EVC0, k_eCmbParamEvcBattFuseDrops,     4,    EvcQHBattFuseDrops   },
   { CMB_EVC0, k_eCmbParamEvcBattTemperatures,  4,    EvcQHBattTemps       },
   { CMB_EVC0, k_eCmbParamEvcFanSpeeds,         8,    EvcQHFanSpeeds       },
   { CMB_EVC0, k_eCmbParamEvcKeyPosition,       1,    EvcQHKeyPos          },
   { CMB_EVC1, k_eCmbParamEvcPrimStatus,        4,    EvcQHPrimStatus      },
   { CMB_EVC1, k_eCmbParamEvcPrimVoltage,       2,    EvcQHPrimVoltage     },
   { CMB_EVC1, k_eCmbParamEvcAuxCurrents,       0xA,  EvcQHAuxCurrents     },
   { CMB_EVC1, k_eCmbParamEvcAuxTemperatures,   0xA,  EvcQHAuxTemps        },
   { CMB_EVC1, k_eCmbParamEvcAuxVoltages,       6,    EvcQHAuxVoltages     },
   { CMB_EVC1, k_eCmbParamEvcAuxEnables,        1,    EvcQHAuxEnables      },
   { CMB_EVC1, k_eCmbParamEvcBattFuseDrops,     4,    EvcQHBattFuseDrops   },
   { CMB_EVC1, k_eCmbParamEvcBattTemperatures,  4,    EvcQHBattTemps       },
   { CMB_EVC1, k_eCmbParamEvcFanSpeeds,         8,    EvcQHFanSpeeds       },
   { CMB_EVC1, k_eCmbParamEvcKeyPosition,       1,    EvcQHKeyPos          },
   { IOP_NONE, (CmbHwParamType) 0,              0,    NULL                 }
};  /* end of aEvcQueryList[] */



//  little helper for network byte order conversion (MIPS uses
//  big-endian, which *is* network byte order, so this is easy :-).
static inline S16  ntohs (S16 sNetOrder)
                     {  return (sNetOrder);  };

//  helper specifically for doing ntohs() on packet abTail values
//  [ibValue is byte offset from start of abTail to first byte of value]
static inline S16  ntohs (const CmbPacket *pPkt, U32 ibValue)
{
   return (ntohs (*(S16 *) (pPkt->abTail + ibValue)));
}

//  here's one for changing one of our 12-bit ADC readings into
//  a sign-extended 16 bit number.  A few of our parameters need this.
static inline S16 Adc12ToS16 (S16 sAdcRaw)
{
   //  all we have to do is sign-extend the number:
   if (sAdcRaw & 0x800)
      {
      sAdcRaw |= 0xF000;
      }
   return (sAdcRaw);
}


//  here are some scaling constants for converting various EVC readings
//  to the form we store in PTS:

const U32   k_ulV33Scale  =  2;     // multiplier to get 3.3V lsb=???
const U32   k_ulV5Scale   =  3;     // multiplier to get 5V lsb=???
const U32   k_ulV12Scale  =  11;    // multiplier to get 12V lsb=???
const U32   k_ulV54Scale  =  51;    // multiplier to get 54V lsb=???

//  maximum current outputs for each DC-to-DC converter in aux supply (lsb=1ma)
const U32   k_ulI33Max   =  25000;     // 25A
const U32   k_ulI5Max    =  40000;     // 40A
const U32   k_ulI12AMax  =   9000;     // 9A
const U32   k_ulI12BMax  =  18330;     // 18.33A
const U32   k_ulI12CMax  =  18330;     // 18.33A


//  scale from raw A/D reading to thermistor voltage in microvolts
//  (A/D reads with 9.5mv lsb; we wish to normalize to 1uv lsb)
const U32   k_ulBattTempScale  =  10000 / 95;

//  here's the lookup table which we use to map from battery thermistor
//  voltage reading to battery temperature:
struct ThermistorMap {
   S32   lTemperature;        // degrees C
   U32   ulVoltsAD;           // thermistor voltage @ EVC's A/D converter,
                              //  expressed here in microvolts
};

static const ThermistorMap  aThermistorMap []  =  {
   {  -55, 2424846  },
   {  -54, 2422677  },
   {  -53, 2420508  },
   {  -52, 2418339  },
   {  -51, 2416170  },
   {  -50, 2414002  },
   {  -49, 2411011  },
   {  -48, 2408020  },
   {  -47, 2405029  },
   {  -46, 2402038  },
   {  -45, 2399048  },
   {  -44, 2395101  },
   {  -43, 2391154  },
   {  -42, 2387207  },
   {  -41, 2383260  },
   {  -40, 2379313  },
   {  -39, 2374071  },
   {  -38, 2368829  },
   {  -37, 2363588  },
   {  -36, 2358346  },
   {  -35, 2353105  },
   {  -34, 2346280  },
   {  -33, 2339456  },
   {  -32, 2332632  },
   {  -31, 2325808  },
   {  -30, 2318984  },
   {  -29, 2310264  },
   {  -28, 2301545  },
   {  -27, 2292825  },
   {  -26, 2284106  },
   {  -25, 2275387  },
   {  -24, 2264566  },
   {  -23, 2253746  },
   {  -22, 2242925  },
   {  -21, 2232105  },
   {  -20, 2221285  },
   {  -19, 2207977  },
   {  -18, 2194670  },
   {  -17, 2181362  },
   {  -16, 2168055  },
   {  -15, 2154748  },
   {  -14, 2138806  },
   {  -13, 2122864  },
   {  -12, 2106922  },
   {  -11, 2090980  },
   {  -10, 2075038  },
   {   -9, 2056411  },
   {   -8, 2037785  },
   {   -7, 2019159  },
   {   -6, 2000533  },
   {   -5, 1981907  },
   {   -4, 1960664  },
   {   -3, 1939421  },
   {   -2, 1918178  },
   {   -1, 1896935  },
   {    0, 1875692  },
   {    1, 1852135  },
   {    2, 1828579  },
   {    3, 1805022  },
   {    4, 1781466  },
   {    5, 1757910  },
   {    6, 1732448  },
   {    7, 1706986  },
   {    8, 1681525  },
   {    9, 1656063  },
   {   10, 1630602  },
   {   11, 1603894  },
   {   12, 1577186  },
   {   13, 1550478  },
   {   14, 1523770  },
   {   15, 1497063  },
   {   16, 1469775  },
   {   17, 1442488  },
   {   18, 1415201  },
   {   19, 1387914  },
   {   20, 1360627  },
   {   21, 1333501  },
   {   22, 1306376  },
   {   23, 1279250  },
   {   24, 1252125  },
   {   25, 1225000  },
   {   26, 1198652  },
   {   27, 1172304  },
   {   28, 1145956  },
   {   29, 1119608  },
   {   30, 1093261  },
   {   31, 1068214  },
   {   32, 1043167  },
   {   33, 1018120  },
   {   34,  993073  },
   {   35,  968026  },
   {   36,  944702  },
   {   37,  921379  },
   {   38,  898055  },
   {   39,  874732  },
   {   40,  851409  },
   {   41,  830091  },
   {   42,  808774  },
   {   43,  787456  },
   {   44,  766139  },
   {   45,  744822  },
   {   46,  725616  },
   {   47,  706410  },
   {   48,  687205  },
   {   49,  667999  },
   {   50,  648794  },
   {   51,  631705  },
   {   52,  614617  },
   {   53,  597529  },
   {   54,  580441  },
   {   55,  563353  },
   {   56,  548305  },
   {   57,  533258  },
   {   58,  518211  },
   {   59,  503164  },
   {   60,  488117  },
   {   61,  474931  },
   {   62,  461746  },
   {   63,  448560  },
   {   64,  435375  },
   {   65,  422190  },
   {   66,  410766  },
   {   67,  399342  },
   {   68,  387918  },
   {   69,  376494  },
   {   70,  365071  },
   {   71,  355227  },
   {   72,  345384  },
   {   73,  335540  },
   {   74,  325697  },
   {   75,  315854  },
   {   76,  307359  },
   {   77,  298865  },
   {   78,  290371  },
   {   79,  281877  },
   {   80,  273383  },
   {   81,  266108  },
   {   82,  258834  },
   {   83,  251559  },
   {   84,  244285  },
   {   85,  237011  },
   {   86,  230742  },
   {   87,  224474  },
   {   88,  218206  },
   {   89,  211938  },
   {   90,  205670  },
   {   91,  200302  },
   {   92,  194934  },
   {   93,  189567  },
   {   94,  184199  },
   {   95,  178832  },
   {   96,  174229  },
   {   97,  169627  },
   {   98,  165025  },
   {   99,  160423  },
   {  100,  155821  },
   {  101,  151868  },
   {  102,  147916  },
   {  103,  143964  },
   {  104,  140012  },
   {  105,  136060  },
   {  106,  132660  },
   {  107,  129261  },
   {  108,  125862  },
   {  109,  122463  },
   {  110,  119064  },
   {  111,  116136  },
   {  112,  113208  },
   {  113,  110280  },
   {  114,  107352  },
   {  115,  104425  },
   {  116,  101905  },
   {  117,   99385  },
   {  118,   96866  },
   {  119,   94346  },
   {  120,   91827  },
   {  121,   89647  },
   {  122,   87467  },
   {  123,   85287  },
   {  124,   83107  },
   {  125,   80928  },
   {  126,   79042  },
   {  127,   77156  },
   {  128,   75270  },
   {  129,   73384  },
   {  130,   71498  },
   {  131,   69862  },
   {  132,   68226  },
   {  133,   66590  },
   {  134,   64954  },
   {  135,   63319  },
   {  136,   61896  },
   {  137,   60473  },
   {  138,   59051  },
   {  139,   57628  },
   {  140,   56206  },
   {  141,   54968  },
   {  142,   53730  },
   {  143,   52492  },
   {  144,   51254  },
   {  145,   50016  },
   {  146,   48932  },
   {  147,   47848  },
   {  148,   46764  },
   {  149,   45680  },
   {  150,   44596  }
};  /* end of aThermistorMap [] */

//  count of entries in aThermistorMap[]:
static U32  cThermistorMap  =  sizeof (aThermistorMap) / sizeof (*aThermistorMap);


//
//  CDdmCMB::ReqPollEnvironment (pEnvPollRequest)
//
//  Description:
//    Called when we receive a CMB_POLL_ENVIRONMENT request message.
//
//    We do a sweep through all Odyssey hardware, reading its current
//    environmental sensor values.  We do all this via the CMB,
//    of course.
//
//    We place the results of the sweep in the EVC Raw Parameters table.
//    We also update the temperature columns of each IOP status table row
//    which represents a non-empty "slot".
//
//    We defer our response until we have completed our sweep, and
//    completed our requisite PTS updates.  Finally, we also return a copy
//    of our poll data in the reply to the CMB_POLL_ENVIRONMENT message also.
//
//  Inputs:
//    pEnvPollRequest - Points to request message sent to us to trigger
//             our environmental poll.
//
//  Outputs:
//    CDdmCMB::ReqPollEnvironment - Always returns CTS_SUCCESS, as a
//             good message handler should.
//

STATUS  CDdmCMB::ReqPollEnvironment (Message *pEnvPollRequest)
{

CEnvPollContext * pContext;


   assert ((pEnvPollRequest != NULL) &&
           (pEnvPollRequest->reqCode == CMB_POLL_ENVIRONMENT));

   //  we first do all of the EVC readings.  These are table-driven,
   //  just to keep from driving ourselves crazy.  :-)

   //  make up a context carrier, since we have a *lot* of steps
   //  to walk through.
   pContext = new CEnvPollContext ((MsgCmbPollEnvironment *) pEnvPollRequest);

   //  build up context to be digestable by our poll continuation routine
   pContext -> m_pCurEvcQuery = aEvcQueryList;

   //  make sure termination condition isn't true yet
   assert (pContext->m_pCurEvcQuery->pfnHandler != NULL);

   //  and let our continuation routine start things off

   ReqPollEnvironment2 (pContext);

   return (CTS_SUCCESS);

}  /* end of CDdmCMB::ReqPollEnvironment */


//  We're called to start up the next EVC query from aEvcQueryList[].
//
//  Inputs:
//    pContext - Context carrier used to track query list processing.
//

void  CDdmCMB::ReqPollEnvironment2 (CEnvPollContext *pContext)
{

IopState       eState;
TSModifyRow  * pModifyRow;
STATUS         sRet;


   //  find next viable query -- make sure we haven't run off the end
   //  of our query list, and that the next query's EVC appears to be
   //  present in the system:
   while ((pContext->m_pCurEvcQuery->pfnHandler != NULL) &&
          (m_aIopStatusImage [
              m_aiContigEvc[pContext->m_pCurEvcQuery->eTarget - CMB_EVC0]].eState
                  != IOPS_POWERED_ON))
      {
      //  skip this query..
      pContext->m_pCurEvcQuery ++;
      }

   //  if we found a workable one, send it            
   if (pContext -> m_pCurEvcQuery ->pfnHandler != NULL)
      {
      //  got another query to do, get it started
      SendCmbMsg (pContext, k_eCmbCmdGetLastValue,
                  pContext->m_pCurEvcQuery->eTarget,
                  CMBCALLBACK (CDdmCMB, ReqPollEnvironment3),
                  1, pContext->m_pCurEvcQuery->eInfoSubCode);
      }
   else
      {
      //  whoops, all done with EVC queries.  Save EVC param records,
      //  and then move on to IOP Status table slot scan (for temperatures)

      //  first, update EVC-reachable flags, which aren't otherwise touched

      //  grab first EVC's current state
      eState = m_aIopStatusImage[m_aiContigEvc[0]].eState;

      //  reduce it to a simple go/no-go boolean
      pContext->m_EnvStatus.EVCRawParameters[0].fEvcReachable =
                     ((eState != IOPS_EMPTY) && (eState != IOPS_BLANK));

      //  grab second EVC's current state
      eState = m_aIopStatusImage[m_aiContigEvc[1]].eState;

      //  and reduce it to a simple go/no-go boolean
      pContext->m_EnvStatus.EVCRawParameters[1].fEvcReachable =
                     ((eState != IOPS_EMPTY) && (eState != IOPS_BLANK));

      //  copy dynamic values from our evc env cache into the records
      m_EvcEnvCache[0].CopyToRawRec (pContext->m_EnvStatus.EVCRawParameters[0]);
      m_EvcEnvCache[1].CopyToRawRec (pContext->m_EnvStatus.EVCRawParameters[1]);

      //  finally, update EVC raw param records in PTS

      pModifyRow = new TSModifyRow;

      //  we change the row to our built-up param struct
      sRet = pModifyRow -> Initialize (
         this,
         CT_EVC_RAW_PARAM_TABLE,
         CT_EVCRP_EVCSLOTID,        // use EVC ID field as our key
         &pContext->m_EnvStatus.EVCRawParameters[0].EvcSlotId,
         sizeof (pContext->m_EnvStatus.EVCRawParameters[0].EvcSlotId),
         pContext->m_EnvStatus.EVCRawParameters + 0,
         sizeof (pContext->m_EnvStatus.EVCRawParameters[0]),
         1,                         // modify just one row
         NULL,                      // ignore rows-modified return
         &pContext->m_ridNewRow,
         sizeof (pContext->m_ridNewRow),
         (pTSCallback_t) &ReqPollEnvironment4,     // (skip over ...3)
         pContext);

      assert (sRet == CTS_SUCCESS);

      //  send the PTS row modify message
      pModifyRow->Send();
      }

   //  response or error resumes in ReqPollEnvironment3()
   return;

}  /* end of CDdmCMB::ReqPollEnvironment2 */


//  We're called back via SendCmbMsg() to handle the CMB's reply
//  to one of our EVC queries.
//  We run the response through a custom query handler, and then
//  go around for the next query.
//
//  Inputs:
//    pvContext - Context carrier used to track query list processing.
//                [SendCmbMsg()'s cookie value.]
//    status - Result of CMB send operation.  Should always be success,
//                unless we couldn't talk to our local CMA (in which
//                case we're in big trouble).
//    pReply - Result packet received from CMB.
//

void  CDdmCMB::ReqPollEnvironment3 (void *pvContext, STATUS status,
                                    const CmbPacket *pReply)
{

CEnvPollContext * pContext  =  (CEnvPollContext *) pvContext;


   //  raw status should always be happy:
//* (we occasionally get a bad packet back, on purpose :-)
//*   assert (status == CTS_SUCCESS);

   assert (pContext != NULL);
   assert (pContext->m_pCurEvcQuery != NULL);
   assert (pContext->m_pCurEvcQuery->pfnHandler != NULL);
   assert ((pReply->Hdr.bSrcAddr == CMB_EVC0) ||
           (pReply->Hdr.bSrcAddr == CMB_EVC1));

   if (status == CTS_SUCCESS)
      {
      //  got a good response, so process it
      (*pContext->m_pCurEvcQuery->pfnHandler)
            (pContext->m_EnvStatus.EVCRawParameters[
                                 pReply->Hdr.bSrcAddr - CMB_EVC0], pReply);
      }

   //  regardless, advance to next EVC query list entry
   pContext->m_pCurEvcQuery ++;

   //  and let our query sender do the rest
   ReqPollEnvironment2 (pContext);

   return;

}  /* end of CDdmCMB::ReqPollEnvironment3 */


//  We're called back with the result of our first EVC Raw Param record
//  update (via TSModifyRow).  We continue by sending the update request
//  for our second EVC Raw Param record.
//
//  Inputs:
//    pContext - Context carrier used to track processing.
//    sStatus - Result of EVC Raw Param table row modify.
//

void  CDdmCMB::ReqPollEnvironment4 (CEnvPollContext *pContext,
                                    STATUS sStatus)
{

TSModifyRow  * pModifyRow;
STATUS         sRet;


   //  first row modify should always succeed
   assert (sStatus == CTS_SUCCESS);

   //  regardless, start the second row modify
   pModifyRow = new TSModifyRow;

   //  we change the row to our built-up param struct
   sRet = pModifyRow -> Initialize (
      this,
      CT_EVC_RAW_PARAM_TABLE,
      CT_EVCRP_EVCSLOTID,        // use EVC ID field as our key
      &pContext->m_EnvStatus.EVCRawParameters[1].EvcSlotId,
      sizeof (pContext->m_EnvStatus.EVCRawParameters[1].EvcSlotId),
      pContext->m_EnvStatus.EVCRawParameters + 1,
      sizeof (pContext->m_EnvStatus.EVCRawParameters[1]),
      1,                         // modify just one row
      NULL,                      // ignore rows-modified return
      &pContext->m_ridNewRow,
      sizeof (pContext->m_ridNewRow),     // or sizeof(ptr) ??
      (pTSCallback_t) &ReqPollEnvironment5,
      pContext);

   assert (sRet == CTS_SUCCESS);

   if (sRet == CTS_SUCCESS)
      {
      //  send the PTS row modify message
      pModifyRow->Send();
      }
   else
      {
      //  whoops, gotta make our callback directly
      ReqPollEnvironment5 (pContext, sRet);
      }

   return;

}  /* end of CDdmCMB::ReqPollEnvironment4 */


//  We're called back with the result of our EVC Raw Param record update
//  (via TSModifyRow), or with the result of an IOP Status table row
//  update (via TSModifyField).
//
//  We move on to sweep through all known-populated CMB slots, reading
//  the Dallas DS1720 temperature values from each, and updating the
//  corresponding columns in the IOP Status table.
//
//  Inputs:
//    pContext - Context carrier used to track processing.
//    sStatus - Result of EVC Raw Param table row modify.
//

void  CDdmCMB::ReqPollEnvironment5 (CEnvPollContext *pContext,
                                     STATUS sStatus)
{


   //  PTS modify should never fail (if it does, we keep going anyway)
   assert (sStatus == CTS_SUCCESS);

   assert (pContext != NULL);

   //  start off with first slot
   pContext->m_iContigSlot = 0;

   //  and drop into our follow-on routine to do the actual query
   ReqPollEnvironment6 (pContext);

   return;

}  /* end of CDdmCMB::ReqPollEnvironment5 */


//  We're called back when we've done zero or more IOP temperature reads,
//  and are ready to do the next one.
//
//  We find the next non-empty slot, and send off a poll of its temperature.
//
//  Inputs:
//    pContext - Context carrier used to track processing.
//

void  CDdmCMB::ReqPollEnvironment6 (CEnvPollContext *pContext)
{

IopState    eState;


   //  find next non-empty slot (m_iContigSlot is already set for us)

   eState = m_aIopStatusImage [pContext->m_iContigSlot].eState;

   while (((eState == IOPS_EMPTY) || (eState == IOPS_BLANK)) &&
          (pContext->m_iContigSlot < CT_IOPST_MAX_IOP_SLOTS))
      {
      //  mark this slot as not present in our results
      pContext->CurSlot().fPresent = FALSE;

      //  move to next slot, and cache its state
      pContext->m_iContigSlot ++;
      eState = m_aIopStatusImage [pContext->m_iContigSlot].eState;
      }

   //  if we've got any slots left to read, get to it:
   if (pContext->m_iContigSlot < CT_IOPST_MAX_IOP_SLOTS)
      {
      //  mark that we think this slot is present, even if we fail to
      //  read its temperature or threshold params
      pContext->CurSlot().fPresent = TRUE;

      //  read slot's temperature
      SendCmbMsg (pContext, k_eCmbCmdGetLastValue,
                  m_aeContigToSlot[pContext->m_iContigSlot],
                  CMBCALLBACK (CDdmCMB, ReqPollEnvironment7),
                  1, k_eCmbParamTemperature);
      }
   else
      {
      //  all done, reply to requestor & clean up.

      //  plunk our accumulated data into the reply message
      pContext->m_pReqMsg->SetPayload (pContext->m_EnvStatus);

      //  now send reply back to caller
      Reply (pContext->m_pReqMsg, CTS_SUCCESS);

      //  finally, clean up our context cookie
      delete pContext;
      }

   //  ReqPollEnvironment7() will pick up results, good, bad or indifferent
   return;

}  /* end of CDdmCMB::ReqPollEnvironment6 */


//  We're called back via SendCmbMsg() to handle the CMB's reply
//  to one of our CMB temperature queries.
//  We save the temperature away in the IOP Status table
//
//  Inputs:
//    pContext - Context carrier used to track query list processing.
//                [SendCmbMsg()'s cookie value.]
//    status - Result of CMB send operation.  Should always be success,
//                unless we couldn't talk to our local CMA (in which
//                case we're in big trouble).
//    pReply - Result packet received from CMB.
//

void  CDdmCMB::ReqPollEnvironment7 (void *pvContext, STATUS status,
                                    const CmbPacket *pReply)
{

CEnvPollContext * pContext  =  (CEnvPollContext *) pvContext;
TSModifyField   * pmsgTempUpd;
S16               sTemp;            // native byte-order temperature value
STATUS            sRet;


   //  raw status should always be happy:
   assert (status == CTS_SUCCESS);

   if (status == CTS_SUCCESS)
      {
      //  got a good response, so stash it in PTS

      //  first, save temperature in our context struct
      sTemp = ::ntohs (pReply, 0);
      pContext->CurSlot().Temperature = 5 * sTemp;     // scale to 0.1 degree lsb

      //  make basic "field update" message instance
      pmsgTempUpd = new TSModifyField;
      assert (pmsgTempUpd != NULL);

      //  build up our "field update" for changing IOP's temperature.
      //  Note that we use our (carefully contrived) request cookie
      //  as the field update cookie also.
      sRet = pmsgTempUpd -> Initialize (
                  this, CT_IOPST_TABLE_NAME,
                  CT_PTS_RID_FIELD_NAME,
                  &m_aIopStatusImage[pContext->m_iContigSlot].rid,
                  sizeof (m_aIopStatusImage[
                                pContext->m_iContigSlot].rid),
                  CT_IOPST_TEMP,
                  &pContext->CurSlot().Temperature,
                  sizeof (pContext->CurSlot().Temperature),
                  1,                   // update just one row
                  NULL,                // ignore rows-modified return
                  NULL, 0,
                  (pTSCallback_t) &ReqPollEnvironment8,
                  pContext);

      assert (sRet == CTS_SUCCESS);

      if (sRet == CTS_SUCCESS)
         {
         //  send message off to do exciting things..
         pmsgTempUpd -> Send ();
         }
      else
         {
         //  whoops, PTS stuff messed up.  Do our callback directly.
         ReqPollEnvironment8 (pContext, sRet);

         //  dispose of now-unused TS instance
         delete pmsgTempUpd;
         }
      }
   else
      {
      //  whoops, bad temperature reply, so don't update PTS.  To keep things
      //  going, we call our PTS callback directly.
      ReqPollEnvironment8 (pContext, status);
      }

   return;

}  /* end of CDdmCMB::ReqPollEnvironment7 */


//  We're called back with the result of our IOP Status table row update
//  (via TSModifyField).
//
//  We then read the IOP Status table row to obtain the current
//  temperature threshold values.
//
//  Inputs:
//    pContext - Context carrier used to track processing.
//    sStatus - Result of IOP Status table row modify.
//

void  CDdmCMB::ReqPollEnvironment8 (CEnvPollContext *pContext,
                                     STATUS sStatus)
{

IOPStatusRecord::RqReadRow  * pReadRow;


   //  modify-field is on best-effort basis
   assert (sStatus == CTS_SUCCESS);

   pReadRow = new IOPStatusRecord::RqReadRow (
                        m_aIopStatusImage[pContext->m_iContigSlot].rid);

   sStatus = Send (pReadRow, pContext,
                   REPLYCALLBACK(CDdmCMB, ReqPollEnvironment9));

   assert (sStatus == CTS_SUCCESS);

   return;

}  /* end of CDdmCMB::ReqPollEnvironment8 */


//  We're called back with the result of our IOP Status table row read
//  (via IOPStatusRecord::RqReadRow).
//
//  We start the next CMB temperature query going or, if we've done all
//  slots, then we send the results back to our caller.
//
//  We move on to sweep through all known-populated CMB slots, reading
//  the Dallas DS1720 temperature values from each, and updating the
//  corresponding columns in the IOP Status table.
//
//  Inputs:
//    pmsgReadRow - Results of our row-read request.
//

STATUS  CDdmCMB::ReqPollEnvironment9 (Message *pmsgReadRow)
{

IOPStatusRecord::RqReadRow  * pReadRow;
CEnvPollContext             * pContext;


   pReadRow = (IOPStatusRecord::RqReadRow *) pmsgReadRow;
   assert (pReadRow != NULL);

   pContext = (CEnvPollContext *) pReadRow->GetContext ();
   assert (pContext != NULL);

   assert (pReadRow->Status() == CTS_SUCCESS);

   if (pReadRow->Status() == CTS_SUCCESS)
      {
      //  got row back, save away temperature thresholds

      pContext->CurSlot().TemperatureHiThreshold = 
               pReadRow->GetRowPtr()->TempHiThreshold;
      pContext->CurSlot().TemperatureNormThreshold =
               pReadRow->GetRowPtr()->TempNormThreshold;
      }


   //  bump contiguous slot number
   pContext->m_iContigSlot ++;

   //  dispose of PTS reply
   delete pReadRow;

   //  and move on to next occupied slot
   ReqPollEnvironment6 (pContext);

   return (CTS_SUCCESS);      // as reply callbacks always do

}  /* end of CDdmCMB::ReqPollEnvironment9 */

//
//  EvcQH* (EvcRec, pPkt)
//
//  Description:
//    This family of routines is used to parse out environmental data
//    in a packet-specific way within our table-driven EVC scan.
//
//    The various routines each extract their own bit of data from the
//    given packet, and place it into the supplied EVC Status record.
//
//  Inputs:
//    pPkt - Points to CMB response packet containing data of interest.
//
//  Outputs:
//    EvcRec - Updated with appropriate data from *pPkt.  [*Which* data
//                depends on the particular routine.]
//

//  capture primary power supply status info
static void  EvcQHPrimStatus    (CtEVCRawParameterRecord& EvcRec,
                                 const CmbPacket *pPkt)
{


   assert (pPkt->Hdr.cbData == 4);

   if (pPkt->Hdr.cbData == 4)
      {
      assert ((pPkt->Hdr.bStatus & CmbStatAck) != 0);

      //  got three supplies' worth of data, roll it all in
      for (int i = 0;  i < 3;  i ++)
         {
         EvcRec.fInputOK[i] = (((pPkt->abTail[0] >> i) & 1) != 0);
         EvcRec.fOutputOK[i] = (((pPkt->abTail[1] >> i) & 1) != 0);
         EvcRec.fFanFailOrOverTemp[i] = (((pPkt->abTail[2] >> i) & 1) != 0);
         EvcRec.fPrimaryEnable[i] = (((pPkt->abTail[3] >> i) & 1) != 0);
         }
      }

   return;

}  /* end of EvcQHPrimStatus */


//  capture primary power supply voltage readings
static void  EvcQHPrimVoltage   (CtEVCRawParameterRecord& EvcRec,
                                 const CmbPacket *pPkt)
{
   
   
   assert (pPkt->Hdr.cbData == 2);

   if (pPkt->Hdr.cbData == 2)
      {
      assert ((pPkt->Hdr.bStatus & CmbStatAck) != 0);
      assert ((pPkt->Hdr.bSrcAddr == CMB_EVC0) ||
              (pPkt->Hdr.bSrcAddr == CMB_EVC1));
      assert (pPkt->Hdr.bSrcAddr == EvcRec.EvcSlotId);

      EvcRec.SMP48Voltage = k_ulV54Scale * ntohs (pPkt, 0);
      }

   return;

}  /* end of EvcQHPrimVoltage */


//  capture aux power supply current readings

//  compute one aux current value, from parameter in CMB packet
//  (units are same as in input arg ulCurrentMax)
static inline U32 AuxCurrent (const CmbPacket *pPkt, U32 ibValue,
                              U32 ulCurrentMax)
{
   //  note that we add 1000 before dividing, so that divide is "rounded"
   return ((ntohs (pPkt, ibValue) * ulCurrentMax + 1000) / 2000);
}

static void  EvcQHAuxCurrents   (CtEVCRawParameterRecord& EvcRec,
                                 const CmbPacket *pPkt)
{


   assert (pPkt->Hdr.cbData == 0x0A);

   if (pPkt->Hdr.cbData == 0x0A)
      {
      assert ((pPkt->Hdr.bStatus & CmbStatAck) != 0);
      assert ((pPkt->Hdr.bSrcAddr == CMB_EVC0) ||
              (pPkt->Hdr.bSrcAddr == CMB_EVC1));
      assert (pPkt->Hdr.bSrcAddr == EvcRec.EvcSlotId);

      //  do each DC-to-DC's conversion separately, since they most all
      //  have different individual current limits (and our raw readings
      //  are roughly a percentage of maximum current ouput).
      //  Note that the different values for various 12V converters are
      //  not a typo; they really are different.

      EvcRec.DCtoDC33Current  = AuxCurrent (pPkt, 0, k_ulI33Max);
      EvcRec.DCtoDC5Current   = AuxCurrent (pPkt, 2, k_ulI5Max);
      EvcRec.DCtoDC12ACurrent = AuxCurrent (pPkt, 4, k_ulI12AMax);
      EvcRec.DCtoDC12BCurrent = AuxCurrent (pPkt, 6, k_ulI12BMax);
      EvcRec.DCtoDC12CCurrent = AuxCurrent (pPkt, 8, k_ulI12CMax);
      }

   return;

}  /* end of EvcQHAuxCurrents */


//  compute one aux supply DC-to-DC temp value, from raw CMB param
//  (output units are lsb=1 deg C)
static inline U32 AuxTemp (const CmbPacket *pPkt, U32 ibValue)
{
   return (ntohs (pPkt, ibValue) * 2 * 100 - 273);
}

//  read aux power supply DC-to-DC converter temperatures
static void  EvcQHAuxTemps      (CtEVCRawParameterRecord& EvcRec,
                                 const CmbPacket *pPkt)
{


   assert (pPkt->Hdr.cbData == 0x0A);

   if (pPkt->Hdr.cbData == 0x0A)
      {
      assert ((pPkt->Hdr.bStatus & CmbStatAck) != 0);
      assert ((pPkt->Hdr.bSrcAddr == CMB_EVC0) ||
              (pPkt->Hdr.bSrcAddr == CMB_EVC1));
      assert (pPkt->Hdr.bSrcAddr == EvcRec.EvcSlotId);

      //  what is funny conversion which Bob C documents for ENB7 on ADC 7?
      EvcRec.DCtoDC33Temp  = AuxTemp (pPkt, 0);
      EvcRec.DCtoDC5Temp   = AuxTemp (pPkt, 2);
      EvcRec.DCtoDC12ATemp = AuxTemp (pPkt, 4);
      EvcRec.DCtoDC12BTemp = AuxTemp (pPkt, 6);
      EvcRec.DCtoDC12CTemp = AuxTemp (pPkt, 8);
      }

   return;

}  /* end of EvcQHAuxTemps */


//  read aux power supply voltages (combined outputs of both aux supplies)
static void  EvcQHAuxVoltages   (CtEVCRawParameterRecord& EvcRec,
                                 const CmbPacket *pPkt)
{


   assert (pPkt->Hdr.cbData == 0x06);

   if (pPkt->Hdr.cbData == 0x06)
      {
      assert ((pPkt->Hdr.bStatus & CmbStatAck) != 0);
      assert ((pPkt->Hdr.bSrcAddr == CMB_EVC0) ||
              (pPkt->Hdr.bSrcAddr == CMB_EVC1));
      assert (pPkt->Hdr.bSrcAddr == EvcRec.EvcSlotId);

      //  do aux voltage conversions
      EvcRec.DCtoDC33Voltage = k_ulV33Scale * ntohs (pPkt, 0);
      EvcRec.DCtoDC5Voltage  = k_ulV5Scale  * ntohs (pPkt, 2);
      EvcRec.DCtoDC12Voltage = k_ulV12Scale * ntohs (pPkt, 4);
      }

   return;

}  /* end of EvcQHAuxVoltages */


//  read aux power supply enable flags
static void  EvcQHAuxEnables    (CtEVCRawParameterRecord& EvcRec,
                                 const CmbPacket *pPkt)
{


   assert (pPkt->Hdr.cbData == 0x01);

   if (pPkt->Hdr.cbData == 0x01)
      {
      assert ((pPkt->Hdr.bStatus & CmbStatAck) != 0);
      assert ((pPkt->Hdr.bSrcAddr == CMB_EVC0) ||
              (pPkt->Hdr.bSrcAddr == CMB_EVC1));
      assert (pPkt->Hdr.bSrcAddr == EvcRec.EvcSlotId);

      EvcRec.fDCtoDCEnable[0] = ((pPkt->abTail[0] & 1) != 0);
      EvcRec.fDCtoDCEnable[1] = ((pPkt->abTail[0] & 2) != 0);
      }

   return;

}  /* end of EvcQHAuxEnables */


//  read voltage drops across battery fuses, infer current from them
static void  EvcQHBattFuseDrops (CtEVCRawParameterRecord& EvcRec,
                                 const CmbPacket *pPkt)
{


   assert (pPkt->Hdr.cbData == 0x04);

   if (pPkt->Hdr.cbData == 0x04)
      {
      assert ((pPkt->Hdr.bStatus & CmbStatAck) != 0);
      assert ((pPkt->Hdr.bSrcAddr == CMB_EVC0) ||
              (pPkt->Hdr.bSrcAddr == CMB_EVC1));
      assert (pPkt->Hdr.bSrcAddr == EvcRec.EvcSlotId);

      //  fuse readings are one of those dynamically-updatable things,
      //  so we toss them into our evc env cache:
      CDdmCMB::SetFuseDrops ((TySlot) pPkt->Hdr.bSrcAddr, *pPkt);

      //  ReqPollEnvironment2() will copy these into our output record
      }

   return;

}  /* end of EvcQHBattFuseDrops */


//  use lookup table to map battery thermistor reading into a temperature
//    pPkt - Points to packet containing temperature query reply
//    iBatt - Zero-based index of battery which was queried
//    EvcRec - Reference to record where we place temperature data.
static void  BattTemp (const CmbPacket *pPkt, U32 iBatt,
                       CtEVCRawParameterRecord& EvcRec)
{

U32   ulThermV;
S32   iFirst;
S32   iLast;
S32   iMidpoint;
U32   ulMidpointVolts;
U16   usRawTemp;
const U16   usInvalidTemp  =  0xFFFF;     // == no battery present


   //  grab reading from target battery's thermistor, scaled to microvolts
   usRawTemp = ntohs (pPkt, 2 * iBatt);   // (each battery has two bytes of temp)
   if (usRawTemp == usInvalidTemp)
      {
      //  whoops, stop right here.  No battery present in iBatt slot.
      EvcRec.BatteryTemperature[iBatt] = 0;
      EvcRec.BatteryPresent[iBatt] = FALSE;
      return;
      }

   //  we appear to have a battery, figure out its temperature

   //  scale to microvolts
   ulThermV = k_ulBattTempScale * (U32) usRawTemp;

   //  the raw reading which we have here is the A/D voltage off of
   //  a divider which incorporates each battery's thermistor.
   //  We have carefully contrived the ulVoltsAD members of
   //  aThermistorMap[] so that they exactly match this voltage,
   //  except for being scaled to microvolts instead of millivolts.
   //  The preceding k_ulBattTempScale factor fixed that.

   //  Now, let's do a binary search in our lookup table to find
   //  the nearest entry to our reading.

   //  Note:  the voltage entries which we use as a key are monotonic,
   //  but are decreasing.  Thus, some of the conditions may look
   //  backwards.  Also, we will almost never find an exact match,
   //  so the logic is searching for a nearest match.

   iFirst = 0;
   iLast = cThermistorMap - 1;      // highest legal 0-based index

   while (iFirst <= iLast)
      {
      iMidpoint = (iFirst + iLast) / 2;
      ulMidpointVolts = aThermistorMap[iMidpoint].ulVoltsAD;

      if (ulThermV < ulMidpointVolts)
         {
         //  our value is lower, so move up in the table
         //  (negative temperature coefficients, and all that)
         iFirst = iMidpoint + 1;
         }
      else if (ulThermV > ulMidpointVolts)
         {
         //  our value is higher, so move down in table, seeking higher values
         iLast = iMidpoint - 1;
         }
      else
         {
         //  egads, we exactly hit a value.  Zounds!
         iFirst = iLast = iMidpoint;
         break;
         }
      }

   //  grab the most recent midpoint, and the nearest legal value.
   //  This should put us on the right reading, plus or minus one
   //  degree.  Since the hardware is generally considered less
   //  accurate than this, we don't sweat it.
   assert (iMidpoint < cThermistorMap);

   //  return nearest temperature equivalent to caller
   EvcRec.BatteryTemperature[iBatt] = aThermistorMap[iMidpoint].lTemperature;
   EvcRec.BatteryPresent[iBatt] = TRUE;

}  /* end of BattTemp */


//  read temperature of cells in each battery module (FRU)
static void  EvcQHBattTemps     (CtEVCRawParameterRecord& EvcRec,
                                 const CmbPacket *pPkt)
{

int   iEvc;


   assert (pPkt->Hdr.cbData == 0x04);

   if (pPkt->Hdr.cbData == 0x04)
      {
      assert ((pPkt->Hdr.bStatus & CmbStatAck) != 0);
      assert ((pPkt->Hdr.bSrcAddr == CMB_EVC0) ||
              (pPkt->Hdr.bSrcAddr == CMB_EVC1));
      assert (pPkt->Hdr.bSrcAddr == EvcRec.EvcSlotId);

      iEvc = pPkt->Hdr.bSrcAddr - CMB_EVC0;

      //  write respective battery temps to their slots in EVC rec
      BattTemp (pPkt, 0, EvcRec);
      BattTemp (pPkt, 1, EvcRec);
      }

   return;

}  /* end of EvcQHBattTemps */


//  a little helper for fan speed determination (see comment
//  in EvcQHFanSpeeds() below for more details)
inline void  FudgeFanSpeed (CtEVCRawParameterRecord& EvcRec,
                            const CmbPacket *pPkt,
                            U32 iFan, CmbFanSpeedSetFanSelect eFanPair)
{
static const S16  usFanRunning  =  15;    // pulse count threshold at which
                                          // we consider a fan to be spinning

   EvcRec.FanSpeed[iFan] = (ntohs (pPkt, 2*iFan) >= usFanRunning) ?
               CDdmCMB::FanSpeedSet(eFanPair) : 0;
}


//  read main exhaust & intake fan speeds
static void  EvcQHFanSpeeds     (CtEVCRawParameterRecord& EvcRec,
                                 const CmbPacket *pPkt)
{


   assert (pPkt->Hdr.cbData == 0x08);

   if (pPkt->Hdr.cbData == 0x08)
      {
      assert ((pPkt->Hdr.bStatus & CmbStatAck) != 0);
      assert ((pPkt->Hdr.bSrcAddr == CMB_EVC0) ||
              (pPkt->Hdr.bSrcAddr == CMB_EVC1));
      assert (pPkt->Hdr.bSrcAddr == EvcRec.EvcSlotId);

      //  convert EVC-reported fan pulse count into % of fan full speed.

      //  The EVCs return us fan speed as a simple pulse count.
      //  At present, this count cannot be reliably mapped into
      //  a fan RPM, or even a % of fan full speed, as our control
      //  values are expressed.
      //  So for now, we take any "running" pulse count, defined
      //  as any count >= 15, and translate it into that fan's
      //  specified fan speed setting.  For counts < 15, we change
      //  the fan's speed readout to zero.
      //  By placing this rather kludgey logic here, we preserve
      //  the apparent integrity of the EVC [Raw] Status table, so
      //  if we get better hardware in the future the changes to
      //  accomodate it should be limited to this code right here.

      //  fans 0 and 1 are exhaust:
      FudgeFanSpeed (EvcRec, pPkt, 0, k_eCmbFanPairExhaust);
      FudgeFanSpeed (EvcRec, pPkt, 1, k_eCmbFanPairExhaust);

      //  fans 2 and 3 are intake:
      FudgeFanSpeed (EvcRec, pPkt, 2, k_eCmbFanPairIntake);
      FudgeFanSpeed (EvcRec, pPkt, 3, k_eCmbFanPairIntake);
      }

   return;

}  /* end of EvcQHFanSpeeds */


//  read position of rotary locking key switch
static void  EvcQHKeyPos        (CtEVCRawParameterRecord& EvcRec,
                                 const CmbPacket *pPkt)
{


   assert (pPkt->Hdr.cbData == 0x01);

   if (pPkt->Hdr.cbData == 0x01)
      {
      assert ((pPkt->Hdr.bStatus & CmbStatAck) != 0);
      assert ((pPkt->Hdr.bSrcAddr == CMB_EVC0) ||
              (pPkt->Hdr.bSrcAddr == CMB_EVC1));
      assert (pPkt->Hdr.bSrcAddr == EvcRec.EvcSlotId);

      //  we carefully chose the CT_EVC_KEYPOS enum so that its member
      //  values match those returned by the EVCs.  So this is easy.
      //  Note that we save the position value in our cache, so it
      //  will reflect any subsequent updates sent by the EVC.
      CDdmCMB::SetKeyPos ((TySlot) pPkt->Hdr.bSrcAddr,
                          (CT_EVC_KEYPOS) (pPkt->abTail[0] & 3));
      
      //  ReqPollEnvironment2() will copy these into our output record
      }

   return;

}  /* end of EvcQHKeyPos */


#ifdef NEED_SERIAL_NUM_COPY
//
//  EvcQHCommonSN (SerialNumber, pPkt)
//
//  Description:
//    A helper for copying a serial number value from a CMB packet
//    into a String16 (usually inside an EVC record).
//
//  Inputs:
//    pPkt - Points to CMB response packet containing serial number data.
//
//  Outputs:
//    SerialNumber - Loaded with serial number data from *pPkt.  We use
//                a binary copy, so the serial number may contain
//                embedded nuls.  We truncate the serial number or zero-fill it
//                on the right, as necessary, to make a full String16 value.
//

static void  EvcQHCommonSN (String16& SerialNumber, const CmbPacket *pPkt)
{

U32   cbCopy;


   assert (pPkt->Hdr.cbData <= sizeof (SerialNumber));

   //  find max(packet data size, s/n var size)
   cbCopy = pPkt->Hdr.cbData;
   if (cbCopy > sizeof (SerialNumber))
      {
      cbCopy = sizeof (SerialNumber);
      }

   //  if we have any packet data, copy it over
   if (cbCopy > 0)
      {
      memcpy (SerialNumber, pPkt->abTail, cbCopy);
      }

   //  fill rest of s/n var with nice tidy zeros
   if (cbCopy < sizeof (SerialNumber))
      {
      memset (SerialNumber, 0, sizeof (SerialNumber) - cbCopy);
      }

   return;

}  /* end of EvcQHCommonSN */
#endif  // #ifdef NEED_SERIAL_NUM_COPY




