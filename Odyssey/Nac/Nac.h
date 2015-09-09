/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Nac.h
//
// Description:
// This is NAC Build Prefix file.  This seems to be the best place for
// global defines for any given build.  In particular, this file includes
// defines for a DEBUG with trace type build.
//
//	Include this file in your project settings under "C/C++ Language" in
//  the "Prefix File" box.
// 
// Update Log: 
// 07/21/99 Michael G. Panas: Create file
/*************************************************************************/

#ifndef __Nac_h
#define __Nac_h


//#include "ansi_prefix.MIPS_bare.h"

#define	_DEBUG			// turn on debug code
#define	INCLUDE_ODYSSEY	
#define CONFIG_INT_PRIORITY		// Flag for prioritized interrupts.


#define NAC_BUILD		// This build is for the NAC board
//#define	NO_INIT		// no Initiator FCP code here
//#define	NO_TARGET	// no Target FCP code here
//#define	NO_BSA		// no BSA code here
#define	SCSI_TARGET_SERVER		// need SCSI Target Server code here
#define	RAMDISK			// need RAM Disk code here
#endif