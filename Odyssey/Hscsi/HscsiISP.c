/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiISP.c
// 
// Description:
// This file implements the ISP1040B interface methods 
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiISP.c $
// 
// 2     10/18/99 7:03p Cchan
// Added function HSCSI_Set_SCSI to set SCSI selection retries to 1 rather
// than the default 8.
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/
#include "HscsiCommon.h"
#include "HscsiData.h"
#include "HscsiError.h"
#include "HscsiDebug.h"
#include "HscsiISP.h"
#include "HscsiMailbox.h"
#include "HscsiMemory.h"
#include "HscsiRequestFIFO.h"
#include "HscsiResponseFIFO.h"
#include "Nucleus.h"
#include "HscsiString.h"
#include "types.h"
#include "pcidev.h"

// RISC code to load ISP is in fw1040ei.c (initiator-only firmware)
extern unsigned short	hscsi_risc_code01[];
extern unsigned short   hscsi_risc_code_length01;
extern unsigned short	hscsi_risc_code_version;
extern unsigned short	hscsi_risc_code_addr01;

UNSIGNED	hscsi_risc_code_swapped = 0;

/*************************************************************************/
// Forward References
/*************************************************************************/
STATUS	HSCSI_Execute_RISC_Firmware(PHSCSI_INSTANCE_DATA Id);
STATUS	HSCSI_ISP_Init(PHSCSI_INSTANCE_DATA Id);
STATUS	HSCSI_Load_RISC_RAM(PHSCSI_INSTANCE_DATA Id);
STATUS	HSCSI_Mailbox_Test(PHSCSI_INSTANCE_DATA Id);
STATUS	HSCSI_Init_Request_Queue(PHSCSI_INSTANCE_DATA Id);
STATUS	HSCSI_Init_Response_Queue(PHSCSI_INSTANCE_DATA Id);
STATUS	HSCSI_Verify_RISC_Checksum(PHSCSI_INSTANCE_DATA Id);

STATUS	HSCSI_Execute_IOCB(PHSCSI_INSTANCE_DATA Id);
STATUS	HSCSI_Bus_Reset(PHSCSI_INSTANCE_DATA Id);
STATUS	HSCSI_Marker_IOCB(PHSCSI_INSTANCE_DATA Id);
STATUS	HSCSI_Set_SCSI(PHSCSI_INSTANCE_DATA Id);
STATUS	HSCSI_BIOS_RAM(PHSCSI_INSTANCE_DATA Id);

//*************************************************************************
// Globals
//*************************************************************************

/*************************************************************************/
// HSCSI_ISP_Create
// Create HSCSI_ISP object
// Start the ISP device.
// pp_memory points to a pointer to memory to be used.
// This object currently does not use any memory.
/*************************************************************************/
STATUS HSCSI_ISP_Create(PHSCSI_INSTANCE_DATA Id)
{
 	HSCSI_TRACE_ENTRY(HSCSI_ISP_Create);
 	
 	// Set up to print out values of ISR register as we read and write them
#if defined(HSCSI_DEBUG) && defined(_DEBUG)
 	Id->HSCSI_if_print_ISR = TRACE_L8;
#else
 	Id->HSCSI_if_print_ISR = 0;		// no TRACE
#endif

	return NU_SUCCESS;
	
} // HSCSI_ISP_Create

/*************************************************************************/
// HSCSI_ISP_Destroy
// Destroy HSCSI_ISP object.
/*************************************************************************/
void HSCSI_ISP_Destroy()
{

} // HSCSI_ISP_Destroy

/*************************************************************************/
// HSCSI_Execute_RISC_Firmware
// Verify RISC RAM command has completed.
/*************************************************************************/
STATUS HSCSI_Execute_RISC_Firmware(PHSCSI_INSTANCE_DATA Id)
{
	UNSIGNED	mailbox_status;
	STATUS		status = NU_SUCCESS;
	
 	HSCSI_TRACE_ENTRY(HSCSI_Execute_RISC_Firmware);
	
    status = HSCSI_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    
 	// Execute RISC firmware command
    Write_ISP1040(Id, HSCSI_MAILBOX_0, HSCSI_MBC_EXE_FW); 
    	
    // Destination address (RISC)
    Write_ISP1040(Id, HSCSI_MAILBOX_1, hscsi_risc_code_addr01); 
    	
	mailbox_status = HSCSI_Mailbox_Command(Id);

    if (mailbox_status != MB_STS_GOOD)
    {
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Execute_RISC_Firmware", 
			"Execute firmware failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return HSCSI_ERROR_EXECUTE_FIRMWARE;
	}

    return status;
    
} // HSCSI_Execute_RISC_Firmware


