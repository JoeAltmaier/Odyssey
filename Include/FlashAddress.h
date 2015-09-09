/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FlashAddress.h
// 
// Description:
// This file defines Flash_Address object.
// This 32-bit object completely defines the real address of a flash page.
// 
// 6/14/99 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FlashAddress_H)
#define FlashAddress_H

#include "Simple.h"
#include "ErrorLog.h"

class Flash_Capacity;

/*************************************************************************/
// Flash_Address
/*************************************************************************/
class Flash_Address
{
public:

	Flash_Address();

	Flash_Address(U32 real_flash_address);

	// Get/Set array
	U32  Array();
	void Array(U32 array);

	// Bad block indexes
	// We would like to arrange our bad block 
	// table so that, when it is written to flash, all the entries for card A 
	// will go to card A, and all the entries for card B will go to card B.  
	// We can accomplish this by simply arranging our bitmap as follows.  Let

	//  ABBBBBBBBBBBbb

	// Be an index into a table of doublewords, and let

	// CCCDDD be an index into the doubleword.

	// A = Array, B = Block number, C = Column, b = bank number.
	U32 Bad_Block_Bit_Index();
	void Bad_Block_Bit_Index(U32 bit_index);
	U32 Bad_Block_Entry_Index();
	void Bad_Block_Entry_Index(U32 entry_index);
	
	// Given a page offset, return the corresponding bank
	// O O O O O O O O O b b D D D
	static U32 Bank_From_Offset(U32 offset);

	static U32 Banks_Per_Array();

	// Get_Set block
	U32  Block();
	void Block(U32 block);

	static U32 Blocks_Per_Device();

	static UI64 Bytes_Per_Array();
	static UI64 Bytes_Per_Bank();
	static UI64 Bytes_Per_Column();
	static U32 Bytes_Per_Device_Page();
	static U32 Bytes_Per_Page();

	// Get/Set column
	U32 Column();
	void Column(U32 column);
	
	static U32 Columns_Per_Array();

	// Set the device and bank from the offset in the page.
	// O O O O O O O O O b b D D D
	void Device(U32 offset);

	// Given a page offset, return the corresponding device number
	static U32 Device_From_Offset(U32 offset);

	// Device_Index is used to address the device as
	// bbDDD, where b = bank, D = device page
	U32  Device_Index();

	// Given a page offset, return the corresponding device column.
	static U32 Device_Column_From_Offset(U32 offset);

	// Given a page offset, return the corresponding device index
	static U32 Device_Index_From_Offset(U32 offset);

	static U32 Device_Pages_Per_Page();

	static U32 DWords_Per_Page();

	// Increment methods
	void Increment_Array();
	void Increment_Block();
	U32 Increment_Unit_Block();
	U32 Increment_Unit_Page();
	void Increment_Column();
	void Increment_Page();
	void Increment_Unit();

	// Get index -- concatenate all fields, e.g.,
	// Our index would look like the following if we have two arrays,
	// 1,024 blocks per unit, and 16 sectors per block:
	// bbbbbbbbbbSSSSACCC
	// bbbbbbbbbbSSSSUUUU
	// b = block
	// S = sector
	// A = Array
	// C = Column
	// U = Unit, combination of array, column
	U32 Index();

	// Initialize object.
	void Initialize();

	// Initialize static members from address capacity.
	static Status Initialize(Flash_Capacity *p_flash_capacity);

	// Return true if object is null
	int Is_Null();

	static U32 Num_Address_Bits_For_Sector();
	static U32 Num_Address_Bits_For_Offset();
	static U32 Num_Arrays();
	static U32 Num_Blocks();
	static U32 Num_Units();
	static U32 Num_Virtual_Pages();

	static U32 Offset_From_Virtual_Address(U32 virtual_address);

    // operator ==
    int operator==(const Flash_Address& rightSide);

    friend int operator==(const Flash_Address& leftSide,
                    const Flash_Address& rightSide);

    // operator !=
    int operator!=(const Flash_Address& rightSide);

    friend int operator!=(const Flash_Address& leftSide,
                    const Flash_Address& rightSide);

    // operator =
    Flash_Address& operator=(const U32& rightSide);

	// Get/Set page -- concatenation of block and sector
	U32 Page_Index();
	void Page_Index(U32 page);

	// offset from logical byte address
	static U32 Offset_From_Logical_Byte_Address(UI64 logical_byte_address);
	
	// Page_Index from logical byte address
	static U32 Page_Index_From_Logical_Byte_Address(UI64 logical_byte_address);
	
	// Get/Set sector -- sector within block
	U32 Sector();
	void Sector(U32 sector);	
	
