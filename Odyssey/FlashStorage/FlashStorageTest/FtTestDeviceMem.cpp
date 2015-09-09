/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FtTestDeviceMem.cpp
// 
// Description:
// This file simulates the flash device for the Flash File test.
// This file contains the methods for simulating the flash file
// in memory. 
// Remove this module from the project for the real thing.
// 
// 1/13/99 Jim Frandeen: Create file
/*************************************************************************/

#include "ErrorLog.h"
#include "Simple.h"
#include "Callback.h"
#include "ErrorLog.h"
#include "FfController.h"
#include "FlashDevice.h"
#include "SsdDeviceISR.h"
#include "FfWatchdog.h"
#include "FtFlashTest.h"
#include "FlashTestData.h"
#include "FfCommon.h"

#include "FtTestDevice.h"
#include <String.h>
#include <StdLib.h>

	
// Wait for device to be ready before we send the data -- for simulation.
#define FF_READ_TIMER_TICKS 1

// Wait for device to be ready when transfer has completed -- for simulation.
#define FF_WRITE_TIMER_TICKS 1

// If MEMORY_FILE, then use file for memory
#ifdef MEMORY_FILE

/*************************************************************************/
// Unit globals
/*************************************************************************/
U32		FT_device_initialized = 0;

// We use one instance of the watchdog object.
FF_Watchdog					FF_watchdog;

FF_INTERRUPT_STATUS interrupt_status;
FF_INTERRUPT_STATUS busy_status;

#ifndef _WINDOWS
#ifdef THREADX
extern TX_BYTE_POOL  	System_Memory;
#else
extern NU_MEMORY_POOL  	System_Memory;
#endif
#endif



void Set_Bad_Block(Flash_Address flash_address, U32 offset);
void Set_Bad_Spots();

/*************************************************************************/
// FF_Sim_Device::Get_Capacity
// Return capacity information.
/*************************************************************************/
STATUS FF_Sim_Device::Get_Capacity(Flash_Capacity *p_flash_capacity)
{
	// Return address capacity information.
	p_flash_capacity->num_address_bits_for_block = NUM_ADDRESS_BITS_FOR_BLOCK; 
	p_flash_capacity->num_address_bits_for_sector = NUM_ADDRESS_BITS_FOR_SECTOR; 
	p_flash_capacity->num_address_bits_for_array = NUM_ADDRESS_BITS_FOR_ARRAY; 
	p_flash_capacity->num_address_bits_for_bank = NUM_ADDRESS_BITS_FOR_BANK;
	p_flash_capacity->num_address_bits_for_column = NUM_ADDRESS_BITS_FOR_COLUMN; 
	p_flash_capacity->num_address_bits_for_device = NUM_ADDRESS_BITS_FOR_DEVICE; 
	p_flash_capacity->num_address_bits_for_offset = 10; 

	return OK;
}

/*************************************************************************/
// FF_Sim_Device::Open
/*************************************************************************/
STATUS FF_Sim_Device::Open(FF_Mem *p_mem)
{
	// When simulating with memory, we must not initialize
	// the device twice; otherwise we will wipe out our memory.
	if (FT_device_initialized)
		return OK;

	FT_device_initialized = 1;

	// Initialize the interrupt service routine.
	Status status = FF_ISR_Open(p_mem, &Interrupt_Callback);
	if (status != OK)
		return status;
		
	// Initialize the watchdog routine.
	status = FF_watchdog.Open(p_mem, Flash_Address::Num_Units() );
	if (status != OK)
		return status;
		
	// We need to initialize the device before we can ask it how many
	// device blocks it has, so Flash_Address has not been initialized.
	//U32 num_bytes = Flash_Address::Num_Units() * Flash_Address::Pages_Per_Unit() * Flash_Address::Bytes_Per_Page();
	U32 num_bytes = SIM_NUM_UNITS * SIM_PAGES_PER_UNIT * SIM_BYTES_PER_PAGE;

#ifndef _WINDOWS
#ifdef THREADX
    // Allocate memory for simulating file system.
    STATUS status = tx_byte_allocate(&System_Memory, (void **)&p_mem_file, num_bytes, TX_WAIT_FOREVER);
#else
    // Allocate memory for simulating file system.
    STATUS status = NU_Allocate_Memory(&System_Memory, (void **)&p_mem_file, num_bytes, NU_NO_SUSPEND);
#endif
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Initialize_Test", 
			"Allocate memory failed",
			status,
			0);
	}
