/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpError.h
// 
// Description:
// This file defines error handling methods. 
// 
// Update Log 
// 5/12/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 9/2/98 Michael G. Panas: add C++ stuff
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/
#if !defined(FCPError_H)
#define FCPError_H

#include "Nucleus.h"
#include "FCPData.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

/*************************************************************************/
// FCP_ERROR codes begin with FCP_ERROR_BASE to distinguish them
// from Nucleus error codes defined in Nucleus.h.
/*************************************************************************/
#define FCP_ERROR_BASE 1000
typedef enum
{
	FCP_ERROR_INVALID_CONFIG_DATA 		= FCP_ERROR_BASE + 1,
	FCP_ERROR_RISC_CHECKSUM		 		= FCP_ERROR_BASE + 2,
	FCP_ERROR_LOAD_RISC_RAM		 		= FCP_ERROR_BASE + 3,
	FCP_ERROR_EXECUTE_FIRMWARE	 		= FCP_ERROR_BASE + 4,
	FCP_ERROR_RISC_NOT_READY	 		= FCP_ERROR_BASE + 5,
	FCP_ERROR_INIT_CONTROL_BLOCK		= FCP_ERROR_BASE + 6,
	FCP_ERROR_MAILBOX_TIMEOUT			= FCP_ERROR_BASE + 7,
	FCP_ERROR_DISABLED_LUN				= FCP_ERROR_BASE + 8,
	FCP_ERROR_RESOURCE_OVERFLOW			= FCP_ERROR_BASE + 9,
	FCP_ERROR_DEVICE_RESET				= FCP_ERROR_BASE + 10,
	FCP_ERROR_INVALID_ATIO_STATUS		= FCP_ERROR_BASE + 11,
	FCP_ERROR_INVALID_ATIO_OPERATION	= FCP_ERROR_BASE + 12,
	FCP_ERROR_BUFFER_SIZE_TOO_BIG		= FCP_ERROR_BASE + 13,
	FCP_ERROR_INVALID_RESPONSE_IOCB		= FCP_ERROR_BASE + 14,
	FCP_ERROR_CTIO_FAILED				= FCP_ERROR_BASE + 15,
	FCP_ERROR_INVALID_CONFIG_VERSION	= FCP_ERROR_BASE + 16,
	FCP_ERROR_TEST_RISC_RAM				= FCP_ERROR_BASE + 17,
	FCP_ERROR_TEST_MAILBOX				= FCP_ERROR_BASE + 18,
	FCP_ERROR_GET_INIT_CONTROL_BLOCK	= FCP_ERROR_BASE + 19,
	FCP_ERROR_COMMAND_TIMEOUT			= FCP_ERROR_BASE + 20,
	FCP_ERROR_ENABLE_LUN				= FCP_ERROR_BASE + 21,
	FCP_ERROR_INTERRUPT_SETUP_FAILED	= FCP_ERROR_BASE + 22,
	FCP_ERROR_EVENT_SETUP_FAILED		= FCP_ERROR_BASE + 23,
	FCP_ERROR_MAILBOX_SETUP_FAILED		= FCP_ERROR_BASE + 24
} FCP_ERROR;

/*************************************************************************/
// FCP_ERROR_TYPE used in FCP_LoG_Error
/*************************************************************************/
typedef enum 
{
	FCP_ERROR_TYPE_FATAL,
	FCP_ERROR_TYPE_INFORMATION,
	FCP_ERROR_TYPE_WARNING
} FCP_ERROR_TYPE;


STATUS	FCP_Error_Create(PINSTANCE_DATA Id);
void	FCP_Error_Destroy();
void	FCP_Error_Stop();

void FCP_Log_Error(FCP_ERROR_TYPE error_type,
	char* p_module_name, 
	char* p_error_description, 
	STATUS status, 
	UNSIGNED data);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* FCPError_H  */
