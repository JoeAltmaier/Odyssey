/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfInterface.cpp
// 
// Description:
// This file implements the interface to the flash file system. 
// Each method converts a flash handle into a pointer to
// a flash object and then calls the corresponding flash method.
// It was written this was so that it could be called from a "C" interface.
// 
// 4/12/99 Jim Frandeen: Create file
/*************************************************************************/

#include "Align.h"
#include "FfInterface.h"

/*************************************************************************/
// FF_Create
// Create the bad block table.
/*************************************************************************/
Status FF_Create(
	FF_CONFIG *p_flash_config,
	CM_CONFIG *p_cache_config,
	Callback_Context *p_callback_context)
{
	return FF_Interface::Open(
		p_flash_config,
		p_cache_config,
		p_callback_context,
		FF_CREATE);

} // FF_Create


/*************************************************************************/
// FF_Close
// Close everything
/*************************************************************************/
Status FF_Close(FF_HANDLE flash_handle,
				Callback_Context *p_callback_context)
{
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
		return status;

	return p_flash->Close(p_callback_context);

} // FF_Close

/*************************************************************************/
// FF_Display_Cache_Stats_And_Events.
// Display the cache stats and event data.
/*************************************************************************/
Status FF_Display_Cache_Stats_And_Events(
	FF_HANDLE flash_handle)
{	
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
		return status;

	return p_flash->Display_Cache_Stats_And_Events();

} // FF_Display_Cache_Stats_And_Events

/*************************************************************************/
// FF_Display_Stats_And_Events.
// Create a string that contains the flash stats and event data.
/*************************************************************************/
Status FF_Display_Stats_And_Events(
	FF_HANDLE flash_handle)
{	
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
		return status;

	return p_flash->Display_Stats_And_Events();

} // FF_Display_Stats_And_Events

/*************************************************************************/
// FF_Format is the external entry point to format the flash file.
/*************************************************************************/
Status FF_Format(
	FF_HANDLE flash_handle,
	FF_CONFIG *p_config,
	void *p_context,
	FF_CALLBACK *p_completion_callback)
{	
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
		return status;

	return p_flash->Format(p_config,
		p_context,
		p_completion_callback);

} // FF_Format

/*************************************************************************/
// FF_Format_Cache_Stats_And_Events.
// Create a string that contains the cache stats and event data.
/*************************************************************************/
Status FF_Format_Cache_Stats_And_Events(
	FF_HANDLE flash_handle, char *p_string)
{	
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
		return status;

	return p_flash->Format_Cache_Stats_And_Events(p_string);

} // FF_Format_Cache_Stats_And_Events

/*************************************************************************/
// FF_Format_Stats_And_Events.
// Create a string that contains the flash stats and event data.
/*************************************************************************/
Status FF_Format_Stats_And_Events(
	FF_HANDLE flash_handle, char *p_string)
{	
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
		return status;

	return p_flash->Format_Stats_And_Events(p_string);

} // FF_Format_Stats_And_Events

/*************************************************************************/
// FF_Get_Cache_Statistics
// Get CM_STATISTICS record
/*************************************************************************/
Status FF_Get_Cache_Statistics(
	FF_HANDLE flash_handle,
	CM_STATISTICS *p_statistics_buffer,
	U32 size_buffer)
{
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
	{
		// Even if state is closing, allow statistics for debugging.
		if (status != FF_ERROR_CODE(INVALID_HANDLE))
			return status;
	}

	return p_flash->Get_Cache_Statistics(
		p_statistics_buffer, size_buffer);

} // FF_Get_Cache_Statistics

/*************************************************************************/
// FF_Get_Cache_Config
// Get CM_Config record
/*************************************************************************/
Status FF_Get_Cache_Config(
	FF_HANDLE flash_handle,
	CM_CONFIG *p_config_buffer,
	U32 size_buffer)
{
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
	{
		// Even if state is closing, allow statistics for debugging.
		if (status != FF_ERROR_CODE(INVALID_HANDLE))
			return status;
	}

	return p_flash->Get_Cache_Config(  
		p_config_buffer, size_buffer);

} // FF_Get_Cache_Config


/*************************************************************************/
// FF_Get_Cache_Event_Data
// Get CM_EVENT_DATA record
/*************************************************************************/
Status FF_Get_Cache_Event_Data(
	FF_HANDLE flash_handle,
	CM_EVENT_DATA *p_event_buffer,
	U32 size_buffer)
{
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
	{
		// Even if state is closing, allow statistics for debugging.
		if (status != FF_ERROR_CODE(INVALID_HANDLE))
			return status;
	}


	return p_flash->Get_Cache_Event_Data(
		p_event_buffer, size_buffer);

} // FF_Get_Cache_Event_Data

/*************************************************************************/
// FF_Get_Config
// Get Flash config record
/*************************************************************************/
Status FF_Get_Config(
	FF_HANDLE flash_handle,
	FF_CONFIG *p_config_buffer,
	U32 size_buffer)
{
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
	{
		// Even if state is closing, allow statistics for debugging.
		if (status != FF_ERROR_CODE(INVALID_HANDLE))
			return status;
	}

	return p_flash->Get_Config(  
		p_config_buffer, size_buffer);

} // FF_Get_Cache_Config

