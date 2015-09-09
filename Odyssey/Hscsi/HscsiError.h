/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiError.h
// 
// Description:
// This file defines error handling methods. 
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiError.h $ 
// 
// 2     10/18/99 7:02p Cchan
// Added entry for HSCSI_ERROR_SET_SCSI (see HscsiISP.c)
// 
// 1     9/14/99 7:23p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/
#if !defined(HSCSIError_H)
#define HSCSIError_H

#include "Nucleus.h"
#include "HSCSIData.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

/*************************************************************************/
// HSCSI_ERROR codes begin with HSCSI_ERROR_BASE to distinguish them
// from Nucleus error codes defined in Nucleus.h.
/*************************************************************************/
#define HSCSI_ERROR_BASE 1000
typedef enum
{
	HSCSI_ERROR_INVALID_CONFIG_DATA 	= HSCSI_ERROR_BASE + 1,
	HSCSI_ERROR_RISC_CHECKSUM		 	= HSCSI_ERROR_BASE + 2,
	HSCSI_ERROR_LOAD_RISC_RAM		 	= HSCSI_ERROR_BASE + 3,
	HSCSI_ERROR_EXECUTE_FIRMWARE	 	= HSCSI_ERROR_BASE + 4,
	HSCSI_ERROR_RISC_NOT_READY	 		= HSCSI_ERROR_BASE + 5,
	HSCSI_ERROR_INIT_CONTROL_BLOCK		= HSCSI_ERROR_BASE + 6,
	HSCSI_ERROR_MAILBOX_TIMEOUT			= HSCSI_ERROR_BASE + 7,
	HSCSI_ERROR_DISABLED_LUN			= HSCSI_ERROR_BASE + 8,
	HSCSI_ERROR_RESOURCE_OVERFLOW		= HSCSI_ERROR_BASE + 9,
	HSCSI_ERROR_DEVICE_RESET			= HSCSI_ERROR_BASE + 10,
	HSCSI_ERROR_INVALID_ATIO_STATUS		= HSCSI_ERROR_BASE + 11,
	HSCSI_ERROR_INVALID_ATIO_OPERATION	= HSCSI_ERROR_BASE + 12,
	HSCSI_ERROR_BUFFER_SIZE_TOO_BIG		= HSCSI_ERROR_BASE + 13,
	HSCSI_ERROR_INVALID_RESPONSE_IOCB	= HSCSI_ERROR_BASE + 14,
	HSCSI_ERROR_CTIO_FAILED				= HSCSI_ERROR_BASE + 15,
	HSCSI_ERROR_INVALID_CONFIG_VERSION	= HSCSI_ERROR_BASE + 16,
	HSCSI_ERROR_TEST_RISC_RAM			= HSCSI_ERROR_BASE + 17,
	HSCSI_ERROR_TEST_MAILBOX			= HSCSI_ERROR_BASE + 18,
	HSCSI_ERROR_GET_INIT_CONTROL_BLOCK	= HSCSI_ERROR_BASE + 19,
	HSCSI_ERROR_COMMAND_TIMEOUT			= HSCSI_ERROR_BASE + 20,
	HSCSI_ERROR_ENABLE_LUN				= HSCSI_ERROR_BASE + 21,
	HSCSI_ERROR_INTERRUPT_SETUP_FAILED	= HSCSI_ERROR_BASE + 22,
	HSCSI_ERROR_EVENT_SETUP_FAILED		= HSCSI_ERROR_BASE + 23,
	HSCSI_ERROR_MAILBOX_SETUP_FAILED	= HSCSI_ERROR_BASE + 24,
	HSCSI_ERROR_INIT_REQUEST_QUEUE		= HSCSI_ERROR_BASE + 25,
	HSCSI_ERROR_INIT_RESPONSE_QUEUE		= HSCSI_ERROR_BASE + 26,
	HSCSI_ERROR_BUS_RESET				= HSCSI_ERROR_BASE + 27,
	HSCSI_ERROR_SET_SCSI				= HSCSI_ERROR_BASE + 28
} HSCSI_ERROR;

/*************************************************************************/
// HSCSI_ERROR_TYPE used in HSCSI_LoG_Error
/*************************************************************************/
typedef enum 
{
	HSCSI_ERROR_TYPE_FATAL,
	HSCSI_ERROR_TYPE_INFORMATION,
	HSCSI_ERROR_TYPE_WARNING
} HSCSI_ERROR_TYPE;


STATUS	HSCSI_Error_Create(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_Error_Destroy();
void	HSCSI_Error_Stop();

void HSCSI_Log_Error(HSCSI_ERROR_TYPE error_type,
	char* p_module_name, 
	char* p_error_description, 
	STATUS status, 
	UNSIGNED data);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* HSCSIError_H  */
