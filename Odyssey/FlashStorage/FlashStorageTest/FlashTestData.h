/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FlashTestData.h
// 
// Description:
// This file contains static and global data structure definitions used
// within the Flash Test.
// This module can be compiled in two modes:
// 	1.  If FtData is defined, the data is declared
// 		as if it were a .c file.  It must be compiled once this way.
// 	2.  If FtData is not defined, the data is only defined
// 		as if it were a .h file.
// 
// $Log: /Gemini/Odyssey/FlashStorage/FlashStorageTest/FlashTestData.h $
// 
// 2     9/06/99 6:08p Jfrandeen
// New cache
// 
// 1     8/03/99 11:34a Jfrandeen
// 
// 2     5/06/99 5:40p Jfrandeen
// 
// 1     4/01/99 7:36p Jfrandeen
// Files common to all flash file system test drivers
//
// 11/16/98 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FlashTestData_H)
#define FlashTestData_H

#include "Cache.h"
#include "FlashStorage.h"
#include "FtTestDevice.h"

/*************************************************************************/
//    Global Data
//    should be compiled once with
//    #define FtData 
//    This causes the data in the module to be declared without the extern
/*************************************************************************/
#if defined(FtData)
#define _EX 
#define _INIT(value) = value
#else
#define _EX extern
#define _INIT(value)
#endif

_EX FF_HANDLE			FT_flash_handle;
_EX CM_CONFIG			FT_cache_config;
_EX FF_CONFIG			FT_flash_config;
_EX void				*FT_p_memory;
_EX void				*FT_p_read_buffer;
_EX void				*FT_p_write_buffer;
_EX void				*FT_p_mem_file;
_EX U32					FT_stop_test _INIT(0);
_EX char			  	FT_file_path[100] _INIT("FlashFile");
_EX FF_Sim_Device		FF_test_device;


#endif // FlashTestData_H
