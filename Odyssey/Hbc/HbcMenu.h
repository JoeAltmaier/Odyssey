/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Rac.h
//
// Description:
// This is RAC Build Prefix file.  This seems to be the best place for
// global defines for any given build.  In particular, this file includes
// defines for a DEBUG with trace type build.
//
//	Include this file in your project settings under "C/C++ Language" in
//  the "Prefix File" box.
// 
// Update Log: 
// 03/26/99 Michael G. Panas: Create file
/*************************************************************************/

#ifndef __RacPrefix_h
#define __RacPrefix_h


//#include "ansi_prefix.MIPS_bare.h"

#define	_DEBUG			// turn on debug code


#define RAC_BUILD		// This build is the RAC Image for Odyssey
//#define	NO_INIT		// no Initiator FCP code here
#define	NO_TARGET		// no Target FCP code here
//#define	NO_BSA		// no BSA code here
#define	SCSI_TARGET_SERVER		// need SCSI Target Server code here
#define	RAMDISK		// need RAM Disk code here

#endif