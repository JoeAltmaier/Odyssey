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
// File: CtString.h
//
// Description:
//    Contains the definition of a simple, versatile, const-strong string
//    template class.  Both Unicode and ASCII derivatives are typedef'd
//    in this file:
//       typedef CtStringT<char>    CtAString;
//       typedef CtStringT<wchar_t> CtString;
//
//    Each has the following methods available (Chr is the name of
//    the string's underlying character type, either char or wchar_t):
//       CtStringT (const Chr * pValue = NULL)
//       CtStringT (const CtStringT& )
//       (const Chr *)   [cast operator, shows string's internal buffer]
//       Length ()       [count of Chr's, *not* bytes, in present value]
//       operator = (const Chr *pRhs)
//       operator = (const CtStringT& str)
//       Assign (const Chr *pRhs, U32 cchValue)    [similar to strncpy()]
//       AssignFromAscii (const char *pchValue, cchValue) [ASCII->Unicode strncpy()]
//       AssignFromUnicode (const wchar_t *pwchValue, cwchValue)
//                                                 [Unicode->ASCII strncpy()]
//       operator += (Chr chValue)
//       operator += (const Chr *pValue)
//       operator += (const CtStringT& str)
//       Append (const Chr *pRhs, U32 cchValue)    [similar to strncat()]
//       AppendFromAscii (const char *pchValue, cchValue) [ASCII->Unicode strncat()]
//       AppendFromUnicode (const wchar_t *pwchValue, cwchValue)
//                                                 [Unicode->ASCII strncat()]
//       Fill (Chr chValue, U32 cchFill)
//       ultoa (U32 ulValue)
//       itoa (S32 sValue)
//       Resize (cchNewBuf)      [force string buffer to given *minimum* size]
//       Clear ()                [set to empty value, does *not* free buffer]
//
//    and a full complement (pardon the expression ;-) of relational ops:
//       ==, !=, <, <=, >, >=
//    for CtStringT LHSes, and CtStringT / (const Chr *) RHSes.
//
//
// $Log: /Gemini/Include/CtString.h $
// 
// 2     12/16/99 1:02a Ewedel
// General cleanup, added Unicode conversion support, slightly increased
// repertoire of available operations.
// 
// 1     11/15/99 6:08p Ewedel
// Initial revision.
// 
/*************************************************************************/

#ifndef _CtString_h_
#define _CtString_h_



#ifndef Simple_H
# include "Simple.h"
#endif

# include <string.h>

#ifndef WIN32
// Metrowerks
# ifndef __wchar_t__
//#  include  "wchar_t.h"
# endif
#else  // #ifndef WIN32
# if !defined( _BASETYPS_H_ )
#  include  <BaseTyps.h>
# endif
#endif  // #ifndef WIN32

#ifndef assert
# include  <assert.h>
#endif



