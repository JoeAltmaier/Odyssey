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
// File: CtIsoCodes.h
// 
// Description:
//    Defines the ISO language and other codes which we might need.
//    These defs used to be in CtMessages.h, but were moved here to avoid
//    problems when using this header in win32 (typedef fights in simple.h,
//    etc.).
// 
// $Log: /Gemini/Include/CtIsoCodes.h $
// 
// 1     7/12/99 11:43a Ewedel
// Initial revision.
// 
/*************************************************************************/

#ifndef _CtIsoCodes_h_
#define _CtIsoCodes_h_



//  here are some language ID values.  These are compatible with
//  the values defined in winnt.h for use with the macro MAKELANGID().
//  [These values may come from ISO 639, but I have been unable to locate
//   a version of this standard which uses numeric coding.]
enum CtLanguageId {
   k_eLangDefault  = 0,           // normally, means English
   k_eLangEnglish  = 0x409,
   k_eLangGerman   = 0x407,
   k_eLangFrench   = 0x40C,
   k_eLangItalian  = 0x410,
   k_eLangSpanish  = 0x40A,
   k_eLangJapanese = 0x411
};


#endif  // #ifndef _CtIsoCodes_h_

