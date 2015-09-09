/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CacheTestData.h
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
// 9/10/98 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(CacheTestData_H)
#define CacheTestData_H

#include "Cache.h"
#include "WinAsyncIO.h"

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

_EX char			  	CT_initial_file_name[256] _INIT("CacheFile");
_EX char			  	CT_file_path[100] _INIT("");
_EX CM_CONFIG			CT_config;
_EX U32					CT_controller_number _INIT(0);
_EX CM_CACHE_HANDLE		CT_cache_handle_0;
_EX CM_CACHE_HANDLE		CT_cache_handle_1;
_EX WinAsyncIO			CT_asyncIO;
_EX U32					CT_memory_size_to_allocate;
_EX U32					CT_stop_test _INIT(0);
_EX U32					CT_write_back;


#endif // CacheTestData_H