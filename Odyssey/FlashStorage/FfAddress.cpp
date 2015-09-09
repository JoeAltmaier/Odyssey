/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfAddress.cpp
// 
// Description:
// This file defines the addressing used by the Flash File.
// The number of flash units, the number of sectors per page, the number
// of bytes per page, and so forth all depend on the type of flash units
// present.
// 
// 2/16/99 Jim Frandeen: Create file
/*************************************************************************/

#include "FlashDevice.h"
#include "FlashStatus.h"
#include "FfCommon.h"

/*************************************************************************/
// Static member variables.
/*************************************************************************/
U32	Flash_Address::m_num_address_bits_for_array;
U32	Flash_Address::m_num_address_bits_for_bank;
U32	Flash_Address::m_num_address_bits_for_block;
U32	Flash_Address::m_num_address_bits_for_column;
U32	Flash_Address::m_num_address_bits_for_device;
U32	Flash_Address::m_num_address_bits_for_device_index;
U32	Flash_Address::m_num_address_bits_for_offset;
U32	Flash_Address::m_num_address_bits_for_page;
U32	Flash_Address::m_num_address_bits_for_sector;
U32	Flash_Address::m_num_address_bits_for_unit;
U32	Flash_Address::m_num_address_bits_for_virtual_page;

U32	Flash_Address::m_shift_array;
U32	Flash_Address::m_shift_array_index;
U32	Flash_Address::m_shift_block;
U32	Flash_Address::m_shift_block_index;
U32	Flash_Address::m_shift_device_column;
U32	Flash_Address::m_shift_sector;
U32	Flash_Address::m_shift_virtual_page;

U32	Flash_Address::m_banks_per_array;
U32	Flash_Address::m_blocks_per_device;
UI64 Flash_Address::m_bytes_per_array;
UI64 Flash_Address::m_bytes_per_bank;
UI64 Flash_Address::m_bytes_per_column;
U32	Flash_Address::m_bytes_per_device_page;
U32	Flash_Address::m_bytes_per_page;
U32	Flash_Address::m_columns_per_array;
U32	Flash_Address::m_device_indexes_per_page;
U32	Flash_Address::m_devices_per_page;
U32	Flash_Address::m_dwords_per_page;
U32	Flash_Address::m_num_arrays;
U32	Flash_Address::m_num_blocks;
U32	Flash_Address::m_num_units;
U32	Flash_Address::m_num_virtual_pages;
U32	Flash_Address::m_pages_per_device;
U32	Flash_Address::m_sectors_per_block;

U32	Flash_Address::m_mask_array;
U32	Flash_Address::m_mask_array_index;
U32	Flash_Address::m_mask_bank;
U32	Flash_Address::m_mask_bank_index;
U32	Flash_Address::m_mask_block;
U32	Flash_Address::m_mask_block_index;
U32	Flash_Address::m_mask_column;
U32	Flash_Address::m_mask_column_index;
U32	Flash_Address::m_mask_device;
U32	Flash_Address::m_mask_device_index;
U32	Flash_Address::m_mask_offset;
UI64 Flash_Address::m_mask_virtual_page;
U32	Flash_Address::m_mask_sector;

