/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ErrorLog.c
// 
// Description:
// This file handles errors.
// 
// Update Log 
// 9/23/98 Jim Frandeen: Create file
// 02/26/99 Jim Frandeen: Replace Print_String with printf
/*************************************************************************/
#include "ErrorLog.h"
#include <stdio.h>



/*************************************************************************/
// CT_Log_Error
/*************************************************************************/
void CT_Log_Error(CT_ERROR_TYPE error_type,
	char* p_module_name, 
	char* p_error_description, 
	S32 status, 
	U32 data)
{
	
	printf(EOL "CT_Log_Error in module %s", p_module_name);
	printf(EOL "     %s", p_error_description);
	printf(", status = 0X%X", status);
	printf(", data =  0X%X\n\r", data);
	
	return;
	
} // CT_Log_Error


