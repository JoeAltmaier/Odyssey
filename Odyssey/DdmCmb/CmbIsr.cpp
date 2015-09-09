/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: CmbIsr.cpp
// 
// Description:
// Driver for MIPS/CMA interface.
// 
// $Log: /Gemini/Odyssey/DdmCmb/CmbIsr.cpp $
// 
// 17    2/15/00 11:33a Eric_wedel
// Added low-level AVR tracing code.  [Tracking DFCT12059]
// 
// 16    11/16/99 1:00p Ataylor
// Include Tracef
// 
// 15    11/10/99 1:24p Ewedel
// Added ID patch for use on HBC1, and Craig's most excellent bug fix for
// AVR interface hardware usage.
// 
// 13    8/26/99 12:33p Ataylor
// Used CMB_BROADCAST_BIT to accept ACK response to local.
// 
// 12    8/26/99 11:16a Ataylor
// Simplified to full duplex read/write
// 
// 11    8/19/99 3:13p Jlane
// Use new receive buffer mask.
// 
// 10    8/19/99 2:00p Ewedel
// Added critical section protection around access to low-level ISR's
// shared buffers.
// 
// 9     8/04/99 8:51a Ataylor
// Fixed RCVR_DONE window and added debug printfs
// 
// 8     7/22/99 11:39a Ataylor
// Added unsolicited response address
// 
// 7     7/21/99 8:06p Ataylor
// Added state to differentiate unsolicited command from response
// 
// 6     7/21/99 6:07p Rkondapalli
// [At]  Correct problems with new xmit backoff logic.
// 
// 5     7/21/99 11:47a Ataylor
// Added changes to allow abort of command from MIPS if receive
// unsolicited input
// Added changes to ACK unsolicited input
// 
// 4     7/19/99 5:11p Rkondapalli
// [ewx]  Added test jig linkage, under conditional compilation.
// 
// 3     7/16/99 1:14p Ataylor
// E2 changes
// 
// 2     6/18/99 10:51a Rkondapalli
// Various fixes to make it work, and added a little debug output.
// [at/ew]
// 
// 1     6/17/99 1:56p Ewedel
// Initial checkin.
// 
//  05/07/99 - Created by aTaylor
/*************************************************************************/

#include "nucleus.h"
#include "kernel.h"
#include "CmbIsr.h"
#include "CtEvent.h"    // standard system status codes (generated file)
#include "CmbHwIntfMsgs.h"
#include "trace_index.h"
#define TRACE_INDEX TRACE_BOOT
#include "odyssey_trace.h"
#ifdef CT_TEST_HW_INTF
# include  "HwTestIntf.h"
#endif

#include "CmbDebugHack.h"     // for AVR i/f data tracking ring


NU_HISR		CMB_HISR;
void CMB_ISR_Low(INT) ;
void CMB_ISR_High(VOID) ;
void SendCMBbyte(unsigned char ) ;
#define	CMB_HISR_PRIORITY	0

//  signal code from our host DDM's code space.  This signal is reserved
//  for our internal use in propagating AVR-to-MIPS IRQs.
SIGNALCODE  nSigCmbHwIrq;

//  pointer to our host DDM, so we can signal it
DdmServices     * pHwHostDdm;

unsigned char  CMB_My_ID;
unsigned char  CMBflags ; 	

CMB_CONTROL_BLOCK	CMB_ControlBlock ;

#ifdef CT_TEST_HW_INTF
//  a little helper for Nucleus-based signalling
unsigned Send_Message;
#endif
int		CMBdebugIndex=0 ;
int		CMBtIndex = 0;
unsigned char  CMBdebugBuff[2048] ; 	
I64		CMBtimestamp[256] ;

/*************************************************************************/
// CMB_ISR_Create
// Create CMB_ISR low and high.
/*************************************************************************/

