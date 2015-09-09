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
// File: CtMessages.h
// 
// Description:
// Contains the class definitions used to encapsulate message compiler
// output data (the message database) for compiled-in use with Odyssey
// images & tools.
// This same class (CCtMessages) provides an interface for rendering
// message codes and optional parameters into display strings.
// 
// $Log: /Gemini/Include/CtMessages.h $
// 
// 5     12/16/99 12:54a Ewedel
// Added support for parameterized extraction to CtStringT<> derivatives,
// which then enables unicode & ascii<->unicode conversion support.
// 
// 4     10/27/99 12:15p Ewedel
// Added prototype for GetMessageText() overload which does parameter
// substitution (starts from an Event instance).
// 
// 3     7/15/99 4:33p Ewedel
// Fixed bug in CT_ST_SEVERITY() macro.
// 
// 2     7/12/99 11:43a Ewedel
// Moved language ID enum into CtIsoCodes.h, and added some more field
// extractors for status codes.
// 
// 1     6/30/99 4:35p Ewedel
// Initial checkin.
// 
/*************************************************************************/

#ifndef _CtMessages_h_
#define _CtMessages_h_


#ifndef _CtIsoCodes_h_
# include  "CtIsoCodes.h"
#endif

#ifndef OsTypes_H
# include  "OsTypes.h"
#endif

#ifndef _CtString_h_
# include  "CtString.h"          // more efficient, converts ascii<->unicode
#endif

#ifndef	 __STRING_CLASS_H__
# include  "StringClass.h"       // less efficient, used extensively in SSAPI
#endif


//* the Event.h include screws up compilation, for some reason.
//* So we simply forward-def class Event below, instead.
//*#ifndef _Event_h_
//*# include  "Event.h"
//*#endif


//  here's a little helper for extracting just the primary language
//  part of a language ID (see CtIsoCodes.h)
inline U32 CT_LANG_PRIMARY_ID (CtLanguageId eLangId)
               {  return (eLangId & 0x3F);  };

//  extract "facility" part (CTS_FACILITY_...) of a status code
inline U32 CT_ST_FACILITY_ID (STATUS ulStatus)
               {  return ((ulStatus >> 16) & 0xFFF);  };

//  extract "severity" code (STATUS_SEVERITY_...) from a status code
inline U32 CT_ST_SEVERITY (STATUS ulStatus)
               {  return ((ulStatus >> 30) & 3);  };

//  extract detailed event code from status code: the detail code
//  does not include the facility code or other fields.  The same detail
//  code may occur in multiple facility definitions.  We hope (but do not
//  guarantee) that all facilities will share a common detail code map.
inline U32 CT_ST_DETAIL_ID (STATUS ulStatus)
               {  return (ulStatus & 0xFFFF);  };



//  we need to forward-declare our language database wrapper class
class CCtMessageDb;


//  and here's a forward ref for parameter use
class Event;            // event objects (oos\Event.h)


//  here's the class which is used to "render" an event code
//  (from CtEvent.mc / .h) into a legible text string.
class CCtMsgView
{
public:

   //  our constructor
   CCtMsgView (CtLanguageId ulLangId = k_eLangDefault);

   //  a little parameter enum to indicate how tight language ID matching is
   enum LangMatchTolerance {
      k_eExact = 1,        // language ID must match exactly
      k_ePrimaryOnly,      // language ID's "primary" language ID must match,
                           //  sublanguage may differ
      k_eNearest           // find nearest match, fall back to English if
                           //  no better match is available.
   };

   //  you can set/change language IDs on the fly
   STATUS  SetViewLanguage (CtLanguageId ulLangId,
                            LangMatchTolerance eTolerance = k_eNearest);

   //  and, just for fun, you can read what our current language is
   inline CtLanguageId  GetViewLanguage (void)  const
                  {  return (m_eViewLangId);  };

   //  we have various flavors of rendering:

   //  do simple extraction of a message's text, without param insertion
   STATUS  GetMessageText (const char *&pszText, STATUS ulEventCode);

   //  here's one which can do parameter substitution.  This maps from
   //  an Event instance (which may contain Unicode parameters) into a
   //  CtStringT<wchar_t> instance.
   STATUS  GetMessageText (CtString& strMessageText,
                           const Event& evtMessageData);

   //  this is just like the foregoing, but it maps the Event instance
   //  into a CtStringT<char> instance instead.  If non-ASCII Unicode
   //  chars are present they are down-converted to ASCII by "greeking"
   //  (replacing with '?' chars).  In the future, this may also
   //  support converting to the Unicode UTF-8 representation
   //  (refer to RFC 2279 for more information about this encoding).
   STATUS  GetMessageText (CtAString& strMessageText,
                           const Event& evtMessageData);

   //  This is just like the preceding two, except that it maps
   //  into a StringClass, which is a somewhat less-friendly, but
   //  more widely used string implementation.
   //  Note that this version is ANSI only; neither Event nor
   //  StringClass support Unicode at this time.
   STATUS  GetMessageText (StringClass& strMessageText,
                           const Event& evtMessageData);


private:

   //  what language we're supposed to render in
   CtLanguageId   m_eViewLangId;

   //  pointer to the best-fit language database for our current LangId
   CCtMessageDb * m_pViewLang;

   //  a helper routine for looking up a given message
   BOOL  FindMessage (STATUS ulEventCode, const char *& pszMessage,
                      STATUS& ulResult)  const;

};  /* end of class CCtMsgView */


//  what each language's message text is encapsulated in:
class CCtMessageDb
{

friend CCtMsgView;      // who pokes around to find/use our language DBs

public:

   //  what our input database rows look like
   typedef struct {
      U32            ulMsgId;    // ID of message
      const char   * pszMsg;     // text of message (NULL -> end of list)
   } MsgEntry;

   CCtMessageDb (U32 ulLangID, const MsgEntry *pMsgList, U32 cMessages);

   ~CCtMessageDb ();

private:

   //  we maintain a list of all known language databases
   //  (each is registered by its constructor)
   static CCtMessageDb * ms_pLangList;

   //  here is the per-language database info

   //  link to next language's instance in our master list
   CCtMessageDb    * m_pNextLanguage;

   //  ID of this instance's language
   U32               m_ulLangId;

   //  pointer to sorted array of message strings for this language
   const MsgEntry  * const m_pMessageEntries;

   //  count of entries in *m_pMessageEntries
   const U32         m_cMessageEntries;

};  /* end of CCtMessageDb */


#endif  /* #ifndef _CtMessages_h_ */

