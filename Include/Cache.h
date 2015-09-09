/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: CacheManager.h
// 
// Description:
// This file defines the interface to the Cache Manager. 
// 
// Update Log 
// 
// 7/6/98 Jim Frandeen: Create file
// 2/3/99 Jim Frandeen: Align I64 variables.
/*************************************************************************/
#if !defined(Cache_H)
#define Cache_H

#include "Simple.h"

class Callback_Context;

/*************************************************************************/
// CM_CONFIG
// Cache Manager configuration record.
// A pointer to this record is passed in to CM_Initialize.
// The record can be retrieved by the CM_Get_Config method.
// The idea is that the configuration options will change over time 
// and be backwards compatible, but the interface to the Cache Manager 
// will not change.
// Version 3: secondary cache
/*************************************************************************/
#define CM_CONFIG_VERSION 3

typedef struct {

	// version of CM_CONFIG record
	U32	version;
	
	// Size of CM_CONFIG record in bytes.
	// This need not be set by FF_Initialize, but it is returned 
	// by CM_Get_Config.
	U32 size;

	// Number of bytes per cache page.
	U32	page_size;

	// Number of pages in the cache.
	U32 num_pages;
	
	// Pointer to memory to be used for pages.
	// The user must have allocated page_size * num_pages bytes of memory.
	void 					*p_page_memory; 

	// Number of pages in the secondary cache -- 0 if secondary not used.
	U32 num_pages_secondary;
	
	// Pointer to memory to be used for pages for secondary cache
	// 0 if secondary cache not used.
	// The user must have allocated page_size * num_pages_secondary bytes of memory.
	void 					*p_page_memory_secondary; 

	// Pointer to memory to be used for internal cache manager tables.
	// The user must have allocated CM_Get_Memory_Size_Required() bytes of memory.
	void 					*p_table_memory; 

	// page_table_size specifies the number of entries in the page table 
	// for a cache that uses linear mapping.
	U32	page_table_size;
	
	// hash_table_size specifies the number of entries in the hash table 
	// for a cache that uses associative mapping.  page_table_size and 
	// hash_table_size are mutually exclusive.
	U32	hash_table_size;
	
	// Number of reserve pages in the cache for CM_Open_Page with
	// CM_PRIORITY_RESERVE.
	U32	num_reserve_pages;
	
	// Dirty page writeback threshold.
	// When dirty page writeback threshold is reached, user's CM_WRITE_CALLBACK
	// is called to write out a dirty page.
	// This number is a percentage of the total page frames, which must be
	// less than 100.
	U32	dirty_page_writeback_threshold;

	// Dirty page error threshold.
	// When dirty page error threshold is reached, 
	// CM_ERROR_MAX_DIRTY_PAGES is returned.
	// This number is a percentage of the total page frames, 
	// which can be 100.
	U32	dirty_page_error_threshold;

	// Number of pages to prefetch forward when a cache miss is encountered.
	U32	num_prefetch_forward;

	// Number of pages to prefetch backward when a cache miss is encountered.
	U32	num_prefetch_backward;
	
} CM_CONFIG;

/*************************************************************************/
// CM_EVENT_DATA
// Cache Manager event data record.
// This describes information indicating errors, events, 
// and possibly health conditions. 
// This record can be retrieved with the CM_Get_Event_Data method.
/*************************************************************************/
#define CM_EVENT_DATA_VERSION 2

typedef struct {

	// Version of CM_EVENT_DATA record.
	U32	version;
	
	// Number of bytes in record
	U32	size;
	
	// Number of times CM_ERROR_MAX_DIRTY_PAGES was returned to the user.
	I64			num_error_max_dirty_pages;
	
	// Number of times CM_ERROR_NO_PAGE_FRAMES was returned to the user.
	I64			num_error_no_page_frame;
	
	// Number of calls to user's CM_WRITE_CALLBACK that failed.
	I64			num_write_callback_failed;
	
} CM_EVENT_DATA;

/*************************************************************************/
// CM_STATISTICS
// Cache Manager statistics record.
// This record can be retrieved with the CM_Get_Statistics method.
/*************************************************************************/
#define CM_STATISTICS_VERSION 2

