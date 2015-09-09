/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiDebug.c
// 
// Description:
// This file implements the ISP Debug methods 
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiDebug.c $ 
// 
// 1     9/14/99 7:23p Cchan
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


/*************************************************************************/
// HSCSI_Debug_Create
// Create HSCSI_Memory object
/*************************************************************************/
STATUS	HSCSI_Debug_Create(PHSCSI_INSTANCE_DATA Id)
{
 	HSCSI_TRACE_ENTRY(HSCSI_Debug_Create);
	
	return NU_SUCCESS;
	
} // HSCSI_Debug_Create

/*************************************************************************/
// HSCSI_Debug_Destroy
/*************************************************************************/
void	HSCSI_Debug_Destroy()
{
 	HSCSI_TRACE_ENTRY(HSCSI_Debug_Destroy);
		
} // HSCSI_Debug_Destroy

#if defined(HSCSI_DEBUG) && defined(_DEBUG)
// Don't include any of this if we are not debugging

// RISC code to load ISP is in fw1040ei.c (initiator-only firmware)
extern unsigned short	hscsi_risc_code01[];
extern unsigned short   hscsi_risc_code_length01;
extern unsigned short	hscsi_risc_code_version;
extern unsigned short	hscsi_risc_code_addr01;

/*************************************************************************/
// HSCSI_Dump_RISC_RAM
// Load RISC RAM from memory using Load RAM command.
/*************************************************************************/
STATUS HSCSI_Dump_RISC_RAM(PHSCSI_INSTANCE_DATA Id, U8 *Address, U32 Length, U16 Start_Addr)
{
	U16					 mailbox_status;
	STATUS		 		 status = NU_SUCCESS;
	
 	HSCSI_TRACE_ENTRY(HSCSI_Dump_RISC_RAM);
 	
    status = HSCSI_Mailbox_Wait_Ready_Intr(Id);
    if (status != NU_SUCCESS)
    	return status;
    	
	HSCSI_PRINT_STRING(TRACE_L8, "\n\rDumping RISC RAM from memory using Dump RISC RAM command.");
	HSCSI_PRINT_HEX(TRACE_L8, "\n\rAddress of buffer = ", (UNSIGNED)Address);
	 
 	// Load RISC RAM command
    Write_ISP1040(Id, HSCSI_MAILBOX_0, HSCSI_MBC_DUMP_RAM); 			// 32 bit dump
    	
    // Source address (RISC)
    Write_ISP1040(Id, HSCSI_MAILBOX_1, Start_Addr);
    
    // Source address, bits 31-16
    // Mask off PCI bits
    Write_ISP1040(Id, HSCSI_MAILBOX_2, ((U32)HSCSI_Get_DMA_Address((void *)Address) >>16)); 
    	
    // Source address, bits 15-0 
    Write_ISP1040(Id, HSCSI_MAILBOX_3, (U32)HSCSI_Get_DMA_Address((void *)Address) & 0xffff); 

    // Length
    Write_ISP1040(Id, HSCSI_MAILBOX_4, Length & 0xffff); 
    	
    // Initialization CB address, bits 63-48
    Write_ISP1040(Id, HSCSI_MAILBOX_6, 0); 
    	
    // Initialization CB address, bits 47-32
    Write_ISP1040(Id, HSCSI_MAILBOX_7, 0); 
    	
	mailbox_status = HSCSI_Mailbox_Command(Id);
	if (mailbox_status != MB_STS_GOOD)
	{
		HSCSI_Log_Error(HSCSI_ERROR_TYPE_FATAL,
			"HSCSI_Dump_RISC_RAM", 
			"Dump RISC RAM failed",
			mailbox_status,
			(UNSIGNED)Id);
			
		return HSCSI_ERROR_LOAD_RISC_RAM;
	}
	
    return status;
	
} // HSCSI_Dump_RISC_RAM

/*************************************************************************/
// HSCSI_ISP_Print_Regs
// Print all ISP registers.
/*************************************************************************/
void HSCSI_ISP_Print_Regs(PHSCSI_INSTANCE_DATA Id)
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
		*value++ = Read_ISP1040(Id, index);

	TraceLevel[TRACE_INDEX] = trace_level_save;
		
	HSCSI_DUMP_HEX(TRACE_L8, "\n\rISP Registers =", (U8 *)Buffer, 256);

} // HSCSI_ISP_Print_Regs


