/*
 *
 * MODULE: gt.c - driver for Galileo
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 12/31/98	- Created by Sudhir
 * 02/12/99 - Jim Frandeen: Change long long to I64
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
 * 
 * The Code does not support Multiple Channel DMA simultaneously 
 *
 */


#include "nucleus.h"
#include "types.h"
#include "hw.h"
#include "pcidev.h"
#include "pcimap.h"
#include "system.h"
#include "galileo.h"
#include "gt.h"
#include "pcialloc.h"
#include "que.h"
#include "ioptypes.h"
#include "bootblock.h"
#include "mips_util.h"

/* Software I2O queueing  implementation */
#define	I2O_SOFT			1

/* Workaround for I2O Problem */
//#define	WORKAROUND		1
/* Hardware workaround , else software workaround suggested by Galileo */
//#define WORKAROUND_HARD		1

/* For work around to work this need to be uncommented */
//#define NO_WATERMARK_CHECK	1

extern  bootblock_t bootblock;


#ifndef CONFIG_BOOT
U32	board_type;
#endif

/* Data needed for Nucleus stuff */
NU_HISR  	GT_HISR;


void DispatchI2o(gtmsg_t *msg);
void DispatchReqResp(gtmsg_t *msg);
void DispatchOther(gtmsg_t *msg);
void 		(*old_lisr_gt)(int);
unsigned char	*gt_hisr_stack = (unsigned char *)0;
/* extern 		NU_MEMORY_POOL   System_Memory;
*/
unsigned char gt_hisr_buf[GT_HISR_STACK_SIZE];


char *galileoErrMsg[GT_ERR_NUM] = {
		"None",
		"Memory Access Out of Range",
		"Dma Access Out of Range",
		"CPU Access Out of Range",
		"", "", "", "", "", "", "", "",
		"Parity Error : PCI Master Read",
		"Parity Error : PCI Slave Write",
		"Parity Error : PCI Master Write",
		"Parity Error : PCI Slave Read",
		"Parity Error : PCI Address Lines",
		"Parity Error : Memory",
		"PCI Master Abort",
		"PCI Target Abort",
		"PCI Retry Count Expired",
		"PCI Read returned 0xFFFFFFFF"
	};
/* Interrupt Handlers */
void 		gt_lih(int );
void		gtisr();

U32	virtualAddressSpace;
unsigned long long	sysclk1	= 75000000;
dmarec_t	*dmarec;
U8			dmarecbuf[DMAREC_SIZE * 16 + 18];
dmainfo_t	dmainfo[4];
verify_t	verifyCb;

/* Function proto for the driver, these functions are not exposed to the
 * User
 */
I32			gt_addintr(void (*pintr)());
U32	gt_int_mask = 0;

int display_flag = 0;
int consoleIndex = CONSOLE_MAIN_MENU;

/* New stuff */
void InitSoftQueue(i2o_t *i2o_ptr);
void StartDma(U32 chan, U32 src, U32 dst, U32 len, gtmsg_t *msg);
void HandleInboundPost();
void HandleDmaComplete(int chan);
void StartVerify(gtmsg_t *msg);
void DisplayMonitorHeader(U32 flag);
U32  VerifyBuffer(gtmsg_t *msg);
void InitPattern(U8 *buf, U32 len);
msgque_t	msgq_st;
msgque_t	*msgq = &msgq_st;
i2o_t *i2odev = (i2o_t *)0;
BoardDev_t	BoardDev[MAX_SLOTS];
BoardDev_t	*remoteBoardDev;



/* Start of Functions */
void
gt_init()
{
	U16 sts;
	
	/* Setup LISR and HISR for Galileo*/
	gt_addintr(gtisr);
	

	/* Intialize the hardware */
	/* Initialize the counter */
	gt_write(GT_TIMER0_OFF, 0x00FFFFFF);
	gt_write(GT_TIMER_CONTROL_OFF, 0x01);

	/* Clear the PCI Status Interrupt */
	sts =  pciconf_readw(0x80000000, PCI_CONF_STAT);
	pciconf_writew(0x80000000, PCI_CONF_STAT, sts);
	/* Clear the cuase */
	gt_write(GT_INT_CAUSE_OFF, 0xFFFFFFFF);
	/* Enable the Interrupts */
	/*
	gt_int_mask = GT_MEMOUT | GT_CPUOUT | GT_MASRDERR0 | GT_SLVWRERR0 
				  | GT_MASWRERR0 | GT_SLVRDERR0 | GT_ADDRERR0 | GT_MEMERR
				  | GT_MASABORT0 | GT_TARABORT0 | GT_RETRYCTR0 | GT_T0EXP;
	*/
	gt_int_mask |= GT_MEMOUT | GT_CPUOUT | GT_MASRDERR0 | GT_SLVWRERR0 
				  | GT_MASWRERR0 | GT_SLVRDERR0 | GT_ADDRERR0 | GT_MEMERR
				  | GT_MASABORT0 | GT_TARABORT0 | GT_RETRYCTR0 | GT_T0EXP ;
	gt_write(GT_INT_CPU_MASK_OFF, gt_int_mask);
	
	/* Clear In Message0 Interrupt */
	gt_write(0x1C00 + GT_I2O_INTIN_CAUSE_OFF, 0x01);
}

void
gt_initdma(U32 chan)
{
	if ( (chan < 0) || (chan > 3))
		return;
	/* Program the DMA Control Register for BLOCK Mode & 64 Byte Bursts*/
	gt_write(GT_DMA_CH0_CONTROL_OFF + (4 * chan), GT_DMA_NOCHAIN_MODE |
			GT_DMA_BLOCK_MODE | GT_DMA_SDIR_INC | GT_DMA_DDIR_INC |
			GT_DMA_XMIT_64 | GT_DMA_SLP_NO | GT_DMA_DLP_NO | GT_DMA_RLP_NO );
	/* Enable the DMA Completion Interrupt */
	gt_int_mask |= ( GT_DMA0_COMP << chan);
	gt_write(GT_INT_CPU_MASK_OFF, gt_int_mask);
	dmarec = (dmarec_t *)(ALIGN((U32)dmarecbuf, 16) | 0xA0000000);
	
}

/* The Code does not support Multiple Channel DMA simultaneously */
void
StartDma(U32 chan, U32 src, U32 dst, U32 len, gtmsg_t *msg)
{
	int offset;
	U32	val;
	U16	dsize;
	int	i;
	dmarec_t *dmarecp = dmarec;
	
	if ( (chan < 0) || (chan > 3))
		return;
	/* Init the Length and Message to the Dma Channel */
	dmainfo[chan].dmalen = len;
	dmainfo[chan].msg = msg;

	offset = chan * 4;
	/* Creat the Value to write to Control Register */
	val = GT_DMA_CHAN_EN | GT_DMA_BLOCK_MODE | GT_DMA_XMIT_64 |
		  GT_DMA_INT_MODE | GT_DMA_FET_NEXTREC;
	if ( gt_read(GT_CPU_CONFIG_OFF) & GT_PCI_2GIG ) {
		/* We are Using PCI 2G Sapce, the regular decoder is bypassed
		 * So we need to Set the Overrride bits for the Source and
		 * Destination Addresses
		 */
		if ( src >= PCI_WINDOW_START ) {
			val |= GT_DMA_SLP_PCI0;
		}
		if ( dst >= PCI_WINDOW_START ) {
			val |= GT_DMA_DLP_PCI0;
		}
	}
	/* Init the Fetch Register */
	gt_write(GT_DMA_CH0_NEXT_OFF + offset, VTOP((U32)dmarecp));

	/* Init the DMA Scatter Gather Desc */
	i = 0;
	while(len) {
		if ( len > DMA_SIZE) {
			dsize = DMA_SIZE;
			dmarecp->next = DWSWAP(VTOP((U32)dmarecp + sizeof(dmarec_t)));
		} else {
			dsize = len;
			dmarecp->next = 0;
		}
		dmarecp->src = DWSWAP(src);
		dmarecp->dst = DWSWAP(dst);
		dmarecp->count = DWSWAP(dsize);
		len	-= dsize;
		src	+= dsize;
		dst	+= dsize;
		dmarecp++;
	}
	/* Start DMA */
	gt_write(GT_DMA_CH0_CONTROL_OFF + offset, val);

	/* Start the Counter */
	 *((U32 *)(0xB4000850)) = 0xFFFFFFFF;
}

I32
gt_addintr(void (*pintr)())
{
	STATUS  status;

	/* This function should be executed only one, so if this is the
	 * nth time exit
	 */
	if( gt_hisr_stack )
		return(GT_SUCCESS);
	

#if 0	
	/* Allocate memory for HISR stack */
	status = NU_Allocate_Memory(&System_Memory,(void **)&(gt_hisr_stack),
				   GT_HISR_STACK_SIZE, NU_NO_SUSPEND);	
	if ( status != NU_SUCCESS ) {
		goto gt_addintr_abort;
	}
#else
	gt_hisr_stack = gt_hisr_buf;
#endif
		
	/* Create HISR */
	status = NU_Create_HISR(&GT_HISR, "gt_hisr", pintr, 2,
							gt_hisr_stack, GT_HISR_STACK_SIZE);
	if ( status != NU_SUCCESS ) {
		goto gt_addintr_abort;
	}
	
	/* Create Event for Handshake with the Verify Process */
	status = NU_Create_Event_Group(&verifyCb.startVerify, "STARTV");
	if (status != NU_SUCCESS)
		goto gt_addintr_abort;

	return(GT_SUCCESS);
	
gt_addintr_abort:
	
	/* In case of error deallocate the memories */
	if ( gt_hisr_stack)
		gt_hisr_stack = 0;
	printf("Error in GT: Interrupt Init\n\r");
	return(GT_FAIL);

}

void
EnableGalileoIntHandler()
{
	STATUS status;
	
	/* Rigister LISR */
	status = NU_Register_LISR(GT_CPU_INT_INDEX, gt_lih, &old_lisr_gt);
}

void
gt_lih(int vector)
{
	/* Get the timer value */
	U32 gtimer = *((U32 *)(0xB4000850));

	/* Use the TIMER value for DMA 0 thruput calculations */
	if ( gt_read(GT_INT_CAUSE_OFF) & GT_DMA0_COMP ) {
		dmainfo[0].dmatime = DWSWAP(gtimer);
	}

	/* Use the TIMER value for DMA 1 thruput calculations */
	if ( gt_read(GT_INT_CAUSE_OFF) & GT_DMA1_COMP ) {
		dmainfo[1].dmatime = DWSWAP(gtimer);
	}

	/* Disable Galileo Interrupt */
	gt_write(GT_INT_CPU_MASK_OFF, 0);

	/* Disable In Message0 Interrupt */
	gt_write(0x1C00 + GT_I2O_INTIN_MASK_OFF, 0x37);

	/* Activate the HISR */
	NU_Activate_HISR(&GT_HISR);
#pragma unused (vector)
}


/* ------------------------------Start of Support for I2O Ques -----------*/