typedef struct {

	// Version of CM_STATISTICS record.
	U32	version;
	
	// Number of bytes in record
	U32	size;

	// N.B. Put all of the I64 members first for alignment.
	
	// Number of times dirty page threshold was reached.
	I64			num_dirty_page_threshold;

	// Number of calls to CM_Open_Page in CM_MODE_READ.
	I64			num_open_read;
	
	// Number of calls to CM_Open_Page in CM_MODE_WRITE.
	I64			num_open_write;
	
	// Number of calls to CM_Open_Page in CM_MODE_REMAP.
	I64			num_open_remap;
	
	// Number of calls to CM_Close_Page.
	I64			num_close_page;
	
	// Number of calls to CM_Abort_Page.
	I64			num_abort_page;
	
	// Number of calls to user's CM_WRITE_CALLBACK.
	I64			num_write_callback;
	
	// Number of calls CM_PREFETCH_CALLBACK to prefetch a page forward.
	I64			num_prefetch_forward;
	
	// Number of pages prefetched forward that were used.
	I64			num_prefetch_forward_used;

	// Number of pages prefetched forward that were never used.  These pages were reclaimed by
	// the Clock algorithm and reassigned.  Note that in general, num_prefetch_forward is 
	// greater than the sum of num_prefetch_forward_used and num_prefetch_forward_unused because
	// some pages are usually still marked not-present and prefaulted in the page table.  
	// That is, the pages have not been used, and they have not been taken for replacement
	// pages by the Clock algorithm.
	I64			num_prefetch_forward_unused;
	
	// Number of calls CM_PREFETCH_CALLBACK to prefetch a page backward.
	I64			num_prefetch_backward;
	
	// Number of pages prefetched backward that were used.
	I64			num_prefetch_backward_used;

	// Number of pages prefetched backward that were never used.  
	I64			num_prefetch_backward_unused;
	
	// Number of cache hits.
	I64			num_cache_hits;
	
	// Number of times CM_ERROR_CACHE_MISS was returned to the user.
	I64			num_cache_misses;
	
	// Number of times CM_ERROR_PAGE_LOCKED was returned to the user.
	I64			num_error_page_locked;
	
	// Number of times a page frame became dirty again while it was
	// being written.
	I64			num_dirty_again;
	
	// Number of times a context was queued to wait for a page frame.
	I64			num_waits_for_page_frame;
	
	// Number of times a context was queued to wait for a read lock.
	I64			num_waits_for_read_lock;
	
	// Number of times a context was queued to wait for a write lock.
	I64			num_waits_for_write_lock;
	
	// Number of times a context was queued to wait for a writeback lock.
	I64			num_waits_for_writeback_lock;
	
	// Number of times a context was queued to wait for a remap lock.
	I64			num_waits_for_remap_lock;
	
	// Number of times a flush context was queued to wait for a lock.
	I64			num_flush_waits_for_lock;
	
	// Number of lookups for associative cache.
	I64			  m_num_hash_lookups;

	// Number of lookup steps for associative cache. 
	// Number of steps per lookup is m_num_hash_lookup_steps / m_num_hash_lookups.
	I64			  m_num_hash_lookup_steps;

	// Number of flush contexts currently waiting for a lock.
	// This is only used when the cache is flushed.
	U32			num_flush_waiting_for_lock;
	
	// Number of contexts currently waiting for a lock: -- 
	// a write lock, a writeback lock, a read lock, or a remap lock.
	U32			num_waiting_for_lock;
	
	// Number of bytes of memory used by the Cache Manager.
	U32	memory_used;
	
	// Minimum number of reserved pages.  Could be smaller than
	// config.num_reserve_pages if adjusted to be smaller than
	// number of page frames.
	U32	num_reserve_pages;
		
	// Dirty page writeback threshold calculated from config.dirty_page_writeback_threshold.
	// Could be smaller than amount specified in config if necessary to make
	// it smaller than maximum number of dirty pages.
	U32	dirty_page_writeback_threshold;
		
	// Dirty page error threshold calculated from config.dirty_page_error_threshold.
	// Could be smaller than amount specified in config if necessary to make
	// it smaller than m_dirty_page_writeback_threshold.
	U32	dirty_page_error_threshold;

	// Number of page frames in the cache.
	U32	num_page_frames;
	
	// Number of the number of pages that were either accessed or locked the
	// last time the clock hand went all the way around.  The replacement
	// algorithm is called the Clock algorithm.  System page frames are
	// arranged in a circular list (like the circumference of a clock).  The
	// clock pointer (or hand) points at the last page replaced and moves
	// clockwise when the algorithm is invoked to find the next replaceable
	// page.  If a page has been accessed or locked, it is considered to be
	// part of the working set.
	U32	num_pages_working_set;

	// Number of pages currently open by CM_Open_Page.  
	U32	num_pages_open;
	
	// Maximum number of pages ever open at one time by CM_Open_Page.  
	U32	num_pages_open_max;
	
	// Number of pages that currently have a write lock.  
	U32	num_pages_locked_write;
	
	// Number of pages that currently have a writeback lock.  
	U32	num_pages_locked_writeback;
	
	// Number of pages that currently have a read lock.  
	U32	num_pages_locked_read;
	
	// Number of pages that currently have a remap lock.  
	U32	num_pages_locked_remap;
	
	// Number of pages that currently have a read lock not present.  
	U32	num_pages_locked_read_not_present;
	
	// Number of pages that can be replaced.
	U32	num_pages_replaceable;
	
	// Minimum number encountered of pages that can be replaced.
	U32	num_pages_replaceable_min;
	
	// Number of dirty pages.  
	U32	num_pages_dirty;
	
	// Maximum number of dirty pages encountered.
	U32	num_pages_dirty_max;
	
	// Number of contexts currently waiting for a page frame.
	U32	num_waiting_for_page_frame;
	
	// Number of contexts currently waiting for a writeback lock.
	U32			num_waiting_for_writeback_lock;
	
	// Max number of lookup steps for associative cache.
	U32			  m_num_hash_lookup_steps_max;
} CM_STATISTICS;

