/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpISP.c
// 
// Description:
// This file implements the ISP interface methods 
// 
// Update Log 
// 5/5/98 Jim Frandeen: Use C++ comment style
// 4/14/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 8/17/98 Michael G. Panas: Re-organize file, move some code to FcpDebug.c
// 8/25/98 Michael G. Panas: Updates to allow two QL2100 chips
// 10/1/98 Michael G. Panas: changes to support init and target in same instance
// 10/6/98 Michael G. Panas: changes to support interrupt driven mailbox commands
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/
#include "FcpCommon.h"
#include "FcpData.h"
#include "FcpError.h"
#include "FcpDebug.h"
#include "FcpISP.h"
#include "FcpMailbox.h"
#include "FcpMemory.h"
#include "FcpMessage.h"
#include "FcpRequestFIFO.h"
#include "FcpResponseFIFO.h"
#include "Nucleus.h"
#include "FcpString.h"
#include "Fcp.h"


/*************************************************************************/
// Forward References
/*************************************************************************/
STATUS	FCP_Execute_RISC_Firmware(PINSTANCE_DATA Id);
STATUS	FCP_ISP_Init(PINSTANCE_DATA Id);
STATUS	FCP_Load_2100_RISC_RAM(PINSTANCE_DATA Id);
STATUS	FCP_Load_2200_RISC_RAM(PINSTANCE_DATA Id);
STATUS	FCP_Mailbox_Test(PINSTANCE_DATA Id);
STATUS	FCP_Preload_Enable_LUNs(PINSTANCE_DATA Id);
STATUS	FCP_Send_2100_Initialization_Control_Block(PINSTANCE_DATA Id);
STATUS	FCP_Send_2200_Initialization_Control_Block(PINSTANCE_DATA Id);
STATUS	FCP_Verify_RISC_Checksum(PINSTANCE_DATA Id);

//*************************************************************************
// Globals
//*************************************************************************
// RISC code to load ISP is in 2100ef.c
extern unsigned short	risc_code01_2100[];
extern unsigned short   risc_code_length01_2100;
extern unsigned short	risc_code_version_2100;
extern unsigned short	risc_code_addr01_2100;

UNSIGNED	risc_code_swapped_2100 = 0;

// RISC code to load ISP is in 2200efm.c
extern unsigned short	risc_code01[];
extern unsigned short   risc_code_length01;
extern unsigned short	risc_code_version;
extern unsigned short	risc_code_addr01;

UNSIGNED	risc_code_swapped = 0;

/*************************************************************************/
// FCP_ISP_Create
// Create FCP_ISP object
// Start the ISP device.
// pp_memory points to a pointer to memory to be used.
// This object currently does not use any memory.
/*************************************************************************/
STATUS FCP_ISP_Create(PINSTANCE_DATA Id)
{
 	FCP_TRACE_ENTRY(FCP_ISP_Create);
 	
 	// Set up to print out values of ISR register as we read and write them
#if defined(FCP_DEBUG) && defined(_DEBUG)
 	Id->FCP_if_print_ISR = TRACE_L8;
#else
 	Id->FCP_if_print_ISR = 0;		// no TRACE
#endif

	return NU_SUCCESS;
	
} // FCP_ISP_Create

/*************************************************************************/
// FCP_ISP_Destroy
// Destroy FCP_ISP object.
/*************************************************************************/
void FCP_ISP_Destroy()
{

} // FCP_ISP_Destroy

/*************************************************************************/
// FCP_Execute_RISC_Firmware
// Verify RISC RAM command has completed.
/*************************************************************************/
STATUS FCP_Execute_RISC_Firmware(PINSTANCE_DATA Id)
{
	UNSIGNED	mailbox_status;
	STATUS		status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Execute_RISC_Firmware);
	
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    
 	// Execute RISC firmware command
    Write_ISP(Id, ISP_MAILBOX_0, MBC_EXE_FW); 
    	
    // Destination address (RISC)
    Write_ISP(Id, ISP_MAILBOX_1, risc_code_addr01); 
    	
	mailbox_status = FCP_Mailbox_Command(Id);

    if (mailbox_status != MB_STS_GOOD)
    {
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Execute_RISC_Firmware", 
			"Execute firmware failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return FCP_ERROR_EXECUTE_FIRMWARE;
	}

    return status;
    
} // FCP_Execute_RISC_Firmware


