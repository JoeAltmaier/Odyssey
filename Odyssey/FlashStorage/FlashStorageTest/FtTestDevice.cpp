/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FtTestUnit.cpp
// 
// Description:
// This file simulates the flash device for the Flash File test. 
// This file contains methods that are common to the file test version
// and the memory test version.
// Replace this module with FfUnit.cpp for the real thing.
// 
// $Log: /Gemini/Odyssey/FlashStorage/FlashStorageTest/FtTestDevice.cpp $
// 
// 5     2/05/00 2:44p Jfrandeen
// 
// 4     1/12/00 10:32a Jfrandeen
// Use SGL interface
// 
// 3     11/22/99 10:38a Jfrandeen
// Implement interleaved flash
// 
// 2     9/06/99 6:08p Jfrandeen
// New cache
// 
// 1     8/03/99 11:34a Jfrandeen
// 
// 2     5/05/99 2:13p Jfrandeen
// 
// 1     4/01/99 7:36p Jfrandeen
// Files common to all flash file system test drivers
// 
// 1/13/99 Jim Frandeen: Create file
/*************************************************************************/

#include "ErrorLog.h"
#include "Simple.h"
#include "Callback.h"
#include "ErrorLog.h"
#include "FfController.h"
#include "FfWatchdog.h"
#include "FlashDevice.h"
#include "FtFlashTest.h"
#include "FlashTestData.h"
#include "FfCommon.h"

#include "FtTestDevice.h"
#include "Mips_reg.h"

extern "C" {
void swint1(void);
}

void Set_Bad_Spots();

/*************************************************************************/
// Unit globals
/*************************************************************************/

// We use one instance of the watchdog object.
extern FF_Watchdog					FF_watchdog;

Flash_Device_Status device_status[FF_NUM_UNITS_MAX];
Status operation_status[FF_NUM_UNITS_MAX];
#ifdef THREADX
TX_TIMER timer[FF_NUM_UNITS_MAX];
#else
NU_TIMER timer[FF_NUM_UNITS_MAX];
#endif

Callback_Context *timer_context[FF_NUM_UNITS_MAX];
void *p_erase_buffer;
extern U32					  FF_bit_mask[32];
extern FF_INTERRUPT_STATUS FF_interrupt_status;
char *p_mem_file;
char *p_mem_file_last;

// These globals are set for each test.
U32 test_percentage_format_errors;
U32 test_percentage_erase_errors;
U32 test_percentage_write_errors;

U32 test_ECC_error = 0;

// These globals are set by the error simulation dialog.
U32 percentage_format_errors;
U32 percentage_erase_errors;
U32 percentage_write_errors;

	
#define FF_NULL_STATUS (U32) -1

	
/*************************************************************************/
// FF_Get_Interrupt_Status
// Called by High Level Interrupt Service Routine
// Get status of page pusher.  
// A bit for each device indicates whether the device is busy (1) or ready (0).
/*************************************************************************/
void FF_Get_Interrupt_Status(FF_INTERRUPT_STATUS *p_interrupt_status)
{
	// Return value from global stored by interrupt handler.
	(*p_interrupt_status).source = interrupt_status.source;
	(*p_interrupt_status).controller_status = interrupt_status.controller_status;

	// Zero global
	interrupt_status.source = 0;
	interrupt_status.controller_status = 0;
}