/*************************************************************************/
// CM_ERROR codes begin with CM_ERROR_BASE to distinguish them
// from Nucleus error codes defined in Nucleus.h.
/*************************************************************************/
#define CM_ERROR_BASE 4000
typedef enum
{
	CM_ERROR_INVALID_CONFIG_DATA 			= CM_ERROR_BASE + 1,
	CM_ERROR_INVALID_CONFIG_VERSION			= CM_ERROR_BASE + 2,
	CM_ERROR_INVALID_CACHE_HANDLE			= CM_ERROR_BASE + 3,
	CM_ERROR_INVALID_PAGE_HANDLE			= CM_ERROR_BASE + 4,
	CM_ERROR_CACHE_MISS						= CM_ERROR_BASE + 5,
	CM_ERROR_PAGE_LOCKED					= CM_ERROR_BASE + 6,
	CM_ERROR_NO_PAGE_FRAMES					= CM_ERROR_BASE + 7,
	CM_ERROR_MAX_DIRTY_PAGES				= CM_ERROR_BASE + 8,
	CM_ERROR_INVALID_MODE					= CM_ERROR_BASE + 9,
	CM_ERROR_NOT_IMPLEMENTED				= CM_ERROR_BASE + 10,
	CM_ERROR_INVALID_PAGE_NUMBER			= CM_ERROR_BASE + 11,
	CM_ERROR_INTERNAL						= CM_ERROR_BASE + 12,
	CM_ERROR_INVALID_MEMORY_SIZE			= CM_ERROR_BASE + 13,
	CM_ERROR_ALL_PAGES_LOCKED				= CM_ERROR_BASE + 14,
	CM_ERROR_DIRTY_PAGES					= CM_ERROR_BASE + 15,
	CM_ERROR_CONFIG_MEMORY_OVERLAP			= CM_ERROR_BASE + 16,
	CM_ERROR_NO_MEMORY						= -32 // same as NU_NO_MEMORY
} CM_ERROR;

// Cache Manager Handle is really a pointer to a Cache Manager context
typedef void *CM_CACHE_HANDLE;

// Page Handle is really a handle to a Page Access object.
typedef  U32 CM_PAGE_HANDLE;