/*************************************************************************/
// FCP_ISP_Init
// Called by FCP_Event_Task to initialize the ISP.
/*************************************************************************/
STATUS FCP_ISP_Init(PINSTANCE_DATA Id)
{
	STATUS				 status = NU_SUCCESS;
	UNSIGNED			 timeout;
	U16					 mailbox_0;
	U16					 target_up = 0;
	
 	FCP_TRACE_ENTRY(FCP_ISP_Init);
	
	// Issue soft reset
    Write_ISP(Id, ISP_CONTROL_STATUS, 1); 
	
	// During initialization, we run with interrupts off, in polling mode.
	Write_ISP(Id, ISP_PCI_INT_CONTROL, 0);	
		
    // Issue hard reset on RISC processor
	Write_ISP(Id, ISP_HCCR, HCTLRESETRISC);
	
	// Release RISC from Reset
	Write_ISP(Id, ISP_HCCR, HCTLRLSRISC);
	
	// Disable BIOS access  (Needed by Targets only?)
	Write_ISP(Id, ISP_HCCR, HCTLDISABLEBIOS);
	
	// Wait for RISC to be Ready
	// timeout = FCP_TIMEOUT; 
	timeout = 10;
	mailbox_0 = Read_ISP(Id, ISP_MAILBOX_0);
	while(mailbox_0 != 0)
	{
		NU_Sleep(1);
		mailbox_0 = Read_ISP(Id, ISP_MAILBOX_0);
		FCP_PRINT_NUMBER(TRACE_L8, "\n\rmailbox_0 = ", mailbox_0);
		if (timeout-- == 0)
		{
			FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
				"FCP_ISP_Init", 
				"Waiting for RISC to be ready",
				mailbox_0,
				(UNSIGNED)Id);
			return FCP_ERROR_RISC_NOT_READY;
		}		
	}
    
	// Enable interrupts from the RISC processor.
	// Bit15 is a global interrupt enable and disable bit.
	// Bit5 is Enable FPM Interrupt on PCI
	// Bit4 is Enable FB Interrupt on PCI
	// Bit3 is Enable RISC Interrupt on PCI
	// Bit2 is Enable Command DMA Channel Interrupt on PCI
	// Bit1 is Enable Receive DMA Channel Interrupt on PCI
	// Bit0 is Enable Transmit DMA Channel Interrupt on PCI
	Write_ISP(Id, ISP_PCI_INT_CONTROL, 0x8008);	
		
    status = FCP_Mailbox_Test(Id);
    if (status != NU_SUCCESS)
    	return status;
    	
#if 0
	// DEBUG
	FCP_Verify_RISC_RAM_2200(Id, 0x1000);
	FCP_Verify_RISC_RAM_2200(Id, 0x8000);
	FCP_Verify_RISC_RAM_2200(Id, 0xA000);
#endif

	// check for 2100 or 2200
	if (Id->ISP_Type == 0x00002200)
	{
	    status = FCP_Load_2200_RISC_RAM(Id);
	    if (status != NU_SUCCESS)
	    	return status;
    }
    else
    {
	    status = FCP_Load_2100_RISC_RAM(Id);
	    if (status != NU_SUCCESS)
	    	return status;
    }
	
#if 0
	// DEBUG mgp
	status = FCP_Dump_RISC_RAM(Id, Buffer, risc_code_length01, 0x1000);
    if (status != NU_SUCCESS)
    	return status;
    	
    // compare data read to data sent
    {
    	U32		loop;
    	unsigned short	*p1, *p2;
    	
    	p1 = (unsigned short *)Buffer;
    	p2 = &risc_code01[0];
    	for (loop = 0; loop < risc_code_length01; loop++) {
     		if (*p1 != *p2) {
    			FCP_PRINT_HEX16(TRACE_L8, "\n\rMismatch - Addr = ", loop);
    			FCP_PRINT_HEX16(TRACE_L8, "\n\rp1 = ", *p1);
    			FCP_PRINT_HEX16(TRACE_L8, " p2 = ", *p2);
    			break;
    		}
    		p1++;
    		p2++;
    	}
    }
#endif

	status = FCP_Verify_RISC_Checksum(Id);
    if (status != NU_SUCCESS)
    	return status;
	
	status = FCP_Execute_RISC_Firmware(Id);
    if (status != NU_SUCCESS)
    	return status;
	
	// check for 2100 or 2200
	if (Id->ISP_Type == 0x00002200)
	{
		status = FCP_Send_2200_Initialization_Control_Block(Id);
	    if (status != NU_SUCCESS)
	    	return status;
	}
	else
	{
		status = FCP_Send_2100_Initialization_Control_Block(Id);
	    if (status != NU_SUCCESS)
	    	return status;
	}
 
#if 0
	// startup the FC loop - if needed
	if (Id->FCP_config.options & IFCB_DISABLE_INITIAL_LIP)
	{
		status = FCP_Initiate_LIP(Id);
	    if (status != NU_SUCCESS)
	    	return status;
    }
#endif

#if defined(_NAC)

	TRACEF(TRACE_L2, ("\n\rWaiting for loop %d to come up", Id->FCP_instance));
	
	// Wait for loop to come up
	status = NU_Obtain_Semaphore(&Id->FCP_Loop_Sema, NU_SUSPEND);
	
	TRACEF(TRACE_L2, ("\n\rLoop %d Up", Id->FCP_instance));
	
	if (Id->FCP_config.enable_target_mode)
	{
		target_up = 1;
	}

#else
	if (Id->FCP_config.enable_target_mode)
	{
		FCP_PRINT_STRING(TRACE_L2, "\n\rWaiting for Target loop to come up");
	}
	else
	{
		FCP_PRINT_STRING(TRACE_L2, "\n\rWaiting for Init loop to come up");
	}
	
	// Wait for loop to come up
	status = NU_Obtain_Semaphore(&Id->FCP_Loop_Sema, NU_SUSPEND);

	if (Id->FCP_config.enable_target_mode)
	{
		target_up = 1;
		FCP_PRINT_STRING(TRACE_L2, "\n\rTarget Loop Up");
	}
	else
	{
		FCP_PRINT_STRING(TRACE_L2, "\n\rInit Loop Up");
	}
#endif

#if 0
	// Enable LUNs for the 2100/2200 target
	if (Id->FCP_config.enable_target_mode && target_up)
	{
		// Enable all LUNs if target mode.
		status = FCP_Enable_LUNs(Id);
		
	}