/*************************************************************************/
// HSCSI_ISP_Init
// Called by HSCSI_Event_Task to initialize the ISP.
/*************************************************************************/
STATUS HSCSI_ISP_Init(PHSCSI_INSTANCE_DATA Id)
{
	STATUS				 status = NU_SUCCESS;
	UNSIGNED			 timeout;
	U16					 mailbox_0;
	U16					 target_up = 0;
	
 	HSCSI_TRACE_ENTRY(HSCSI_ISP_Init);
 	
 	Id->HSCSI_state = HSCSI_STATE_INIT;
 	
	// Issue soft reset
	Write_ISP1040(Id, HSCSI_PCI_INT_CONTROL, 1); 
	
	Write_ISP1040(Id, HSCSI_HCCR, HCCRRESETRISC);
	Write_ISP1040(Id, HSCSI_HCCR, HCCRRLSRISC); 
	Write_ISP1040(Id, HSCSI_HCCR, HCCRDISABLEBIOS);
	
	// Wait for RISC to be Ready
	// timeout = HSCSI_TIMEOUT; 
	timeout = 10;
	mailbox_0 = Read_ISP1040(Id, HSCSI_MAILBOX_0);
	while(mailbox_0 != 0)
	{
		NU_Sleep(1);
		mailbox_0 = Read_ISP1040(Id, HSCSI_MAILBOX_0);
		HSCSI_PRINT_NUMBER(TRACE_L8, "\n\rmailbox_0 = ", mailbox_0);
		if (timeout-- == 0)
		{
			HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
				"HSCSI_ISP_Init", 
				"Waiting for RISC to be ready",
				mailbox_0,
				(UNSIGNED)Id);
			return HSCSI_ERROR_RISC_NOT_READY;
		}		
	}

	// Clear PCI Semaphore register
	Write_ISP1040(Id, HSCSI_SEMAPHORE, 0);
	
	// Burst Enable & FIFO Threshold Control bits set.
	Write_ISP1040(Id, HSCSI_CONFIG_1, 7); 
		
    // Enable RISC & PCI interrupt bits
	Write_ISP1040(Id, HSCSI_PCI_INT_CONTROL, 0x0006);
	
    status = HSCSI_Mailbox_Test(Id);
    if (status != NU_SUCCESS)
    	return status;
    	
    status = HSCSI_Load_RISC_RAM(Id);
    if (status != NU_SUCCESS)
    	return status;
	
#if 0
	// DEBUG mgp
	status = HSCSI_Dump_RISC_RAM(Id, Buffer, hscsi_risc_code_length01, 0x1000);
    if (status != NU_SUCCESS)
    	return status;
    	
    // compare data read to data sent
    {
    	U32		loop;
    	unsigned short	*p1, *p2;
    	
    	p1 = (unsigned short *)Buffer;
    	p2 = &hscsi_risc_code01[0];
    	for (loop = 0; loop < hscsi_risc_code_length01; loop++) {
     		if (*p1 != *p2) {
    			HSCSI_PRINT_HEX16(TRACE_L8, "\n\rMismatch - Addr = ", loop);
    			HSCSI_PRINT_HEX16(TRACE_L8, "\n\rp1 = ", *p1);
    			HSCSI_PRINT_HEX16(TRACE_L8, " p2 = ", *p2);
    			break;
    		}
    		p1++;
    		p2++;
    	}
    }
#endif

	status = HSCSI_Verify_RISC_Checksum(Id);
    if (status != NU_SUCCESS)
    	return status;
	
	status = HSCSI_Execute_RISC_Firmware(Id);
    if (status != NU_SUCCESS)
    	return status;

	status = HSCSI_Set_SCSI(Id);
	if (status != NU_SUCCESS)
		return status;
		
	status = HSCSI_Init_Request_Queue(Id);
	if (status != NU_SUCCESS)
		return status;
		
	status = HSCSI_Init_Response_Queue(Id);
	if (status != NU_SUCCESS)
		return status;

	status=HSCSI_Bus_Reset(Id);
	if (status != NU_SUCCESS)
		return status;
		
	status=HSCSI_Marker_IOCB(Id);
	if (status != NU_SUCCESS)
		return status;
		
	Id->HSCSI_state = HSCSI_STATE_ACTIVE;
		
	return status;
	    
} // HSCSI_ISP_Init

