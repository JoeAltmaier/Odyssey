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
// File: CtMessages.cpp
// 
// Description:
// CtMessages.cpp
// This file contains the implementations of classes CCtMessageDb
// (language-specific text database) and CCtMsgDisp (the renderer).
// 
// $Log: /Gemini/Odyssey/Message Text/CtMessages.cpp $
// 
// 6     12/16/99 12:59a Ewedel
// Added support for parameterized extraction to CtStringT<> derivatives,
// which then enables unicode & ascii<->unicode conversion support.
// [Renderer foundation is now Unicode-based.]
// 
// 5     11/15/99 6:27p Ewedel
// Worked around limitation of old StringClass code.
// 
// 4     11/15/99 6:08p Ewedel
// Changed to use new CtAString class.
// 
// 3     11/10/99 8:09p Ewedel
// Added overload of CCtMsgView::GetMessageText() which does parameter
// substitution into a StringClass accumulator.
// 
// 2     7/12/99 6:07p Ewedel
// Made various silly bug fixes, and added needed destructor.
// 
// 1     6/30/99 4:50p Ewedel
// Initial checkin.
// 
/*************************************************************************/


#include  "CtMessages.h"

#include  "CtString.h"

#include  "CtEvent.h"

#include  "Event.h"           // for real class Event defs

#include  <stdio.h>           // gets us NULL, etc.
#include  <stdlib.h>          // for atol(), etc.
#include  <ctype.h>           // for isdigit()

#include  <assert.h>



//  here's the global/static list of available message languages
CCtMessageDb * CCtMessageDb::ms_pLangList  =  NULL;

//  helper for expanding one event parameter as text
static void  ExpandParam (CtString& strBuf, const Event& evtMessageData,
                          U32 iParam);


//
//  CCtMsgView::CCtMsgView (eLangId)
//
//  Description:
//    Constructs an instance of our message viewing helper class.
//
//  Inputs:
//    eLangID - Optional ID of the language whose database we're
//                to use for message text.  Value should be something
//                generated by the NT MAKELANGID() macro or equivalent.
//                For example, English would code as 0x0409, or in NT:
//                MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT).
//
//  Outputs:
//    none
//

CCtMsgView::CCtMsgView (CtLanguageId eLangId /* = 0 */ )
{


   //  establish some semi-safe defaults
   m_pViewLang = NULL;
   m_eViewLangId = k_eLangDefault;

   //  implicitly do a "nearest" match on language ID
   SetViewLanguage (eLangId, k_eNearest);      // (never fails, unless English
                                               //  is unavailable)

   return;

}  /* end of CCtMsgView::CCtMsgView */

//
//  CCtMsgView::SetViewLanguage (eLangId, eTolerance)
//
//  Description:
//    Sets our viewer instance's current choice of language for
//    translating messages.
//
//    The caller can control how precisely their supplied language
//    ID must match one of our text databases in order for the set
//    operation to be successful [see eTolerance, below].
//
//  Inputs:
//    eLangId - ID of the language whose database we're to use for
//                translating message text.  Value may be something
//                generated by the NT MAKELANGID() macro or equivalent.
//                For example, English would code as k_eLangEnglish, or
//                in NT:  MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT).
//    eTolerance - Indicates how close of a match the user requires for
//                their supplied eLangId.  This is an issue since the
//                available language set may be subject to change.
//                At our default (loosest) setting, we'll attempt an exact
//                match of eLangId, but will settle for a match in only
//                the primary language part, or will fall back to English
//                if even a loose match can't be found.
//                The caller may also specify more restrictive match
//                criteria; refer to the LangMatchTolerance enum for more.
//
//  Outputs:
//    CCtMsgView::SetViewLanguage - Returns CTS_SUCCESS, or some sort
//                of deeply meaningful error code.
//

