/*  Metrowerks Standard Library  Version 2.1.2  1997 May  */

/*
 *	time.mac.c
 *	
 *		Copyright © 1995-1997 Metrowerks, Inc.
 *		All rights reserved.
 *	
 *	Routines
 *	--------
 *		__get_clock
 *		__get_time
 *
 *		__to_gm_time
 *
 *
 */

// Include your OS dependent include files
#include <time.h>

#ifndef _No_Time_OS_Support

clock_t __get_clock(void)
{
	return 0; // replace with an OS call.
}

time_t __get_time(void)
{
	return 0; // replace with an OS call.
}

int __to_gm_time(time_t * time)
{
	return 0; // replace with an OS call.
}

#endif

/*  Change Record
 *	20-Jul-97 MEA  First code release.
*/
