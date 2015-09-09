/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: TimerTicks.h
// 
// Description:
// This file defines timer ticks used by Odyssey. 
// 
// Update Log 
// 
// 8/12/98 Jim Frandeen: Create file from mail message from Jeff Nespor
// 02/12/99 Jim Frandeen: Add end of line to end of file
/*************************************************************************/
#if !defined(TimerTicks_H)
#define TimerTicks_H

// The 64120 manual doesn't come right out and say so, but the timers 
// count at the clock rate of the part, which is 75MHz  ( I cheated and 
// looked in an older manual).  The 64120 is code compatible with their
// older 64010, and the 64010 manual does come right out and say that
// the timers and counters decrement every clock.  I did a little math,
// and here's what I got: 

//     75000000 or 0x047868C0 =  1 second 
//        75000 or 0x000124F8 =  1 ms 
//           75 or 0x0000004B =  1 us 
//            1 or 0x00000001 = 23 ns 

#define NS_PER_TIMER_TICK		      23
#define TIMER_TICKS_1_SECOND	75000000
#define TIMER_TICKS_1_MS		   75000
#define TIMER_TICKS_1_US			  75

#endif // TimerTicks_H
