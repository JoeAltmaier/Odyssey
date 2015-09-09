/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HBC_Flash_Device.cpp
// 
// Description:
// This file implements the unit interface for the Flash File on HBC
// 
// 7/29/99 Jim Frandeen/Sudhir Kasargod
/*************************************************************************/
#include "CtPrefix.h"	// turn on _DEBUG
#include "FfCommon.h"
#include "ErrorLog.h"
#include "Simple.h"
#include "Callback.h"
#include "Trace_Index.h"
#include "nflash.h"
#include "FlashStatus.h"
#include "FlashAddress.h"
#include "FlashDevice.h"
#include "FlashStorage.h"
#include "HbcFlashDevice.h"
#define TRACE_INDEX TRACE_SSD
#include "Odyssey_Trace.h"
#include "CtEvent.h"
#include "FfSgl.h"

extern "C" int bcmp(U8 *s1, U8 *s2, int n);

// #define BREAK_ON_BUG_ADDRESS 1
extern Flash_Address bug_address;
void Break();

// Local Buffer for Verify
static U8 lbuf[NFLASH_PAGE_SIZE];
/*************************************************************************/
// HBC_Flash_Device::Get_Capacity
// This is the first unit method called.
// Return a Flash_Capacity record to specify the number of units,
// bytes per page, etc.
/*************************************************************************/
Status HBC_Flash_Device::Get_Capacity(Flash_Capacity *p_flash_capactiy)
{
	// Get the device ID to find out the capacity.
	unsigned long  device_ID;
	Status rc;
	
	
	// Initialize the Flash
	if ( (rc = nflash_init()) != OK)
		return(rc);
	
	// Get the ID of the Flash Array 0
	device_ID = nflash_get_id(0);
	if((device_ID != FF_DEVICE_ID_TOSHIBA_256) && (device_ID != FF_DEVICE_ID_SAMSUNG_128))
	{
		Tracef("\nInvalid flash device, device ID = %LX", device_ID);
		return FF_ERROR(INVALID_FLASH_DEVICE);
	}
	
	// This is a 128 or 256 MBit device
	p_flash_capactiy->num_address_bits_for_sector = 5; 
		
	// Only one array installed.
	p_flash_capactiy->num_address_bits_for_array = 0; 
		
	// 1024 blocks, or 10 bits for 128 MBit device; 
	// 256 MBit device would be 2048 blocks or 11 bits.
	p_flash_capactiy->num_address_bits_for_block = 10;
	 
	p_flash_capactiy->num_address_bits_for_bank = 0; 
	
	// Only one column
	p_flash_capactiy->num_address_bits_for_column = 0;
	
	// Only One Device
	p_flash_capactiy->num_address_bits_for_device = 2;
	
	// Page size is 2048, or 11 bits.
	p_flash_capactiy->num_address_bits_for_offset = 11; 

	// Get the ID of the Flash Array 0
	device_ID = nflash_get_id(1);
#if 1
	// array 1 was not working before, and this caused the bb table to be too small.
	if ((device_ID == FF_DEVICE_ID_TOSHIBA_256) || (device_ID == FF_DEVICE_ID_SAMSUNG_128))
	{
		// Found the Array 1, so make the number of arrays 2
		p_flash_capactiy->num_address_bits_for_array = 1; 
			
	}
#endif
	return OK;
	
} // HBC_Flash_Device::Get_Capacity



/*************************************************************************/
// HBC_Flash_Device::Erase_Page_Block
/*************************************************************************/
Status HBC_Flash_Device::Erase_Page_Block(
	Callback_Context *p_callback_context, 
	Flash_Address flash_address,
	Flash_Page_Map flash_page_map
	)
{
	
#pragma unused (flash_page_map)	
	U32 array, block, block_addr;
	STATUS rc;
	
	TRACEF( TRACE_L5, 
		(EOL "HBC_Flash_Device::Erase_Page_Block, array = %d, column = %d",
		flash_address.Array(), 
		flash_address.Column()));
	
	// Get the Array address
	array = flash_address.Array();

	// Get the block address within array
	block = flash_address.Block();
	
#if BREAK_ON_BUG_ADDRESS
	if ((array == bug_address.Array()) && (block == bug_address.Block()))
		Break();
#endif
	
	// Create the absolute block address for the NAND Flash Driver
	block_addr = (array << 10) | block;
	
	// Erase the Block
	rc = nflash_erase_block(block_addr);
	
	// set the Status to OK, the actual return value is passed in the 
	// Get_Device_Status() call
	p_callback_context->Set_Status(OK);
	
	// Since nflash_erase_block() is a blocking call, the device is ready
	p_callback_context->Make_Ready();
	return(OK);


}  // HBC_Flash_Device::Erase_Page_Block