#else
	p_mem_file = (char*)FT_p_mem_file;
#endif
	p_mem_file_last = p_mem_file + num_bytes;
	
	// Set file to all erased.
	memset(p_mem_file, 0XFF, num_bytes);

	Set_Bad_Spots();
	return OK;
}

/*************************************************************************/
// Set_Bad_Spots
/*************************************************************************/
void Set_Bad_Spots()
{
#if 1 // TEMPORARY
	// Set some pages to non blank for the bad block catcher.
	Flash_Address flash_address;
	flash_address.Initialize();

#if 0
	// Make first block bad so that bad block table 
	// will have to be in some other block.
	flash_address.Block(0);
	Set_Bad_Block(flash_address, 0);
#endif

	flash_address.Block(6);
	Set_Bad_Block(flash_address, 1);

	flash_address.Block(7);
	Set_Bad_Block(flash_address, 2);

	flash_address.Block(8);
	flash_address.Sector(1);
	Set_Bad_Block(flash_address, 3);

	flash_address.Block(14); // bat block //
	flash_address.Sector(1);
	Set_Bad_Block(flash_address, 4);

	flash_address.Block(16);
	flash_address.Sector(1);
	Set_Bad_Block(flash_address, 5);

	flash_address.Block(16);
	flash_address.Sector(8);
	Set_Bad_Block(flash_address, 6);

	flash_address.Block(20);
	flash_address.Sector(11);
	Set_Bad_Block(flash_address, 7);

	flash_address.Block(20);
	flash_address.Sector(31);
	Set_Bad_Block(flash_address, 0);

	flash_address.Block(30);
	flash_address.Sector(0);
	Set_Bad_Block(flash_address, 1);

	flash_address.Block(20);
	flash_address.Sector(2);
	Set_Bad_Block(flash_address, 2);

	flash_address.Block(20);
	flash_address.Sector(4);
	Set_Bad_Block(flash_address, 3);

	flash_address.Column(1);
	flash_address.Block(14);
	flash_address.Sector(0);
	Set_Bad_Block(flash_address, 4);

	// Now bad blocks for array 1
	if (Flash_Address::Num_Arrays() == 2)
	{
		flash_address.Array(1);
		flash_address.Block(6);
		Set_Bad_Block(flash_address, 0);

		flash_address.Block(7);
		Set_Bad_Block(flash_address, 1);

		flash_address.Block(8);
		flash_address.Sector(1);
		Set_Bad_Block(flash_address, 2);

		flash_address.Block(14); // bat block //
		flash_address.Sector(1);
		Set_Bad_Block(flash_address, 3);

		flash_address.Block(16);
		flash_address.Sector(1);
		Set_Bad_Block(flash_address, 4);

		flash_address.Block(16);
		flash_address.Sector(8);
		Set_Bad_Block(flash_address, 5);

		flash_address.Block(20);
		flash_address.Sector(11);
		Set_Bad_Block(flash_address, 6);

		flash_address.Block(20);
		flash_address.Sector(31);
		Set_Bad_Block(flash_address, 7);

		flash_address.Block(30);
		flash_address.Sector(0);
		Set_Bad_Block(flash_address, 0);

		flash_address.Block(20);
		flash_address.Sector(2);
		Set_Bad_Block(flash_address, 1);

		flash_address.Block(20);
		flash_address.Sector(4);
		Set_Bad_Block(flash_address, 2);
	}

#endif

} // Set_Bad_Spots