/*************************************************************************/
// HSCSI_Load_RISC_RAM
// Load RISC RAM from memory using Load RAM command.
/*************************************************************************/
STATUS HSCSI_Load_RISC_RAM(PHSCSI_INSTANCE_DATA Id)
{
	U16			 mailbox_status;
	STATUS		 status = NU_SUCCESS;
	U32			 dma_address;
	
 	HSCSI_TRACE_ENTRY(HSCSI_Load_RISC_RAM);
 	
    status = HSCSI_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;

#if defined(HSCSI_DEBUG) && defined(_DEBUG)
	// Prepare RISC RAM image to be loaded from memory
	HSCSI_PRINT_STRING(TRACE_L8, "\n\rLoading RISC RAM from memory using Load RISC RAM command.");
	HSCSI_PRINT_HEX16(TRACE_L8, "\n\rrisc_code_length01 = ", hscsi_risc_code_length01);
    HSCSI_PRINT_HEX16(TRACE_L8, "\n\r1st word of risc_code01 = ", hscsi_risc_code01[0]); 
    HSCSI_PRINT_HEX16(TRACE_L8, "\n\rlast word of risc_code01 = ", hscsi_risc_code01[hscsi_risc_code_length01 - 1]); 
	HSCSI_PRINT_HEX(TRACE_L8, "\n\rAddress of risc_code01 = ", (UNSIGNED)&hscsi_risc_code01[0]);

	// Read some words of RISC RAM to see what is there
	// before we load.
	HSCSI_PRINT_STRING(TRACE_L8, "\n\rRAM before load:");
	if (TraceLevel[TRACE_INDEX] >= TRACE_L8) {
		HSCSI_Print_RAM_Word(Id, 0); 
		HSCSI_Print_RAM_Word(Id, 1); 
		HSCSI_Print_RAM_Word(Id, hscsi_risc_code_length01 - 2); 
		HSCSI_Print_RAM_Word(Id, hscsi_risc_code_length01 - 1);
	}
#endif
	
	// note: this code is always needed
	if (hscsi_risc_code_swapped == 0)
	{
		// endian conversion for firmware download
		UNSIGNED	 index;
		U16			 *p;		// access is through KSEG1
		
		p = (U16 *)((UNSIGNED)&hscsi_risc_code01[0] | KSEG1);
		
		for (index = 0; index < hscsi_risc_code_length01; index++)
		{
			*p = BYTE_SWAP16(*p);
			p++;
		}
		// Just to make sure we really swap them all...
		HSCSI_PRINT_HEX16(TRACE_L8, "\n\rLast index = ", index - 1);
		HSCSI_PRINT_HEX16(TRACE_L8, "\n\rLast word swaped = ", hscsi_risc_code01[index - 1]);
		hscsi_risc_code_swapped = 1;	// only need to do this once
	}
	 
	dma_address = (U32)HSCSI_Get_DMA_Address((void *)&hscsi_risc_code01[0]);
	
 	// Load RISC RAM command
    Write_ISP1040(Id, HSCSI_MAILBOX_0, HSCSI_MBC_LOAD_RAM); // 32 bit load
    	
    // Destination address (RISC)
    Write_ISP1040(Id, HSCSI_MAILBOX_1, hscsi_risc_code_addr01);
    
    // Source address, bits 31-16
    // Mask off PCI bits
    Write_ISP1040(Id, HSCSI_MAILBOX_2, ((U32)dma_address) >>16); 
    	
    // Source address, bits 15-0 
    Write_ISP1040(Id, HSCSI_MAILBOX_3, (U32)dma_address & 0xffff); 

    // Length
    Write_ISP1040(Id, HSCSI_MAILBOX_4, hscsi_risc_code_length01); 
    	
    // Initialization CB address, bits 63-48
    Write_ISP1040(Id, HSCSI_MAILBOX_6, 0); 
    	
    // Initialization CB address, bits 47-32
    Write_ISP1040(Id, HSCSI_MAILBOX_7, 0); 
    	
	mailbox_status = HSCSI_Mailbox_Command(Id);
	if (mailbox_status != MB_STS_GOOD)
	{
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Load_RISC_RAM", 
			"Load RISC RAM failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return HSCSI_ERROR_LOAD_RISC_RAM;
	}
	
    return status;
	
} // HSCSI_Load_RISC_RAM
	
