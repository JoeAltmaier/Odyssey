/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: CmFrameTable.h
// 
// Description:
// This file defines the frame table of the Cache Manager. 
// The frame table is used to keep track of the state of page frames in the cache. 
// The frame table has one entry for each page frame.  The frame table is
// indexed by the frame table index from the page table.   Frames are
// maintained on two lists:
//	· Clock List
//	· Dirty Clock List
// 
// Update Log 
// 
// 9/7/98 Jim Frandeen: Create file
// 02/08/99 Jim Frandeen: Add write through, 64-bit keys and hash
/*************************************************************************/
#if !defined(CmFrameTable_H)
#define CmFrameTable_H

#include "ErrorLog.h"
#include "CmFrame.h"
#include "CmFrameHandle.h"
#include "CmPageTable.h"
#include "CmStats.h"
#include "List.h"
#include "TraceMon.h"

#ifdef THREADX
#include "tx_api.h"
#else
#include "Nucleus.h"
#endif

#ifdef _WINDOWS
#include <Windows.h>
#endif

class CM_Cache;
class CM_Frame_Table {

public:	 // methods

	// Allocate and initialize frame table object
	static CM_Frame_Table *Allocate(
		CM_Mem					*p_mem, 
		CM_CONFIG				*p_config,
		CM_PREFETCH_CALLBACK	*p_prefetch_callback,
		CM_WRITE_CALLBACK		*p_write_callback,
		CM_Stats				*p_stats,
		void					*p_callback_context,
		U32						 num_page_frames,
		void					*p_page_memory);

    // Open page
    Status Open_Page( 
	    U32 frame_index,
	    U32 mode,
	    Callback_Context *p_callback_context,
	    void **pp_page_frame,
	    CM_PAGE_HANDLE *p_page_handle);

    // Close page
	void Close_Page(CM_CACHE_HANDLE cache_handle,
        CM_Frame_Handle frame_handle, BOOL page_aborted,
		CM_Cache *p_cache);

	// Replace a page in the cache.
	CM_Frame *Replace_Page(U32 flags,
		Callback_Context *p_callback_context);
	
	// Returns pointer to frame, given an index.
	CM_Frame *Get_Frame(U32 frame_index);

	// Return pointer to page frame, given an index.
	void *Get_Page_Frame(U32 frame_index);

	// Check to be sure we don't have too many dirty pages in the cache.
	Status Check_For_Max_Dirty_Pages();

	// Check to see if specified page is dirty.
	int Is_Page_Dirty(U32 frame_index);

	// Set specified page dirty.
	Status Set_Page_Dirty(U32 frame_index);

	Status Flush_Cache(Callback_Context *p_callback_context, 
		CM_CACHE_HANDLE cache_handle, int destroy);

	void Release_Resources();

	void Reset_Statistics();


	void Obtain_Mutex();
	void Release_Mutex();

	void Find_Open_Pages();
	void Find_Waiting_Contexts();

	// Return memory size required by object.
	static U32 Memory_Size(U32 num_pages) ;
	void Validate();

	void Dec_Replaceable_Pages();

	U32 Num_Page_Frames();

private: // helper methods

	CM_Frame *Clock();
	CM_Frame *Write_Clock();
    Status Write_Dirty_Page(CM_CACHE_HANDLE cache_handle);
	static void Flush_Next(void *p_context, Status status);
	void Inc_Replaceable_Pages(LIST *p_list_waiting_contexts);
	void Move_Frame_To_Dirty_Clock(CM_Frame *p_frame);
	void Move_Frame_To_Clock(CM_Frame *p_frame);
	void Validate_Clock();
	void Validate_Frames();
	void Validate_Dirty_Clock();

private:  // member data

	// List of Frames	in the clock.  This is not really a frame.
	// It's a dummy frame so that, when the last frame points to the
	// head of the list, we don't have to check to see if we are 
	// pointing to the head of the list.
	CM_Frame	m_clock_list;