STATUS	CMB_ISR_Create  (CMB_CONTROL_BLOCK *pCMB_ControlBlock, UNSIGNED stack_size)
{
	VOID			(*old_lisr)(INT); // pointer to old LISR
	STATUS			 status;
	
// Allocate stack for HISR
	

	pCMB_ControlBlock->pHISR_stack = new char [CMB_HISR_STACK_SIZE];

// Register low level interrupt service routine

	status = NU_Register_LISR(pCMB_ControlBlock->ISR_DataReadyNumber,
								&CMB_ISR_Low, &old_lisr);
			
	if (status == NU_SUCCESS)
	{
		pCMB_ControlBlock->CMBflags |= CMB_LISR_REG_BIT1 ;
// Create high level interrupt service routine 
		status = NU_Create_HISR(&pCMB_ControlBlock->CMB_HISR, 
			"CmbISRhi",				// name assigned to HISR
			 &CMB_ISR_High,			// function entry point for HISR
			CMB_HISR_PRIORITY,
			pCMB_ControlBlock->pHISR_stack,
			stack_size);
		if (status == NU_SUCCESS)
		{
			pCMB_ControlBlock->CMBflags |= CMB_HISR_CREATE_BIT ;
			status = NU_Register_LISR(pCMB_ControlBlock->ISR_BufferEmptyNumber,
										&CMB_ISR_Low, &old_lisr);
			if (status == NU_SUCCESS)
			{
				pCMB_ControlBlock->CMBflags |= CMB_LISR_REG_BIT2 ;
			}
		}
	}

	return status;
	
} // CMB_ISR_Create

/*************************************************************************/
// CMB_ISR_Destroy
// Destroy CMB_ISR object.
/*************************************************************************/
void CMB_ISR_Destroy()
{
	// TODO
	
} // CMB_ISR_Destroy


/* initialize assumes that if it is called the low level functions of  */
/* Who am I ? and Who is master have been performed and set in the flags  */

void	CMB_Initialize() 
{
	CMB_CONTROL_BLOCK			*pCMB_CB ;
	unsigned char	*pstatus, status ;
	unsigned char	*pdata, data ;

	//  save away boot ROM's detected slot number for our board:
	CMB_My_ID = (U8) Address::iSlotMe;
	
	pstatus = CMB_STATUS_REG_ADDR ;
	pdata = CMB_DATA_REG_ADDR ;

		// Dummy read of status and data
	status = *pstatus ;
	data = *pdata;
	data = 0x0;
	pCMB_CB = &CMB_ControlBlock ;
	pCMB_CB->CMB_Xstate = CMB_XMTR_RDY ;
	pCMB_CB->CMB_Rstate = CMB_RCVR_RDY ;
	pCMB_CB->CMBstatus = 0;
	pCMB_CB->CMBdata = 0;
	pCMB_CB->CMB_Xcount = 0;
	pCMB_CB->CMB_Xindex = 0;
	pCMB_CB->pCMB_Xbuffer = NULL ;
	pCMB_CB->CMB_XnibIndex = 0;
	pCMB_CB->CMB_Rcount = 0;
	pCMB_CB->CMB_Rindex = 0;
	pCMB_CB->CMB_RnibIndex = 0 ;
	pCMB_CB->CMBflags = 0 ;
	pCMB_CB->CMB_bSendingReply = FALSE;
	pCMB_CB->CMB_RcurrentX = 0 ;				// 0-7 buffer index for current receive	
	pCMB_CB->CMB_RprocessX = 0 ;				// 0-7 buffer index for next buffer to process	
	pCMB_CB->ISR_DataReadyNumber = CMB_DATA_RDY_INT_VECTOR ;
	pCMB_CB->ISR_BufferEmptyNumber = CMB_XMT_EMPTY_INT_VECTOR ;
	// create ISR for MIPS
	CMB_ISR_Create  (pCMB_CB, 4096) ;

}
void	BuildCMBreceived(char *pOutstring)
{
	unsigned i=0, more=1, count;
	unsigned char hexchar, data_count ;
	CMB_CONTROL_BLOCK	*pCMB_CB ;

	pCMB_CB = &CMB_ControlBlock ;

	count = CMB_MAX_MESSAGE_INDEX*pCMB_CB->CMB_RprocessX;

	while (more)
	{
		hexchar = pCMB_CB->CMB_Rbuffer[i+count] ;
		*pOutstring = hexchar >> 4 ;
		*pOutstring += 0x30 ;
		if (*pOutstring > 0x39)
				*pOutstring += 7 ;
		pOutstring++ ; // step ptr
		*pOutstring = hexchar & 0x0f ;
		*pOutstring += 0x30 ;
		if (*pOutstring > 0x39)
				*pOutstring += 7 ;
		pOutstring++ ; // step ptr
		*pOutstring = 0x20 ; // space
		pOutstring++ ; // step ptr

		switch(i)
		{
		case CMB_PCKT_DEST:
		case CMB_PCKT_SRCE:
		case CMB_PCKT_CMD:
		case CMB_PCKT_STATUS:
			break ;
		case CMB_PCKT_COUNT:
			data_count = hexchar ;
			break ;
		case CMB_PCKT_HDR_CRC:
			if (data_count == 0)
				more = 0 ;
			break ;
		default:
			if (data_count == 0)
				more = 0 ;
			else
				data_count-- ;
			break ;
		}
		i++ ;
	}
	*pOutstring = 0 ; // null terminate string
// step processed input messages
	pCMB_CB->CMB_RprocessX++;
	pCMB_CB->CMB_RprocessX &= CMB_RECEIVE_BUFFER_INDEX_MASK ; // modulo # of receive buffers
}