/*************************************************************************/
// Erase_Page_Block
// Simulates flash erase page block.
/*************************************************************************/
STATUS FF_Sim_Device::Erase_Page_Block(
	Callback_Context *p_callback_context, 
	Flash_Address flash_address,
	Flash_Page_Map flash_page_map)
{
	// Get column from flash address.
	U32 column = flash_address.Column();

	U32 unit = flash_address.Unit_Index();

	// Get array number for the next byte.
	U32 array = flash_address.Array();

	// Make sure unit is not busy.
	CT_ASSERT(((busy_status.source & FF_bit_mask[unit]) == 0), FF_Test_Device_Erase_Page_Block);

	// Set unit busy bit.
	busy_status.source |= FF_bit_mask[unit];

	// For each page of the block...
	for (U32 sector = 0; sector < Flash_Address::Sectors_Per_Block(); sector++)
	{

		// For each byte of the page...
		for (U32 offset = 0; offset < Flash_Address::Bytes_Per_Page(); offset++)
		{
			// Get the device index for the next byte.
			// a concatenation of bank and device number.
			// C C C C C C C C C b b D D D
			U32 device_index = Flash_Address::Device_Index_From_Offset(offset);

			// Get the offset in the device page for the next byte.
			U32 device_column = Flash_Address::Device_Column_From_Offset(offset);

			// Get the page address for the next byte.
			U32 page_address = flash_page_map[device_index] + sector;

			// Calculate the byte address in simulated memory for the
			// next dest byte.
			char *p_destination;
			U32 column_offset = (U32)(column * Flash_Address::Bytes_Per_Column() );
			U32 device_offset = device_index * Flash_Address::Bytes_Per_Device_Page();
			U32 page_offset = page_address * Flash_Address::Bytes_Per_Page();
			U32 array_offset = array * (U32)Flash_Address::Bytes_Per_Array();

			p_destination = p_mem_file 
				+ column_offset
				+ device_offset
				+ page_offset
				+ array_offset
				+ device_column;

			// Make sure we don't write past the end of memory buffer.
			CT_ASSERT((p_destination <= p_mem_file_last), Erase_Page_Block);

			// Store the next byte in the destination.
			*(unsigned char *)(p_destination) = 0Xff;
		}
	}

	// Reset device busy bit.
	busy_status.source &= ~FF_bit_mask[unit];

	// Run callback context
	p_callback_context->Make_Ready();

	//Set_Bad_Spots();


	return OK;

}  // FF_Test_Device_Erase_Page_Block

#if 0
// Rewrite this later
/*************************************************************************/
// FF_Device_Erase_Page_Complete
/*************************************************************************/
void FF_Sim_Device::FF_Device_Erase_Page_Complete(void *p_context, STATUS status)
{
	FT_Test_Context *p_test_context = (FT_Test_Context *)p_context;

	Flash_Device *p_device = p_test_context->m_p_device;

	U32 device = p_device->Unit_From_SSD_Address(p_test_context->m_real_SSD_address);
	U32 page_number = p_device->Page_From_SSD_Address(p_test_context->m_real_SSD_address);

	// Save completion status.
	device_status[device].status[0] = status; // Assume no error
	U32 percentage_errors = test_percentage_format_errors;
	if (percentage_errors == 0)
		percentage_errors = test_percentage_erase_errors;
	if (percentage_errors)
	{
		// For testing, we can simulate erase errors.
		U32 erase_status = 0; // assume no error
		U32 rand99 = (U32)Random_Number() % 100; // random number from 0 to 99
		U32 error = 100 / percentage_errors;
		if (error)
			error = rand99 % error;
		if (error == 0)
		{

			// Simulate error.  Choose one sector.
			U32 error_sector = (U32)Random_Number() % p_device->Sectors_Per_Block();
			erase_status = 1 << error_sector;

			Create_Bad_Sector(
				device,
				(page_number + error_sector) * p_device->Bytes_Per_Page(), // offset in file 
				p_device->Bytes_Per_Page() // number of bytes 
				);

		}
		device_status[device].status[0] = erase_status;
	}

	// Turn off busy bit for this device.
	CT_ASSERT(((busy_status.source & FF_bit_mask[device])), FF_Device_Erase_Page_Complete);
	busy_status.source &= ~FF_bit_mask[device];

	// Turn on interrupt for this device.
	interrupt_status.source |= FF_bit_mask[device];

	Callback_Context::Terminate(p_test_context, status);

	// Simulate interrupt
	// Get status of page pusher.  
	// A bit for each device indicates whether the device is busy (1) or ready (0).
	// We must get the status here in order to reset the interrupt.

#ifdef _WINDOWS
	FF_Get_Interrupt_Status(&FF_interrupt_status);
	FF_ISR_High();
#else
	swint1();
#endif


} // FF_Device_Erase_Page_Complete
#endif

