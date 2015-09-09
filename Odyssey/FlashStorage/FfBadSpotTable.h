/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FfBadSpotTable.h
// 
// Description:
// This file defines the FF_Bad_Spot_Table for the Flash File 
// 
// 5/11/99 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FfBadSpotTable_H)
#define FfBadSpotTable_H

#include "FfBadSpotEntry.h"
#include "FfBadSpotIterator.h"

/*************************************************************************/
// FF_Bad_Spot_Table class
// There is one FF_Bad_Spot_Table for each unit.
// FF_Bad_Spot_Table has one FF_Bad_Spot_Entry for every block.
/*************************************************************************/
class FF_Bad_Spot_Table
{
public: // methods

	// Initialize object.
	void Initialize(void *p_memory, U32 num_entries);

	// Return the block number of an alternate block for the specified device_address.
	// device_address is used to address the device as
	// bbDDD, where b = bank, D = device page
	// Return 0 if none are available.
	U32 Get_Alternate_Block(U32 device_address, FF_Bad_Spot_Iterator bad_spot_iterator);

	// Return the iterator of a replacement block.
	FF_Bad_Spot_Iterator Get_Replacement_Block(FF_Bad_Spot_Iterator bad_spot_iterator);

	// Mark bad spot for specified device address.
	// device_address is used to address the device as
	// bbDDD, where b = bank, D = device page
	void Add_Bad_Spot(U32 block_number, U32 device_address);

	// Get number of replacement entries.
	U32 Get_Num_Replacement_Entries();

    // Iterator methods to access bad spot table.
    FF_Bad_Spot_Iterator Add_Replacement();
    FF_Bad_Spot_Iterator Begin();
    FF_Bad_Spot_Iterator Begin_Replacement();
    FF_Bad_Spot_Iterator End();
    FF_Bad_Spot_Iterator End_Replacement();

	Status Find_Nth_Good_Block(U32 n, U32 *p_block_number, U32 if_skip_block0);

private: // helper methods

	void Add_Replacement_Entry();

private: // member data

	// Number of entries in table.
	U32						m_num_entries;

	// Number of replacement entries in table.
	// Replacement entries are at the end of the table.
	U32						m_num_replacement_entries;

	// Array of bad spot entries.
	// The bad spot table is indexed by block number to access a FF_Bad_Spot_Entry.
	FF_Bad_Spot_Entry		*m_p_bad_spot_entry;

}; // FF_Bad_Spot_Table

#endif // FfBadSpotTable_H

