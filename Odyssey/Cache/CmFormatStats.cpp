/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmFormatStats.cpp
// 
// Description:
// This file formats cache statistics for display.
// 
// Update Log 
// 
// 9/24/98 Jim Frandeen: Create file
/*************************************************************************/

#include "CmCache.h"
#include "CmCommon.h"
#include <stdio.h>
#include <string.h>
 
#define EOL "\n"

union Num64 {
	U64 u64;
	I64 int64;
} ;

void Print_Long_String(char *p_string);

/*************************************************************************/
// Insert_Commas
/*************************************************************************/
void Insert_Commas(char *buffer)
{
	int comma_pos = strlen(buffer) - 4;
	while (comma_pos > 4)
	{
		if (buffer[comma_pos] == ' ')
			return;

		// Move everything over one place to make room for the comma
		memcpy(buffer, (const void *)&buffer[1], comma_pos);

		// Insert comma
		buffer[comma_pos] = ',';

		comma_pos -= 4;
	}
}

/*************************************************************************/
// Format_Stats
/*************************************************************************/
void Format_Stats(char *buffer, U32 number)
{

	// Prefix the output value with a blank if the output value is signed and positive;
	// Width is 15 digits
	// u is unsigned decimal integer
	sprintf(buffer, "%15u", number);
	Insert_Commas(buffer);
}

/*************************************************************************/
// Format_Stats
/*************************************************************************/
void Format_Stats(char *buffer, U64 number)
{
	Num64 num64;

	num64.u64 = number;

	// Prefix the output value with a blank if the output value is signed and positive;
	// Width is 15 digits
	// h means use __int64
	// u is unsigned decimal integer
	sprintf(buffer, "%15hu", num64.int64);
	Insert_Commas(buffer);
}