#ifndef CONFIG_BOOT
void
gtisr()
{
	U32	int_sum, int_clear;
	U16 sts;
	U32	errStat = 0;

	/* Message0 Interrupt */
	if ( gt_read(0x1C00 + GT_I2O_INTIN_CAUSE_OFF) & 0x01) {
		/* Got an I2O In Message0 Interrupt */

		/* Clear I2O In Message0 Interrupt */
		gt_write(0x1C00 + GT_I2O_INTIN_CAUSE_OFF, 0x01);

		/* Enable In Message0 Interrupt */
		gt_write(0x1C00 + GT_I2O_INTIN_MASK_OFF, 0x36);
		/* clear the Doorbell Rigister */
		gt_write(0x1C00 + GT_I2O_IN_DOORBELL_OFF, 0xFFFFFFFF);
	}
	/* Inbound Post Que interrupt */
	if ( gt_read(0x1C00 + GT_I2O_INTIN_CAUSE_OFF) & 0x10) {
		/* Got an I2O Inbound Post Que Interrupt */ 
		HandleInboundPost();

		/* Clear I2O Inbound Post Que Interrupt */
		gt_write(0x1C00 + GT_I2O_INTIN_CAUSE_OFF, 0x10);

		/* Enable Inbound Post Que Interrupt */
		gt_write(0x1C00 + GT_I2O_INTIN_MASK_OFF, 0x27);
	}
	/* Read the other Interrupt Summary */
	int_sum = gt_read(GT_INT_CAUSE_OFF);
	if ( int_sum & GT_INTSUM ) {
		int_clear = ~int_sum;
		int_sum &= gt_int_mask;
		/* Memory Access Out of range */
		if ( int_sum & GT_MEMOUT) {
			errStat |= GT_MEMOUT;
			/* Clear the Interrupt */
			gt_write(GT_INT_CAUSE_OFF, ~GT_MEMOUT);
		}
		/* Dma Access Out of range */
		if ( int_sum & GT_DMAOUT) {
			errStat |= GT_DMAOUT;
			/* Clear the Interrupt */
			gt_write(GT_INT_CAUSE_OFF, ~GT_DMAOUT);
		}
		/* CPU Access Out of range */
		if ( int_sum & GT_CPUOUT) {
			errStat |= GT_CPUOUT;
			/* Clear the Interrupt */
			gt_write(GT_INT_CAUSE_OFF, ~GT_CPUOUT);
		}
		/* Timer 0 expired */
		if ( int_sum & GT_T0EXP) {
			/* Clear the Interrupt */
			gt_write(GT_INT_CAUSE_OFF, ~GT_T0EXP);
		}
		/* Parity Error during PCI Master Read */
		if ( int_sum & GT_MASRDERR0) {
			errStat |= GT_MASRDERR0;
			/* Clear the Interrupt */
			gt_write(GT_INT_CAUSE_OFF, ~GT_MASRDERR0);
		}
		/* Parity Error During PCI Slave Write */
		if ( int_sum & GT_SLVWRERR0) {
			errStat |= GT_SLVWRERR0;
			/* Clear the Interrupt */
			gt_write(GT_INT_CAUSE_OFF, ~GT_SLVWRERR0);
		}
		/* Parity Error during PCI Master Write */
		if ( int_sum & GT_MASWRERR0) {
			errStat |= GT_MASWRERR0;
			/* Clear the Interrupt */
			gt_write(GT_INT_CAUSE_OFF, ~GT_MASWRERR0);
		}
		/* Parity Error During PCI Slave Read */
		if ( int_sum & GT_SLVRDERR0) {
			errStat |= GT_SLVRDERR0;
			/* Clear the Interrupt */
			gt_write(GT_INT_CAUSE_OFF, ~GT_SLVRDERR0);
		}
		/* Parity Error in PCI Address Lines */
		if ( int_sum & GT_ADDRERR0) {
			errStat |= GT_ADDRERR0;
			/* Clear the Interrupt */
			gt_write(GT_INT_CAUSE_OFF, ~GT_ADDRERR0);
		}
		/* Memory Parity Error */
		if ( int_sum & GT_MEMERR) {
			errStat |= GT_MEMERR;
			/* Clear the Interrupt */
			gt_write(GT_INT_CAUSE_OFF, ~GT_MEMERR);
		}
		
		if ( int_sum & GT_DMA0_COMP) {
			/* Clear the Interrupt */
			gt_write(GT_INT_CAUSE_OFF, ~GT_DMA0_COMP);
			/* Disable the DMA Channel */
			gt_write(GT_DMA_CH0_CONTROL_OFF, 
						(gt_read(GT_DMA_CH0_CONTROL_OFF) & ~GT_DMA_CHAN_EN));
			/* Handle the Dma complete for Channel 0 */
			HandleDmaComplete(0);

		}
		if ( int_sum & GT_DMA1_COMP) {
			/* Clear the Interrupt */
			gt_write(GT_INT_CAUSE_OFF, ~GT_DMA1_COMP);
			/* Disable the DMA Channel */
			gt_write(GT_DMA_CH1_CONTROL_OFF, 
						(gt_read(GT_DMA_CH1_CONTROL_OFF) & ~GT_DMA_CHAN_EN));
			/* Handle the Dma complete for Channel 1 */
			HandleDmaComplete(1);
		}
		if ( int_sum & GT_DMA2_COMP) {
			/* Clear the Interrupt */
			gt_write(GT_INT_CAUSE_OFF, ~GT_DMA2_COMP);
			/* Disable the DMA Channel */
			gt_write(GT_DMA_CH2_CONTROL_OFF, 
						(gt_read(GT_DMA_CH2_CONTROL_OFF) & ~GT_DMA_CHAN_EN));
			/* Handle the Dma complete for Channel 2 */
			HandleDmaComplete(2);
		}
		if ( int_sum & GT_DMA3_COMP) {
			/* Clear the Interrupt */
			gt_write(GT_INT_CAUSE_OFF, ~GT_DMA3_COMP);
			/* Disable the DMA Channel */
			gt_write(GT_DMA_CH3_CONTROL_OFF, 
						(gt_read(GT_DMA_CH3_CONTROL_OFF) & ~GT_DMA_CHAN_EN));
			/* Handle the Dma complete for Channel 3 */
			HandleDmaComplete(3);
		}

		/* PCI Master Abort */
		if ( int_sum & GT_MASABORT0) {
			//errStat |= GT_MASABORT0;
			/* Clear the Interrupt */
			gt_write(GT_INT_CAUSE_OFF, ~GT_MASABORT0);
			/* Clear the PCI Status Register in Config Space */
			sts =  pciconf_readw(0x80000000, PCI_CONF_STAT);
			pciconf_writew(0x80000000, PCI_CONF_STAT, sts);
		}

		/* PCI Target Abort */
		if ( int_sum & GT_TARABORT0) {
			errStat |= GT_TARABORT0;
			/* Clear the Interrupt */
			gt_write(GT_INT_CAUSE_OFF, ~GT_TARABORT0);
			/* Clear the PCI Status Register in Config Space */
			sts =  pciconf_readw(0x80000000, PCI_CONF_STAT);
			pciconf_writew(0x80000000, PCI_CONF_STAT, sts);
			
			
			/* Stop All Tests */
			StopDmaTest(0xFFFFFFFF);
			/* Disable the DMA Channel and Abort */
			gt_write(GT_DMA_CH0_CONTROL_OFF, 
						(gt_read(GT_DMA_CH0_CONTROL_OFF) & ~GT_DMA_CHAN_EN)
						| GT_DMA_ABR);
		}
		/* PCI Retry Count Expired */
		if ( int_sum & GT_RETRYCTR0) {
			errStat |= GT_RETRYCTR0;
			/* Clear the Interrupt */
			gt_write(GT_INT_CAUSE_OFF, ~GT_RETRYCTR0);

			/* Stop All Tests */
			StopDmaTest(0xFFFFFFFF);
			/* Disable the DMA Channel and Abort */
			gt_write(GT_DMA_CH0_CONTROL_OFF, 
						(gt_read(GT_DMA_CH0_CONTROL_OFF) & ~GT_DMA_CHAN_EN)
						| GT_DMA_ABR);
		}
			
	}
	/* Assign the errStat to the BoardDev */
	if ( errStat )
		BoardDev[bootblock.b_slot].errStat = errStat;

	/* Enable Inbound Post Que Interrupt */
	gt_write(0x1C00 + GT_I2O_INTIN_MASK_OFF, 0x27);

	/* Enable Galileo Ints */
	gt_write(GT_INT_CPU_MASK_OFF, gt_int_mask);
}
#endif


/* This function processes the Inbound Post Q Interrupt */
void
HandleInboundPost()
{
	gtmsg_t	*msg;
	while ((msg = GetPostedMsg(i2odev)) != (gtmsg_t *)0xFFFFFFFF) {
		/* We found an entry */
		msg->i2oPtr = i2odev;
		/* Set the Message isI2o flag */
		msg->isI2o = 1;
		/* Set the retry count to 0 */
		msg->retryCount  = 0;

		/* Crit Section */
		DISABLE_INT;
		/* Insert it to Message Q */
		MSGQPUT(msgq, msg);
		/* End Crit Section */
		ENABLE_INT;
	}
}
/* This function processes the Dma Complete Interrupt */
void
HandleDmaComplete(int chan)
{
	U32 val;
	gtmsg_t *msg = dmainfo[chan].msg;
	/* Get the time taken fro the DMA to complete */
	val = 0xFFFFFFFF - dmainfo[chan].dmatime;
	/* Calculate the DMA Thruput */
	if (val ) {
		 dmainfo[chan].thruput = (dmainfo[chan].dmalen * sysclk1 * 8) / val;
		 /* Convert it into Mbits/sec */
		 dmainfo[chan].thruput = dmainfo[chan].thruput >> 20;
	} else { 
		dmainfo[chan].thruput = 0;
	}
	if (!msg) {
		/* Flag the Dma is over */
		dmainfo[chan].dmaflag = 0;
		return;
	}
	/* Store the DMA Thruput into the BoardDev struct in
	 * Mbits/sec */
	if ( dmainfo[chan].thruput)
		BoardDev[msg->mSlot].dmaThruput = dmainfo[chan].thruput;
	/* Crit Section */
	DISABLE_INT;
	/* Reset the Time taken for Dma */
	dmainfo[chan].dmatime = 0;
	/* Flag the Dma is over */
	dmainfo[chan].dmaflag = 0;
	dmainfo[chan].msg     = 0;
	/* This Message is not coming from Remote, so
	 * reset isI2o flasg */
	msg->isI2o = 0;
	/* Set the retry count to 0 */
	msg->retryCount  = 0;
	/* Set the Message type */
	msg->mType = MTYPE_END_DMA;
	/* Insert it to Message Q */
	MSGQPUT(msgq, msg);
	/* End Crit Section */
	ENABLE_INT;
}


/* This function allocates the Mem required for the
 * I2O Queues and Messages */
i2o_t *
InitI2oBuffers(NU_MEMORY_POOL   *Mem_Pool)
{
	int i;
	U32	*ptr;
	gtmsg_t *gtmsg;
	U32	addr;
	i2o_t *i2o_ptr;

	/* If virtualAddressSpace is not initialized init it now */
	if (!virtualAddressSpace)
		virtualAddressSpace = (U32)Mem_Pool & 0xE0000000;
	/* Allocate Meomry for remoteBoardDev */
	NU_Allocate_Memory(Mem_Pool, (void **)&addr,
					(sizeof(BoardDev_t) * MAX_SLOTS) + 64, NU_NO_SUSPEND);
	remoteBoardDev = (BoardDev_t *)addr;

	/* Allocate Memory for Message Buffer*/
	NU_Allocate_Memory(Mem_Pool, (void **)&addr,
					sizeof(i2o_t)+64, NU_NO_SUSPEND);

	/* Align at 64 byte boundary */
	addr = ALIGN(addr, 64);
	/* Always keep i2o_ptr in uncached memory */
	addr = 0xA0000000 | addr;
	i2o_ptr = (i2o_t *)addr;
	/* Allocate Memory for Inbound Free and Post Queue */ 
	NU_Allocate_Memory(Mem_Pool, (void **)&addr, (I2O_QSIZE *2 )+M(1), 
					NU_NO_SUSPEND);
	/* Align the addr to 1M boundary */
	addr = ALIGN(addr, M(1));
	
	/* Always keep  the queues in uncached memory */
	addr = 0xA0000000 | addr;
	i2o_ptr->qbase = addr;
	i2o_ptr->infree_base = addr;
	i2o_ptr->inpost_base = addr + I2O_QSIZE;


	/* Allocate Memory for Message Buffer*/
	NU_Allocate_Memory(Mem_Pool, (void **)&addr,
					(I2O_QNUM * sizeof(gtmsg_t))+64, NU_NO_SUSPEND);
	addr = ALIGN(addr, 64);
	i2o_ptr->msg_base = addr;
	
	ptr = (U32 *)i2o_ptr->infree_base;
	gtmsg = (gtmsg_t *)i2o_ptr->msg_base;
	/* Assign the Message Buffer to Inbound Free Queue */
	for(i=0; i < I2O_QNUM; i++) {
		ptr[i] = VTOP((U32)(&gtmsg[i]));
	}
	
	i2o_ptr->infree_head = i2o_ptr->infree_base + I2O_QSIZE - 4;
	i2o_ptr->inpost_tail = i2o_ptr->inpost_base;
	
	/* Initialize the Message Q , Maintained by the Driver */
	MSGQINIT(msgq);

	return(i2o_ptr);
}

