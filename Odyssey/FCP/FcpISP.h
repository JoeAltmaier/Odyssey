/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpISP.h
// 
// Description:
// This file defines registers and other hardware addresses
// for the ISP2100.
// 
// Update Log 
// 5/5/98 Jim Frandeen: Use C++ comment style
// 4/14/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 7/23/98 Mike Panas: Add more status and AE info
// 9/2/98 Michael G. Panas: add C++ stuff
/*************************************************************************/

#if !defined(FcpISP_H)
#define FcpISP_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

// Methods to be called by FCP_Event_Task 
STATUS	FCP_ISP_Init();

#define MAXIMUM_FRAME_SIZE 1024

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

/*************************************************************************/
// Initialize Firmware control Block
//	Base version for 2100 and 2200
/*************************************************************************/
#pragma pack(1)
typedef struct _IFWCB
{
    U8		version;
    U8		reserved;
    U16		options;
    U16		frame_size;
    U16		num_buffers_max;
    U16		throttle;
    U8		retry_count;
    U8		retry_delay;
    U8		port_name[8];
    U16		hard_address;
    U8		inquiry_data;
    U8		reserved1;
    U8		node_name[8];
    U16		request_queue_index;
    U16		response_queue_index;
    U16		request_queue_size;
    U16		response_queue_size;
    U32		request_queue_address0;
    U32		request_queue_address1;
    U32		response_queue_address0;
    U32		response_queue_address1;
} INITIALIZE_FIRMWARE_CONTROL_BLOCK;

/*************************************************************************/
// ECB Extended Control Block
/*************************************************************************/
typedef	struct _ECB 
{
	U16		LUN_enables;
	U8		command_resource_count;
	U8		immediate_notify_res_count;
	U16		timeout;
	U16		reserved;
	U16		ext_options;
	U8		response_acc_timer;
	U8		interrupt_delay_timer;
	U16		special_options;
	U8		reserved2[26];
} ECB;

/*************************************************************************/
// ISP2200 Virtual Port Configuration record
/*************************************************************************/
typedef	struct	_VP_CONFIG
{
	U8		options;
	U8		hard_ID;
    U8		port_name[8];
    U8		node_name[8];
} VP_CONFIG;

/*************************************************************************/
// ISP2200 Virtual Port Database Entry
// returned by the MBC_GET_VP_DATA or the MBC_GET_VP_DB_ENTRY commands
// 32 of these are returned by the MBC_GET_VP_DATA command
/*************************************************************************/
typedef	struct	_VP_DATA
{
	U8		options;
	U8		hard_ID;
    U8		port_name[8];
    U8		node_name[8];
    U16		status;
    U8		pcb_allocation;
    U8		pending_logout_acc_cnt;
    U16		acquired_loop_ID;			// VPLID
    U16		LUN_flags;
    U16		command_resource_cnt;
    U16		imm_notify_resource_count;
    U16		LUN_timeout;
    U16		init_asso_table_pointer;
    U16		ALPA;
    U16		reserved[2];
} VP_DATA;

/*************************************************************************/
// ISP2200 standard IFWCB or ISP2100 Extended IFWCB
/*************************************************************************/
typedef	struct	_IFWCB_ECB
{
	INITIALIZE_FIRMWARE_CONTROL_BLOCK	cb;
	ECB									ecb;
} IFWCB_ECB;

/*************************************************************************/
// ISP2200 Multiple ID IFWCB
/*************************************************************************/
typedef	struct	_2200_IFW_MID_CB
{
	INITIALIZE_FIRMWARE_CONTROL_BLOCK	cb;
	ECB									ecb;
	U8									vp_count;
	U8									vp_options;
	VP_CONFIG							vp_config[31];
} IFWCB_2200;

/*************************************************************************/
// Mailbox message consists of the contents of all 8 mailboxes
/*************************************************************************/
typedef struct _MAILBOX_MESSAGE
{
	U16	mailbox[8];
} FCP_MAILBOX_MESSAGE;


