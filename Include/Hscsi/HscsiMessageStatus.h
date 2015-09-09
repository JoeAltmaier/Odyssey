/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiMessageStatus.h
// 
// Description:
// This module defines the status values used in the DetailedStatusCode
// for message handling under HSCSI.
// 
// Update Log
//	$Log: /Gemini/Include/Hscsi/HscsiMessageStatus.h $ 
// 
// 1     9/15/99 11:11a Cchan
// Includes required for the HSCSI library (QL1040B support)
//
/*************************************************************************/
#if !defined(HscsiMessageStatus_H)
#define HscsiMessageStatus_H

/*
    Detailed Status Codes for SCSI and BSA operations

    The 16-bit Detailed Status Code field for SCSI operations is divided 
    into two separate 8-bit fields.  The lower 8 bits are used to report 
    Device Status information.  The upper 8 bits are used to report
    Adapter Status information.  The definitions for these two fields, 
    however, will be consistent with the standard reply message frame 
    structure declaration, which treats this as a single 16-bit field.
*/  


/*  SCSI Device Completion Status Codes (defined by SCSI-2/3)*/

#define HSCSI_SCSI_DEVICE_DSC_MASK                0x00FF

#define HSCSI_SCSI_DSC_SUCCESS                    0x0000
#define HSCSI_SCSI_DSC_CHECK_CONDITION            0x0002
#define HSCSI_SCSI_DSC_BUSY                       0x0008
#define HSCSI_SCSI_DSC_RESERVATION_CONFLICT       0x0018
#define HSCSI_SCSI_DSC_COMMAND_TERMINATED         0x0022
#define HSCSI_SCSI_DSC_TASK_SET_FULL              0x0028
#define HSCSI_SCSI_DSC_ACA_ACTIVE                 0x0030

/*  SCSI Adapter Status Codes (based on CAM-1) */

#define HSCSI_SCSI_HBA_DSC_MASK                   0xFF00

#define HSCSI_SCSI_HBA_DSC_SUCCESS                0x0000

#define HSCSI_SCSI_HBA_DSC_REQUEST_ABORTED        0x0200
#define HSCSI_SCSI_HBA_DSC_UNABLE_TO_ABORT        0x0300
#define HSCSI_SCSI_HBA_DSC_COMPLETE_WITH_ERROR    0x0400
#define HSCSI_SCSI_HBA_DSC_ADAPTER_BUSY           0x0500
#define HSCSI_SCSI_HBA_DSC_REQUEST_INVALID        0x0600
#define HSCSI_SCSI_HBA_DSC_PATH_INVALID           0x0700
#define HSCSI_SCSI_HBA_DSC_DEVICE_NOT_PRESENT     0x0800
#define HSCSI_SCSI_HBA_DSC_UNABLE_TO_TERMINATE    0x0900
#define HSCSI_SCSI_HBA_DSC_SELECTION_TIMEOUT      0x0A00
#define HSCSI_SCSI_HBA_DSC_COMMAND_TIMEOUT        0x0B00

#define HSCSI_SCSI_HBA_DSC_MR_MESSAGE_RECEIVED    0x0D00
#define HSCSI_SCSI_HBA_DSC_SCSI_BUS_RESET         0x0E00
#define HSCSI_SCSI_HBA_DSC_PARITY_ERROR_FAILURE   0x0F00
#define HSCSI_SCSI_HBA_DSC_AUTOSENSE_FAILED       0x1000
#define HSCSI_SCSI_HBA_DSC_NO_ADAPTER             0x1100
#define HSCSI_SCSI_HBA_DSC_DATA_OVERRUN           0x1200
#define HSCSI_SCSI_HBA_DSC_UNEXPECTED_BUS_FREE    0x1300
#define HSCSI_SCSI_HBA_DSC_SEQUENCE_FAILURE       0x1400
#define HSCSI_SCSI_HBA_DSC_REQUEST_LENGTH_ERROR   0x1500
#define HSCSI_SCSI_HBA_DSC_PROVIDE_FAILURE        0x1600
#define HSCSI_SCSI_HBA_DSC_BDR_MESSAGE_SENT       0x1700
#define HSCSI_SCSI_HBA_DSC_REQUEST_TERMINATED     0x1800

#define HSCSI_SCSI_HBA_DSC_IDE_MESSAGE_SENT       0x3300
#define HSCSI_SCSI_HBA_DSC_RESOURCE_UNAVAILABLE   0x3400
#define HSCSI_SCSI_HBA_DSC_UNACKNOWLEDGED_EVENT   0x3500
#define HSCSI_SCSI_HBA_DSC_MESSAGE_RECEIVED       0x3600
#define HSCSI_SCSI_HBA_DSC_INVALID_CDB            0x3700
#define HSCSI_SCSI_HBA_DSC_LUN_INVALID            0x3800
#define HSCSI_SCSI_HBA_DSC_SCSI_TID_INVALID       0x3900
#define HSCSI_SCSI_HBA_DSC_FUNCTION_UNAVAILABLE   0x3A00
#define HSCSI_SCSI_HBA_DSC_NO_NEXUS               0x3B00
#define HSCSI_SCSI_HBA_DSC_SCSI_IID_INVALID       0x3C00
#define HSCSI_SCSI_HBA_DSC_CDB_RECEIVED           0x3D00
#define HSCSI_SCSI_HBA_DSC_LUN_ALREADY_ENABLED    0x3E00
#define HSCSI_SCSI_HBA_DSC_BUS_BUSY               0x3F00

#define HSCSI_SCSI_HBA_DSC_QUEUE_FROZEN           0x4000


#endif