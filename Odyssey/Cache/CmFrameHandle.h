/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: CmFrameHandle.h
// 
// Description:
// This file defines the interface to the CM_Frame_Handle of the Cache Manager. 
// 
// Update Log 
// 
// 9/6/98 Jim Frandeen: Create file
// 02/08/99 Jim Frandeen: Add write through, 64-bit keys and hash
/*************************************************************************/
#if !defined(CmFrameHandle_H)
#define CmFrameHandle_H

#include "Cache.h"

// Open mode values
// First three are defined in Cache.h because they are external
//#define CM_OPEN_MODE_READ						0x0FE
//#define CM_OPEN_MODE_WRITE					0x0FD
#define CM_OPEN_MODE_REMAP						0x0FC
#define CM_OPEN_MODE_READ_NOT_PRESENT			0x0FB
#define CM_OPEN_MODE_REMAP_NOT_PRESENT			0x0FA
#define CM_OPEN_MODE_WRITE_BACK					0x0F9
#define CM_OPEN_MODE_WRITE_NOT_PRESENT			0x0F8
#define CM_OPEN_MODE_WRITE_THROUGH				0x0F7
#define CM_OPEN_MODE_WRITE_THROUGH_NOT_PRESENT	0x0F6

#define CM_OPEN_MODE(flags) (flags & 0xFF)
#define CM_SET_OPEN_MODE(flags, mode) flags = (flags & 0xFFFFFF00) | mode

// Page Handle is really a structure in one 32-bit word.
// The high order byte is the open mode, and the low 24 bits are the frame index.
class CM_Frame_Handle
{
public:  // methods

	// constructor
	CM_Frame_Handle();
	CM_Frame_Handle(CM_PAGE_HANDLE page_handle);

	// Return index of frame
	U32 Get_Frame_Index();

	// Return open mode
	U32 Get_Open_Mode();

	// Return handle as an unsigned number for Debugging trace.
	U32 Get_Page_Handle();

	// Return a page handle
	static CM_PAGE_HANDLE Create_Page_Handle(U32 flags, U32 frame_index);

private:  // member data
/*
	This one works in Visual C++, but not in Metrowerks
	union
	{
		struct {
			BF	m_frame_index:	24;
			BF	m_open_mode:	 8;
		};
		U32 m_page_handle;
	};
*/
	typedef struct {
			BF	m_frame_index:	24;
			BF	m_open_mode:	 8;
		} Page_Handle;
		
	union
	{
		Page_Handle		 m_handle;
		U32		 m_page_handle;
	};
	
}; // CM_Frame_Handle


/*************************************************************************/
// Constructor
/*************************************************************************/
inline CM_Frame_Handle::CM_Frame_Handle()
{
	m_page_handle = 0;
	m_handle.m_frame_index = 0;
} // CM_Frame_Handle::CM_Frame_Handle

/*************************************************************************/
// Constructor
// Create a CM_Frame_Handle from a CM_PAGE_HANDLE
/*************************************************************************/
inline CM_Frame_Handle::CM_Frame_Handle(CM_PAGE_HANDLE page_handle)
{
	m_page_handle = page_handle;

} // CM_Frame_Handle::CM_Frame_Handle

/*************************************************************************/
// CM_Frame_Handle::Create_Page_Handle
// Create a CM_PAGE_HANDLE
/*************************************************************************/
inline CM_PAGE_HANDLE CM_Frame_Handle::Create_Page_Handle(
	U32 flags, U32 frame_index)
{
	CM_Frame_Handle frame_handle;
	frame_handle.m_handle.m_frame_index = frame_index;
	frame_handle.m_handle.m_open_mode = CM_OPEN_MODE(flags);
	return frame_handle.m_page_handle;

} // CM_Frame_Handle::Create_Page_Handle

/*************************************************************************/
// CM_Frame_Handle::Get_Frame_Index
// Return 0 if handle is invalid.
/*************************************************************************/
inline U32 CM_Frame_Handle::Get_Frame_Index()
{
	switch (m_handle.m_open_mode)
	{
	case CM_OPEN_MODE_READ:
	case CM_OPEN_MODE_READ_NOT_PRESENT:
	case CM_OPEN_MODE_REMAP:
	case CM_OPEN_MODE_REMAP_NOT_PRESENT:
	case CM_OPEN_MODE_WRITE:
	case CM_OPEN_MODE_WRITE_BACK:
	case CM_OPEN_MODE_WRITE_NOT_PRESENT:
	case CM_OPEN_MODE_WRITE_THROUGH:
	case CM_OPEN_MODE_WRITE_THROUGH_NOT_PRESENT:
		return m_handle.m_frame_index;

	default:
		// invalid handle
		break;
	}

	return 0;

} // CM_Frame_Handle::Get_Frame_Index

/*************************************************************************/
// CM_Frame_Handle::Get_Open_Mode
// Return 0 if handle is invalid.
/*************************************************************************/
inline U32 CM_Frame_Handle::Get_Open_Mode()
{
	switch (m_handle.m_open_mode)
	{
	case CM_OPEN_MODE_READ:
	case CM_OPEN_MODE_READ_NOT_PRESENT:
	case CM_OPEN_MODE_REMAP:
	case CM_OPEN_MODE_REMAP_NOT_PRESENT:
	case CM_OPEN_MODE_WRITE:
	case CM_OPEN_MODE_WRITE_BACK:
	case CM_OPEN_MODE_WRITE_NOT_PRESENT:
	case CM_OPEN_MODE_WRITE_THROUGH:
	case CM_OPEN_MODE_WRITE_THROUGH_NOT_PRESENT:
		return m_handle.m_open_mode;

	default:
		// invalid handle
		break;
	}

	return 0;

} // CM_Frame_Handle::Get_Open_Mode

/*************************************************************************/
// CM_Frame_Handle::Get_Page_Handle
/*************************************************************************/
inline U32 CM_Frame_Handle::Get_Page_Handle()
{
	return m_page_handle;

} // CM_Frame_Handle::Get_Page_Handle

#endif // CmFrameHandle_H