/*************************************************************************/
// CMB_ISR_High
// High Level Interrupt Service Routine for the CMB
// gets activated by CMB_ISR_Low.
/*************************************************************************/
void CMB_ISR_High(VOID)
{
	CMB_CONTROL_BLOCK		*pCMB_CB ;

	Critical CSect;		// do mutex-y stuff
	
	pCMB_CB = &CMB_ControlBlock ;
	
	CSect.Leave ();		// don't be critical while doing text output
	TRACEF(TRACE_L8,("Signaling input received (or reply sent)\n")) ;
			
#ifndef CT_TEST_HW_INTF
		  //  use standard DDM signalling model.  Note that we pass a BOOL
		  //  veiled as a (void *) to keep Signal() happy.  The sense of
		  //  the argument is "message ready to read".
		  //  WARNING: we assume that pCMB_CB->bSendingReply has the same
		  //           value as it did in CMB_ISR_Low().  This is only true
		  //           if higher-level CMB DDM code strictly serializes
		  //           calls to CMB_SendCommand(), which it does.
		  pHwHostDdm->Signal(nSigCmbHwIrq, (void *) !pCMB_CB->CMB_bSendingReply);
#else
		  //  our test jig uses a funny Nucleus-only signalling model
		  STATUS  status;
		status =  NU_Send_To_Queue(&Queue_1, &Send_Message, 1, NU_NO_SUSPEND);
#endif
	
} // CMB_ISR_High

void	SendFirstNibble (unsigned char data)
{
		data >>= 4 ; // get high nibble
		data |= CMB_SOH_FIRST ; //set 1st byte of packet bit 
		SendCMBbyte(data) ;
}

void	SendSecondNibble (unsigned char data)
{
		data &= 0x0f ; // get low nibble
		data |= CMB_SOH_BIT ; //set 1st byte of packet bit 
		SendCMBbyte(data) ;
}

void	SendHighNibble (unsigned char data)
{
		data >>= 4 ; // get high nibble
		data |= CMB_FIRST_NIB_BIT ; //set high nibble bit
		SendCMBbyte(data) ;
}


void	SendLowNibble (unsigned char data)
{
		data &= 0x0f ; // get low nibble
		SendCMBbyte(data) ;
}

/*******************************************************************/
//
//	Routine to continue MIPS output to CMA 
//
/*******************************************************************/
void	CMB_ProcessXISR (CMB_CONTROL_BLOCK	*pCMB_CB) 
{
		unsigned char	data, nxtdata ;
		unsigned		index ;
		
		index = pCMB_CB->CMB_Xindex ; 
		if (pCMB_CB->CMB_Xstate == CMB_XMTR_SENDING)
		{ /* sending a command */
			data = pCMB_CB->pCMB_Xbuffer[index] ;
			nxtdata = pCMB_CB->pCMB_Xbuffer[index+1] ;
		}
 		if (pCMB_CB->CMB_XnibIndex)
		{ /* have sent 2nd nibble of byte */
			pCMB_CB->CMB_XnibIndex = 0 ;	
			if (index == CMB_PCKT_COUNT)
			{ /* save transmission count */
				pCMB_CB->CMB_Xcount = data ;
			}
			else if	(index > CMB_PCKT_COUNT)
			{ /* count down to end of transmission */
				if (pCMB_CB->CMB_Xcount == 0)
				{ /* transmission complete - go to DONE */
					pCMB_CB->CMB_Xstate = CMB_XMTR_DONE ;
					return ;
				}
//				else if (index != CMB_PCKT_HDR_CRC)	  don't send 2nd CRC to CMA
				else
					pCMB_CB->CMB_Xcount-- ;
			}
			pCMB_CB->CMB_Xindex++ ;
			SendHighNibble (nxtdata) ;
		}
		else
		{/* send second half of byte */
			pCMB_CB->CMB_XnibIndex = 1 ;	
			if (index)
				SendLowNibble (data) ;
			else
				SendSecondNibble (data) ;
		}
}

