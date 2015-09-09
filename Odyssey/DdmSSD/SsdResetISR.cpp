/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdResetISR.cpp
// 
// Description:
// This file implements reset interrupt for the SSD DDM. 
// If a reset occurs while the DDM is running, it is essential that
// the flash file system be closed; otherwise the system structures
// will be inconsistent.
// 
// $Log: /Gemini/Odyssey/DdmSSD/SsdResetISR.cpp $
// 
// 3     5/17/99 2:01p Jfrandeen
// Send Quiesce on reset
// 
// 2     5/13/99 11:40a Cwohlforth
// Edits to support conversion to TRACEF
// 
// 1     5/12/99 9:48a Jfrandeen
// 
// 5/12/99 Jim Frandeen: Create file
/*************************************************************************/
#define	TRACE_INDEX		TRACE_SSD1
#include "Trace_Index.h"
#include "Odyssey_Trace.h"
#include "SsdDdm.h"

/*************************************************************************/
// ISR globals
/*************************************************************************/
// Pointer to SSD_Ddm, used by reset HISR.
SSD_Ddm 	*SSD_Ddm::m_p_ddm;

/*************************************************************************/
// Close_Reset_ISR
/*************************************************************************/
Status SSD_Ddm::Close_Reset_ISR()
{
 	TRACE_ENTRY(Close_Reset_ISR);

	// Delete high level interrupt service routine 
	Status status = NU_Delete_HISR(&m_reset_HISR);
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Close_Reset_ISR", 
			"NU_Delete_HISR failed",
			status,
			0);
		return status;
	}
	
	return status;
	
} // Close_Reset_ISR

/*************************************************************************/
// Open_Reset_ISR
/*************************************************************************/
Status SSD_Ddm::Open_Reset_ISR()
{
 	TRACE_ENTRY(Open_Reset_ISR);

	// Save pointer to our DDM for HISR.
	m_p_ddm = this;
	
    // Insure U32 alignment of stack size. 
    U32 stack_size = ((SSD_RESET_HISR_STACK_SIZE 
    	+ sizeof(U32) - 1) / sizeof(U32)) * sizeof(U32);
    	
	// Create a Quiesce message that will be used for reset.
	m_p_reset_message = new Message(REQ_OS_DDM_QUIESCE);
	
	// m_OK_to_reset will not be set until Enable has completed.
	m_OK_to_reset = 0;

	// Allocate stack for HISR
	void *m_p_HISR_stack = new (tBIG) char[stack_size];
	
	// Register low level interrupt service routine
	void (*old_lisr)(INT); // pointer to old LISR
	Status status = NU_Register_LISR(SSD_RESET_INTERRUPT_VECTOR_NUMBER,
		Reset_ISR_Low, &old_lisr);
		
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Open_Reset_ISR", 
			"NU_Register_LISR failed",
			status,
			0);
		return status;
	}
	
	// Create high level interrupt service routine 
	status = NU_Create_HISR(&m_reset_HISR, 
		SSD_RESET_HISR_NAME,	// name assigned to HISR
		Reset_ISR_High, // function entry point for HISR
		SSD_RESET_HISR_PRIORITY,
		m_p_HISR_stack,
		stack_size);
		
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Open_Reset_ISR", 
			"NU_Create_HISR failed",
			status,
			0);
		return status;
	}
		
	return status;
	
} // Open_Reset_ISR

/*************************************************************************/
// Reset_ISR_High
// High Level Interrupt Service Routine
// gets activated by Reset_ISR_Low.
/*************************************************************************/
void SSD_Ddm::Reset_ISR_High()
{
	// Is it OK to reset?  Not unless Enable has been started.
	if (m_p_ddm->m_OK_to_reset == 0)
		return;
		
	// Is a quiesce already in progress?
	if (m_p_ddm->m_p_quiesce_message)
	
		// There is already a quiesce in progress.
		return;
		
	// It's not OK to reset again while we are resetting.
	m_p_ddm->m_OK_to_reset = 0;
	
	// Send a Quiesce message to begin the quiesce process.
    m_p_ddm->Send(m_p_ddm->GetDid(), m_p_ddm->m_p_reset_message);

} // Reset_ISR_High

/*************************************************************************/
// Low Level Interrupt Service Routine
// Come here when reset button gets pushed.
/*************************************************************************/
void SSD_Ddm::Reset_ISR_Low(INT vector_number)
{
	// Acknowledge the interrupt.
	SSD_RESET_ACK_REGISTER = 1;
	
	NU_Activate_HISR(&m_p_ddm->m_reset_HISR);
			
} // Reset_ISR_Low


