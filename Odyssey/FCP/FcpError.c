/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpError.c
// 
// Description:
// This file handles errors.
// 
// Update Log 
// 5/12/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/
#include "Fcp.h"
#include "FcpCommon.h"
#include "FcpError.h"
#include "FcpISP.h"

/*************************************************************************/
// Forward References
/*************************************************************************/
void FCP_Error_Stop();

/*************************************************************************/
// FCP_Error_Create
// Create FCP_Error object
/*************************************************************************/
STATUS	FCP_Error_Create(PINSTANCE_DATA Id)
{
	// TODO
	
	return NU_SUCCESS;
	
} // FCP_Error_Create

/*************************************************************************/
// FCP_Error_Destroy
/*************************************************************************/
void	FCP_Error_Destroy()
{
	// TODO
	
} // FCP_Error_Destroy

/*************************************************************************/
// FCP_Error_Stop
// For debugging, cause exception to stop execution.
/*************************************************************************/
void FCP_Error_Stop()
{
	extern void write_to_one();
		
	write_to_one();
	
} // FCP_Error_Stop

/*************************************************************************/
// FCP_Log_Error
/*************************************************************************/
void FCP_Log_Error(FCP_ERROR_TYPE error_type,
	char* p_module_name, 
	char* p_error_description, 
	STATUS status, 
	UNSIGNED data)
{

#if defined(FCP_DEBUG) && defined(_DEBUG)
// none of this will print if we are not debugging
	printf("\n\rFCP_Log_Error in module ");
	printf(p_module_name);
	printf("\n\r     ");
	printf(p_error_description);
	
	printf(", status = %d", status);
    
	printf(", data =  %08x", data);
    
	printf("\n\r");
#endif

	if (error_type == FCP_ERROR_TYPE_FATAL)
		FCP_Error_Stop();
	
	return;
	
} // FCP_Log_Error

