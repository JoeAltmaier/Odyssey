/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Nic.h
//
// Description:
// This is NIC Test Build Prefix file.  This seems to be the best place for
// global defines for any given build.  In particular, this file includes
// defines for a DEBUG with trace type build.
//
//	Include this file in your project settings under "C/C++ Language" in
//  the "Prefix File" box.
// 
// Update Log: 
// 9/29/98 Michael G. Panas: Create file
// 12/23/98 Michael G. Panas: Create version from CtPrefix.h
// 03/03/99 Michael G. Panas: Create version from RacPrefix.h
/*************************************************************************/

#ifndef __Nic_h
#define __Nic_h


//#include "ansi_prefix.MIPS_bare.h"

#include "CtPrefix.h"		// defines _DEBUG


#define NIC_TEST_BUILD		// This is a NIC emulation on the Eval Bd
#define	NORAC				// no RAC code here
#endif