	// Get/Set unit index -- concatenation of array and column, e.g.,
	// ACCC if Array is one bit, and Column is 3 bits.
	U32 Unit_Index();
	void Unit_Index(U32 unit_index);

	static U32 Pages_Per_Device();
	static U32 Sectors_Per_Block();
	
	static Status Validate(Flash_Address flash_address);

private: // member data

	typedef struct {
		BF		array:	 3;
		BF		bank:	 4;
		BF		column:	 4;
		BF		block:	16;

		// When use Flash_Address for Bad_Block_Entry_Index 
		// and Bad_Block_Bit_Index, we use the sector field
		// to store the device.
		BF		sector:	 5;
	} Address;

	union 
	{
		U32		entry;
		Address	addr;
	};


	static U32	m_num_address_bits_for_array;
	static U32  m_num_address_bits_for_bank;
	static U32	m_num_address_bits_for_block;
	static U32	m_num_address_bits_for_column;
	static U32	m_num_address_bits_for_device;
	static U32	m_num_address_bits_for_device_index;
	static U32	m_num_address_bits_for_sector;
	static U32	m_num_address_bits_for_offset;
	static U32	m_num_address_bits_for_page;
	static U32	m_num_address_bits_for_unit;
	static U32	m_num_address_bits_for_virtual_page;

	static U32	m_shift_array;
	static U32	m_shift_array_index;
	static U32	m_shift_block;
	static U32	m_shift_block_index;
	static U32	m_shift_device_column;
	static U32	m_shift_sector;
	static U32	m_shift_virtual_page;

	static U32 m_banks_per_array;
	static U32 m_blocks_per_device;
	static UI64 m_bytes_per_array;
	static UI64 m_bytes_per_bank;
	static UI64 m_bytes_per_column;
	static U32 m_bytes_per_device_page;
	static U32 m_bytes_per_page;
	static U32 m_columns_per_array;
	static U32 m_device_indexes_per_page;
	static U32 m_devices_per_page;
	static U32 m_dwords_per_page;
	static U32 m_num_arrays;
	static U32 m_num_blocks;
	static U32 m_num_units;
	static U32 m_num_virtual_pages;
	static U32 m_pages_per_device;
	static U32 m_sectors_per_block;

	static U32 m_mask_array;
	static U32 m_mask_array_index;
	static U32 m_mask_bank;
	static U32 m_mask_bank_index;
	static U32 m_mask_block;
	static U32 m_mask_block_index;
	static U32 m_mask_column;
	static U32 m_mask_column_index;
	static U32 m_mask_device;
	static U32 m_mask_device_index;
	static U32 m_mask_offset;
	static U32 m_mask_sector;
	static UI64 m_mask_virtual_page;

}; // Flash_Address

/*************************************************************************/
// constructor 
/*************************************************************************/
inline Flash_Address::Flash_Address() : entry(0) {}

/*************************************************************************/
// Get/Set array
/*************************************************************************/
inline U32 Flash_Address::Array()
{
	return addr.array;
}
inline void Flash_Address::Array(U32 array)
{
	addr.array = array;
}

/*************************************************************************/
// Get Bad_Block_Bit_Index
// CCCDDD is an index into the bit in the doubleword entry.
/*************************************************************************/
inline U32 Flash_Address::Bad_Block_Bit_Index()
{
	// sector is reused for device.
	return addr.sector | (addr.column << m_num_address_bits_for_device);
}

/*************************************************************************/
// Set Bad_Block_Bit_Index
// CCCDDD is an index into the bit in the doubleword entry.
/*************************************************************************/
inline void Flash_Address::Bad_Block_Bit_Index(U32 index)
{
	// sector is reused for device.
	addr.sector = index & m_mask_device;

	addr.column = (index & m_mask_column_index) >> m_num_address_bits_for_device;
}

/*************************************************************************/
// Get Bad_Block_Entry_Index
// ABBBBBBBBBBBbb is index into bad block doubleword entry.
// A = Array, B = Block number, b = bank number.
/*************************************************************************/
inline U32 Flash_Address::Bad_Block_Entry_Index()
{
	return ((addr.array << m_shift_array_index)
		| (addr.block << m_num_address_bits_for_bank)
		| addr.bank);
}

/*************************************************************************/
// Set Bad_Block_Entry_Index
// ABBBBBBBBBBBbb is index into bad block doubleword entry.
// A = Array, B = Block number, b = bank number.
/*************************************************************************/
inline void Flash_Address::Bad_Block_Entry_Index(U32 index)
{
	addr.array = (index & m_mask_array_index) >> m_shift_array_index;
	addr.block = (index & m_mask_block_index) >> m_num_address_bits_for_bank;
	addr.bank = index & m_mask_bank_index;
}