/*************************************************************************/
// HSCSI_Mailbox_Test
// Test the mailbox to assure that we can send and receive mailbox
// data from the ISP.
/*************************************************************************/
STATUS HSCSI_Mailbox_Test(PHSCSI_INSTANCE_DATA Id)
{
	STATUS				 status = NU_SUCCESS;
	U16					 mailbox_status;
	
 	HSCSI_TRACE_ENTRY(HSCSI_Mailbox_Test);
	
    status = HSCSI_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    
    Write_ISP1040(Id, HSCSI_MAILBOX_0, HSCSI_MBC_MB_REG_TEST);
    Write_ISP1040(Id, HSCSI_MAILBOX_1, 1);
    Write_ISP1040(Id, HSCSI_MAILBOX_2, 2);
    Write_ISP1040(Id, HSCSI_MAILBOX_3, 3);
    Write_ISP1040(Id, HSCSI_MAILBOX_4, 4);
    
    // Write zero to mailbox 5 so that, when we get a mailbox interrupt
    // we won't think we have a new IOCB.
    Write_ISP1040(Id, HSCSI_MAILBOX_5, 0);
	    	
	mailbox_status = HSCSI_Mailbox_Command(Id);

    if (mailbox_status != MB_STS_GOOD)
    {
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Mailbox_Test", 
			"test mailbox failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return HSCSI_ERROR_TEST_MAILBOX;
	}
	
	return status;
	
} // HSCSI_Mailbox_Test

/*************************************************************************/
// HSCSI_Init_Request_Queue
//
// Initialize the request queue (replaced by IFCB in ISP2xxx)
/*************************************************************************/
STATUS HSCSI_Init_Request_Queue(PHSCSI_INSTANCE_DATA Id)
{
	U32				pointer; // queue memory address
	U16				mailbox_status;
	STATUS			status = NU_SUCCESS;

	HSCSI_TRACE_ENTRY(HSCSI_Init_Request_Queue);
	
    status = HSCSI_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
	
	pointer = (U32)HSCSI_Get_DMA_Address(Id->HSCSI_p_IOCB_request_FIFO);
	Write_ISP1040(Id, HSCSI_MAILBOX_0, HSCSI_MBC_INIT_REQUEST);
	Write_ISP1040(Id, HSCSI_MAILBOX_1, Id->HSCSI_config.ISP_FIFO_request_size);
	Write_ISP1040(Id, HSCSI_MAILBOX_2, pointer>>16); // bits 31-16
	Write_ISP1040(Id, HSCSI_MAILBOX_3, pointer&0xffff); // bits 15-0
	Write_ISP1040(Id, HSCSI_MAILBOX_4, 0); // index=0
	
	mailbox_status = HSCSI_Mailbox_Command(Id);
	
	if (mailbox_status != MB_STS_GOOD)
	{
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL, "HSCSI_Init_Request_Queue",
			"init request queue failed", mailbox_status, (UNSIGNED)Id);
		return HSCSI_ERROR_INIT_REQUEST_QUEUE;
	}
	
	return status;
} // HSCSI_Init_Request_Queue

/*************************************************************************/
// HSCSI_Init_Response_Queue
//
// Initialize the response queue (replaced by IFCB in ISP2xxx)
/*************************************************************************/
STATUS HSCSI_Init_Response_Queue(PHSCSI_INSTANCE_DATA Id)
{
	U32				pointer; // queue memory address
	U16				mailbox_status;
	STATUS			status = NU_SUCCESS;

	HSCSI_TRACE_ENTRY(HSCSI_Init_Response_Queue);
	
    status = HSCSI_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
	
	pointer = (U32)HSCSI_Get_DMA_Address(Id->HSCSI_p_IOCB_response_FIFO);
	Write_ISP1040(Id, HSCSI_MAILBOX_0, HSCSI_MBC_INIT_RESPONSE);
	Write_ISP1040(Id, HSCSI_MAILBOX_1, Id->HSCSI_config.ISP_FIFO_response_size);
	Write_ISP1040(Id, HSCSI_MAILBOX_2, pointer>>16); // bits 31-16
	Write_ISP1040(Id, HSCSI_MAILBOX_3, pointer&0xffff); // bits 15-0
	Write_ISP1040(Id, HSCSI_MAILBOX_5, 0); // index=0, also sets MB=0
	
	mailbox_status = HSCSI_Mailbox_Command(Id);
	
	if (mailbox_status != MB_STS_GOOD)
	{
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL, "HSCSI_Init_Response_Queue",
			"init response queue failed", mailbox_status, (UNSIGNED)Id);
		return HSCSI_ERROR_INIT_RESPONSE_QUEUE;
	}
	
	return status;
} // HSCSI_Init_Response_Queue
						
