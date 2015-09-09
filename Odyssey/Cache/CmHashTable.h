/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: CmHashTable.h
// 
// Description:
// This file defines the hash methods used by the Cache Manager. 

// Update Log 
// 
// 2/5/99 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(CmHashTable_H)
#define CmHashTable_H

#include "Cache.h"
#include "List.h"
#include "CmFrame.h"
#include "CmMem.h"
#include "CmCommon.h"

typedef struct  {
	I64			  m_num_hash_lookups;
	I64			  m_num_hash_lookup_steps;
	U32			  m_num_hash_lookup_steps_max;
} CM_Hash_Statistics;

class CM_Hash_Table 
{

public: // methods

	// Allocate and initialize object
	Status Allocate(CM_Mem *p_mem, U32 hash_table_size);

	// Returns index of Frame if page is present.
	// Return zero if page is not present.
	U32 Get_Frame_Index(I64 page_number);
	
	// Map this page in the hash table.
	void Map_Page(I64 page_number, CM_Frame *p_frame);
	
	// Unmap this page in the hash table.
	void Unmap_Page(I64 page_number);

	// Get hash statistics
	void Get_Statistics(CM_Hash_Statistics *p_statistics_buffer, 
		U32 size_buffer);
	
	// Reset Statistics
	void Reset_Statistics();

	// Return memory size required by hash table.
	static U32 Memory_Size(CM_CONFIG *p_config) ;

private: // helper methods

	U32 Hash(I64 page_number);

private: // members

	CM_Hash_Statistics	  m_hash_stats;
	U32					  m_hash_table_size;
	CM_Frame			**m_p_hash_table;


}; // CM_Hash_Table

/*************************************************************************/
// CM_Hash_Table::Allocate
// Allocate and initialize object
/*************************************************************************/
inline Status CM_Hash_Table::Allocate(CM_Mem *p_mem, U32 hash_table_size)
{
	// Allocate hash table.
	m_hash_table_size = hash_table_size;
	m_p_hash_table = 
		(CM_Frame **)p_mem->Allocate(sizeof(U32) * hash_table_size);
	if (m_p_hash_table == 0)
		return CM_NO_MEMORY;

	// Set table to zero -- no pages present
	memset(m_p_hash_table, 0, (sizeof(U32) * hash_table_size));

	// Set stats to zero.
	Reset_Statistics();

	return OK;

} // CM_Hash_Table::Allocate

/*************************************************************************/
// CM_Hash_Table::Get_Frame_Index
// Return the frame index for the specified page number.
// Return zero if page is not present.
// Keep statistics for the number of lookup steps.
// Note that we only do this on the initial quey, though we need to do
// another lookup when the page is mapped.
/*************************************************************************/
inline U32 CM_Hash_Table::Get_Frame_Index(I64 page_number)
{
	// Increment total number of lookups.
	m_hash_stats.m_num_hash_lookups++;

	// Point to the first frame for this bucket.
	U32 hash_index = Hash(page_number);
	U32 num_lookup_steps = 1;
	CM_Frame *p_frame = m_p_hash_table[hash_index];

	while(p_frame)
	{
		// Does this hash entry match the page number we are looking for?
		if (p_frame->Get_Page_Number() == page_number)
		{
			// Increment total number of lookup steps by the 
			// number of lookup steps it took to find this frame.
			m_hash_stats.m_num_hash_lookup_steps += num_lookup_steps;
			return p_frame->Get_Frame_Index();
		}

		// Point to the next frame for this hash bucket.
		p_frame = p_frame->Get_Next_Hash_Entry();
		num_lookup_steps++;
	}
	// Increment total number of lookup steps by the 
	// number of lookup steps it took to find this frame.
	m_hash_stats.m_num_hash_lookup_steps += num_lookup_steps;

	// Update the maximum number of lookup steps.
	if (num_lookup_steps > m_hash_stats.m_num_hash_lookup_steps_max)
		m_hash_stats.m_num_hash_lookup_steps_max = num_lookup_steps;
	return 0;

} // CM_Hash_Table::Get_Frame_Index