/*************************************************************************/
// Memory_Size
// Return memory size required by device.
/*************************************************************************/
U32 FF_Sim_Device::Memory_Size(FF_CONFIG *p_config) 
{ 
	U32 memory_size = FF_ISR_Memory_Required();

	memory_size += FF_Watchdog::Memory_Size();

	memory_size += SIM_NUM_UNITS * SIM_PAGES_PER_UNIT * SIM_BYTES_PER_PAGE;

	return memory_size; 
}
/*************************************************************************/
// Set_Bad_Block
// Store non-FF byte in page to simulate the page being bad.
/*************************************************************************/
void Set_Bad_Block(Flash_Address flash_address, U32 offset)
{
	// Get array number for the next byte.
	U32 array = flash_address.Array();

	U32 column = flash_address.Column();

	U32 unit = flash_address.Unit_Index();

	// Get the device index for the next byte.
	// a concatenation of bank and device number.
	// C C C C C C C C C b b D D D
	U32 device_index = Flash_Address::Device_Index_From_Offset(offset);

	// Get the offset in the device page for the next byte.
	U32 device_column = Flash_Address::Device_Column_From_Offset(offset);

	// Get the page address for the next byte.
	U32 page_address = flash_address.Page_Index();

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
	CT_ASSERT((p_destination <= p_mem_file_last), Set_Bad_Block);

	U32 num_bytes = Flash_Address::Bytes_Per_Device_Page();

	U32 num_devices = Flash_Address::Device_Pages_Per_Page();
	char *p_device_byte;

#if 0
	for (U32 index = 0; index < num_bytes; index++)
	{
		p_device_byte = p_destination + index * num_devices;

		// Store the next byte in the destination.
		*(p_device_byte) = 0;
	}
#else
	*p_destination = 0;
#endif

} // Set_Bad_Block

//*************************************************************************/
// FF_Sim_Device::Interrupt_Callback
// This static procedure is called by the interrupt handler when
// an interrupt occurs for a device.
/*************************************************************************/
void FF_Sim_Device::Interrupt_Callback(U32 device, UI64 controller_status)
{
	FF_watchdog.Wake_Up_Waiting_Context(device, controller_status);

} // FF_Sim_Device::Interrupt_Callback

/*************************************************************************/
// Close the file.
/*************************************************************************/
STATUS Device_Close()
{
	return OK;
} // Device_Close

/*************************************************************************/
// Device_Open
// Create a file.
/*************************************************************************/
STATUS Device_Open(const char *p_file_name)
{

	for (U32 device = 0; device < FF_NUM_UNITS_MAX; device++)
	{
 		// Initialize timers 
		timer_context[device] = 0;
#ifdef THREADX
 		STATUS status = tx_timer_create(&timer[device],
 			"FbTimer", // name of timer
 			&Device_Timer_Expiration_Routine,
 			
 			// The ID passed to FF_Timer_Expiration_Routine
 			// will have the device number.
 			device,
 			1,	// zero initial ticks
 			0, // reschedule time
 			TX_NO_ACTIVATE);
#else
 		STATUS status = NU_Create_Timer(&timer[device],
 			"FbTimer", // name of timer
 			&Device_Timer_Expiration_Routine,
 			
 			// The ID passed to FF_Timer_Expiration_Routine
 			// will have the device number.
 			device,
 			0,	// zero initial ticks
 			0, // reschedule time
 			NU_DISABLE_TIMER);
#endif
 			
		if (status != OK)
		{
			CT_Log_Error(CT_ERROR_TYPE_FATAL,
				"Device_Open", 
				"NU_Create_Timer failed",
				status,
				0);
			return status;
		}

		// Reset device busy flag.
		busy_status.source &= ~FF_bit_mask[device];
	}

	// Seed the random-number generator with current time so that
	// the numbers will be different every time we run.    
	Seed_Random();

	return OK;

} // Device_Open

