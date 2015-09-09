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
// File: CmbIsr.h
// 
// Description:
// Header file for the ISR for the MIPS CMB IF (E1 boards).
// 
// $Log: /Gemini/Odyssey/DdmCmb/CmbIsr.h $
// 
// 9     8/26/99 12:32p Ataylor
// Check on bit 0x40 for acceptance of input (broadcast). Will work for
// unsolicited 0xfc and for ox7d which we get on acks to local cmds
// 
// 8     8/26/99 11:17a Ataylor
// defined new masks & constants
// 
// 7     8/19/99 3:13p Jlane
// Use new receive buffer mask.
// 
// 6     7/22/99 11:40a Ataylor
// Added test for unsolicited response address
// 
// 5     7/21/99 8:06p Ataylor
// Added state to differentiate unsolicited command from response
// 
// 4     7/21/99 11:48a Ataylor
// Added new states for ACKing CMA and for aborting command on unsolicted
// input
// 
// 3     7/16/99 1:20p Ataylor
// Removed all simulation & E1 ifdefs - for E2 boards only
// 
// 2     6/18/99 10:49a Rkondapalli
// Added alternate def for CMB_INT_BIT (presently not used), rearranged
// members of CMB_CONTROL_BLOCK to fix metrowerks/mips alignment problems,
// changed var decls to be extern.  [at/ew]
// 
// 1     6/17/99 1:56p Ewedel
// Initial checkin.
//
//  05/06/99 - Created by At Taylor
// 
/*************************************************************************/


#ifndef OsTypes_H
# include  "OsTypes.h"
#endif

#ifndef __DdmOsServices_h
# include  "DdmOsServices.h"
#endif

#ifndef _CmbHwIntfMsgs_h_
# include  "CmbHwIntfMsgs.h"
#endif

#include "types.h"
#ifndef		_CMBISR
#define		_CMBISR

#define	CMB_HISR_STACK_SIZE 4096

/* These offsets are the parameters for the E2 boards */
//	Interrupt for MIPS - Data Ready
#define	CMB_DATA_RDY_INT_VECTOR			2
#define	CMB_DATA_REG_ADDR		((U8 * )0xBC0C0000)
// Interrupt for MIPS - Xmt Buffer Empty
#define	CMB_XMT_EMPTY_INT_VECTOR		3
#define	CMB_STATUS_REG_ADDR		((U8 * )0xBC0B0000)

/* define transmitter states and poll/interrupt */
#define	CMB_XMTR_IDLE		0x00
#define	CMB_XMTR_RDY		0x01
#define	CMB_XMTR_SENDING	0x02
#define	CMB_XMTR_WAITING	0x03
#define CMB_XMTR_ACKNAK		0x04
#define	CMB_XMTR_DONE		0x05
#define	CMB_XMTR_ABORT		0x06

/* define receiver states */
#define	CMB_RCVR_IDLE			0x00
#define	CMB_RCVR_RDY			0x01
#define	CMB_RCVR_BUSY			0x02
#define	CMB_RCVR_BUSY_UNSOL		0x03
#define	CMB_RCVR_BUSY_UNSOLCMD	0x04
#define	CMB_RCVR_BUSY_REPLY		0x05
#define	CMB_RCVR_DONE			0x06

/* define BUSY substates - 1st 5 correspond to Rindex and Xindex */
/* data state entered if count > 0 - remain in state for count data bytes */
#define	CMB_PCKT_DEST			0x00
#define	CMB_PCKT_SRCE			0x01
#define	CMB_PCKT_CMD			0x02
#define	CMB_PCKT_STATUS			0x03
#define	CMB_PCKT_COUNT			0x04
#define	CMB_PCKT_HDR_CRC		0x05
#define	CMB_PCKT_DATA			0x06
#define	CMB_PCKT_DATA_CRC		0x07
/* special addresses */
#define CMB_UNSOL_ADDR			0xFC
/* define bit masks for the status byte in the message */
#define	CMB_CMDRSP_BIT		0x80	
#define	CMB_ACKNAK_BIT		0x40	
#define	CMB_CMDAVAIL_BIT	0x20

