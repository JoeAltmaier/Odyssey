/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiI2O.cpp
// 
// Description:
// This file implements interfaces to the I2O message.
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiI2O.cpp $
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/
#include "HscsiCommon.h"
#include "HscsiI2O.h"
#include "HscsiMessage.h"
#include "CTIdLun.h"
#include "HscsiMessageFormats.h"
#include "HscsiMemory.h"

#include "Message.h"

/*************************************************************************/
// HSCSI_I2O_Create
// Create HSCSI_I2O object
// pp_memory points to a pointer to memory to be used.
// On return, this pointer is updated.
/*************************************************************************/
STATUS	HSCSI_I2O_Create(PHSCSI_INSTANCE_DATA Id)
{
 	HSCSI_TRACE_ENTRY(HSCSI_I2O_Create);
	
	return NU_SUCCESS;
	
} // HSCSI_I2O_Create

/*************************************************************************/
// HSCSI_I2O_Destroy
/*************************************************************************/
void	HSCSI_I2O_Destroy()
{
 	HSCSI_TRACE_ENTRY(HSCSI_I2O_Destroy);
		
} // HSCSI_I2O_Destroy

/*************************************************************************/
// HSCSI_I2O_Add_Payload
// Add Payload area to an I2O message
/*************************************************************************/
void HSCSI_I2O_Add_Payload(void *p_msg,
	void *p_payload, UNSIGNED payload_size)
{
	Message		*pMsg = (Message *)p_msg;
	
 	HSCSI_TRACE_ENTRY(HSCSI_I2O_Add_Payload);
		
	pMsg->AddPayload(p_payload, payload_size);
	
} // HSCSI_I2O_Add_Payload
	
/*************************************************************************/
// HSCSI_I2O_Add_Reply_Payload
// Add a Reply Payload area to an I2O message
/*************************************************************************/
void HSCSI_I2O_Add_Reply_Payload(void *p_msg,
	void *p_payload, UNSIGNED payload_size,
	STATUS DetailedStatusCode)
{
	Message		*pMsg = (Message *)p_msg;
	
 	HSCSI_TRACE_ENTRY(HSCSI_I2O_Add_Reply_Payload);
		
	pMsg->AddReplyPayload(p_payload, payload_size);
	
	// return the DetailedStatusCode
	pMsg->DetailedStatusCode = DetailedStatusCode;
	
} // HSCSI_I2O_Add_Reply_Payload
	
/*************************************************************************/
// HSCSI_I2O_Add_SGL_Entry
// Add entry to SGL in I2O message
/*************************************************************************/
void HSCSI_I2O_Add_SGL_Entry(void *p_msg,
	void *p_buffer, UNSIGNED buffer_size, UNSIGNED SGL_index)
{
	Message		*pMsg = (Message *) p_msg;
	
 	HSCSI_TRACE_ENTRY(HSCSI_I2O_Add_SGL_Entry);
	
	pMsg->AddSgl(SGL_index, (void *)p_buffer, buffer_size);
	
	HSCSI_PRINT_HEX(TRACE_L8, "\n\rp_buffer = ", (U32) p_buffer);
	HSCSI_PRINT_HEX(TRACE_L8, "  buffer_size = ", (U32) buffer_size);
	
} // HSCSI_I2O_Add_SGL_Entry

/*************************************************************************/
// HSCSI_I2O_Get_Payload
// Get the address of the Payload area from a message
/*************************************************************************/
void *HSCSI_I2O_Get_Payload(void *p_msg)
{
	Message		*pMsg = (Message *)p_msg;
	
 	HSCSI_TRACE_ENTRY(HSCSI_I2O_Get_Payload);
		
	return ((void *)pMsg->GetPPayload());
	
} // HSCSI_I2O_Get_Payload
	
/*************************************************************************/
// HSCSI_I2O_Get_ID_LUN
// Get the address of the ID / LUN struct area from a message
/*************************************************************************/
void *HSCSI_I2O_Get_ID_LUN(void *p_msg)
{
	Message		*pMsg = (Message *)p_msg;
	SCB_PAYLOAD	*pP = (SCB_PAYLOAD *)pMsg->GetPPayload();
	
 	HSCSI_TRACE_ENTRY(HSCSI_I2O_Get_ID_LUN);
	HSCSI_PRINT_HEX(TRACE_L8, "\n\rID/LUN=", (U32) *((U32*)&pP->IdLun));
		
	return ((void *)&pP->IdLun);
	
} // HSCSI_I2O_Get_ID_LUN
	
/*************************************************************************/
// HSCSI_I2O_Get_DetailedStatusCode
// Get the DetailedStatusCode from a message
/*************************************************************************/
STATUS HSCSI_I2O_Get_DetailedStatusCode(void *p_msg)
{
	Message		*pMsg = (Message *)p_msg;
	
 	HSCSI_TRACE_ENTRY(HSCSI_I2O_Get_DetailedStatusCode);
	HSCSI_PRINT_HEX(TRACE_L8, "\n\rDetailedStatusCode=",
				(U32) pMsg->DetailedStatusCode);
		
	return (pMsg->DetailedStatusCode);
	
} // HSCSI_I2O_Get_DetailedStatusCode
	
/*************************************************************************/
// HSCSI_I2O_Get_SGL_Entry
// Fetch the SGL entry from I2O message, insert data at p_data_segment
/*************************************************************************/
void HSCSI_I2O_Get_SGL_Data_Segment(void* p_msg,
	HSCSI_DATA_SEGMENT_DESCRIPTOR *p_data_segment, UNSIGNED SGL_index)
{
	Message		*pMsg = (Message *) p_msg;
	U8			*buffer;
	U32			length;
	
 	HSCSI_TRACE_ENTRY(HSCSI_I2O_Get_SGL_Data_Segment);
 	
	pMsg->GetSgl(SGL_index, (void**) &buffer, (unsigned long *) &length);
	
	HSCSI_PRINT_HEX(TRACE_L8, "\n\rbuffer = ", (U32) buffer);
	HSCSI_PRINT_HEX(TRACE_L8, "  length = ", (U32) length);
	
	// p_data_segment points to a descriptor in an IOCB so we need to
	// translate Big Endian to Little Endian format
	//buffer = (U8*)((UNSIGNED)buffer & ~KSEG1);  // make physical for DMA
	buffer = (U8*)HSCSI_Get_DMA_Address((void *)buffer);  // make physical for DMA
	p_data_segment->Address = BYTE_SWAP32((UNSIGNED)buffer);
	p_data_segment->Length = BYTE_SWAP32(length);

	HSCSI_DUMP_HEX(TRACE_L8, "\n\rdata segement = ", (U8 *) p_data_segment, 8);
	
} // HSCSI_I2O_Get_SGL_Data_Segment
	

