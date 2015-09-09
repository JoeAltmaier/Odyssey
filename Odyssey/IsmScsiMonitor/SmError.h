/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SmError.h
// 
// Description:
// This file defines error handling methods for the ScsiMonitor. 
// 
// Update Log
//	$Log: /Gemini/Odyssey/IsmScsiMonitor/SmError.h $ 
// 
// 1     10/11/99 5:35p Cchan
// HSCSI (QL1040B) version of the Drive Monitor
//
/*************************************************************************/
#if !defined(SMError_H)
#define SMError_H

#include "Nucleus.h"


/*************************************************************************/
// SM_ERROR_TYPE used in SM_Log_Error
/*************************************************************************/
typedef enum 
{
	SM_ERROR_TYPE_FATAL,
	SM_ERROR_TYPE_INFORMATION,
	SM_ERROR_TYPE_WARNING
} SM_ERROR_TYPE;


STATUS	SM_Error_Create(void **pp_memory);
void	SM_Error_Destroy();
void	SM_Error_Stop();

void SM_Log_Error(SM_ERROR_TYPE error_type,
	char* p_module_name, 
	char* p_error_description, 
	STATUS status, 
	UNSIGNED data);


#endif /* SMError_H  */