/*************************************************************************/
// FF_Get_Controller
// For testing, returns pointer to controller object.
/*************************************************************************/
Status FF_Get_Controller(FF_HANDLE flash_handle, FF_Controller **pp_controller)
{
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
		return status;

	p_flash->Get_Controller(pp_controller);
	return OK;
}

/*************************************************************************/
// FF_Get_Event_Data
/*************************************************************************/
Status FF_Get_Event_Data(
	FF_HANDLE flash_handle,
	FF_EVENT_DATA *p_event_data_buffer, 
	U32 size_buffer) 
{
#ifndef _WINDOWS
#ifndef GREEN_HILLS
	CT_ASSERT(IS_ALIGNED_8(p_event_data_buffer), FF_Get_Event_Data);
#endif
#endif

	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
	{
		// Even if state is closing, allow statistics for debugging.
		if (status != FF_ERROR_CODE(INVALID_HANDLE))
			return status;
	}

	p_flash->m_stats.Get_Event_Data(p_event_data_buffer, size_buffer);
	return OK;

} // FF_Get_Event_Data

/*************************************************************************/
// FF_Get_Memory_Size_Required
// Get memory size required for internal tables.
// The user calls this method in order to find out how much 
// memory to allocate.
/*************************************************************************/
Status FF_Get_Memory_Size_Required(FF_CONFIG *p_config, U32 *p_memory_size)
{
	U32 memory_size;

	// Check version of FF_CONFIG record.
	if (p_config->version != FF_CONFIG_VERSION)
		return FF_ERROR(INVALID_CONFIG_VERSION);
	
	// Get the capacity information.
	// Find out how many flash arrays, banks, columns are installed.
	Flash_Capacity flash_capacity;
	Status status = p_config->p_device->Get_Capacity(&flash_capacity);
	if (status != OK)
		return status;

	// Initialize the flash capacity information.
	// This depends on how much flash is installed.
	status = Flash_Address::Initialize(&flash_capacity);
	if (status != OK)
		return status;
	
	// Get memory required by device.
	memory_size = p_config->p_device->Memory_Size(p_config);

	// Get size of block address object
	memory_size += FF_Block_Address::Memory_Size(p_config);

	// Allocate a zero page to verify erased pages.
	memory_size += Flash_Address::Bytes_Per_Page();

	// Allocate a page buffer to check for blank pages.
	memory_size += Flash_Address::Bytes_Per_Page();

	// Allocate memory for page map object.
	memory_size += FF_Page_Map::Memory_Size(p_config);

	// Return memory size required to user.
	*p_memory_size = memory_size;

	return OK;

} // FF_Get_Memory_Size_Required

/*************************************************************************/
// FF_Get_Page_Map
// For testing, returns pointer to page map object.
/*************************************************************************/
Status FF_Get_Page_Map(FF_HANDLE flash_handle, FF_Page_Map **pp_page_map)
{
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
		return status;

	p_flash->Get_Page_Map(pp_page_map);
	return OK;
}

/*************************************************************************/
// FF_Get_Statistics
/*************************************************************************/
Status FF_Get_Statistics(
	FF_HANDLE flash_handle,
	FF_STATISTICS *p_statistics_buffer, 
	U32 size_buffer) 
{
#ifndef _WINDOWS
#ifndef GREEN_HILLS
	CT_ASSERT(IS_ALIGNED_8(p_statistics_buffer), FF_Get_Statistics);
#endif
#endif

	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
	{
		// Even if state is closing, allow statistics for debugging.
		if (status != FF_ERROR_CODE(INVALID_HANDLE))
			return status;
	}

	// Break on each waiting context for debugging.
	p_flash->Find_Waiting_Contexts();

	p_flash->Get_Statistics(p_statistics_buffer, size_buffer);

	return OK;

} // FF_Get_Statistics

/*************************************************************************/
// FF_Open
// Initialize the flash object 
/*************************************************************************/
Status FF_Open(
	FF_CONFIG *p_flash_config,
	CM_CONFIG *p_cache_config,
	Callback_Context *p_callback_context,
	FF_HANDLE *p_flash_handle)
{
	// Initialize handle.
	*p_flash_handle = 0;

	return FF_Interface::Open(
		p_flash_config,
		p_cache_config,
		p_callback_context,
		p_flash_handle);

} // FF_Open

/*************************************************************************/
// FF_Read is the external entry point to read data from the flash file.
// Read transfer_byte_count from logical_byte_address into specified buffer.
// Call callback routine with pointer to context when read has completed.
/*************************************************************************/
Status FF_Read(
	FF_HANDLE flash_handle,
	void *p_buffer, 
	U32 transfer_byte_count, 
	I64 logical_byte_address,
	void *p_context,
	FF_CALLBACK *p_completion_callback)
{
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
		return status;

	// Create SGL from pb/cb
	FF_SGL sgl(p_buffer, transfer_byte_count);

	return p_flash->Read(&sgl,
		transfer_byte_count,
		logical_byte_address,
		p_context,
		p_completion_callback);

} // FF_Read

