/*
 *
 * MODULE: dsx20.h - header file for the Temperature Sensor
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 09/18/98	- Created by Sudhir
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


#ifndef		_DSX20_
#define		_DSX20_

#define	DSX_SUCCESS		1
#define	DSX_FAIL		0

#define	DSX_INT_VECTOR		3
#define	DSX_HISR_STACK_SIZE 4096

/* These offsets will change */
#define	DSX_DQ_REG		((unsigned long *)0xB4000000)
#define	DSX_DQ_CLK		((unsigned long *)0xB4000004)
#define	DSX_DQ_RST		((unsigned long *)0xB4000008)


#define	DSX_CMD_WRITETH			0x01
#define	DSX_CMD_WRITETL			0x02
#define	DSX_CMD_WRITECONFIG		0x0C
#define	DSX_CMD_READCOUNTER		0xA0
#define	DSX_CMD_READTH			0xA1
#define	DSX_CMD_READTL			0xA2
#define	DSX_CMD_READSLOPE		0xA9
#define	DSX_CMD_READCONFIG		0xAC
#define	DSX_CMD_READTEMP		0xAA

#define	DSX_CMD_START			0xEE
#define	DSX_CMD_STOP			0x22

#define	DSX_CONFIG_DONE			0x80
#define	DSX_CONFIG_THF			0x40
#define	DSX_CONFIG_TLF			0x20
#define	DSX_CONFIG_NVB			0x10
#define	DSX_CONFIG_CPU			0x02
#define	DSX_CONFIG_1SHOT		0x01

extern	void  dsx_write(int, int);
extern	unsigned int dsx_read(int);
extern	int  dsx_init(int, int);
extern	void dsxisr();

#endif		/* _DSX20_ */
