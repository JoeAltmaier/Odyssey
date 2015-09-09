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
// File: HwTestIntf.h
// 
// Description:
//    Provides some common definitions used by ..\CmbIsr.cpp and our local
//    test code.
//
//    This header is conditionally included by ..\cmbIsr.cpp, as gated by
//    the CT_TEST_HW_INTF manifest defined in Prefix_TestHwIntf.h, which
//    is included only in our test project.
// 
// $Log: /Gemini/Odyssey/DdmCmb/TestHwIntf/HwTestIntf.h $
// 
// 1     7/20/99 5:37p Ewedel
// [At Taylor] Initial revision.
// 
/*************************************************************************/

#ifndef _HwTestIntf_h_
#define _HwTestIntf_h_


#ifndef         NUCLEUS
# include  "Nucleus.h"
#endif


//  queue used to signal message receipt (we're not using the DDM model)
extern NU_QUEUE     Queue_1;


#endif  // #ifndef _HwTestIntf_h_


