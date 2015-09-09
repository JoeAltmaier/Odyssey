/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FlashStorage.h
// 
// Description:
// This file defines the external interfaces to the Flash Storage. 
// Flash Storage allows flash to emulate a block unit.  To the user, the
// flash appears as a contiguous array of storage blocks numbered from
// zero to one less than the total number of blocks.
// Flash Storage is a translation layer between the native file system and 
// flash storage.
//
// The Flash Storage interface is used by the SSD driver.
// It can also be used by other drivers that use flash storage.
// 
// 7/20/98 Jim Frandeen: Create file FlashFile.h
// 5/24/99 Jim Frandeen: Create file from FlashFile.h to implement
//				interleaving
/*************************************************************************/

#if !defined(FlashStorage_H)
#define FlashStorage_H

#include <stdlib.h>
#include "Cache.h"
#include "Simple.h"
#include "Message.h"

class Flash_Device;

/*************************************************************************/
// FF_CONFIG
// defines configuration structure passed to FF_Initialize.
// This record can be retrieved by the FF_Get_Config method.
// Version 3 add write_verify_level and erase_verify_level.
// Version 4 add callback_memory_size.
// Version 5 add wear_level_threshold and verify_page_erased_before_write.
/*************************************************************************/
#define FF_CONFIG_VERSION 5

typedef struct 
{

	// version of FF_CONFIG record
	U32 	version;

	// Size of FF_CONFIG record in bytes.
	// This need not be set by FF_Initialize, but it is returned 
	// by FF_Get_Config.
	U32		size;

	// Pointer to flash device.
	Flash_Device *p_device;

	// Pointer to memory to be used by the flash manager.
	void	*p_memory;

	// Amount of memory to be allocated for the flash storage system.
	// The user may call FF_Get_Memory_Size_Required
	// to allocate the proper amount of memory for the flash storage system.
	// If the user does not fill in the memory parameters for the 
	// cache config, the left over memory will be used for the cache manager.
	U32		memory_size;

	// Verify write 1 means verify every write.
	U32		verify_write;

	// Verify erase level 1 means verify block sectors for all ones
	// after erase.
	// Verify erase level level 2 means write and verify block sectors 
	// for checkerboard after level 1 check.
	// Verify erase level level 3 means write and verify block sectors 
	// for inverted checkerboard after level 2 check.
	U32		verify_erase;

	// Verify erased before write is normally used only for testing
	// to assure that the erase operation is working properly.
	U32		verify_page_erased_before_write;

	// When wear_level_threshold pages have been erased, the wear level
	// algorithm is started.  The wear level address is advanced by one
	// block, and the block is erased.
	U32		wear_level_threshold;

	// Format configuration parameters begin here.

	U32		percentage_erased_pages;
	U32		percentage_replacement_pages;

	// When the number of replacement pages falls below a percentage
	// of the total number allocated, no more Write commands 
	// are permitted.
	U32		replacement_page_threshold;

	// If erase_all_pages is true, all pages will be erased when the
	// unit is formatted (a time-consuming operation)
	// otherwise, only erase as many pages
	// as needed when the unit is formatted; 
	U32		erase_all_pages;
	
	// For debugging, set verify_structures.
	// This can cause code to run slow!
	U32		verify_structures;

	// Set all error tests to go off once every test_all_random
	// at random.
	U32		test_all_random;
	
	// Set all error tests to go off once every test_all_static
	// times the test is executed.
	U32		test_all_static;

	// Simulate a write error every write_error_frequency_value.
	U32		write_error_frequency_value;
	
	// Simulate an erase error every erase_error_frequency_value.
	U32		erase_error_frequency_value;
	
	// Simulate a read ECC every erase_error_frequency_value.
	U32		read_error_frequency_value;
	
} FF_CONFIG;

/*************************************************************************/
// FF_EVENT_DATA
// defines event record returned by FF_Get_Event_Data.
// This describes information indicating errors, events, 
// and possibly health conditions. 
/*************************************************************************/
#define FF_EVENT_DATA_VERSION 2

#define FF_NUM_CELL_PARTS 64

