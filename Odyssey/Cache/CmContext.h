/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmContext.h
// 
// Description:
// 
// 
// Update Log 
// 8/27/98 Jim Frandeen: Create file
/*************************************************************************/

#if !defined(CmContext_H)
#define CmContext_H

#include "Callback.h"
#include "CmFrameTable.h"

class CM_Cache_Context : public Callback_Context
{
public:
	
	CM_CACHE_HANDLE				 m_cache_handle;
	CM_Frame_Table				*m_p_frame_table;
	U32					 m_destroy;
}; // CM_Cache_Context


#endif //   CmContext_H