/*************************************************************************/
// FF_Read is the external entry point to read data from the flash file.
// Read transfer_byte_count from logical_byte_address into specified SGL.
/*************************************************************************/
Status FF_Read(
	FF_HANDLE flash_handle,
	SGE_SIMPLE_ELEMENT	*p_element, 
	U32 transfer_byte_count, 
	I64 logical_byte_address,
	void *p_context,
	FF_CALLBACK *p_completion_callback)
{
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
		return status;

	// Create SGL from pointer to SGE_SIMPLE_ELEMENT
	FF_SGL sgl(p_element);

	return p_flash->Read(&sgl,
		transfer_byte_count,
		logical_byte_address,
		p_context,
		p_completion_callback);

} // FF_Read

/*************************************************************************/
// FF_Run_Bat is the external entry point to run
// the basic assurance test.
/*************************************************************************/
Status FF_Run_Bat(
	FF_HANDLE flash_handle,
	FF_BLOCK_TEST_DATA *p_block_test_data_buffer,
	U32 size_buffer,
	Callback_Context *p_callback)
{
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
		return status;

	return p_flash->Run_Bat(p_block_test_data_buffer,
		size_buffer,
		p_callback);

} // FF_Run_Bat

/*************************************************************************/
// FF_Test_Device is the external entry point to run
// the surface test on the entire unit.
// NOTE: This will destroy the contents of the unit!
/*************************************************************************/
Status FF_Test_Device(
	FF_HANDLE flash_handle,
	FF_BLOCK_TEST_DATA *p_block_test_data_buffer,
	U32 size_buffer,
	U32 array_range_low,
	U32 array_range_high,
	U32 column_range_low,
	U32 column_range_high,
	U32 block_range_low,
	U32 block_range_high,
	Callback_Context *p_callback)
{
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
		return status;

	return p_flash->Test_Unit(
		p_block_test_data_buffer,
		size_buffer,
		array_range_low,
		array_range_high,
		column_range_low,
		column_range_high,
		block_range_low,
		block_range_high,
		p_callback);

} // FF_Test_Device

/*************************************************************************/
// FF_Validate
// For debugging, validate internal structures.
/*************************************************************************/
Status FF_Validate(FF_HANDLE flash_handle)
{
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
		return status;

	p_flash->m_page_map.VALIDATE_MAP;
	return OK;
}

/*************************************************************************/
// FF_Write is the external entry point to write data to the flash file.
// Write transfer_byte_count bytes from specified buffer
// into logical_byte_address of the file.
// Call callback routine with pointer to context when write has completed.
/*************************************************************************/
Status FF_Write(
	FF_HANDLE flash_handle,
	void *p_buffer, 
	U32 transfer_byte_count, 
	I64 logical_byte_address,
	void *p_context,
	FF_CALLBACK *p_completion_callback)
{
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
		return status;

	// Create SGL from pb/cb
	FF_SGL sgl(p_buffer, transfer_byte_count);

	return p_flash->Write(&sgl,
		transfer_byte_count,
		logical_byte_address,
		p_context,
		p_completion_callback);

} // FF_Write

/*************************************************************************/
// FF_Write is the external entry point to write data to the flash file.
// Write transfer_byte_count bytes from specified SGL
// into logical_byte_address of the file.
/*************************************************************************/
Status FF_Write(
	FF_HANDLE flash_handle,
	SGE_SIMPLE_ELEMENT	*p_element, 
	U32 transfer_byte_count, 
	I64 logical_byte_address,
	void *p_context,
	FF_CALLBACK *p_completion_callback)
{
	FF_Interface *p_flash;
	Status status = FF_Interface::Validate_Handle(flash_handle, &p_flash);
	if (status != OK)
		return status;

	// Create SGL from pointer to SGE_SIMPLE_ELEMENT
	FF_SGL sgl(p_element);

	return p_flash->Write(&sgl,
		transfer_byte_count,
		logical_byte_address,
		p_context,
		p_completion_callback);

} // FF_Write

// TEMPORARY move inlines here because Metrowerks won't step into them.
/*************************************************************************/
// FF_Interface::Validate_Handle
/*************************************************************************/
/* inline */ Status FF_Interface::Validate_Handle(FF_HANDLE flash_handle,
									 FF_Interface **pp_handle)
{
	FF_Interface *p_flash = (FF_Interface *)flash_handle;

	// Return handle so user can get statistics.
	*pp_handle = (FF_Interface *)flash_handle;

	if (p_flash == 0)
		return FF_ERROR(INVALID_HANDLE);

	if (p_flash->m_cookie != FF_INTERFACE_COOKIE)
		return FF_ERROR(INVALID_HANDLE);

	if (p_flash->m_state != FF_STATE_OPEN)
		return FF_ERROR(NOT_OPEN);

	return OK;

} // Validate_Handle

