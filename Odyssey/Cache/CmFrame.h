/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: CmFrame.h
// 
// Description:
// This file defines the Frame of the Cache Manager. 
// A frame describes a page in the cache.  A frame is always in a list.
// 
// Update Log 
// 
// 9/6/98 Jim Frandeen: Create file
// 02/08/99 Jim Frandeen: Add write through, 64-bit keys and hash
/*************************************************************************/
#if !defined(CmFrame_H)
#define CmFrame_H

#include "ErrorLog.h"
#include "CmFrameHandle.h"
#include "CmStats.h"
#include "List.h"

// Define page states.  State determines what list the page is in.
typedef enum {
	CM_PAGE_STATE_CLEAN,
	CM_PAGE_STATE_DIRTY,
	CM_PAGE_STATE_WRITE
} CM_PAGE_STATE;

// Define page state changes
typedef enum {
    CM_JUST_BECAME_DIRTY,
	CM_JUST_BECAME_DIRTY_AGAIN,
    CM_JUST_BECAME_CLEAN,
    CM_JUST_BECAME_CLEAN_REPLACEABLE,
    CM_JUST_BECAME_REPLACEABLE,
    CM_JUST_BECAME_UNREPLACEABLE,
	CM_JUST_BECAME_NOT_PRESENT,
    CM_STATE_NO_CHANGE
} CM_STATE_CHANGE;

class CM_Frame {

public:	 // methods

	// Initialize object
	void Initialize(U32 frame_index, CM_PAGE_STATE page_state);

	// Open Page
	// Return OK if page can be locked.
	// Return CM_ERROR_PAGE_LOCKED if page locked; context is queued.
	Status Open_Page(
		U32 flags,
		Callback_Context *p_callback_context,
		CM_PAGE_HANDLE *p_page_handle,
        CM_STATE_CHANGE *p_state_change,
		CM_Stats *p_stats);

	// Close Page
	CM_STATE_CHANGE Close_Page(CM_Frame_Handle frame_handle, CM_Stats *p_stats,
		LIST *p_list_waiting_contexts, BOOL page_aborted);

	// Abort Page
	CM_STATE_CHANGE Abort_Page(CM_Frame_Handle frame_handle, CM_Stats *p_stats,
		LIST *p_list_waiting_contexts);

	// Lock Page when the page being opened is not present.
	CM_PAGE_HANDLE Lock_Page_Not_Present(U32 flags, CM_Stats *p_stats);

	// Lock/Unlock Page for writeback.
	CM_PAGE_HANDLE Lock_Page_Writeback(CM_Stats *p_stats);
	void Unlock_Page_Writeback(CM_Stats *p_stats, LIST *p_list_waiting_contexts);

	// Get/Set page number
	void Set_Page_Number(I64 page_number);
	I64 Get_Page_Number();

	// Get/Set page state
	void Set_Page_State(CM_PAGE_STATE state);
	CM_PAGE_STATE Get_Page_State();

	// Get/Set next hash entry
	void Set_Next_Hash_Entry(CM_Frame *p_frame);
	CM_Frame *Get_Next_Hash_Entry();

	// Return index of frame
	U32 Get_Frame_Index();

	// Return true if page is locked.
	int Is_Page_Locked();

	// Return true if this is a dummy frame with a frame index of 0.
	// This would be the head of a list
	int Is_Dummy_Frame();

	// Return true if page is write locked.
	int Is_Page_Write_Locked();

	// Return true if page is remap locked.
	int Is_Page_Remap_Locked();

	// Return true if page is dirty.
	int Is_Page_Dirty();
	void Set_Page_Dirty();

	// Return true if page is clean.
	int Is_Page_Clean();
	void Set_Page_Clean();

	// Return true if page was accessed the last time around the clock.
	int Is_Page_Accessed();

	void Set_Page_Accessed();
	void Set_Page_Not_Accessed();

	// For debugging, assure that frame can be replaced.
	int Is_Replaceable();

	// Return size of frame.  The size is calculated because we round it
	// to a multiple of 8 bytes so that the page frame can follow.
	static U32 Size();
	
	void Find_Open_Pages();
	void Find_Waiting_Contexts();

private: // helper methods