typedef struct {

	// Version of FF_EVENT_DATA record
	U32 	version;
	
	// Size of FF_EVENT_DATA record in bytes.
	U32		size;

	// Number of times FF_ERROR_REPLACEMENT_PAGE_THRESHOLD was returned to the user.
	// This happens if a write request is received, and the number of replacement
	// pages available is less than the replacement page threshold.
	I64		num_error_replacement_threshold;
	
	// Number of times FF_ERROR_NO_ERASED_PAGES was returned to the user.
	// This happens if a write request is received, and there are no
	// erased pages currently available.
	I64		num_error_no_erased_pages;
	
	// Number of times FF_ERROR_NO_CONTEXTS_AVAILABLE was returned.
	// This happens if a request is received, but there are no 
	// context records available.  This could happen if not enough context
	// records were initially allocated, or if an internal error such as
	// a leak occurs.
	I64		num_error_no_contexts_available;
   
	// Number of page read  errors detected for each unit part.
	I64		num_read_errors[FF_NUM_CELL_PARTS];
     
	// Number of page write errors detected for each unit part.
	I64		num_write_errors[FF_NUM_CELL_PARTS];
 
	// Number of page verify errors detected for each unit part.
	I64		num_verify_errors[FF_NUM_CELL_PARTS];
   
	// Number of page erase errors detected for each unit part.
	I64		num_erase_errors[FF_NUM_CELL_PARTS];
   
	// Number of times write callback failed to write a page and
	// retried with a replacement page.
	I64		num_page_write_callback_failed;

	// Number of times write of pagemap failed and retried
	// with a replacement page.
	I64		num_write_pagemap_failed;

	// Number of times write of the Table of Contents failed.
	I64		num_write_toc_failed;

	// Number of times no replacement page was available when needed.
	I64		num_error_no_replacement_page;
   
}  FF_EVENT_DATA;


/*************************************************************************/
//    FF_STATISTICS
//    defines statistics record returned by FF_Get_Statistics.
/*************************************************************************/
#define FF_STATISTICS_VERSION 2

#define NUM_CELLS 2