#else
	// Enable LUNs for the 2100 target only
	if ((Id->ISP_Type != 0x00002200) && 
					Id->FCP_config.enable_target_mode && target_up)
	{
		// Enable all LUNs if target mode.
		status = FCP_Enable_LUNs(Id);
		
	}
#endif

	if (status == NU_SUCCESS)
	{
		// now we can accept commands
		Id->FCP_state = FCP_STATE_ACTIVE;
		
		// Send the Enable Reply if we are the Initiator
		if (Id->FCP_config.enable_initiator_mode)
	 		FCP_Send_Enable_Reply(Id);
	}
	
	return status;
	    
} // FCP_ISP_Init

/*************************************************************************/
// FCP_Load_2100_RISC_RAM
// Load RISC RAM from memory using Load RAM command.
/*************************************************************************/
STATUS FCP_Load_2100_RISC_RAM(PINSTANCE_DATA Id)
{
	U16			 mailbox_status;
	STATUS		 status = NU_SUCCESS;
	U32			 dma_address;
	
 	FCP_TRACE_ENTRY(FCP_Load_2100_RISC_RAM);
 	
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;

#if defined(FCP_DEBUG) && defined(_DEBUG)
	// Prepare RISC RAM image to be loaded from memory
	FCP_PRINT_STRING(TRACE_L8, "\n\rLoading RISC RAM from memory using Load RISC RAM command.");
	FCP_PRINT_HEX16(TRACE_L8, "\n\rrisc_code_length01_2100 = ", risc_code_length01_2100);
    FCP_PRINT_HEX16(TRACE_L8, "\n\r1st word of risc_code01_2100 = ", risc_code01_2100[0]); 
    FCP_PRINT_HEX16(TRACE_L8, "\n\rlast word of risc_code01_2100 = ", risc_code01[risc_code_length01_2100 - 1]); 
	FCP_PRINT_HEX(TRACE_L8, "\n\rAddress of risc_code01_2100 = ", (UNSIGNED)&risc_code01_2100[0]);

	// Read some words of RISC RAM to see what is there
	// before we load.
	FCP_PRINT_STRING(TRACE_L8, "\n\rRAM before load:");
	if (TraceLevel[TRACE_INDEX] >= TRACE_L8) {
		FCP_Print_RAM_Word(Id, 0); 
		FCP_Print_RAM_Word(Id, 1); 
		FCP_Print_RAM_Word(Id, risc_code_length01_2100 - 2); 
		FCP_Print_RAM_Word(Id, risc_code_length01_2100 - 1);
	}
#endif
	
	// note: this code is always needed
	if (risc_code_swapped_2100 == 0)
	{
		// Jim June 15
		// We need to byte swap the data, but it doesn't all end up 
		// in RISC memory byte-swapped when we send it to the ISP.
		// mgp 07/20/98 this failure was because the cache was not flushed.
		UNSIGNED	 index;
		U16			 *p;		// access is through KSEG1
		
		p = (U16 *)((UNSIGNED)&risc_code01_2100[0] | KSEG1);
		
		for (index = 0; index < risc_code_length01_2100; index++)
		{
			*p = BYTE_SWAP16(*p);
			p++;
		}
		// Just to make sure we really swap them all...
		FCP_PRINT_HEX16(TRACE_L8, "\n\rLast index = ", index - 1);
		FCP_PRINT_HEX16(TRACE_L8, "\n\rLast word swaped = ", risc_code01_2100[index - 1]);
		risc_code_swapped_2100 = 1;	// only need to do this once
	}
	 
	dma_address = (U32)FCP_Get_DMA_Address((void *)&risc_code01_2100[0]);

 	// Load RISC RAM command
    Write_ISP(Id, ISP_MAILBOX_0, MBC_LOAD_RISC_RAM);		// 64 bit load
    	
    // Destination address (RISC)
    // Write_ISP(ISP_MAILBOX_1, risc_code_addr01);
    Write_ISP(Id, ISP_MAILBOX_1, risc_code_addr01_2100);
    
    // Source address, bits 31-16
    // Mask off PCI bits
    Write_ISP(Id, ISP_MAILBOX_2, ((U32)dma_address) >>16); 
    	
    // Source address, bits 15-0 
    Write_ISP(Id, ISP_MAILBOX_3, (U32)dma_address & 0xffff); 

    // Length
    Write_ISP(Id, ISP_MAILBOX_4, risc_code_length01_2100); 
    	
    // Initialization CB address, bits 63-48
    Write_ISP(Id, ISP_MAILBOX_6, 0); 
    	
    // Initialization CB address, bits 47-32
    Write_ISP(Id, ISP_MAILBOX_7, 0); 
    	
	mailbox_status = FCP_Mailbox_Command(Id);
	if (mailbox_status != MB_STS_GOOD)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Load_2100_RISC_RAM", 
			"Load RISC RAM failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return FCP_ERROR_LOAD_RISC_RAM;
	}
	
    return status;
	
} // FCP_Load_2100_RISC_RAM

	