/*************************************************************************/
// FORMAT
// Macro to format any parameter
/*************************************************************************/
#define FORMAT(stat, description) \
	buffer[0] = 0; \
	Format_Stats(buffer, (U32)stat); \
	strcat(string, buffer); \
	strcat(string, separator); \
	strcat(string, #description); \
	strcat(string, EOL);

/*************************************************************************/
// FORMAT_CONFIG
// Macro to format a config parameter
/*************************************************************************/
#define FORMAT_CONFIG(stat, description) \
	FORMAT(cache_config.stat, description)

/*************************************************************************/
// FORMAT_EVENT
// Macro to format an event
/*************************************************************************/
#define FORMAT_EVENT(stat, description) \
	FORMAT(cache_event_data.stat, description)

/*************************************************************************/
// FORMAT_STAT
// Macro to format a statistic
/*************************************************************************/
#define FORMAT_STAT(stat, description) \
	FORMAT(cache_statistics.stat, description)


/*************************************************************************/
// Display_Stats_And_Events
/*************************************************************************/
Status CM_Cache::Display_Stats_And_Events()
{
	char *string = new char[8000];
	if (string == NULL)
		return CM_ERROR(NO_MEMORY);
		
	Status status = Format_Stats_And_Events(string);
	Print_Long_String(string);
	delete [] string;
	return status;
	
} // Display_Stats_And_Events

/*************************************************************************/
// Format_Stats_And_Events
/*************************************************************************/
Status CM_Cache::Format_Stats_And_Events(char *string)
{
	// Get cache config record.
	CM_CONFIG cache_config;
	Get_Config(&cache_config, sizeof(cache_config));
		
	// Get cache event data.
	CM_EVENT_DATA cache_event_data;
	Get_Event_Data(&cache_event_data, sizeof(cache_event_data));
		
	// Get flash statistics
	CM_STATISTICS cache_statistics;
	Get_Statistics(&cache_statistics, sizeof(cache_statistics));
	
	char buffer [256];
	char separator[3] = "  "; // 2 spaces
	*string = 0;

	FORMAT_STAT(memory_used, Number of bytes of memory used by the Cache Manager.);
	FORMAT_CONFIG(page_size, Number of bytes per cache page.);
	FORMAT_CONFIG(page_table_size, Number of pages mapped by the cache.);
	FORMAT_STAT(num_page_frames, Number of page frames in the cache.);
	FORMAT_STAT(num_pages_replaceable, Number of pages that can be replaced.);
	FORMAT_STAT(num_pages_replaceable_min, Minimum number encountered of pages that can be replaced.);
	FORMAT_STAT(num_reserve_pages, Number of reserve pages in the cache.);
	FORMAT_CONFIG(dirty_page_writeback_threshold, Dirty page writeback threshold (percentage).);
	FORMAT_STAT(dirty_page_writeback_threshold, Dirty page writeback threshold (number of pages).);
	FORMAT_STAT(num_dirty_page_threshold, Number of times dirty page writeback threshold was reached.);
	FORMAT_CONFIG(dirty_page_error_threshold, Dirty page error threshold (percentage).);
	FORMAT_STAT(dirty_page_error_threshold, Dirty page error threshold (number of pages).);
	FORMAT_EVENT(num_error_max_dirty_pages, Number of times dirty page error threshold was reached.);
	FORMAT_EVENT(num_error_no_page_frame, Number of times CM_ERROR_NO_PAGE_FRAMES was returned to the user.);
	FORMAT_STAT(num_pages_working_set, Number of pages that were accessed on the last clock cycle.);
	FORMAT_STAT(num_pages_open, Number of pages currently open by CM_Open_Page.  );
	FORMAT_STAT(num_pages_open_max, Maximum number of pages ever open at one time by CM_Open_Page.  );
	FORMAT_STAT(num_pages_locked_write, Number of pages that currently have a write lock.);
	FORMAT_STAT(num_pages_locked_writeback, Number of pages that currently have a writeback lock.);
	FORMAT_STAT(num_pages_locked_read, Number of pages that currently have a read lock.);
	FORMAT_STAT(num_pages_locked_read_not_present, Number of pages that currently have a read lock not present.);
	FORMAT_STAT(num_pages_locked_remap, Number of pages that are locked for remap.);
	FORMAT_STAT(num_pages_dirty, Number of dirty pages.);
	FORMAT_STAT(num_pages_dirty_max, Maximum number of dirty pages encountered.);
	FORMAT_STAT(num_open_read, Number of calls to CM_Open_Page in CM_MODE_READ.);
	FORMAT_STAT(num_open_write, Number of calls to CM_Open_Page in CM_MODE_WRITE.);
	FORMAT_STAT(num_open_remap, Number of calls to CM_Open_Page in CM_MODE_REMAP.);
	FORMAT_STAT(num_close_page, Number of calls to CM_Close_Page.);
	FORMAT_STAT(num_abort_page, Number of calls to CM_Abort_Page.);
	FORMAT_STAT(num_write_callback, Number of calls to write callback.);
	FORMAT_EVENT(num_write_callback_failed, Number of calls to write callback that failed.);
	FORMAT_STAT(num_cache_hits, Number of cache hits.);
	FORMAT_STAT(num_cache_misses, Number of cache misses.);
	FORMAT_STAT(num_error_page_locked, Number of times CM_ERROR_PAGE_LOCKED was returned to the user.);
	FORMAT_STAT(num_dirty_again, Number of times a page frame became dirty while being written);
	FORMAT_STAT(num_waits_for_page_frame, Number of times a context was queued to wait for a page frame.);
	FORMAT_STAT(num_waiting_for_page_frame, Number of contexts currently waiting for a page frame.);
	FORMAT_STAT(num_waits_for_read_lock, Number of times a context was queued to wait for a read lock.);
	FORMAT_STAT(num_waiting_for_lock, Number of contexts currently waiting for a lock.);
	FORMAT_STAT(num_waits_for_write_lock, Number of times a context was queued to wait for a write lock.);
	FORMAT_STAT(num_waits_for_writeback_lock, Number of times a context was queued to wait for a writeback lock.);
	FORMAT_STAT(num_waiting_for_writeback_lock, Number of contexts currently waiting for a writeback locked page.);

	if (cache_config.hash_table_size)
	{
		I64 steps_per_lookup = cache_statistics.m_num_hash_lookup_steps / cache_statistics.m_num_hash_lookups;
		FORMAT_CONFIG(hash_table_size, Number of buckets in hash table.);
		FORMAT_STAT(m_num_hash_lookups, Number of hash lookups.);
		FORMAT_STAT(m_num_hash_lookup_steps, Number of hash lookup steps.);
		FORMAT(steps_per_lookup, Average number of steps per lookup.);
		FORMAT_STAT(m_num_hash_lookup_steps_max, Maximum number of hash lookup steps.);
	}

	return OK;

}  // Format_Cache_Stats

/*************************************************************************/
// 	Print_Long_String
// because printf can only handle 512 characters at one time.
/*************************************************************************/
void Print_Long_String(char *p_string)
{
	char buffer[512];
	while (*p_string != EOS)
	{
		U32 index = 0;
		while (1)
		{
			buffer[index++] = *p_string;
			if (*p_string++ == '\n')
			{
				if (*p_string == '\r')
				{
					buffer[index++] = *p_string++;
				}
				buffer[index] = EOS;
				printf(buffer);
				break;
			}
		}
	}
	
}


