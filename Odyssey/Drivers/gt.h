/*
 *
 * MODULE: tty.h - header file for the Exar UART
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

#ifndef		_GT_
#define		_GT_


#define		GT_SUCCESS				0
#define		GT_FAIL				-1

#define		GT_HISR_STACK_SIZE	4096

/* Interrupt Definitions */
#define		GT_INTSUM		0x00000001
#define		GT_CPUINTSUM	0x40000000

#define		GT_MEMOUT		0x00000002
#define		GT_DMAOUT		0x00000004
#define		GT_CPUOUT		0x00000008
#define		GT_DMA0_COMP	0x00000010
#define		GT_DMA1_COMP	0x00000020
#define		GT_DMA2_COMP	0x00000040
#define		GT_DMA3_COMP	0x00000080
#define		GT_T0EXP		0x00000100
#define		GT_T1EXP		0x00000200
#define		GT_T2EXP		0x00000400
#define		GT_T3EXP		0x00000800
#define		GT_MASRDERR0	0x00001000
#define		GT_SLVWRERR0	0x00002000
#define		GT_MASWRERR0	0x00004000
#define		GT_SLVRDERR0	0x00008000
#define		GT_ADDRERR0		0x00010000
#define		GT_MEMERR		0x00020000
#define		GT_MASABORT0	0x00040000
#define		GT_TARABORT0	0x00080000
#define		GT_RETRYCTR0	0x00100000

/* DMA Control Register Masks */
#define		GT_DMA_FLYBYEN		0x00000001
#define		GT_DMA_RDWRFLY		0x00000002
#define		GT_DMA_NOCHAIN_MODE	0x00000200
#define		GT_DMA_INT_MODE		0x00000400
#define		GT_DMA_BLOCK_MODE	0x00000800
#define		GT_DMA_CHAN_EN		0x00001000
#define		GT_DMA_FET_NEXTREC	0x00002000
#define		GT_DMA_ACT_STS		0x00004000
#define		GT_DMA_SDA			0x00008000
#define		GT_DMA_MDREQ		0x00010000
#define		GT_DMA_CDE			0x00020000
#define		GT_DMA_EOTE			0x00040000
#define		GT_DMA_EOTIE		0x00080000
#define		GT_DMA_ABR			0x00100000
#define		GT_DMA_DMAREG_TIMER	0x10000000

#define		GT_DMA_SDIR_MASK	0x0000000C
#define		GT_DMA_SDIR_INC		0x00000000
#define		GT_DMA_SDIR_DEC		0x00000004
#define		GT_DMA_SDIR_HOLD	0x00000008

#define		GT_DMA_DDIR_MASK	0x00000030
#define		GT_DMA_DDIR_INC		0x00000000
#define		GT_DMA_DDIR_DEC		0x00000010
#define		GT_DMA_DDIR_HOLD	0x00000020

#define		GT_DMA_XMIT_MASK	0x000001C0
#define		GT_DMA_XMIT_1		0x00000140
#define		GT_DMA_XMIT_2		0x00000180
#define		GT_DMA_XMIT_4		0x00000080
#define		GT_DMA_XMIT_8		0x00000000
#define		GT_DMA_XMIT_16		0x00000040
#define		GT_DMA_XMIT_32		0x000000C0
#define		GT_DMA_XMIT_64		0x000001C0

#define		GT_DMA_SLP_MASK		0x00600000
#define		GT_DMA_SLP_NO		0x00000000
#define		GT_DMA_SLP_PCI0		0x00200000
#define		GT_DMA_SLP_PCI1		0x00400000

#define		GT_DMA_DLP_MASK		0x01800000
#define		GT_DMA_DLP_NO		0x00000000
#define		GT_DMA_DLP_PCI0		0x00800000
#define		GT_DMA_DLP_PCI1		0x01000000

#define		GT_DMA_RLP_MASK		0x06000000
#define		GT_DMA_RLP_NO		0x00000000
#define		GT_DMA_RLP_PCI0		0x02000000
#define		GT_DMA_RLP_PCI1		0x04000000