typedef struct {
	U32 	version;
	
	// Size of FF_STATISTICS record in bytes.
	U32		size;

	// N.B. All I64s are together for alignment.

	// Number of times the erased page threshold was reached, and a block
	// erase was started.
	I64		num_erased_page_threshold;

	// Number of times the wear level threshold was reached, and a block
	// erase was started.
	I64		num_wear_level_threshold;

	// Number of times a write had to wait for an erased page.
	I64		num_waits_erased_page;

	// Number of times a command had to wait because the
	// buss was busy with another command.
	I64		num_waits_buss;

	// Number of times a command had to wait because a
	// unit was busy with another command.
	I64		num_waits_unit[NUM_CELLS];

	// Total number of page reads.
	I64		num_page_reads;
   
	// Number of page reads that had a cache hit.
	I64		num_page_reads_cache_hit;
   
	// Number of page reads that had a cache miss.
	I64		num_page_reads_cache_miss;
   
	// Number of page reads for a partial page.
	I64		num_page_reads_partial;
   
	// Timer ticks for page reads when the page was in the cache.
	// Each time a read page command is received, the value of the
	// real time clock is noted.  When the read has completed, and the data
	// has been transferred to the user's buffer on another IOP, the value of
	// the real time clock is again noted, and the number of timer ticks is 
	// added to the total. This can be used to calculate the average
	// page read time when the page was in the cache.
	// TODO
	I64		num_page_read_timer_ticks_cache_hit;
   
	// Timer ticks for page reads when the page was not in the cache.
	// This can be used to calculate the average
	// page read time when the page was not in the cache.
	// TODO
	I64		num_page_read_timer_ticks_cache_miss;
   
	// Total number of bytes transferred for reads.
	// This can be used to calculate the average number of bytes per transfer.
	I64		num_read_bytes_total;
   
	// Timer ticks for unit reads.
	// Each time a read command is sent to the unit, the value of the
	// real time clock is noted.  When the read is completed, the value of
	// the real time clock is again noted, and the number of timer ticks is 
	// added to the total. This can be used to calculate the average
	// unit read time.
	// TODO
	I64		num_unit_read_timer_ticks;
   
	// Total timer ticks for unit reads, including wait time.
	// Similar to num_unit_read_timer_ticks, but this includes any time
	// spent waiting for a previous controller command to complete.
	// TODO
	I64		num_unit_read_timer_ticks_total;
   
	// Total number of page writes.
	I64		num_page_writes;
   
	// Number of page writes that had a cache hit.
	I64		num_page_writes_cache_hit;
   
	// Number of page writes that had a cache miss.
	I64		num_page_writes_cache_miss;
   
	// Number of page writes for a partial page.  
	I64		num_page_writes_partial;
   
	// Number of page writes for a partial page that had a cache miss.  
	// When the page is not in the cache it must be read before the
	// data can be moved into part of a page in the cache.
	I64		num_page_writes_partial_cache_miss;

	// Timer ticks for page writes
	// Each time a write page command is received, the value of the
	// real time clock is noted.  When the write has completed, and the data
	// has been transferred to the cache,
	// the real time clock is again noted, and the number of timer ticks is 
	// added to the total. This can be used to calculate the average
	// page write time.  Note that this does not include timer to actually
	// write the page to the unit.
	// TODO
	I64		num_page_write_timer_ticks;
   
	// Total number of bytes transferred for writes.
	// This can be used to calculate the average number of bytes per transfer.
	I64		num_write_bytes_total;
   
	// Timer ticks for unit page writes.
	// TODO
	I64		num_unit_write_timer_ticks;
   
	// Total timer ticks for unit page writes, including wait time.
	// TODO
	I64		num_unit_write_timer_ticks_total;
   
	// Total number of page block erases.
	I64		num_page_blocks_erased;

	// Number of times page block erase was aborted due to an error.
	I64		num_block_erase_aborted;
   
	// Timer ticks for page erases.
	// TODO
	I64		num_page_erase_timer_ticks;
   
	// Total timer ticks for page erases, including wait time.
	// TODO
	I64		num_page_erase_timer_ticks_total;
	
	// Number of nanoseconds per timer tick
	U32		nanoseconds_per_timer_tick;
   
	// Page size in bytes
	U32		page_size;
	
	// Total capacity of flash memory in pages
	U32		num_total_pages;
	
	// User capacity of flash memory in pages
	U32		num_user_pages;
	
	// Number of bad blocks.
	U32		num_bad_blocks;

	// Number of initial bad pages encountered during format.
	U32		num_initial_bad_pages;

	// Number of pages currently available in erased page pool.
	U32		num_erased_pages_available;
   
	// Number of pages currently allocated from the erased page pool.
	U32		num_erased_pages_allocated;
   
	// Number of pages currently available in the replacement page pool.
	U32		num_replacement_pages_available;
   
	// Number of pages currently allocated from the replacement page pool.
	U32		num_replacement_pages_allocated;
   
	// Erased page threshold.  When num_erased_pages_available falls below
	// this number, a background process is started to erase some pages.
	U32		erased_page_threshold;
	
	// Replacement page threshold.  When num_replacement_pages_available falls 
	// below this number, FF_ERROR_REPLACEMENT_PAGE_THRESHOLD is returned 
	// for write requests.
	U32		replacement_page_threshold;
	
	// Wear level threshold.  When erased page counter exceeds
	// this number, a background process is started to increment the
	// wear level address and erase the next page block.
	U32		wear_level_threshold;
	
	// Maximum ready list size.  When requests are received, they
	// are placed in a ready list.
	// TODO
	U32		max_ready_list_size; 
   
	// Minimum ready list available list size.  When requests are received, they
	// are allocated from an available list. 
	// TODO
	U32		min_available_list_size; 
   
	// Number of contexts currently waiting for an erased page.
	U32		num_waiting_erased_page;
   
	// Number of contexts currently waiting for the buss.
	U32		num_waiting_buss;
   
	// Number of contexts currently waiting for a unit.
	U32		num_waiting_unit[NUM_CELLS];
   
	// The mininum number of timer ticks for a page read 
	// when the page was in the cache.
	// TODO
	U32		min_page_read_timer_ticks_cache_hit;
   
	// The maximum number of timer ticks for a page read 
	// when the page was in the cache.
	// TODO
	U32		max_page_read_timer_ticks_cache_hit;
   
	// The mininum number of timer ticks for a page read 
	// when the page was not in the cache.
	// TODO
	U32		min_page_read_timer_ticks_cache_miss;
   
	// The maximum number of timer ticks for a page read 
	// when the page was not in the cache.
	// TODO
	U32		max_page_read_timer_ticks_cache_miss;
   
	// Minimum read transfer size. 
	U32		min_read_size; 
   
	// Maximum read transfer size. 
	U32		max_read_size; 
   
	// The mininum number of timer ticks for a page write. 
	// TODO
	U32		min_page_write_timer_ticks;
   
	// The maximum number of timer ticks for a page write. 
	// TODO
	U32		max_page_write_timer_ticks;
   
	// Minimum write transfer size. 
	U32		min_write_size; 
   
	// Maximum write transfer size. 
	U32		max_write_size; 
   
}  FF_STATISTICS;


