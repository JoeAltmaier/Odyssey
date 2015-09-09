/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// HscsiPrefix.h
//
// Description:
// This is HSCSI Test Build Prefix file.  This seems to be the best place for
// global defines for any given build.  In particular, this file includes
// defines for a DEBUG with trace type build.
//
//	Include this file in your project settings under "C/C++ Language" in
//  the "Prefix File" box.
// 
// Update Log: 
// 9/29/98 Michael G. Panas: Create file
// 12/23/98 Michael G. Panas: Create version from CtPrefix.h
/*************************************************************************/

#ifndef __HscsiPrefix_h
#define __HscsiPrefix_h


//#include "ansi_prefix.MIPS_bare.h"

#include "CtPrefix.h"

#define HSCSI_BUILD			// and the FCP Test Build
//#define NONIC				// no NIC in this config
#define HSCSI

#define TRK_TRANSPORT_INT_DRIVEN
#define	INCLUDE_EV64120

#endif