STATUS  CCtMsgView::SetViewLanguage (CtLanguageId eLangId,
                                     LangMatchTolerance eTolerance
                                                         /* = k_eNearest*/)
{

CCtMessageDb * pCurLangDb;
CCtMessageDb * pExactMatch;
CCtMessageDb * pPrimaryMatch;
CCtMessageDb * pEnglish;
STATUS         ulMyRet;


   pExactMatch   = NULL;
   pPrimaryMatch = NULL;
   pEnglish      = NULL;

   //  zip through the list of known languages, looking for an exact match
   for (pCurLangDb = CCtMessageDb::ms_pLangList;
        pCurLangDb != NULL;
        pCurLangDb = pCurLangDb -> m_pNextLanguage)
      {
      if (pCurLangDb->m_ulLangId == eLangId)
         {
         //  got exact match, save it away
         pExactMatch = pCurLangDb;
         }

      if (CT_LANG_PRIMARY_ID ((CtLanguageId) pCurLangDb->m_ulLangId) ==
                  CT_LANG_PRIMARY_ID (eLangId))
         {
         //  got a match in primary language, save it too
         pPrimaryMatch = pCurLangDb;
         }

      if (pCurLangDb->m_ulLangId == k_eLangEnglish)
         {
         //  found English version, save it too
         pEnglish = pCurLangDb;
         }
      }

   //  now figure out if we got a good enough match

   ulMyRet = CTS_SUCCESS;     // a happy default :-)

   if (pExactMatch != NULL)
      {
      //  we're cool, save it away
      m_pViewLang = pExactMatch;
      m_eViewLangId = eLangId;
      }
   else if ((pPrimaryMatch != NULL) &&
            ((eTolerance == k_ePrimaryOnly) || (eTolerance == k_eNearest)))
      {
      //  got a primary-only match, and caller says that's good enough
      m_pViewLang = pPrimaryMatch;
      m_eViewLangId = (CtLanguageId) m_pViewLang->m_ulLangId;
      }
   else if ((pEnglish != NULL) && (eTolerance == k_eNearest))
      {
      //  got no match to speak of, but caller will accept "default"
      m_pViewLang = pEnglish;
      m_eViewLangId = (CtLanguageId) m_pViewLang->m_ulLangId;
      }
   else
      {
      //  no acceptable match, flame out
      ulMyRet = CTS_UNSUPPORTED_LANGUAGE;

      //  this should never happen if we're on our loosest setting
      //  (i.e., we should always have at least English available)
      assert (eTolerance != k_eNearest);
      }

   return (ulMyRet);

}  /* end of CCtMsgView::SetViewLanguage */

//
//  CCtMsgView::GetMessageText (pszText, ulEventCode)
//
//  Description:
//    Given a simple event code with no parameters, we return the
//    code's message text in our current language.
//
//  Inputs:
//    ulEventCode - Event code whose message text we're to return.
//
//  Outputs:
//    pszText - Loaded with message text for ulEventCode.
//    CCtMsgView::GetMessageText - Returns CTS_SUCCESS, or an error code
//                if the given ulEventCode has no associated message text,
//                or we have no language set.
//

STATUS  CCtMsgView::GetMessageText (const char *&pszText, STATUS ulEventCode)
{

const char   * pszMessage;
STATUS         sMyRet;


   //  search for the requested string
   if (FindMessage (ulEventCode, pszMessage, sMyRet))
      {
      //  found a matching message, simply return it
      pszText = pszMessage;
      }

   return (sMyRet);

}  /* end of CCtMsgView::GetMessageText */

//
//  CCtMsgView::GetMessageText (strMessageText, evtMessageData)
//
//  Description:
//    This overload of GetMessageText maps a given Event class instance
//    into its equivalent message text.  If the message text includes
//    formal parameter substitution markers, then we expand those using
//    the parameters supplied within the Event instance.
//
//    *  This routine is the real message lookup enchilada.
//
//  Inputs:
//    evtMessageData - Event instance containing both a basic event code,
//                and any parameters associated with the event.
//
//  Outputs:
//    strMessageText - Loaded with text of event's message, with
//                substitution parameter markers expanded with whatever
//                parameters are available in the supplied event instance.
//    CCtMsgView::GetMessageText - Returns CTS_SUCCESS, or an error code
//                if the given ulEventCode has no associated message text,
//                or we have no language set.  Note that we do not return
//                an error if the event instance has insufficient parameters
//                for the given message text.  We just do the best we can,
//                and leave the extra slots in the message text empty.
//