/*************************************************************************/
// HSCSI_Bus_Reset
//
// Mailbox command to reset the SCSI bus
/*************************************************************************/
STATUS HSCSI_Bus_Reset(PHSCSI_INSTANCE_DATA Id)
{
	U16		mailbox_status;
	STATUS	status = NU_SUCCESS;

	HSCSI_TRACE_ENTRY(HSCSI_Bus_Reset);
	HSCSI_PRINT_STRING(TRACE_L2, "\n\rSCSI Bus Reset");
	
    status = HSCSI_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;

	Write_ISP1040(Id, HSCSI_MAILBOX_0, HSCSI_MBC_RESET);
	Write_ISP1040(Id, HSCSI_MAILBOX_1, 5); // 5 seconds delay
	Write_ISP1040(Id, HSCSI_MAILBOX_2, 0); // Port 0
	
	mailbox_status = HSCSI_Mailbox_Command(Id);
	
	if (mailbox_status != MB_STS_GOOD)
	{
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_WARNING, "HSCSI_Bus_Reset",
		"bus reset failed", mailbox_status, (UNSIGNED)Id);
		
		return HSCSI_ERROR_BUS_RESET;
	}
	
	return status;
}

/*************************************************************************/
// HSCSI_Execute_IOCB
//
// For debugging - send an IOCB directly, bypassing queues
/*************************************************************************/
STATUS HSCSI_Execute_IOCB(PHSCSI_INSTANCE_DATA Id)
{
	U16 mailbox_status;
	STATUS status = NU_SUCCESS;
	U32 pointer;

    status = HSCSI_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;

	pointer=(U32)HSCSI_Get_DMA_Address(Id->HSCSI_p_IOCB_request_FIFO);
	
	Write_ISP1040(Id, HSCSI_MAILBOX_0, 0x12);
	Write_ISP1040(Id, HSCSI_MAILBOX_1, Id->HSCSI_response_FIFO_index);
	Write_ISP1040(Id, HSCSI_MAILBOX_2, pointer>>16);
	Write_ISP1040(Id, HSCSI_MAILBOX_3, pointer&0xffff);
	
	mailbox_status = HSCSI_Mailbox_Command(Id);
	
	Read_ISP1040(Id, HSCSI_MAILBOX_1);
	Read_ISP1040(Id, HSCSI_MAILBOX_2);
	Read_ISP1040(Id, HSCSI_MAILBOX_3);
	
	return mailbox_status;
}

/*************************************************************************/
// HSCSI_Marker_IOCB
//
// Send Marker IOCB - needed after a SCSI bus reset
/*************************************************************************/
STATUS HSCSI_Marker_IOCB(PHSCSI_INSTANCE_DATA Id)
{
	IOCB_MARKER_ENTRY	*p_marker;
	STATUS				status = NU_SUCCESS;
	
	HSCSI_TRACE_ENTRY(HSCSI_Marker_IOCB);
	HSCSI_PRINT_STRING(TRACE_L2, "\n\rMarker IOCB Sent");
		
	p_marker = (IOCB_MARKER_ENTRY*)HSCSI_Request_FIFO_Get_Pointer(Id);
	p_marker->entry_type = HSCSI_IOCB_TYPE_MARKER;
	p_marker->entry_count = 1;
	p_marker->flags = 0;
	p_marker->modifier = 2; // ignore target ID/LUN
	
	HSCSI_Request_FIFO_Update_Index(Id); // alert ISP of new IOCB
	
	return status;
}

/*************************************************************************/
// HSCSI_Verify_RISC_Checksum
// Verify checksum after RISC code has been loaded.
/*************************************************************************/
STATUS HSCSI_Verify_RISC_Checksum(PHSCSI_INSTANCE_DATA Id)
{
	UNSIGNED	mailbox_status;
	STATUS		status = NU_SUCCESS;
	
 	HSCSI_TRACE_ENTRY(HSCSI_Verify_RISC_Checksum);
	
    status = HSCSI_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    
 	// Verify checksum command
    Write_ISP1040(Id, HSCSI_MAILBOX_0, HSCSI_MBC_VER_CHKSUM); 
    	
    // Destination address (RISC)
    Write_ISP1040(Id, HSCSI_MAILBOX_1, hscsi_risc_code_addr01); 
    	
	mailbox_status = HSCSI_Mailbox_Command(Id);

    if (mailbox_status != MB_STS_GOOD)
    {
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Verify_RISC_Checksum", 
			"RISC checksumfailed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return HSCSI_ERROR_TEST_RISC_RAM;
	}

    return status;
    
} // HSCSI_Verify_RISC_Checksum