	U32			m_padding; // to align next frame on a doubleword boundary
	
	// List of Frames that are dirty. This is not really a frame.
	CM_Frame	m_dirty_clock_list;
		
	// List of contexts waiting for lock.
	// This is used when we are waiting for pages to be written
	// for a Flush operation.
	LIST		m_list_waiting_to_flush;
		
	// List of contexts waiting for frame.
	LIST		m_list_wait_frame;
		
	// Callbacks saved from Initialize method.
	CM_PREFETCH_CALLBACK	*m_p_prefetch_callback;
	CM_WRITE_CALLBACK		*m_p_write_callback;

	// Pointer to callback context to pass when calling
	// prefetch callback or write callback, 
	// saved from Initialize method.
	void					*m_p_callback_context;

	// Minimum number of reserved pages.  A certain number of
    // pages is reserved for CM_PRIORITY_RESERVE.
	U32	m_num_reserve_pages;
		
	// When dirty page threshold is reached, user's CM_WRITE_CALLBACK
	// is called to write out a dirty page.
	U32	m_dirty_page_writeback_threshold;
		
	// Maximum number of dirty pages that may be in the cache.
	// If CM_Open_Page with CM_MODE_WRITE would surpass this number,
	// CM_ERROR_MAX_DIRTY_PAGES is returned.
	U32	m_dirty_page_error_threshold;

	// Page size in number of bytes.
	U32	m_page_size;
		
	// Number of pages currently in the clock that are replaceable.
    // A page that is locked cannot be replaced.
    // This number must not fall below m_num_reserve_pages.
	U32	m_num_pages_replaceable;
		
	// Number of pages currently in the clock.
	U32	m_num_pages_clock;
		
	// Number of pages that were either accessed or locked the
	// last time the clock hand went all the way around.  
	U32	m_num_pages_working_set;
		
	// Number of pages currently in the dirty list.
	U32	m_num_pages_dirty_clock;
		
	// Number of pages currently being written.
	U32	m_num_pages_being_written;
		
	// Pointer to the next frame in the clock.
	CM_Frame	*m_p_clock_frame;

	// Pointer to the next frame in the dirty clock.
    // We use this to find the next page to write.
	CM_Frame	*m_p_dirty_clock_frame;

	// Number of page frames in the cache.
	U32	 m_num_page_frames;

	// Pointer to array of frames.
	CM_Frame	*m_p_frame_array;

	// Pointer to array of page frames.
	char		*m_p_page_frame_array;

	// Pointer to CM_Stats object
	CM_Stats	*m_p_stats;

#ifdef _DEBUG

	// For debugging, to know if we are stuck waiting for the mutex.
	static int Got_Mutex;
#endif

#ifdef _WINDOWS

		// Mutex to make the Cache Manager thread safe
	HANDLE	m_mutex;

#else
#ifdef THREADX
	TX_SEMAPHORE	m_mutex;
#else
	NU_SEMAPHORE	m_mutex;
#endif

#endif

}; // CM_Frame_Table

/*************************************************************************/
// CM_Frame_Table::Check_For_Max_Dirty_Pages
/*************************************************************************/
inline Status CM_Frame_Table::Check_For_Max_Dirty_Pages()
{
	 if ((m_num_pages_dirty_clock + m_num_pages_being_written + 1)
		 >= m_dirty_page_error_threshold)
	 {
		 m_p_stats->Inc_Error_Max_Dirty_Pages();
		 return	CM_ERROR_MAX_DIRTY_PAGES;
	 }
	 return OK;
}

/*************************************************************************/
// CM_Frame_Table::Dec_Replaceable_Pages
/*************************************************************************/
inline void CM_Frame_Table::Dec_Replaceable_Pages()
{
	CT_ASSERT((m_num_pages_replaceable), CM_Frame_Table::Dec_Replaceable_Pages);
    m_num_pages_replaceable--;
	m_p_stats->Set_Num_Pages_Replaceable(m_num_pages_replaceable);
}