/* This funciotn Initializes the Galileo for the
 * I2O Queue opration  */
void
InitI2oRegs(i2o_t *i2o_ptr)
{
	int i;
	U32	*ptr;
	gtmsg_t *gtmsg;
	U32	i2oval;
	
	
	/* Assign the Message Buffer to Inbound Free Queue,
	 * I do it again, to support non-nucleus builds, in which
	 * case all you have to do is init the corresponding
	 * fields in i2o_ptr( dont call init_i2obuffers())
	 * and call this function directly */
	ptr = (U32 *)i2o_ptr->infree_base;
	gtmsg = (gtmsg_t *)i2o_ptr->msg_base;
	for(i=0; i < I2O_QNUM; i++) {
		/* Init the retryCount to 0 */
		gtmsg[i].retryCount = 0;
		/* Assign the Messages to the Q entries */
		ptr[i] = VTOP((U32)(&gtmsg[i]));
	}
	
	i2o_ptr->inpost_tail = i2o_ptr->inpost_base;
#ifdef	I2O_SOFT
	/* Make the Inbound Free Q empty */
	i2o_ptr->infree_head = i2o_ptr->infree_base;
	/* Initialize the Software Queue for each slot */
	InitSoftQueue(i2o_ptr);
#else
	/* Make free Q full */
	//i2o_ptr->infree_head = i2o_ptr->infree_base + I2O_QSIZE - 4;
	/* Make Free Q to have just 2 entries */
	i2o_ptr->infree_head = i2o_ptr->infree_base + 8;
	/* Do this only if the Q has few entries */
	for(i=0; i < 32; i++) {
		if ( BoardDev[i].found)
			BoardDev[i].ready = 1;
	}
		
#endif
	/* Sync the gtmsg to the device */
	mips_sync_cache((void *)(i2o_ptr->msg_base), (I2O_QNUM * sizeof(gtmsg_t)), 
					SYNC_CACHE_FOR_DEV);
	

	/* Set the Que size(16 Kbytes) and disable the que 
	 * till we finish the initialization */
	gt_write(0x1C00 + GT_I2O_QCTRL_OFF, 0x2);
	
	/* Initialize the QBASE Register */
	gt_write(0x1C00 + GT_I2O_QBAR_OFF, VTOP(i2o_ptr->qbase));
	
	/* Initialize the Inbound Free Head Pointer */
	gt_write(0x1C00 + GT_I2O_INFREE_HEAD_OFF, VTOP(i2o_ptr->infree_head));

	/* Initialize the Inbound Free Tail Pointer */
	gt_write(0x1C00 + GT_I2O_INFREE_TAIL_OFF, VTOP(i2o_ptr->infree_base));

	/* Initialize the Inbound Post Head Pointer */
	gt_write(0x1C00 + GT_I2O_INPOST_HEAD_OFF, VTOP(i2o_ptr->inpost_base));

	/* Initialize the Inbound Post Tail Pointer */
	gt_write(0x1C00 + GT_I2O_INPOST_TAIL_OFF, VTOP(i2o_ptr->inpost_tail));
	
	/* enable the Interrupt for Inbound Post Que Interrupt
	 * Clear the Interrupt first in the cause register,
	 * then enabl;e it */
	gt_write(0x1C00 + GT_I2O_INTIN_CAUSE_OFF, 0x10);
	gt_write(0x1C00 + GT_I2O_INTIN_MASK_OFF, 0x27);
	
	/* Now enable the que */
	gt_write(0x1C00 + GT_I2O_QCTRL_OFF, 
				gt_read(0x1C00 + GT_I2O_QCTRL_OFF) | 0x1);
	
	/* Write the Virtual PCI Base address of the i2odev to the 
	 * Outbound Message Reg 0*/
	i2oval = VTOP((U32)i2o_ptr);
	i2oval = (memmaps.pciSlave + i2oval - memmaps.paSlave) - 0x80000000;
	/* write to the galileo , no need to swap it*/
	*((U32 *)(0xB4001C00 + GT_I2O_OUTMSG_0_OFF)) = i2oval;

	return;
		
}

void 
InitSoftQueue(i2o_t *i2o_ptr)
{
	U32	qbase;
	gtmsg_t *gtmsg;
	int i, softQIndex;

	/* Pointer to the Start of the Msg Base */
	gtmsg = (gtmsg_t *)i2o_ptr->msg_base;
	qbase = i2o_ptr->infree_base;

	/* For HBC0 */
	softQIndex = 0;
	/* Inittilise the slot number in the msg */
	for(i=0; i < SOFT_QNUM; i++) {
		gtmsg->softQIndex = softQIndex;
		gtmsg++;
	}

	/* Inittialize the head and tail */
	i2o_ptr->soft_tail[softQIndex] = qbase;
	i2o_ptr->soft_head[softQIndex] = qbase + SOFT_QSIZE - 4;
	i2o_ptr->remoteI2optr[softQIndex] = 0;
	qbase += SOFT_QSIZE;
	/* For HBC1 */
	softQIndex = 1;
	/* Inittilise the slot number in the msg */
	for(i=0; i < SOFT_QNUM; i++) {
		gtmsg->softQIndex = softQIndex;
		gtmsg++;
	}
	/* Inittialize the head and tail */
	i2o_ptr->soft_tail[softQIndex] = qbase;
	i2o_ptr->soft_head[softQIndex] = qbase + SOFT_QSIZE - 4;
	i2o_ptr->remoteI2optr[softQIndex] = 0;
	qbase += SOFT_QSIZE;
	/* For Rest of the Slots */
	for(softQIndex=16; softQIndex < 32; softQIndex++) {

		/* Inittilise the slot number in the msg */
		for(i=0; i < SOFT_QNUM; i++) {
			gtmsg->softQIndex = softQIndex;
			gtmsg++;
		}
		/* Inittialize the head and tail */
		i2o_ptr->soft_tail[softQIndex] = qbase;
		i2o_ptr->soft_head[softQIndex] = qbase + SOFT_QSIZE - 4;
		i2o_ptr->remoteI2optr[softQIndex] = 0;
		qbase += SOFT_QSIZE;
	}
}

/* Initializes Local(src buffers) and dst Buffers */
STATUS
InitDataBuffers(NU_MEMORY_POOL *localPool, NU_MEMORY_POOL *pciPool)
{
	int i;
	STATUS rc;
	U8	*ptr;
	
	for(i=0; i < MAX_SLOTS; i++) {
		/* No Buffers corresponding to this board, because
		 * there is no i2o message/transfer where src and dst is 
		 * same(this board) */
		if ( i ==  bootblock.b_slot)
			continue;
		/* Initilaize the buffers only if the board is present */
		if (BoardDev[i].found) {
			/* Allocate the Local Buffer */
			rc = NU_Allocate_Memory(localPool, (VOID **)&ptr,
					(DMA_BSIZE + 64), NU_NO_SUSPEND);
			if ( rc != NU_SUCCESS) {
				printf("Error in Allocating Local Pool\n\r");
				return(rc);
			}
			
			
			/* Align the Buffer to 64 bytes */
			ptr = (U8 *)(ALIGN((U32)ptr, 64));
			/* Assign it to the BoardDev */
			BoardDev[i].srcBuf = ptr;

			/*Initialize the Pattern, to src Buffer */
			InitPattern(BoardDev[i].srcBuf, DMA_BSIZE);

			/* Allocate the Pci Buffer */
			rc = NU_Allocate_Memory(pciPool, (VOID **)&ptr,
					(DMA_BSIZE + 64), NU_NO_SUSPEND);
			if ( rc != NU_SUCCESS) {
				printf("Error in Allocating Pci Pool\n\r");
				return(rc);
			}
			
			/* Align the Buffer to 64 bytes */
			ptr = (U8 *)(ALIGN((U32)ptr, 64));
			/* Assign it to the BoardDev */
			BoardDev[i].dstBuf = ptr;
			/* Convert the Address to Pci Space */
			BoardDev[i].dstBufPci = memmaps.pciSlave + 
					(VTOP((U32)ptr) - REMAP_ADDRESS);
		}
	}
	return(OK);
}

void
InitPattern(U8 *buf, U32 len)
{
	U64 *lPtr;
	U64 pattern;
	U64 shiftPattern;
	int i;
	int tlen = len;
	
	/* Make a 64-bit pointer */
	lPtr = (U64 *)buf;
	/* Since we are writing 64-bit wide, len becomes
	 * number of quad-words in the buffer = len/8 */
	len = len >> 3;
	i = 0;
	/* This variable is used for shifting 1's and 0's */
	shiftPattern = 1;

	while(len) {
		/* Pattern 0 */
		pattern = 0x0000000000000000;
		if ( len ) {
			lPtr[i] = pattern;
			i++; len--;
		}
		/* Pattern 0xFFFFFFFFFFFFFFFF */
		if ( len ) {
			lPtr[i] = ~pattern;
			i++; len--;
		}
		/* Pattern 0x5A5A5A5A5A5A5A5A */
		pattern = 0x5A5A5A5A5A5A5A5A;
		if ( len ) {
			lPtr[i] = pattern;
			i++; len--;
		}
		/* Pattern 0xA5A5A5A5A5A5A5A5 */
		if ( len ) {
			lPtr[i] = ~pattern;
			i++; len--;
		}
		/* If shiftPattern is shifted the 1 out logically after
		 * 64 times, reinit it to 1 again */
		if ( !shiftPattern)
			shiftPattern = 1;
		/* Pattern = shifting 1 */
		pattern = shiftPattern;
		if ( len ) {
			lPtr[i] = pattern;
			i++; len--;
		}
		/* Pattern = shifting 0 */
		if ( len ) {
			lPtr[i] = ~pattern;
			i++; len--;
		}
		/* Shit the 1 one bit left */
		shiftPattern = shiftPattern << 1;
	}
}

/* This function returns a pointer to the message,
 * which was posted by the remote PCI board */

gtmsg_t *
GetPostedMsg(i2o_t *i2o_ptr)
{
	U32 pMsg;

	/* Enter Crit Section */
	DISABLE_INT;

	/* Check whether Post Que is empty ? */
	if ( gt_read(0x1C00 + GT_I2O_INPOST_HEAD_OFF) ==
			VTOP(i2o_ptr->inpost_tail)) {
		/* Out of Crit Section */
		ENABLE_INT;
		return((gtmsg_t *)0xFFFFFFFF);
	}
	
	/* Get the Message */
	pMsg = *((U32 *)(i2o_ptr->inpost_tail));
	
	/* Update the Inbound Post Tail Pointer */
	i2o_ptr->inpost_tail = i2o_ptr->inpost_base |
							((i2o_ptr->inpost_tail + 4) & I2O_QMASK); 

	/* Write  the Inbound Post Tail Pointer to Galileo */
	gt_write(0x1C00 + GT_I2O_INPOST_TAIL_OFF, VTOP(i2o_ptr->inpost_tail));
	
	/* Out of Crit Section */
	ENABLE_INT;
	/* Get the Virtual Address */
	pMsg |= virtualAddressSpace;
	/* Sync the mem for CPU */
	mips_sync_cache((void *)pMsg, sizeof(gtmsg_t), SYNC_CACHE_FOR_CPU);
	return((gtmsg_t *)pMsg);
}


