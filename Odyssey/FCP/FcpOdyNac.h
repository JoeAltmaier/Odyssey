/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// FcpOdyNac.h
//
// Description:
// This is the FCP Odyssey Prefix file.  This seems to be the best place for
// global defines for any given build.  In particular, this file includes
// defines for a DEBUG with trace type build.
//
//	Include this file in your project settings under "C/C++ Language" in
//  the "Prefix File" box.
// 
// Update Log: 
// 9/29/98 Michael G. Panas: Create file
// 5/10/99 Michael G. Panas: Create file from CtPrefix.h
/*************************************************************************/

#ifndef __FcpOdyNac_h
#define __FcpOdyNac_h


//#include "ansi_prefix.MIPS_bare.h"

//#define	_DEBUG				// turn on debug code
#include "CtPrefix.h"		// defines _DEBUG

#define	_ODYSSEY			// build for Odyssey Platform
#define	_NAC

#endif