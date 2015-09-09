/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpDebug.c
// 
// Description:
// This file implements the ISP Debug methods 
// 
// Update Log 
// 8/17/98 Michael G. Panas: Create file
// 10/6/98 Michael G. Panas: changes to support interrupt driven mailbox commands
// 11/30/98 Michael G. Panas: Add create/destroy
/*************************************************************************/
#include "FcpCommon.h"
#include "FcpData.h"
#include "FcpError.h"
#include "FcpDebug.h"
#include "FcpISP.h"
#include "FcpMailbox.h"
#include "FcpMemory.h"
#include "FcpRequestFIFO.h"
#include "FcpResponseFIFO.h"
#include "Nucleus.h"
#include "FcpString.h"


/*************************************************************************/
// FCP_Debug_Create
// Create FCP_Memory object
/*************************************************************************/
STATUS	FCP_Debug_Create(PINSTANCE_DATA Id)
{
 	FCP_TRACE_ENTRY(FCP_Debug_Create);
	
	return NU_SUCCESS;
	
} // FCP_Debug_Create

/*************************************************************************/
// FCP_Debug_Destroy
/*************************************************************************/
void	FCP_Debug_Destroy()
{
 	FCP_TRACE_ENTRY(FCP_Debug_Destroy);
		
} // FCP_Debug_Destroy

#if defined(FCP_DEBUG) && defined(_DEBUG)
// Don't include any of this if we are not debugging

// RISC code to load ISP is in 2100e.c
extern unsigned short	risc_code01[];
extern unsigned short   risc_code_length01;
extern unsigned short	risc_code_version;
extern unsigned short	risc_code_addr01;

/*************************************************************************/
// FCP_Dump_RISC_RAM
// Read RISC RAM into memory
/*************************************************************************/
STATUS FCP_Dump_RISC_RAM(PINSTANCE_DATA Id, U8 *Address, U32 Length, U16 Start_Addr)
{
	U16					 mailbox_status;
	STATUS		 		 status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Dump_RISC_RAM);
 	
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    	
	FCP_PRINT_STRING(TRACE_L8, "\n\rDumping RISC RAM to memory using Dump RISC RAM command.");
	FCP_PRINT_HEX(TRACE_L8, "\n\rAddress of buffer = ", (UNSIGNED)Address);
	 
 	// Load RISC RAM command
//    Write_ISP(Id, ISP_MAILBOX_0, MBC_DUMP_RAM); 			// 32 bit dump
    Write_ISP(Id, ISP_MAILBOX_0, MBC_DUMP_RISC_RAM);		// 64 bit dump
    	
    // Source address (RISC)
    Write_ISP(Id, ISP_MAILBOX_1, Start_Addr);
    
    // Source address, bits 31-16
    // Mask off PCI bits
    Write_ISP(Id, ISP_MAILBOX_2, ((U32)FCP_Get_DMA_Address((void *)Address) >>16)); 
    	
    // Source address, bits 15-0 
    Write_ISP(Id, ISP_MAILBOX_3, (U32)FCP_Get_DMA_Address((void *)Address) & 0xffff); 

    // Length
    Write_ISP(Id, ISP_MAILBOX_4, Length & 0xffff); 
    	
    // Initialization CB address, bits 63-48
    Write_ISP(Id, ISP_MAILBOX_6, 0); 
    	
    // Initialization CB address, bits 47-32
    Write_ISP(Id, ISP_MAILBOX_7, 0); 
    	
	mailbox_status = FCP_Mailbox_Command(Id);
	if (mailbox_status != MB_STS_GOOD)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_WARNING,
			"FCP_Dump_RISC_RAM", 
			"Dump RISC RAM failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return FCP_ERROR_LOAD_RISC_RAM;
	}
	
    return status;
	
} // FCP_Dump_RISC_RAM


/*************************************************************************/
// FCP_Get_Initialization_Control_Block
// The purpose of this method is to get the initialization control block
// for debugging purposes.
/*************************************************************************/
STATUS FCP_Get_Initialization_Control_Block(PINSTANCE_DATA Id,
									FCP_EVENT_CONTEXT *p_context)
{
	INITIALIZE_FIRMWARE_CONTROL_BLOCK 	ifcb;
	UNSIGNED							mailbox_status;
	STATUS								status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Get_Initialization_Control_Block);

    bzero((char *)&ifcb, sizeof(ifcb));
	
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    
    // Get ICB command
    Write_ISP(Id, ISP_MAILBOX_0, MBC_GET_ICB); 
    
    // Initialization CB address, bits 31-16
    // Mask off PCI bits
    Write_ISP(Id, ISP_MAILBOX_2, ((U32)FCP_Get_DMA_Address((void *)&ifcb) >>16)); 
    
    // Initialization CB address, bits 15-0
    Write_ISP(Id, ISP_MAILBOX_3, (U32)FCP_Get_DMA_Address((void *)&ifcb) & 0xffff); 
    	
    // Initialization Control Block address, bits 63-48
    Write_ISP(Id, ISP_MAILBOX_6, 0); 
    	
    // Initialization Control Block address, bits 47-32
    Write_ISP(Id, ISP_MAILBOX_7, 0); 
    		
	mailbox_status = FCP_Mailbox_Command(Id);
	if (mailbox_status != MB_STS_GOOD)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_FATAL,
			"FCP_Get_Initialization_Control_Block", 
			"Get ICB failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return FCP_ERROR_GET_INIT_CONTROL_BLOCK;
	}
	
    {
    	U16			*p_ifcb_word = (U16*)&ifcb;
    	UNSIGNED	 index;
    	UNSIGNED	 index_last = sizeof(ifcb) / 2 - 1;
    	
    	for (index = 0; index < index_last; index++)
    	{
    		FCP_PRINT_HEX16(TRACE_L8, "\n\r", index);
    		FCP_PRINT_HEX16(TRACE_L8, " ", p_ifcb_word[index]);
    	}    	
    }

    return status;
    
} // FCP_Get_Initialization_Control_Block


