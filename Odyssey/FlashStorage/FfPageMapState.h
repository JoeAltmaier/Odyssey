/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FfPageMapState.h
// 
// Description:
// This file defines the page map state object. 
// This object keeps track of dirty pages in the page map.
// 
// 9/07/99 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FfPageMapState_H)
#define FfPageMapState_H

#include "FfCommon.h"
#include <stdlib.h>
#include <string.h>

/*************************************************************************/
// FF_Page_Map_State
/*************************************************************************/
class FF_Page_Map_State
{
public:
	
	// Allocate all internal structures.
	Status Allocate(FF_Mem *p_mem);

	// Open -- set all pages clean.
	void Set_Page_Map_Clean();

	// Open -- set all pages dirty.
	void Set_Page_Map_Dirty();

	static U32 Pages_Per_Page_Map();

	static U32 Memory_Size();

	void Set_Page_Map_Clean(U32 page_index);
	void Set_Page_Map_Dirty(U32 page_index);
	char Is_Page_Map_Dirty(U32 page_index);
	char Is_Page_Map_Dirty();

private: // member data
	
	U32								 m_pages_per_page_map;

	// Pointer to array of bytes, one for each page of the page map.
	// This is used to keep track of dirty page map pages as the page
	// map is being closed.
	char							*m_p_dirty_map;

	char							 m_is_open;

}; // FF_Page_Map_State


/*************************************************************************/
// FF_Page_Map_State::Allocate
/*************************************************************************/
inline Status FF_Page_Map_State::Allocate(FF_Mem *p_mem)
{
	m_is_open = 0;
	m_p_dirty_map = 0;

	// Allocate array of bytes, one for each page of the page map.
	// This is used to keep track of dirty page map pages as the page
	// map is being closed.
	m_pages_per_page_map = Pages_Per_Page_Map();
	m_p_dirty_map = (char *)p_mem->Allocate(m_pages_per_page_map);
	if (m_p_dirty_map == 0)
		return FF_NO_MEMORY;
	return OK;
}

/*************************************************************************/
// FF_Page_Map_State::Is_Page_Map_Dirty
/*************************************************************************/
inline char FF_Page_Map_State::Is_Page_Map_Dirty(U32 page_index)
{
	CT_ASSERT((page_index < m_pages_per_page_map), Set_Page_Map_Dirty);
	return *(m_p_dirty_map + page_index);
}

/*************************************************************************/
// FF_Page_Map_State::Is_Page_Map_Dirty
// Return true if any page is dirty.
/*************************************************************************/
inline char FF_Page_Map_State::Is_Page_Map_Dirty()
{
	for (U32 index = 0; index < m_pages_per_page_map; index++)
	{
		if (*(m_p_dirty_map + index))
			return 1;
	}

	return 0;
}

/*************************************************************************/
// FF_Page_Map_State::Memory_Size
/*************************************************************************/
inline U32 FF_Page_Map_State::Memory_Size()
{
	return Pages_Per_Page_Map();
}

/*************************************************************************/
// FF_Page_Map_State::Set_Page_Map_Clean
/*************************************************************************/
inline void FF_Page_Map_State::Set_Page_Map_Clean()
{
	// Set every page clean.  Pages will be set dirty as they are modified.
	ZERO(m_p_dirty_map, m_pages_per_page_map);
	m_is_open = 1;
}

/*************************************************************************/
// FF_Page_Map_State::Set_Page_Map_Dirty
/*************************************************************************/
inline void FF_Page_Map_State::Set_Page_Map_Dirty()
{
	// Set every page dirty.  Pages will be set clean as they are written.
	memset(m_p_dirty_map, 1, m_pages_per_page_map);
	m_is_open = 1;
}

/*************************************************************************/
// FF_Page_Map_State::Pages_Per_Page_Map
/*************************************************************************/
inline U32 FF_Page_Map_State::Pages_Per_Page_Map()
{
	// Calculate how many bytes to store the page map (4 meg today)
	U32 bytes_per_map = Flash_Address::Num_Virtual_Pages() * 
		FF_BYTES_PER_PAGE_MAP_ENTRY;
	CT_ASSERT((FF_BYTES_PER_PAGE_MAP_ENTRY == 4), Pages_Per_Page_Map);

	// Calculate how many pages to store the page map (256 today)
	return
		(bytes_per_map + Flash_Address::Bytes_Per_Page() - 1)
		/ Flash_Address::Bytes_Per_Page();
}

/*************************************************************************/
// FF_Page_Map_State::Set_Page_Map_Dirty
/*************************************************************************/
inline void FF_Page_Map_State::Set_Page_Map_Dirty(U32 page_index)
{
	// Don't waste time if object is not open.
	if (m_is_open == 0)
		return;

	CT_ASSERT((page_index < m_pages_per_page_map), Set_Page_Map_Dirty);
	*(m_p_dirty_map + page_index) = 1;
}

/*************************************************************************/
// FF_Page_Map_State::Set_Page_Map_Clean
/*************************************************************************/
inline void FF_Page_Map_State::Set_Page_Map_Clean(U32 page_index)
{
	CT_ASSERT((page_index < m_pages_per_page_map), Set_Page_Map_Clean);
	*(m_p_dirty_map + page_index) = 0;
}

#endif // FfPageMapState_H