/*************************************************************************/
// CM_Frame_Table::Get_Frame
// Returns pointer to frame, given the index of the frame.
// Note that frame index starts with 1.
/*************************************************************************/
inline CM_Frame * CM_Frame_Table::Get_Frame(U32 frame_index)
{
	CT_ASSERT((frame_index <= m_num_page_frames), CM_Frame_Table::Get_Frame);
	CM_Frame *p_frame = m_p_frame_array + frame_index - 1;

	// If m_p_clock_frame is zero, the frame table is not yet initialized.
	CT_ASSERT(((m_p_clock_frame == 0) || 
		(frame_index == (p_frame)->Get_Frame_Index())), CM_Frame_Table::Get_Frame);

	return p_frame;
}

/*************************************************************************/
// CM_Frame_Table::Get_Page_Frame
// Returns pointer to page frame, given the index of the frame.
// Note that frame index starts with 1.
/*************************************************************************/
inline void * CM_Frame_Table::Get_Page_Frame(U32 frame_index)
{
	CT_ASSERT((frame_index <= m_num_page_frames), CM_Frame_Table::Get_Page_Frame);

	return (frame_index - 1) * m_page_size + m_p_page_frame_array;
}

/*************************************************************************/
// CM_Frame_Table::Is_Page_Dirty
/*************************************************************************/
inline int CM_Frame_Table::Is_Page_Dirty(U32 frame_index)
{
	CT_ASSERT((frame_index <= m_num_page_frames), CM_Frame_Table::CM_Frame_Table);

	return Get_Frame(frame_index)->Is_Page_Dirty();
}

/*************************************************************************/
// CM_Frame_Table::Num_Page_Frames
/*************************************************************************/
inline U32 CM_Frame_Table::Num_Page_Frames()
{
	return m_num_page_frames;
}

/*************************************************************************/
// CM_Frame_Table::Set_Page_Dirty
/*************************************************************************/
inline Status CM_Frame_Table::Set_Page_Dirty(U32 frame_index)
{
	CT_ASSERT((frame_index <= m_num_page_frames), CM_Frame_Table::CM_Frame_Table);

	Get_Frame(frame_index)->Set_Page_Dirty();
	return OK;
}

/*************************************************************************/
// CM_Frame_Table::Obtain_Mutex
/*************************************************************************/
inline void CM_Frame_Table::Obtain_Mutex()
{
#ifdef _WINDOWS
	DWORD  erc = WaitForSingleObject(m_mutex, INFINITE);
	if (erc == WAIT_FAILED)
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"CM_Frame_Table::Obtain_Mutex", 
			"WaitForSingleObject failed",
			GetLastError(),
			0);
#else
#ifdef THREADX
	Status status = tx_semaphore_get(&m_mutex, TX_WAIT_FOREVER);
#else
	Status status = NU_Obtain_Semaphore(&m_mutex, NU_SUSPEND);
#endif

	if (status != OK)
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"CM_Frame_Table::Obtain_Mutex", 
			"NU_Obtain_Semaphore failed",
			status,
			0);
#endif

#ifdef _DEBUG
	Got_Mutex++;
#endif
}

/*************************************************************************/
// CM_Frame_Table::Release_Mutex
/*************************************************************************/
inline void CM_Frame_Table::Release_Mutex()
{
#ifdef _WINDOWS
	if (ReleaseMutex(m_mutex) == 0)
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"CM_Frame_Table::Release_Mutex", 
			"ReleaseMutex failed",
			GetLastError(),
			0);
#else
#ifdef THREADX
	Status status = tx_semaphore_put(&m_mutex);
#else
	Status status = NU_Release_Semaphore(&m_mutex);
#endif
	if (status != OK)
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"CM_Frame_Table::Release_Mutex", 
			"NU_Release_Semaphore failed",
			status,
			0);
#endif

#ifdef _DEBUG
	Got_Mutex--;
#endif
}

