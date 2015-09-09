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
// File: Prefix_TestHwIntf.h
// 
// Description:
//    Magic Metrowerks "prefix" file, as specified in our test project's
//    C++ settings panel.
//
//    This file defines a single manifest, CT_TEST_HW_INTF, which is used
//    to enable some debug linkage code in ..\CmbIsr.cpp.
//    And just for completeness, we also include the standard Odyssey
//    prefix file.
// 
// $Log: /Gemini/Odyssey/DdmCmb/TestHwIntf/Prefix_TestHwIntf.h $
// 
// 1     7/20/99 5:37p Ewedel
// [At Taylor] Initial revision.
// 
/*************************************************************************/

#ifndef _Prefix_TestHwIntf_h_
#define _Prefix_TestHwIntf_h_


//  include our standard Metrowerks "prefix" file for use with Odyssey targets
//#include  "Prefix_Odyssey.h"

// special constant to flag that we're building a debug project
#define  CT_TEST_HW_INTF


#endif  // #ifndef _Prefix_TestHwIntf_h_