template <class Chr> class CtStringT
{
private:

   //  points to our internal buffer, or to a dummy if we have heap trouble
   Chr   * m_pString;

   //  count of chars present in *m_pString, excluding trailing EOS.
   U32   m_cchCur;

   //  how many chars our current *m_pString buffer can hold, excl. EOS
   U32   m_cchBufMax;


public:

   inline CtStringT (const Chr *pChInitialValue = NULL)
      {  m_cchBufMax = 0;
         *this = pChInitialValue;
      };


   inline CtStringT (const CtStringT &InitialValue)
      {  m_cchBufMax = 0;
         *this = InitialValue;
      };


   ~CtStringT ();


   //  return a pointer to our internal string data.  Guaranteed never
   //  to be NULL, even if the heap is messed up.  [Of course, it may
   //  return an empty string.]
   //  The caller MUST NOT FREE the returned memory!
   inline operator const Chr * (void)  const
         {  return (m_pString);  };

   //  length of our current string value, not counting the trailing EOS
   inline U32  Length (void)  const
         {  return (m_cchCur);  };


   //  assign from an instance of the same type
   inline CtStringT&  operator = (const CtStringT& strRhs)
         {  Assign (strRhs.m_pString, strRhs.m_cchCur);
            return (*this);
         };

   //  assign from an EOS-terminated string
   inline CtStringT&  operator = (const Chr *pchRhs)
         {  Assign (pchRhs);
            return (*this);
         };

//*   //  assign from an EOS-terminated ASCII string
//*   inline CtStringT&  operator = (const char *pszRhs)
//*         {  Assign (pszRhs, ::strlen (pszRhs));
//*            return (*this);
//*         };
//*   inline 

   //  generic assignment of a counted char sequence [aka, strncpy()]
   inline void  Assign (const Chr *pchValue, U32 cchValue = -1)
   {  Clear ();
      Append (pchValue, cchValue);
   };

   //  assign from ASCII text, regardless of our Chr type
   inline void  AssignFromAscii (const char *pchValue, U32 cchValue = -1)
   {  Clear ();
      AppendFromAscii (pchValue, cchValue);
   };

   //  assign from Unicode text, regardless of our Chr type
   inline void  AssignFromUnicode (const wchar_t *pwchValue, U32 cwchValue = -1)
   {  Clear ();
      AppendFromUnicode (pwchValue, cwchValue);
   }

   //  append a single Chr character to our value
   //  (char should transparently cast to Chr here, for ASCII->Unicode use)
   CtStringT&  operator += (Chr chValue);

   //  append a simple string to our value
   inline CtStringT&  operator += (const Chr *pchValue)
         {  Append (pchValue);
            return (*this);  };

   //  append one of our own instances to our value
   inline CtStringT&  operator += (const CtStringT& strValue)
         {  Append (strValue.m_pString, strValue.m_cchCur);
            return (*this);  };

   //  do counted append to our instance
   void  Append (const Chr *pchValue, U32 cchValue = -1);

   //  do counted append from ASCII string to our Chr[] value
   void  AppendFromAscii (const char *pchValue, U32 cchValue = -1);

   //  do counted append from Unicode string to our Chr[] value
   void  AppendFromUnicode (const wchar_t *pwchValue, U32 cwchValue = -1);

   //  replace our value with a sequence of cchFill chFill chars:
   void  Fill (Chr chFill, U32 cchFill);

   //  replace our value with the given unsigned integer, in text form
   void  ultoa (U32 ulValue)
   {
      char  achNum [16];

      sprintf (achNum, "%u", ulValue);
      AssignFromAscii (achNum);
   }

   //  replace our value with the given signed integer, in text form
   void  itoa (S32 sValue)
   {
      char  achNum [16];

      sprintf (achNum, "%d", sValue);
      AssignFromAscii (achNum);
   }

   //  resize our internal buffer - count is of char items, *not* bytes.
   //  [Buffer size only grows up, never down.]
   BOOL  Resize (U32 cchNewBufMax);

   //  clear our instance's value to "".  Does not release its buffer, if any.
   void  Clear (void);


   //  relationals, where RHS is another CtStringT instance

   inline BOOL operator == (const CtStringT &strRhs)  const
         {  return (strcmp (m_pString, strRhs.m_pString) == 0);  };

   inline BOOL operator != (const CtStringT &strRhs)  const
         {  return (strcmp (m_pString, strRhs.m_pString) != 0);  };

   inline BOOL operator <  (const CtStringT &strRhs)  const
         {  return (strcmp (m_pString, strRhs.m_pString) < 0);  };

   inline BOOL operator <= (const CtStringT &strRhs)  const
         {  return (strcmp (m_pString, strRhs.m_pString) <= 0);  };

   inline BOOL operator >  (const CtStringT &strRhs)  const
         {  return (strcmp (m_pString, strRhs.m_pString) > 0);  };

   inline BOOL operator >= (const CtStringT &strRhs)  const
         {  return (strcmp (m_pString, strRhs.m_pString) >= 0);  };


   //  relationals, where RHS is a (Chr *) EOS-terminated string

   inline BOOL operator == (const Chr *pchRhs)  const
         {  return (strcmp (m_pString, pchRhs) == 0);  };

   inline BOOL operator != (const Chr *pchRhs)  const
         {  return (strcmp (m_pString, pchRhs) != 0);  };

   inline BOOL operator <  (const Chr *pchRhs)  const
         {  return (strcmp (m_pString, pchRhs) < 0);  };

   inline BOOL operator <= (const Chr *pchRhs)  const
         {  return (strcmp (m_pString, pchRhs) <= 0);  };

   inline BOOL operator >  (const Chr *pchRhs)  const
         {  return (strcmp (m_pString, pchRhs) > 0);  };

   inline BOOL operator >= (const Chr *pchRhs)  const
         {  return (strcmp (m_pString, pchRhs) >= 0);  };


private:

   //  a char analogue of strlen():
   static U32  strlen (const Chr *pchValue);

   //  a char analogue of strncpy():
   static void  strncpy (Chr *pchDest, const Chr *pchSrc, U32 cchMax);

   //  a char analogue of strcmp():
   static S32  strcmp (const Chr *pchLhs, const Chr *pchRhs);

   //  our standard end-of-string sentinel, included for readability
   static const Chr    chEOS;

};  /* end of class template <class Chr> class CtStringT */