/*************************************************************************/
// Low Level Interrupt Service Routine for the CMB
// Come here when receive interrupt
/*************************************************************************/
void CMB_ISR_Low(INT vector_number)
{

	CMB_CONTROL_BLOCK	*pCMB_CB ;
	STATUS				nu_status ;
	unsigned char	*pstatus, status ;
	unsigned char	*pdata, *pInBuffer, data ;

	unsigned i, count;

	pCMB_CB = &CMB_ControlBlock ;

	if (vector_number == pCMB_CB->ISR_DataReadyNumber)
	{
// Low level ISR called when data available from CMA
// get data - clears ISR		
		pdata = CMB_DATA_REG_ADDR ;
		data = *pdata ;

      //BUGBUG - track each raw byte received
      CmbDebugHwIntfLog.RecordByteRx(data);
      //BUGBUG - endit

		TRACEF(TRACE_L8,("CMB_ISR_DataReady: data = 0x%02X\n", data)); 		

		count = CMB_MAX_MESSAGE_INDEX*pCMB_CB->CMB_RcurrentX ;
		i = pCMB_CB->CMB_Rindex ;
		i += count ;

		pInBuffer = &pCMB_CB->CMB_Rbuffer[i] ;
	
		switch (pCMB_CB->CMB_Rstate)
		{
			case CMB_RCVR_IDLE:
			/* not initialized - ignore	input */
				break ;	
			case CMB_RCVR_RDY:	 
				pCMB_CB->CMB_Rindex = 0 ;
				pInBuffer = &pCMB_CB->CMB_Rbuffer[count] ;
				if (pCMB_CB->CMB_RnibIndex)
				{ // 2nd half of destination
					pCMB_CB->CMB_RnibIndex = 0 ;
					if ((data & CMB_PROTO_BITS_MASK) != CMB_SOH_BIT)
					{ // s/b 2nd nibble of 1st byte - if not start over
						return ;
					}
					*pInBuffer |= (data & 0x0f) ; // OR in 2nd nibble
					if ( ((*pInBuffer & ~CmbAddrMips) == CMB_My_ID) || (*pInBuffer & CMB_BROADCAST_BIT) )
					{ // message for me - go to busy receiving
						pCMB_CB->CMB_Rstate = CMB_RCVR_BUSY ;
						pCMB_CB->CMB_Rindex = 1 ;
					} 
				}
				else
				{ // 1st half of destination
					if ((data & CMB_PROTO_BITS_MASK) == CMB_SOH_FIRST)
					{ // high nibble of 1st byte - accept as start of hdr high nibble
						*pInBuffer = data << 4 ;							 
						pCMB_CB->CMB_RnibIndex = 1 ;
					}
					else
					{
						return ; // not SOH
					}
				}
				break ;
			case CMB_RCVR_BUSY:
				if (pCMB_CB->CMB_RnibIndex)
				{ // 2nd half of input - low nibble => 0 high order 4 bits
					pCMB_CB->CMB_RnibIndex = 0 ;
					if ((data & CMB_PROTO_BITS_MASK) != CMB_SECOND_NIB) 
					{ // not 2nd half of succeeding byte - start over
						pCMB_CB->CMB_Rstate = CMB_RCVR_RDY ;
						pCMB_CB->CMB_Rindex = 0 ;
						if ((data & CMB_PROTO_BITS_MASK) == CMB_SOH_FIRST)
						{ /* start of message */
							pCMB_CB->CMB_Rbuffer[count] = data << 4 ;
							pCMB_CB->CMB_RnibIndex = 1 ;
						}
						return ;
					}
					*pInBuffer |= data  ; // OR in 2nd nibble
					if (pCMB_CB->CMB_Rindex == CMB_PCKT_COUNT)
					{ /* set data count */
						pCMB_CB->CMB_Rcount = *pInBuffer ;
					}
					else if (pCMB_CB->CMB_Rindex > CMB_PCKT_COUNT)
					{ /* in data phase or receiving CRC */
						if (pCMB_CB->CMB_Rcount == 0)
						{ /* CRC byte received - done with receive */
	  					  /* wake up our class to signal CMB_DDM that we have something */
	  						pCMB_CB->CMB_Rstate = CMB_RCVR_DONE ;
							pCMB_CB->CMB_RcurrentX++  ;
							pCMB_CB->CMB_RcurrentX &= CMB_RECEIVE_BUFFER_INDEX_MASK;
							pCMB_CB->CMB_Rstate = CMB_RCVR_RDY ;
							nu_status = NU_Activate_HISR(&pCMB_CB->CMB_HISR);
						}
						else
							pCMB_CB->CMB_Rcount-- ; 
						
					}
					pCMB_CB->CMB_Rindex ++ ;
				}
				else
				{ // 1st half of data
					if ((data & CMB_PROTO_BITS_MASK) == CMB_FIRST_NIB_BIT)
					{ // high nibble of 1st byte - accept as start of hdr high nibble
						*pInBuffer = data << 4 ;							 
						pCMB_CB->CMB_RnibIndex = 1 ;
					}
					else
					{
						pCMB_CB->CMB_Rstate = CMB_RCVR_RDY ;
						pCMB_CB->CMB_Rindex = 0 ;
						if ((data & CMB_PROTO_BITS_MASK) == CMB_SOH_FIRST)
						{ /* start of message */
							pCMB_CB->CMB_Rbuffer[count] = data << 4 ;
							pCMB_CB->CMB_RnibIndex = 1 ;
						}
					}
				}
				break ;
		default:
				/* ignore */
				break ;
		}
	}
	else if (vector_number == pCMB_CB->ISR_BufferEmptyNumber)
	{ /* XMTR buffer empty interrupt */
	
		//  acknowledge AVR's from-MIPS buf empty interrupt, and prepare
		//  hardware for us to do another write to the AVR.
		pstatus = CMB_STATUS_REG_ADDR ;
		status = *pstatus ;
		
		//  if we have more to write to the AVR, do it.
		if (pCMB_CB->CMB_Xstate == CMB_XMTR_SENDING)
		{
			CMB_ProcessXISR (pCMB_CB) ;
			if (pCMB_CB->CMB_Xstate == CMB_XMTR_DONE)
			{
				//  finished sending - shift back to ready state
				pCMB_CB->CMB_Xstate = CMB_XMTR_RDY ;

				//  we only notify the sender if no reply is expected.
				//  This happens just when we are sending a reply.
				if (pCMB_CB->CMB_bSendingReply)
				{
					//  in this case, we do want to notify of send completion,
					//  since we'll not receive any confirming reply.
					nu_status = NU_Activate_HISR(&pCMB_CB->CMB_HISR);
				}
			}
		}
	}
} // CMB_ISR_Low