/*************************************************************************/
// FCP_ISP_Print_Regs
// Print all ISP registers.
/*************************************************************************/
void FCP_ISP_Print_Regs(PINSTANCE_DATA Id)
{
	U16					index;
	U16					*value;
	UNSIGNED			trace_level_save;
	U8					Buffer[256*4];
    		
	// Don't print registers or trace during this routine.
	trace_level_save = TraceLevel[TRACE_INDEX];
	TraceLevel[TRACE_INDEX] = TRACE_OFF_LVL;

	value = (U16 *) Buffer;
	for (index = 0; index < 256; index += 2)
		*value++ = Read_ISP(Id, index);

	TraceLevel[TRACE_INDEX] = trace_level_save;
		
	FCP_DUMP_HEX(TRACE_L8, "\n\rISP Registers =", (U8 *)Buffer, 256);

} // FCP_ISP_Print_Regs


/*************************************************************************/
// FCP_Load_RISC_RAM_DBG
// Load RISC RAM from memory using Load RAM command.
/*************************************************************************/
STATUS FCP_Load_RISC_RAM_DBG(PINSTANCE_DATA Id, U8 *Address, U32 Length, U16 Start_Addr)
{
	U16					 mailbox_status;
	STATUS		 		 status = NU_SUCCESS;
	U32			 		 dma_address;
	
 	FCP_TRACE_ENTRY(FCP_Dump_RISC_RAM);
 	
    status = FCP_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    	
	FCP_PRINT_STRING(TRACE_L8, "\n\rLoading RISC RAM from memory using Load RISC RAM command.");
	FCP_PRINT_HEX(TRACE_L8, "\n\rAddress of buffer = ", (UNSIGNED)Address);
	 
	dma_address = (U32)FCP_Get_DMA_Address((void *)Address);
	
 	// Load RISC RAM command
//    Write_ISP(Id, ISP_MAILBOX_0, MBC_LOAD_RAM); 			// 32 bit dump
    Write_ISP(Id, ISP_MAILBOX_0, MBC_LOAD_RISC_RAM);		// 64 bit dump
    	
    // Source address (RISC)
    Write_ISP(Id, ISP_MAILBOX_1, Start_Addr);
    
    // Source address, bits 31-16
    // Mask off PCI bits
    Write_ISP(Id, ISP_MAILBOX_2, ((U32)dma_address >>16)); 
    	
    // Source address, bits 15-0 
    Write_ISP(Id, ISP_MAILBOX_3, (((U32)dma_address) & 0xffff)); 

    // Length
    Write_ISP(Id, ISP_MAILBOX_4, Length & 0xffff); 
    	
    // Initialization CB address, bits 63-48
    Write_ISP(Id, ISP_MAILBOX_6, 0); 
    	
    // Initialization CB address, bits 47-32
    Write_ISP(Id, ISP_MAILBOX_7, 0); 
    	
	mailbox_status = FCP_Mailbox_Command(Id);
	if (mailbox_status != MB_STS_GOOD)
	{
		FCP_Log_Error(FCP_ERROR_TYPE_WARNING,
			"FCP_Load_RISC_RAM_DBG", 
			"Load RISC RAM failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return FCP_ERROR_LOAD_RISC_RAM;
	}
	
    return status;
	
} // FCP_Load_RISC_RAM_DBG

/*************************************************************************/
// FCP_Print_RAM_Word
// For debugging, print value of RAM word.
// This only works in polling mode. 
/*************************************************************************/
 void FCP_Print_RAM_Word(PINSTANCE_DATA Id, U16 index)
{
	UNSIGNED			trace_level_save;
	
	// Don't print registers or trace during this routine.
	trace_level_save = TraceLevel[TRACE_INDEX];
	TraceLevel[TRACE_INDEX] = TRACE_OFF_LVL;
	
	FCP_Mailbox_Wait_Ready_Intr(Id);
	
 	// Read RISC RAM command
    Write_ISP(Id, ISP_MAILBOX_0, MBC_RD_RAM_WORD); 
    	
    // Destination address (RISC)
    Write_ISP(Id, ISP_MAILBOX_1, risc_code_addr01 + index); 
    	
	// Wait for Read RISC RAM command
	FCP_Mailbox_Command(Id);
	
	TraceLevel[TRACE_INDEX] = trace_level_save;
	
	FCP_PRINT_HEX16(TRACE_L8, "\n\r\RAM address = ", index);
	FCP_PRINT_HEX16(TRACE_L8, ", word = ", Read_ISP(Id, ISP_MAILBOX_2));
	
} // FCP_Print_RAM_Word

/*************************************************************************/
// FCP_Verify_RISC_RAM
// Read each word of RISC RAM and compare it to what we sent. 
/*************************************************************************/
 void FCP_Verify_RISC_RAM(PINSTANCE_DATA Id)
{
	UNSIGNED			trace_level_save;
	UNSIGNED			index;
	U16					value;
	
	// Don't print registers or trace during this routine.
	trace_level_save = TraceLevel[TRACE_INDEX];
	TraceLevel[TRACE_INDEX] = TRACE_OFF_LVL;
	
	for (index = 0; index < risc_code_length01; index++)
	{
 		// Read RAM command
    	Write_ISP(Id, ISP_MAILBOX_0, MBC_RD_RAM_WORD); 
    	
    	// Destination address (RISC) of first word
    	Write_ISP(Id, ISP_MAILBOX_1, risc_code_addr01 + index);
        
    	// Destination word
    	Write_ISP(Id, ISP_MAILBOX_2, risc_code01[index]);

 		// Wait for Read RISC RAM command
		FCP_Mailbox_Command(Id);
		
		// Test word read
		value = Read_ISP(Id, ISP_MAILBOX_2);
		if (value != BYTE_SWAP16(risc_code01[index]))
		{			
			TraceLevel[TRACE_INDEX] = trace_level_save;
			printf("\n\rFCP_Verify_RISC_RAM failed, index = %04x", index);
			printf("\n\rValue read = %04x", value);
			printf(", risc_code01[index] = %04x", BYTE_SWAP16(risc_code01[index]));
			printf("\n\rAddress of risc_code01[index] = %08x", (UNSIGNED)&risc_code01[index]);
			FCP_Error_Stop();
		}
   }

	TraceLevel[TRACE_INDEX] = trace_level_save;
	
} // FCP_Verify_RISC_RAM

	
/*************************************************************************/
// FCP_Verify_RISC_RAM_2200
// Verfy the second RAM chip on the 2200 is working
/*************************************************************************/
void FCP_Verify_RISC_RAM_2200(PINSTANCE_DATA Id, U16 Start_Addr)
{
	STATUS				status;
	UNSIGNED			index;
	U8					wrbuffer[256];
	U8					rdbuffer[256];
	
	// fill with a known pattern
	for (index = 0; index < 256; index++)
		wrbuffer[index] = index;
		
    FCP_PRINT_HEX(TRACE_L2, "\n\rStart Address: ", Start_Addr);

	// Load the RISC RAM
	status = FCP_Load_RISC_RAM_DBG(Id, wrbuffer, 256, Start_Addr);
    //if (status != NU_SUCCESS)
    	//return status;
    	
	// Dump the RISC RAM
	status = FCP_Dump_RISC_RAM(Id, rdbuffer, 256, Start_Addr);
    //if (status != NU_SUCCESS)
    	//return status;
    
    // print both buffers
    FCP_DUMP_HEX(TRACE_L2, "\n\rWritten: ", wrbuffer, 256);
    FCP_DUMP_HEX(TRACE_L2, "\n\rRead: ", rdbuffer, 256);
    
    // compare the data
	for (index = 0; index < 256; index++)
	{
		if (wrbuffer[index] != rdbuffer[index])
		{
			printf("\n\rData miscompare: index=%d", index);
			break;
		}
	}
	
} // FCP_Verify_RISC_RAM_2200

/*************************************************************************/
// Read_ISP 
// For debugging, print the value read
/*************************************************************************/
U16 Read_ISP(PINSTANCE_DATA Id, U16 ISP_register)
{
	U16		*p_ISP_Register = (U16*)((UNSIGNED)Id->Regs + ISP_register);
	U16		 value;
	
	value = BYTE_SWAP16(*p_ISP_Register);
	if (TraceLevel[TRACE_INDEX] >= Id->FCP_if_print_ISR)
	{
		printf("\n\rReading ISP_Register %08x", (UNSIGNED)p_ISP_Register);
		printf("  %04x", ISP_register);
		printf(", value = %04x", value);
	}
	return value;
} // Read_ISP
	
/*************************************************************************/
// Write_ISP 
// For debugging, print the value written
/*************************************************************************/
void Write_ISP(PINSTANCE_DATA Id, U16 ISP_register, U16 value)
{
	U16		*p_ISP_Register = (U16*)((UNSIGNED)Id->Regs + ISP_register);
	
	if (TraceLevel[TRACE_INDEX] >= Id->FCP_if_print_ISR)
	{
		printf("\n\rWriting to ISP_Register %08x", (UNSIGNED)p_ISP_Register);
		printf(", value = %04x", value);
	}
	*p_ISP_Register = BYTE_SWAP16(value);
} // Write_ISP

#endif
