/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SmError.c
// 
// Description:
// This file handles error logging.
// 
// Update Log
//	$Log: /Gemini/Odyssey/IsmScsiMonitor/SmError.cpp $ 
// 
// 1     10/11/99 5:35p Cchan
// HSCSI (QL1040B) version of the Drive Monitor
//
/*************************************************************************/

#include <stdio.h>
#include "SmCommon.h"


/*************************************************************************/
// Forward References
/*************************************************************************/
void SM_Error_Stop();

/*************************************************************************/
// SM_Error_Create
// Create SM_Error object
/*************************************************************************/
STATUS	SM_Error_Create(void **pp_memory)
{
	// TODO
	
	return NU_SUCCESS;
	
} // SM_Error_Create

/*************************************************************************/
// SM_Error_Destroy
/*************************************************************************/
void	SM_Error_Destroy()
{
	// TODO
	
} // SM_Error_Destroy

/*************************************************************************/
// SM_Error_Stop
// For debugging, cause exception to stop execution.
/*************************************************************************/
void SM_Error_Stop()
{
	UNSIGNED	*p_illegal_address;
	
	// turn off QLogic chip interrupts
	//Write_ISP(ISP_PCI_INT_CONTROL, 0x0);	
	
	// Cause exception.
    p_illegal_address = (UNSIGNED * ) 0x1;
    *p_illegal_address = 0xFFFF0000;
    
} // SM_Error_Stop

/*************************************************************************/
// SM_Log_Error
/*************************************************************************/
void SM_Log_Error(SM_ERROR_TYPE error_type,
	char* p_module_name, 
	char* p_error_description, 
	STATUS status, 
	UNSIGNED data)
{

#if defined(SM_DEBUG) && defined(_DEBUG)
// none of this will print if we are not debugging
	printf("\n\rSM_Log_Error in module ");
	printf(p_module_name);
	printf("\n\r     ");
	printf(p_error_description);
	
	printf(", status = %d", status);
    
	printf(", data =  %08x", data);
    
	printf("\n\r");
#endif

	if (error_type == SM_ERROR_TYPE_FATAL)
		SM_Error_Stop();
	
	
	// send this error to the system error logger
	// TODO:
	
	return;
	
} // SM_Log_Error

