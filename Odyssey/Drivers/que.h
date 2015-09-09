/*
 *
 * MODULE: que.h - header file for the que
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

#ifndef		_QUE_
#define		_QUE_

#ifdef  __cplusplus
extern "C" {
#endif


#define	QSIZE	1024
#define	QMASK	(QSIZE -1)

typedef	struct {
	int	type;
	int	slot_no;
	unsigned long	saddr;
	unsigned long	daddr;
	unsigned long	count;
} item_t;

typedef	struct	{
	item_t	item[QSIZE];
	unsigned int	rptr;
	unsigned int	wptr;
	unsigned int	bcount;
	item_t	item_null;
} que_t;


#define	QINIT(X)		{(X)->wptr =0; (X)->rptr =0; (X)->bcount =0;}
#define	QEMPTY(X)		(((X)->bcount == 0)? 1:0)
#define	QFULL(X)		(((X)->bcount >= QSIZE)?1:0 )
#define	QGET(X)			(((X)->bcount--)? (X)->item[(X)->rptr++ & QMASK] : \
										(X)->item_null)
#define	QPUT(X, Y)		{(X)->item[(X)->wptr++ & QMASK] = Y; (X)->bcount++;}
		
/* Message Que definitions */
#define	MSGQSIZE	4096
#define	MSGQMASK	(MSGQSIZE -1)


typedef	struct	{
	void 			*item[MSGQSIZE];
	unsigned int	rptr;
	unsigned int	wptr;
	unsigned int	bcount;
} msgque_t;


#define	MSGQINIT(X)		{(X)->wptr =0; (X)->rptr =0; (X)->bcount =0;}
#define	MSGQEMPTY(X)	(((X)->bcount == 0)? 1:0)
#define	MSGQFULL(X)		(((X)->bcount >= MSGQSIZE)?1:0 )
#define	MSGQGET(X)		(((X)->bcount--)? (X)->item[(X)->rptr++ & MSGQMASK] : \
										0 )
#define	MSGQPUT(X, Y)	{(X)->item[(X)->wptr++ & MSGQMASK] = Y; (X)->bcount++;}
	
		
#define	TYPE_READ	1
#define	TYPE_WRITE	2
#define	TYPE_CONFIG	3
#define	TYPE_READ_VERIFY	4
#define	TYPE_I2O_RETRY		5
#define	TYPE_STOP			6
#define	TYPE_READ_DONE	(TYPE_READ | 0x80)
#define	TYPE_WRITE_DONE	(TYPE_WRITE | 0x80)
#define	TYPE_CONFIG_DONE	(TYPE_CONFIG | 0x80)

#define	TYPE_READ_LOCAL_DONE	(TYPE_READ | 0x40)
#define	TYPE_WRITE_LOCALDONE	(TYPE_WRITE | 0x40)
#define	TYPE_CONFIG_LOCALDONE	(TYPE_CONFIG | 0x40)
#ifdef  __cplusplus
}
#endif
#endif		/* _QUE_ */