template <class Chr>
const Chr CtStringT<Chr>::chEOS  =  0;



//
//  CtStringT::~CtStringT ()
//
//  Description:
//    Our destructor.  Frees any allocated memory, and renders our
//    internal pointers harmless (just in case somebody tries to use
//    our destroyed instance).
//    
//  Inputs:
//    none
//
//  Outputs:
//    none
//

template <class Chr>
CtStringT<Chr>::~CtStringT ()
{


   //  [can't check m_pString itself, since we keep it non-NULL for
   //   ease of (const Ch *) casting.]
   if (m_cchBufMax > 0)
      delete [] m_pString;

   m_pString   = NULL;
   m_cchCur    = 0;
   m_cchBufMax = 0;

   return;

}  /* end of CtStringT<Chr>::~CtStringT */

//
//  CtStringT::operator += (Chr chValue)
//
//  Description:
//    Appends a single char to our instance.  Returns a reference
//    to our instance.
//
//  Inputs:
//    chValue - char to append to our instance's value.
//
//  Outputs:
//    none
//

template <class Chr>
CtStringT<Chr>&  CtStringT<Chr>::operator += (Chr chValue)
{


   if (m_cchCur >= m_cchBufMax)
      {
      //  no room for new char, got to resize
      if (! Resize (m_cchCur + 1))
         {
         //  whoops, do nothing
         return (*this);
         }
      }

   assert (m_cchCur < m_cchBufMax);

   m_pString [m_cchCur] = chValue;
   m_cchCur ++;
   m_pString [m_cchCur] = chEOS;

   return (*this);

}  /* end of CtStringT::operator += (char chValue) */

//
//  CtStringT::Append (pchValue, cchValue)
//
//  Description:
//    Appends a text string to our instance.  We make a copy of the
//    supplied original, resizing our internal buffer if necessary.
//
//    We stop the append at the soonest of 1) finding an EOS in the
//    supplied value, or 2) appending as many chars as specified by
//    the supplied count.
//    
//  Inputs:
//    pchValue - Points to string value to append to our instance.
//    cchValue - One-based count of chars in *pchValue which we
//                append to our instance.  If *pchValue has an EOS
//                before we reach this count, we stop at the EOS instead.
//
//  Outputs:
//    none
//

template <class Chr>
void  CtStringT<Chr>::Append (const Chr *pchValue, U32 cchValue /* = -1 */ )
{

U32   cchActual;


   if (pchValue == NULL)
      {
      return;
      }

   //  take care of possible cases where cchValue is -1 or otherwise
   //  not quite right.  But allow cchValue to truncate *pchValue.
   cchActual = strlen (pchValue);
   if (cchValue > cchActual)
      {
      cchValue = cchActual;
      }

   if ((m_cchCur + cchValue) > m_cchBufMax)
      {
      //  no room for new string, got to resize
      if (! Resize (m_cchCur + cchValue))
         {
         //  whoops, do nothing
         return;
         }
      }

   assert ((m_cchCur + cchValue) <= m_cchBufMax);

   strncpy (m_pString + m_cchCur, pchValue, cchValue);

   m_cchCur += cchValue;

   return;

}  /* end of CtStringT::Append (const Chr *pchValue, U32 cchValue) */

//
//  CtStringT::AppendFromAscii (pchValue, cchValue)
//
//  Description:
//    Appends a text string to our instance.  We make a copy of the
//    supplied original, resizing our internal buffer if necessary.
//
//    We stop the append at the soonest of 1) finding an EOS in the
//    supplied value, or 2) appending as many chars as specified by
//    the supplied count.
//
//    *  This routine is just like Append(), except that it assumes
//       an RHS of (const char *) regardless of what our Chr type
//       instantiates as.  This routine thus serves as an ASCII ->
//       Unicode converter, when our Chr type is wchar_t.
//       [Yes, you could do AssignFromAscii() on a CtStringT<char>
//        instance also, if you wanted to.]
//    
//  Inputs:
//    pchValue - Points to ASCII string value to append to our instance.
//    cchValue - One-based count of chars in *pchValue which we
//                append to our instance.  If *pchValue has an EOS
//                before we reach this count, we stop at the EOS instead.
//
//  Outputs:
//    none
//

