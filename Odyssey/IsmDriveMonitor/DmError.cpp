/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DmError.c
// 
// Description:
// This file handles error logging.
// 
// Update Log 
// 10/14/98 Michael G. Panas: Create file
/*************************************************************************/

#include <stdio.h>
#include "DmCommon.h"


/*************************************************************************/
// Forward References
/*************************************************************************/
void DM_Error_Stop();

/*************************************************************************/
// FCP_Error_Create
// Create FCP_Error object
/*************************************************************************/
STATUS	DM_Error_Create(void **pp_memory)
{
	// TODO
	
	return NU_SUCCESS;
	
} // DM_Error_Create

/*************************************************************************/
// DM_Error_Destroy
/*************************************************************************/
void	DM_Error_Destroy()
{
	// TODO
	
} // DM_Error_Destroy

/*************************************************************************/
// DM_Error_Stop
// For debugging, cause exception to stop execution.
/*************************************************************************/
void DM_Error_Stop()
{
	UNSIGNED	*p_illegal_address;
	
	// turn off QL2100 interrupts
	//Write_ISP(ISP_PCI_INT_CONTROL, 0x0);	
	
	// Cause exception.
    p_illegal_address = (UNSIGNED * ) 0x1;
    *p_illegal_address = 0xFFFF0000;
    
} // DM_Error_Stop

/*************************************************************************/
// DM_Log_Error
/*************************************************************************/
void DM_Log_Error(DM_ERROR_TYPE error_type,
	char* p_module_name, 
	char* p_error_description, 
	STATUS status, 
	UNSIGNED data)
{

#if defined(DM_DEBUG) && defined(_DEBUG)
// none of this will print if we are not debugging
	printf("\n\rDM_Log_Error in module ");
	printf(p_module_name);
	printf("\n\r     ");
	printf(p_error_description);
	
	printf(", status = %d", status);
    
	printf(", data =  %08x", data);
    
	printf("\n\r");
#endif

	if (error_type == DM_ERROR_TYPE_FATAL)
		DM_Error_Stop();
	
	
	// send this error to the system error logger
	// TODO:
	
	return;
	
} // DM_Log_Error