STATUS  CCtMsgView::GetMessageText (CtString& strMessageText,
                                    const Event& evtMessageData)
{

const char   * pszMessage;
CtAString      strNum;        // buffer for doing numeric conversions
STATUS         sMyRet;
const char     chMcEscape = '%';
char           ch;
const char   * pch;
U32            iParam;


   //  clear our output message buffer
   strMessageText.Clear ();

   //  search for the requested string
   if (! FindMessage (evtMessageData.GetEventCode(), pszMessage, sMyRet))
      {
      //  whoops, basic text lookup failed.  Give up here.
      return (sMyRet);
      }

   //  resize our output buf to handle at least the literal size
   //  of our message text
   strMessageText.Resize (strlen (pszMessage));

   //  got message text, copy it into buffer, doing parameter expansion
   //  as needed

   while (*pszMessage != NULL)
      {
      ch = *pszMessage;

      if (ch != chMcEscape)
         {
         //  got a simple char, just add it to our message buffer
         strMessageText += ch;
         pszMessage ++;
         }
      else
         {
         //  got an escape char - what is it escaping?
         ch = pszMessage[1];

         if (isdigit (ch))
            {
            //  got a substitution param

            //  first grab whole number.
            pch = pszMessage + 2;
            while (isdigit (*pch))
               {
               pch ++;
               }
            strNum.Assign(pszMessage + 1, pch - (pszMessage + 1));

            //  compute zero-based param index
            //  (note that message substitution param indices are one-based)
            iParam = atol (strNum) - 1;

            //  expand parameter from Event instance into text
            ExpandParam (strMessageText, evtMessageData, iParam);

            //  skip over parameter formal
            pszMessage = pch;
            }
         else
            {
            //  not a substitution param, what else can it be?
            switch (ch)
               {
               case chMcEscape:
                  //  escaping a literal escape char, so copy it across
                  strMessageText += ch;
                  pszMessage += 2;
                  break;

               case '.':
                  //  if an escaped '.' occurs at end-of-line, then we want
                  //  to suppress the following EOL
                  if (pszMessage[2] == '\n')
                     {
                     //  got the magic sequence, so skip '.' and '\n':
                     pszMessage += 3;
                     }
                  else
                     {
                     //  not the magic sequence, so treat escaped '.' as literal:
                     pszMessage ++;
                     }
                  break;

               default:
                  //  no other escaped chars are supported, they all are
                  //  treated as simple literals:
                  pszMessage ++;
               }
            }
         }

      } /* end of while (more chars in message template) */

   return (CTS_SUCCESS);

}  /* end of CCtMsgView::GetMessageText (CtString& str, const Event& evt) */


//  down-convert to ASCII
STATUS  CCtMsgView::GetMessageText (CtAString& strMessageText,
                                    const Event& evtMessageData)
{

CtString    strBuf;
STATUS      sMyRet;


   sMyRet = GetMessageText (strBuf, evtMessageData);
   if (sMyRet == CTS_SUCCESS)
      {
      strMessageText.AssignFromUnicode (strBuf);
      }
   else
      {
      //  whoopsie, lookup failed (illegal event code?)
      strMessageText.Clear ();
      }

   return (sMyRet);

}  /* end of CCtMsgView::GetMessageText (CtAString& str, const Event& evt) */


//  down-convert to SSAPI-style string instance (also ASCII)
STATUS  CCtMsgView::GetMessageText (StringClass& strMessageText,
                                    const Event& evtMessageData)
{

CtAString   strBuf;
STATUS      sMyRet;


   sMyRet = GetMessageText (strBuf, evtMessageData);
   if (sMyRet == CTS_SUCCESS)
      {
      //  (StringClass isn't const-strengthened, *sigh*)
      strMessageText = (char *) (const char *) strBuf;
      }
   else
      {
      //  whoopsie, lookup failed (illegal event code?)
      strMessageText = "";
      }

   return (sMyRet);

}  /* end of CCtMsgView::GetMessageText (StringClass& str, const Event& evt) */


//
//  CCtMsgView::FindMessage (ulEventCode, pszMessage, ulResult)
//
//  Description:
//    An internal helper routine.  We verify that our instance presently
//    has a valid language selected, and then search that language's
//    message database for the given event code.  Assuming a match is found
//    we return a pointer to the matching message's text.
//
//  Inputs:
//    ulEventCode - Event code whose message text we're to return.
//
//  Outputs:
//    pszMessage - Set to point to desired message's text, if found.
//                Set to the empty string otherwise.
//    ulResult - Set to CTS_SUCCESS if we find a matching message, or
//                to a suitable error code otherwise.
//    CCtMsgView::FindMessage - Returns TRUE if we return a valid message,
//                or FALSE otherwise (when ulResult != CTS_SUCCESS).
//