void	SendCMBbyte(unsigned char data)
{
	unsigned char	*pdata;
	pdata = CMB_DATA_REG_ADDR ;
	
TRACEF(TRACE_L8,("Sent byte =	0x%02X\n", data));

   //BUGBUG - trace raw interface bytes sent
   CmbDebugHwIntfLog.RecordByteTx (data);
   //BUGBUG - endit

	*pdata = data ; /* send data i.e. put in data register */
	return ;		
}
			

STATUS	CMB_SendCommand(CmbPacket *pBuffer)
{
	STATUS	SendStatus ;
	CMB_CONTROL_BLOCK			*pCMB_CB ;
	unsigned char	data ;

	pCMB_CB = &CMB_ControlBlock ;
	if (pCMB_CB->CMB_Xstate == CMB_XMTR_RDY)
	{/* this is start output - output will continue on XMTR interrupt */
		pCMB_CB->CMB_Xstate = CMB_XMTR_SENDING ;
		pCMB_CB->pCMB_Xbuffer =	(U8 *) pBuffer ;
		pCMB_CB->CMB_Xindex = 0 ;
		pCMB_CB->CMB_XnibIndex = 0 ;
		pCMB_CB->CMB_bSendingReply = ((pBuffer->Hdr.bStatus & CmbStatCmd) == 0);
		data = *(U8 *) pBuffer ;
		SendFirstNibble (data) ;
		SendStatus = CTS_SUCCESS;
	}
	else
		SendStatus = CTS_CMB_XMTR_BUSY;
	return (SendStatus) ;
}