/*************************************************************************/
// FCP_Load_2200_RISC_RAM
// Load RISC RAM from memory using Load RAM command.
/*************************************************************************/
STATUS FCP_Load_2200_RISC_RAM(PINSTANCE_DATA Id)
{
	U16			 mailbox_status;
	STATUS		 status = NU_SUCCESS;
	U32			 dma_address;
	
 	FCP_TRACE_ENTRY(FCP_Load_2200_RISC_RAM);
 	
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;

#if defined(FCP_DEBUG) && defined(_DEBUG)
	// Prepare RISC RAM image to be loaded from memory
	FCP_PRINT_STRING(TRACE_L8, "\n\rLoading RISC RAM from memory using Load RISC RAM command.");
	FCP_PRINT_HEX16(TRACE_L8, "\n\rrisc_code_length01 = ", risc_code_length01);
    FCP_PRINT_HEX16(TRACE_L8, "\n\r1st word of risc_code01 = ", risc_code01[0]); 
    FCP_PRINT_HEX16(TRACE_L8, "\n\rlast word of risc_code01 = ", risc_code01[risc_code_length01 - 1]); 
	FCP_PRINT_HEX(TRACE_L8, "\n\rAddress of risc_code01 = ", (UNSIGNED)&risc_code01[0]);

	// Read some words of RISC RAM to see what is there
	// before we load.
	FCP_PRINT_STRING(TRACE_L8, "\n\rRAM before load:");
	if (TraceLevel[TRACE_INDEX] >= TRACE_L8) {
		FCP_Print_RAM_Word(Id, 0); 
		FCP_Print_RAM_Word(Id, 1); 
		FCP_Print_RAM_Word(Id, risc_code_length01 - 2); 
		FCP_Print_RAM_Word(Id, risc_code_length01 - 1);
	}
#endif
	
	// note: this code is always needed
	if (risc_code_swapped == 0)
	{
		// Jim June 15
		// We need to byte swap the data, but it doesn't all end up 
		// in RISC memory byte-swapped when we send it to the ISP.
		// mgp 07/20/98 this failure was because the cache was not flushed.
		UNSIGNED	 index;
		U16			 *p;		// access is through KSEG1
		
		p = (U16 *)((UNSIGNED)&risc_code01[0] | KSEG1);
		
		for (index = 0; index < risc_code_length01; index++)
		{
			*p = BYTE_SWAP16(*p);
			p++;
		}
		// Just to make sure we really swap them all...
		FCP_PRINT_HEX16(TRACE_L8, "\n\rLast index = ", index - 1);
		FCP_PRINT_HEX16(TRACE_L8, "\n\rLast word swaped = ", risc_code01[index - 1]);
		risc_code_swapped = 1;	// only need to do this once
	}
	 
	dma_address = (U32)FCP_Get_DMA_Address((void *)&risc_code01[0]);
	
#define	ERRATA_2200_1

#if defined(ERRATA_2200_1)
// must load RISC RAM in 128 byte chunks
	{
		int chunks = ((risc_code_length01 * 2) / 128 );
		int last = ((risc_code_length01 * 2) % 128 );
		int risc_addr = risc_code_addr01;
		
		FCP_PRINT_HEX16(TRACE_L2, "\n\rrisc_code_length01 = ", risc_code_length01);
		FCP_PRINT_HEX(TRACE_L2, "\n\rchunks= ", chunks);
		FCP_PRINT_HEX(TRACE_L2, "\n\rlast= ", last);
		FCP_PRINT_STRING(TRACE_L2, "\n\r");
		
		for (; chunks; chunks--)
		{
		 	// Load RISC RAM command
		    Write_ISP(Id, ISP_MAILBOX_0, MBC_LOAD_RISC_RAM);		// 64 bit load
		    	
		    // Destination address (RISC)
		    Write_ISP(Id, ISP_MAILBOX_1, risc_addr);
		    
		    // Source address, bits 31-16
		    Write_ISP(Id, ISP_MAILBOX_2, ((U32)dma_address) >>16); 
		    	
		    // Source address, bits 15-0 
		    Write_ISP(Id, ISP_MAILBOX_3, (U32)dma_address & 0xffff); 
		
		    // Length
		    Write_ISP(Id, ISP_MAILBOX_4, 128/2); 
		    	
		    // Initialization CB address, bits 63-48
		    Write_ISP(Id, ISP_MAILBOX_6, 0); 
		    	
		    // Initialization CB address, bits 47-32
		    Write_ISP(Id, ISP_MAILBOX_7, 0); 
		    	
			mailbox_status = FCP_Mailbox_Command(Id);
			if (mailbox_status != MB_STS_GOOD)
			{
				FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
					"FCP_Load_2100_RISC_RAM", 
					"Load RISC RAM failed",
					mailbox_status,
					(UNSIGNED)Id);
					
				return FCP_ERROR_LOAD_RISC_RAM;
			}
			
			// bump memory address and risc address
			dma_address += 128;	// this is a byte pointer
			risc_addr += 64;	// this is a word pointer
			FCP_PRINT_HEX16(TRACE_L8, " ", risc_addr);
		}

		// load the last chunk of code if any
		if (last)
		{
		 	// Load RISC RAM command
		    Write_ISP(Id, ISP_MAILBOX_0, MBC_LOAD_RISC_RAM);		// 64 bit load
		    	
		    // Destination address (RISC)
		    Write_ISP(Id, ISP_MAILBOX_1, risc_addr);
		    
		    // Source address, bits 31-16
		    Write_ISP(Id, ISP_MAILBOX_2, ((U32)dma_address) >>16); 
		    	
		    // Source address, bits 15-0 
		    Write_ISP(Id, ISP_MAILBOX_3, (U32)dma_address & 0xffff); 
		
		    // Length
		    Write_ISP(Id, ISP_MAILBOX_4, last/2); 
		    	
		    // Initialization CB address, bits 63-48
		    Write_ISP(Id, ISP_MAILBOX_6, 0); 
		    	
		    // Initialization CB address, bits 47-32
		    Write_ISP(Id, ISP_MAILBOX_7, 0); 
		    	
			mailbox_status = FCP_Mailbox_Command(Id);
			if (mailbox_status != MB_STS_GOOD)
			{
				FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
					"FCP_Load_2100_RISC_RAM", 
					"Load RISC RAM failed",
					mailbox_status,
					(UNSIGNED)Id);
					
				return FCP_ERROR_LOAD_RISC_RAM;
			}
		}
		
	}

