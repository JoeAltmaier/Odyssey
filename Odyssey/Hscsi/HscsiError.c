/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiError.c
// 
// Description:
// This file handles errors.
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiError.c $ 
// 
// 1     9/14/99 7:23p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/
#include "Hscsi.h"
#include "HscsiCommon.h"
#include "HscsiError.h"
#include "HscsiISP.h"

/*************************************************************************/
// Forward References
/*************************************************************************/
void HSCSI_Error_Stop();

/*************************************************************************/
// HSCSI_Error_Create
// Create HSCSI_Error object
/*************************************************************************/
STATUS	HSCSI_Error_Create(PHSCSI_INSTANCE_DATA Id)
{
	// TODO
	
	return NU_SUCCESS;
	
} // HSCSI_Error_Create

/*************************************************************************/
// HSCSI_Error_Destroy
/*************************************************************************/
void	HSCSI_Error_Destroy()
{
	// TODO
	
} // HSCSI_Error_Destroy

/*************************************************************************/
// HSCSI_Error_Stop
// For debugging, cause exception to stop execution.
/*************************************************************************/
void HSCSI_Error_Stop()
{
	UNSIGNED	*p_illegal_address;
	
	// turn off QL1040 interrupts
	//Write_ISP1040(ISP_PCI_INT_CONTROL, 0x0);	
	
	// Cause exception.
    p_illegal_address = (UNSIGNED * ) 0x1;
    *p_illegal_address = 0xFFFF0000;
    
} // HSCSI_Error_Stop

/*************************************************************************/
// HSCSI_Log_Error
/*************************************************************************/
void HSCSI_Log_Error(HSCSI_ERROR_TYPE error_type,
	char* p_module_name, 
	char* p_error_description, 
	STATUS status, 
	UNSIGNED data)
{

#if defined(HSCSI_DEBUG) && defined(_DEBUG)
// none of this will print if we are not debugging
	printf("\n\rHSCSI_Log_Error in module ");
	printf(p_module_name);
	printf("\n\r     ");
	printf(p_error_description);
	
	printf(", status = %d", status);
    
	printf(", data =  %08x", data);
    
	printf("\n\r");
#endif

	if (error_type == HSCSI_ERROR_TYPE_FATAL)
		HSCSI_Error_Stop();
	
	return;
	
} // HSCSI_Log_Error

