/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmStats.h
// 
// Description:
// This file defines the interfaces for the Cache statistics.
// 
// Update Log 
// 
// 9/23/98 Jim Frandeen: Create file
/*************************************************************************/


#if !defined(CmStats_H)
#define CmStats_H

#include "Align.h"
#include "Simple.h"
#include "ErrorLog.h"
#include "CmMem.h"
#include "Cache.h"
#include <String.h>

class CM_Stats
{
public:
	// Allocate and initialize object
	static CM_Stats * Allocate(CM_Mem *p_mem);

	// Reset event data
	void Reset_Event_Data();

	// Reset Statistics
	void Reset_Statistics();

	void Inc_Error_Max_Dirty_Pages() {
		m_p_event_data->num_error_max_dirty_pages++; }
	
	void Inc_Error_No_Page_Frame() {
		m_p_event_data->num_error_no_page_frame++; }
	
	void Inc_Num_Pages_Open() {
		if (++m_p_statistics->num_pages_open > m_p_statistics->num_pages_open_max)
			m_p_statistics->num_pages_open_max = m_p_statistics->num_pages_open; }
	
	void Dec_Num_Pages_Open() {
		CT_ASSERT((m_p_statistics->num_pages_open != 0), Dec_Num_Pages_Open);
		m_p_statistics->num_pages_open--; }
	
	void Inc_Num_Pages_Locked_Write() {
		m_p_statistics->num_pages_locked_write++;}

	void Dec_Num_Pages_Locked_Write() {
		CT_ASSERT((m_p_statistics->num_pages_locked_write != 0), Dec_Num_Pages_Locked_Write);
		m_p_statistics->num_pages_locked_write--; }

	void Inc_Num_Pages_Locked_Writeback() {
		m_p_statistics->num_pages_locked_writeback++;}

	void Dec_Num_Pages_Locked_Writeback() {
		CT_ASSERT((m_p_statistics->num_pages_locked_writeback != 0), Dec_Num_Pages_Locked_Writeback);
		m_p_statistics->num_pages_locked_writeback--; }

	U32 Get_Num_Pages_Locked_Writeback() {
		return m_p_statistics->num_pages_locked_writeback;}

	void Inc_Num_Pages_Locked_Read() {
		m_p_statistics->num_pages_locked_read++;}

	void Dec_Num_Pages_Locked_Read() {
		CT_ASSERT((m_p_statistics->num_pages_locked_read != 0), Dec_Num_Pages_Locked_Read);
		m_p_statistics->num_pages_locked_read--; }

	void Inc_Num_Pages_Locked_Remap() {
		m_p_statistics->num_pages_locked_remap++;}

	void Dec_Num_Pages_Locked_Remap() {
		CT_ASSERT((m_p_statistics->num_pages_locked_remap != 0), Dec_Num_Pages_Locked_Remap);
		m_p_statistics->num_pages_locked_remap--; }

	void Inc_Num_Pages_Locked_Read_Not_Present() {
		m_p_statistics->num_pages_locked_read_not_present++;}

	void Dec_Num_Pages_Locked_Read_Not_Present() {
		CT_ASSERT((m_p_statistics->num_pages_locked_read_not_present != 0), Dec_Num_Pages_Locked_Read_Not_Present);
		m_p_statistics->num_pages_locked_read_not_present--; }

	void Inc_Num_Open_Read() {
		m_p_statistics->num_open_read++; }
	
	void Inc_Num_Open_Remap() {
		m_p_statistics->num_open_remap++; }
	
	void Inc_Num_Open_Write() {
		m_p_statistics->num_open_write++; }
	
	void Inc_Num_Abort_Page() {
		m_p_statistics->num_abort_page++; }
	
	void Inc_Num_Close_Page() {
		m_p_statistics->num_close_page++; }
	
	void Inc_Num_Cache_Hits() {
		m_p_statistics->num_cache_hits++; }
	
	void Inc_Num_Cache_Misses() {
		m_p_statistics->num_cache_misses++; }

	void Inc_Dirty_Page_Threshold() {
		m_p_statistics->num_dirty_page_threshold++; }
	
	void Inc_Num_Error_Page_Locked() {
		m_p_statistics->num_error_page_locked++; }
	
	void Inc_Num_Write_Callback() {
		m_p_statistics->num_write_callback++; }
	
	void Inc_Num_Write_Callback_Failed() {
		m_p_event_data->num_write_callback_failed++; }

	void Inc_Num_Dirty_Again() {
		m_p_statistics->num_dirty_again++; }