/*************************************************************************/
// FF_Get_Unit_ID
// Get status of operation
/*************************************************************************/
FF_DEVICE_ID FF_Get_Unit_ID(U32 real_SSD_address)
{
	return FF_DEVICE_ID_SAMSUNG_128;
}

/*************************************************************************/
// FF_Get_Device_Status
// Get status of operation
/*************************************************************************/
Status FF_Sim_Device::Get_Device_Status(Flash_Address flash_address, 
		UI64 controller_status, Flash_Device_Status *p_device_status)
{
	U32 unit_index = flash_address.Unit_Index();

	// Make sure device is not busy.
	CT_ASSERT(((busy_status.source & FF_bit_mask[unit_index]) == 0), 
		Get_Device_Status);

	if (operation_status[unit_index])
	{
		(*p_device_status).bank_status[0] = 1;
	}
	else
	{
		(*p_device_status).bank_status[0] = 0;
	}

	(*p_device_status).device_is_ready = 1;
	Status status = operation_status[unit_index];
	operation_status[unit_index] = OK;
	return status;

} // FF_Get_Device_Status

/*************************************************************************/
// Read_Page
// This method is called when the device is ready to transfer the data
// to be read.
/*************************************************************************/
STATUS FF_Sim_Device::Read_Page(
	Callback_Context *p_callback_context, 
	Flash_Address flash_address,
	Flash_Page_Map flash_page_map,
	FF_SGL *p_sgl)
{
	TyDma *p_tydma;

	// Create TyDma from SGL.
	STATUS status = p_sgl->Build_TyDma(0, // TransferAddress (source) not used
		Flash_Address::Bytes_Per_Page(), // transfer_byte_count
		FF_SGL::Transfer_Address_Is_Source,
		NULL, // p_callback
		NULL, // p_context
		0, // flags
		&p_tydma);
	if (status != OK)
		return status;

	// Initialize element pointer and count.
	char *p_destination = (char *)p_tydma->pDst;
	U32 element_count = p_tydma->cb;

	// Get column from flash address.
	U32 column = flash_address.Column();

	U32 unit = flash_address.Unit_Index();

	// Get array number for the next byte.
	U32 array = flash_address.Array();

	// Make sure unit is not busy.
	CT_ASSERT(((busy_status.source & FF_bit_mask[unit]) == 0), FF_Test_Device_Erase_Page_Block);

	// Set unit busy bit.
	busy_status.source |= FF_bit_mask[unit];

	// For each byte of the page...
	char *p_source;
	for (U32 offset = 0; offset < Flash_Address::Bytes_Per_Page(); offset++)
	{
		// Get the device index for the next byte.
		// a concatenation of bank and device number.
		// C C C C C C C C C b b D D D
		U32 device_index = Flash_Address::Device_Index_From_Offset(offset);

		// Get the offset in the device page for the next byte.
		U32 device_column = Flash_Address::Device_Column_From_Offset(offset);

		// Get the page address for the next byte.
		// This page_address includes the sector.
		U32 page_address = flash_page_map[device_index];

		// Calculate the byte address in simulated memory for the
		// next dest byte.
		U32 column_offset = (U32)(column * Flash_Address::Bytes_Per_Column() );
		U32 device_offset = device_index * Flash_Address::Bytes_Per_Device_Page();
		U32 page_offset = page_address * Flash_Address::Bytes_Per_Page();
		U32 array_offset = array * (U32)Flash_Address::Bytes_Per_Array();

		p_source = p_mem_file 
			+ column_offset
			+ device_offset
			+ page_offset
			+ array_offset
			+ device_column;

		// Make sure we don't write past the end of memory buffer.
		CT_ASSERT((p_destination <= p_mem_file_last), Erase_Page_Block);

		if (element_count == 0)
		{
			// Get next element.
			TyDma *p_tydma_last = p_tydma;
			p_tydma = p_tydma->pNext;
			delete p_tydma_last;

			// Make sure we have another element.
			p_destination = (char *)p_tydma->pDst;
			element_count = p_tydma->cb;
		}

		// Store the next byte in the destination.
		*(p_destination) = *p_source;

		// Decrement number of bytes left in element.
		--element_count;

		// Calculate the next destination address in the user's buffer.
		p_destination++;

	}

	delete p_tydma;

	// Reset device busy bit.
	busy_status.source &= ~FF_bit_mask[unit];

#if 1
	if (++test_ECC_error == 2)
	{
		test_ECC_error = 0;
		operation_status[unit] = FF_ERROR_CODE(ECC_CORRECTED);
	}
	else
		operation_status[unit] = OK;
#endif

	// Run callback context
	p_callback_context->Make_Ready();

	return OK;

}  // Read_Page
	
