/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpCheck.c
// 
// Description:
// This module keeps track of check information for each LUN for each 
// initiator.  There are cases where the target driver will return a 
// SCSI check condition before the SCSI command is sent to a driver to 
// be served:
//	á A buffer cannot be allocated for the command.
//	á An unrecognized command is received.
//	á The driver is in the quiescent state and must not accept any new 
//		commands.
//	á A timeout occurs when the SCSI I2O message is sent to be served.
// In each of these cases, the host will send a SCSI Check command  back
// to the target driver to find out why the command failed.  The Target 
// driver must remember why the command failed and be able to return 
// appropriate check information.
// 
// Update Log 
// 5/20/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/
#include "FcpCommon.h"
#include "Fcp.h"
#include "FcpCheck.h"
#include "FcpError.h"
#include "FcpString.h"
#include "FcpMemory.h"
#include "OsTypes.h"

/*************************************************************************/
// Forward References
/*************************************************************************/

/*************************************************************************/
// Check globals  TODO: rewrite to use dynamic array
/*************************************************************************/
typedef	U8			(*FCP_P_CHECK_DATA)[FCP_INITIATOR_MAX][FCP_LUNS_MAX];
FCP_P_CHECK_DATA	FCP_p_check_data; // pointer to array 

//*************************************************************************/
// FCP_Check_Create
// Create FCP_Check object
/*************************************************************************/
STATUS FCP_Check_Create(PINSTANCE_DATA Id)
{
	VOID			*p_memory;
	STATUS			 status = NU_SUCCESS;
	UNSIGNED		 check_data_size;
	
 	FCP_TRACE_ENTRY(FCP_Check_Create);
			
	// Calculate the amount of space we will need for check data:
	// one per LUN per initiator
	check_data_size = sizeof(U8) * FCP_INITIATOR_MAX 
		* FCP_LUNS_MAX;
		
	FCP_PRINT_HEX(TRACE_L8,"\n\rcheck_data_size = ", check_data_size);
	
    // Insure UNSIGNED alignment of check_data_size
    check_data_size = ((check_data_size 
    	+ sizeof(UNSIGNED) - 1) / sizeof(UNSIGNED)) * sizeof(UNSIGNED);
    	
	// get memory for the check_data array
	p_memory = FCP_Alloc((tSMALL|tUNCACHED), check_data_size);
	
	// Save pointer to check_data
	FCP_p_check_data = (FCP_P_CHECK_DATA)p_memory;
	
    FCP_PRINT_STRING(TRACE_L8, "\n\rFCP_p_check_data set");

    // Initialize check data
    bzero(p_memory, check_data_size);
	
	// Update p_memory with size of check data
	p_memory = (char*)p_memory + check_data_size;
		
	return status;
	
} // FCP_Check_Create

//*************************************************************************/
// FCP_Check_Destroy
// Destroy FCP_Check object
/*************************************************************************/
void FCP_Check_Destroy()
{
 	FCP_TRACE_ENTRY(FCP_Check_Destroy);
 	
 	FCP_Free((void *)FCP_p_check_data);
			
} // FCP_Check_Destroy

//*************************************************************************/
// FCP_Check_Save
// Save check data for this initiator and this LUN
/*************************************************************************/
void FCP_Check_Save(U16 LUN, U8 initiator, 
	FCP_CHECK_CONDITION check_condition)
{
 	FCP_TRACE_ENTRY(FCP_Check_Save);

	FCP_ASSERT(initiator < FCP_INITIATOR_MAX, "FCP_Check_Save");
	
	*FCP_p_check_data[initiator][LUN] = check_condition;
	
} // FCP_Check_Save


//*************************************************************************/
// FCP_Check_Get
// Return value of check condition for this initiator and this LUN
// Returns zero if no check condition.
/*************************************************************************/
U8 FCP_Check_Get(U16 LUN, U8 initiator)
{
 	FCP_TRACE_ENTRY(FCP_Check_Get);
			
	FCP_ASSERT(initiator < FCP_INITIATOR_MAX, "FCP_Check_Get");
	
	return *FCP_p_check_data[initiator][LUN];
	
} // FCP_Check_Get