#ifdef	I2O_SOFT
/* This function is called to free the message back to the
 * free que, which was earlier retuned by the getPostedMsg()
 * call */
void
FreeMsg(gtmsg_t *msg)
{
	U32	softQIndex 	= msg->softQIndex;
	i2o_t *i2o_ptr 	= msg->i2oPtr;
	register U32	soft_head;
	
	/* Sync the mem for Dev */
	mips_sync_cache((void *)msg, sizeof(gtmsg_t), SYNC_CACHE_FOR_DEV);
	/* Enter Crit Section */
	DISABLE_INT;

	soft_head = i2o_ptr->soft_head[softQIndex];
	/* Free the Message */
	*((U32 *)(soft_head)) = VTOP((U32)msg);

	/* Update the Software Q head for this slot */
	i2o_ptr->soft_head[softQIndex] = (soft_head & ~SOFT_QMASK) | 
									((soft_head + 4) & SOFT_QMASK);
	/* Out of Crit Section */
	ENABLE_INT;
		
}


/* This function returns the next available free msg
 * from the remote board, addressed by slot_no */
gtmsg_t *
GetRemoteFreeMsg(i2o_t *i2o_ptr, U32 slot_no)
{
	i2o_t *remoteI2optr = (i2o_t *)(i2o_ptr->remoteI2optr[slot_no]);
	U32	mySlot = bootblock.b_slot;
	register U32	soft_tail;
	volatile U32 ptr;
	
	if (remoteI2optr == 0) {
		volatile U32 *OutMsg0;
		/* This will be executed only once for each slot */
		/* Get the virtual address of Outbound Mæssage Register 0 on 
	 	 * Remote board */
		OutMsg0 = (U32 *)(memmaps.aPaPci[slot_no] + GT_I2O_OUTMSG_0_OFF - 
					  0x80000000);
	
		/* Get the i2odev address of the remote board */
		remoteI2optr = (i2o_t *)(*OutMsg0);
		if ( remoteI2optr == (i2o_t *)0xFFFFFFFF)
			remoteI2optr = 0;
		i2o_ptr->remoteI2optr[slot_no] = (U32)remoteI2optr;
	
		/* if remoteI2optr == 0 , the remote iß not ready yet */
		if (remoteI2optr == 0)
			return((gtmsg_t *)0xFFFFFFFF);
	}

	/* Enter Crit Section */
	DISABLE_INT;
	
	/* Get the tail of the que */
	soft_tail = remoteI2optr->soft_tail[mySlot];

	/* Chack for Target Abort */
	if ( soft_tail == 0xFFFFFFFF) {
		/* Out of Crit Section */
		ENABLE_INT;
		BoardDev[bootblock.b_slot].errStat = PCI_READ_FFFFFFFF;
		return((gtmsg_t *)0xFFFFFFFF);
	}
	
	/* check whether Q is empty */
	if ( soft_tail == remoteI2optr->soft_head[mySlot]) {
		/* Out of Crit Section */
		ENABLE_INT;
		/* Que is empty */
		return((gtmsg_t *)0xFFFFFFFF);
	}
	
	ptr = *((U32 *)((soft_tail & 0x1FFFFFFF) + memmaps.aPaPci[slot_no] 
			- 0x80000000 - REMAP_ADDRESS));

	/* Chack for Target Abort */
	if ( ptr == 0xFFFFFFFF) {
		/* Out of Crit Section */
		ENABLE_INT;
		BoardDev[bootblock.b_slot].errStat = PCI_READ_FFFFFFFF;
		return((gtmsg_t *)0xFFFFFFFF);
	}

	/* Update the remote tail */
	remoteI2optr->soft_tail[mySlot] = (soft_tail & ~SOFT_QMASK) |
							((soft_tail + 4) & SOFT_QMASK);

	/* Out of Crit Section */
	ENABLE_INT;

	/* Convert the Message address into virtual */
	ptr = memmaps.aPaPci[slot_no] + ptr - 0x80000000 - REMAP_ADDRESS;
	return((gtmsg_t *)ptr);
}
#else /* I2O_SOFT */

void
FreeMsg(gtmsg_t *msg)
{
	i2o_t *i2o_ptr = msg->i2oPtr;
	U32	in_free_tail;
	void pcihold_update_index(U32 addr, U32 index);

	/* Enter Crit Section */
	DISABLE_INT;

	/* Free the Message */
	*((U32 *)(i2o_ptr->infree_head)) = VTOP((U32)msg);

#ifdef NO_WATERMARK_CHECK
#pragma unused (in_free_tail)
	/* Check whether Free Que is empty ? */
	if ( gt_read(0x1C00 + GT_I2O_INFREE_TAIL_OFF) ==
			VTOP(i2o_ptr->infree_head)) 
#else /* NO_WATERMARK_CHECK  */
	/* Get the Inbound Free Tail */
	in_free_tail = gt_read(0x1C00 + GT_I2O_INFREE_TAIL_OFF);
	/* Check the number of entries in the free que, if it is less
	 * than the I2O_QLOW_WATER_MAR, ask the EPLD to hold the PCI Bus
	 * so that remote boards cannot issue PCI read to get the free
	 * messages , errata # 14 for Galileo
	 */
	if ( I2O_NUM_OF_QENTRIES(VTOP(i2o_ptr->infree_head), in_free_tail)
				   	<= I2O_QLOW_WATER_MARK) 
#endif /* NO_WATERMARK_CHECK */
	{
			
		/* Update the Inbound Free Head Pointer */
		i2o_ptr->infree_head = i2o_ptr->infree_base |
							((i2o_ptr->infree_head + 4) & I2O_QMASK);
#ifdef WORKAROUND
		/* Update the Inbound Free Head pointer */
		pcihold_update_index(0xB4001C00 + GT_I2O_INFREE_HEAD_OFF, 
										i2o_ptr->infree_head);
#else /* WORKAROUND */
		/* Write the Head to Galileo */
		gt_write(0x1C00 + GT_I2O_INFREE_HEAD_OFF, VTOP(i2o_ptr->infree_head));
#endif /* WORKAROUND */

	} else {
			
		/* Update the Inbound Free Head Pointer */
		i2o_ptr->infree_head = i2o_ptr->infree_base |
							((i2o_ptr->infree_head + 4) & I2O_QMASK);
		/* Write it to Galileo */
		gt_write(0x1C00 + GT_I2O_INFREE_HEAD_OFF, VTOP(i2o_ptr->infree_head));
			
	}
	
	/* Out of Crit Section */
	ENABLE_INT;
		
}


/* This function returns the next available free msg
 * from the remote board, addressed by slot_no */
gtmsg_t *
GetRemoteFreeMsg(i2o_t *i2o_ptr, U32 slot_no)
{
	volatile U32 *InQPVReg;
	volatile U32 ptr;
#pragma unused (i2o_ptr)
	
	/* Get the virtual address of Inbound Que Port Virtaul Register
	 * Address of the Remote board */
	InQPVReg = (U32 *)(memmaps.aPaPci[slot_no] + GT_I2O_INQ_VR_OFF - 
					  0x80000000);
	
	/* Get the physical address of the Message */
	ptr = *InQPVReg;
	if ( ptr == 0xFFFFFFFF) {
		/* Que is empty */
		return((gtmsg_t *)0xFFFFFFFF);
	} else {
		/* Convert the Message address into virtual */
		ptr = memmaps.aPaPci[slot_no] + ptr - 0x80000000 - REMAP_ADDRESS;
		return((gtmsg_t *)ptr);
	}
}

/* This function is used to Update the Inbound Free Head, or
 * any other I2O indexes which needs the PCI locking fix.
 * IMPORTANT: THIS FUNCTION HAS TO BE CALLED WITH ALL INTERRUPTS
 * DISBALED, OTHERWISE THE SYSTEM WILL GET INTO WEIRD STATE
 */
void
asm pcihold_update_index(U32 addr, U32 index)
{
	/* Convert index Virtual to Physical Value */
	/* and swap the value to little Endian */
	andi         v0,a1,0xff
	sll          v1,v0,24
	andi         v0,a1,0xff00
	sll          v0,v0,8
	or           v1,v1,v0
	lui          v0,0xFF
	and          v0,a1,v0
	srl          v0,v0,8
	or           v1,v0,v1
	lui          v0,0x1F00
	and          v0,a1,v0
	srl          v0,v0,24
	or           a1,v0,v1

#ifdef	WORKAROUND_HARD
	/* load the registers with adresses */
	li		t0, PCI_HOLD_CONTROL_ADDR
	li		t1, 0x1

	/* Hold the PCI Bus */
	sb		$0, 0(t0)
	
	/* Wait for the bus to become Idle */ 
pcihold_update_index_loop:
	lbu		v0, 0(t0)
	bne		v0, $0, pcihold_update_index_loop
	andi	v0, v0, 0x1
			
	/* Update the Index */
	sw		a1, 0(a0)
	
	/* release the PCI Bus */
	sb		t1, 0(t0)
#else /* WORKAROUND_HARD */
	/* Software workaround suggested by Galileo */

	/* load the registers with adresses */
	li		t3, (0xB4000000 + 0x0C0)
	/* Retry register address */
	li		t0, (0xB4000000 + GT_PCI_RETRY_OFF)
	/* retry Value */
	li		t1, 0x03FF0000
	/* Save the original Value */
	lw		t2, 0(t0)

	
	/* Make the Timeout0 = 0x03 */
	sw		t1, 0(t0)
	
	/* Wait  for the PCI Slave FIFO to become empty */
//	lw		t1, 0(t3)
	nop; nop; nop; nop; nop; nop; nop

	/* Update the Index */
	sw		a1, 0(a0)

	/* Wait  for the PCI Slave FIFO to become empty */
//	lw		t1, 0(t3)
	nop; nop; nop; nop; nop; nop; nop
		
	/* Restore the Timeout0 ( 0xFF) register */
	sw		t2, 0(t0)
	
	/* Wait  for the PCI Slave FIFO to become empty */
//	lw		t1, 0(t3)
	nop; nop; nop; nop; nop; nop; nop
#endif /* WORKAROUND_HARD */
	
	/* return */
	jr		ra
	nop
}

#endif /* I2O_SOFT */


/* This function posts the message to
 * the remote board, addressed by slot_no */
void
PostRemoteMsg(U32 slot_no, gtmsg_t *msg)
{
	volatile U32 *InQPVReg;
	volatile U32 ptr;

	/* Get the virtual address of Inbound Que Port Virtaul Register
	 * Address of the Remote board */
	InQPVReg = (volatile U32 *)(memmaps.aPaPci[slot_no] + GT_I2O_INQ_VR_OFF - 
					  0x80000000);
	
	/* Convert the Message address into physical */
	ptr = (U32)msg + 0x80000000 - memmaps.aPaPci[slot_no] + REMAP_ADDRESS;

	/* Post the Message, we dont need to worry about the
	 * Que being full. If the que is full, following operation
	 * results with PCI Retry, till the que becomes non-full1 */
	*InQPVReg = ptr;

	return;
}

