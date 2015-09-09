/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This file contains prototypes for "private" functions used to do
// string formatting within the util\ library.
// 
// $Log: /Gemini/Odyssey/Util/ct_fmt.h $
// 
// 2     4/23/99 6:13p Ewedel
// Broke _fmt()'s function arg into its own typedef, and changed _fmt() to
// take a va_list stdarg argument tail.
// 
// 1     3/04/99 4:07p Ewedel
// Common header for formatting functions (Fmt*.c).
//
/*************************************************************************/
#ifndef _ct_fmt_h_
#define _ct_fmt_h_


#ifndef OsTypes_H
# include "OsTypes.h"
#endif


//  standard character output function passed to / used by _fmt
typedef  STATUS (*pfnCharOut)(void *pCookie, char chOut);

//  format worker - passes pCookie through to fnCharOut(), which is called
//  for each character of formatted output
STATUS  _fmt(pfnCharOut fnCharOut, void *pCookie, const char *pszFormat,
             va_list ArgList);

//  our one instance of *pfnCharOut -- pCookie is ppBuf, which we update
STATUS _movech(void *pCookie,char ch);


#endif // _ct_fmt_h_

