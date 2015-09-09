/*
 *
 * MODULE: systty.h - header file for the Exar UART
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 09/11/98	- Created by Sudhir
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


#ifndef		_SYSTTY_
#define		_SYSTTY_

extern	void	ttyinit(I32 port, I32 baud);
extern	void	ttyioctl(I32 port, I32 cmd, I32 arg);
extern	I32		ttyhit(I32 port);
extern	I32		ttyin(I32 port);
extern	I32		ttyout(I32 port, I32 data);

#endif		/* _SYSTTY_ */

