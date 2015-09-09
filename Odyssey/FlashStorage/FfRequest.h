/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfRequest.h
// 
// Description:
// A request callback context is created when a BSA read or write
// request is received.
// 
// 8/27/98 Jim Frandeen: Create file
/*************************************************************************/

#if !defined(FfRequest_H)
#define FfRequest_H

#include "Callback.h"
#include "FfInterface.h"
#include "FfSgl.h"
class FF_Controller;

class FF_Request_Context : public Callback_Context
{
public:

	// Called from FF_Read and FF_Write
	static Status Create_Child_Read_Write_Contexts(
		FF_Interface *p_flash,
		FF_SGL *p_sgl, 
		U32 transfer_byte_count,
		I64 logical_byte_address,
		void *p_context,
		FF_CALLBACK *p_callback,
		Callback read_write_callback);

	// Called from FF_Format
	static Status Format(
		FF_Interface *p_flash,
		FF_CONFIG *p_config,
		void *	p_context,
		FF_CALLBACK *p_completion_callback);


	static void Create_Child_Format_Contexts(void *p_context, Status status);
	static void Read_Cache					(void *p_context, Status status);
	static void Write_Cache					(void *p_context, Status status);
	static void Write_DMA_Buffer_To_Cache	(void *p_context, Status status);

private: // callback methods

	static void Format_Erase_Complete			(void *p_context, Status status);
	static void Format_Page_Map					(void *p_context, Status status);
	static void Open_Existing_Complete			(void *p_context, Status status);
	static void Open_Create_Complete			(void *p_context, Status status);
	static void Create_Complete					(void *p_context, Status status);
	static void Page_Map_Opened					(void *p_context, Status status);
	static void Read_DMA_Complete				(void *p_context, Status status);
	static void Read_Request_Complete			(void *p_context, Status status);
	static void Read_DMA_Cache_To_Buffer		(void *p_context, Status status);
	static void Read_From_Cache					(void *p_context, Status status);
	static void Read_Write_Transfer_Complete	(void *p_context, Status status);
	static void Read_Cache_Complete				(void *p_context, Status status);
	static void Remap_Corrected_Page			(void *p_context, Status status);
	static void Write_DMA_Complete				(void *p_context, Status status);
	static void Write_Request_Complete			(void *p_context, Status status);
	static void Write_Transfer_Complete			(void *p_context, Status status);

private: // helper methods

	void Terminate_With_Error(Status status);

	Status Create_Read_Write_Child_Context(U32 function_code);
	
	Status Create_Format_Child_Context(U32 function_code);

	static void Abort_Open(void *p_context, Status status);

private: // member data

	// friends can access  context data
	friend class FF_Interface;
	friend class FF_Controller;
	
	// m_logical_byte_address is used to save the
	// point of error for the return.   
	I64					 m_logical_byte_address;

	// Status returned from opening bad block table --
	// allows us to continue if bad block table is not present
	// for testing and debugging.
	Status				 m_bad_block_status;

	// Pointer to flash object.
	FF_Interface		*m_p_flash;

	// In parent, total transfer byte count.
	// In child, transfer byte count for this context.
	U32					 m_transfer_byte_count;

	// callback passed as parameter from user
	FF_CALLBACK			*m_p_completion_callback;

	// p_context to be used for callback
	void				*m_p_context; 

	// parameters for child context
	U32					 m_virtual_address;

	// sgl used for transfer to/from client buffers.
	FF_SGL				 m_sgl;

	void				*m_p_data_buffer;
	void				*m_p_page_frame;
	CM_PAGE_HANDLE		 m_page_handle;
	FF_CONFIG			*m_p_config;

}; // FF_Request_Context


#endif //   FfRequest_H