#else
	
 	// Load RISC RAM command
//    Write_ISP(Id, ISP_MAILBOX_0, MBC_LOAD_RAM); 			// 32 bit load
    Write_ISP(Id, ISP_MAILBOX_0, MBC_LOAD_RISC_RAM);		// 64 bit load
    	
    // Destination address (RISC)
    // Write_ISP(ISP_MAILBOX_1, risc_code_addr01);
    Write_ISP(Id, ISP_MAILBOX_1, risc_code_addr01);
    
    // Source address, bits 31-16
    // Mask off PCI bits
    Write_ISP(Id, ISP_MAILBOX_2, ((U32)dma_address) >>16); 
    	
    // Source address, bits 15-0 
    Write_ISP(Id, ISP_MAILBOX_3, (U32)dma_address & 0xffff); 

    // Length
    Write_ISP(Id, ISP_MAILBOX_4, risc_code_length01); 
    	
    // Initialization CB address, bits 63-48
    Write_ISP(Id, ISP_MAILBOX_6, 0); 
    	
    // Initialization CB address, bits 47-32
    Write_ISP(Id, ISP_MAILBOX_7, 0); 
    	
	mailbox_status = FCP_Mailbox_Command(Id);
	if (mailbox_status != MB_STS_GOOD)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Load_2100_RISC_RAM", 
			"Load RISC RAM failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return FCP_ERROR_LOAD_RISC_RAM;
	}
#endif
	
    return status;
	
} // FCP_Load_2200_RISC_RAM
	
/*************************************************************************/
// FCP_Mailbox_Test
// Test the mailbox to assure that we can send and receive mailbox
// data from the ISP.
/*************************************************************************/
STATUS FCP_Mailbox_Test(PINSTANCE_DATA Id)
{
	STATUS				 status = NU_SUCCESS;
	U16					 mailbox_status;
	
 	FCP_TRACE_ENTRY(FCP_Mailbox_Test);
	
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    
    Write_ISP(Id, ISP_MAILBOX_0, MBC_MB_REG_TEST);
    Write_ISP(Id, ISP_MAILBOX_1, 1);
    Write_ISP(Id, ISP_MAILBOX_2, 2);
    Write_ISP(Id, ISP_MAILBOX_3, 3);
    Write_ISP(Id, ISP_MAILBOX_4, 4);
    
    // Write zero to mailbox 5 so that, when we get a mailbox interrupt
    // we won't think we have a new IOCB.
    Write_ISP(Id, ISP_MAILBOX_5, 0);
    Write_ISP(Id, ISP_MAILBOX_6, 6);
    Write_ISP(Id, ISP_MAILBOX_7, 7);
    	
	mailbox_status = FCP_Mailbox_Command(Id);

    if (mailbox_status != MB_STS_GOOD)
    {
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Mailbox_Test", 
			"test mailbox failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return FCP_ERROR_TEST_MAILBOX;
	}
	
	return status;
	
} // FCP_Mailbox_Test

/*************************************************************************/
// FCP_Preload_Enable_LUNs
// Rewritten to handle 16 bit LUNs sends only one ENABLE_LUN IOCB
/*************************************************************************/
STATUS FCP_Preload_Enable_LUNs(PINSTANCE_DATA Id)
{
 	IOCB_ENABLE_LUN		*p_command;
 	STATUS				 status = NU_SUCCESS;
    
 	FCP_TRACE_ENTRY(FCP_Preload_Enable_LUNs);
 	
    // Get pointer to next IOCB in request FIFO.
	p_command = (IOCB_ENABLE_LUN*)FCP_Request_FIFO_Get_Pointer(Id);

    // Build enable LUN queue entry 
    p_command->entry_type = IOCB_TYPE_ENABLE_LUN;	// entry type 
    p_command->entry_count= 0x01;        			//entry count (always 1) 
    p_command->command_count = COMMAND_COUNT *
    									Id->FCP_config.num_LUNs;
    p_command->immediate_notify_count = IMMEDIATE_NOTIFY_COUNT *
    									Id->FCP_config.num_LUNs;
    p_command->status = 0;
    
    // Send Command IOCB request to the ISP.
	// Update Request_FIFO_Index.  This lets the ISP know that a new
	// request is ready to execute.  
	//FCP_Request_FIFO_Update_Index(Id);
	
	return status;
	
} // FCP_Preload_Enable_LUNs
    	
