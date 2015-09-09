/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiISP.h
// 
// Description:
// This file defines registers and other hardware addresses
// for the ISP1040B.
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiISP.h $
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#if !defined(HscsiISP_H)
#define HscsiISP_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

// Methods to be called by HSCSI_Event_Task 
STATUS	HSCSI_ISP_Init();

// Called by Send_IOCB in IOCB.c
STATUS HSCSI_Execute_IOCB();
STATUS HSCSI_Marker_IOCB(); // needed for handling async resets

#define MAXIMUM_FRAME_SIZE 1024

/************************************************************************/
/*		Host Command/control Register Command Definitions   	*/
/************************************************************************/

#define	HCCRNOP			0x0000		/* No Operation */
#define	HCCRRESETRISC	0x1000		/* Reset the RISC Processor */
#define	HCCRPAUSERISC	0x2000		/* Pause the RISC Processor */
#define	HCCRRLSRISC		0x3000		/* Release RISC from Reset or Pause */
#define	HCCRSTEPRISC	0x4000		/* Single Step the RISC */
#define	HCCRSETH2RINTR	0x5000		/* Set the Host to RISC Interrupt */
#define	HCCRCLRH2RINTR	0x6000		/* Clear the Host to RISC Interrupt */
#define	HCCRCLRR2HINTR	0x7000		/* Clear the RISC to Host Interrupt */
#define	HCCRWRPBENABLES	0x8000		/* Write P/B Enables */
#define HCCRDISABLEBIOS 0x9000		/* Disable BIOS access (Undocumneted) */

/************************************************************************/
/*		Host Command/control Register Bit Definitions		*/
/************************************************************************/
#define	HCCRBRKPT0		0x0004		/* Bit for Enable/Disable Breakpoint 0 */
#define	HCCRBRKPT1		0x0008		/* Bit for Enable/Disable Breakpoint 1 */
#define	HCCRBRKPTEXT	0x0010		/* Bit for Enable/Disable Ext Brkpoint */
#define	HCCRRPAUSED		0x0020		/* RISC is in Pause Mode */
#define	HCCRRRESET		0x0040		/* RISC is in Reset Mode */
#define	HCCRH2RINTR		0x0080		/* Host to RISC Interrupt status */

/*************************************************************************/
//	Bus Interface Registers 				
/*************************************************************************/
#define HSCSI_MAILBOX_0			0x0070	
#define HSCSI_MAILBOX_1			0x0072	
#define HSCSI_MAILBOX_2			0x0074	
#define HSCSI_MAILBOX_3			0x0076	
#define HSCSI_MAILBOX_4			0x0078	
#define HSCSI_MAILBOX_5			0x007a
#define HSCSI_MAILBOX_6			0x007c	
#define HSCSI_MAILBOX_7			0x007e	
#define HSCSI_SEMAPHORE			0x000c
#define HSCSI_HCCR				0x00c0 // Host Command and control Register
#define HSCSI_CONFIG_1			0x0006
#define HSCSI_PCI_INT_CONTROL	0x0008
#define HSCSI_PCI_INT_STATUS	0x000a

/************************************************************************/
//  ISP to PCI Interrupt Status Register Definitions (Isp2PciIntSts)	
/************************************************************************/
#define HSCSI_RISC_INT_PENDING 	0x0004
#define HSCSI_CDMA_INT_PENDING 	0x0010
#define HSCSI_DDMA_INT_PENDING 	0x0020


/*************************************************************************/
// Mailbox message consists of the contents of all 8 mailboxes
/*************************************************************************/
typedef struct _HSCSI_MAILBOX_MESSAGE
{
	U16	mailbox[8];
} HSCSI_MAILBOX_MESSAGE;

/*************************************************************************/
// Mailbox commands
/*************************************************************************/
#define	HSCSI_MBC_NOP				0x00
#define	HSCSI_MBC_LOAD_RAM			0x01		// 32 Bit
#define HSCSI_MBC_EXE_FW			0x02
#define HSCSI_MBC_DUMP_RAM			0x03		// 32 Bit
#define HSCSI_MBC_WRT_RAM_WORD		0x04
#define HSCSI_MBC_RD_RAM_WORD		0x05
#define HSCSI_MBC_MB_REG_TEST		0x06
#define HSCSI_MBC_VER_CHKSUM		0x07
#define HSCSI_MBC_ABOUT_PROM		0x08
#define	HSCSI_MBC_CHSUM_FW			0x0E
#define HSCSI_MBC_INIT_REQUEST		0x10
#define HSCSI_MBC_INIT_RESPONSE		0x11
#define HSCSI_MBC_EXECUTE_IOCB		0x12
#define	HSCSI_MBC_WAKEUP			0x13
#define	HSCSI_MBC_RESET				0x18
#define HSCSI_MBC_START_QUEUE		0x1A
#define HSCSI_MBC_SET_ID			0x30
#define HSCSI_MBC_SET_SEL_TIMEOUT	0x31
#define HSCSI_MBC_SET_RETRY_COUNT	0x32
#define HSCSI_MBC_SET_TAG_AGE		0x33
#define HSCSI_MBC_SET_CR			0x34
#define HSCSI_MBC_SET_ACTIVE_NEG	0x35
#define HSCSI_MBC_ASYNC_SETUP		0x36
#define HSCSI_MBC_SET_BUS_CONTROL	0x37
#define HSCSI_MBC_SET_SCSI_PARM		0x38
#define HSCSI_MBC_SET_DEVICE		0x39
#define HSCSI_MBC_SET_SYSTEM_PARM	0x45
#define HSCSI_MBC_SET_FIRMWARE		0x4A
#define HSCSI_MBC_SET_OVERRUN		0x5A

/*************************************************************************/
// Mailbox return status
/*************************************************************************/
#define MB_STS_GOOD			0x4000

/*************************************************************************/
//Asynchronous events
/*************************************************************************/


#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* HscsiISP_H  */
