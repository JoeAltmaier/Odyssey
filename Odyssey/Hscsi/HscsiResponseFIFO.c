/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiResponseFIFO.c
// 
// Description:
// This file implements interfaces to the ISP response FIFO.
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiResponseFIFO.c $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#include "HscsiCommon.h"
#include "HscsiData.h"
#include "HscsiResponseFIFO.h"
#include "HscsiString.h"
#include "HscsiMemory.h"
#include "OsTypes.h"

/*************************************************************************/
// Forward References
/*************************************************************************/
void	HSCSI_Response_FIFO_Increment_Index();

/*************************************************************************/
// Response FIFO globals
/*************************************************************************/

/*************************************************************************/
// HSCSI_Response_FIFO_Create
// Create HSCSI_Response_FIFO object.
// pp_memory points to a pointer to memory to be used.
// On return, this pointer is updated.
/*************************************************************************/
STATUS HSCSI_Response_FIFO_Create(PHSCSI_INSTANCE_DATA Id)
{
	VOID			*p_memory;
	STATUS			 status = NU_SUCCESS;
	
 	HSCSI_TRACE_ENTRY(HSCSI_Response_FIFO_Create);
			
	// Allocate IOCB_Response_FIFO
	// This FIFO of IOCBs is used receive requests and responses from the ISP.  
	// The address of the IOCB_Response_FIFO is passed to the ISP. 
	
	// get memory for the IOCB Response FIFO
	p_memory = HSCSI_Alloc((tSMALL|tUNCACHED),
					(Id->HSCSI_config.ISP_FIFO_response_size + 1) *
					IOCB_SIZE);

    // Insure 64 byte boundary alignment of FIFO. 
    p_memory = (VOID*)((char*)p_memory + IOCB_SIZE - 1);   	
    p_memory = (VOID*)(((UNSIGNED)p_memory >> IOCB_SHIFT) << IOCB_SHIFT);
	
    // Save pointer to response_FIFO
	Id->HSCSI_p_IOCB_response_FIFO = (IOCB_STATUS_ENTRY*)p_memory;
	
	// Initialize HSCSI_response_FIFO_index
	Id->HSCSI_response_FIFO_index = 0;
	
	return status;
	
} // HSCSI_Response_FIFO_Create


/*************************************************************************/
// HSCSI_Response_FIFO_Destroy
// Destroy HSCSI_Response_FIFO object.
/*************************************************************************/
void HSCSI_Response_FIFO_Destroy()
{
	    
 	HSCSI_TRACE_ENTRY(HSCSI_Response_FIFO_Destroy);
			
} // HSCSI_Response_FIFO_Destroy

/*************************************************************************/
// HSCSI_Response_FIFO_Increment_Index.
// Increment the index when we have processed this IOCB.
// This FIFO index is only incremented from the HISR, so there is 
// no need to have a semaphore around it.
/*************************************************************************/
void HSCSI_Response_FIFO_Increment_Index(PHSCSI_INSTANCE_DATA Id)
{
 	
 	HSCSI_TRACE_ENTRY(HSCSI_Response_FIFO_Increment_Index);
			
    Id->HSCSI_response_FIFO_index++;
    Id->HSCSI_response_FIFO_index %= Id->HSCSI_config.ISP_FIFO_response_size;
    
    // Update response queue in pointer in mailbox 5.
    Write_ISP1040(Id, HSCSI_MAILBOX_5, Id->HSCSI_response_FIFO_index);
    
} // HSCSI_Response_FIFO_Increment_Index


