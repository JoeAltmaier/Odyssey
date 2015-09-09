/* Kernel.cpp -- Wrapper Class for underlying Kernel (Nucleus)
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Description:
 * 		This class encapsulates the features of the Nucleus Kernel.
 *
**/

// Revision History: 
// 4/27/99 Joe Altmaier: Create file
// 7/09/99 Joe Altmaier: Added GREEN_HILLS version

#include "Kernel.h"
#include "BuildSys.h"
#include "Critical.h"

	FAILNAME(Kernel, Kernel::Stop_Scheduler);
	
//static
// argc contains the actual function pointer
// argv contains argv
asm void Kernel::Task_Entry_Nucleus(UNSIGNED argc, VOID *argv) {
#if defined(IS_GREEN_HILLS)
	asm (" addu $t0, $a0, $0 ");
	asm (" addu $a0, $a1, $0 ");
	asm (" jr $t0 ");
	asm (" nop ");
#else
	addu t0, a0, $0;
	addu a0, a1, $0;
	jr t0;
	nop;
#endif
}

void Kernel::Stop_Scheduler() {
	Critical::Disable();
	}