/*************************************************************************/
// FCP_Send_2100_Initialization_Control_Block
// Swap bytes to get Little Endian
/*************************************************************************/
STATUS FCP_Send_2100_Initialization_Control_Block(PINSTANCE_DATA Id)
{
	INITIALIZE_FIRMWARE_CONTROL_BLOCK 	ifcb;
	UNSIGNED							mailbox_status;
	U32									dma_address;
	STATUS								status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Send_2100_Initialization_Control_Block);

	bzero((char *)&ifcb, sizeof(ifcb));
	
    // Build Initialization control Block 
	// Set up options field from config parameters
	ifcb.options = Id->FCP_config.options;
	
	// When IFCB_FAST_STATUS_POSTING, enables fast posting of status IOCBs
	// that have good completion status.
	// See FCP_Mailbox_HISR.
	if (Id->FCP_config.enable_fast_posting)
 	    ifcb.options |= IFCB_FAST_STATUS_POSTING;
 
    ifcb.hard_address = 0;
	if (Id->FCP_config.hard_address)
	{
		ifcb.options |= IFCB_HARD_ADDRESS_ENABLE;
		ifcb.hard_address = BYTE_SWAP16(Id->FCP_config.hard_address);
		FCP_PRINT_HEX16(TRACE_L3, "\n\rEnabling hard_address ", Id->FCP_config.hard_address);
	}
		
	if (Id->FCP_config.enable_target_mode)
	{
		FCP_PRINT_STRING(TRACE_L3, "\n\rEnabling target mode");
		ifcb.options |= IFCB_TARGET_MODE_ENABLE;
	}
    
	if (!Id->FCP_config.enable_initiator_mode)
	{
		FCP_PRINT_STRING(TRACE_L3, "\n\rDisabling initiator mode");
		ifcb.options |= IFCB_INITIATOR_MODE_DISABLE;
	}
	else
	{
		FCP_PRINT_STRING(TRACE_L3, "\n\rEnabling initiator mode");
	}
    
	if (Id->FCP_config.enable_fairness)
		ifcb.options |= IFCB_FAIRNESS;
    
    ifcb.version = 1;
    FCP_PRINT_HEX16(TRACE_L3, "\n\rrisc_code_version = ", risc_code_version_2100);
    
    ifcb.frame_size = BYTE_SWAP16(Id->FCP_config.max_frame_size);
    FCP_PRINT_HEX(TRACE_L3, "\n\rMaximum frame size = ", Id->FCP_config.max_frame_size);
    
    // Define the maximum number of buffers allocated to any one port.
    ifcb.num_buffers_max = BYTE_SWAP16(32);
    
    // Set the maximum number of commands executing on any one port.
    ifcb.throttle = BYTE_SWAP16(16);
    
    ifcb.retry_count = 8;
    
    // Specify delay time before retry in 100 millisecond increments.
    ifcb.retry_delay = 1;
    
    // Set up node name that the ISP firmware will use during FC-AL
    // loop initialization.  This is a worldwide unique name
    ifcb.node_name[0] = Id->FCP_config.node_name[0];
    ifcb.node_name[1] = Id->FCP_config.node_name[1];
    ifcb.node_name[2] = Id->FCP_config.node_name[2];
    ifcb.node_name[3] = Id->FCP_config.node_name[3];
    ifcb.node_name[4] = Id->FCP_config.node_name[4];
    ifcb.node_name[5] = Id->FCP_config.node_name[5];
    ifcb.node_name[6] = Id->FCP_config.node_name[6];
    ifcb.node_name[7] = Id->FCP_config.node_name[7];
    
    ifcb.request_queue_index= 0;
    ifcb.response_queue_index = 0;
    
    ifcb.request_queue_size = BYTE_SWAP16(Id->FCP_config.ISP_FIFO_request_size);
    FCP_PRINT_NUMBER(TRACE_L3, "\n\rrequest_queue_size = ", Id->FCP_config.ISP_FIFO_request_size);
    
    ifcb.response_queue_size = BYTE_SWAP16(Id->FCP_config.ISP_FIFO_response_size);
    FCP_PRINT_NUMBER(TRACE_L3, "\n\rresponse_queue_size = ", Id->FCP_config.ISP_FIFO_response_size);
    
    // Set address of request FIFO
    ifcb.request_queue_address0 = (U32)FCP_Get_DMA_Address(Id->FCP_p_IOCB_request_FIFO);
    ifcb.request_queue_address0 = BYTE_SWAP32((U32)ifcb.request_queue_address0);
    ifcb.request_queue_address1 = 0L;
    
    // Set address of response FIFO
    ifcb.response_queue_address0 = (U32)FCP_Get_DMA_Address(Id->FCP_p_IOCB_response_FIFO);
    ifcb.response_queue_address0 = BYTE_SWAP32((U32)ifcb.response_queue_address0);
    ifcb.response_queue_address1 = 0L;
	
	ifcb.options = BYTE_SWAP16(ifcb.options);		// fix the options
    FCP_PRINT_HEX16(TRACE_L3, "\n\roptions = ", ifcb.options);
	
	// DEBUG - copy the ifcb to phys memory
	{
//		Mem_Copy((char*)((UNSIGNED)&ifcb | KSEG1),	// to
//			(char*)&ifcb,		// from
//			sizeof(ifcb));		// number of bytes
		bcopy(
			(char*)&ifcb,		// from
			(char*)((UNSIGNED)&ifcb | KSEG1),	// to
			sizeof(ifcb));		// number of bytes

	}
#if 0
	// Pre-Load Enable LUNs IOCB for the target only
	if (Id->FCP_config.enable_target_mode)
	{
		// Enable all LUNs if target mode.
		status = FCP_Preload_Enable_LUNs(Id);
		
	}