#define CM_NULL_PAGE_HANDLE (U32)-1

// The write callback is called by the Cache Manager when it needs to
// write a page from the cache to the user's storage device.
typedef Status CM_WRITE_CALLBACK(
	void *p_callback_context, 
	I64 page_number,
	void *p_page_frame,
	CM_PAGE_HANDLE page_handle);
	
// The prefetch callback is called by the Cache Manager when it wants to
// prefetch a page.
typedef Status CM_PREFETCH_CALLBACK(
	void *p_callback_context, 
	U32 page_number,
	void *p_page_frame,
	CM_PAGE_HANDLE page_handle);
	
// Values for flags word in Open Page
#define CM_PRIORITY_RESERVE			0x0100
#define CM_CALLBACK_IF_NO_FRAME		0x0200
#define CM_CALLBACK_IF_LOCKED		0x0400
#define CM_SECONDARY				0x0800
#define CM_OPEN_MODE_READ			0x0FE
#define CM_OPEN_MODE_WRITE			0x0FD
#define CM_OPEN_MODE_REMAP			0x0FC

// Initialize the Cache Manager object and return a handle.
Status CM_Initialize(
	CM_CONFIG				*p_config,
	CM_PREFETCH_CALLBACK	*p_prefetch_callback,
	CM_WRITE_CALLBACK		*p_write_callback,
	CM_CACHE_HANDLE 		*p_cache_handle,
	void					*p_callback_context);

// Open Page gets read or write access to a page in the cache unless the page is already
// locked by another operation. If a cache hit is reported, a pointer to the page frame is
// returned.  If a cache miss is reported, a pointer to an empty page frame is returned.  
// The page is locked until the user calls Close Page.
Status CM_Open_Page(CM_CACHE_HANDLE cache_handle, 
	I64 page_number,
	U32 flags,
	Callback_Context *p_callback_context,
	void **pp_page_frame,
	CM_PAGE_HANDLE *p_page_handle);

// The user calls Close Page when the read or write operation has completed.  
// If any contexts are waiting for the page, the Cache Manager calls 
// their Notify callback at this time.
Status CM_Close_Page(CM_CACHE_HANDLE handle, CM_PAGE_HANDLE page_handle); 

// The user calls Abort Page when the read or write operation has completed.
// The page has not been modified.
Status CM_Abort_Page(CM_CACHE_HANDLE handle, CM_PAGE_HANDLE page_handle); 

// The user calls Set_Page_Dirty when a page opened for read has been
// modified to correct an ECC error.
Status CM_Set_Page_Dirty(CM_CACHE_HANDLE handle, CM_PAGE_HANDLE page_handle); 

// Get statistics record.
Status CM_Get_Statistics(CM_CACHE_HANDLE handle,
	CM_STATISTICS *p_statistics_buffer, 
	U32 size_buffer); 

// Reset statistics record.
Status CM_Reset_Statistics(CM_CACHE_HANDLE handle);

// Get event data record.
Status CM_Get_Event_Data(CM_CACHE_HANDLE handle,
	CM_EVENT_DATA *p_event_data_buffer, 
	U32 size_buffer); 

// Reset event data record.
Status CM_Reset_Event_Data(CM_CACHE_HANDLE handle);

// Get configuration record.
Status CM_Get_Config(CM_CACHE_HANDLE handle,
	CM_CONFIG *p_config_buffer, 
	U32 size_buffer); 

// Flush cache.  The flush callback method will be called with a status
// when the flush operation has completed.
Status CM_Flush_Cache(CM_CACHE_HANDLE handle, Callback_Context *p_callback_context);

// Flush the cache, close the Cache Manager object and release all resources.	
Status CM_Close_Cache(CM_CACHE_HANDLE handle, Callback_Context *p_callback_context);

// Return the number of bytes of memory required.
Status CM_Get_Memory_Size_Required(CM_CONFIG *p_config, U32 *p_memory_size);

// Display cache statistics and events.
Status CM_Display_Stats_And_Events(CM_CACHE_HANDLE handle);

// Format cache statistics and events into string for display.
Status CM_Format_Stats_And_Events(CM_CACHE_HANDLE handle, char *p_string);


#endif // Cache_H

