//
//	ansi_prefix.MIPS_bare.debug.h
//	
//		Copyright � 1997 Metrowerks, Inc.
//		All rights reserved.
//
//      This is a debug version of the standard ansi_prefix.MIPS_bare.h
//      "prefix" file used under Metrowerks for Odyssey programs.
//
//      This version of the prefix file enables heap checking when
//      building the Odyssey replacement MSL.
//
// $Log: /Gemini/Include/ansi_prefix.mips_bare.debug.h $
// 
// 2     3/23/99 2:54p Jaltmaier
// _NO_CACHE set.
// 
// 1     3/19/99 3:17p Ewedel
// Initial checkin.
//

#ifndef __ansi_prefix__
#define __ansi_prefix__


//  enable OOS debugging
#define _DEBUG                  /* trace support */
#define _HEAPCHECK              /* what it says :-) */
#define _NO_CACHE				/* don't use kseg0 cached ptrs 0x8xxxxxxx */

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#include <__config.h>
#include <ansi_parms.h>


#define __dest_os	__mips_bare

	/* Even with floating point emulation, certain chips, such	*/
	/* as the 821, do not have floating point registers.  		*/
	/* Comment out the definition of _No_Floating_Point_Regs	*/
	/* if your chip does have a floating point unit.			*/
/*#define _No_Floating_Point_Regs*/

	/* the following are OS services that aren't available 		*/
#define _No_Time_OS_Support
#define _No_Alloc_OS_Support
#define _No_Disk_File_OS_Support

	/* uncomment _No_Console if you do not want to	*/
	/* write and read to a console window.  		*/
//#define _No_Console
#ifndef _No_Console
	/* the serial 1 and 2 UARTlibs have unbuffered	*/
	/* IO; comment out the following line if  		*/
	/* you are either not using either the serial 1 */
	/* or 2 UARTlibs or if your OS has buffered IO.	*/
#define _Unbuffered_Console
#endif

//#define TARGET_BIG_ENDIAN 	/* unused */
#define TARGET_IEEE754
#define TARGET_HEAP_ALIGNMENT	8
//#define TARGET_HEAP_ALIGNMENT	16

#define NEWMODE NEWMODE_MALLOC		//	always use malloc for new

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif
