/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// RamDiskPrefix.h
//
// Description:
// This is STS with Ram Disk Test Build Prefix file.
// This seems to be the best place for
// global defines for any given build.  In particular, this file includes
// defines for a DEBUG with trace type build.
//
//	Include this file in your project settings under "C/C++ Language" in
//  the "Prefix File" box.
// 
// Update Log: 
// 03/16/99 Michael G. Panas: Create version from StsPrefix.h
/*************************************************************************/

#ifndef __RamDiskPrefix_h
#define __RamDiskPrefix_h


//#include "ansi_prefix.MIPS_bare.h"

#include "CtPrefix.h"

//#define INCLUDE_ODYSSEY		// building for an Odyssey

#define RD_BUILD			// This build is for Ram Disks
#define STS_BUILD			// include the Target Servers and Bsas
#define SCSI_TARGET_SERVER	// Need to include Sts code in BuildSys

#define	INCLUDE_EV64120  1      // Flag for running on EV64120 systems.
#undef	INCLUDE_ODYSSEY         // Flag for running on ODYSSEY systems.
#define TRK_TRANSPORT_INT_DRIVEN  1

#endif