template <class Chr>
void  CtStringT<Chr>::AppendFromAscii (const char *pchValue,
                                       U32 cchValue /* = -1 */ )
{

U32   cchActual;
Chr * pDst;


   if (pchValue == NULL)
      {
      return;
      }

   //  take care of possible cases where cchValue is -1 or otherwise
   //  not quite right.  But allow cchValue to truncate *pchValue.
   cchActual = ::strlen (pchValue);
   if (cchValue > cchActual)
      {
      cchValue = cchActual;
      }

   if ((m_cchCur + cchValue) > m_cchBufMax)
      {
      //  no room for new string, got to resize
      if (! Resize (m_cchCur + cchValue))
         {
         //  whoops, do nothing
         return;
         }
      }

   assert ((m_cchCur + cchValue) <= m_cchBufMax);

   //  set starting dest of copy, before updating size member
   pDst = m_pString + m_cchCur;

   //  update size member, before zapping cchValue
   m_cchCur += cchValue;

   //  do the copy
   while (cchValue > 0)
      {
      *pDst = *pchValue;
      pDst ++;
      pchValue ++;
      cchValue --;
      }

   //  and don't forget to post a sentinel
   *pDst = EOS;

   return;

}  /* end of CtStringT::AppendFromAscii (const char *pchValue, U32 cchValue) */

//
//  CtStringT::AppendFromUnicode (pwchValue, cwchValue)
//
//  Description:
//    Appends a text string to our instance.  We make a copy of the
//    supplied original, resizing our internal buffer if necessary.
//
//    We stop the append at the soonest of 1) finding an EOS in the
//    supplied value, or 2) appending as many chars as specified by
//    the supplied count.
//
//    *  This routine is just like Append(), except that it assumes
//       an RHS of (const wchar_t *) regardless of what our Chr type
//       instantiates as.  This routine thus serves as a Unicode ->
//       ASCII converter, when our Chr type is char.
//       [Yes, you could do AppendFromUnicode() on a CtStringT<wchar_t>
//        instance also, if you wanted to.]
//
//    ** At present, the only supported conversion for non-ASCII
//       characters in our Unicode RHS is to "greek" them, replacing
//       each with a '?' character.  At some point in the future,
//       this may be augmented with Unicode UTF-8 support; see
//       RFC 2279 for more information about UTF-8).
//    
//  Inputs:
//    pwchValue - Points to Unicode string value to append to our instance.
//    cwchValue - One-based count of wchars in *pwchValue which we
//                append to our instance.  If *pwchValue has an EOS
//                before we reach this count, we stop at the EOS instead.
//
//  Outputs:
//    none
//

template <class Chr>
void  CtStringT<Chr>::AppendFromUnicode (const wchar_t *pwchValue,
                                         U32 cwchValue /* = -1 */ )
{

U32               cwchActual;
const wchar_t   * pSrc;
Chr             * pDst;
wchar_t           wchMask;    // for testing whether char needs "greeking"
wchar_t           wch;


   if (pwchValue == NULL)
      {
      return;
      }

   //  take care of possible cases where cwchValue is -1 or otherwise
   //  not quite right.  But allow cwchValue to truncate *pwchValue.

   //  (no standard "wstrlen" avail, so do it inline:)
   pSrc = pwchValue;
   cwchActual = 0;
   while (*pSrc != EOS)
      {
      pSrc ++;
      cwchActual ++;
      }

   if (cwchValue > cwchActual)
      {
      cwchValue = cwchActual;
      }

   if ((m_cchCur + cwchValue) > m_cchBufMax)
      {
      //  no room for new string, got to resize
      if (! Resize (m_cchCur + cwchValue))
         {
         //  whoops, do nothing
         return;
         }
      }

   assert ((m_cchCur + cwchValue) <= m_cchBufMax);

   //  set starting dest of copy, before updating size member
   pDst = m_pString + m_cchCur;

   //  update size member, before zapping cchValue
   m_cchCur += cwchValue;

   //  figure out whether we need to worry about "greeking":
   if (sizeof (Chr) >= sizeof (wchar_t))
      {
      //  char sizes are same, so never greek text
      wchMask = 0;
      }
   else
      {
      //  size is smaller, greek non-ASCII text
      wchMask = ~0x7F;        // mask looks for non-ASCII chars
      }

   //  do the copy
   while (cwchValue > 0)
      {
      //  do "greeking" of non-ASCII chars
      wch = *pwchValue;
      if ((wch & wchMask) != 0)
         {
         //  non-ASCII char, away with it!
         wch = '?';
         }

      *pDst = (Chr) wch;
      pDst ++;
      pwchValue ++;
      cwchValue --;
      }

   //  and don't forget to post a sentinel
   *pDst = EOS;

   return;

}  /* end of CtStringT::AppendFromUnicode (pwchValue, cwchValue) */

