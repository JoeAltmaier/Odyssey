/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FfBlockMapTable.h
// 
// Description:
// This file defines the FF_Block_Map_Table for the Flash File 
// 
// $Log: /Gemini/Odyssey/FlashStorage/FfBlockMapTable.h $
// 
// 4     11/30/99 6:17p Jfrandeen
// Implement erase device structures command.
// 
// 3     11/22/99 10:36a Jfrandeen
// Implement interleaved flash
// 
// 2     8/24/99 10:07a Jfrandeen
// 
// 1     8/03/99 11:30a Jfrandeen
// 
// 5/11/99 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FfBlockMapTable_H)
#define FfBlockMapTable_H

#include "FfBlockMapEntry.h"
#include "FfBlockMapIterator.h"

/*************************************************************************/
// FF_Block_Map_Table class
// There is one  FF_Block_Map_Table for each unit.
// Table has one FF_Block_Map_Entry for every block.
/*************************************************************************/
class FF_Block_Map_Table
{
public: // methods

	// Allocate object.
	void Allocate(void *p_memory, U32 num_entries);

	// Initialize object.
	void Initialize(U32 num_entries);

	// Map entire block to a replacement block.
	void Map_Replacement(U32 block_number, U32 replacement_block_number);

	// Map the specified block number one to one.
	void Map_One_To_One(U32 block_number);

	// Map the specified device page to an alternate block.
	// device_address is used to address the device as
	// bbDDD, where b = bank, D = device page
	void Map_Alternate(U32 block_number, U32 alternate_block_number, U32 device_address,
		U32 index);

	// Given a block number, return a flash address.
	void Get_Flash_Page_Map(Flash_Address flash_address, Flash_Page_Map flash_page_map);

	void Set_Num_Replacement_Entries(U32 num_replacement_entries);
	U32 Get_Num_Replacement_Entries();

   // Iterator methods to access table entries.
    FF_Block_Map_Iterator Begin();
    FF_Block_Map_Iterator Begin_Replacement();
    FF_Block_Map_Iterator End();
    FF_Block_Map_Iterator End_Replacement();

private: // member data

	// Number of entries in table.
	U32						m_num_entries;

	// Number of replacement entries in table.
	// Replacement entries are at the end of the table.
	U32						m_num_replacement_entries;

	// Array of block map entries.
	FF_Block_Map_Entry		*m_p_block_map_entry;

}; // FF_Block_Map_Table

/*************************************************************************/
// Begin returns iterator to first entry in table
/*************************************************************************/
inline FF_Block_Map_Iterator FF_Block_Map_Table::Begin() 
{
	return &m_p_block_map_entry[0];
}

/*************************************************************************/
// Begin_Replacement returns iterator to first replacement entry in table.
// Replacement entries are at the end of the table.
/*************************************************************************/
inline FF_Block_Map_Iterator FF_Block_Map_Table::Begin_Replacement() 
{
	return &m_p_block_map_entry[m_num_entries - m_num_replacement_entries];
}

/*************************************************************************/
// End returns iterator to last entry in table.
/*************************************************************************/
inline FF_Block_Map_Iterator FF_Block_Map_Table::End() 
{
	return &m_p_block_map_entry[m_num_entries];
}

/*************************************************************************/
// End_Replacement returns iterator to last replacement entry in table.
/*************************************************************************/
inline FF_Block_Map_Iterator FF_Block_Map_Table::End_Replacement() 
{
	return &m_p_block_map_entry[m_num_entries];
}

/*************************************************************************/
// Get_Flash_Page_Map
// Given block number, return a flash address.
/*************************************************************************/
inline void FF_Block_Map_Table::Get_Flash_Page_Map(Flash_Address flash_address, 
	Flash_Page_Map flash_page_map)
{
	CT_ASSERT((flash_address.Block() < Flash_Address::Blocks_Per_Device()), Get_Flash_Page_Map);
	m_p_block_map_entry[flash_address.Block()].Get_Flash_Page_Map(flash_address, 
		flash_page_map);
}

/*************************************************************************/
// Map_Alternate
// Map the specified device page to an alternate block.
// device_address is used to address the device as
// bbDDD, where b = bank, D = device page
/*************************************************************************/
inline void FF_Block_Map_Table::Map_Alternate(U32 block_number, 
	U32 alternate_block_number, U32 device_address, U32 index)
{
	CT_ASSERT((block_number < (m_num_entries - m_num_replacement_entries)), Map_Alternate);
	m_p_block_map_entry[block_number].
		Map_Alternate(alternate_block_number, device_address, index);
}

/*************************************************************************/
// Map_One_To_One
// Map the specified block one to one.
/*************************************************************************/
inline void FF_Block_Map_Table::Map_One_To_One(U32 block_number)
{
	CT_ASSERT((block_number < (m_num_entries - m_num_replacement_entries)), Map_One_To_One);
	m_p_block_map_entry[block_number].Map_One_To_One();
}

/*************************************************************************/
// Map_Replacement
// Map this block number to a replacement block.  This puts the replacement block
// in the second alternate block entry.  This leaves room for one or two
// alternate blocks in case the replacement block has any bad spots.
/*************************************************************************/
inline void FF_Block_Map_Table::Map_Replacement(U32 block_number, U32 replacement_block_number)
{
	CT_ASSERT((block_number < (m_num_entries - m_num_replacement_entries)), Map_Replacement);
	m_p_block_map_entry[block_number].Map_Alternate(replacement_block_number, 0, 1);
}

/*************************************************************************/
// Get/Set number of replacement entries.
/*************************************************************************/
inline U32 FF_Block_Map_Table::Get_Num_Replacement_Entries() 
{
	return m_num_replacement_entries;
}

inline void FF_Block_Map_Table::Set_Num_Replacement_Entries(U32 num_replacement_entries) 
{
	m_num_replacement_entries = num_replacement_entries;
}

#endif // FfBlockMapTable_H

