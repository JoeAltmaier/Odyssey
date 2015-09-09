/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfBlockMapEntry.h
// 
// Description:
// This file defines the FF_Block_Map_Entry object for the flash file 
// system. 
// 
// 5/3/99 Jim Frandeen: Create file
/*************************************************************************/

#if !defined(FfBlockMapEntry_H)
#define FfBlockMapEntry_H

#include "FfBadSpotTable.h"
#include "FlashStorage.h"
#include "Simple.h"
#include "List.h"

#define FF_NUM_BLOCK_DEVICES 32

#define FF_MAX_BAD_SPOTS 4

// Make sure FF_Alternate_Block_Address gets packed into 16 bits.
#pragma pack(1)

/*************************************************************************/
// FF_Alternate_Block_Address class
// The purpose of this class is to map a 16-bit field into the equivalent
// of:
//	typedef struct {
//		BF	block_number:	11;
//		BF	device_address:	05;
//	} FF_Alternate_Block_Address;
/*************************************************************************/
class FF_Alternate_Block_Address
{
public: // methods

	// Get/Set block number
	U32 Block();
	void Block(U32 block);

	// Get/Set device
	U32 Device();
	void Device(U32 device);

private: // member data

	// BBBBBBBBBBBDDDDD
	// B = block number
	// D = device address
	U16	m_entry;


}; // FF_Alternate_Block_Address

	const U16	m_num_address_bits_for_device = 5;
	const U16	m_num_devices = (1 << m_num_address_bits_for_device);
	const U16	m_mask_device = m_num_devices - 1;

	const U16	m_num_address_bits_for_block = 11;
	const U16	m_num_blocks = (1 << m_num_address_bits_for_block);
	const U16	m_mask_block = (m_num_blocks - 1) << m_num_address_bits_for_device;

inline U32 FF_Alternate_Block_Address::Block()
{
	return (m_entry & m_mask_block) >> m_num_address_bits_for_device;
}

inline void FF_Alternate_Block_Address::Block(U32 block)
{
	m_entry = (m_entry & m_mask_device) | ((U16)block << m_num_address_bits_for_device);
}

inline U32 FF_Alternate_Block_Address::Device()
{
	return m_entry & m_mask_device;
}

inline void FF_Alternate_Block_Address::Device(U32 device)
{
	m_entry = (m_entry & m_mask_block) | (U16)device;
}

/*************************************************************************/
// FF_Block_Map_Entry class
// This is used to map a block number to an array of device block numbers.
// There is one FF_Block_Map_Entry for each block.
/*************************************************************************/
class FF_Block_Map_Entry
{
public: // methods

	// Map entire block to a replacement block.
	void Map_Replacement(U32 replacement_block_number);

	// Map the specified entry one to one.
	void Map_One_To_One();

	// Map the specified device page to an alternate block.
	// device_address is used to address the device as
	// bbDDD, where b = bank, D = device page
	void Map_Alternate(U32 alternate_block_number, U32 device_address, U32 index);

	// Given a block number, map it to a block number for each device.
	void Map_To_Device_Blocks(U16 block_number, 
		U16 device_block_number[FF_NUM_BLOCK_DEVICES]);

	// Given an flash address, return a flash address.
	void Get_Flash_Page_Map(Flash_Address flash_address, Flash_Page_Map flash_page_map);

    // operator ==
    int operator==(const FF_Block_Map_Entry& rightSide);

    friend int operator==(const FF_Block_Map_Entry& leftSide,
                    const FF_Block_Map_Entry& rightSide);

private: // member data

	// If all m_alternate_entry are zero:
	//		block maps directly with no bad spots.
	// If m_alternate_entry[0] non zero, m_alternate_entry[1] zero:
	//		One alternate block for the device page specified by device_page_0.
	// If m_alternate_entry[0] non zero, m_alternate_entry[1] non zero:
	//		Two alternate blocks.
	// If m_alternate_entry[0] zero, m_alternate_entry[1] non zero:
	//		Replacement block (used when original block had more than max bad spots).

	union {
		UI64						m_entry;
		U16							m_alternate_entry[FF_MAX_BAD_SPOTS];
		FF_Alternate_Block_Address	m_alternate_block_address[FF_MAX_BAD_SPOTS];
	};

}; // FF_Block_Map_Entry


/*************************************************************************/
// FF_Block_Map_Entry operator ==
/*************************************************************************/
inline int FF_Block_Map_Entry::operator==(const FF_Block_Map_Entry&
    rightSide)
{
    return m_entry == rightSide.m_entry;
}

inline int operator==(const FF_Block_Map_Entry& leftSide,
                const FF_Block_Map_Entry& rightSide)
{
    return leftSide.m_entry == rightSide.m_entry;
}

/*************************************************************************/
// Map_One_To_One
// Map this entry one to one.
/*************************************************************************/
void inline FF_Block_Map_Entry::Map_One_To_One()
{
	m_entry = 0;
}

/*************************************************************************/
// Map_Replacement 
// Map entire block to a replacement block.
// Store the replacement block in the second alternate address.
// This encoding of alternate addresses, where the first alternate address is 
// zero, indicates that the second alternate address is a replacement address.
/*************************************************************************/
inline void FF_Block_Map_Entry::Map_Replacement(U32 block_number)
{
	CT_ASSERT((m_alternate_entry[0] == 0), Map_Replacement);
	CT_ASSERT((m_alternate_entry[1] == 0), Map_Replacement);

	m_alternate_block_address[1].Block(block_number);
}

/*************************************************************************/
// Map_Alternate 
// Map the specified device page to an alternate block.
// device_address is used to address the device as
// bbDDD, where b = bank, D = device page
/*************************************************************************/
inline void FF_Block_Map_Entry::Map_Alternate(U32 block_number, U32 device_address,
											  U32 index)
{
	CT_ASSERT((m_alternate_entry[index] == 0), Map_Alternate);

	m_alternate_block_address[index].Block(block_number);
	m_alternate_block_address[index].Device(device_address);
}


#endif /* FfBlockMapEntry_H  */