/*************************************************************************/
// CM_Frame_Table::Reset_Statistics
/*************************************************************************/
inline void CM_Frame_Table::Reset_Statistics()
{
	m_p_stats->Reset_Statistics();

	// Tell statistics object how many page frames we have
	m_p_stats->Set_Num_Page_Frames(m_num_page_frames);

	// Initialize minimum m_num_pages_replaceable for statistics.
	m_p_stats->Set_Num_Pages_Replaceable_Min(m_num_pages_replaceable);

	// Copy calculated numbers into statistics record.
	m_p_stats->Set_Num_Reserve_Pages(m_num_reserve_pages);
	m_p_stats->Set_Dirty_Page_Writeback_Threshold(m_dirty_page_writeback_threshold);
	m_p_stats->Set_Dirty_Page_Error_Threshold(m_dirty_page_error_threshold);

}

/*************************************************************************/
// CM_Frame_Table::Memory_Size
/*************************************************************************/
inline U32 CM_Frame_Table::Memory_Size(U32 num_pages)
{
	return (num_pages * sizeof(CM_Frame)) 
		+ sizeof(CM_Frame_Table) 
		+ 16; // plus alignment

} // Memory_Size

/*************************************************************************/
// CM_Frame_Table::Move_Frame_To_Dirty_Clock
/*************************************************************************/
inline void CM_Frame_Table::Move_Frame_To_Dirty_Clock(CM_Frame *p_frame)
{
	// This page must be mapped.
	CT_ASSERT((p_frame->Get_Page_Number() != -1), Move_Frame_To_Dirty_Clock);
	CT_ASSERT((p_frame->Get_Frame_Index()), Move_Frame_To_Dirty_Clock);

	// If the page is the next clock page,...
	if (p_frame == m_p_clock_frame)

		// Update the clock hand
		m_p_clock_frame = (CM_Frame *)p_frame->m_list.forward_link;

	// Remove this frame from the clock list.
	CT_ASSERT((!LIST_IS_EMPTY(&m_clock_list.m_list)), CM_Frame_Table::Move_Frame_To_Dirty_Clock);
	LIST_REMOVE(&p_frame->m_list);

	// Decrement the number of frames in the clock list
	CT_ASSERT((m_num_pages_clock != 0), CM_Frame_Table::Move_Frame_To_Dirty_Clock);
	m_num_pages_clock--;

	// Link this frame on the dirty clock list.
	LIST_INSERT_TAIL(&m_dirty_clock_list.m_list, &p_frame->m_list);
	m_num_pages_dirty_clock++;

} // Move_Frame_To_Dirty_Clock

/*************************************************************************/
// CM_Frame_Table::Move_Frame_To_Clock
/*************************************************************************/
inline void CM_Frame_Table::Move_Frame_To_Clock(CM_Frame *p_frame)
{
	CT_ASSERT((p_frame->Get_Page_Number() != -1), Move_Frame_To_Dirty_Clock);
	CT_ASSERT((p_frame->Get_Frame_Index()), Move_Frame_To_Dirty_Clock);

	// If the page is the next clock page,...
	if (p_frame == m_p_dirty_clock_frame)

		// Update the clock hand
		m_p_dirty_clock_frame = (CM_Frame *)p_frame->m_list.forward_link;

	// Remove this frame from the dirty clock list.
	CT_ASSERT((!LIST_IS_EMPTY(&m_dirty_clock_list.m_list)), CM_Frame_Table::Move_Frame_To_Clock);
	LIST_REMOVE(&p_frame->m_list);

	// Decrement the number of frames in the dirty clock list
	CT_ASSERT((m_num_pages_dirty_clock != 0), CM_Frame_Table::Move_Frame_To_Clock);
	m_num_pages_dirty_clock--;

	// Link this frame on the clock list.
	LIST_INSERT_TAIL(&m_clock_list.m_list, &p_frame->m_list);
	m_num_pages_clock++;

} // Move_Frame_To_Clock

#endif // CmFrameTable_H

