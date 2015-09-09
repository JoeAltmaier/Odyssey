/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FtFlashTest.h
// 
// Description:
// This file defines the interface to the Flash File test methods. 
// 
// $Log: /Gemini/Odyssey/FlashStorage/FlashStorageTest/FtFlashTest.h $
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
// 11/16/98 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FfFlashTest_H)
#define FfFlashTest_H

#include "Simple.h"
#include "Cache.h"
#include "FlashAddress.h"
#include "FlashDevice.h"
#include "FlashStorage.h"
#include "FlashStorageBat.h"
#include "Callback.h"

#define MEMORY_FOR_CALLBACKS 100000
#define BUFFER_SIZE 8192

#define NUM_ADDRESS_BITS_FOR_OFFSET 11
#define SIM_BYTES_PER_PAGE 2048

#define NUM_ADDRESS_BITS_FOR_BLOCK 5
#define SIM_BLOCKS_PER_UNIT 32

#define NUM_ADDRESS_BITS_FOR_COLUMN 2
#define SIM_NUM_COLUMNS 4

#define NUM_ADDRESS_BITS_FOR_ARRAY 1
#define SIM_NUM_ARRAYS 2

#define NUM_ADDRESS_BITS_FOR_SECTOR 5
#define SIM_SECTORS_PER_BLOCK 32

#define SIM_NUM_UNITS SIM_NUM_COLUMNS * SIM_NUM_ARRAYS

#define SIM_PAGES_PER_UNIT SIM_BLOCKS_PER_UNIT * SIM_SECTORS_PER_BLOCK
#define SIM_NUM_PAGES SIM_PAGES_PER_UNIT * SIM_NUM_UNITS

#define NUM_ADDRESS_BITS_FOR_DEVICE 3

#define SIM_INTERLEAVING

#ifdef SIM_INTERLEAVING
#define NUM_ADDRESS_BITS_FOR_BANK 2
#define SIM_NUM_BANKS 4	
#else

// No interleaving
#define NUM_ADDRESS_BITS_FOR_BANK 0
#define SIM_NUM_BANKS 1	
#endif

/*************************************************************************/
// FT_Test_Context
// This is the callback context used by the test methods.
/*************************************************************************/
class FT_Test_Context : public Callback_Context
{
public: // member data

    U32					 m_page_number;
    U32					 m_num_pages;
    U32					 m_block_size;
	U32					 m_percentage_erased_pages;
	U32					 m_percentage_replacement_pages;
	CM_CONFIG			*m_p_cache_config;
	void				*m_p_buffer;
	Flash_Address		 m_real_flash_address;
	Flash_Device			*m_p_device;

}; // FT_Test_Context

/*************************************************************************/
// Test methods
/*************************************************************************/
STATUS Write_Sequential(
						U32 block_size,
						U32 num_blocks,
						U32 starting_block,
						void *p_buffer,
						Callback_Context *p_callback_context);
STATUS Read_Sequential(
						U32 block_size,
						U32 num_blocks,
						U32 starting_block,
						void *p_buffer,
						Callback_Context *p_callback_context);
STATUS Format_Flash(FF_CONFIG *p_config,
						Callback_Context p_callback_context);

#ifdef _WINDOWS
STATUS Initialize_Test_Flash(WPARAM message, HWND hWnd,
							 FF_CONFIG *p_flash_config,
							 CM_CONFIG *p_cache_config);

STATUS Create_Bad_Block_Table(WPARAM message, HWND hWnd,
							 FF_CONFIG *p_flash_config,
							 CM_CONFIG *p_cache_config);
#else
STATUS Initialize_Test_Flash(
							 FF_CONFIG *p_flash_config,
							 CM_CONFIG *p_cache_config);

STATUS Create_Bad_Block_Table();
#endif

STATUS Device_Open(const char *p_file_name);
STATUS Device_Close();
void Validate();

void Format_Cache_Stats(char *string, CM_CONFIG *p_config, 
						CM_EVENT_DATA *p_event_data,
						CM_STATISTICS *p_statistics);
void Format_Flash_Stats(char *string, FF_CONFIG *p_config, 
						FF_EVENT_DATA *p_event_data,
						FF_STATISTICS *p_statistics);
void Format_Error_Stats(char *string, FF_Error_Injection *p_error_test);

// These globals are set from the error simulation dialog.
extern U32 percentage_format_errors;
extern U32 percentage_erase_errors;
extern U32 percentage_write_errors;

// These globals are set for each test.
extern U32 test_percentage_format_errors;
extern U32 test_percentage_erase_errors;
extern U32 test_percentage_write_errors;

#endif // FfFlashTest_H


