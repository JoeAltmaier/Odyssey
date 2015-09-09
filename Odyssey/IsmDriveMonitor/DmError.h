/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DmError.h
// 
// Description:
// This file defines error handling methods for the DriveMonitor. 
// 
// Update Log 
// 10/14/98 Michael G. Panas: Create file
/*************************************************************************/
#if !defined(DMError_H)
#define DMError_H

#include "Nucleus.h"


/*************************************************************************/
// DM_ERROR_TYPE used in DM_Log_Error
/*************************************************************************/
typedef enum 
{
	DM_ERROR_TYPE_FATAL,
	DM_ERROR_TYPE_INFORMATION,
	DM_ERROR_TYPE_WARNING
} DM_ERROR_TYPE;


STATUS	DM_Error_Create(void **pp_memory);
void	DM_Error_Destroy();
void	DM_Error_Stop();

void DM_Log_Error(DM_ERROR_TYPE error_type,
	char* p_module_name, 
	char* p_error_description, 
	STATUS status, 
	UNSIGNED data);


#endif /* DMError_H  */