/*************************************************************************/
// HBC_Flash_Device::Get_Device_Status
// Get status of operation
/*************************************************************************/
Status HBC_Flash_Device::Get_Device_Status(Flash_Address flash_address, 
		UI64 controller_status,
		Flash_Device_Status *p_device_status)
{
	STATUS rc;
#pragma unused (flash_address)
#pragma unused (controller_status)
	
	rc = NFlashStatus;
	p_device_status->device_status[0] = 0;
	p_device_status->device_status[1] = 0;
	p_device_status->device_status[2] = 0;
	p_device_status->device_status[3] = 0;
	
	// Release the Semaphore aquired by erase/read/write driver calls
	nflash_release_sema();

	return(rc);

} // HBC_Flash_Device::Get_Device_Status

/*************************************************************************/
// HBC_Flash_Device::Open
/*************************************************************************/
Status HBC_Flash_Device::Open(FF_Mem *p_mem)
{
#pragma unused (p_mem)
	// We dont have to do anything Here
	return(OK);
	
} // HBC_Flash_Device::Open

/*************************************************************************/
// HBC_Flash_Device::Close
/*************************************************************************/
Status HBC_Flash_Device::Close()
{
	// We dont have to do anything Here
	return(OK);
	
} // HBC_Flash_Device::Close

/*************************************************************************/
// We dont have Interrupt for Flash Device on HBC
/*************************************************************************/
void HBC_Flash_Device::Interrupt_Callback(U32 unit_index)
{
#pragma unused (unit_index)

	;
} // Interrupt_Callback


/*************************************************************************/
// HBC_Flash_Device::Read_Page
// Read data into the flash buffer, 
/*************************************************************************/
Status HBC_Flash_Device::Read_Page(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address, 
		Flash_Page_Map flash_page_map,
		FF_SGL *p_sgl)
{
 	
#pragma unused (flash_page_map)
	U32 array, block,sector, sectornum;
	STATUS rc;

	TRACEF( TRACE_L5,
		(EOL "HBC_Flash_Device::Read_Page, array = %d, block = %d, sector = %d",
		flash_address.Array(),  
		flash_address.Block(),
		flash_address.Sector()));

	// Get buffer pointer from first element of SGL.
	void *p_data_buffer = (void *)p_sgl->Address(0);
	
#if BREAK_ON_BUG_ADDRESS
	if (flash_address == bug_address)
		Break();
#endif
	
	// Get the Array Address
	array = flash_address.Array();

	// Get the Block Address
	block = flash_address.Block();

	// Get the Sector Address
	sector= flash_address.Sector();
	
	// Create the Address to pass to the Driver
	sectornum = (array << 15) | (block << 5) | sector;
	
	// Read the Page
	rc = nflash_read_page((U8 *)p_data_buffer, sectornum);

	// set the Status to OK, the actual return value is passed in the 
	// Get_Device_Status() call
	p_callback_context->Set_Status(OK);

	// Since nflash_read_page() is a blocking call, the device is ready
	p_callback_context->Make_Ready();
	return(OK);
}  // HBC_Flash_Device::Read_Page

