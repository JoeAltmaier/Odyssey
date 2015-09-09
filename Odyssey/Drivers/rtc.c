/*
 *
 * MODULE: rtc.c - driver for Real Time Clock
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 08/26/99  SK: Created by Sudhir
 *
 *
 *
 * This material is a confidential trade secret and proprietary information
 * of ConvergeNet Technologies, Inc. which may not be reproduced, used, sold
 * or transferred to any third party without the prior written consent of
 * ConvergeNet Technologies, Inc.  This material is also copyrighted as an
 * unpublished work under sections 104 and 408 of Title 17 of the United
 * States Code.  Law prohibits unauthorized use, copying or reproduction.
 *
 */


//#define	SEMAP	1
#ifdef	SEMAP
#include "nucleus.h"
NU_SEMAPHORE    RtcSemaphore;
NU_SEMAPHORE    *rtc_semaptr = NU_NULL;
#endif
#include "types.h"
#include "time.h"
#include "rtc.h"

U8 bcd2bin(U8 val);
U8 bin2bcd(U8 val);

STATUS
InitTime()
{
#ifdef	SEMAP
	if ( !rtc_semaptr) {
		rtc_semaptr = &RtcSemaphore;
		if ( NU_Create_Semaphore(rtc_semaptr, "Rtc", 1, NU_FIFO) !=
				NU_SUCCESS) {
			rtc_semaptr = NU_NULL;
			return(RTC_ERROR_SEMA);
		}
	}
#endif
	return(OK);
}

STATUS
GetTime(struct tm *time)
{
	/* Enable Read */
#ifdef	SEMAP
	if ( !rtc_semaptr) {
		return(RTC_ERROR_SEMA);
	}
	NU_Obtain_Semaphore(rtc_semaptr, NU_SUSPEND);
#endif
	*((U8 *)RTC_CNTL_ADDR) = RTC_CNTL_READ;
		
	/* Get the Info */
	time->tm_year     = bcd2bin(*((U8 *)RTC_YEAR_ADDR));
	time->tm_mon    = (bcd2bin(*((U8 *)RTC_MONTH_ADDR) & RTC_MONTH_MASK)) - 1;
	time->tm_mday     = bcd2bin(*((U8 *)RTC_DATE_ADDR) & RTC_DATE_MASK);
	time->tm_wday      = (bcd2bin(*((U8 *)RTC_DAY_ADDR) & RTC_DAY_MASK)) - 1;
	time->tm_hour     = bcd2bin(*((U8 *)RTC_HOUR_ADDR) & RTC_HOUR_MASK);
	time->tm_min  = bcd2bin(*((U8 *)RTC_MIN_ADDR) & RTC_MIN_MASK);
	time->tm_sec  = bcd2bin(*((U8 *)RTC_SEC_ADDR) & RTC_SEC_MASK);

#if 0
	/* Retrieve the timezone and dstflag */
	src = (U8 *)RTC_TIMEZONE_ADDR;
	dst = (U8 *)&time->timezone;
	for(i=0; i < 8; i++)
		dst[i] = src[i];
	
#endif

	/* Disbale Read */
	*((U8 *)RTC_CNTL_ADDR) = 0;
#ifdef	SEMAP
	NU_Release_Semaphore(rtc_semaptr);
#endif
	return(OK);
		
}

STATUS
SetTime(struct tm  *time)
{
	/* Enable Write */
#ifdef	SEMAP
	if ( !rtc_semaptr) {
		return(RTC_ERROR_SEMA);
	}
	NU_Obtain_Semaphore(rtc_semaptr, NU_SUSPEND);
#endif
	*((U8 *)RTC_CNTL_ADDR) = RTC_CNTL_WRITE;
		
	/* Write the Info */
	*((U8 *)RTC_YEAR_ADDR)      = bin2bcd(time->tm_year);
	*((U8 *)RTC_MONTH_ADDR)     = bin2bcd((time->tm_mon+1) & RTC_MONTH_MASK);
	*((U8 *)RTC_DATE_ADDR)      = bin2bcd(time->tm_mday & RTC_DATE_MASK);
	*((U8 *)RTC_DAY_ADDR)       = bin2bcd((time->tm_wday+1) & RTC_DAY_MASK);
	*((U8 *)RTC_HOUR_ADDR)      = bin2bcd(time->tm_hour & RTC_HOUR_MASK);
	*((U8 *)RTC_MIN_ADDR)       = bin2bcd(time->tm_min & RTC_MIN_MASK);
	*((U8 *)RTC_SEC_ADDR)       = bin2bcd(time->tm_sec & RTC_SEC_MASK);
	
#if 0
	/* Store the timezone and dstflag */
	src = (U8 *)&time->;
	dst = (U8 *)RTC_TIMEZONE_ADDR;
	for(i=0; i < 8; i++)
		dst[i] = src[i];	
#endif		
	/* Disbale Write */
	*((U8 *)RTC_CNTL_ADDR) = 0;
#ifdef	SEMAP
	NU_Release_Semaphore(rtc_semaptr);
#endif
	return(OK);
}

U8
bcd2bin(U8 val)
{
	return(((val & 0xF0) >> 4) * 10 + ( val & 0x0F));
}

U8
bin2bcd(U8 val)
{
	return ( (( val / 10) << 4) + ( val % 10));
}