/*************************************************************************/
// Device_Timer_Expiration_Routine
// The ID has the controller device number.
/*************************************************************************/
void Device_Timer_Expiration_Routine(U32 id)
{
	CT_ASSERT(id < FF_NUM_UNITS_MAX, Device_Timer_Expiration_Routine);
	
	Callback_Context *p_callback_context = timer_context[id];
	if (p_callback_context == 0)
		return;
	timer_context[id] = 0;
	
	p_callback_context->Make_Ready();
	
} // Device_Timer_Expiration_Routine

/*************************************************************************/
// Create_Bad_Sector
/*************************************************************************/
STATUS Create_Bad_Sector(
	U32 device,
	UNSIGNED offset, // offset in file 
	UNSIGNED num_bytes // number of bytes 
	)
{
	char *p_data = p_mem_file + (device * Flash_Address::Pages_Per_Device() 
		* Flash_Address::Bytes_Per_Page()) + offset;
	char *p_data_last = p_data + num_bytes;
	CT_ASSERT((p_data_last <= p_mem_file_last), Create_Bad_Sector);

	memset(p_data, 0X55, num_bytes);

	return OK;

} // Create_Bad_Sector

/*************************************************************************/
// Do_Device_IO
/*************************************************************************/
STATUS Do_Device_IO(
	U32 device,
	UNSIGNED offset, // offset in file 
	void *p_buffer, // pointer to data 
	UNSIGNED num_bytes, // number of bytes 
	Callback_Context *p_callback_context, // context to run when I/O is complete
	Device_IO_Type device_IO_type
	)
{
	char *p_data = p_mem_file + (device * Flash_Address::Pages_Per_Device() 
		* Flash_Address::Bytes_Per_Page()) + offset;
	char *p_data_last = p_data + num_bytes;
	CT_ASSERT((p_data_last <= p_mem_file_last), Do_Device_IO);

	// Save context to run when timer goes off.
	CT_ASSERT((timer_context[device] == 0), Do_Device_IO);
	timer_context[device] = p_callback_context;
	U32 timer_ticks;
	U32 num_words = num_bytes/4;
	U32 *p_word_buffer = (U32 *)p_buffer;
	U32 *p_word_data = (U32 *)p_data;
	U32 index;
	STATUS verify_status = OK;

	switch(device_IO_type)
	{
	case WRITE_CELL:
		// Simulate flash write by ANDing words into buffer.
		// If the memory was not erased, it will not work correctly.
		for (index = 0; index < num_words; index++)
			*(p_word_data + index) &= *(p_word_buffer + index);
		timer_ticks = FF_WRITE_TIMER_TICKS;
 		break;

	case READ_CELL:
		memcpy(p_buffer, p_data, num_bytes);
		timer_ticks = FF_READ_TIMER_TICKS;
		break;

	case ERASE_CELL:
		memset(p_data, 0XFF, num_bytes);
		timer_ticks = FF_WRITE_TIMER_TICKS;
		break;

	case VERIFY_CELL:
		for (index = 0; index < num_words; index++)
			if (*(p_word_data + index) != *(p_word_buffer + index))
				verify_status = FF_ERROR(VERIFY);
		timer_ticks = FF_READ_TIMER_TICKS;

		// Save verify status in context.
		p_callback_context->Set_Status(verify_status);

		break;
		
	}

	STATUS status;
#if 1 // Don't use timer -- it's too slow when we read the whole file.
		// Start the timer to simulate the time for I/O.
#ifdef THREADX
 	status = tx_timer_change(&timer[device],
 		timer_ticks,
 		0); // timer only expires once
	if (status == OK)
		status = tx_timer_activate(&timer[device]);
#else
 	status = NU_Reset_Timer(&timer[device],
 		&Device_Timer_Expiration_Routine,
 		timer_ticks,
 		0, // timer only expires once
 		NU_ENABLE_TIMER);
#endif

#else // instead of the timer.
	timer_context[device] = 0;
	p_callback_context->Make_Ready();
#endif


	return status;
}

#endif // MEMORY_FILE