void
process()
{
	gtmsg_t *msg;
	
	/* Run this funcion in low priotity */
	while(1) {
		/* Find out the Que has any messages */
		if(!MSGQEMPTY(msgq)) {

			/* Crit Section */
			DISABLE_INT;
			/* Get the next message from the Que */
			msg = (gtmsg_t *)MSGQGET(msgq);
			/* End Crit Section */
			ENABLE_INT;
			
			/* If the Message is from I2O call dispatchI2o(), else
			 * call dispatchOther() */
			if (msg->isI2o)
				DispatchI2o(msg);
			else
				DispatchOther(msg);
		} else {
			/* Allow all other ready tasks of the same priority to
	 		 * be executed */
			NU_Relinquish();
		}
		/* Loop Forever */
	}
}

void
VerifyProcess()
{
	gtmsg_t *msg;
	UNSIGNED startV;

	while(1) {
			
		/* Wait for somebody to post Msg for Verifying */
		NU_Retrieve_Events(&verifyCb.startVerify, 2, NU_OR_CONSUME, &startV,
							NU_SUSPEND);
		/* Get the Message */
		msg = verifyCb.msg;
		
		/* Verify the Date */
		msg->vStat = VerifyBuffer(msg);
		/* Crit Section */
		DISABLE_INT;
		/* Flag that Verify is over */
		verifyCb.verifyflag = 0;
		verifyCb.msg        = 0;
		/* This Message is not coming from Remote, so
	 	* reset isI2o flasg */
		msg->isI2o = 0;
		/* Set the retry count to 0 */
		msg->retryCount  = 0;
		/* Set the Message type */
		msg->mType = MTYPE_END_VERIFY;
		/* Insert it to Message Q */
		MSGQPUT(msgq, msg);
		/* End Crit Section */
		ENABLE_INT;
	} /* Loop Forever */
}

void
StartVerify(gtmsg_t *msg)
{
	/* Store the message in the Verify Control Block */
	verifyCb.msg = msg;
	
	/* Activate the Verify Process */
	NU_Set_Events(&verifyCb.startVerify, (UNSIGNED)2, NU_OR);
	/* Allow all other ready tasks of the same priority to
	 * be executed */
	NU_Relinquish();
}


U32
VerifyBuffer(gtmsg_t *msg)
{
	int i, len;
	U64 *src = (U64 *)(BoardDev[msg->mSlot].srcBuf);
	U64 *dst = (U64 *)(BoardDev[msg->mSlot].dstBuf);

	/* Sync the mem for CPU */
	mips_sync_cache((void *)dst, msg->bSize, SYNC_CACHE_FOR_CPU);
	len = msg->bSize >> 3;
	for(i=0; i < len; i++) {
		if (src[i] != dst[i]) {
			msg->dSeed = src[i];
			msg->dCksm = dst[i];
			return(0);
		}
	}
	/* Zero the buffer out */
	bzero64(dst, msg->bSize);
	/* Sync the mem for Device */
	mips_sync_cache((void *)dst, msg->bSize, SYNC_CACHE_FOR_DEV);
	return(1);
}


void
DispatchI2o(gtmsg_t *msg)
{
	gtmsg_t *responseMsg;

	/* Update the Number of Messages Received Count */
	if ( !msg->retryCount)
		BoardDev[msg->mSlot].i2oMsgRxCount++;
	
	/* Take action Depending upon the type of the Message */
	switch(msg->mType) {
	/* Message No-Operation, dont have do anything */
	case MTYPE_NOP:
		if ( consoleIndex == CONSOLE_LOCAL_2)
			printf("Msg From %s Rx %d\n\r", slotname[msg->mSlot], msg->bSize);
		/* Free the Msg back to the Free que */
		FreeMsg(msg);
		break;
	/* Type = PING, repond with a PONG */
	case MTYPE_PING:
		/* Get a Free Mesage from the Remote Board */
		responseMsg = GetRemoteFreeMsg(msg->i2oPtr, msg->mSlot);
		if ( responseMsg == (gtmsg_t *)0xFFFFFFFF) {
			/* Msg Q is empty, put it back to perocess Q, will
			 * try later */
			msg->retryCount++;
			/* Crit Section */
			DISABLE_INT;
			/* Insert it to Message Q */
			MSGQPUT(msgq, msg);
			/* End Crit Section */
			ENABLE_INT;
		} else {
			/* Set the Ping Response Type */
			responseMsg->mType = MTYPE_PONG;
			/* Set the Slot number to My Slot number */
			responseMsg->mSlot = bootblock.b_slot;
			/* Post the Message to the Remote Board */
			PostRemoteMsg(msg->mSlot, responseMsg);
			/* Update the Message Trnsmitted Count */
			BoardDev[msg->mSlot].i2oMsgTxCount++;
			/* Free the Msg back to the Free que */
			FreeMsg(msg);
		}
		break;
	/* Type = PONG */
	case MTYPE_PONG:
		/* Update pingResponseCount Flag */
		BoardDev[msg->mSlot].pingResponseCount++;
		/* Do we need to continue to ping or not ? */
		if ( !BoardDev[msg->mSlot].pingContinue ) {
			/* Free the Msg back to the Free que */
			FreeMsg(msg);
			break;
		}	
		/* Get a Free Mesage from the Remote Board */
		responseMsg = GetRemoteFreeMsg(msg->i2oPtr, msg->mSlot);
		if ( responseMsg == (gtmsg_t *)0xFFFFFFFF) {
			/* Msg Q is empty, put it back to perocess Q, will
			 * try later */
			msg->retryCount++;
			/* Crit Section */
			DISABLE_INT;
			/* Insert it to Message Q */
			MSGQPUT(msgq, msg);
			/* End Crit Section */
			ENABLE_INT;
		} else {
			/* Set the Ping Response Type */
			responseMsg->mType = MTYPE_PING;
			/* Set the Slot number to My Slot number */
			responseMsg->mSlot = bootblock.b_slot;
			/* Post the Message to the Remote Board */
			PostRemoteMsg(msg->mSlot, responseMsg);
			/* Update the Message Trnsmitted Count */
			BoardDev[msg->mSlot].i2oMsgTxCount++;
			/* Update pingResponseCount Flag */
			BoardDev[msg->mSlot].pingRequestCount++;
			/* Free the Msg back to the Free que */
			FreeMsg(msg);
		}
		break;
	/* Type = Get the buffer so that remote board can dma */
	case MTYPE_START_TRANSFER:
		/* Always received from remote send the free buffer */
		/* If the buffer is not assigned already assign it now */
		if ( msg->bAddr == 0) {
			msg->bAddr = BoardDev[msg->mSlot].dstBufPci;
		}
		/* Get a Free Mesage from the Remote Board */
		if ( msg->bAddr)
			responseMsg = GetRemoteFreeMsg(msg->i2oPtr, msg->mSlot);
		if ( (msg->bAddr == 0) || (responseMsg == (gtmsg_t *)0xFFFFFFFF)) {
			/* Msg Q is empty or we dont have buffer
			 *  put it back to perocess Q, will try later */
			msg->retryCount++;
			/* Crit Section */
			DISABLE_INT;
			/* Insert it to Message Q */
			MSGQPUT(msgq, msg);
			/* End Crit Section */
			ENABLE_INT;
		} else {
			/* Set the Type */
			responseMsg->mType = MTYPE_START_DMA;
			/* Set the Slot number to My Slot number */
			responseMsg->mSlot = bootblock.b_slot;
			/* Assign the buffer address */
			responseMsg->bAddr = msg->bAddr;
			msg->bAddr = 0;
			/* Set the size og the Buffer */
			responseMsg->bSize = DMA_BSIZE;
			/* Post the Message to the Remote Board */
			PostRemoteMsg(msg->mSlot, responseMsg);
			/* Update the Message Trnsmitted Count */
			BoardDev[msg->mSlot].i2oMsgTxCount++;
			/* Free the Msg back to the Free que */
			FreeMsg(msg);
		}
		break;
	/* Type = Start the Dma , when it is done we get the message back */
	case MTYPE_START_DMA:
		/* Received from the remote, start the Dma */
		/* Crit Section */
		DISABLE_INT;
		/* Check whther Dma Channel is Busy */
		if ( DMADONE(0)) {
			/* Flag that we have started the Dma */
			DMASTARTED(0);
			/* End Crit Section */
			ENABLE_INT;
			/* Sync the buffer before Dma */
			mips_sync_cache((void *)(BoardDev[msg->mSlot].srcBuf), 
							msg->bSize, SYNC_CACHE_FOR_DEV);
			/* Program Galilleo to do Dma, when we rea done 
			 * we will get an interrupt, which we q the message back
			 */
			StartDma(0, VTOP((U32)(BoardDev[msg->mSlot].srcBuf)), 
							msg->bAddr, msg->bSize, msg);
		} else {
			/* Dma Channel Busy */
			msg->retryCount++;
			/* Insert it to Message Q */
			MSGQPUT(msgq, msg);
			/* End Crit Section */
			ENABLE_INT;
		}
		break;
	/* Type = Get the buffer so that remote board can dma */
	case MTYPE_START_VERIFY:
		/* Always received from the remote, start verify */
		/* Crit Section */
		DISABLE_INT;
		/* Check whether Verify Process is Busy */
		if ( VERIFYDONE()) {
			/* Flag that we have started the Verify */
			VERIFYSTARTED();
			/* End Crit Section */
			ENABLE_INT;
			/* Signal the Verify Process to Start */
			StartVerify(msg);
						
		} else {
			/* Verify Process Busy */
			msg->retryCount++;
			/* Insert it to Message Q */
			MSGQPUT(msgq, msg);
			/* End Crit Section */
			ENABLE_INT;
		}
		
		break;
	case MTYPE_END_VERIFY:
		/* Recived from Remote, if continue , 
		 * start dma, else release buffer */
		if( BoardDev[msg->mSlot].dmaContinue) {
			/* Crit Section */
			DISABLE_INT;
			/* Check whther Dma Channel is Busy */
			if ( DMADONE(0)) {
				/* Flag that we have started the Dma */
				DMASTARTED(0);
				/* End Crit Section */
				ENABLE_INT;
				/* Update the Verify Count */
				if ( msg->vStat) {
					BoardDev[msg->mSlot].verifySuccessCount++;
				} else {
					/* Update the Verify Status */
					BoardDev[msg->mSlot].verifyFailCount++;
					BoardDev[msg->mSlot].verifyDataWritten = msg->dSeed;
					BoardDev[msg->mSlot].verifyDataRead    = msg->dCksm;
					/* Make LED Red */
					*((U8 *)(0xBC080000)) = 0x08;
				}
				/* Sync the buffer before Dma */
				mips_sync_cache((void *)(BoardDev[msg->mSlot].srcBuf), 
							msg->bSize, SYNC_CACHE_FOR_DEV);
				/* Program Galilleo to do Dma, when we rea done 
			 	* we will get an interrupt, which we q the message back
			 	*/
				StartDma(0, VTOP((U32)(BoardDev[msg->mSlot].srcBuf)), 
							msg->bAddr, msg->bSize, msg);
			} else {
				/* Dma Channel Busy */
				msg->retryCount++;
				/* Insert it to Message Q */
				MSGQPUT(msgq, msg);
				/* End Crit Section */
				ENABLE_INT;
			}
		} else {
			/* Get a Free Mesage from the Remote Board */
			responseMsg = GetRemoteFreeMsg(msg->i2oPtr, msg->mSlot);
			if ( responseMsg == (gtmsg_t *)0xFFFFFFFF) {
				/* Msg Q is empty, put it back to perocess Q, will
				 * try later */
				msg->retryCount++;
				/* Crit Section */
				DISABLE_INT;
				/* Insert it to Message Q */
				MSGQPUT(msgq, msg);
				/* End Crit Section */
				ENABLE_INT;
			} else {
				/* Type to END_TRANSFER */
				responseMsg->mType = MTYPE_END_TRANSFER;
				/* Give the Data Buffer Back */
				responseMsg->bAddr = msg->bAddr;
				responseMsg->bSize = msg->bSize;
				/* Set the Slot number to My Slot number */
				responseMsg->mSlot = bootblock.b_slot;
				/* Post the Message to the Remote Board */
				PostRemoteMsg(msg->mSlot, responseMsg);
				/* Update the Message Trnsmitted Count */
				BoardDev[msg->mSlot].i2oMsgTxCount++;
				/* Update the Verify Count */
				if ( msg->vStat) {
					BoardDev[msg->mSlot].verifySuccessCount++;
				} else {
					/* Update the Verify Status */
					BoardDev[msg->mSlot].verifyFailCount++;
					BoardDev[msg->mSlot].verifyDataWritten = msg->dSeed;
					BoardDev[msg->mSlot].verifyDataRead    = msg->dCksm;
					/* Make LED Red */
					*((U8 *)(0xBC080000)) = 0x08;
				}
				/* Free the Msg back to the Free que */
				FreeMsg(msg);
			}
		}
		break;
	case MTYPE_END_TRANSFER:
		/* Always received from the remote, release the buffer */
		BoardDev[msg->mSlot].dstBufPci = msg->bAddr;
		/* Free the Msg back to the Free que */
		FreeMsg(msg);
		break;
	default:
		DispatchReqResp(msg);
		break;
	}
}