/*************************************************************************/
// CM_Hash_Table::Map_Page
// Store the frame pointer in the matching hash table entry.
/*************************************************************************/
inline void CM_Hash_Table::Map_Page(I64 page_number, CM_Frame *p_frame)
{
	CT_ASSERT((p_frame->Get_Next_Hash_Entry() == 0), CM_Hash_Table::Map_Page);

	// Point to the first frame for this bucket.
	U32 hash_index = Hash(page_number);
	CM_Frame *p_first_hash_frame = m_p_hash_table[hash_index];

	// Is the first bucket zero?
	if (p_first_hash_frame == 0)
	{
		m_p_hash_table[hash_index] = p_frame;
		return;
	}

	// Save pointer to previous frame.
	CM_Frame *p_previous_frame = p_first_hash_frame;

	// Point to the next frame for this bucket.
	CM_Frame *p_next_frame = p_first_hash_frame->Get_Next_Hash_Entry();

	// Find the last frame in the list.
	while(p_next_frame)
	{
		p_previous_frame = p_next_frame;

		// Point to the next frame for this hash bucket.
		p_next_frame = p_next_frame->Get_Next_Hash_Entry();
	}

	// Save pointer to this frame in last frame in list.
	p_previous_frame->Set_Next_Hash_Entry(p_frame);

} // CM_Hash_Table::Map_Page

/*************************************************************************/
// CM_Hash_Table::Unmap_Page
// Remove the frame that matches the page_number from the list.
/*************************************************************************/
inline void CM_Hash_Table::Unmap_Page(I64 page_number)
{
	// Point to the first frame for this bucket.
	U32 hash_index = Hash(page_number);
	CM_Frame *p_first_hash_frame = m_p_hash_table[hash_index];

	CT_ASSERT((p_first_hash_frame), CM_Hash_Table::Unmap_Page);

	// Is it the first frame that we need to remove?
	if (p_first_hash_frame->Get_Page_Number() == page_number)
	{
		// Set has bucket to point to this frame.
		m_p_hash_table[hash_index] = p_first_hash_frame->Get_Next_Hash_Entry();

		// Zero next hash entry in frame being removed.
		p_first_hash_frame->Set_Next_Hash_Entry(0);
		return;
	}

	// Save pointer to previous frame so we can unlink the matching frame.
	CM_Frame *p_previous_frame = p_first_hash_frame;

	// Point to the next frame for this bucket.
	CM_Frame *p_next_frame = p_first_hash_frame->Get_Next_Hash_Entry();
	CT_ASSERT((p_next_frame), CM_Hash_Table::Unmap_Page);

	// Find the frame with the matching page number.
	while((p_next_frame->Get_Page_Number() != page_number))
	{
		p_previous_frame = p_next_frame;

		// Point to the next frame for this hash bucket.
		p_next_frame = p_next_frame->Get_Next_Hash_Entry();

		CT_ASSERT((p_next_frame), CM_Hash_Table::Unmap_Page);
	}

	// Remove frame from the list
	p_previous_frame->Set_Next_Hash_Entry(p_next_frame->Get_Next_Hash_Entry());

	// Zero next hash entry in frame being removed.
	p_next_frame->Set_Next_Hash_Entry(0);

} // CM_Hash_Table::Unmap_Page

/*************************************************************************/
// CM_Hash_Table::Hash
// This function takes the key and uses it to derive a hash code,
// which is an integer in the range [0, nBuckets - 1].  The hash
// code is computed using a method called linear congruence.  The
// choice of the value for Multiplier can have a significant effect
// on the performance of the algorithm, but not on its correctness.
/*************************************************************************/

#define Multiplier -1664117991L

inline U32 CM_Hash_Table::Hash(I64 page_number)
{
    return (U32)(((U32)page_number * (U32)Multiplier) % m_hash_table_size);

} // CM_Hash_Table::Hash

/*************************************************************************/
// CM_Hash_Table::Get_Statistics
/*************************************************************************/
inline void CM_Hash_Table::Get_Statistics(CM_Hash_Statistics *p_statistics_buffer, 
		U32 size_buffer)
{
#ifndef _WINDOWS
		CT_ASSERT(IS_ALIGNED_8(p_statistics_buffer), Get_Statistics);
#endif
		memcpy(p_statistics_buffer, &m_hash_stats, size_buffer);
		
} // CM_Hash_Table::Get_Statistics 

/*************************************************************************/
// CM_Hash_Table::Reset_Statistics
/*************************************************************************/
inline void CM_Hash_Table::Reset_Statistics()
{
	// Set stats to zero.
	memset(&m_hash_stats, 0, sizeof(CM_Hash_Statistics));

} // Reset_Statistics

/*************************************************************************/
// CM_Hash_Table::Memory_Size
/*************************************************************************/
inline U32 CM_Hash_Table::Memory_Size(CM_CONFIG *p_config)
{
	return sizeof(U32) * p_config->hash_table_size;

} // Memory_Size

#endif // CmHashTable_H
