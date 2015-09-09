/*
 *
 * MODULE: rtc.h - header file for Real Time Clock
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 08/26/99	- Created by Sudhir
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

#ifndef		_RTC_
#define		_RTC_

#ifdef  __cplusplus
extern "C" {
#endif

#define	RTC_BASE_ADDRESS	0xBC078000
#define	RTC_YEAR_OFFSET		0x7FFF
#define	RTC_MONTH_OFFSET	0x7FFE
#define	RTC_DATE_OFFSET		0x7FFD
#define	RTC_DAY_OFFSET		0x7FFC
#define	RTC_HOUR_OFFSET		0x7FFB
#define	RTC_MIN_OFFSET		0x7FFA
#define	RTC_SEC_OFFSET		0x7FF9
#define	RTC_CNTL_OFFSET		0x7FF8
#define	RTC_DSTFLAG_OFFSET	0x7FF4
#define	RTC_TIMEZONE_OFFSET	0x7FF0

#define	RTC_YEAR_ADDR	(RTC_BASE_ADDRESS + RTC_YEAR_OFFSET)
#define	RTC_MONTH_ADDR	(RTC_BASE_ADDRESS + RTC_MONTH_OFFSET)
#define	RTC_DATE_ADDR	(RTC_BASE_ADDRESS + RTC_DATE_OFFSET)
#define	RTC_DAY_ADDR	(RTC_BASE_ADDRESS + RTC_DAY_OFFSET)
#define	RTC_HOUR_ADDR	(RTC_BASE_ADDRESS + RTC_HOUR_OFFSET)
#define	RTC_MIN_ADDR	(RTC_BASE_ADDRESS + RTC_MIN_OFFSET)
#define	RTC_SEC_ADDR	(RTC_BASE_ADDRESS + RTC_SEC_OFFSET)
#define	RTC_CNTL_ADDR	(RTC_BASE_ADDRESS + RTC_CNTL_OFFSET)
#define	RTC_TIMEZONE_ADDR	(RTC_BASE_ADDRESS + RTC_TIMEZONE_OFFSET)
#define	RTC_DSTFLAG_ADDR	(RTC_BASE_ADDRESS + RTC_DSTFLAG_OFFSET)

/* Control Register Fields */
#define	RTC_CNTL_WRITE	0x80
#define	RTC_CNTL_READ	0x40

/* Masks for various fields */
#define	RTC_MONTH_MASK	0x1F
#define	RTC_DATE_MASK	0x3F
#define	RTC_DAY_MASK	0x07
#define	RTC_HOUR_MASK	0x3F
#define	RTC_MIN_MASK	0x7F
#define	RTC_SEC_MASK	0x7F
		
#define	RTC_ERROR_SEMA	1
#ifndef	OK
#define	OK	0
#endif

STATUS InitTime(void);
STATUS GetTime(struct tm *time);
STATUS SetTime(struct tm *time);

#ifdef  __cplusplus
}
#endif

#endif		/* _RTC_ */