#endif

	dma_address = (U32)FCP_Get_DMA_Address((void *)&ifcb);
		
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    
    // Initialize firmware command
    Write_ISP(Id, ISP_MAILBOX_0, MBC_INIT_FW); 
    
    // Initialization CB address, bits 31-16
    // Mask off PCI bits
    Write_ISP(Id, ISP_MAILBOX_2, ((U32)dma_address) >>16); 
    
    // Initialization CB address, bits 15-0
    Write_ISP(Id, ISP_MAILBOX_3, (U32)dma_address & 0xffff); 

#if 0    	
	if (Id->FCP_config.enable_target_mode)
	{
	    // Request queue in pointer
    	Write_ISP(Id, ISP_MAILBOX_4, 1);
    }
    else
#endif
	    // Request queue in pointer
    	Write_ISP(Id, ISP_MAILBOX_4, 0);
    	
    // Request queue out pointer
    Write_ISP(Id, ISP_MAILBOX_5, 0); 
    	
    // Initialization Control Block address, bits 63-48
    Write_ISP(Id, ISP_MAILBOX_6, 0); 
    	
    // Initialization Control Block address, bits 47-32
    Write_ISP(Id, ISP_MAILBOX_7, 0); 
    		
	mailbox_status = FCP_Mailbox_Command(Id);

    if (mailbox_status != MB_STS_GOOD)
    {
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Send_2100_Initialization_Control_Block", 
			"Initialize firmware failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return FCP_ERROR_INIT_CONTROL_BLOCK;
	}
	
    return status;
    
} // FCP_Send_2100_Initialization_Control_Block
	

/*************************************************************************/
// FCP_Send_2200_Initialization_Control_Block
// Use the Extended Control Block so we don't need to send an
// ENABLE_LUN for each target ID.
/*************************************************************************/
STATUS FCP_Send_2200_Initialization_Control_Block(PINSTANCE_DATA Id)
{
	IFWCB_ECB						 	ifcb;
//	IFWCB_2200						 	ifcb;
	UNSIGNED							mailbox_status;
	U32									dma_address;
	STATUS								status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Send_2200_Initialization_Control_Block);

	bzero((char *)&ifcb, sizeof(ifcb));
	
    // Build Initialization control Block 
	// Set up options field from config parameters
	ifcb.cb.options = Id->FCP_config.options;
	
	// When IFCB_FAST_STATUS_POSTING, enables fast posting of status IOCBs
	// that have good completion status.
	// See FCP_Mailbox_HISR.
	if (Id->FCP_config.enable_fast_posting)
 	    ifcb.cb.options |= IFCB_FAST_STATUS_POSTING;
 
    ifcb.cb.hard_address = 0;
	if (Id->FCP_config.hard_address)
	{
		ifcb.cb.options |= IFCB_HARD_ADDRESS_ENABLE;
		ifcb.cb.hard_address = BYTE_SWAP16(Id->FCP_config.hard_address);
		FCP_PRINT_HEX16(TRACE_L3, "\n\rEnabling hard_address ", Id->FCP_config.hard_address);
	}
		
	if (Id->FCP_config.enable_target_mode)
	{
		FCP_PRINT_STRING(TRACE_L3, "\n\rEnabling target mode");
		ifcb.cb.options |= IFCB_TARGET_MODE_ENABLE;
	}
    
	if (!Id->FCP_config.enable_initiator_mode)
	{
		FCP_PRINT_STRING(TRACE_L3, "\n\rDisabling initiator mode");
		ifcb.cb.options |= IFCB_INITIATOR_MODE_DISABLE;
	}
	else
	{
		FCP_PRINT_STRING(TRACE_L3, "\n\rEnabling initiator mode");
	}
    
	if (Id->FCP_config.enable_fairness)
		ifcb.cb.options |= IFCB_FAIRNESS;
    
    ifcb.cb.version = 1;
    FCP_PRINT_HEX16(TRACE_L3, "\n\rrisc_code_version = ", risc_code_version);
    
    ifcb.cb.frame_size = BYTE_SWAP16(Id->FCP_config.max_frame_size);
    FCP_PRINT_HEX(TRACE_L3, "\n\rMaximum frame size = ", Id->FCP_config.max_frame_size);
    
    // Define the maximum number of buffers allocated to any one port.
    ifcb.cb.num_buffers_max = BYTE_SWAP16(32);
    
    // Set the maximum number of commands executing on any one port.
    ifcb.cb.throttle = BYTE_SWAP16(16);
    
    ifcb.cb.retry_count = 8;
    
    // Specify delay time before retry in 100 millisecond increments.
    ifcb.cb.retry_delay = 1;
    
    // Set up node name that the ISP firmware will use during FC-AL
    // loop initialization.  This is a worldwide unique name
    ifcb.cb.node_name[0] = Id->FCP_config.node_name[0];
    ifcb.cb.node_name[1] = Id->FCP_config.node_name[1];
    ifcb.cb.node_name[2] = Id->FCP_config.node_name[2];
    ifcb.cb.node_name[3] = Id->FCP_config.node_name[3];
    ifcb.cb.node_name[4] = Id->FCP_config.node_name[4];
    ifcb.cb.node_name[5] = Id->FCP_config.node_name[5];
    ifcb.cb.node_name[6] = Id->FCP_config.node_name[6];
    ifcb.cb.node_name[7] = Id->FCP_config.node_name[7];
    
    ifcb.cb.request_queue_index= 0;
    ifcb.cb.response_queue_index = 0;
    
    ifcb.cb.request_queue_size = BYTE_SWAP16(Id->FCP_config.ISP_FIFO_request_size);
    FCP_PRINT_NUMBER(TRACE_L3, "\n\rrequest_queue_size = ", Id->FCP_config.ISP_FIFO_request_size);
    
    ifcb.cb.response_queue_size = BYTE_SWAP16(Id->FCP_config.ISP_FIFO_response_size);
    FCP_PRINT_NUMBER(TRACE_L3, "\n\rresponse_queue_size = ", Id->FCP_config.ISP_FIFO_response_size);
    
    // Set address of request FIFO
    ifcb.cb.request_queue_address0 = (U32)FCP_Get_DMA_Address(Id->FCP_p_IOCB_request_FIFO);
    ifcb.cb.request_queue_address0 = BYTE_SWAP32((U32)ifcb.cb.request_queue_address0);
    ifcb.cb.request_queue_address1 = 0L;
    
    // Set address of response FIFO
    ifcb.cb.response_queue_address0 = (U32)FCP_Get_DMA_Address(Id->FCP_p_IOCB_response_FIFO);
    ifcb.cb.response_queue_address0 = BYTE_SWAP32((U32)ifcb.cb.response_queue_address0);
    ifcb.cb.response_queue_address1 = 0L;
	
	ifcb.cb.options |= IFCB_ECB;
	
	ifcb.cb.options = BYTE_SWAP16(ifcb.cb.options);		// fix the options
    FCP_PRINT_HEX16(TRACE_L3, "\n\roptions = ", ifcb.cb.options);
	
	// setup the Extended Control Block
	// enable all LUNs
	ifcb.ecb.LUN_enables = BYTE_SWAP16(1);

	ifcb.ecb.command_resource_count = 0xff;
	ifcb.ecb.immediate_notify_res_count = 0xff;

	ifcb.ecb.timeout = BYTE_SWAP16(0);
	ifcb.ecb.ext_options = BYTE_SWAP16(0);
	ifcb.ecb.response_acc_timer = 0;
	ifcb.ecb.interrupt_delay_timer = 0;
	ifcb.ecb.special_options = BYTE_SWAP16(0);
	
	// set VP options and count
	//ifcb.vp_count = 0; 		// none for test

	
	// DEBUG - copy the ifcb to phys memory
	{
//		Mem_Copy((char*)((UNSIGNED)&ifcb | KSEG1),	// to
//			(char*)&ifcb,		// from
//			sizeof(ifcb));		// number of bytes
		bcopy(
			(char*)&ifcb,		// from
			(char*)((UNSIGNED)&ifcb | KSEG1),	// to
			sizeof(ifcb));		// number of bytes
	}

	FCP_DUMP_HEX(TRACE_L2, "\n\r2200 IFCB: ", (U8 *)&ifcb, sizeof(ifcb));
	
	dma_address = (U32)FCP_Get_DMA_Address((void *)&ifcb);
		
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    
    // Initialize firmware command
    Write_ISP(Id, ISP_MAILBOX_0, MBC_INIT_FW); 
