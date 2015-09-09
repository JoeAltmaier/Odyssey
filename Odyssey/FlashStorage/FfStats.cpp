/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfStats.cpp
// 
// Description:
// This file implements the FF_Stats class methods of the 
// Flash File System 
// 
// 12/4/98 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "Align.h"
#include "FfInterface.h"
#include "FfStats.h"
#include "TimerTicks.h"

/*************************************************************************/
// FF_Stats::Get_Event_Data
/*************************************************************************/
void FF_Stats::Get_Event_Data(FF_EVENT_DATA *p_event_data_buffer, 
	U32 size_buffer) 
{
#ifndef _WINDOWS
	CT_ASSERT(IS_ALIGNED_8(p_event_data_buffer), Get_Event_Data);
#endif

	memcpy(p_event_data_buffer, &m_event_data, size_buffer); 
}

/*************************************************************************/
// FF_Stats::Get_Statistics
/*************************************************************************/
void FF_Stats::Get_Statistics(FF_STATISTICS *p_statistics_buffer, 
	U32 size_buffer, FF_TOC *p_toc) 
{
#ifndef _WINDOWS
#ifndef GREEN_HILLS
	CT_ASSERT(IS_ALIGNED_8(p_statistics_buffer), Get_Statistics);
#endif
#endif

	m_statistics.nanoseconds_per_timer_tick = NS_PER_TIMER_TICK;
	m_statistics.num_user_pages = p_toc->vp_last_user_page + 1;
	m_statistics.num_initial_bad_pages = p_toc->vp_last_bad_page 
		- p_toc->vp_first_bad_page + 1;
	m_statistics.num_bad_blocks = (p_toc->vp_last_bad_block
		- p_toc->vp_first_bad_block + 1) / Flash_Address::Sectors_Per_Block();
	m_statistics.num_erased_pages_available = p_toc->num_erased_pages_available;
	m_statistics.num_erased_pages_allocated = p_toc->num_erased_pages_allocated;
	m_statistics.num_replacement_pages_available = p_toc->num_replacement_pages_available;
	m_statistics.num_replacement_pages_allocated = p_toc->num_replacement_pages_allocated;
	m_statistics.erased_page_threshold = p_toc->erased_page_threshold;
	m_statistics.replacement_page_threshold = p_toc->replacement_page_threshold;
	m_statistics.wear_level_threshold = p_toc->wear_level_threshold;
	m_statistics.num_wear_level_threshold = p_toc->num_wear_level_threshold;
	m_statistics.max_ready_list_size = 0; // TODO
	m_statistics.min_available_list_size = 0; // TODO

	m_statistics.num_page_blocks_erased= p_toc->num_page_blocks_erased;

	// Copy statistics record into user's buffer.
	memcpy(p_statistics_buffer, &m_statistics, size_buffer); 
}

/*************************************************************************/
// FF_Stats::Reset_Event_Data
/*************************************************************************/
void FF_Stats::Reset_Event_Data() 
{
	// Initialize event data
#ifndef _WINDOWS
#ifndef GREEN_HILLS
	CT_ASSERT(IS_ALIGNED_8(&m_event_data), Reset_Event_Data);
#endif
#endif

	ZERO(&m_event_data, sizeof(FF_EVENT_DATA));
	m_event_data.version = FF_EVENT_DATA_VERSION;
	m_event_data.size = sizeof(FF_EVENT_DATA);

} //FF_Stats::Initialize

/*************************************************************************/
// FF_Stats::Reset_Statistics
/*************************************************************************/
void FF_Stats::Reset_Statistics() 
{
	// Initialize statistics data
#ifndef _WINDOWS
#ifndef GREEN_HILLS
	CT_ASSERT(IS_ALIGNED_8(&m_statistics), Reset_Statistics);
#endif
#endif


	ZERO(&m_statistics, sizeof(FF_STATISTICS));
	m_statistics.version = FF_STATISTICS_VERSION;
	m_statistics.size = sizeof(FF_STATISTICS);

} //FF_Stats::Reset_Statistics

/*************************************************************************/
// FF_Interface::Get_Event_Data
/*************************************************************************/
Status FF_Interface::Get_Event_Data(
	FF_EVENT_DATA *p_event_data_buffer, 
	U32 size_buffer) 
{
#ifndef _WINDOWS
#ifndef GREEN_HILLS
	CT_ASSERT(IS_ALIGNED_8(p_event_data_buffer), FF_Get_Event_Data);
#endif
#endif

	m_stats.Get_Event_Data(p_event_data_buffer, size_buffer);
	return OK;

} // FF_Interface::Get_Event_Data

/*************************************************************************/
// FF_Interface::Get_Statistics
/*************************************************************************/
Status FF_Interface::Get_Statistics(
	FF_STATISTICS *p_statistics_buffer, 
	U32 size_buffer) 
{
#ifndef _WINDOWS
#ifndef GREEN_HILLS
	CT_ASSERT(IS_ALIGNED_8(p_statistics_buffer), FF_Get_Statistics);
#endif
#endif

	m_stats.Get_Statistics(p_statistics_buffer, size_buffer, m_page_map.Get_Toc());

	// Fill in the high level stats.
	p_statistics_buffer->num_total_pages = Flash_Address::Num_Virtual_Pages();
	p_statistics_buffer->page_size = Flash_Address::Bytes_Per_Page();

	return OK;

} // FF_Interface::Get_Statistics