//
//  CtStringT::Fill (chFill, cchFill)
//
//  Description:
//    Clears our instance, and then sets it to a string of cchFill
//    chFill characters.
//
//  Inputs:
//    chFill - Fill character for our new content.
//    cchFill - One-based count of fill characters to write as our value.
//
//  Outputs:
//    none
//

template <class Chr>
void  CtStringT<Chr>::Fill (Chr chFill, U32 cchFill)
{

Chr * p;
U32   i;


   Clear ();      // clear first, so Resize() doesn't do copy

   if (cchFill > m_cchBufMax)
      {
      Resize (cchFill);
      }

   if (m_cchBufMax >= cchFill)
      {
      //  got a suitable buffer, do the fill:
      p = m_pString;
      for (i = 0;  i < cchFill;  i++)
         {
         *p = chFill;
         p ++;
         }

      *p = EOS;
      }

   return;

}  /* end of CtStringT<Chr>::Fill */

//
//  CtStringT::Resize (cchNewMax)
//
//  Description:
//    Resizes our internal string buffer so that it can hold at least
//    the specified number of characters.
//
//    We never reduce the size of the buffer.
//    
//  Inputs:
//    cchNewMax - One-based count of chars which our buffer needs to
//                be able to hold.  We may bump the buffer's size up
//                to some value a bit larger than this.
//
//  Outputs:
//    CtStringT::Resize - Returns TRUE if our buffer is big enough
//                to hold the specified number of chars.  We return
//                TRUE even if we didn't need to resize the buffer.
//

template <class Chr>
BOOL  CtStringT<Chr>::Resize (U32 cchNewMax)
{

Chr   * pchNewBuf;
BOOL        fMyRet;


   fMyRet = TRUE;    // default to happiness

   //  round cbNewMax up to 8-byte granularity, to reduce number
   //  of reallocates as string length grows
   cchNewMax = (cchNewMax + 7) & ~7;

   if (cchNewMax > m_cchBufMax)
      {
      //  grab new buffer, with room for EOS
      pchNewBuf = new Chr [cchNewMax + 1];

      if (pchNewBuf != NULL)
         {
         //  if we have an old buffer, save its contents & delete it
         if (m_cchBufMax > 0)
            {
            strncpy (pchNewBuf, m_pString, m_cchCur);
            delete [] m_pString;
            }

         //  make new buffer our official one
         m_pString = pchNewBuf;
         m_cchBufMax = cchNewMax;
         }
      else
         {
         //  failed to allocate new buffer.
         fMyRet = FALSE;

         if (m_cchBufMax == 0)
            {
            //  no prior buffer, so set up our standard dummy environment
            m_cchCur = 0;
            m_pString = (Chr *) &chEOS;
            }
         else
            {
            //  leave old buffer unchanged
            }
         }
      }

   return (fMyRet);

}  /* end of CtStringT::Resize */

//
//  CtStringT::Clear ()
//
//  Description:
//    Clears our instance, so that it returns the empty string (not NULL)
//    if queried.
//
//    We do not release our buffer, if any.
//    
//  Inputs:
//    none
//
//  Outputs:
//    none
//

template <class Chr>
void  CtStringT<Chr>::Clear (void)
{

   m_cchCur = 0;
   if (m_cchBufMax > 0)
      {
      //  empty our buffer
      *m_pString = chEOS;
      }
   else
      {
      //  no buffer, so fake it
      m_pString = (Chr *) &chEOS;
      }

   return;

}  /* end of CtStringT::Clear */