//*************************************************************************/
// FCP_Check_Create_Sense
// Create a SCSI sense record given a FCP_CHECK_CONDITION.
/*************************************************************************/
void FCP_Check_Create_Sense(U16 LUN, U8 initiator, 
	REQUEST_SENSE *p_sense_data)
{
  	FCP_TRACE_ENTRY(FCP_Check_Create_Sense);
			
	FCP_ASSERT(initiator < FCP_INITIATOR_MAX, "FCP_Check_Create_Sense");
	
    p_sense_data->ResponseCode			= RESPONSE_CODE;
    p_sense_data->Segment				= 0;
    p_sense_data->SenseKey				= 0;
    p_sense_data->Information[0]		= 0;
    p_sense_data->Information[1]		= 0;
    p_sense_data->Information[2]		= 0;
    p_sense_data->Information[3]		= 0;
    p_sense_data->AdditionalLength		= ADDITIONAL_LENGTH;
    p_sense_data->CmdSpecificInfo[0]	= 0;
    p_sense_data->CmdSpecificInfo[1]	= 0;
    p_sense_data->CmdSpecificInfo[2]	= 0;
    p_sense_data->CmdSpecificInfo[3]	= 0;
    p_sense_data->ASC_ASCQ				= 0;
    p_sense_data->Fru					= 0;
    p_sense_data->SenseKeySpecific[0]	= 0;
    p_sense_data->SenseKeySpecific[1]	= 0;
    p_sense_data->SenseKeySpecific[2]	= 0;
    p_sense_data->AdditionalSense[0]	= 0;
    
    // Set up the sense data depending on the check condition
    switch(*FCP_p_check_data[initiator][LUN])
    {
    	case FCP_CHECK_NO_BUFFER:
    		p_sense_data->SenseKey	= SENSE_HARDWARE_ERROR;
    		p_sense_data->ASC_ASCQ	= ASC_SYSTEM_BUFFER_FULL;
    		break;
    		
    	case FCP_CHECK_UNRECOGNIZED_COMMAND:
    		p_sense_data->SenseKey	= SENSE_ILLEGAL_REQUEST;
    		p_sense_data->ASC_ASCQ	= ASC_INVALID_COMMAND_OPERATION_CODE;
    		break;
    		
    	case FCP_CHECK_I2O:
    		p_sense_data->SenseKey	= SENSE_NOT_READY;
    		p_sense_data->ASC_ASCQ	= ASC_SYSTEM_RESOURCE_FAILURE;
    		break;
    		
    	case FCP_CHECK_DRIVER_QUIESCENT:
    		p_sense_data->SenseKey	= SENSE_NOT_READY;
    		p_sense_data->ASC_ASCQ	= ASC_LOGICAL_UNIT_BECOMING_READY;
    		break;
    		
    	case FCP_CHECK_TIMEOUT:
    		p_sense_data->SenseKey	= SENSE_HARDWARE_ERROR;
    		p_sense_data->ASC_ASCQ	= ASC_LOGICAL_UNIT_TIMEOUT;
    		break;
     		
    	case FCP_CHECK_BAD_LUN:
    		p_sense_data->SenseKey	= SENSE_NOT_READY;
    		p_sense_data->ASC_ASCQ	= ASC_LOGICAL_UNIT_NOT_SUPPORTED;
    		break;
     		
    	default:
    		p_sense_data->SenseKey	= SENSE_HARDWARE_ERROR;
    		p_sense_data->ASC_ASCQ	= ACS_INTERNAL_TARGET_FAILURE;
    		
			FCP_Log_Error(FCP_ERROR_TYPE_WARNING,
				"FCP_Check_Create_Sense", 
				"Invalid check data",
				(UNSIGNED)*FCP_p_check_data[initiator][LUN],
				(UNSIGNED)(LUN<<16) | initiator);
    		
    }
    
    // Reset check condition
    *FCP_p_check_data[initiator][LUN] = FCP_CHECK_NULL;
	
} // FCP_Check_Create_Sense



