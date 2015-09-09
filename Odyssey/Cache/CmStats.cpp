/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmStats.cpp
// 
// Description:
// This file implements the CM_Stats class methods of the 
// Cache Manager 
// 
// Update Log 
// 
// 9/23/98 Jim Frandeen: Create file
// 02/08/99 Jim Frandeen: Add write through, 64-bit keys and hash
/*************************************************************************/

#include "CmMem.h"
#include "CmStats.h"
#include <String.h>

/*************************************************************************/
// CM_Stats::Allocate
// Allocate object and return a pointer to it.  Return 0 if none available.
/*************************************************************************/
CM_Stats * CM_Stats::Allocate(CM_Mem *p_mem) 
{
	// Allocate CM_Stats object
	CM_Stats *p_stats = 
		(CM_Stats *)p_mem->Allocate(sizeof(CM_Stats));

	if (p_stats == 0)
		return 0;
	
	// Allocate CM_EVENT_DATA object
	p_stats->m_p_event_data = 
		(CM_EVENT_DATA *)p_mem->Allocate(sizeof(CM_EVENT_DATA));

#ifndef _WINDOWS
	CT_ASSERT(IS_ALIGNED_8(p_stats->m_p_event_data), CM_Stats::Allocate);
#endif

	if (p_stats->m_p_event_data == 0)
		return 0;

	// Allocate CM_STATISTICS object
	p_stats->m_p_statistics = 
		(CM_STATISTICS *)p_mem->Allocate(sizeof(CM_STATISTICS));
#ifndef _WINDOWS
	CT_ASSERT(IS_ALIGNED_8(p_stats->m_p_statistics), CM_Stats::Allocate);
#endif

	if (p_stats->m_p_statistics == 0)
		return 0;

	// Return pointer to CM_Stats object.
	return p_stats;
	
} //CM_Stats::Allocate

/*************************************************************************/
// CM_Stats::Reset_Event_Data
/*************************************************************************/
void CM_Stats::Reset_Event_Data() 
{
	// Initialize event data
#ifndef _WINDOWS
	CT_ASSERT(IS_ALIGNED_8(m_p_event_data), CM_Stats::Reset_Event_Data);
#endif

	memset(m_p_event_data, 0, sizeof(CM_EVENT_DATA));
	m_p_event_data->version = CM_EVENT_DATA_VERSION;
	m_p_event_data->size = sizeof(CM_EVENT_DATA);

} //CM_Mem::Initialize

/*************************************************************************/
// CM_Stats::Reset_Statistics
/*************************************************************************/
void CM_Stats::Reset_Statistics() 
{
	// Initialize statistics data
#ifndef _WINDOWS
	CT_ASSERT(IS_ALIGNED_8(m_p_statistics), CM_Stats::Reset_Statistics);
#endif

	memset(m_p_statistics, 0, sizeof(CM_STATISTICS));
	m_p_statistics->version = CM_STATISTICS_VERSION;
	m_p_statistics->size = sizeof(CM_STATISTICS);

} //CM_Stats::Reset_Statistics