/*************************************************************************/
// HBC_Flash_Device::Verify_Page
// verify the data by comparing it to the data in the buffer.
// Verify flags are not used for HBC flash because there is no hardware
// verify operation.
/*************************************************************************/
Status HBC_Flash_Device::Verify_Page(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address, 
		Flash_Page_Map flash_page_map,
		FF_SGL *p_sgl,
		U32 /* flags */)
{
#pragma unused (flash_page_map)

	U32 array, block,sector, sectornum;
	STATUS rc;

	TRACEF( TRACE_L5,
		(EOL "HBC_Flash_Device::Verify_Page, array = %d, block= %d, sector= %d",
		flash_address.Array(),  
		flash_address.Block(),
		flash_address.Sector()));
		
	// Get buffer pointer from first element of SGL.
	void *p_data_buffer = (void *)p_sgl->Address(0);
	
#if BREAK_ON_BUG_ADDRESS
	if (flash_address == bug_address)
		Break();
#endif
	
	// Get the Array Address
	array = flash_address.Array();

	// Get the Block Address
	block = flash_address.Block();

	// Get the Sector Address
	sector= flash_address.Sector();
	
	// Create the Address to pass to the Driver
	sectornum = (array << 15) | (block << 5) | sector;
	
	// Read the Flash int to Local Buffer
	rc = nflash_read_page(lbuf, sectornum);
	
	// Compare the Buffer
	if ( bcmp(lbuf, (U8 *)p_data_buffer, NFLASH_PAGE_SIZE) != 0)
		rc = FF_ERROR(VERIFY);
	else
		rc = OK;

	// Return the Compare Status, the return value of the read status
	// is passed in the  Get_Device_Status() call
	p_callback_context->Set_Status(rc);

	// Since nflash_read_page() is a blocking call, the device is ready
	p_callback_context->Make_Ready();
	return(OK);
	
	

}  // HBC_Flash_Device::Verify_Page
	
/*************************************************************************/
// HBC_Flash_Device::Read_DMA_Complete
// The DMA of data from FPGA to buffer has completed.
/*************************************************************************/
void HBC_Flash_Device::Read_DMA_Complete(void *p_context, Status status)
{
	Callback_Context *p_callback_context = (Callback_Context *)p_context;
	p_callback_context->Make_Ready();
	p_callback_context->Set_Status(status);
}

/*************************************************************************/
// HBC_Flash_Device::Write_Page
// Write page from flash buffer
/*************************************************************************/
Status HBC_Flash_Device::Write_Page(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address, 
		Flash_Page_Map flash_page_map,
		FF_SGL *p_sgl)
{
#pragma unused (flash_page_map)
	U32 array, block,sector, sectornum;
	STATUS rc;
	
	TRACEF( TRACE_L5,
		(EOL "HBC_Flash_Device::Write_Page, array = %d, block = %d, sector= %d",
		flash_address.Array(),  
		flash_address.Block(),
		flash_address.Sector()));

	// Get buffer pointer from first element of SGL.
	void *p_data_buffer = (void *)p_sgl->Address(0);
	
#if BREAK_ON_BUG_ADDRESS
	if (flash_address == bug_address)
		Break();
#endif
	
	// Get the Array Address
	array = flash_address.Array();

	// Get the Block Address
	block = flash_address.Block();

	// Get the Sector Address
	sector= flash_address.Sector();
	
	// Create the Address to pass to the Driver
	sectornum = (array << 15) | (block << 5) | sector;
	
	// Write the buffer into the Page
	rc = nflash_write_page((U8 *)p_data_buffer, sectornum);

	// set the Status to OK, the actual return value is passed in the 
	// Get_Device_Status() call
	p_callback_context->Set_Status(OK);

	// Since nflash_write_page() is a blocking call, the device is ready
	p_callback_context->Make_Ready();
	return(OK);
	

}  // HBC_Flash_Device::Write_Page

/*************************************************************************/
// HBC_Flash_Device::Write_DMA_Complete
// The DMA of data from buffer to FPGA to buffer has completed.
/*************************************************************************/
void HBC_Flash_Device::Write_DMA_Complete(void *p_context, Status status)
{
	// Schedule the caller's context to run.
	Callback_Context *p_callback_context = (Callback_Context *)p_context;
	p_callback_context->Make_Ready();
	p_callback_context->Set_Status(status);
	
} // Write_DMA_Complete





