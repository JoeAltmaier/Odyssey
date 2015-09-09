/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: CmCache.h
// 
// Description:
// This file defines the cache object CM_Cache used by the Cache Manager. 
// In all the interfaces, CM_CACHE_HANDLE points to a cache object.
// 
// Update Log 
// 
// 9/6/98 Jim Frandeen: Create file
// 02/08/99 Jim Frandeen: Add write through, 64-bit keys and hash
/*************************************************************************/
#if !defined(CmCache_H)
#define CmCache_H

#include "CmFrame.h"
#include "CmPageTable.h"
#include "CmFrameTable.h"
#include "CmHashTable.h"
#include "CmStats.h"
#include "Align.h"
#include "List.h"

#define CM_COOKIE  0X74877000	// JWF
#define CM_MIN_PAGE_SIZE 16

class CM_Cache {
public:

	static Status Initialize(
		CM_CONFIG				*p_config,
		CM_PREFETCH_CALLBACK	*p_prefetch_callback,
		CM_WRITE_CALLBACK		*p_write_callback,
		CM_CACHE_HANDLE 		*p_cache_handle,
		void					*callback_context);

	Status Open_Page( 
		I64 page_number,
		U32 flags,
		Callback_Context *p_callback_context,
		void **pp_page_frame,
		CM_PAGE_HANDLE *p_page_handle);

	Status Close_Page(CM_Frame_Handle frame_handle);

	Status Abort_Page(CM_Frame_Handle frame_handle);

	Status Flush_Cache(Callback_Context *p_callback_context);

	Status Set_Page_Dirty(CM_Frame_Handle frame_handle);

	Status Close(Callback_Context *p_callback_context);

	void Release_Resources();

	void Get_Event_Data(CM_EVENT_DATA *p_event_data_buffer, 
		U32 size_buffer);
	void Reset_Event_Data();

	void Get_Statistics(CM_STATISTICS *p_statistics_buffer, 
		U32 size_buffer); 
	void Reset_Statistics();

	void Get_Config(CM_CONFIG *p_config_buffer, U32 size_buffer); 

	// Validate handle returns pointer to CM_Cache object.
	// It returns zero if the handle is invalid.
	static CM_Cache *Validate_Cache_Handle(CM_CACHE_HANDLE cache_handel);

	void Obtain_Mutex();
	void Release_Mutex();

	void Find_Waiting_Contexts();
	void Find_Open_Pages();
	void static Break_Waiting_Context(Callback_Context *p_callback_context);
	void static Break_Waiting_Context(I64 m_page_number, Callback_Context *p_callback_context);
	void static Break_Open_Page(I64	m_page_number, U32 num_read_locks,
		U32 write_lock, U32 writeback_lock, U32 num_remap_locks);
	void Validate();

	// Unmap this page in the page table
	void Unmap_Page(I64 page_number);
	void Unmap_Page_Secondary(I64 page_number);

	Status Display_Stats_And_Events();
	Status Format_Stats_And_Events(char *string);
	
private: // helper methods

	// Validate configuration parameters.
	static Status Validate_Config(CM_CONFIG *p_config);

	// Returns index of Frame if page is present.
	// Return zero if page is not present.
	U32 Get_Frame_Index(I64 page_number);
	U32 Get_Frame_Index_Secondary(I64 page_number);
	U32 Get_Frame_Index(CM_Frame_Handle frame_handle);

	// Return pointer to page frame, given an index.
	void *Get_Page_Frame(U32 frame_index);
	void *Get_Page_Frame_Secondary(U32 frame_index);

	// Returns pointer to frame, given an index.
	CM_Frame *Get_Frame(U32 frame_index);
	CM_Frame *Get_Frame_Secondary(U32 frame_index);

	// Check to see if specified page is dirty.
	int Is_Page_Dirty(U32 frame_index);

	// Map this page in the page table
	void Map_Page(I64 page_number, U32 frame_index);
	void Map_Page_Secondary(I64 page_number, U32 frame_index);

	// Page is not present. Replace some other page in the cache.
	CM_Frame *Replace_Page(U32 flags, 
		Callback_Context *p_callback_context);
	CM_Frame *Replace_Page_Secondary(U32 flags, 
		Callback_Context *p_callback_context);

	Status Open_Secondary( 
		I64 page_number,
		U32 flags,
		CM_PAGE_HANDLE *p_page_handle,
		CM_Frame *p_frame);

	void Dec_Replaceable_Pages();

private:
	U32				 m_cookie;
	CM_CONFIG		 m_config;
	U32				 m_memory_used;

	// Pointer to Frame Table
	CM_Frame_Table	*m_p_frame_table;
		