	// Unlock page methods
	void Unlock_Page_Read(CM_Stats *p_stats, LIST *p_list_waiting_contexts);
	void Unlock_Page_Remap(CM_Stats *p_stats, LIST *p_list_waiting_contexts);
	void Unlock_Page_Read_Not_Present(CM_Stats *p_stats, LIST *p_list_waiting_contexts);
	void Unlock_Page_Write(CM_Stats *p_stats, LIST *p_list_waiting_contexts);
	void Wakeup_Contexts_Waiting_For_Lock(CM_Stats *p_stats, LIST *p_list_waiting_contexts);

public:	// member data

	// The first member must be a list object so that we can use
	// the LIST primitives.
	LIST		m_list;

private:  // member data

	// The lock wait list is a list of contexts waiting for the page to be unlocked.  
	// If the page is locked for read, this list will include only contexts that wish
	// to open the page for write.  If the page is locked for write, this list may
	// include contexts that wish to open the page for read and contexts that wish to
	// open the page for write.  When the lock count is decremented to zero, all
	// contexts on this wait list will be scheduled to run.  If any of the scheduled
	// contexts have conflicting open modes, then the first context scheduled will run,
	// and the conflicting contexts will be placed back on the wait list.
	LIST		m_list_wait_for_unlock;

	I64	m_page_number;

	// Index assigned to this frame.  Indexes start with 1.
	// A frame must know its index so that it can create a page handle.
	U32	m_frame_index;

	// Pointer to the next frame that has the same hash.
	// This is only used for an associative cache.
	CM_Frame *m_p_next_hash_entry;

#if 0
	/*
	This works in Visual C++, but not in Metrowerks
	// A page has three locks 
    union 
    {
        union 
        {
            union
            {
                struct 
                {
					// A page can have a number of read locks,
					// and they exclude the write lock.
                    U16     m_num_read_locks;

					// A page can have only one write lock, 
					// and it excludes read locks.
                    char    m_write_lock; // on or off
                };
                BF  m_read_write_lock: 24;
            };

			// A page is locked for writeback if the page is being
			// written.  This lock is not exclusive.
			// It only prevents the page from being replaced.
            char m_writeback_lock; // on or off
        };
        U32 m_lock;
    };

  */
#endif

	// A page has four locks 
	// (1) A page can have a number of read locks,
	// and they exclude the write lock.
    U16     m_num_read_locks;

	// (2) A page can have only one write lock, 
	// and it excludes read locks and remap locks.
    char    m_write_lock; // on or off

	// (3) A page is locked for writeback if the page is being
	// written.  This lock is exclusive with the remap locks.
    char m_writeback_lock; // on or off

	// (4) A page is locked for remap if the page must be
	// written to a different location in backing store.  
	// This lock is exclusive with the write lock and the writeback lock.
    U16 m_num_remap_locks;

	CM_PAGE_STATE	m_page_state;

	char m_is_page_dirty;

	char m_is_page_accessed;

	// The actual page frame follows right after the frame.
	// The size of the page frame depends on the page size.
	// The actual starting byte depends on the size of the frame.
	// We align to an 8-byte boundary.

}; // CM_Frame

/*************************************************************************/
// CM_Frame::Get_Frame_Index
/*************************************************************************/
inline U32 CM_Frame::Get_Frame_Index()
{
	return m_frame_index;
}

/*************************************************************************/
// CM_Frame::Get_Next_Hash_Entry
// Return pointer to the next frame with the same hash.
/*************************************************************************/
inline CM_Frame * CM_Frame::Get_Next_Hash_Entry()
{
	return m_p_next_hash_entry;
}

/*************************************************************************/
// CM_Frame::Get_Page_Number
/*************************************************************************/
inline I64 CM_Frame::Get_Page_Number()
{
	return m_page_number;
}

/*************************************************************************/
// CM_Frame::Get_Page_State
/*************************************************************************/
inline CM_PAGE_STATE CM_Frame::Get_Page_State()
{
	return m_page_state;
}

/*************************************************************************/
// CM_Frame::Is_Dummy_Frame
/*************************************************************************/
inline int CM_Frame::Is_Dummy_Frame()
{
	return (m_frame_index == 0);
}

/*************************************************************************/
// CM_Frame::Is_Replaceable
// For debugging, assure that this frame can be replaced.
/*************************************************************************/
inline int CM_Frame::Is_Replaceable()
{
	CT_ASSERT((LIST_IS_EMPTY(&m_list_wait_for_unlock)), CM_Frame::Is_Replaceable);
	CT_ASSERT((!Is_Page_Locked()), CM_Frame::Is_Replaceable);
	CT_ASSERT((m_is_page_dirty == 0), CM_Frame::Is_Replaceable);
	CT_ASSERT((m_page_state == CM_PAGE_STATE_CLEAN), CM_Frame::Is_Replaceable);
	return 1;
}

