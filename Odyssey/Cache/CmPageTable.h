/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: CmPageTable.h
// 
// Description:
// This file defines the Page Table of the Cache Manager. 
// The page table is used to map page numbers to page frames in memory.  
// The page table is an array with one entry per page.  The page table is 
// indexed by the page number.  The entry is zero if is the page is not
// present in the page cache; otherwise, the entry contains the index of
// the page frame that contains the page.  

// The page table can be an array of 16-bit halfword entries, or an array
// of 32-bit word entries, depending on the number of page frames in the
// system.  If the number of page frames is greater than 65,535, then word
// entries are required. 

// Update Log 
// 
// 9/6/98 Jim Frandeen: Create file
// 02/08/99 Jim Frandeen: Add write through, 64-bit keys and hash
/*************************************************************************/
#if !defined(CmPageTable_H)
#define CmPageTable_H

#include "Cache.h"
#include "List.h"
#include "CmFrame.h"
#include "CmMem.h"

class CM_Page_Table 
{

public: // methods

	// Allocate and initialize object
	static CM_Page_Table *Allocate(CM_Mem *p_mem, CM_CONFIG *p_config);

	// Returns index of Frame if page is present.
	// Return zero if page is not present.
	U32 Get_Frame_Index(I64 page_number);
	
	// Map this page in the page table.
	void Map_Page(I64 page_number, U32 frame_index);
	
	// Unmap this page in the page table.
	void Unmap_Page(I64 page_number);
	
	// Return memory size required by page table.
	static U32 Memory_Size(CM_CONFIG *p_config) ;

private: // members

	U32		 m_num_pages;

	// Our page table will use either word entries or halfword entries
	// depending on how many page frames we have.
	U32		*m_p_word_table;
	U16		*m_p_halfword_table;
		
}; // CM_Page_Table

/*************************************************************************/
// CM_Page_Table::Get_Frame_Index
// Return the frame index for the specified page number.
// Return zero if page is not present.
/*************************************************************************/
inline U32 CM_Page_Table::Get_Frame_Index(I64 page_number)
{
	if (m_p_halfword_table)
		return *(m_p_halfword_table + page_number);

	return *(m_p_word_table + page_number);

} // CM_Page_Table::Get_Frame_Index

/*************************************************************************/
// CM_Page_Table::Map_Page
// Store the frame_index in the specified table entry.
/*************************************************************************/
inline void CM_Page_Table::Map_Page(I64 page_number, U32 frame_index)
{
	if (m_p_halfword_table)
		*(m_p_halfword_table + page_number) = (U16)frame_index;
	else 
		*(m_p_word_table + page_number) = frame_index;

} // CM_Page_Table::Map_Page

/*************************************************************************/
// CM_Page_Table::Unmap_Page
// Store 0 in the specified table entry.
/*************************************************************************/
inline void CM_Page_Table::Unmap_Page(I64 page_number)
{
	if (m_p_halfword_table)
		*(m_p_halfword_table + page_number) = 0;
	else 
		*(m_p_word_table + page_number) = 0;

} // CM_Page_Table::Unmap_Page

#endif // CmPageTable_H