/*************************************************************************/
// HSCSI_Set_SCSI
//
// SCSI bus settings (selection retry, timeout, ID, etc.)
/*************************************************************************/
STATUS HSCSI_Set_SCSI(PHSCSI_INSTANCE_DATA Id)
{
	STATUS		status = NU_SUCCESS;
	U16			mailbox_status;
	
	// Set Retry Count (0x32) Mailbox
	Write_ISP1040(Id, HSCSI_MAILBOX_0, HSCSI_MBC_SET_RETRY_COUNT);
	
	// port 0
	Write_ISP1040(Id, HSCSI_MAILBOX_1, 0); // no retries
	Write_ISP1040(Id, HSCSI_MAILBOX_2, 5); // 500ms
	
	// port 1
//	Write_ISP1040(Id, HSCSI_MAILBOX_6, 0);
//	Write_ISP1040(Id, HSCSI_MAILBOX_7, 5);
	
	mailbox_status = HSCSI_Mailbox_Command(Id);
	
	if (mailbox_status != MB_STS_GOOD)
	{
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL, "HSCSI_Set_SCSI",
			"Set Retry Count failed", mailbox_status, (UNSIGNED)Id);
		
		return HSCSI_ERROR_SET_SCSI;
	}
	
	return status;

} // HSCSI_Set_SCSI

/*************************************************************************/
// HSCSI_BIOS_RAM
//
// Place SCSI CDB directly into the 1040B RISC RAM - for debugging SXP
/*************************************************************************/
STATUS HSCSI_BIOS_RAM(PHSCSI_INSTANCE_DATA Id)
{
	U32			pointer;
	U16			mailbox_status, block_address, start_address;
	STATUS		status = NU_SUCCESS;
	int			i;
	U16			iocb[]={0,0, // DMA address
						0,0, // DMA byte count
						0,	 // flags, ID/LUN
						0,0,0, // 6 byte CDB
						0,0,0,0 // leftover bytes
						};
	HSCSI_TRACE_ENTRY(HSCSI_BIOS_RAM);
	
	pointer=(U32)HSCSI_Get_DMA_Address(Id->HSCSI_p_IOCB_request_FIFO);

	iocb[0]=BYTE_SWAP16(pointer&0xffff); // DMA address bits 0-15
	iocb[1]=BYTE_SWAP16(pointer>>16); // DMA address bits 16-31

	iocb[2]=0x0000;	// DMA byte count
	iocb[3]=0x0100; // DMA byte count
	
	iocb[4]=0x0300; // flags, ID/LUN
	iocb[5]=0x0008; // CDB bytes 1, 0 
	iocb[6]=0;		// CDB bytes 3, 2
	iocb[7]=0x0001; // CDB bytes 5, 4

    status = HSCSI_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;

	Write_ISP1040(Id, HSCSI_MAILBOX_0, 0x40); // Return BIOS block address
	
	mailbox_status = HSCSI_Mailbox_Command(Id);
	
	block_address = Read_ISP1040(Id, HSCSI_MAILBOX_1);

	start_address = block_address;
	
	for (i=0;i<12;i+=4) {
		
		Write_ISP1040(Id, HSCSI_MAILBOX_0, 0x41); // Write four RAM words
		Write_ISP1040(Id, HSCSI_MAILBOX_1, block_address);
		Write_ISP1040(Id, HSCSI_MAILBOX_2, iocb[i]);
		Write_ISP1040(Id, HSCSI_MAILBOX_3, iocb[i+1]);
		Write_ISP1040(Id, HSCSI_MAILBOX_4, iocb[i+2]);
		Write_ISP1040(Id, HSCSI_MAILBOX_5, iocb[i+3]);
		
		mailbox_status = HSCSI_Mailbox_Command(Id);
		block_address+=4;
	}
	
	Write_ISP1040(Id, HSCSI_MAILBOX_0, 0x42); // Execute BIOS IOCB
	Write_ISP1040(Id, HSCSI_MAILBOX_1, start_address);
	
	mailbox_status = HSCSI_Mailbox_Command(Id);

	return status;
} // HSCSI_BIOS_RAM 	
	