/*************************************************************************/
// FfAddress::Initialize
// Called by FF_Interface::Initialize after it calls 
// p_device->Get_Capacity
/*************************************************************************/
Status Flash_Address::Initialize(Flash_Capacity *p_flash_capacity)
{
			
	// Our address would look like the following if we have two arrays,
	// 1,024 blocks per unit, and 16 sectors per block:
	// bbbbbbbbbbSSSSACCCDDD
	// bbbbbbbbbbSSSSUUUUDDD
	// b = block
	// S = sector
	// A = Array
	// C = Column
	// U = Unit, combination of array, column
	// D = Device


	// The number of arrays depends on the number of
	// address bits allocated for the array.
	m_num_address_bits_for_array = p_flash_capacity->num_address_bits_for_array;
	m_num_arrays = (1 << m_num_address_bits_for_array);

	// The number of columns per array depends on the number of
	// address bits allocated for the column.
	m_num_address_bits_for_column = p_flash_capacity->num_address_bits_for_column;
	m_columns_per_array = (1 << m_num_address_bits_for_column);

	// The number of blocks per device depends on the number of
	// address bits allocated for the block.
	m_num_address_bits_for_block = p_flash_capacity->num_address_bits_for_block;
	m_blocks_per_device = (1 << m_num_address_bits_for_block);

	// The number of sectors per block depends on the number of
	// address bits allocated for the sector.
	m_num_address_bits_for_sector = p_flash_capacity->num_address_bits_for_sector;
	m_sectors_per_block = (1 << m_num_address_bits_for_sector);

	// Calculate the number of bits used to address the unit.
	// This depends on the number of bits used for array, column.
	m_num_address_bits_for_unit = m_num_address_bits_for_array
		+ m_num_address_bits_for_column;

	// The number of units depends on the number of bits allocated for
	// array, column.
	m_num_units = (1 << m_num_address_bits_for_unit);

	// The total number of blocks depends on the number of units
	// and the number of blocks per device.
	m_num_blocks = m_blocks_per_device * m_num_units;

	// The number of bytes per page depends on the number of bits
	// used for the offset.
	m_num_address_bits_for_offset = p_flash_capacity->num_address_bits_for_offset;

	m_bytes_per_page = (1 << m_num_address_bits_for_offset);
	m_dwords_per_page = m_bytes_per_page / 8;

	// With interleaving,
	// The page offset is used to map the bank number and device number as follows:

	// 1 1 1 1 0 0 0 0 0 0 0 0 0 0
	// 3 2 1 0 9 8 7 6 5 4 3 2 1 0
	// O O O O O O O O O b b D D D

	// D	Device number 
	// b	Bank number 

	// Bits allocated for the flash device are within the
	// bits allocated for the offset when interleaving is available for the device.
	m_num_address_bits_for_device = p_flash_capacity->num_address_bits_for_device;

	// The number of address bits for bank is included in the number of bits
	// for offset.  This is only used when interleaving is available for the device.
	m_num_address_bits_for_bank = p_flash_capacity->num_address_bits_for_bank;
	m_banks_per_array = (1 << (m_num_address_bits_for_bank));

	// The number of devices per page depends on the number of bits allocated for
	// flash device.  This does not include the number of bits allocated for bank.  
	m_devices_per_page = (1 << m_num_address_bits_for_device);

	// Device index includes the device and the bank. 
	m_num_address_bits_for_device_index = 
		m_num_address_bits_for_device + m_num_address_bits_for_bank;

	// The number of device pages per page depends on the number of bits allocated for
	// flash device plus the number of bits allocated for bank. 
	m_device_indexes_per_page = (1 << m_num_address_bits_for_device_index);
	m_shift_device_column = m_num_address_bits_for_device_index;

	// The number of bytes per device page is normally 512.
	// For simulation, this may be less.
	m_bytes_per_device_page = (1 << (m_num_address_bits_for_offset - m_num_address_bits_for_device_index));

	// Calculate the number of bits used for the virtual page number.
	m_num_address_bits_for_virtual_page = 
		m_num_address_bits_for_array
		+ m_num_address_bits_for_column
		+ m_num_address_bits_for_block
		+ m_num_address_bits_for_sector;
	
	// The number of virtual pages depends on the number of bits 
	// used for the virtual page number.
	m_num_virtual_pages = (1 << m_num_address_bits_for_virtual_page);

	// The number of bits in the page determines
	// the number of pages per device.
	m_num_address_bits_for_page = m_num_address_bits_for_block 
		+ m_num_address_bits_for_sector;
	m_pages_per_device = (1 << m_num_address_bits_for_page);
	
	m_shift_array = m_num_address_bits_for_column;
	m_shift_array_index = m_num_address_bits_for_bank + m_num_address_bits_for_block;
	m_shift_sector = m_num_address_bits_for_unit;
	m_shift_block = m_shift_sector + m_num_address_bits_for_sector;
	m_shift_virtual_page = m_num_address_bits_for_offset;

	m_bytes_per_column = m_bytes_per_page * m_pages_per_device;
	m_bytes_per_array = m_bytes_per_column * m_columns_per_array;
	m_bytes_per_bank = m_bytes_per_array / m_banks_per_array;
	
	if (m_num_arrays)
	{
		m_mask_array = (m_num_arrays - 1) << m_shift_array;
		m_mask_array_index = (m_num_arrays - 1) << m_shift_array_index;
	}
	else
	{
		m_mask_array = 0;
		m_mask_array_index = 0;
	}

	if (m_devices_per_page)
	{
		m_mask_device = m_devices_per_page - 1;
		m_mask_device_index = (m_device_indexes_per_page - 1);
	}
	else
	{
		m_mask_device = 0;
		m_mask_device_index = 0;
	}

	m_mask_block = (m_blocks_per_device - 1) << m_shift_block;
	m_shift_block_index = m_num_address_bits_for_bank;
	m_mask_block_index = (m_blocks_per_device - 1) << m_shift_block_index;
	m_mask_column = m_columns_per_array - 1;
	m_mask_column_index = (m_columns_per_array - 1) << m_num_address_bits_for_device;
	m_mask_sector = (m_sectors_per_block - 1) << m_shift_sector;

	m_mask_offset = m_bytes_per_page - 1;
	m_mask_virtual_page = (m_num_virtual_pages - 1) << m_shift_virtual_page;

	if (m_banks_per_array)
	{
		// O O O O O O O O O b b D D D
		m_mask_bank = (m_banks_per_array - 1) << m_num_address_bits_for_device;
		m_mask_bank_index = m_banks_per_array - 1;
	}
	else
	{
		m_mask_bank = 0;
		m_mask_bank_index = 0;
	}

	return OK;

} // Flash_Address::Initialize_Capacity

