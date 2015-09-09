/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// CTPrefix_Odyssey.h
//
// Description:
// This is ConvergeNets Prefix file.  This seems to be the best place for
// global defines for any given build.  In particular, this file includes
// defines for a DEBUG with trace type build.
//
//	Include this file in your project settings under "C/C++ Language" in
//  the "Prefix File" box.
// 
// Update Log: 
// 9/29/98 Michael G. Panas: Create file
/*************************************************************************/

#ifndef __CTPrefix_Odyssey_h
#define __CTPrefix_Odyssey_h


//#define	_DEBUG				// turn on debug code
#include "CtPrefix.h"		// defines _DEBUG now
#include "Prefix_odyssey.h"
#define _ODYSSEY			// target odyssey (not eval)

#endif