// Flash Handle is really a pointer to a flash context
typedef void *FF_HANDLE;

// The callback is called when the Flash File operation has completed.
typedef void FF_CALLBACK(

	// number of bytes successfully transferred
	U32 transfer_byte_count,
	
	// If operation did not succeed, logical byte address of failure.
	I64 logical_byte_address,

	// result of operation
	Status status,

	// pointer passed in to Flash File method
	void *p_context);

// Open Flash File object and return a handle.
// Object will not be opened unless a bad block table has been created.
Status FF_Open(
	FF_CONFIG *p_flash_config,
	CM_CONFIG *p_cache_config,
	Callback_Context *p_callback_context,
	FF_HANDLE *p_flash_handle);

// Check for bad blocks and create bad block table
Status FF_Create(
	FF_CONFIG *p_flash_config,
	CM_CONFIG *p_cache_config,
	Callback_Context *p_callback_context);

// Read num_bytes into specified buffer.
// Call callback routine with pointer to context when read has completed.
Status FF_Read(
	FF_HANDLE flash_handle,
	void *p_buffer, 
	U32 transfer_byte_count, 
	I64 logical_byte_address,
	void *p_context,
	FF_CALLBACK *p_completion_callback);

// Read num_bytes into SGL.
// Call callback routine with pointer to context when read has completed.
Status FF_Read(
	FF_HANDLE flash_handle,
	SGE_SIMPLE_ELEMENT	*p_element, 
	U32 transfer_byte_count, 
	I64 logical_byte_address,
	void *p_context,
	FF_CALLBACK *p_completion_callback);

// Write num_bytes from specified buffer.
// Call callback routine with pointer to context when write has completed.
Status FF_Write(
	FF_HANDLE flash_handle,
	void *p_buffer, 
	U32 transfer_byte_count, 
	I64 logical_byte_address,
	void *p_context,
	FF_CALLBACK *p_callback);

// Write num_bytes from specified SGL.
// Call callback routine with pointer to context when write has completed.
Status FF_Write(
	FF_HANDLE flash_handle,
	SGE_SIMPLE_ELEMENT	*p_element, 
	U32 transfer_byte_count, 
	I64 logical_byte_address,
	void *p_context,
	FF_CALLBACK *p_callback);

// Format flash file system.
// Call callback routine with pointer to context when format has completed.
Status FF_Format(
	FF_HANDLE flash_handle,
	FF_CONFIG *p_config,
	void *p_context,
	FF_CALLBACK *p_callback);


// Close everything.
Status FF_Close(	
	FF_HANDLE flash_handle,
	Callback_Context *p_callback_context);

// Get FF_STATISTICS record.
Status FF_Get_Statistics(
	FF_HANDLE flash_handle,
	FF_STATISTICS *p_statistics_buffer,
	U32 size_buffer);

// Get FF_CONFIG data.
Status FF_Get_Config(
	FF_HANDLE flash_handle,
	FF_CONFIG *p_config_buffer,
	U32 size_buffer);