//
//  CtStringT::strlen (pchValue)
//
//  Description:
//    Counts chars in *pchValue, just as ::strlen() would do, if it only
//    knew about chars.
//    
//  Inputs:
//    pchValue - Points to string to count chars in.
//
//  Outputs:
//    none
//

/* static */
template <class Chr>
U32  CtStringT<Chr>::strlen (const Chr *pchValue)
{

U32   cchMyRet;


   //  (we actually see NULL here, during 0-argument constructor use)
   //  assert (pchValue != NULL);

   cchMyRet = 0;

   if (pchValue != NULL)
      {
      while (*pchValue != chEOS)
         {
         cchMyRet ++;
         pchValue ++;
         }
      }

   return (cchMyRet);

}  /* end of CtStringT::strlen */

//
//  CtStringT::strncpy (pchDest, pchSrc, cchMax)
//
//  Description:
//    Does a count-limited copy of a string into a buffer.
//    Unlike ::strncpy(), we guarantee that the resulting copy
//    has an EOS applied.
//    
//  Inputs:
//    pchDest - Points to buffer to receive copy.  Must have room for
//                at least (cchMax + 1) chars.
//    pchSrc - String to copy from.  May contain an early EOS, in which
//                case we stop copying there.
//    cchMax - One-based maximum count of chars to copy, excluding EOS.
//
//  Outputs:
//    none
//

/* static */
template <class Chr>
void  CtStringT<Chr>::strncpy (Chr *pchDest, const Chr *pchSrc, U32 cchMax)
{


   assert ((pchSrc != NULL) && (pchDest != NULL));

   if (pchDest != NULL)
      {
      //  got a buffer to copy into, do we have a source?
      if (pchSrc != NULL)
         {
         //  got source, do standard counted copy
         while ((*pchSrc != chEOS) && (cchMax > 0))
            {
            *pchDest = *pchSrc;
            pchSrc ++;
            pchDest ++;
            cchMax --;
            }
         }
      else
         {
         //  no source, EOS below makes an empty string.
         }

      //  any time we have an output buffer, set an EOS
      *pchDest = chEOS;
      }

   return;

}  /* end of CtStringT::strncpy */

//
//  CtStringT::strcmp (pchLhs, pchRhs)
//
//  Description:
//    Does a compare of two EOS-terminated char strings.  We act just
//    like ::strcmp() would, if only it knew about chars.
//
//    We treat NULL string pointers just like empty strings.
//    Our comparison is case-sensitive.
//    
//  Inputs:
//    pchLhs - Left-hand side operand.
//    pchRsh - Right-hand side operand.
//
//  Outputs:
//    CtStringT::strcmp - Returns 0 if the two strings are equal in
//                   value and length.
//                   Returns +1 if the LHS string is greater than the RHS
//                   (greater char value, or common prefix but longer than
//                   RHS).
//                   Returns -1 if the LHS string is less than the RHS
//                   (lesser char value, or common prefix but shorter than
//                   RHS).
//

/* static */
template <class Chr>
S32  CtStringT<Chr>::strcmp (const Chr *pchLhs, const Chr *pchRhs)
{

S32   sMyRet;


   assert (pchLhs != NULL);
   assert (pchRhs != NULL);

   if (pchLhs == NULL)
      pchLhs = &chEOS;

   if (pchRhs == NULL)
      pchRhs = &chEOS;

   while ((*pchLhs == *pchRhs) && (*pchLhs != chEOS))
      {
      pchLhs ++;
      pchRhs ++;
      }

   //  in the following, we rely on EOS == 0 (the least-valued char):
   if (*pchLhs == *pchRhs)
      {
      //  they're equal
      assert ((*pchLhs == chEOS) && (*pchRhs == chEOS));
      sMyRet = 0;
      }
   else if (*pchLhs > *pchRhs)
      {
      //  lhs is greater
      sMyRet = 1;
      }
   else
      {
      //  rhs is greater
      sMyRet = -1;
      }

   return (sMyRet);

}  /* end of CtStringT::strcmp */


//  our standard flavors of string:

//  an ASCII version
typedef CtStringT<char> CtAString;

//  a Unicode version, the preferred flavor for Odyssey UI code
typedef CtStringT<wchar_t> CtString;


#endif  // #ifndef _CtString_h_