#define     GT_ERR_NUM      	22
#define		PCI_READ_FFFFFFFF	0x00200000
/* Error Codes */
#define	I2O_ERROR_LOCAL_FREE	1

typedef	struct	_dmarec {
	U32	count;
	U32	src;
	U32	dst;
	U32	next;
} dmarec_t;


typedef	struct	_serv {
	U32	flag;
	U32	row;
	U32	type;
	U32	saddr;
	U32	daddr;
	U32	count;
	U32	dma_counter;
} serv_t;
#define MSIZE	(K(16) - 2)
/*
#define MSIZE	(K(1) - 2)
*/

typedef	struct _bufst {
	U32	size;
	U32	flag;
	unsigned long long	csum;
	unsigned long long	buf[MSIZE];
} buf_t;

extern buf_t *lbufs[];
extern buf_t *pbufs[];

extern int	configured;
extern int	verification;
extern int	display_flag;
extern int	stopped;
extern void	gt_init();
extern void	gt_initdma(U32 chan);
extern void gt_dma(U32 chan, U32 src, U32 dst, U16 len);
extern void gt_dmachain(U32 chan, U32 src, U32 dst, U32 len, U32 slot_no, U32 type);
extern void i2o_send_msg(int slot_no, U32 type, U32 size, U32 msgaddr);
extern void	galileo_dump();
extern U32	get_local_buf(int slot_no);
extern U32	get_pci_buf(int slot_no);


/* I2o queing functions support  */

#define	I2O_QNUM	4096
#define	I2O_QSIZE	(4 * I2O_QNUM)
#define	I2O_QMASK	(I2O_QSIZE-1)
#define	I2O_NUM_OF_QENTRIES(head, tail)	\
					((I2O_QSIZE + ((head) - (tail))) & I2O_QMASK)

#define	I2O_QLOW_WATER_NUM	4
#define	I2O_QLOW_WATER_MARK	(4 * I2O_QLOW_WATER_NUM)
#define	PCI_HOLD_CONTROL_ADDR	(0xBC0A8000)
#define	PCI_HOLD_CONTROL	((volatile U8 *)0xBC0A8000)

#define	I2O_FREEQINIT_NUM		4
		
#define	SOFT_QNUM	128
#define	SOFT_QSIZE	(4 *128)
#define	SOFT_QMASK	(SOFT_QSIZE - 1)

/* If you call InitI2oRegs() directly without calling InitI2oBuffers()
 * the commented fields should be initialized */
typedef	struct _i2o {
	U32	qbase;				/* Start of the i2o Que must be aligned to 1M */
	U32	infree_base;		/* Same as qbase */
	U32	inpost_base;		/* infree_base + I2O_QSIZE (minimum 16Kb) */
	U32	infree_head;		
	U32	inpost_tail;	
	U32	msg_base;			/* Start of Message buffer */
	U32	remoteI2optr[32];
	U32	soft_head[32];
	U32	soft_tail[32];
} i2o_t;

/* Do  not add any more fields to gtmsg_t structure
 * the size of this structure has to aligned at 64 bytes
 */
typedef	struct _gtmsg {
	/* Private fields, dont mess with this */
	U32		isI2o;
	U32		softQIndex;
	i2o_t	*i2oPtr;
	U32		retryCount;
	
	/* General fields for messages */
	U32		mType;
	U32		mSlot;
	U32		mCksm;
	U32		bAddr;
	U32		bSize;
	U32		dType;
	U64		dSeed;
	U64		dCksm;
	U32		vStat;
} gtmsg_t;

/* Defnitions for Message types */
#define	MTYPE_NOP			1
#define	MTYPE_READY			2
#define	MTYPE_PING			3
#define	MTYPE_PONG			4
#define	MTYPE_START_TRANSFER	5
#define	MTYPE_START_DMA		6
#define	MTYPE_END_DMA		7
#define	MTYPE_START_VERIFY	8
#define	MTYPE_END_VERIFY	9
#define	MTYPE_END_TRANSFER 10