// Get FF_EVENT data.
Status FF_Get_Event_Data(
	FF_HANDLE flash_handle,
	FF_EVENT_DATA *p_event_data_buffer,
	U32 size_buffer);

// Get CM_STATISTICS record
Status FF_Get_Cache_Statistics(
	FF_HANDLE flash_handle,
	CM_STATISTICS *p_statistics_buffer,
	U32 size_buffer);

// Get CM_EVENT_DATA record
Status FF_Get_Cache_Event_Data(
	FF_HANDLE flash_handle,
	CM_EVENT_DATA *p_event_buffer,
	U32 size_buffer);

// Get CM_Config record
Status FF_Get_Cache_Config(
	FF_HANDLE flash_handle,
	CM_CONFIG *p_config_buffer,
	U32 size_buffer);

// For debugging, validate internal structures.
Status FF_Validate(FF_HANDLE flash_handle);

// Return the number of bytes of memory required.
Status FF_Get_Memory_Size_Required(FF_CONFIG *p_config, U32 *p_memory_size);

// Display flash statistics and events.
Status FF_Display_Stats_And_Events(FF_HANDLE flash_handle);

// Format flash statistics and events into string for display.
Status FF_Format_Stats_And_Events(FF_HANDLE flash_handle, char *p_string);

// Display cache statistics and events.
Status FF_Display_Cache_Stats_And_Events(FF_HANDLE flash_handle);

// Format cache statistics and events into string for display.
Status FF_Format_Cache_Stats_And_Events(FF_HANDLE flash_handle, char *p_string);

/*************************************************************************/
// FF_Error_Test_Name
// Names of error tests, used for error injection.
/*************************************************************************/
typedef enum
{
	READ_ECC_ERROR,
	WRITE_ERROR,
	ERASE_ERROR,
	FF_Error_Test_Last
} FF_Error_Test_Name;

/*************************************************************************/
// FF_Error_Injection
/*************************************************************************/
class FF_Error_Injection
{
public: // methods

	// constructor
	FF_Error_Injection();

	// Allocate memory for static object.
	Status Allocate();

	// Free memory for static object.
	Status Free();

	int Test_Error(int condition, FF_Error_Test_Name test_name);

	// Set all test counters for random.
	void Set_Random(unsigned frequency_value);

	// Set specific test for random.
	void Set_Random(unsigned frequency_value, FF_Error_Test_Name test_name);

	// Set all test counters for static.
	void Set_Static(unsigned frequency_value);

	// Set specific test for static.
	void Set_Static(unsigned frequency_counter, FF_Error_Test_Name test_name);

	// Set up to test acccording to parameters specified in the
	// configuration file.
	Status Set_Config_Options(FF_CONFIG *p_config);

	unsigned Get_Execution_Count(FF_Error_Test_Name test_name);
	unsigned Get_Error_Count(FF_Error_Test_Name test_name);
	unsigned Get_Frequency_Value(FF_Error_Test_Name test_name);
	int Is_Random(FF_Error_Test_Name test_name);

private: // helper methods

	static unsigned Random(void);
	static unsigned Random_Max (unsigned max_value);


private: // member data

	typedef struct _Test_Entry
	{
		unsigned execution_counter;
		unsigned error_injection_counter;
		unsigned frequency_value;
		unsigned frequency_counter;
	} Test_Entry;

	static Test_Entry *m_p_test_entry;
	static unsigned m_next;
	static unsigned m_seed;

}; // FF_Error_Injection


/*************************************************************************/
// FF_ERROR_TEST macro
// If debugging, call FF_Error_Injection; otherwise simply execute the test.
/*************************************************************************/
#ifdef ERROR_INJECTION
extern FF_Error_Injection flash_error_test;
#define IF_FF_ERROR(condition, test_name) \
	if (flash_error_test.Test_Error(condition, test_name))	
#else
#define IF_FF_ERROR(condition, test_name) \
	if (condition)
#endif

/*************************************************************************/
// FF_INJECT_STATUS macro
// If debugging, set status to an error code.  
// This is used in conjunction with FF_ERROR_TEST when a status was
// actually OK, we simulated an error, and now we need to set the status
// to something other than OK in order to complete the test.
/*************************************************************************/
#ifdef ERROR_INJECTION
#define FF_INJECT_STATUS(status, test_status) \
	if (status == OK) \
		status = test_status	
