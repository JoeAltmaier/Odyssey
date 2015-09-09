/* Watch.c -- RM7000 Watchpoint support
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
**/

// Revision History:
//  8/4/98 Joe Altmaier: Create file
//  8/6/99 Eric Wedel: Added Joe's notes as a function header.

#include "Watch.h"
#include "Hw.h"


#define PA_P(p) (unsigned long)( (unsigned long)(p) < KSEG0_BASE_ADDR ? 0 \
		: (unsigned long)(p) < KSEG1_BASE_ADDR ? (unsigned long)(p) - KSEG0_BASE_ADDR \
		: (unsigned long)(p) - KSEG1_BASE_ADDR )


//
//  Watch (watch, pWatch, cb, mode)
//
//  Description:
//    Enables "watch-point" monitoring, using the MIPS RM-7000 debug
//    registers.
//
//    Note that there are two sets of registers; this routine allows
//    either set to be used.
//
//    ** Only one watchpoint register set may be enabled at a time.
//       This is a restriction of this routine.  However, even if both
//       could be set at once, they would share a common mask (size) value.
//       The latter is a restriction of the MIPS R7000 architecture.
//
//    ** The region pointer value pWatch should be on a binary boundary,
//       because the size cb is actually used as a mask.
//       For example,
//             pWatch = 0xA0137250
//             cb     = 0x00001000
//       actually sets a watchpoint from 0xA0137000 to 0xA0137FFF
//       because the cb becomes a mask of FFF.
//
//    ***  This code requires an RM-7000 class of MIPS CPU in order to
//         operate.  If you attempt to use it with one of the Galileo
//         evaluation boards, please verify that your CPU daughter-card
//         contains an RM-7000 processor.
//         All Odyssey boards use RM-7000 MIPS processors, so testing
//         there should be no problem.
//
//  Inputs:
//    watch - Selects which watchpoint register set we program.
//                Value should be one of WATCH1, WATCH2.
//    pWatch - Pointer to beginning address of region to watch.  This value
//                interacts with cb, see the discussion above.
//    cb - One-based count of bytes to watch, starting with byte at
//                *pWatch.  This value must be an even power of two.
//                It interacts with pWatch, see the discussion above.
//                Setting cb to zero disables the selected watchpoint.
//                *** This value is common to both watchpoint register sets.
//                    See the discussion above for more info.
//    mode - Indicates the type of operation for which the watchpoint
//                is searching.  This is the OR of one or more of the values
//                   WATCHSTORE   - detect writes to memory
//                   WATCHLOAD    - detect data reads from memory
//                   WATCHEXECUTE - detect instruction fetches from memory
//
//  Outputs:
//    none
//

void Watch(WATCH watch, void *pWatch, int cb, WATCHMODE mode) {
	long long w = mode << 60;
	unsigned long pa=PA_P(pWatch);
	w |= (pa & 0xFFFFFFFC);
	if (cb)
		cb = (cb-1) & 0xFFFFFFFC;
	
	switch (watch) {
	case WATCH1:
		//  write w to watch1 control reg (64 bits)
      asm { ld t0, w; dmtc0 t0, $18; nop; nop; }
		if (cb) {
         //  got non-zero count, so set watch1 enable
			cb |= WATCH1;
			}
      //  write count/enable flags to WatchMask (common 32 bit reg for watch1/2)
		asm { lw t0, cb; mtc0 t0, $24; nop; nop; }
		break;

	case WATCH2:
		//  write w to watch2 control reg (64 bits)
		asm { ld t0, w; dmtc0 t0, $19; nop; nop; }
		if (cb) {
         //  got non-zero count, so set watch2 enable
			cb |= WATCH2;
			}
      //  write count/enable flags to WatchMask (common 32 bit reg for watch1/2)
      asm { lw t0, cb; mtc0 t0, $24; nop; nop; }
		break;
	}
	
}  /* end of Watch */