void
DispatchOther(gtmsg_t *msg)
{
	gtmsg_t *responseMsg;

	switch(msg->mType) {
	case MTYPE_END_DMA:
		if ( !(BoardDev[msg->mSlot].doVerify) && 
						(BoardDev[msg->mSlot].dmaContinue)) {
			/* Crit Section */
			DISABLE_INT;
			/* Check whther Dma Channel is Busy */
			if ( DMADONE(0)) {
				/* Flag that we have started the Dma */
				DMASTARTED(0);
				/* End Crit Section */
				ENABLE_INT;
				/* Update the dmaCount */
				BoardDev[msg->mSlot].dmaCount++;
				/* Sync the buffer before Dma */
				mips_sync_cache((void *)(BoardDev[msg->mSlot].srcBuf), 
							msg->bSize, SYNC_CACHE_FOR_DEV);
				/* Program Galilleo to do Dma, when we rea done 
				 * we will get an interrupt, which we q the message back
				 */
				StartDma(0, VTOP((U32)(BoardDev[msg->mSlot].srcBuf)), 
							msg->bAddr, msg->bSize, msg);
			} else {
				/* Dma Channel Busy */
				msg->retryCount++;
				/* Insert it to Message Q */
				MSGQPUT(msgq, msg);
				/* End Crit Section */
				ENABLE_INT;
			}
		} else {
			/* Get the message from remote board */
			responseMsg = GetRemoteFreeMsg(msg->i2oPtr, msg->mSlot);
			if ( responseMsg == (gtmsg_t *)0xFFFFFFFF) {
				/* Msg Q is empty, put it back to perocess Q, will
				 * try later */
				msg->retryCount++;
				/* Crit Section */
				DISABLE_INT;
				/* Insert it to Message Q */
				MSGQPUT(msgq, msg);
				/* End Crit Section */
				ENABLE_INT;
				break;
			}
			/* if the doVerify flag is on send verify message
		 	* to the remote, Release the BUffer */
			if ( BoardDev[msg->mSlot].doVerify ) {
				/* Set the type of the Mesage */
				responseMsg->mType = MTYPE_START_VERIFY;
				
			} else {
				/* Set the type of the Mesage */
				responseMsg->mType = MTYPE_END_TRANSFER;
				responseMsg->vStat = 0;
			}
			/* Set the address and Size of the buffer */
			responseMsg->bAddr = msg->bAddr;
			responseMsg->bSize = msg->bSize;
			/* Set the Slot number to My Slot number */
			responseMsg->mSlot = bootblock.b_slot;
			/* Post the Message to the Remote Board */
			PostRemoteMsg(msg->mSlot, responseMsg);
			/* Update the Message Trnsmitted Count */
			BoardDev[msg->mSlot].i2oMsgTxCount++;
			/* Free the Msg back to the Free que */
			FreeMsg(msg);
			/* Update the dmaCount */
			BoardDev[msg->mSlot].dmaCount++;
		}
		break;
	case MTYPE_END_VERIFY:
		/* send it to remote */
		/* Get the message from remote board */
		responseMsg = GetRemoteFreeMsg(msg->i2oPtr, msg->mSlot);
		if ( responseMsg == (gtmsg_t *)0xFFFFFFFF) {
			/* Msg Q is empty, put it back to perocess Q, will
			 * try later */
			msg->retryCount++;
			/* Crit Section */
			DISABLE_INT;
			/* Insert it to Message Q */
			MSGQPUT(msgq, msg);
			/* End Crit Section */
			ENABLE_INT;
			break;
		}
		/* Set the type of the Message */
		responseMsg->mType = MTYPE_END_VERIFY;
		/* Set the address and Size of the buffer */
		responseMsg->bAddr = msg->bAddr;
		responseMsg->bSize = msg->bSize;
		/* Set the Slot number to My Slot number */
		responseMsg->mSlot = bootblock.b_slot;
		/* Set the Verify Status */
		responseMsg->vStat = msg->vStat;
		responseMsg->dSeed = msg->dSeed;
		responseMsg->dCksm = msg->dCksm;
		/* Post the Message to the Remote Board */
		PostRemoteMsg(msg->mSlot, responseMsg);
		/* Update the Message Trnsmitted Count */
		BoardDev[msg->mSlot].i2oMsgTxCount++;
		/* Free the Msg back to the Free que */
		FreeMsg(msg);
		break;
	}
}

void
DispatchReqResp(gtmsg_t *msg)
{
	gtmsg_t *responseMsg;

	switch(msg->mType) {
	case MTYPE_STATISTICS_REQ:
		/* Get the message from remote board */
		responseMsg = GetRemoteFreeMsg(msg->i2oPtr, msg->mSlot);
		if ( responseMsg == (gtmsg_t *)0xFFFFFFFF) {
			/* Msg Q is empty, put it back to perocess Q, will
			 * try later */
			msg->retryCount++;
			/* Crit Section */
			DISABLE_INT;
			/* Insert it to Message Q */
			MSGQPUT(msgq, msg);
			/* End Crit Section */
			ENABLE_INT;
			break;
		}
		/* Set the type of the Message */
		responseMsg->mType = MTYPE_STATISTICS_RESP;
		/* Copy the BoardDev */
		bcopy(BoardDev, ((U32)msg->bAddr) - 0x80000000, sizeof(BoardDev));
		/* Set the address and Size of the buffer */
		responseMsg->bAddr = msg->bAddr;
		responseMsg->bSize = msg->bSize;
		/* Set the Slot number to My Slot number */
		responseMsg->mSlot = bootblock.b_slot;
		/* Post the Message to the Remote Board */
		PostRemoteMsg(msg->mSlot, responseMsg);
		/* Update the Message Trnsmitted Count */
		BoardDev[msg->mSlot].i2oMsgTxCount++;
		/* Free the Msg back to the Free que */
		FreeMsg(msg);
		break;
	case MTYPE_STATISTICS_RESP:
		/* Crit Section */
		DISABLE_INT;
		/* Assign the msg to BoadrDev */
		BoardDev[msg->mSlot].respMsg = msg;
		/* Inform the  waiting task that resp has arrived */
		BoardDev[msg->mSlot].respWaiting = 0;
		/* End Crit Section */
		ENABLE_INT;
		break;
	case MTYPE_STARTTEST_REQ:
		/* Get the message from remote board */
		responseMsg = GetRemoteFreeMsg(msg->i2oPtr, msg->mSlot);
		if ( responseMsg == (gtmsg_t *)0xFFFFFFFF) {
			/* Msg Q is empty, put it back to perocess Q, will
			 * try later */
			msg->retryCount++;
			/* Crit Section */
			DISABLE_INT;
			/* Insert it to Message Q */
			MSGQPUT(msgq, msg);
			/* End Crit Section */
			ENABLE_INT;
			break;
		}
		/* Start the Test */
		responseMsg->vStat = StartDmaTest(msg->vStat);
		/* Set the type of the Message */
		responseMsg->mType = MTYPE_STARTTEST_RESP;
		/* Set the Slot number to My Slot number */
		responseMsg->mSlot = bootblock.b_slot;
		/* Post the Message to the Remote Board */
		PostRemoteMsg(msg->mSlot, responseMsg);
		/* Update the Message Trnsmitted Count */
		BoardDev[msg->mSlot].i2oMsgTxCount++;
		/* Free the Msg back to the Free que */
		FreeMsg(msg);
		break;
	case MTYPE_STARTTEST_RESP:
		/* Crit Section */
		DISABLE_INT;
		/* Assign the msg to BoadrDev */
		BoardDev[msg->mSlot].respMsg = msg;
		/* Inform the  waiting task that resp has arrived */
		BoardDev[msg->mSlot].respWaiting = 0;
		/* End Crit Section */
		ENABLE_INT;
		break;
	case MTYPE_STOPTEST_REQ:
		/* Stop the Test */
		StopDmaTest(msg->vStat);
		/* Free the Msg back to the Free que */
		FreeMsg(msg);
		break;
	case MTYPE_READY:
		if ( BoardDev[msg->mSlot].found && (!(BoardDev[msg->mSlot].ready))) {
			/* Get the message from remote board */
			responseMsg = GetRemoteFreeMsg(msg->i2oPtr, msg->mSlot);
			if ( responseMsg == (gtmsg_t *)0xFFFFFFFF) {
				/* Msg Q is empty, put it back to perocess Q, will
				 * try later */
				msg->retryCount++;
				/* Crit Section */
				DISABLE_INT;
				/* Insert it to Message Q */
				MSGQPUT(msgq, msg);
				/* End Crit Section */
				ENABLE_INT;
				break;
			}
			/* Set the state to ready */
			BoardDev[msg->mSlot].ready = 1;
			/* Set the type of the Message */
			responseMsg->mType = MTYPE_READY;
			/* Set the Slot number to My Slot number */
			responseMsg->mSlot = bootblock.b_slot;
			/* Post the Message to the Remote Board */
			PostRemoteMsg(msg->mSlot, responseMsg);
			/* Update the Message Trnsmitted Count */
			BoardDev[msg->mSlot].i2oMsgTxCount++;
			/* Free the Msg back to the Free que */
			FreeMsg(msg);
		} else {
			/* Free the Msg back to the Free que */
			FreeMsg(msg);
		}
		break;
	case MTYPE_HBC_READY:
		/* Send the Ready Message */
		SendReadyMsg();
		/* Free the Msg back to the Free que */
		FreeMsg(msg);
		break;
	}
}

