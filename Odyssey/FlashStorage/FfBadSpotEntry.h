/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfBadSpotEntry.h
// 
// Description:
// This file defines the FF_Bad_Spot_Entry object for the flash file 
// system. 
// 
// 5/3/99 Jim Frandeen: Create file
/*************************************************************************/

#if !defined(FfBadSpotEntry_H)
#define FfBadSpotEntry_H

#include "ErrorLog.h"
#include "Simple.h"
#include <String.h>

#define FF_NUM_BLOCK_DEVICES 32

extern U32					  FF_bit_mask[32];

/*************************************************************************/
// FF_Bad_Spot_Entry class
// This describes the bad spots (if any)  for the block.
// The bad spot table is indexed by block number to access a FF_Bad_Spot_Entry.
/*************************************************************************/
class FF_Bad_Spot_Entry
{
public: // methods

	// constructor
	FF_Bad_Spot_Entry(U32 block_number);

	// Return the number of bad spots for this entry.
	U32 Get_Num_Bad_Spots();

	// Return the block number for this entry
	U32 Get_Block_Number();

	// Return the block number for this entry if the
	// entry can be used for a replacement block.
	U32 Get_Replacement_Block_Number();

	// Return the device address specified by the bad spot index.
	// device_address is used to address the device as
	// bbDDD, where b = bank, D = device page
	U32 Get_Bad_Spot(U32 bad_spot_index);

	// Add a bad spot for this block
	// device_address is used to address the device as
	// bbDDD, where b = bank, D = device page
	void Add_Bad_Spot(U32 device_address);

	// Get a good spot for this block
	// device_address is used to address the device as
	// bbDDD, where b = bank, D = device page
	U32 Get_Good_Spot(U32 device_address);

private: // member data

	U16			m_block_number;
	U8			m_num_bad_spots;
	U8			m_is_replacement_block;
	U32			m_bad_spot_mask;

}; // FF_Bad_Spot_Entry

/*************************************************************************/
// FF_Bad_Spot_Entry constructor
/*************************************************************************/
inline FF_Bad_Spot_Entry::FF_Bad_Spot_Entry(U32 block_number)
{
	m_block_number = (U16)block_number;
	m_num_bad_spots = 0;
	m_bad_spot_mask = 0;
	m_is_replacement_block = 0;
}

/*************************************************************************/
// FF_Bad_Spot_Entry::Get_Num_Bad_Spots  
// Returns number of bad spots for this block.
/*************************************************************************/
inline U32 FF_Bad_Spot_Entry::Get_Num_Bad_Spots()
{
	return m_num_bad_spots;
}

/*************************************************************************/
// FF_Bad_Spot_Entry::Get_Block_Number  
// Returns block number for this entry.
/*************************************************************************/
inline U32 FF_Bad_Spot_Entry::Get_Block_Number()
{
	return m_block_number;
}

/*************************************************************************/
// FF_Bad_Spot_Entry::Get_Replacement_Block_Number  
// Return the block number for this entry if the
// entry can be used for a replacement block.
/*************************************************************************/
inline U32 FF_Bad_Spot_Entry::Get_Replacement_Block_Number()
{
	// Is this already a replacement block?
	if (m_is_replacement_block)
		return 0;

	if (m_num_bad_spots > 2)

		// This entry has too many bad spots to be used for a replacement block.
		return 0;

	// Turn on replacement flag so we won't try to use it.
	m_is_replacement_block = 1;
	return m_block_number;
}

/*************************************************************************/
// FF_Bad_Spot_Entry::Add_Bad_Spot
// Add a bad spot for this block at the specified device address.
// device_address is used to address the device as
// bbDDD, where b = bank, D = device page
/*************************************************************************/
inline void FF_Bad_Spot_Entry::Add_Bad_Spot(U32 device_address)
{
	CT_ASSERT(((m_bad_spot_mask & FF_bit_mask[device_address]) == 0), Add_Bad_Spot);
	m_bad_spot_mask |= FF_bit_mask[device_address];
	m_num_bad_spots++;
}

/*************************************************************************/
// FF_Bad_Spot_Entry::Get_Bad_Spot
// Return the device address specified by the bad spot index.
// i.e., if bad_spot_index is zero, return first bad spot.
// device_address is used to address the device as
// bbDDD, where b = bank, D = device page
/*************************************************************************/
inline U32 FF_Bad_Spot_Entry::Get_Bad_Spot(U32 bad_spot_index)
{
	U32 bad_spot_mask = m_bad_spot_mask;
	U32 bad_spot_found = 0;
	U32 device_address = 0;

	// Search for the specified bad spot.
	while (bad_spot_mask)
	{
		// Is this device page bad?
		if (bad_spot_mask & 1)
		{
			// Is this the bad spot we were looking for?
			if (bad_spot_found == bad_spot_index)

				// Return the device page that corresponds to this bad spot.
				return device_address;

			// Increment number of bad spots found.
			bad_spot_found++;
		}

		// Increment the device_address.
		device_address++;

		// Check the bad spot for the next device page.
		bad_spot_mask = (bad_spot_mask >> 1);
	}

	// We should not run out of bad spots before we come to the one
	// we are looking for.
	CT_ASSERT((bad_spot_mask != 0), Get_Bad_Spot);

	return 0;
}

/*************************************************************************/
// FF_Bad_Spot_Entry::Get_Good_Spot
// Return block number if specified device address is good.
// device_address is used to address the device as
// bbDDD, where b = bank, D = device page
/*************************************************************************/
inline U32 FF_Bad_Spot_Entry::Get_Good_Spot(U32 device_address)
{
	// If this is a replacement block, it has already been used.
	if (m_is_replacement_block)
		return 0;

	if ((m_bad_spot_mask & FF_bit_mask[device_address]) == 0)
	{
		// Mark spot bad so we won't use it again.
		Add_Bad_Spot(device_address);

		// Return good block number.
		return m_block_number;
	}
	return 0;
}


#endif /* FfBadSpotEntry_H  */
