
#define	KSEG1		0xA0000000
#define Read_ISP(ISP_register)  *((U16*)(KSEG1 + ISP_BASE + ISP_register))
#define Write_ISP(ISP_register, value) \
    *((U16*)(KSEG1 + ISP_BASE + ISP_register)) = value;
	
	


/************************************************************************/
/*		Host Command/control Register Command Definitions   	*/
/************************************************************************/
#define RESET_RISC		0x1000
#define PAUSE_RISC		0x2000
#define RELEASE_RISC	0x3000

#define	HCTLNOP			0x0000		/* No Operation */
#define	HCTLRESETRISC	0x1000		/* Reset the RISC Processor */
#define	HCTLPAUSERISC	0x2000		/* Pause the RISC Processor */
#define	HCTLRLSRISC		0x3000		/* Release RISC from Reset or Pause */
#define	HCTLSTEPRISC	0x4000		/* Single Step the RISC */
#define	HCTLSETH2RINTR	0x5000		/* Set the Host to RISC Interrupt */
#define	HCTLCLRH2RINTR	0x6000		/* Clear the Host to RISC Interrupt */
#define	HCTLCLRR2HINTR	0x7000		/* Clear the RISC to Host Interrupt */
#define	HCTLWRPBENABLES	0x8000		/* Write P/B Enables */
#define HCTLDISABLEBIOS 0x9000		/* Disable BIOS access (Undocumneted) */

/************************************************************************/
/*		Host Command/control Register Bit Definitions		*/
/************************************************************************/
#define	HCTLBRKPT0		0x0004		/* Bit for Enable/Disable Breakpoint 0 */
#define	HCTLBRKPT1		0x0008		/* Bit for Enable/Disable Breakpoint 1 */
#define	HCTLBRKPTEXT	0x0010		/* Bit for Enable/Disable Ext Brkpoint */
#define	HCTLRPAUSED		0x0020		/* RISC is in Pause Mode */
#define	HCTLRRESET		0x0040		/* RISC is in Reset Mode */
#define	HCTLH2RINTR		0x0080		/* Host to RISC Interrupt status */

/*************************************************************************/
//	Bus Interface Registers 				
/*************************************************************************/
#define ISP_MAILBOX_0			0x0010	
#define ISP_MAILBOX_1			0x0012	
#define ISP_MAILBOX_2			0x0014	
#define ISP_MAILBOX_3			0x0016	
#define ISP_MAILBOX_4			0x0018	
#define ISP_MAILBOX_5			0x001a
#define ISP_MAILBOX_6			0x001c	
#define ISP_MAILBOX_7			0x001e	
#define ISP_SEMAPHORE			0x000c
#define ISP_HCCR				0x00C0 // Host Command and control Register
#define ISP_CONTROL_STATUS		0x0006
#define ISP_PCI_INT_CONTROL		0x0008
#define ISP_TO_PCI_INT_STATUS	0x000a

/************************************************************************/
//  ISP to PCI Interrupt Status Register Definitions (Isp2PciIntSts)	
/************************************************************************/
#define ISP2100_INT_PENDING	0x8000
#define FPM_INT_PENDING		0x0020
#define FB_INT_PENDING		0x0010
#define RISC_INT_PENDING 	0x0008
#define CDMA_INT_PENDING 	0x0004
#define RDMA_INT_PENDING 	0x0002
#define TDMA_INT_PENDING 	0x0001

