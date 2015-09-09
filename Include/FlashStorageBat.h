/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FlashStorageBat.h
// 
// Description:
// This file defines the external interfaces to the Flash Storage 
// Basic Assurance Test.
//
// Under compile-time option, the Flash Storage can be configured to reserve
// one block (16 pages) in each cell block for basic assurance testing.
// The Basic Assurance Test can erase, write, and read the bat block
// pages without interfering with the normal flash storage operations.
// 

// 1/6/99 Jim Frandeen: Create file
// 5/24/99 Jim Frandeen: Create file from FlashFileBat.h to implement
//			interleaving.
/*************************************************************************/

#if !defined(Flash_Storage_Bat_H)
#define Flash_Storage_Bat_H

#include "FlashStorage.h"

class FF_Controller;
class FF_Page_Map;

/*************************************************************************/
// FF_BLOCK_TEST_DATA
// defines event record returned by FF_Get_Event_Data.
// This describes information indicating errors, events, 
// and possibly health conditions. 
/*************************************************************************/

typedef struct {

	// Write sector test results.
	// We write a test pattern to each sector in the block.
	// If the write is successful, we set the bit for the corresponding sector.
	// If the write is not successful, we reset the bit.
	U32		write_sector_success_map;
   
	// Read sector test results.
	// We read a test pattern to each sector in the block.
	// If the read is successful, we set the bit for the corresponding sector.
	// If the read is not successful, we reset the bit.
	U32		read_sector_success_map;
     
	// Verify sector test results.
	// We verify a test pattern to each sector in the block.
	// If the verify is successful, we set the bit for the corresponding sector.
	// If the verify is not successful, we reset the bit.
	U32		verify_sector_success_map;
     
}  FF_BLOCK_TEST_DATA;



// Run Basic Assurance Test and return bat data.
Status FF_Run_Bat(
	FF_HANDLE flash_handle,
	FF_BLOCK_TEST_DATA *p_block_test_data_buffer,
	U32 size_buffer,
	Callback_Context *p_callback);

// Test the entire unit
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
	Callback_Context *p_callback);

Status FF_Get_Controller(FF_HANDLE flash_handle, FF_Controller **pp_controller);
Status FF_Get_Page_Map(FF_HANDLE flash_handle, FF_Page_Map **pp_page_map);

#endif /* Flash_Storage_Bat_H  */
