/* OsStatus.h -- Simple types used by OOS
 *
 * Copyright (C) ConvergeNet Technologies, 1998 
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Description:
 *		Status (error) codes returned by OOS
 *
 * Revision History:
 *     10-05-98  Tom Nelson: Created
 *		4-01-99  Tom Nelson: Delete OS_REPLY_STATUS... codes
 *
**/


#ifndef OsStatus_H
#define OsStatus_H

#define OS_STATUS_BASE		1001
#define OS_STATUS_END		1999


enum { 
	HEAPTRASHEDerc = OS_STATUS_BASE,
	UNEXPECTEDerc,
	DDMINITFAILerc,
	DDMNOPARENTerc,
	DMABADADDRESSerc
};	
// Until all I2O references are removed, these have the same values
// as their I2O_ counterparts.

#define    OS_DETAIL_STATUS_SUCCESS                        0x0000
#define    OS_DETAIL_STATUS_BAD_KEY                        0x0002
#define    OS_DETAIL_STATUS_TCL_ERROR                      0x0003
#define    OS_DETAIL_STATUS_REPLY_BUFFER_FULL              0x0004
#define    OS_DETAIL_STATUS_NO_SUCH_PAGE                   0x0005
#define    OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT     0x0006
#define    OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_HARD     0x0007
#define    OS_DETAIL_STATUS_CHAIN_BUFFER_TOO_LARGE         0x0009
#define    OS_DETAIL_STATUS_UNSUPPORTED_FUNCTION           0x000A
#define    OS_DETAIL_STATUS_DEVICE_LOCKED                  0x000B
#define    OS_DETAIL_STATUS_DEVICE_RESET                   0x000C
#define    OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION         0x000D
#define    OS_DETAIL_STATUS_INVALID_INITIATOR_ADDRESS      0x000E
#define    OS_DETAIL_STATUS_INVALID_MESSAGE_FLAGS          0x000F
#define    OS_DETAIL_STATUS_INVALID_OFFSET                 0x0010
#define    OS_DETAIL_STATUS_INVALID_PARAMETER              0x0011
#define    OS_DETAIL_STATUS_INVALID_REQUEST                0x0012
#define    OS_DETAIL_STATUS_INVALID_TARGET_ADDRESS         0x0013
#define    OS_DETAIL_STATUS_MESSAGE_TOO_LARGE              0x0014
#define    OS_DETAIL_STATUS_MESSAGE_TOO_SMALL              0x0015
#define    OS_DETAIL_STATUS_MISSING_PARAMETER              0x0016
#define    OS_DETAIL_STATUS_TIMEOUT                        0x0017
#define    OS_DETAIL_STATUS_UNKNOWN_ERROR                  0x0018
#define    OS_DETAIL_STATUS_UNKNOWN_FUNCTION               0x0019
#define    OS_DETAIL_STATUS_UNSUPPORTED_VERSION            0x001A
#define    OS_DEATIL_STATUS_DEVICE_BUSY                    0x001B
#define    OS_DETAIL_STATUS_DEVICE_NOT_AVAILABLE           0x001C

#define    OS_FAILURE_CODE_TRANSPORT_SERVICE_SUSPENDED    	0x81
#define    OS_FAILURE_CODE_TRANSPORT_SERVICE_TERMINATED   	0x82
#define    OS_FAILURE_CODE_TRANSPORT_CONGESTION           	0x83
#define    OS_FAILURE_CODE_TRANSPORT_FAIL                 	0x84
#define    OS_FAILURE_CODE_TRANSPORT_STATE_ERROR          	0x85
#define    OS_FAILURE_CODE_TRANSPORT_TIME_OUT             	0x86
#define    OS_FAILURE_CODE_TRANSPORT_ROUTING_FAILURE      	0x87
#define    OS_FAILURE_CODE_TRANSPORT_INVALID_VERSION      	0x88
#define    OS_FAILURE_CODE_TRANSPORT_INVALID_OFFSET       	0x89
#define    OS_FAILURE_CODE_TRANSPORT_INVALID_MSG_FLAGS    	0x8A
#define    OS_FAILURE_CODE_TRANSPORT_FRAME_TOO_SMALL      	0x8B
#define    OS_FAILURE_CODE_TRANSPORT_FRAME_TOO_LARGE      	0x8C
#define    OS_FAILURE_CODE_TRANSPORT_INVALID_TARGET_ID    	0x8D
#define    OS_FAILURE_CODE_TRANSPORT_INVALID_INITIATOR_ID 	0x8E
#define    OS_FAILURE_CODE_TRANSPORT_INVALID_INITIATOR_CONTEXT    0x8F
#define    OS_FAILURE_CODE_TRANSPORT_UNKNOWN_FAILURE      	0xFF

#endif // OsStatus.H