/*************************************************************************/
// Columns_Per_Array
/*************************************************************************/
inline U32 Flash_Address::Columns_Per_Array()
{
	return m_columns_per_array;
}

/*************************************************************************/
// Banks_Per_Array
// How many banks per array?
/*************************************************************************/
inline U32 Flash_Address::Banks_Per_Array()
{
	return m_banks_per_array;
}

/*************************************************************************/
// Get/Set block
/*************************************************************************/
inline U32 Flash_Address::Block()
{
	return addr.block;
}
inline void Flash_Address::Block(U32 block)
{
	addr.block = block;
}

/*************************************************************************/
// Bytes_Per_Array
/*************************************************************************/
inline UI64 Flash_Address::Bytes_Per_Array()
{
	return m_bytes_per_array;
}

/*************************************************************************/
// Bytes_Per_Bank
/*************************************************************************/
inline UI64 Flash_Address::Bytes_Per_Bank()
{
	return m_bytes_per_bank;
}

/*************************************************************************/
// Bytes_Per_Column
/*************************************************************************/
inline UI64 Flash_Address::Bytes_Per_Column()
{
	return m_bytes_per_column;
}

/*************************************************************************/
// Bytes_Per_Page
/*************************************************************************/
inline U32 Flash_Address::Bytes_Per_Page()
{
	return m_bytes_per_page;
}

/*************************************************************************/
// Bytes_Per_Device_Page
/*************************************************************************/
inline U32 Flash_Address::Bytes_Per_Device_Page()
{
	return m_bytes_per_device_page;
}

/*************************************************************************/
// Blocks_Per_Device
/*************************************************************************/
inline U32 Flash_Address::Blocks_Per_Device()
{
	return m_blocks_per_device;
}

/*************************************************************************/
// Get/Set column
/*************************************************************************/
inline U32 Flash_Address::Column()
{
	return addr.column;
}
inline void Flash_Address::Column(U32 column)
{
	addr.column = column;
}

/*************************************************************************/
// Device
// Set the device and bank from the offset in the page.
// O O O O O O O O O b b D D D
/*************************************************************************/
inline void Flash_Address::Device(U32 offset)
{
	addr.bank = (offset & m_mask_bank) >> m_num_address_bits_for_device;

	// sector gets reused for device.
	addr.sector = offset & m_mask_device;
}

/*************************************************************************/
// Bank_From_Offset
// Given a page offset, return the corresponding bank
// O O O O O O O O O b b D D D
/*************************************************************************/
inline U32 Flash_Address::Bank_From_Offset(U32 offset)
{
	return (offset & m_mask_bank) >> m_num_address_bits_for_device;
}

/*************************************************************************/
// Device_From_Offset
// Given a page offset, return the corresponding device number
// O O O O O O O O O b b D D D
/*************************************************************************/
inline U32 Flash_Address::Device_From_Offset(U32 offset)
{
	return (offset & m_mask_device);
}

/*************************************************************************/
// Device_Index is used to address the device as
// bbDDD, where b = bank, D = device page
/*************************************************************************/
inline U32 Flash_Address::Device_Index()
{
	// sector gets reused for device.
	return addr.sector | (addr.bank << m_num_address_bits_for_device);

}

/*************************************************************************/
// Device_Column_From_Offset
// Given a page offset, return the corresponding device column.
// This is equivalent to the offset in the device page.
// C C C C C C C C C b b D D D
/*************************************************************************/
inline U32 Flash_Address::Device_Column_From_Offset(U32 offset)
{
	return (offset >> m_shift_device_column);
}

/*************************************************************************/
// Device_Index_From_Offset
// Given a page offset, return the corresponding device.
// O O O O O O O O O b b D D D
// Device_Index is used to address the device as
// bbDDD, where b = bank, D = device page
/*************************************************************************/
inline U32 Flash_Address::Device_Index_From_Offset(U32 offset)
{
	return offset & m_mask_device_index;
}

/*************************************************************************/
// Device_Pages_Per_Page
// How many devices must be accessed to make up one full page?
/*************************************************************************/
inline U32 Flash_Address::Device_Pages_Per_Page()
{
	return m_device_indexes_per_page;
}

/*************************************************************************/
// DWords_Per_Page
/*************************************************************************/
inline U32 Flash_Address::DWords_Per_Page()
{
	return m_dwords_per_page;
}