BOOL  CCtMsgView::FindMessage (STATUS ulEventCode,
                               const char *& pszMessage, STATUS& ulResult) const
{

U32                              iFirst;
U32                              iLast;
U32                              iCur;
U32                              ulCurEvent;    // STATUS, but must be unsigned
const CCtMessageDb::MsgEntry   * pMsgList;


   if (m_pViewLang == NULL)
      {
      //  tsk, no language set.  This should *never* happen.
      assert (m_pViewLang != NULL);
      ulResult = CTS_UNSUPPORTED_LANGUAGE;
      }
   else
      {
      //  got a language database, search for the given event code
      assert (m_pViewLang->m_pMessageEntries != NULL);
      assert (m_pViewLang->m_cMessageEntries > 0);

      //  set up window for binary search
      iFirst = 0;
      iLast = m_pViewLang->m_cMessageEntries - 1;

      pMsgList = m_pViewLang->m_pMessageEntries;

      while (iFirst <= iLast)
         {
         //  find mid-point
         iCur = (iFirst + iLast) / 2;

         ulCurEvent = pMsgList[iCur].ulMsgId;

         //  NOTE:  Our message table is sorted in increasing event code order,
         //         assuming that event codes are treated as unsigned numbers.
         if ((U32) ulEventCode < ulCurEvent)
            {
            //  current midpoint too high, move window down
            iLast = iCur - 1;
            }
         else if ((U32) ulEventCode > ulCurEvent)
            {
            //  current midpoint too low, move window up
            iFirst = iCur + 1;
            }
         else
            {
            //  got an exact match, we're happy :-)
            pszMessage = pMsgList[iCur].pszMsg;
            ulResult = CTS_SUCCESS;
            break;
            }
         }

      if (iFirst > iLast)
         {
         //  aww, no match
         ulResult = CTS_INVALID_MESSAGE_ID;
         }
      }

   //  all done, report our result in an easy-to-digest way
   return (ulResult == CTS_SUCCESS);

}  /* end of CCtMsgView::FindMessage */

//
//  CCtMessageDb::CCtMessageDb (ulLangID, pMsgList, cMessages)
//
//  Description:
//    Constructs a message text language database instance.
//    We gather up and stash our construct-time parameters,
//    the language ID and list of message strings.
//
//    We then add our instance to the global (static) list of
//    language database instances maintained by our class.
//    [This list is used by the event rendering code to find
//     the proper language's message text.]
//
//  Inputs:
//    ulLangID - ID of this message database instance's language.
//    pMsgList - Points to sorted array of message text entries.
//    cMessages - One-based count of entries in *pMsgList.
//
//  Outputs:
//    none
//

CCtMessageDb::CCtMessageDb (U32 ulLangID, const MsgEntry *pMsgList,
                            U32 cMessages) :
                                 m_ulLangId (ulLangID),
                                 m_pMessageEntries (pMsgList),
                                 m_cMessageEntries (cMessages)
{


   //  verify (after the fact) that our input params are ok
   assert (ulLangID != 0);
   assert (pMsgList != NULL);
   assert (cMessages > 0);

   //  initialized our instance data above, now hook our instance
   //  into the master list of languages
   m_pNextLanguage = ms_pLangList;
   ms_pLangList = this;

   return;

}  /* end of CCtMessageDb::CCtMessageDb */

//
//  CCtMessageDb::~CCtMessageDb ()
//
//  Description:
//    Language database instance destructor.
//
//    We do nothing, and are here to make that perfectly clear.
//    In particular, we do *not* attempt to destroy the supplied
//    database table, as it is likely allocated at global scope.
//
//  Inputs:
//    none
//
//  Outputs:
//    none
//

CCtMessageDb::~CCtMessageDb ()
{


   //  aught to do
   return;

}  /* end of CCtMessageDb::~CCtMessageDb */

//
//  ExpandParam (strBuf, evtMessageData, iParam)
//
//  Description:
//    Translates a selected parameter from the given event instance
//    into text form, and appends it to the supplied string instance.
//
//    If the given parameter index doesn't select a valid parameter
//    in the given event instance, then we simply do nothing.
//
//  Inputs:
//    strBuf - String instance which we append to.
//    evtMessageData - Event instance containing parameter which we
//                expand (and maybe others, too).
//    iParam - Zero-based index of param in evtMessageData which
//                we expand onto tail of strBuf.
//
//  Outputs:
//    strBuf - Textual form of param[iParam] from evtMessageData
//                is stuck on the end.
//