#define MTYPE_STATISTICS_REQ	11
#define MTYPE_STATISTICS_RESP	12
#define MTYPE_STARTTEST_REQ		13
#define MTYPE_STARTTEST_RESP	14
#define MTYPE_STOPTEST_REQ		15

#define	MTYPE_HBC_READY			16


typedef	struct	_dmainfo {
	volatile U32	dmaflag;
	gtmsg_t	 *msg;
	unsigned long long	thruput;
	U32	dmalen;
	U32	dmatime;

	U32	slot_no;
	U32	type;
} dmainfo_t;
typedef	struct	_verifyst {
	volatile U32	verifyflag;
	gtmsg_t	 *msg;
	NU_EVENT_GROUP    startVerify;
} verify_t;


#define	VERIFYDONE()	( verifyCb.verifyflag?0:1 )
#define	VERIFYSTARTED()	 verifyCb.verifyflag = 1

#define	DMA_BSIZE 			K(32)
extern	dmainfo_t	dmainfo[];

#define	DMADONE(X)	( dmainfo[X].dmaflag?0:1 )
#define	DMASTARTED(X)	 dmainfo[X].dmaflag = 1
#define	DMAREC_SIZE		4096
#define DMA_SIZE    	( K(64) - 64)



typedef	struct _BoardDev {
	/* Board Status Info */
	U32	found;
	U32	ready;
	U32	reqRespActivity;
	volatile U32	respWaiting;
	gtmsg_t	*respMsg;
	/* I2o Message Info */
	U32	i2oMsgRxCount;
	U32	i2oMsgTxCount;
	/* Ping Info */
	U32	pingContinue;
	U32	pingRequestCount;
	U32	pingResponseCount;
	/* Dma Info */
	U32 dmaActive;
	U32	dmaContinue;
	U32	dmaCount;
	U32	dmaThruput;
	/* Verify Info */
	U32	doVerify;
	U32	verifySuccessCount;
	U32	verifyFailCount;
	U64 verifyDataWritten;
	U64 verifyDataRead;
	/* Bufer Info */
	U8 *srcBuf;
	U8 *dstBuf;
	U32 dstBufPci;
	/* Galileo Error Status */
	U32 errStat;
} BoardDev_t;


#define	CONSOLE_MAIN_MENU		101
#define CONSOLE_SYSTEM_MONITOR	100
#define CONSOLE_LOCAL			99
#define CONSOLE_LOCAL_2			98

extern i2o_t	*InitI2oBuffers(NU_MEMORY_POOL   *Mem_Pool);
extern void		InitI2oRegs(i2o_t *i2o_ptr);
extern STATUS	InitDataBuffers(NU_MEMORY_POOL *localPool, 
								NU_MEMORY_POOL *pciPool);
extern void		EnableGalileoIntHandler();
extern gtmsg_t 	*GetPostedMsg(i2o_t *i2o_ptr);
extern void		FreeMsg(gtmsg_t *msg);
extern gtmsg_t 	*GetRemoteFreeMsg(i2o_t *i2o_ptr, U32 slot_no);
extern void 	PostRemoteMsg(U32 slot_no, gtmsg_t *msg);
extern void 	DisplayHeader(U32 type, U32 slot_no, U32 flag);
extern void 	DisplayStats(BoardDev_t *BoardDevArray, U32 slot_no, U32 flag);
extern STATUS	GetRemoteStatistics(BoardDev_t *remoteBoardDevPtr, U32 slot_no);
extern void		process();
extern void 	VerifyProcess();
extern void		DisplayProcess();
extern void		SendReadyMsg();
extern void		SendHbcReadyMsg();
extern U32		StartDmaTest(U32 bitFlag);
extern void		StopDmaTest(U32 bitFlag);
extern void		StartRemoteTest();
extern void		StopRemoteTest();
extern U32		DoNopTest(U32 bitFlag);
extern void 	DisplayTestStartStat();

extern	i2o_t *i2odev;
extern	BoardDev_t BoardDev[];
extern	int	consoleIndex;
extern	U32 virtualAddressSpace;

#endif		/* _GT_ */
