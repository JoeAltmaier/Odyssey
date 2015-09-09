/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FfBat.h
// 
// Description:
// This file defines the context used by the Flash File
// Basic Assurance Test. 
// 
// 1/6/99 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FfBat_H)
#define FfBat_H

#include "Callback.h"
#include "FlashAddress.h"
#include "FlashStorageBat.h"

class FF_Page_Map;
class FF_Interface;


/*************************************************************************/
// FF_Bat_Context
// This is the callback context used by Basic Assurance Test.
/*************************************************************************/
class FF_Bat_Context : public Callback_Context
{
	// FF_Page_Map is a friend so it can access its context data
	friend FF_Page_Map;
	
	FF_Interface		*m_p_flash;
	FF_BLOCK_TEST_DATA	*m_p_block_test_data_buffer;
	FF_BLOCK_TEST_DATA	*m_p_next_block_test_data_buffer;
	Flash_Address		 m_flash_address;
	Flash_Address		 m_next_flash_address;
	U32					 m_column_range_low;
	U32					 m_column_range_high;
	U32					 m_array_range_low;
	U32					 m_array_range_high;
	U32					 m_block_range_low;
	U32					 m_block_range_high;
	U32					 m_array_number;
	U32					 m_column_number;
	U32					 m_block_number;
	U32					 m_sector_number;
	int					 m_first_page;

}; // FF_Bat_Context

#endif // FfBat_H