#else
#define FF_INJECT_STATUS(status, test_status) 
	
#endif

/*************************************************************************/
// FF_Error_Injection constructor.
/*************************************************************************/
inline FF_Error_Injection::FF_Error_Injection()
{
	// Seed the random number.
	// TODO -- set seed to timer or ??
	m_seed = 1103515245;
	m_next = 2;

	m_p_test_entry = 0;
}

/*************************************************************************/
// Set_Random sets all entries to return true at random times
// between 0 and frequency_value.
// Random entries do not have a frequency_counter.
/*************************************************************************/
inline void FF_Error_Injection::Set_Random(unsigned frequency_value) 
{
	for (unsigned index = 0; index < FF_Error_Test_Last; index++)
	{
		(m_p_test_entry + index)->frequency_value = frequency_value;
		(m_p_test_entry + index)->frequency_counter = 0;
	}
}

/*************************************************************************/
// Set_Random sets a specific test to return true at random times
// between 0 and frequency_value.
/*************************************************************************/
inline void FF_Error_Injection::Set_Random(unsigned frequency_value, FF_Error_Test_Name test_name) 
{
	(m_p_test_entry + test_name)->frequency_value = frequency_value;
	(m_p_test_entry + test_name)->frequency_counter = 0;
}

/*************************************************************************/
// Set_Static sets all entries to return true every frequency_counter
// time the test is executed.
/*************************************************************************/
inline void FF_Error_Injection::Set_Static(unsigned frequency_value) 
{
	for (unsigned index = 0; index < FF_Error_Test_Last; index++)
	{
		(m_p_test_entry + index)->frequency_counter = frequency_value;
		(m_p_test_entry + index)->frequency_value = frequency_value;
	}
}

/*************************************************************************/
// Set_Static sets a specific test to return true every frequency_counter
// time the test is executed.
/*************************************************************************/
inline void FF_Error_Injection::Set_Static(unsigned frequency_value, FF_Error_Test_Name test_name) 
{
	(m_p_test_entry + test_name)->frequency_counter = frequency_value;
	(m_p_test_entry + test_name)->frequency_value = frequency_value;
}

/*************************************************************************/
// Get_Execution_Count returns the execution count.
/*************************************************************************/
inline unsigned FF_Error_Injection::Get_Execution_Count(FF_Error_Test_Name test_name) 
{
	return (m_p_test_entry + test_name)->execution_counter;
}

/*************************************************************************/
// Get_Error_Count returns the error injection count.
/*************************************************************************/
inline unsigned FF_Error_Injection::Get_Error_Count(FF_Error_Test_Name test_name) 
{
	return (m_p_test_entry + test_name)->error_injection_counter;
}

/*************************************************************************/
// Is_Random returns true if this entry is a random counter.
/*************************************************************************/
inline int FF_Error_Injection::Is_Random(FF_Error_Test_Name test_name) 
{
	if ((m_p_test_entry + test_name)->frequency_counter)
	
		// Random entries do not have a frequency counter.
		return 0;
	return 1;
}

/*************************************************************************/
// Get_Frequency_Value returns the frequency value.
/*************************************************************************/
inline unsigned FF_Error_Injection::Get_Frequency_Value(FF_Error_Test_Name test_name) 
{
	return (m_p_test_entry + test_name)->frequency_value;
}

/*************************************************************************/
// Random_Max returns a random number between 0 and max_value inclusive.
/*************************************************************************/
inline unsigned FF_Error_Injection::Random_Max (unsigned max_value) 
{
   return(((2 * Random() * max_value + RAND_MAX) /
      RAND_MAX - 1) / 2);
}

/*************************************************************************/
// Random returns a pseudorandom number.
/*************************************************************************/
inline unsigned FF_Error_Injection::Random(void)
{
	m_next = m_next * 1103515245 + 12345;
	return((m_next >> 16) & 0x7FFF);
}

#endif /* FlashStorage_H  */