void
StartRemoteTest()
{
	gtmsg_t *msg;
	int i;
	for(i=0; i < MAX_SLOTS; i++) {
		/* For this board we start the test locally */
		if ( i ==  bootblock.b_slot)
			continue;
		/* Start the test only if board is present */
		if ( BoardDev[i].found) {
			/* Crit Section */
			DISABLE_INT;
			while(BoardDev[i].reqRespActivity) {
				/* End Crit Section */
				ENABLE_INT;
				/* Allow all other ready tasks of the same priority to
	 			 * be executed */
				NU_Relinquish();
				/* Crit Section */
				DISABLE_INT;
			}
			BoardDev[i].reqRespActivity = 1;
			/* End Crit Section */
			ENABLE_INT;
			/* Get the Free Message */
			if ( (msg = GetRemoteFreeMsg(i2odev, i)) == (gtmsg_t *)0xFFFFFFFF){
				/* reset the Flag */
				BoardDev[i].reqRespActivity = 0;
				/* print the error */
				printf("Could not start test on remote %s\n\r", slotname[i]);
				continue;
			}
			/* Set Type */
			msg->mType = MTYPE_STARTTEST_REQ;
			/* Set the Slot */
			msg->mSlot      = bootblock.b_slot;
			/* Set the bit flag to remote start */
			msg->vStat =  0xFFFFFFFF;
			/* Set the Flag */
			BoardDev[i].respWaiting = 1;
			/* Post the Message */
			PostRemoteMsg(i, msg);
			/* Update Counter */
			BoardDev[i].i2oMsgTxCount++;
			
			while(BoardDev[i].respWaiting) {
				/* Allow all other ready tasks of the same priority to
	 	 		 * be executed */
				NU_Relinquish();
			}
			/* Print the start status */
			DisplayTestStartStat(i, BoardDev[i].respMsg->vStat);
			/* Free the Msg back to the Free que */
			FreeMsg(BoardDev[i].respMsg);
			
			/* Req/Resp Activity Done */
			BoardDev[i].reqRespActivity = 0;
		}
	}
	/* Start the test from Local */
	DisplayTestStartStat(bootblock.b_slot, StartDmaTest(0xFFFFFFFF));

}

void
StopRemoteTest()
{
	gtmsg_t *msg;
	int i;
	for(i=0; i < MAX_SLOTS; i++) {
		/* For this board we stop the test locally */
		if ( i ==  bootblock.b_slot)
			continue;
		/* Stop the test only if board is present */
		if ( BoardDev[i].found) {
			/* Get the Free Message */
			if ( (msg = GetRemoteFreeMsg(i2odev, i)) == (gtmsg_t *)0xFFFFFFFF){
				/* reset the Flag */
				BoardDev[i].reqRespActivity = 0;
				/* print the error */
				printf("Could not stop test on remote %s\n\r", slotname[i]);
				continue;
			}
			/* Set Type */
			msg->mType = MTYPE_STOPTEST_REQ;
			/* Set the Slot */
			msg->mSlot      = bootblock.b_slot;
			/* Set the bit flag to remote start */
			msg->vStat =  0xFFFFFFFF;
			/* Post the Message */
			PostRemoteMsg(i, msg);
			/* Update Counter */
			BoardDev[i].i2oMsgTxCount++;
			
		}
	}
	/* Stop the test from Local */
	StopDmaTest(0xFFFFFFFF);
}

void
SendHbcReadyMsg()
{
	int i;
	gtmsg_t *msg;
	
	for(i=0; i < 32; i++) {
		/* if the board is us dont do anything */
		if ( i == bootblock.b_slot )
			continue;
		/* Send the Mesage, if board is present */
		if ( BoardDev[i].found) {
			/* Get the free Message */
			msg = GetRemoteFreeMsg(i2odev, i);
			/* If successful in getting the message */
			if ( msg != (gtmsg_t *)0xFFFFFFFF) {
				msg->mType	= MTYPE_HBC_READY;
				msg->mSlot      = bootblock.b_slot;
				/* Post the Message */
				PostRemoteMsg(i, msg);
				BoardDev[i].i2oMsgTxCount++;
			}
		}
	}
}
void
SendReadyMsg()
{
	int i;
	gtmsg_t *msg;
	
	for(i=0; i < 32; i++) {
		/* if the board is us dont do anything */
		if ( i == bootblock.b_slot )
			continue;
		/* Send the Mesage, if board is present */
		if ( BoardDev[i].found) {
			/* Get the free Message */
			msg = GetRemoteFreeMsg(i2odev, i);
			/* If successful in getting the message */
			if ( msg != (gtmsg_t *)0xFFFFFFFF) {
				msg->mType	= MTYPE_READY;
				msg->mSlot      = bootblock.b_slot;
				/* Post the Message */
				PostRemoteMsg(i, msg);
				BoardDev[i].i2oMsgTxCount++;
			}
		}
	}
}
U32
StartDmaTest(U32 bitFlag)
{
	int i;
	gtmsg_t *msg;
	U32 stat = 0;

	for(i=0; i < 32; i++, bitFlag >>= 1) {
		/* if the board is us dont do anything */
		if ( i == bootblock.b_slot )
			continue;
		
		/* Check to see whether we need to start the test for this slot */
		if ( !(bitFlag & 0x1))
			continue;
		
		/* if the test is already active dont start it again */
		if (BoardDev[i].dmaActive)
			continue;

		/* Send the Mesage, if board is present */
		if ( BoardDev[i].found ) {
			msg = (gtmsg_t *)0xFFFFFFFF;
			/* Get the free Message if the board is ready */
			if (BoardDev[i].ready)
				msg = GetRemoteFreeMsg(i2odev, i);
			/* If successful in getting the message */
			if ( msg != (gtmsg_t *)0xFFFFFFFF) {
				/* Init the Message */
				msg->mType	= MTYPE_START_TRANSFER;
				msg->mSlot	= bootblock.b_slot;
				msg->bAddr  = 0;
				/* Set the flags */
				BoardDev[i].dmaContinue = 1;
				BoardDev[i].doVerify = 1;
				BoardDev[i].dmaActive = 1;
				/* Post the Message */
				PostRemoteMsg(i, msg);
				BoardDev[i].i2oMsgTxCount++;
			} else {
				/* Set the particular bit to inform that test could not be
				 * started */
				stat = stat | ( 1 << i);
			}
		}
	}
	return(stat);
}
void
StopDmaTest(U32 bitFlag)
{
	int i;

	for(i=0; i < 32; i++, bitFlag >>= 1) {
		/* if the board is us dont do anything */
		if ( i == bootblock.b_slot )
			continue;
		
		/* Check to see whether we need to stop the test for this slot */
		if ( !(bitFlag & 0x1))
			continue;
		
		if (BoardDev[i].dmaActive) {
			/* Set the flags */
			BoardDev[i].dmaContinue = 0;
			BoardDev[i].doVerify = 0;
			BoardDev[i].dmaActive = 0;
		}
	}
}


U32
DoNopTest(U32 bitFlag)
{
	int i;
	gtmsg_t *msg;
	U32		stat = 0;

	for(i=0; i < 32; i++, bitFlag >>= 1) {
		/* if the board is us dont do anything */
		if ( i == bootblock.b_slot )
			continue;
		
		/* Check to see whether we need to start the test for this slot */
		if ( !(bitFlag & 0x1))
			continue;
		
		/* Send the Mesage, if board is present */
		if ( BoardDev[i].found ) {
			msg = (gtmsg_t *)0xFFFFFFFF;
			/* Get the free Message if the board is ready */
			if (BoardDev[i].ready)
				msg = GetRemoteFreeMsg(i2odev, i);
			/* If successful in getting the message */
			if ( msg != (gtmsg_t *)0xFFFFFFFF) {
				/* Init the Message */
				msg->mType	= MTYPE_NOP;
				msg->mSlot	= bootblock.b_slot;
				BoardDev[i].i2oMsgTxCount++;
				msg->bSize	= BoardDev[i].i2oMsgTxCount;
				/* Post the Message */
				PostRemoteMsg(i, msg);
			} else {
				/* Set the particular bit to inform that test could not be
				 * started */
				stat = stat | ( 1 << i);
			}
		}
	}
	return(stat);
}


STATUS
GetRemoteStatistics(BoardDev_t *remoteBoardDevPtr, U32 slot_no)
{
	gtmsg_t *msg;

	/* BordDev corresponding to this board, because
	 * we have it here */
	if ( slot_no ==  bootblock.b_slot) {
		bcopy(BoardDev, remoteBoardDevPtr, sizeof(BoardDev));
		/* Sync the remoteBoardDevPtr back to memory */
		mips_sync_cache((void *)remoteBoardDevPtr, sizeof(BoardDev), 
						SYNC_CACHE_FOR_DEV);
		return(OK);
	}
		/* Get the stat only if board is present */
	if ( BoardDev[slot_no].found) {
		/* Crit Section */
		DISABLE_INT;
		while(BoardDev[slot_no].reqRespActivity) {
			/* End Crit Section */
			ENABLE_INT;
			/* Allow all other ready tasks of the same priority to
	 		 * be executed */
			NU_Relinquish();
			/* Crit Section */
			DISABLE_INT;
		}
		BoardDev[slot_no].reqRespActivity = 1;
		/* End Crit Section */
		ENABLE_INT;
		/* Get the Free Message */
		if ( (msg = GetRemoteFreeMsg(i2odev, slot_no))== (gtmsg_t *)0xFFFFFFFF){
			/* reset the Flag */
			BoardDev[slot_no].reqRespActivity = 0;
			/* return the error */
			return(1);
		}
		/* Set Type */
		msg->mType = MTYPE_STATISTICS_REQ;
		/* Set the Slot */
		msg->mSlot      = bootblock.b_slot;
		/* Set the Address to remoteBoardDev */
		msg->bAddr = memmaps.pciSlave + (VTOP((U32)remoteBoardDevPtr)
							- REMAP_ADDRESS);
		/* Set the Flag */
		BoardDev[slot_no].respWaiting = 1;
		/* Post the Message */
		PostRemoteMsg(slot_no, msg);
		/* Update Counter */
		BoardDev[slot_no].i2oMsgTxCount++;
			
		while(BoardDev[slot_no].respWaiting) {
			/* Allow all other ready tasks of the same priority to
	 	 	 * be executed */
			NU_Relinquish();
		}
		/* Free the Msg back to the Free que */
		FreeMsg(BoardDev[slot_no].respMsg);
			
		/* Sync remoteBoardDevPtr from mem to CPU */
		mips_sync_cache((void *)remoteBoardDevPtr, sizeof(BoardDev), 
						SYNC_CACHE_FOR_CPU);
		/* Req/Resp Activity Done */
		BoardDev[slot_no].reqRespActivity = 0;
		return(OK);
	} else {
		/* Return Error */
		return(1);
	}
}



void
DisplayProcess()
{
	int prevIndex = consoleIndex;
	int headerFlag = 1;
	U32 prevDmaCount[32][32];
	int i, j;
	int num_sessions_started, num_sessions_running, num_vrfy_err;;
	
	/* init the prevDmaCount */
	for(i=0; i < 32; i++) {
		for(j=0; j < 32; j++)
			prevDmaCount[i][j] = 0;
	}

	while(1) {
		/* Check for Consoles */
		if ( consoleIndex < CONSOLE_SYSTEM_MONITOR) {
			if ( (consoleIndex == CONSOLE_LOCAL) || 
							(consoleIndex == bootblock.b_slot) ) {
				/* Local Console, print the header first  */
				DisplayHeader(bootblock.b_type, bootblock.b_slot, headerFlag);
				/* Display the Stats */
				DisplayStats(BoardDev, bootblock.b_slot, 1);
			} else if ( consoleIndex == CONSOLE_LOCAL_2 ) {
				/* Local Console for Debugging */
				/* Do nothing for right now */
				printf(".");	
			} else {	
				/* Remote Console , print the header first */
				DisplayHeader(memmaps.aBoardType[consoleIndex], 
												consoleIndex, headerFlag);
				/* Get the Stats */
				if ( GetRemoteStatistics(remoteBoardDev, consoleIndex) == OK){
					/* Print the remote Stat */
					DisplayStats(remoteBoardDev, consoleIndex, 1);
				}
			}

		}
		/* Check for System Monitor */
		if ( consoleIndex == CONSOLE_SYSTEM_MONITOR ) {
			/* print the System Monitor Header */
			DisplayMonitorHeader(1);
			/* Set the Cursor Position */
			printf("\033[07;01H");
			for(i=0; i < 32; i++) {
				num_sessions_started = num_sessions_running = num_vrfy_err = 0;
				/* Get the Stats */
				if ( GetRemoteStatistics(remoteBoardDev, i) == OK){
					for(j=0; j < 32; j++) {
						num_sessions_started += remoteBoardDev[j].dmaActive;
						/* Check the verfy errors */
						num_vrfy_err += remoteBoardDev[j].verifyFailCount;
						/* Check whether DMA is running */
						if ( remoteBoardDev[j].dmaCount != prevDmaCount[i][j]){
							num_sessions_running++;
						}
						/* Set the New Value */
						prevDmaCount[i][j] = remoteBoardDev[j].dmaCount;
					}
					/* Print the Stats */
					printf("%s    %8d  %8d  %8d  ", 
							slotname[i], num_sessions_started, 
							num_sessions_running, num_vrfy_err);
					/* Print Error, if any */
					if ( remoteBoardDev[i].errStat )
						printf("Error:  Check Console\n\r");
					else if ( num_sessions_running < num_sessions_started)
						printf("Error: %d sessions stopped, Check Console\n\r",
								num_sessions_started - num_sessions_running);
					else if ( num_vrfy_err)
						printf("Error:  Verify, Check Console\n\r");
					else
						printf("None\n\r");
				}
			}
		}
		NU_Sleep(100);
		/* If the Console Changed, print the Header Also */
		if ( prevIndex != consoleIndex)
			headerFlag = 1;
		else
			headerFlag = 0;
		prevIndex = consoleIndex;
	}
}
			
