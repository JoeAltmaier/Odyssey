/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfFormatStats.cpp
// 
// Description:
// This file formats Flash Storage statistics for display.
// 
// 12/4/98 Jim Frandeen: Create file
/*************************************************************************/

#include "FfInterface.h"
#include <stdio.h>
#include <string.h>
 
union Num64 {
	U64 u64;
	I64 i64;
} ;

#define EOL "\n"

/*************************************************************************/
// Externals from FormatCachStats
/*************************************************************************/
void Insert_Commas(char *buffer);
void Format_Stats(char *buffer, U32 number);
void Format_Stats(char *buffer, U64 number);
void Print_Long_String(char *p_string);

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
	FORMAT(flash_config.stat, description)

/*************************************************************************/
// FORMAT_EVENT
// Macro to format an event
/*************************************************************************/
#define FORMAT_EVENT(stat, description) \
	FORMAT(flash_event_data.stat, description)

/*************************************************************************/
// FORMAT_STAT
// Macro to format a statistic
/*************************************************************************/
#define FORMAT_STAT(stat, description) \
	FORMAT(flash_statistics.stat, description)

/*************************************************************************/
// Display_Cache_Stats_And_Events
/*************************************************************************/
Status FF_Interface::Display_Cache_Stats_And_Events()
{
	return CM_Display_Stats_And_Events(m_cache_handle);
	
} // Display_Cache_Stats_And_Events

/*************************************************************************/
// Format_Cache_Stats_And_Events
/*************************************************************************/
Status FF_Interface::Format_Cache_Stats_And_Events(char *string)
{
	return CM_Format_Stats_And_Events(m_cache_handle, string);
	
} // Format_Cache_Stats_And_Events

/*************************************************************************/
// Display_Stats_And_Events
/*************************************************************************/
Status FF_Interface::Display_Stats_And_Events()
{
	char *string = new char[8000];
	if (string == NULL)
		return CTS_FLASH_NO_MEMORY;
		
	Status status = Format_Stats_And_Events(string);
	Print_Long_String(string);
	delete [] string;
	return status;
	
} // Display_Stats_And_Events

/*************************************************************************/
// Format_Stats_And_Events
/*************************************************************************/
Status FF_Interface::Format_Stats_And_Events(char *string)
{
	// Get flash config record.
	FF_CONFIG flash_config;
	Status status = Get_Config(&flash_config, sizeof(flash_config));
	if (status != OK)
		return OK;
		
	// Get flash event data.
	FF_EVENT_DATA flash_event_data;
	status = Get_Event_Data(&flash_event_data, sizeof(flash_event_data));
	if (status != OK)
		return OK;
		
	// Get flash statistics
	FF_STATISTICS flash_statistics;
	status = Get_Statistics(&flash_statistics, sizeof(flash_statistics));
	if (status != OK)
		return OK;
	
	char buffer [256];
	char separator[3] = "  "; // 2 spaces
	*string = 0;

	FORMAT_STAT(page_size, Size of flash page.);
	FORMAT_STAT(num_total_pages, Number of flash pages.);
	FORMAT_STAT(num_user_pages, Number of user pages.);

	FORMAT_STAT(num_erased_pages_allocated, Number of erased pages allocated.);
	FORMAT_STAT(num_erased_pages_available, Number of erased pages available.);
	FORMAT_STAT(erased_page_threshold, Erased page threshold.);
	FORMAT_STAT(num_erased_page_threshold, Number of times erase threshold was exceeded.);

	FORMAT_STAT(num_bad_blocks, Number of bad blocks.);
	FORMAT_STAT(num_initial_bad_pages, Number of initial bad pages encountered during format.);
	FORMAT_STAT(num_replacement_pages_allocated, Number of replacement pages allocated.);
	FORMAT_STAT(num_replacement_pages_available, Number of replacement pages available.);
	FORMAT_STAT(replacement_page_threshold, Replacement page threshold.);
	
	FORMAT_STAT(num_page_blocks_erased, Number of page blocks erased.);
	FORMAT_STAT(wear_level_threshold, Wear level page threshold.);
	FORMAT_STAT(num_wear_level_threshold, Number of times block wear level erase was started.);

	FORMAT_STAT(num_waits_buss, Number times the operation had to wait for the buss.);
	FORMAT_STAT(num_waiting_buss, Number of contexts currently waiting for the buss.);
	FORMAT_STAT(num_waits_unit[0], Number of times the operation had to wait for unit 0.);
	FORMAT_STAT(num_waiting_unit[0], Number of contexts currently waiting for unit 0.);
	FORMAT_STAT(num_waits_unit[1], Number of times the operation had to wait for unit 1.);
	FORMAT_STAT(num_waiting_unit[1], Number of contexts currently waiting for unit 1.);
	FORMAT_STAT(num_waits_erased_page, Number times the operation had to wait for an erased page.);
	FORMAT_STAT(num_waiting_erased_page, Number of contexts currently waiting for an erased page.);

	FORMAT_EVENT(num_error_replacement_threshold, Number of times replacement page threshold was reached);
	FORMAT_EVENT(num_error_no_erased_pages, Number of times FF_ERROR_NO_ERASED_PAGES returned);
	FORMAT_EVENT(num_error_no_contexts_available, Number of times FF_ERROR_NO_CONTEXTS_AVAILABLE returned);

	FORMAT_EVENT(num_erase_errors[0], Number of times unit 0 encountered a page erase error.);
	FORMAT_EVENT(num_erase_errors[1], Number of times unit 1 encountered a page erase error.);
	FORMAT_EVENT(num_write_errors[0], Number of times unit 0 encountered a page write error.);
	FORMAT_EVENT(num_write_errors[1], Number of times unit 1 encountered a page write error.);
	FORMAT_STAT(num_block_erase_aborted, Number of times block erase was aborted due to error.);
	FORMAT_EVENT(num_page_write_callback_failed, Number of times write callback failed and retried.);
	FORMAT_EVENT(num_write_pagemap_failed, Number of times write of pagemap failed and retried);
	FORMAT_EVENT(num_write_toc_failed, Number of times write of Table of Contents page failed);
	FORMAT_EVENT(num_error_no_replacement_page, Number of times no replacement page was available when needed.);
	

	FORMAT_STAT(num_page_reads, Total number of page reads.);
	FORMAT_STAT(num_page_reads_cache_hit, Number of page reads that had a cache hit.);
	FORMAT_STAT(num_page_reads_cache_miss, Number of page reads that had a cache miss.);
	FORMAT_STAT(num_page_reads_partial, Number of partial page reads.);
	FORMAT_STAT(num_read_bytes_total, Total number of bytes transferred by read.);
	FORMAT_STAT(min_read_size, Minimum size of block read.);
	FORMAT_STAT(max_read_size, Maximum size of block read.);

	FORMAT_STAT(num_page_writes, Total number of page writes.);
	FORMAT_STAT(num_page_writes_cache_hit, Number of page writes that had a cache hit.);
	FORMAT_STAT(num_page_writes_cache_miss, Number of page writes that had a cache miss.);
	FORMAT_STAT(num_page_writes_partial, Number of partial page writes.);
	FORMAT_STAT(num_page_writes_partial_cache_miss, Number of partial page writes that had a cache miss.);
	FORMAT_STAT(num_write_bytes_total, Total number of bytes transferred by write.);
	FORMAT_STAT(min_write_size, Minimum size of block write.);
	FORMAT_STAT(max_write_size, Maximum size of block write.);
	
	return OK;

}  // Format_Stats_And_Events

	

