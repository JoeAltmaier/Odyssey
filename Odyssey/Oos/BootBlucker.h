/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This class implements the bootblock server
// 
// Update Log: 
// 9/29/99 Ryan Braun: Create file
/*************************************************************************/

#ifndef __BootBlucker_h
#define __BootBlucker_h

#include "Task.h"
#include "HbcSubnet.h"
#include "Network.h"
#include "externs.h"

extern "C" long long bootblockp;
extern "C" bootblock_t bootblock;

class BootBlucker {

public:
	static void Initialize();
	
private:
	static void SuckBootBlock();
	
};

#endif