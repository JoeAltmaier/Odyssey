/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Hbc_E2.h
//
// Description:
// This is HBC Rev E2 Build Prefix file.  This seems to be the best place
// for global defines for any given build.  In particular, this file
// includes defines for a DEBUG with trace type build.
//
//	Include this file in your project settings under "C/C++ Language" in
//  the "Prefix File" box.
// 
// Update Log: 
// 07/27/99 Michael G. Panas: Create file
/*************************************************************************/

#ifndef __Hbc_SysEx_h
#define __Hbc_SysEx_h


#include "prefix_odyssey.h"

#define	_DEBUG			// turn on debug code


#define HBC_BUILD		// This build is for the HBC board
#define FLASH_STORAGE
#define PERSIST

#define SYSTEM_EXERCISER  // This turns on SysEx extensions

#endif	// __Hbc_SysEx_h