/*************************************************************************/
// Firmware Options
/*************************************************************************/
#define IFCB_ECB						0x8000
#define IFCB_NAME_OPTION				0x4000			// Bits 6-14 are from Addendum
#define IFCB_FULL_LOGIN_AFTER_LIP		0x2000
#define IFCB_STOP_PORT_QUE_FULL_STS		0x1000
#define IFCB_PREV_ASSIGNED_LID			0x0800
#define IFCB_DECENDING_LID_SRCH			0x0400
#define IFCB_DISABLE_INITIAL_LIP		0x0200
#define IFCB_ENABLE_PDB_CHGD_AE			0x0100
#define IFCB_ENABLE_TARGET_DEV_TYPE		0x0080
#define IFCB_ENABLE_ADISC				0x0040
#define IFCB_INITIATOR_MODE_DISABLE		0x0020
#define IFCB_TARGET_MODE_ENABLE			0x0010
#define IFCB_FAST_STATUS_POSTING		0x0008
#define IFCB_FULL_DUPLEX				0x0004
#define IFCB_FAIRNESS					0x0002
#define IFCB_HARD_ADDRESS_ENABLE		0x0001

/*************************************************************************/
// Mailbox commands
// Common to 2100/2200
/*************************************************************************/
#define	MBC_NOP						0x00
#define	MBC_LOAD_RAM				0x01		// 32 Bit
#define MBC_EXE_FW					0x02
#define MBC_DUMP_RAM				0x03		// 32 Bit
#define MBC_WRT_RAM_WORD			0x04
#define MBC_RD_RAM_WORD				0x05
#define MBC_MB_REG_TEST				0x06
#define MBC_VER_CHKSUM				0x07
#define MBC_ABOUT_PROM				0x08
#define	MBC_LOAD_RISC_RAM			0x09		// 64 Bit
#define	MBC_DUMP_RISC_RAM			0x0A		// 64 bit
#define	MBC_CHSUM_FW				0x0E
#define MBC_EXECUTE_IOCB			0x12
#define	MBC_WAKEUP					0x13
#define	MBC_RESET					0x18
#define	MBC_GET_LOOP_ID				0x20
#define MBC_INIT_FW					0x60
#define MBC_GET_ICB					0x61
#define MBC_INITIATE_LIP			0x62
#define MBC_GET_FC_AL_MAP			0x63
#define MBC_GET_PORT_DB				0x64
#define MBC_TARGET_RESET			0x66
#define MBC_GET_FW_STATE			0x69
#define MBC_GET_PORT_NAME			0x6A
#define MBC_GET_LINK_STATUS			0x6B
#define MBC_INITIATE_LIP_RESET		0x6C

#define MBC_LIP_FOLLOWED_BY_LOGIN	0x72
#define MBC_GET_LOOP_ID_LIST		0x75

/*************************************************************************/
// Mailbox commands
// Fabric support - these commands need the "f" version of firmware
// to be usable
/*************************************************************************/
#define MBC_SEND_SNS				0x6E
#define MBC_LOGIN_FABRIC_PORT		0x6F
#define MBC_SEND_CHANGE_REQ			0x70
#define MBC_LOGOUT_FABRIC_PORT		0x71

/*************************************************************************/
// Mailbox commands
// ISP2200 only
/*************************************************************************/
#define MBC_INIT_FW_MID				0x48
#define MBC_GET_VP_DATA				0x49
#define MBC_GET_VP_DB_ENTRY			0x4A
#define MBC_LOGIN_LOOP_PORT			0x74

/*************************************************************************/
// Mailbox return status
/*************************************************************************/
#define MB_STS_GOOD					0x4000
#define MB_STS_INVALID_CMD			0x4001
#define MB_STS_HOST_INTF_ERR		0x4002
#define MB_STS_TEST_FAILED			0x4003
#define MB_STS_CMD_ERR				0x4005
#define MB_STS_CMD_PARM_ERR			0x4006
#define MB_STS_PORT_ID_USED			0x4007
#define MB_STS_LOOP_ID_USED			0x4008
#define MB_STS_ALL_IDS_IN_USE		0x4009
#define MB_STS_NOT_LOGGED_IN		0x400A

/*************************************************************************/
//Asynchronous events
/*************************************************************************/

// IMMEDIATE_NOTIFY_COUNT is the initial number of immediate
// notifies allocated by the target driver.
#define IMMEDIATE_NOTIFY_COUNT		0x0016

// COMMAND_COUNT is the initial number of commands (ATIOs) allocated
// by the target driver.
//#define COMMAND_COUNT				0x0016
#define COMMAND_COUNT				255

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* FcpISP_H  */
