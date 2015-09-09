/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpI2O.cpp
// 
// Description:
// This file implements interfaces to the I2O message.
// 
// Update Log 
// 4/14/98 Jim Frandeen: Create file
// 5/5/98 Jim Frandeen: Use C++ comment style
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 9/18/98 Michael G. Panas: Convert to C++ file, use Message Class to
//                           access SGL in I2O format.
// 11/30/98 Michael G. Panas: New memory allocation methods
// 02/17/99 Michael G. Panas: convert to new Message format
/*************************************************************************/
#include "FcpCommon.h"
#include "FcpI2O.h"
#include "FcpMessage.h"
#include "CTIdLun.h"
#include "FcpMessageFormats.h"
#include "FcpMemory.h"

#include "Message.h"

/*************************************************************************/
// FCP_I2O_Create
// Create FCP_I2O object
// pp_memory points to a pointer to memory to be used.
// On return, this pointer is updated.
/*************************************************************************/
STATUS	FCP_I2O_Create(PINSTANCE_DATA Id)
{
 	FCP_TRACE_ENTRY(FCP_I2O_Create);
	
	return NU_SUCCESS;
	
} // FCP_I2O_Create

/*************************************************************************/
// FCP_I2O_Destroy
/*************************************************************************/
void	FCP_I2O_Destroy()
{
 	FCP_TRACE_ENTRY(FCP_I2O_Destroy);
		
} // FCP_I2O_Destroy

/*************************************************************************/
// FCP_I2O_Add_Payload
// Add Payload area to an I2O message
/*************************************************************************/
void FCP_I2O_Add_Payload(void *p_msg,
	void *p_payload, UNSIGNED payload_size)
{
	Message		*pMsg = (Message *)p_msg;
	
 	FCP_TRACE_ENTRY(FCP_I2O_Add_Payload);
		
	pMsg->AddPayload(p_payload, payload_size);
	
} // FCP_I2O_Add_Payload
	
/*************************************************************************/
// FCP_I2O_Add_Reply_Payload
// Add a Reply Payload area to an I2O message
/*************************************************************************/
void FCP_I2O_Add_Reply_Payload(void *p_msg,
	void *p_payload, UNSIGNED payload_size,
	STATUS DetailedStatusCode)
{
	Message		*pMsg = (Message *)p_msg;
	
 	FCP_TRACE_ENTRY(FCP_I2O_Add_Reply_Payload);
		
	pMsg->AddReplyPayload(p_payload, payload_size);
	
	// return the DetailedStatusCode
	pMsg->DetailedStatusCode = DetailedStatusCode;
	
} // FCP_I2O_Add_Reply_Payload
	
/*************************************************************************/
// FCP_I2O_Add_SGL_Entry
// Add entry to SGL in I2O message
/*************************************************************************/
void FCP_I2O_Add_SGL_Entry(void *p_msg,
	void *p_buffer, UNSIGNED buffer_size, UNSIGNED SGL_index)
{
	Message		*pMsg = (Message *) p_msg;
	
 	FCP_TRACE_ENTRY(FCP_I2O_Add_SGL_Entry);
	
	pMsg->AddSgl(SGL_index, (void *)p_buffer, buffer_size);
	
	FCP_PRINT_HEX(TRACE_L8, "\n\rp_buffer = ", (U32) p_buffer);
	FCP_PRINT_HEX(TRACE_L8, "  buffer_size = ", (U32) buffer_size);
	
} // FCP_I2O_Add_SGL_Entry

/*************************************************************************/
// FCP_I2O_Get_Payload
// Get the address of the Payload area from a message
/*************************************************************************/
void *FCP_I2O_Get_Payload(void *p_msg)
{
	Message		*pMsg = (Message *)p_msg;
	
 	FCP_TRACE_ENTRY(FCP_I2O_Get_Payload);
		
	return ((void *)pMsg->GetPPayload());
	
} // FCP_I2O_Get_Payload
	
/*************************************************************************/
// FCP_I2O_Get_ID_LUN
// Get the address of the ID / LUN struct area from a message
/*************************************************************************/
void *FCP_I2O_Get_ID_LUN(void *p_msg)
{
	Message		*pMsg = (Message *)p_msg;
	SCB_PAYLOAD	*pP = (SCB_PAYLOAD *)pMsg->GetPPayload();
	
 	FCP_TRACE_ENTRY(FCP_I2O_Get_ID_LUN);
	FCP_PRINT_HEX(TRACE_L8, "\n\rID/LUN=", (U32) *((U32*)&pP->IdLun));
		
	return ((void *)&pP->IdLun);
	
} // FCP_I2O_Get_ID_LUN
	