/*************************************************************************/
// HSCSI_Print_RAM_Word
// For debugging, print value of RAM word.
// This only works in polling mode. 
/*************************************************************************/
 void HSCSI_Print_RAM_Word(PHSCSI_INSTANCE_DATA Id, U16 index)
{
	UNSIGNED			trace_level_save;
	
	// Don't print registers or trace during this routine.
	trace_level_save = TraceLevel[TRACE_INDEX];
	TraceLevel[TRACE_INDEX] = TRACE_OFF_LVL;
	
	HSCSI_Mailbox_Wait_Ready_Intr(Id);
	
 	// Read RISC RAM command
    Write_ISP1040(Id, HSCSI_MAILBOX_0, HSCSI_MBC_RD_RAM_WORD); 
    	
    // Destination address (RISC)
    Write_ISP1040(Id, HSCSI_MAILBOX_1, hscsi_risc_code_addr01 + index); 
    	
	// Wait for Read RISC RAM command
	HSCSI_Mailbox_Command(Id);
	
	TraceLevel[TRACE_INDEX] = trace_level_save;
	
	HSCSI_PRINT_HEX16(TRACE_L8, "\n\r\RAM address = ", index);
	HSCSI_PRINT_HEX16(TRACE_L8, ", word = ", Read_ISP1040(Id, HSCSI_MAILBOX_2));
	
} // HSCSI_Print_RAM_Word

/*************************************************************************/
// HSCSI_Verify_RISC_RAM
// Read each word of RISC RAM and compare it to what we sent. 
/*************************************************************************/
 void HSCSI_Verify_RISC_RAM(PHSCSI_INSTANCE_DATA Id)
{
	UNSIGNED			trace_level_save;
	UNSIGNED			index;
	U16					value;
	
	// Don't print registers or trace during this routine.
	trace_level_save = TraceLevel[TRACE_INDEX];
	TraceLevel[TRACE_INDEX] = TRACE_OFF_LVL;
	
	for (index = 0; index < hscsi_risc_code_length01; index++)
	{
 		// Read RAM command
    	Write_ISP1040(Id, HSCSI_MAILBOX_0, HSCSI_MBC_RD_RAM_WORD); 
    	
    	// Destination address (RISC) of first word
    	Write_ISP1040(Id, HSCSI_MAILBOX_1, hscsi_risc_code_addr01 + index);
        
    	// Destination word
    	Write_ISP1040(Id, HSCSI_MAILBOX_2, hscsi_risc_code01[index]);

 		// Wait for Read RISC RAM command
		HSCSI_Mailbox_Command(Id);
		
		// Test word read
		value = Read_ISP1040(Id, HSCSI_MAILBOX_2);
		if (value != BYTE_SWAP16(hscsi_risc_code01[index]))
		{			
			TraceLevel[TRACE_INDEX] = trace_level_save;
			printf("\n\rHSCSI_Verify_RISC_RAM failed, index = %04x", index);
			printf("\n\rValue read = %04x", value);
			printf(", risc_code01[index] = %04x", BYTE_SWAP16(hscsi_risc_code01[index]));
			printf("\n\rAddress of risc_code01[index] = %08x", (UNSIGNED)&hscsi_risc_code01[index]);
			HSCSI_Error_Stop();
		}
   }

	TraceLevel[TRACE_INDEX] = trace_level_save;
	
} // HSCSI_Verify_RISC_RAM

	
/*************************************************************************/
// Read_ISP1040 
// For debugging, print the value read
/*************************************************************************/
U16 Read_ISP1040(PHSCSI_INSTANCE_DATA Id, U16 ISP_register)
{
	U16		*p_ISP_Register = (U16*)((UNSIGNED)Id->Regs + ISP_register);
	U16		 value;
	
	value = BYTE_SWAP16(*p_ISP_Register);
	if (TraceLevel[TRACE_INDEX] >= Id->HSCSI_if_print_ISR)
	{
		printf("\n\rReading ISP_Register %08x", (UNSIGNED)p_ISP_Register);
		printf("  %04x", ISP_register);
		printf(", value = %04x", value);
	}
	return value;
} // Read_ISP1040
	
/*************************************************************************/
// Write_ISP1040 
// For debugging, print the value written
/*************************************************************************/
void Write_ISP1040(PHSCSI_INSTANCE_DATA Id, U16 ISP_register, U16 value)
{
	U16		*p_ISP_Register = (U16*)((UNSIGNED)Id->Regs + ISP_register);
	
	if (TraceLevel[TRACE_INDEX] >= Id->HSCSI_if_print_ISR)
	{
		printf("\n\rWriting to ISP_Register %08x", (UNSIGNED)p_ISP_Register);
		printf(", value = %04x", value);
	}
	*p_ISP_Register = BYTE_SWAP16(value);
} // Write_ISP1040

#endif