/*************************************************************************/
// Verify_Page
// This method is called when the device is ready to transfer the data
// to be read.
/*************************************************************************/
STATUS FF_Sim_Device::Verify_Page(
	Callback_Context *p_callback_context, 
	Flash_Address flash_address,
	Flash_Page_Map flash_page_map,
	FF_SGL *p_sgl)
{
	TyDma *p_tydma;

	// Create TyDma from SGL.
	STATUS status = p_sgl->Build_TyDma(0, // TransferAddress (source) not used
		Flash_Address::Bytes_Per_Page(), // transfer_byte_count
		FF_SGL::Transfer_Address_Is_Source,
		NULL, // p_callback
		NULL, // p_context
		0, // flags
		&p_tydma);
	if (status != OK)
		return status;

	// Initialize element pointer and count.
	char *p_destination = (char *)p_tydma->pDst;
	U32 element_count = p_tydma->cb;

	// Get column from flash address.
	U32 column = flash_address.Column();

	U32 unit = flash_address.Unit_Index();

	// Get array number for the next byte.
	U32 array = flash_address.Array();

	// Make sure unit is not busy.
	CT_ASSERT(((busy_status.source & FF_bit_mask[unit]) == 0), FF_Test_Device_Erase_Page_Block);

	// Set unit busy bit.
	busy_status.source |= FF_bit_mask[unit];

	// For each byte of the page...
	char *p_source;
	for (U32 offset = 0; offset < Flash_Address::Bytes_Per_Page(); offset++)
	{
		// Get the device index for the next byte.
		// a concatenation of bank and device number.
		// C C C C C C C C C b b D D D
		U32 device_index = Flash_Address::Device_Index_From_Offset(offset);

		// Get the offset in the device page for the next byte.
		U32 device_column = Flash_Address::Device_Column_From_Offset(offset);

		// Get the page address for the next byte.
		// This page_address includes the sector.
		U32 page_address = flash_page_map[device_index];

		// Calculate the byte address in simulated memory for the
		// next dest byte.
		U32 column_offset = (U32)(column * Flash_Address::Bytes_Per_Column() );
		U32 device_offset = device_index * Flash_Address::Bytes_Per_Device_Page();
		U32 page_offset = page_address * Flash_Address::Bytes_Per_Page();
		U32 array_offset = array * (U32)Flash_Address::Bytes_Per_Array();

		p_source = p_mem_file 
			+ column_offset
			+ device_offset
			+ page_offset
			+ array_offset
			+ device_column;

		// Make sure we don't write past the end of memory buffer.
		CT_ASSERT((p_source <= p_mem_file_last), Erase_Page_Block);

		if (element_count == 0)
		{
			// Get next element.
			TyDma *p_tydma_last = p_tydma;
			p_tydma = p_tydma->pNext;
			delete p_tydma_last;

			// Make sure we have another element.
			p_destination = (char *)p_tydma->pDst;
			element_count = p_tydma->cb;
		}

		// Compare the next source byte with the destination.
		if (*(p_destination) != *p_source)
		{
			p_callback_context->Set_Status(FF_ERROR(VERIFY));
			break;
		}

		// Decrement number of bytes left in element.
		--element_count;

		// Calculate the next destination address in the user's buffer.
		p_destination++;

	}

	delete p_tydma;

	// Reset device busy bit.
	busy_status.source &= ~FF_bit_mask[unit];

	// Run callback context
	p_callback_context->Make_Ready();

	return OK;

}  // Verify_Page
	