/*************************************************************************/
// CM_Frame::Is_Page_Clean
/*************************************************************************/
inline int CM_Frame::Is_Page_Clean()
{
	return !m_is_page_dirty;
}

/*************************************************************************/
// CM_Frame::Is_Page_Dirty
/*************************************************************************/
inline int CM_Frame::Is_Page_Dirty()
{
	return m_is_page_dirty;
}

/*************************************************************************/
// CM_Frame::Is_Page_Locked
/*************************************************************************/
inline int CM_Frame::Is_Page_Locked()
{
	return (m_num_read_locks | m_write_lock | m_writeback_lock | m_num_remap_locks);
}

/*************************************************************************/
// CM_Frame::Size
// Return size rounded to a multiple of 8 bytes so that frame can follow.
/*************************************************************************/
inline U32 CM_Frame::Size()
{
	return ((sizeof (CM_Frame) + 7) / 8) * 8;
}

/*************************************************************************/
// CM_Frame::Is_Page_Write_Locked
/*************************************************************************/
inline int CM_Frame::Is_Page_Write_Locked()
{
	return m_write_lock;
}

/*************************************************************************/
// CM_Frame::Is_Page_Remap_Locked
/*************************************************************************/
inline int CM_Frame::Is_Page_Remap_Locked()
{
	return m_num_remap_locks;
}

/*************************************************************************/
// CM_Frame::Is_Page_Accessed
// Each time we test the accessed flag, turn it off.
/*************************************************************************/
inline int CM_Frame::Is_Page_Accessed()
{
	char is_page_accessed = m_is_page_accessed;
	m_is_page_accessed = 0;
	return is_page_accessed;
}

/*************************************************************************/
// CM_Frame::Set_Next_Hash_Entry
/*************************************************************************/
inline void CM_Frame::Set_Next_Hash_Entry(CM_Frame *p_frame)
{
	m_p_next_hash_entry = p_frame;
}

/*************************************************************************/
// CM_Frame::Set_Page_Accessed
/*************************************************************************/
inline void CM_Frame::Set_Page_Accessed()
{
	m_is_page_accessed = 1;
}

/*************************************************************************/
// CM_Frame::Set_Page_Not_Accessed
/*************************************************************************/
inline void CM_Frame::Set_Page_Not_Accessed()
{
	m_is_page_accessed = 0;
}

/*************************************************************************/
// CM_Frame::Set_Page_Clean
/*************************************************************************/
inline void CM_Frame::Set_Page_Clean()
{
	m_is_page_dirty = 0;
}

/*************************************************************************/
// CM_Frame::Set_Page_Dirty
/*************************************************************************/
inline void CM_Frame::Set_Page_Dirty()
{
	m_is_page_dirty = 1;
}

/*************************************************************************/
// CM_Frame::Set_Page_Number
/*************************************************************************/
inline void CM_Frame::Set_Page_Number(I64 page_number)
{
	m_page_number = page_number;
}

/*************************************************************************/
// CM_Frame::Set_Page_State
/*************************************************************************/
inline void CM_Frame::Set_Page_State(CM_PAGE_STATE page_state)
{

#ifdef _DEBUG
	switch (page_state)
	{
	case CM_PAGE_STATE_CLEAN:

		// If new state is clean, old state can only be...
		CT_ASSERT((m_page_state == CM_PAGE_STATE_WRITE), CM_Frame::Is_Replaceable);
		break;

	case CM_PAGE_STATE_DIRTY:

		// If new state is dirty, old state can only be...
		CT_ASSERT(((m_page_state == CM_PAGE_STATE_CLEAN)
			|| (m_page_state == CM_PAGE_STATE_WRITE)), CM_Frame::Is_Replaceable);
		break;

	case CM_PAGE_STATE_WRITE:

		// If new state is write, old state can only be...
		CT_ASSERT((m_page_state == CM_PAGE_STATE_DIRTY), CM_Frame::Is_Replaceable);
		break;
	}

#endif // _DEBUG
	m_page_state = page_state;
}

#endif // CmFrame_H