	// Pointer to Frame Table for secondary cache (if used)
	CM_Frame_Table	*m_p_frame_table_2;
		
	// Pointer to Page Table
	// The page table is used for linear mapping
	CM_Page_Table	*m_p_page_table;
		
	// The hash table is used for associative mapping.
	CM_Hash_Table	 m_hash_table;

	// Pointer to Page Table for secondary cache (if used)
	CM_Page_Table	*m_p_page_table_2;
		
	// Hash table for secondary cache (if used).
	CM_Hash_Table	 m_hash_table_2;

	// Pointer to CM_Stats object
	CM_Stats		*m_p_stats;

	// Pointer to page buffer, used to exchange frames
	// when secondary cache is used.
	void			*m_p_page_buffer;

	// Is this a write through cache or a write back cache?
	U32				 m_is_write_back;

	// Memory object
	CM_Mem			 m_mem;

}; // CM_Cache

/*************************************************************************/
// CM_Cache::Dec_Replaceable_Pages
/*************************************************************************/
inline void CM_Cache::Dec_Replaceable_Pages()
{
	m_p_frame_table->Dec_Replaceable_Pages();
}

/*************************************************************************/
// CM_Cache::Destroy
/*************************************************************************/
inline Status CM_Cache::Close(Callback_Context *p_callback_context)
{
	return m_p_frame_table->Flush_Cache(p_callback_context,
		(CM_CACHE_HANDLE)this, 1 /* Close */);
}

/*************************************************************************/
// CM_Cache::Flush_Cache
/*************************************************************************/
inline Status CM_Cache::Flush_Cache(Callback_Context *p_callback_context)
{
	return m_p_frame_table->Flush_Cache(p_callback_context,
		(CM_CACHE_HANDLE)this, 0 /* Don't close */);
}

/*************************************************************************/
// CM_Cache::Get_Frame
/*************************************************************************/
inline CM_Frame *CM_Cache::Get_Frame(U32 frame_index)
{
	return m_p_frame_table->Get_Frame(frame_index);
}						 

/*************************************************************************/
// CM_Cache::Get_Frame_Secondary
/*************************************************************************/
inline CM_Frame *CM_Cache::Get_Frame_Secondary(U32 frame_index)
{
	return m_p_frame_table_2->Get_Frame(frame_index);
}						 

/*************************************************************************/
// CM_Cache::Get_Frame_Index
/*************************************************************************/
inline U32 CM_Cache::Get_Frame_Index(I64 page_number)
{
	if (m_p_page_table)
		return m_p_page_table->Get_Frame_Index(page_number);

	// We are using a hash table.
	return m_hash_table.Get_Frame_Index(page_number);
}

/*************************************************************************/
// CM_Cache::Get_Frame_Index_Secondary
/*************************************************************************/
inline U32 CM_Cache::Get_Frame_Index_Secondary(I64 page_number)
{
	if (m_p_page_table_2)
		return m_p_page_table_2->Get_Frame_Index(page_number);

	// We are using a hash table.
	return m_hash_table_2.Get_Frame_Index(page_number);
}

/*************************************************************************/
// CM_Cache::Get_Frame_Index
/*************************************************************************/
inline U32 CM_Cache::Get_Frame_Index(CM_Frame_Handle frame_handle)
{
	return frame_handle.Get_Frame_Index();
}

/*************************************************************************/
// CM_Cache::Get_Config
/*************************************************************************/
inline void CM_Cache::Get_Config(CM_CONFIG *p_config_buffer, 
	U32 size_buffer) 
{
	memcpy(p_config_buffer, &m_config, size_buffer);
}

/*************************************************************************/
// CM_Cache::Get_Event_Data
/*************************************************************************/
inline void CM_Cache::Get_Event_Data(CM_EVENT_DATA *p_event_data_buffer, 
		U32 size_buffer)
{
	m_p_stats->Get_Event_Data(p_event_data_buffer, size_buffer);
}						 

/*************************************************************************/
// CM_Cache::Get_Page_Frame
/*************************************************************************/
inline void * CM_Cache::Get_Page_Frame(U32 frame_index)
{
	return m_p_frame_table->Get_Page_Frame(frame_index);
}

/*************************************************************************/
// CM_Cache::Get_Page_Frame_Secondary
/*************************************************************************/
inline void * CM_Cache::Get_Page_Frame_Secondary(U32 frame_index)
{
	return m_p_frame_table_2->Get_Page_Frame(frame_index);
}

