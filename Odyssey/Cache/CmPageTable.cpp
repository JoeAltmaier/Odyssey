/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmPageTable.cpp
// 
// Description:
// This file implements the CM_Page_Table class methods of the 
// Cache Manager 
// 
// Update Log 
// 
// 9/9/98 Jim Frandeen: Create file
// 02/08/99 Jim Frandeen: Add write through, 64-bit keys and hash
/*************************************************************************/

#include "CmPageTable.h"
#include "CmMem.h"
#include <String.h>

/*************************************************************************/
// CM_Page_Table::Allocate
// Allocate and initialize the page table.
// Return pointer to allocated page table or zero if table
// could not be allocated.
/*************************************************************************/
CM_Page_Table * CM_Page_Table::Allocate(CM_Mem *p_mem, CM_CONFIG *p_config) 
{
	// Allocate CM_Page_Table object
	CM_Page_Table *p_page_table = 
		(CM_Page_Table *)p_mem->Allocate(sizeof(CM_Page_Table));

	if (p_page_table == 0)
		return 0;
	
	p_page_table->m_p_halfword_table = 0;
	p_page_table->m_p_word_table = 0;
	
	// How many entries do we need to allocate?
	p_page_table->m_num_pages = p_config->page_table_size;
	if (p_page_table->m_num_pages > 65536)
	{
		// Our page table will need one-word entries
		p_page_table->m_p_word_table = 
			(U32 *)p_mem->Allocate(sizeof(U32) * p_page_table->m_num_pages);
		if (p_page_table->m_p_word_table == 0)
			return 0;

		// Set table to zero -- no pages present
		memset(p_page_table->m_p_word_table, 0, (sizeof(U32) * p_page_table->m_num_pages));
	} // full word entries
	else
	{
		// Our page table can use half word entries
		p_page_table->m_p_halfword_table = 
			(U16 *)p_mem->Allocate(sizeof(U16) * p_page_table->m_num_pages);
		if (p_page_table->m_p_halfword_table == 0)
			return 0;

		// Set table to zero -- no pages present
		memset(p_page_table->m_p_halfword_table, 0, (sizeof(U16) * p_page_table->m_num_pages));
	} // half word entries

	// Return pointer to page table object
	return p_page_table;

} //CM_Page_Table::Allocate

/*************************************************************************/
// CM_Page_Table::Memory_Size
// Calculate the memory size required for the page table.
/*************************************************************************/
U32 CM_Page_Table::Memory_Size(CM_CONFIG *p_config) 
{
	U32 memory_size = sizeof(CM_Page_Table) + 16; // + 2 times alignment

	// How many entries do we need to allocate?
	U32 num_pages = p_config->page_table_size;
	if (num_pages > 65536)

		// Our page table will need one-word entries
		memory_size += (sizeof(U32) * num_pages);
	else

		// Our page table can use half word entries
		memory_size += (sizeof(U16) * num_pages);

	// Return number of bytes of memory required.
	return memory_size;

} //CM_Page_Table::Memory_Size