//  a little helper for param conversion
template <class T>  const T& ToVal (const void *pvVal)
         {  return (*(const T *) pvVal);  };


static void  ExpandParam (CtString& strBuf, const Event& evtMessageData,
                          U32 iParam)
{

Event::ParmType   eParamType;
const void      * pvParam;
U32               cbParam;
I64               i64Value;   // for 64-bit values
U32               ulValue;    // for 32-bit or smaller values
char              szNum [32]; // big enough for signed 64-bit numbers
const U8        * pbParam;
U32               cBytesOnLine;
const U32         cBytesOnLineMax = 16;      // each byte uses three chars



   if (evtMessageData.GetParameterCount() <= iParam)
      {
      //  no matching param, so bail out now.
      return;
      }

   //  got a param, grab its type & raw pointer
   eParamType = evtMessageData.GetParameterType(iParam);
   pvParam    = evtMessageData.GetPParameter(iParam);
   cbParam    = evtMessageData.GetParameterSize(iParam);

   if (pvParam == NULL)
      {
      //  nothing to expand
      return;
      }

   //  got a param, expand it as appropriate

   //  first, normalize integer types of params
   switch (eParamType)
      {
      case Event::S8_PARM:
         ulValue = ToVal<S8> (pvParam);
         break;

      case Event::U8_PARM:
         ulValue = ToVal<U8> (pvParam);
         break;

      case Event::U16_PARM:
         ulValue = ToVal<U16> (pvParam);
         break;

      case Event::S16_PARM:
         ulValue = ToVal<S16> (pvParam);
         break;

      case Event::S32_PARM:
         ulValue = ToVal<S32> (pvParam);
         break;

      case Event::U32_PARM:
         ulValue = ToVal<U32> (pvParam);
         break;

      case Event::S64_PARM:
         i64Value = ToVal<I64> (pvParam);
         break;

      case Event::U64_PARM:
         i64Value = ToVal<I64> (pvParam);
         break;
      }

   //  now expand per argument type
   switch (eParamType)
      {
      case Event::CHAR_PARM:
         strBuf += ToVal<char> (pvParam);
         break;

      case Event::STR_PARM:
         //  up-convert from ASCII to Unicode
         strBuf.AppendFromAscii ((const char *) pvParam);
         break;

      case Event::USTR_PARM:
         //  what a concept, a simple append!
         strBuf += (const wchar_t *) pvParam;
         break;

      case Event::S8_PARM:
      case Event::S16_PARM:
      case Event::S32_PARM:
         //  signed values, normalized to 32 bits
         sprintf (szNum, "%d", ulValue);
         strBuf.AppendFromAscii (szNum);
         break;

      case Event::U8_PARM:
      case Event::U16_PARM:
      case Event::U32_PARM:
         //  unsigned values, normalized to 32 bits
         sprintf (szNum, "%u", ulValue);
         strBuf.AppendFromAscii (szNum);
         break;

      case Event::S64_PARM:
         sprintf (szNum, "%Ld", i64Value);
         strBuf.AppendFromAscii (szNum);
         break;

      case Event::U64_PARM:
         sprintf (szNum, "%Lu", i64Value);
         strBuf.AppendFromAscii (szNum);
         break;

      case Event::HEX_PARM:
         //  arbitrary binary data.  This is likely large, or else the
         //  event definer would have chosen a different type.
         //  So for now, we always do a sort of "hex dump" display,
         //  which sticks stuff on multiple lines.
         cBytesOnLine = cBytesOnLineMax;     // trigger initial newline
         pbParam = (const U8 *) pvParam;
         while (cbParam > 0)
            {
            if (cBytesOnLine >= cBytesOnLineMax)
               {
               //  wrap to a new line
               strBuf += L"\n";
               cBytesOnLine = 0;
               }

            sprintf (szNum, " %02X", *pbParam);
            strBuf.AppendFromAscii (szNum);

            cBytesOnLine ++;
            cbParam --;
            }
         break;
      }

   return;

}  /* end of ExpandParam */


