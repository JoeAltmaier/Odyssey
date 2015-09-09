/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpResponseFIFO.c
// 
// Description:
// This file implements interfaces to the ISP response FIFO.
// 
// Update Log 
// 5/18/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/

#include "FcpCommon.h"
#include "FcpData.h"
#include "FcpResponseFIFO.h"
#include "FcpString.h"
#include "FcpMemory.h"
#include "OsTypes.h"

/*************************************************************************/
// Forward References
/*************************************************************************/
void	FCP_Response_FIFO_Increment_Index();

/*************************************************************************/
// Response FIFO globals
/*************************************************************************/

/*************************************************************************/
// FCP_Response_FIFO_Create
// Create FCP_Response_FIFO object.
// pp_memory points to a pointer to memory to be used.
// On return, this pointer is updated.
/*************************************************************************/
STATUS FCP_Response_FIFO_Create(PINSTANCE_DATA Id)
{
	VOID			*p_memory;
	STATUS			 status = NU_SUCCESS;
	
 	FCP_TRACE_ENTRY(FCP_Response_FIFO_Create);
			
	// Allocate IOCB_Response_FIFO
	// This FIFO of IOCBs is used receive requests and responses from the ISP.  
	// The address of the IOCB_Response_FIFO is passed to the ISP. 
	
	// get memory for the IOCB Response FIFO
	p_memory = FCP_Alloc((tSMALL|tUNCACHED),
					(Id->FCP_config.ISP_FIFO_response_size + 1) *
					IOCB_SIZE);

    // Insure 64 byte boundary alignment of FIFO. 
    p_memory = (VOID*)((char*)p_memory + IOCB_SIZE - 1);   	
    p_memory = (VOID*)(((UNSIGNED)p_memory >> IOCB_SHIFT) << IOCB_SHIFT);
	
    // Save pointer to response_FIFO
	Id->FCP_p_IOCB_response_FIFO = (IOCB_STATUS_TYPE0*)p_memory;
	
	// Initialize FCP_response_FIFO_index
	Id->FCP_response_FIFO_index = 0;
	
	return status;
	
} // FCP_Response_FIFO_Create


/*************************************************************************/
// FCP_Response_FIFO_Destroy
// Destroy FCP_Response_FIFO object.
/*************************************************************************/
void FCP_Response_FIFO_Destroy()
{
	    
 	FCP_TRACE_ENTRY(FCP_Response_FIFO_Destroy);
			
} // FCP_Response_FIFO_Destroy

/*************************************************************************/
// FCP_Response_FIFO_Increment_Index.
// Increment the index when we have processed this IOCB.
// This FIFO index is only incremented from the HISR, so there is 
// no need to have a semaphore around it.
/*************************************************************************/
void FCP_Response_FIFO_Increment_Index(PINSTANCE_DATA Id)
{
 	
 	FCP_TRACE_ENTRY(FCP_Response_FIFO_Increment_Index);
			
    Id->FCP_response_FIFO_index++;
    Id->FCP_response_FIFO_index %= Id->FCP_config.ISP_FIFO_response_size;
    
    // Update response queue in pointer in mailbox 5.
    Write_ISP(Id, ISP_MAILBOX_5, Id->FCP_response_FIFO_index);
    
} // FCP_Response_FIFO_Increment_Index