/*************************************************************************/
// Increment_Array
/*************************************************************************/
inline void Flash_Address::Increment_Array()
{
	if (addr.array == (m_num_arrays - 1))
	{
		// The array has overflowed
		addr.array = 0;
		return;
	}

	addr.array++;
}

/*************************************************************************/
// Increment_Block.
// If block overflows, increment array.
/*************************************************************************/
inline void Flash_Address::Increment_Block()
{
	// Increment the real address by one block.
	// Is this the last block?
	if (addr.block != (m_blocks_per_device - 1))
	{
		addr.block++;
		return;
	}

	// The block has overflowed, so increment the column.
	addr.block = 0;

	// Is this the last column?
	if (addr.column != (m_columns_per_array - 1))
	{
		// Increment to the next column.
		addr.column++;
		return;
	}

	// The column has overflowed, so increment the array
	addr.column = 0;

	// Is this the last array?
	if (addr.array != (m_num_arrays - 1))
	{
		// Increment array
		addr.array++;
		return;
	}

	// The array has overflowed
	addr.array = 0;
}

/*************************************************************************/
// Increment_Column
/*************************************************************************/
inline void Flash_Address::Increment_Column()
{
	addr.column++;

	// Is this the last column?
	CT_ASSERT((addr.column < m_columns_per_array), Increment_Column);

}

/*************************************************************************/
// Increment_Page
// If page overflows, increment block.
/*************************************************************************/
inline void Flash_Address::Increment_Page()
{
	// Is this the last sector in the block?
	if (addr.sector != (m_sectors_per_block - 1))
	{
		addr.sector++;
		return;
	}

	// Start at sector 0 of the next block.
	addr.sector = 0;
	Increment_Block();
}

/*************************************************************************/
// Increment_Unit
/*************************************************************************/
inline void Flash_Address::Increment_Unit()
{
	// Is this the last column?
	if (addr.column != (m_columns_per_array - 1))
	{
		// Increment to the next column.
		addr.column++;
		return;
	}

	// The column has overflowed, so increment the array
	addr.column = 0;

	// Is this the last array?
	if (addr.array != (m_num_arrays - 1))
		addr.array++;
	else
		addr.array = 0;

} // Increment_Unit

/*************************************************************************/
// Increment_Unit_Block 
// Increment block for this unit and return incremented block value.
// If block overflows, return 0.
/*************************************************************************/
inline U32 Flash_Address::Increment_Unit_Block()
{
	// Increment the real address by one block.
	// Is this the last block?
	if (addr.block != (m_blocks_per_device - 1))
	{
		return ++addr.block;
	}

	// The block has overflowed, so increment the column.
	addr.block = 0;

	return 0;
}

/*************************************************************************/
// Increment_Unit_Page
// Increment page.  If page overflows, increment block for this unit.
// Return 0 if last page for this uit.
/*************************************************************************/
inline U32 Flash_Address::Increment_Unit_Page()
{
	// Is this the last sector in the block?
	if (addr.sector != (m_sectors_per_block - 1))
	{
		return ++addr.sector;
	}

	// Start at sector 0 of the next block.
	addr.sector = 0;
	return Increment_Unit_Block();
}

/*************************************************************************/
// Index
// Get index -- concatenate all fields, e.g.,
// Our index would look like the following if we have two arrays,
// 1,024 blocks per unit, and 16 sectors per block:
// bbbbbbbbbbSSSSACCC
// bbbbbbbbbbSSSSUUUU
// b = block
// S = sector
// A = Array
// C = Column
// U = Unit, combination of array, column
/*************************************************************************/
inline U32 Flash_Address::Index()
{
	return (addr.column
		| (addr.array << m_shift_array)
		| (addr.sector << m_shift_sector)
		| (addr.block << m_shift_block));
}

/*************************************************************************/
// Initialize object
/*************************************************************************/
inline void Flash_Address::Initialize()
{
	entry = 0;
}

/*************************************************************************/
// Is_Null
// Return true if object is null
/*************************************************************************/
inline int Flash_Address::Is_Null()
{
	return (entry == 0);
}

/*************************************************************************/
// Num_Address_Bits_For_Offset
/*************************************************************************/
inline U32 Flash_Address::Num_Address_Bits_For_Offset()
{
	return m_num_address_bits_for_offset;
}

/*************************************************************************/
// Num_Address_Bits_For_Sector
/*************************************************************************/
inline U32 Flash_Address::Num_Address_Bits_For_Sector()
{
	return m_num_address_bits_for_sector;
}

/*************************************************************************/
// Num_Arrays
// How many arrays are there on this board?
/*************************************************************************/
inline U32 Flash_Address::Num_Arrays()
{
	return m_num_arrays;
}