	void Inc_Num_Waits_For_Page_Frame() {
		m_p_statistics->num_waits_for_page_frame++;
		m_p_statistics->num_waiting_for_page_frame++; }

	void Dec_Num_Waits_For_Page_Frame() {
		CT_ASSERT((m_p_statistics->num_waiting_for_page_frame != 0), Dec_Num_Waits_For_Page_Frame);
		m_p_statistics->num_waiting_for_page_frame--; }

	void Inc_Num_Waits_For_Read_Lock() {
		m_p_statistics->num_waits_for_read_lock++;
		m_p_statistics->num_waiting_for_lock++; }

	void Inc_Num_Waits_For_Write_Lock() {
		m_p_statistics->num_waits_for_write_lock++;
		m_p_statistics->num_waiting_for_lock++; }

	void Inc_Num_Waits_For_Writeback_Lock() {
		m_p_statistics->num_waits_for_writeback_lock++;
		m_p_statistics->num_waiting_for_lock++; }

	void Inc_Num_Waits_For_Remap_Lock() {
		m_p_statistics->num_waits_for_remap_lock++;
		m_p_statistics->num_waiting_for_lock++; }

	void Dec_Num_Waits_For_Lock() {
		CT_ASSERT((m_p_statistics->num_waiting_for_lock != 0), Dec_Num_Waits_For_Lock);
		m_p_statistics->num_waiting_for_lock--; }

	void Inc_Num_Flush_Waits_For_Lock() {
		m_p_statistics->num_flush_waits_for_lock++;
		m_p_statistics->num_flush_waiting_for_lock++; }

	void Dec_Num_Flush_Waits_For_Lock() {
		CT_ASSERT((m_p_statistics->num_flush_waiting_for_lock != 0), Dec_Num_Flush_Waits_For_Writeback_Lock);
		m_p_statistics->num_flush_waiting_for_lock--; }

	void Set_Num_Pages_Replaceable(U32 num_pages) {
		m_p_statistics->num_pages_replaceable = num_pages;
		if (num_pages < m_p_statistics->num_pages_replaceable_min)
			m_p_statistics->num_pages_replaceable_min = num_pages;}
	
	void Set_Num_Pages_Replaceable_Min(U32 num_pages) {
		m_p_statistics->num_pages_replaceable_min = num_pages;}
	
	void Set_Num_Reserve_Pages(U32 num_pages) {
		m_p_statistics->num_reserve_pages = num_pages;}
	
	void Set_Dirty_Page_Writeback_Threshold(U32 num_pages) {
		m_p_statistics->dirty_page_writeback_threshold = num_pages;}
	
	void Set_Dirty_Page_Error_Threshold(U32 num_pages) {
		m_p_statistics->dirty_page_error_threshold = num_pages;}
	
	void Set_Num_Page_Frames(U32 num_page_frames) {
		m_p_statistics->num_page_frames = num_page_frames; }

	void Set_Memory_Used(U32 memory_used) {
		m_p_statistics->memory_used = memory_used; }

	void Set_Num_Pages_Working_Set(U32 num_pages) {
		m_p_statistics->num_pages_working_set = num_pages; }

	void Set_Num_Pages_Dirty(U32 num_pages) {
		m_p_statistics->num_pages_dirty = num_pages; 
		if (m_p_statistics->num_pages_dirty > m_p_statistics->num_pages_dirty_max)
			m_p_statistics->num_pages_dirty_max = m_p_statistics->num_pages_dirty; }

	void Get_Event_Data(CM_EVENT_DATA *p_event_data_buffer, 
		U32 size_buffer) {
#ifndef _WINDOWS
		CT_ASSERT(IS_ALIGNED_8(p_event_data_buffer), Get_Event_Data);
#endif
		memcpy(p_event_data_buffer, m_p_event_data, size_buffer); }

	void Get_Statistics(CM_STATISTICS *p_statistics_buffer, 
		U32 size_buffer) {
#ifndef _WINDOWS
		CT_ASSERT(IS_ALIGNED_8(p_statistics_buffer), Get_Statistics);
#endif
		memcpy(p_statistics_buffer, m_p_statistics, size_buffer); }

	// Return memory size required by stats data.
	static U32 Memory_Size(CM_CONFIG *p_config) 
	{
		return sizeof(CM_Stats) + 8 + sizeof(CM_EVENT_DATA) + 8
			+ sizeof(CM_STATISTICS) + 8;
	}

private:
	CM_EVENT_DATA 		*m_p_event_data;
	CM_STATISTICS 		*m_p_statistics;
	
};

#endif // CmStats_H
