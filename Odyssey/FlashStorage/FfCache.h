/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfCache.h
// 
// Description:
// This file implements the Flash File cache methods that interface
// to the cache manager. 
// 
// 8/27/98 Jim Frandeen: Create file
/*************************************************************************/

#if !defined(FbCache_H)
#define FbCache_H

#include "Cache.h"
#include "Callback.h"
#include "List.h"
#include "FlashAddress.h"

class FF_Interface;

class FF_Cache_Context : public Callback_Context
{
public:
	
	static void Close_Cache						(void *p_context, Status status);

	void Set_Cache(CM_CACHE_HANDLE cache_handle);

	static Status Write_Cache_Callback(CM_CACHE_HANDLE cache_handle, 
		I64 page_number,
		void *p_page_frame,
		CM_PAGE_HANDLE page_handle);

private: // callback methods

	static void Write_Cache_Callback_Complete	(void *p_context, Status status);

private: // member data

	// FF_Interface is a friend so it can access its context data
	friend FF_Interface;

	CM_PAGE_HANDLE 			 m_page_handle;
	CM_CACHE_HANDLE			 m_cache_handle;
	Flash_Address				 m_flash_address;
	void					*m_p_page_frame;
	FF_Interface			*m_p_flash;

}; // FF_Cache_Context

inline void FF_Cache_Context::Set_Cache(CM_CACHE_HANDLE cache_handle)
{
	m_cache_handle = cache_handle;
}

#endif //   FbCache_H