/*************************************************************************/
// CM_Cache::Get_Statistics
/*************************************************************************/
inline void CM_Cache::Get_Statistics(CM_STATISTICS *p_statistics_buffer, 
		U32 size_buffer)
{
	CM_Hash_Statistics hash_statistics;
	m_p_stats->Get_Statistics(p_statistics_buffer, size_buffer);
	m_hash_table.Get_Statistics(&hash_statistics, sizeof(hash_statistics));
	p_statistics_buffer->m_num_hash_lookups = hash_statistics.m_num_hash_lookups;
	p_statistics_buffer->m_num_hash_lookup_steps = hash_statistics.m_num_hash_lookup_steps;
	p_statistics_buffer->m_num_hash_lookup_steps_max = hash_statistics.m_num_hash_lookup_steps_max;
}						 

/*************************************************************************/
// CM_Cache::Is_Page_Dirty
/*************************************************************************/
inline int CM_Cache::Is_Page_Dirty(U32 frame_index)
{
	return m_p_frame_table->Is_Page_Dirty(frame_index);
}

/*************************************************************************/
// CM_Cache::Map_Page
/*************************************************************************/
inline void CM_Cache::Map_Page(I64 page_number, U32 frame_index)
{
	if (m_p_page_table)
		m_p_page_table->Map_Page(page_number, frame_index);
	else
		m_hash_table.Map_Page(page_number, Get_Frame(frame_index));
}

/*************************************************************************/
// CM_Cache::Map_Page_Secondary
/*************************************************************************/
inline void CM_Cache::Map_Page_Secondary(I64 page_number, U32 frame_index)
{
	if (m_p_page_table_2)
		m_p_page_table_2->Map_Page(page_number, frame_index);
	else
		m_hash_table_2.Map_Page(page_number, Get_Frame_Secondary(frame_index));
}

/*************************************************************************/
// CM_Cache::Unmap_Page
/*************************************************************************/
inline void CM_Cache::Unmap_Page(I64 page_number)
{
	if (m_p_page_table)
		m_p_page_table->Unmap_Page(page_number);
	else m_hash_table.Unmap_Page(page_number);
}

/*************************************************************************/
// CM_Cache::Unmap_Page_Secondary
/*************************************************************************/
inline void CM_Cache::Unmap_Page_Secondary(I64 page_number)
{
	if (m_p_page_table_2)
		m_p_page_table_2->Unmap_Page(page_number);
	else m_hash_table_2.Unmap_Page(page_number);
}

/*************************************************************************/
// CM_Cache::Obtain_Mutex
/*************************************************************************/
inline void CM_Cache::Obtain_Mutex()
{
	m_p_frame_table->Obtain_Mutex();
}

/*************************************************************************/
// CM_Cache::Release_Mutex
/*************************************************************************/
inline void CM_Cache::Release_Mutex()
{
	m_p_frame_table->Release_Mutex();
}

/*************************************************************************/
// CM_Cache::CM_Reset_Event_Data
/*************************************************************************/
inline void CM_Cache::Reset_Event_Data()
{
	m_p_stats->Reset_Event_Data();
}

/*************************************************************************/
// CM_Cache::Set_Page_Dirty
/*************************************************************************/
inline Status CM_Cache::Set_Page_Dirty(CM_Frame_Handle frame_handle)
{
	return m_p_frame_table->Set_Page_Dirty(frame_handle.Get_Frame_Index());
}

/*************************************************************************/
// CM_Cache::CM_Reset_Statistics
/*************************************************************************/
inline void CM_Cache::Reset_Statistics()
{
	// Reset from the page table first so it can initialize statistics.
	m_p_frame_table->Reset_Statistics();
	m_hash_table.Reset_Statistics();

	// Tell statistics object how much memory we use.
	m_p_stats->Set_Memory_Used(m_memory_used);
}

/*************************************************************************/
// CM_Cache::Replace_Page
/*************************************************************************/
inline CM_Frame *CM_Cache::Replace_Page(U32 flags,
										Callback_Context *p_callback_context)
{
	return m_p_frame_table->Replace_Page(flags, p_callback_context);
}

/*************************************************************************/
// CM_Cache::Replace_Page_Secondary
/*************************************************************************/
inline CM_Frame *CM_Cache::Replace_Page_Secondary(U32 flags,
										Callback_Context *p_callback_context)
{
	return m_p_frame_table_2->Replace_Page(flags, p_callback_context);
}

/*************************************************************************/
// CM_Cache::Validate_Cache_Handle
/*************************************************************************/
inline CM_Cache *CM_Cache::Validate_Cache_Handle(CM_CACHE_HANDLE cache_handle)
{
	if ((CM_Cache *)cache_handle == 0)
		return 0;
	if (((CM_Cache *)cache_handle)->m_cookie != CM_COOKIE)
		return 0;
	return (CM_Cache *)cache_handle;
}

#endif // CmFrame_H