/*************************************************************************/
// Num_Blocks
// How many blocks total are there on all units on this board?
/*************************************************************************/
inline U32 Flash_Address::Num_Blocks()
{
	return m_num_blocks;
}

/*************************************************************************/
// Num_Units
// How many units total are there on this board?
/*************************************************************************/
inline U32 Flash_Address::Num_Units()
{
	return m_num_units;
}

/*************************************************************************/
// Num_Virtual_Pages
// The number of virtual pages depends on the number of bits 
// used for the virtual page number: array, column, block, sector.
/*************************************************************************/
inline U32 Flash_Address::Num_Virtual_Pages()
{
	return m_num_virtual_pages;
}

/*************************************************************************/
// Offset from logical byte address
/*************************************************************************/
inline U32 Flash_Address::Offset_From_Logical_Byte_Address(UI64 logical_byte_address)
{
	return (U32)(logical_byte_address & m_mask_offset);
}

/*************************************************************************/
// Flash_Address operator ==
/*************************************************************************/
inline int Flash_Address::operator==(const Flash_Address&
    rightSide)
{
    return entry == rightSide.entry;
}

inline int operator==(const Flash_Address& leftSide,
                const Flash_Address& rightSide)
{
    return leftSide.entry == rightSide.entry;
}

/*************************************************************************/
// Flash_Address operator !=
/*************************************************************************/
inline int Flash_Address::operator!=(const Flash_Address&
    rightSide)
{
    return entry != rightSide.entry;
}

inline int operator!=(const Flash_Address& leftSide,
                const Flash_Address& rightSide)
{
    return leftSide.entry != rightSide.entry;
}

/*************************************************************************/
// Flash_Address operator = where right side is U32
/*************************************************************************/
inline Flash_Address& Flash_Address::operator=(const U32& real_flash_address)
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
	addr.array = (real_flash_address & m_mask_array) >> m_shift_array;
	addr.block = (real_flash_address & m_mask_block) >> m_shift_block;
	addr.column = real_flash_address & m_mask_column;
	addr.sector = (real_flash_address & m_mask_sector) >> m_shift_sector;
    return *this;
}

/*************************************************************************/
// Page_Index
// Get page index -- concatenation of sector and block, e.g.,
// BBBBBBBBBBSSSSS if Block is 10 bits, and Sector is 5 bits.
/*************************************************************************/
inline U32 Flash_Address::Page_Index()
{
	return addr.sector | (addr.block << m_num_address_bits_for_sector);
}

/*************************************************************************/
// Page_Index
// Set page index -- concatenation of sector and block, e.g.,
// BBBBBBBBBBSSSSS if Block is 10 bits, and Sector is 5 bits.
/*************************************************************************/
inline void Flash_Address::Page_Index(U32 page_index)
{
	addr.sector = page_index & m_mask_sector;
	addr.block = (page_index & m_mask_block) >> m_num_address_bits_for_sector;
}

/*************************************************************************/
// Page_Index from logical byte address
/*************************************************************************/
inline U32 Flash_Address::Page_Index_From_Logical_Byte_Address(UI64 logical_byte_address)
{
	return (U32)(((logical_byte_address  & m_mask_virtual_page) >> (m_shift_virtual_page)));
}

/*************************************************************************/
// Pages_Per_Device
/*************************************************************************/
inline U32 Flash_Address::Pages_Per_Device()
{
	return m_pages_per_device;
}

/*************************************************************************/
// Get/Set sector
/*************************************************************************/
inline U32 Flash_Address::Sector()
{
	return addr.sector;
}
inline void Flash_Address::Sector(U32 sector)
{
	addr.sector = sector;
}

/*************************************************************************/
// Sectors_Per_Block
/*************************************************************************/
inline U32 Flash_Address::Sectors_Per_Block()
{
	return m_sectors_per_block;
}

/*************************************************************************/
// Unit_Index
// Get unit index -- concatenation of array and column, e.g.,
// ACCC if Array is one bit, and Column is 3 bits.
/*************************************************************************/
inline U32 Flash_Address::Unit_Index()
{
	return addr.column | (addr.array << m_num_address_bits_for_column);
}

/*************************************************************************/
// Unit_Index
// Set unit -- concatenation of array and column, e.g.,
// ACCC if Array is one bit, and Column is 3 bits.
/*************************************************************************/
inline void Flash_Address::Unit_Index(U32 unit_index)
{
	addr.column = unit_index & m_mask_column;
	addr.array = (unit_index & m_mask_array) >> m_num_address_bits_for_column;
}


#endif // FlashAddress_H