// TEMPORARY inlines here so we can step into them with Metrowerks

/*************************************************************************/
// constructor from address index sets up all fields
/*************************************************************************/
/* inline */ Flash_Address::Flash_Address(U32 index)  
{
	// Create object from real flash address
	// Our real flash address would look like the following if we have two arrays,
	// 1,024 blocks per unit, and 16 sectors per block:
	// bbbbbbbbbbSSSSACCC
	// bbbbbbbbbbSSSSUUUU
	// b = block
	// S = sector
	// A = Array
	// C = Column
	// U = Unit, combination of array, column
	addr.array = (index & m_mask_array) >> m_shift_array;
	addr.bank = 0;
	addr.column = index & m_mask_column;
	addr.block = (index & m_mask_block) >> m_shift_block;
	addr.sector = (index & m_mask_sector) >> m_shift_sector;
}

/*************************************************************************/
// Validate
// Validate each field of the flash address.
/*************************************************************************/
Status Flash_Address::Validate(Flash_Address flash_address)  
{
	if (flash_address.addr.array > m_num_arrays)
		return FF_ERROR(BAD_FLASH_ADDRESS);
		
	if (flash_address.addr.bank > m_banks_per_array)
		return FF_ERROR(BAD_FLASH_ADDRESS);
		
	if (flash_address.addr.column > m_columns_per_array)
		return FF_ERROR(BAD_FLASH_ADDRESS);
		
	if (flash_address.addr.block > m_blocks_per_device)
		return FF_ERROR(BAD_FLASH_ADDRESS);
		
	if (flash_address.addr.sector > m_sectors_per_block)
		return FF_ERROR(BAD_FLASH_ADDRESS);
		
	return OK;
	
} // Validate