/* define receive buffer index mask & max index */
#define CMB_RECEIVE_BUFFER_INDEX_MASK	0x7F	
#define CMB_RECEIVE_BUFFER_MAX_INDEX	0x80	
#define CMB_MAX_MESSAGE_INDEX			32	

/* define flag bits */
#define	CMB_MASTER_BIT		0x01	
#define	CMB_LISR_REG_BIT1	0x02	
#define	CMB_LISR_REG_BIT2	0x04	
#define	CMB_HISR_CREATE_BIT	0x08

/* define message bits and bytes */
/* destination addresses */
#define CMB_MIPS_UNSOL_ADDR	0xFC
#define CMB_CMA_LOCAL_ADDR	0x7D
/* nibble bits */
#define CMB_PROTO_BITS_MASK	0xF0
#define CMB_SOH_BIT			0x20
#define CMB_BROADCAST_BIT	0x40
#define CMB_FIRST_NIB_BIT	0x10
#define	CMB_SOH_FIRST		CMB_SOH_BIT | CMB_FIRST_NIB_BIT	
#define	CMB_SECOND_NIB		0x00

extern unsigned char  CMB_My_ID;
extern unsigned char  CMBflags ; 	

typedef	struct _CMB_CONTROL_BLOCK {
	unsigned char		  CMBstatus;
	unsigned char		  CMBdata;
	unsigned char		  CMB_Xstate;
	unsigned char		  CMB_Xcount;
	unsigned char		  CMB_Xindex;
	unsigned char		  CMB_XnibIndex ;
	unsigned char		  CMB_Rstate;
	unsigned char		  CMB_Rcount;
	unsigned char		  CMB_Rindex;
	unsigned char		  CMB_RnibIndex ;
	unsigned char		  CMB_RcurrentX	;			// 0-7 buffer index for current receive	
	unsigned char		  CMB_RprocessX	;			// 0-7 buffer index for next buffer to process	
	unsigned char		  *pCMB_Xbuffer ; 
	unsigned char		  CMB_Rbuffer[4096] ;       // 128 * 32 byte CMB packets
	unsigned char		  CMB_AckBuffer[6] ;		// no longer used
	VOID				  *pHISR_stack;				// points to HISR stack area
	int					  ISR_DataReadyNumber ;
	int					  ISR_BufferEmptyNumber ;
	unsigned char		  CMBflags ;
	unsigned char		  CMB_bSendingReply;
	NU_HISR				  CMB_HISR;					// HISR object
} CMB_CONTROL_BLOCK, *pCMB_CONTROL_BLOCK;
extern CMB_CONTROL_BLOCK	CMB_ControlBlock ;

void	BuildCMBreceived(char *pOutstring);
STATUS	CMB_ISR_Create  (CMB_CONTROL_BLOCK *pCMB_ControlBlock, UNSIGNED stack_size) ;
void	CMB_Initialize(void) ; 
void CMB_ISR_High(VOID) ;
void CMB_ISR_Destroy(void) ;
void	SendFirstNibble (unsigned char data) ;
void	SendSecondNibble (unsigned char data);
void	SendHighNibble (unsigned char data)	 ;
void	SendLowNibble (unsigned char data)	 ;
void	CMB_ProcessXISR (CMB_CONTROL_BLOCK	*pCMB_CB) ;
void CMB_ISR_Low(INT vector_number);
void	SendCMBbyte(unsigned char data);
STATUS	CMB_SendCommand(CmbPacket *pBuffer);

   //  signal code from our host DDM's code space.  This signal is reserved
   //  for our internal use in propagating AVR-to-MIPS IRQs.
extern SIGNALCODE  nSigCmbHwIrq;

   //  pointer to our host DDM, so we can signal it
extern DdmServices     * pHwHostDdm;

#endif		/* _CMBISR */
