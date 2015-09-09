//******************************************************************************
// FIlE:		Raid2SsapiErrorConvertor.cpp
//
// PURPOSE:		Converts Raid Master error codes to the SSAPI exception codes
//******************************************************************************


#include "Raid2SsapiErrorConvertor.h"
#include "RaidDefs.h"
#include "RmstrErrors.h"
#include "PmstrErrors.h"

//******************************************************************************
// GetSsapiException:
//
// PURPOSE:		Performs a lookup of exception code by raid error code
//******************************************************************************

U32 
Raid2SsapiErrorConvertor::GetSsapiException( U32 raidErrorCode ){
	
	U32		exception;

	switch( raidErrorCode ){

		case RMSTR_ERR_UTIL_NOT_SUPPORTED:
			exception = CTS_SSAPI_INVALIDPARM_EXCEPTION_UTIL_NOT_SUPPORTED;
			break;

		case RMSTR_ERR_UTIL_ALREADY_RUNNING:
			exception = CTS_SSAPI_INVALIDPARM_EXCEPTION_UTIL_ALREADY_RUNNING;
			break;

		case RMSTR_ERR_ARRAY_STATE_OFFLINE:
			exception = CTS_SSAPI_INVALIDPARM_EXCEPTION_ARRAY_OFFLINE;
			break;

		case RMSTR_ERR_ARRAY_STATE_NOT_CRITICAL:
			exception = CTS_SSAPI_INVALIDPARM_EXCEPTION_ARRAY_NOT_CRITICAL;
			break;

		case RMSTR_ERR_ARRAY_STATE_CRITICAL:
			exception = CTS_SSAPI_INVALIDPARM_EXCEPTION_ARRAY_CRITICAL;
			break;

		case RMSTR_ERR_UTIL_NOT_RUNNING:
			exception = CTS_SSAPI_INVALIDPARM_EXCEPTION_UTIL_NOT_RUNNING;
			break;

		case RMSTR_ERR_INVALID_PRIORITY_LEVEL:
			exception = CTS_SSAPI_INVALIDPARM_EXCEPTION_WRONG_PRIORITY;
			break;

		case RMSTR_ERR_INSUFFICIENT_MEMBER_CAPACITY:
			exception = CTS_SSAPI_INVALIDPARM_EXCEPTION_INSUFF_MEMBER_SIZE;
			break;

		case RMSTR_ERR_INSUFFICIENT_SPARE_CAPACITY:
			exception = CTS_SSAPI_INVALIDPARM_EXCEPTION_INSUFF_SPARE_SIZE;
			break;

		case RMSTR_ERR_STORAGE_ELEMENT_FAILED:
			exception = CTS_SSAPI_INVALIDPARM_EXCEPTION_STORAGE_ELEMENT_FAILED;
			break;

		case RMSTR_ERR_INVALID_COMMAND:
			exception = CTS_SSAPI_INVALIDPARM_EXCEPTION_INV_CMD;
			break;

		case RMSTR_ERR_STORAGE_ELEMENT_IN_USE:
			exception = CTS_SSAPI_INVALIDPARM_EXCEPTION_STORAGE_ELEMENT_IN_USE;
			break;

		case RMSTR_ERR_NAME_ALREADY_EXISTS:
			exception = CTS_SSAPI_INVALIDPARM_EXCEPTION_NAME_EXISTS;
			break;

		case RMSTR_ERR_UTIL_RUNNING:
			exception = CTS_SSAPI_INVALIDPARM_UTIL_RUNNING;
			break;

		case RMSTR_ERR_PARTITION_EXISTS:
			exception = CTS_SSAPI_INVALIDPARM_PARTITION_EXISTS;
			break;

		case RMSTR_ERR_MAX_SPARES_ALREADY_CREATED:
			exception = CTS_SSAPI_INVALIDPARM_MAX_SPARES_REACHED;
			break;

		case RMSTR_ERR_SPARE_DOES_NOT_EXIST:
			exception = CTS_SSAPI_INVALIDPARM_SPARE_DOES_NOT_EXIST;
			break;


		case RMSTR_ERR_MAX_ARRAY_MEMBERS:
			exception = CTS_SSAPI_EXCEPTION_MAX_ARRAY_MEMBERS_REACHED;
			break;

		case RMSTR_ERR_MEMBER_ALREADY_DOWN:
			exception = CTS_SSAPI_EXCEPTION_MEMBER_ALREADY_DOWN;
			break;

		case RMSTR_ERR_UTIL_ABORT_NOT_ALLOWED:
			exception = CTS_SSAPI_EXCEPTION_CANT_ABORT_UTILITY;
			break;

		case RMSTR_ERR_ARRAY_OFFLINE:
			exception = CTS_SSAPI_EXCEPTION_ARRAY_OFFLINE;
			break;

		case RMSTR_ERR_ARRAY_CRITICAL:
			exception = CTS_SSAPI_EXCEPTION_ARRAY_CRITICAL;
			break;

		case RMSTR_ERR_INVALID_RAID_LEVEL:
			exception = CTS_SSAPI_EXCEPTION_INVALID_RAID_LEVEL;
			break;

		case RMSTR_ERR_INVALID_DATA_BLOCK_SIZE:
			exception = CTS_SSAPI_RAID_MSTR_INVALID_DATA_BLOCK;
			break;

		case RMSTR_ERR_INVALID_PARITY_BLOCK_SIZE:
			exception = CTS_SSAPI_RAID_MSTR_INVALID_PARITY_BLOCK;
			break;

		default:
			exception = CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED;
			break;
	}

	return exception;
}


U32 
Raid2SsapiErrorConvertor::GetSsapiPartitionException( U32 partitionErrorCode ){

	U32			exception;

	switch( partitionErrorCode ){

		case PMSTR_ERR_PMSTR_NOT_INITIALIZED:
			exception = CTS_SSAPI_EXCEPTION_PART_MASTER_NOT_INITED;
			break;

		case PMSTR_ERR_STORAGE_ELEMENT_USED:
			exception = CTS_SSAPI_INVALIDPARM_EXCEPTION_STORAGE_ELEMENT_IN_USE;
			break;

		case PMSTR_ERR_INVALID_PARTITION_SIZE:
			exception = CTS_SSAPI_EXCEPTION_INVALID_PARTITION_SIZE;
			break;

		case PMSTR_ERR_INVALID_COMMAND:
			exception = CTS_SSAPI_INVALIDPARM_EXCEPTION_INV_CMD;
			break;
			default:
			exception = CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED;
			break;
	}

	return exception;
}
