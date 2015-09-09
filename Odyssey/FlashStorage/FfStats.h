/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfStats.h
// 
// Description:
// This file defines the interfaces for the Flash File statistics.
// 
// Update Log 
// 
// 12/04/98 Jim Frandeen: Create file
/*************************************************************************/


#if !defined(FfStats_H)
#define FfStats_H

#include "FfCommon.h"
#include "FfPageMap.h"
#include <String.h>

class FF_Stats
{
public:
	// Reset event data
	void Reset_Event_Data();

	// Reset statistics
	void Reset_Statistics();

	void Inc_Num_Page_Reads() {
		m_statistics.num_page_reads++; }

	void Inc_Num_Page_Reads_Cache_Hit() {
		m_statistics.num_page_reads_cache_hit++; }

	void Inc_Num_Page_Reads_Cache_Miss() {
		m_statistics.num_page_reads_cache_miss++; }

	void Inc_Num_Page_Reads_Partial() {
		m_statistics.num_page_reads_partial++; }

	void Inc_Num_Read_Bytes_Total(U32 num_bytes) {
		if (num_bytes)
		{
			m_statistics.num_read_bytes_total += num_bytes;
			if ((m_statistics.min_read_size == 0) ||
				(num_bytes < m_statistics.min_read_size))
					m_statistics.min_read_size = num_bytes;
			if (num_bytes > m_statistics.max_read_size)
				m_statistics.max_read_size = num_bytes;
		}
	}

	void Inc_Num_Page_Writes() {
		m_statistics.num_page_writes++; }

	void Inc_Num_Page_Writes_Cache_Hit() {
		m_statistics.num_page_writes_cache_hit++; }

	void Inc_Num_Page_Writes_Cache_Miss() {
		m_statistics.num_page_writes_cache_miss++; }

	void Inc_Num_Page_Writes_Partial() {
		m_statistics.num_page_writes_partial++; }

	void Inc_Num_Page_Writes_Partial_Cache_Miss() {
		m_statistics.num_page_writes_partial_cache_miss++; }

	void Inc_Num_Write_Bytes_Total(U32 num_bytes) {
		if (num_bytes)
		{
			m_statistics.num_write_bytes_total += num_bytes;
			if ((m_statistics.min_write_size == 0) ||
				(num_bytes < m_statistics.min_write_size))
					m_statistics.min_write_size = num_bytes;
			if (num_bytes > m_statistics.max_write_size)
				m_statistics.max_write_size = num_bytes;
		}
	}

	void Inc_Num_Write_Callback_Failed() {
		m_event_data.num_page_write_callback_failed++; }

	void Inc_Num_Write_Toc_Failed() {
		m_event_data.num_write_toc_failed++; }

	void Inc_Num_Write_Pagemap_Failed() {
		m_event_data.num_write_pagemap_failed++; }

	void Inc_Num_Block_Erase_Aborted() {
		m_statistics.num_block_erase_aborted++; }

	void Inc_Num_Replacement_Threshold() {
		m_event_data.num_error_replacement_threshold++; }

	void Inc_Num_Error_No_Replacement_Page() {
		m_event_data.num_error_no_replacement_page++; }

	void Inc_Num_No_Erased_Pages() {
		m_event_data.num_error_no_erased_pages++; }

	void Inc_Num_Erased_Page_Threshold() {
		m_statistics.num_erased_page_threshold++; }
	
	void Inc_Num_No_Contexts() {
		m_event_data.num_error_no_contexts_available++; }

	void Inc_Num_Waits_Erased_Page() {
		m_statistics.num_waits_erased_page++;
		m_statistics.num_waiting_erased_page++; }

	void Dec_Num_Waits_Erased_Page() {
		CT_ASSERT((m_statistics.num_waiting_erased_page != 0), Dec_Num_Waits_Ersased_Page);
		m_statistics.num_waiting_erased_page--; }

	void Inc_Num_Waits_Buss() {
		m_statistics.num_waits_buss++;
		m_statistics.num_waiting_buss++; }

	void Dec_Num_Waits_Buss() {
		CT_ASSERT((m_statistics.num_waiting_buss != 0), Dec_Num_Waits_Buss);
		m_statistics.num_waiting_buss--; }

	void Inc_Num_Waits_Unit(U32 unit) {
		m_statistics.num_waits_unit[unit]++;
		m_statistics.num_waiting_unit[unit]++; }

	void Dec_Num_Waits_Unit(U32 unit) {
		CT_ASSERT((m_statistics.num_waiting_unit[unit] != 0), Dec_Num_Waits_Unit);
		m_statistics.num_waiting_unit[unit]--; }

	void Inc_Num_Erase_Errors(U32 unit) {
		m_event_data.num_erase_errors[unit]++; }

	void Inc_Num_Write_Errors(U32 unit) {
		m_event_data.num_write_errors[unit]++; }

	void Inc_Num_Verify_Errors(U32 unit) {
		m_event_data.num_verify_errors[unit]++; }

	void Get_Event_Data(FF_EVENT_DATA *p_event_data_buffer, 
		U32 size_buffer);

	void Get_Statistics(FF_STATISTICS *p_statistics_buffer, 
		U32 size_buffer, FF_TOC *p_toc);

private:
	FF_EVENT_DATA 		m_event_data;
	FF_STATISTICS 		m_statistics;
	
};

#endif // FfStats_H