/*************************************************************************/
// FCP_I2O_Get_DetailedStatusCode
// Get the DetailedStatusCode from a message
/*************************************************************************/
STATUS FCP_I2O_Get_DetailedStatusCode(void *p_msg)
{
	Message		*pMsg = (Message *)p_msg;
	
 	FCP_TRACE_ENTRY(FCP_I2O_Get_DetailedStatusCode);
	FCP_PRINT_HEX(TRACE_L8, "\n\rDetailedStatusCode=",
				(U32) pMsg->DetailedStatusCode);
		
	return (pMsg->DetailedStatusCode);
	
} // FCP_I2O_Get_DetailedStatusCode
	
/*************************************************************************/
// FCP_I2O_Get_SGL_Entry
// Fetch the SGL entry from I2O message, insert data at p_data_segment
/*************************************************************************/
void FCP_I2O_Get_SGL_Data_Segment(void* p_msg,
	DATA_SEGMENT_DESCRIPTOR *p_data_segment, UNSIGNED SGL_index)
{
	Message		*pMsg = (Message *) p_msg;
	U8			*buffer;
	U32			length;
	
 	FCP_TRACE_ENTRY(FCP_I2O_Get_SGL_Data_Segment);
 	
	pMsg->GetSgl(SGL_index, (void**) &buffer, (unsigned long *) &length);
	
	FCP_PRINT_HEX(TRACE_L8, "\n\rbuffer = ", (U32) buffer);
	FCP_PRINT_HEX(TRACE_L8, "  length = ", (U32) length);
	
	// p_data_segment points to a descriptor in an IOCB so we need to
	// translate Big Endian to Little Endian format
	//buffer = (U8*)((UNSIGNED)buffer & ~KSEG1);  // make physical for DMA
	buffer = (U8*)FCP_Get_DMA_Address((void *)buffer);  // make physical for DMA
	p_data_segment->Address = BYTE_SWAP32((UNSIGNED)buffer);
	p_data_segment->Length = BYTE_SWAP32(length);

	FCP_DUMP_HEX(TRACE_L8, "\n\rdata segement = ", (U8 *) p_data_segment, 8);
	
} // FCP_I2O_Get_SGL_Data_Segment
	
/*************************************************************************/
// FCP_I2O_Get_SGL_Entries
// Fetch the SGL entries from I2O message
/*************************************************************************/
void FCP_I2O_Get_SGL_Entries(FCP_EVENT_CONTEXT *p_context, IOCB_COMMAND_TYPE4 *p_command)
{
	DATA_SEGMENT_DESCRIPTOR *p_data_segment;
	Message					*pMsg = (Message *) p_context->message;
	U8						*buffer;
	U32						length;
	U32						list_address;
	U16						cSGLs;
	
 	FCP_TRACE_ENTRY(FCP_I2O_Get_SGL_Entries);
 	
	cSGLs = pMsg->GetCSgl();

	if (cSGLs)
	{
		p_command->dsd_list_type = 0;
		p_command->dsd_list_base_address = 0;
		// allocate a data segment descriptor list
		p_data_segment = new (tUNCACHED|tPCI) DATA_SEGMENT_DESCRIPTOR[cSGLs];
		// save data segment descriptor list ptr in context to delete later
		p_context->p_dsd = p_data_segment;
		list_address = (U32)FCP_Get_DMA_Address((void *) p_data_segment);
		p_command->dsd_list_address = BYTE_SWAP32(list_address);
		p_command->dsd_list_address_lo = 0;
		for (U16 i = 0; i < cSGLs; i++)
		{
			pMsg->GetSgl(i, (void**) &buffer, (unsigned long *) &length);
			// translate Big Endian to Little Endian format
			buffer = (U8*)FCP_Get_DMA_Address((void *)buffer);  // make physical for DMA
			p_data_segment->Address = BYTE_SWAP32((UNSIGNED)buffer);
			p_data_segment->Length = BYTE_SWAP32(length);
			p_data_segment++;
		}
	    p_command->data_segment_count = BYTE_SWAP16(cSGLs);
	}
} // FCP_I2O_Get_SGL_Entries
	

/*************************************************************************/
// FCP_Delete_SGL
// Delete the SGL allocated in FCP_I2O_Get_SGL_Entries
/*************************************************************************/
void FCP_Delete_SGL(FCP_EVENT_CONTEXT *p_context)
{

	// delete Data Segment Descriptor
	if (p_context->p_dsd)
		delete []p_context->p_dsd;
}


