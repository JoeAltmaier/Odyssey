/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmData.h
// 
// Description:
// This file contains static and global data structure definitions used
// within the Flash Block.
// This module can be compiled in two modes:
// 	1.  If FbData is defined, the data is declared
// 		as if it were a .c file.  It must be compiled once this way.
// 	2.  If FbData is not defined, the data is only defined
// 		as if it were a .h file.
// 
// Update Log: 
// 7/20/98 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(CmData_H)
#define CmData_H

#include "Nucleus.h"

/*************************************************************************/
//    Global Data
//    should be compiled once with
//    #define CmData 
//    This causes the data in the module to be declared without the extern
/*************************************************************************/
#if defined(CmData)
#define _EX 
#define _INIT(value) = value
#else
#define _EX extern
#define _INIT(value)
#endif

_EX UNSIGNED			  CM_if_trace_on;


#endif // CmData_H