//    Write_ISP(Id, ISP_MAILBOX_0, MBC_INIT_FW_MID); 
    
    // Initialization CB address, bits 31-16
    // Mask off PCI bits
    Write_ISP(Id, ISP_MAILBOX_2, ((U32)dma_address) >>16); 
    
    // Initialization CB address, bits 15-0
    Write_ISP(Id, ISP_MAILBOX_3, (U32)dma_address & 0xffff); 

	// Request queue in pointer
    Write_ISP(Id, ISP_MAILBOX_4, 0);
    	
    // Request queue out pointer
    Write_ISP(Id, ISP_MAILBOX_5, 0); 
    	
    // Initialization Control Block address, bits 63-48
    Write_ISP(Id, ISP_MAILBOX_6, 0); 
    	
    // Initialization Control Block address, bits 47-32
    Write_ISP(Id, ISP_MAILBOX_7, 0); 
    		
	mailbox_status = FCP_Mailbox_Command(Id);

    if (mailbox_status != MB_STS_GOOD)
    {
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Send_2200_Initialization_Control_Block", 
			"Initialize firmware failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return FCP_ERROR_INIT_CONTROL_BLOCK;
	}
	
    return status;
    
} // FCP_Send_2200_Initialization_Control_Block
	
/*************************************************************************/
// FCP_Verify_RISC_Checksum
// Verify checksum after RISC code has been loaded.
/*************************************************************************/
STATUS FCP_Verify_RISC_Checksum(PINSTANCE_DATA Id)
{
	UNSIGNED	mailbox_status;
	STATUS		status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Verify_RISC_Checksum);
	
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    
 	// Verify checksum command
    Write_ISP(Id, ISP_MAILBOX_0, MBC_VER_CHKSUM); 
    	
    // Destination address (RISC)
    Write_ISP(Id, ISP_MAILBOX_1, risc_code_addr01); 
    	
	mailbox_status = FCP_Mailbox_Command(Id);

    if (mailbox_status != MB_STS_GOOD)
    {
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Verify_RISC_Checksum", 
			"RISC checksumfailed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return FCP_ERROR_TEST_RISC_RAM;
	}

    return status;
    
} // FCP_Verify_RISC_Checksum