void
DisplayTestStartStat(U32 slot_no, U32 stat)
{
	int i;
	
	for(i=0; i < 32; i++, stat >>= 1) {
		/* if the board is us dont do anything */
		if ( i == slot_no )
			continue;
		
		/* If the bit is not set no error */
		if ( !(stat & 0x1))
			continue;
		
		/* print the error session */
		if ( slot_no == bootblock.b_slot)
			printf("Error Starting test from local  to slot %s\n\r", 
							slotname[i]);
		else 
			printf("Error Starting test from slot %s to slot %s\n\r", 
							slotname[slot_no], slotname[i]);
	}
}


void
DisplayHeader(U32 type, U32 slot_no, U32 flag)
{
	char pStars[] = 
	"*********************************************************************";

	/* Dont print if flag is not set */
	if ( !flag)
		return;
	
	/* Clear Screeen */
	printf("\033[H\033[2J\033[0m");
	
	/* print the header */
	printf("\033[01;07H%s", pStars);
	printf("\033[02;15H%s Console, Slot %s",
					bname[type], slotname[slot_no]);
	printf("\033[03;07H%s", pStars);

	/* Set the Cursor Position */
	printf("\033[05;01H");
	printf("Slot  MesgInCt  DmaOutCt  DmaMbs/s  Vrfy_Err"); 
}

void
DisplayStats(BoardDev_t *BoardDevArray, U32 slot_no, U32 flag)
{
	int i;
	BoardDev_t *bd;
	U8 *ptr;
	U32 errStat;

	/* Dont print if flag is not set */
	if ( !flag)
		return;

	/* Set the Cursor Position */
	printf("\033[06;01H");

	for(i=0; i < MAX_SLOTS; i++) {
		/* BoardDev corresponding to us does not represent anything */
		if ( i == slot_no )
			continue;

		bd = &BoardDevArray[i];
		/* Print the stat only if board is present */
		if ( bd->found) {
			/* Print the Counters */
			printf("%s    %8d  %8d  %8d  %8d  ", slotname[i], 
					bd->i2oMsgRxCount, 
					bd->dmaCount, bd->dmaThruput, bd->verifyFailCount);
			if ( bd->verifyFailCount) {
				/* Print the Expexted and actual values in Little
				 * Endian Fromat */
				ptr = (U8 *)(&(bd->verifyDataWritten));
				printf("%02X%02X%02X%02X%02X%02X%02X%02X ", ptr[7], ptr[6],
							ptr[5], ptr[4], ptr[3], ptr[2], ptr[1], ptr[0]);
				ptr = (U8 *)(&(bd->verifyDataRead));
				printf("%02X%02X%02X%02X%02X%02X%02X%02X ", ptr[7], ptr[6],
							ptr[5], ptr[4], ptr[3], ptr[2], ptr[1], ptr[0]);
			} else {
				/* print Dma Burst Thruput */
					
			}
			printf("\n\r");
		}
	}
	/* print the Galileo Error Status */
	errStat = BoardDevArray[slot_no].errStat;
	/* Noerror */
	if (!errStat)
		errStat = 1;
	/* Set the Cursor Position */
	printf("\033[24;01H");
	/* Check each bit for Error */
	for(i=0; i < GT_ERR_NUM; i++, errStat >>= 1) {
		if ( errStat & 0x1)
			printf("********Error : %s ", galileoErrMsg[i]);
	}
}

void
DisplayMonitorHeader(U32 flag)
{
	char pStars[] = 
	"*********************************************************************";

	/* Dont print if flag is not set */
	if ( !flag)
		return;
	
	/* Clear Screeen */
	printf("\033[H\033[2J\033[0m");
	
	/* print the header */
	printf("\033[01;07H%s", pStars);
	printf("\033[02;15HSystem Monitor - Press <A/B/C/D><1/2/3/4> for remote consoles");
	printf("\033[03;07H%s", pStars);

	/* Set the Cursor Position */
	printf("\033[05;01H");
	printf("Slot  Sessions  Sessions  Vrfy_Err  System_Err\n\r"); 
	printf("       Started   Running"); 
}







/* ---------------------- Start of DEBUG support function -------------*/



void
galileo_regdump()
{
	int	i,j;
	/* We skip the registers 0x0C8, 0xC30 and 0xC34 becuase the
	 * the system hangs if we read these registers 
	 * We skip 0x11C-0x3FC, 0x474-0x7FC and 0x880-0xDFC because the
	 * Galileo does not have anything there
	 */
	i = 0;
	j = 1;
	printf("Dumping the Galileo Registers:\n\r");
	while ( i < 0xCF4 ) {
		printf("%03X : %08X", i, gt_read(i));
		if ( (j % 4) == 0) 
			printf("\n\r");
		else
			printf(",	");	
		i += 4;
		j++;
		if ( i == 0x0C8 ) i = 0x0CC;
		if ( i == 0x11C ) { i = 0x400; printf("\n\r"); j = 1; }
		if ( i == 0x474 ) { i = 0x800; printf("\n\r"); j = 1; }
		if ( i == 0x880 ) { i = 0xC00; printf("\n\r"); j = 1; }
		if ( i == 0xC30 ) i = 0xC38;
	}
	printf("\n\r");
}

void
gt_dma(U32 chan, U32 src, U32 dst, U16 len)
{
	int offset;
	U32	val;
	
	if ( (chan < 0) || (chan > 3))
		return;
	printf("DMA%1d Started Len %08X...", chan, len);
	dmainfo[chan].dmalen = len;
	offset = chan * 4;
	gt_write(GT_DMA_CH0_SRC_OFF + offset, src);
	gt_write(GT_DMA_CH0_DEST_OFF + offset, dst);
	gt_write(GT_DMA_CH0_COUNT_OFF + offset, len);
	
	val = GT_DMA_CHAN_EN | GT_DMA_NOCHAIN_MODE | GT_DMA_BLOCK_MODE | 
		  GT_DMA_XMIT_64;
	if ( gt_read(GT_CPU_CONFIG_OFF) & GT_PCI_2GIG ) {
		/* We are Using PCI 2G Sapce, the regular decoder is bypassed
		 * So we need to Set the Overrride bits for the Source and
		 * Destination Addresses
		 */
		if ( src >= PCI_WINDOW_START )
			val |= GT_DMA_SLP_PCI0;
		if ( dst >= PCI_WINDOW_START )
			val |= GT_DMA_DLP_PCI0;
	}
	dmainfo[chan].dmaflag = 1;
	gt_write(GT_TIMER0_OFF, 0xFFFFFFFF);
	/* Start DMA */
	gt_write(GT_DMA_CH0_CONTROL_OFF + offset, val);
}

/* The Code does not support Multiple Channel DMA simultaneously */
void
gt_dmachain(U32 chan, U32 src, U32 dst, U32 len, U32 slot_no, U32 type)
{
	int offset;
	U32	val;
	U16	dsize;
	int	i;
	dmarec_t *dmarecp = dmarec;
	
#if 0
	printf("Chan %d, Slot %d, Src %08X, Dst %08X, Len %d\n\r", chan, slot_no, 
					src, dst, len);
#endif
	if ( (chan < 0) || (chan > 3))
		return;
	dmainfo[chan].dmalen = len;
	offset = chan * 4;
	/* Creat the Value to write to Control Register */
	val = GT_DMA_CHAN_EN | GT_DMA_BLOCK_MODE | GT_DMA_XMIT_64 |
		  GT_DMA_INT_MODE | GT_DMA_FET_NEXTREC;
	if ( gt_read(GT_CPU_CONFIG_OFF) & GT_PCI_2GIG ) {
		/* We are Using PCI 2G Sapce, the regular decoder is bypassed
		 * So we need to Set the Overrride bits for the Source and
		 * Destination Addresses
		 */
		if ( src >= PCI_WINDOW_START ) {
			val |= GT_DMA_SLP_PCI0;
		}
		if ( dst >= PCI_WINDOW_START ) {
			val |= GT_DMA_DLP_PCI0;
		}
	}
	/* Init the Fetch Register */
	gt_write(GT_DMA_CH0_NEXT_OFF + offset, VTOP((U32)dmarecp));

	i = 0;
	while(len) {
		if ( len > DMA_SIZE) {
			dsize = DMA_SIZE;
			dmarecp->next = DWSWAP(VTOP((U32)dmarecp + sizeof(dmarec_t)));
		} else {
			dsize = len;
			dmarecp->next = 0;
		}
		dmarecp->src = DWSWAP(src);
		dmarecp->dst = DWSWAP(dst);
		dmarecp->count = DWSWAP(dsize);
		len	-= dsize;
		src	+= dsize;
		dst	+= dsize;
		dmarecp++;
	}
	/* Start the Counter */
	dmainfo[chan].slot_no = slot_no;
	dmainfo[chan].type = type;
	gt_write(GT_TIMER0_OFF, 0xFFFFFFFF);
	/* Start DMA */
	gt_write(GT_DMA_CH0_CONTROL_OFF + offset, val);
}

void
print_pointers()
{
	printf("Control    %08X\n\r", gt_read(0x1C00 + GT_I2O_QCTRL_OFF));
	printf("QBar       %08X\n\r", gt_read(0x1C00 + GT_I2O_QBAR_OFF));
	printf("Cause      %08X\n\r", gt_read(0x1C00 + GT_I2O_INTIN_CAUSE_OFF));
	printf("Mask       %08X\n\r", gt_read(0x1C00 + GT_I2O_INTIN_MASK_OFF));
	printf("Free Head  %08X\n\r", gt_read(0x1C00 + GT_I2O_INFREE_HEAD_OFF));
	printf("Free Tail  %08X\n\r", gt_read(0x1C00 + GT_I2O_INFREE_TAIL_OFF));
	printf("Post Head  %08X\n\r", gt_read(0x1C00 + GT_I2O_INPOST_HEAD_OFF));
	printf("Post Tail  %08X\n\r", gt_read(0x1C00 + GT_I2O_INPOST_TAIL_OFF));
		
	printf("SFree Head %08X\n\r", i2odev->infree_head);
	printf("SPost Tail %08X\n\r", i2odev->inpost_tail);
}

