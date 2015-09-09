/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FfInterface.h
// 
// Description:
// This file defines the interface object FF_Interface used by the 
// flash file system. 
// In all the interfaces, FF_HANDLE points to a flash interface object.
// 
// Update Log 
// 
// 4/12/99 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FfInterface_H)
#define FfInterface_H

#define FF_INTERFACE_COOKIE  0X74877070	// JWFF

#include "Cache.h"
#include "FfBlockAddress.h"
#include "FfCommon.h"
#include "FlashDevice.h"
#include "FfPageMap.h"
#include "FfController.h"
#include "FlashStorageBat.h"
#include "FfSgl.h"
#include "FfStats.h"
#include <String.h>

#define FF_CREATE (void **)0XFFFFFFFF
class FF_Interface {

public: // methods

	static Status Open(
		FF_CONFIG *p_flash_config,
		CM_CONFIG *p_cache_config,
		Callback_Context *p_callback_context,
		FF_HANDLE *p_flash_handle);

	Status Close(Callback_Context *p_callback_context);

	Status Read(
		FF_SGL *p_sgl, 
		U32 transfer_byte_count, 
		I64 logical_byte_address,
		void *p_context,
		FF_CALLBACK *p_completion_callback);

	Status Write(
		FF_SGL *p_sgl, 
		U32 transfer_byte_count, 
		I64 logical_byte_address,
		void *p_context,
		FF_CALLBACK *p_completion_callback);

	Status Run_Bat(
		FF_BLOCK_TEST_DATA *p_block_test_data_buffer,
		U32 size_buffer,
		Callback_Context *p_callback);

	Status Test_Unit(
		FF_BLOCK_TEST_DATA *p_block_test_data_buffer,
		U32 size_buffer,
		U32 array_range_low,
		U32 array_range_high,
		U32 column_range_low,
		U32 column_range_high,
		U32 block_range_low,
		U32 block_range_high,
		Callback_Context *p_callback);

	Status Get_Cache_Statistics(
		CM_STATISTICS *p_statistics_buffer,
		U32 size_buffer);

	Status Get_Cache_Event_Data(
		CM_EVENT_DATA *p_event_buffer,
		U32 size_buffer);

	Status Get_Cache_Config(
		CM_CONFIG *p_config_buffer,
		U32 size_buffer);

	Status Get_Event_Data(
		FF_EVENT_DATA *p_event_data_buffer, 
		U32 size_buffer);

	Status Get_Statistics(
		FF_STATISTICS *p_statistics_buffer, 
		U32 size_buffer);

	Status Format(
		FF_CONFIG *p_config,
		void *p_context,
		FF_CALLBACK *p_completion_callback);

	Status Display_Stats_And_Events();
	Status Format_Stats_And_Events(char *string);
	
	Status Display_Cache_Stats_And_Events();
	Status Format_Cache_Stats_And_Events(char *string);
	
	// Get configuration record.
	Status Get_Config(FF_CONFIG *p_config_buffer,
		U32 size_buffer);
		
	// Returns pointer to controller object for test program.
	Status Get_Controller(FF_Controller **pp_controller);

	// Returns pointer to page map object for test program.
	Status Get_Page_Map(FF_Page_Map **pp_page_map);

	// Validate handle returns pointer to FF_Interface object.
	// It returns zero if the handle is invalid.
	static Status Validate_Handle(FF_HANDLE flash_handle,
		FF_Interface **pp_handle);

	void Find_Waiting_Contexts();

private: // helper methods

	Status Initialize_Config(FF_CONFIG *p_config);

public: // member data

	// Put FF_Stats object for for alignment of I64.
	FF_Stats				 m_stats;

	FF_CONFIG				 m_flash_config;				// saved configuration data

	CM_CACHE_HANDLE			 m_cache_handle;
	U32						 m_state;
	CM_CONFIG				 m_cache_config;
	void					*m_p_cache_memory;
	U32						 m_size_cache_memory;
	void					*m_p_erased_page;
	U32						 m_cookie;

	// Page buffer only used if creating bad block table.
	void					*m_p_page_buffer;

	// Page map object
	FF_Page_Map				 m_page_map;

	// Fix alignment bug
	char					 m_pad[4];

	// Controller object
	FF_Controller			 m_controller;
	
	// Memory object
	FF_Mem					 m_mem;

	// Device object
	Flash_Device			*m_p_device;

	// Block address object
	FF_Block_Address		 m_block_address;

}; // FF_Interface

/*************************************************************************/
// Get_Config returns configuration record.
/*************************************************************************/
inline Status FF_Interface::Get_Config(FF_CONFIG *p_config_buffer,
	U32 size_buffer)
{
	memcpy(p_config_buffer, &m_flash_config, size_buffer);
	return OK;
}

/*************************************************************************/
// Get_Controller returns pointer to controller object for testing.
/*************************************************************************/
inline Status FF_Interface::Get_Controller(FF_Controller **pp_controller)
{
	*pp_controller = &m_controller;
	return OK;
}

/*************************************************************************/
// Get_Page_Map returns pointer to page map object for testing.
/*************************************************************************/
inline Status FF_Interface::Get_Page_Map(FF_Page_Map **pp_page_map)
{
	*pp_page_map = &m_page_map;
	return OK;
}

#endif // FfInterface_H