/*************************************************************************/
// Write_Page
// Write page from flash buffer
/*************************************************************************/
STATUS FF_Sim_Device::Write_Page(
	Callback_Context *p_callback_context, 
	Flash_Address flash_address,
	Flash_Page_Map flash_page_map,
	FF_SGL *p_sgl)
{
	TyDma *p_tydma;

	// Create TyDma from SGL.
	STATUS status = p_sgl->Build_TyDma(0, // TransferAddress (dest) not used
		Flash_Address::Bytes_Per_Page(), // transfer_byte_count
		FF_SGL::Transfer_Address_Is_Dest,
		NULL, // p_callback
		NULL, // p_context
		0, // flags
		&p_tydma);
	if (status != OK)
		return status;

	// Initialize element pointer and count.
	char *p_source = (char *)p_tydma->pSrc;
	U32 element_count = p_tydma->cb;
	U32 column = flash_address.Column();

	U32 unit = flash_address.Unit_Index();

	// Get array number for the next byte.
	U32 array = flash_address.Array();

	// Make sure unit is not busy.
	CT_ASSERT(((busy_status.source & FF_bit_mask[unit]) == 0), FF_Test_Device_Erase_Page_Block);

	// Set unit busy bit.
	busy_status.source |= FF_bit_mask[unit];

	// For each byte of the page...
	for (U32 offset = 0; offset < Flash_Address::Bytes_Per_Page(); offset++)
	{
		// Get the device index for the next byte.
		// a concatenation of bank and device number.
		// C C C C C C C C C b b D D D
		U32 device_index = Flash_Address::Device_Index_From_Offset(offset);

		// Get the offset in the device page for the next byte.
		U32 device_column = Flash_Address::Device_Column_From_Offset(offset);

		// Get the page address for the next byte.
		// This page_address includes the sector.
		U32 page_address = flash_page_map[device_index];

		// Calculate the byte address in simulated memory for the
		// next dest byte.
		char *p_destination;
		U32 column_offset = (U32)(column * Flash_Address::Bytes_Per_Column() );
		U32 device_offset = device_index * Flash_Address::Bytes_Per_Device_Page();
		U32 page_offset = page_address * Flash_Address::Bytes_Per_Page();
		U32 array_offset = array * (U32)Flash_Address::Bytes_Per_Array();

		p_destination = p_mem_file 
			+ column_offset
			+ device_offset
			+ page_offset
			+ array_offset
			+ device_column;

		// Make sure we don't write past the end of memory buffer.
		CT_ASSERT((p_destination <= p_mem_file_last), Erase_Page_Block);

		if (element_count == 0)
		{
			// Get next element.
			TyDma *p_tydma_last = p_tydma;
			p_tydma = p_tydma->pNext;
			delete p_tydma_last;

			// Make sure we have another element.
			p_source = (char *)p_tydma->pSrc;
			element_count = p_tydma->cb;
		}

		// Store the next byte in the destination.
		*(p_destination) = *p_source;

		// Decrement number of bytes left in element.
		--element_count;

		// Calculate the next source address in the user's buffer.
		p_source++;

	}

	delete p_tydma;

	// Reset device busy bit.
	busy_status.source &= ~FF_bit_mask[unit];

	// Run callback context
	p_callback_context->Make_Ready();

	return OK;

}  // Write_Page
	
/*************************************************************************/
// FF_Sim_Device::Wait_Write_Complete
// Wait for the device to complete the write operation.
/*************************************************************************/
Status FF_Sim_Device::Wait_Write_Complete(
	Callback_Context *p_callback_context,
	Flash_Address flash_address) 
{
	// Wait for the interrupt to occur.
	p_callback_context->Make_Ready();
	return OK;
}

