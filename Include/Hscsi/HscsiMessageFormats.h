/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: MessageFormats.h
// 
// Description:
// This module defines structures used for message handling under HSCSI.
// It includes Bsa and Scsi style message formats.
// 
// Update Log
//	$Log: /Gemini/Include/Hscsi/HscsiMessageFormats.h $ 
// 
// 1     9/15/99 11:11a Cchan
// Includes required for the HSCSI library (QL1040B support)
//
/*************************************************************************/
#if !defined(MessageFormats_H)
#define MessageFormats_H

#include "CTIdLun.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#define HSCSI_SCSI_CDB_LENGTH			12
#define HSCSI_SCSI_SENSE_DATA_SZ      	40


typedef U16     HSCSI_SCB_FLAGS;

#define HSCSI_SCB_FLAG_XFER_DIR_MASK          0xC000
#define HSCSI_SCB_FLAG_NO_DATA_XFER           0x0000
#define HSCSI_SCB_FLAG_XFER_FROM_DEVICE       0x4000
#define HSCSI_SCB_FLAG_XFER_TO_DEVICE         0x8000

#define HSCSI_SCB_FLAG_ENABLE_DISCONNECT      0x2000

#define HSCSI_SCB_FLAG_TAG_TYPE_MASK          0x0380
#define HSCSI_SCB_FLAG_NO_TAG_QUEUEING        0x0000
#define HSCSI_SCB_FLAG_SIMPLE_QUEUE_TAG       0x0080
#define HSCSI_SCB_FLAG_HEAD_QUEUE_TAG         0x0100
#define HSCSI_SCB_FLAG_ORDERED_QUEUE_TAG      0x0180
#define HSCSI_SCB_FLAG_ACA_QUEUE_TAG          0x0200

#define HSCSI_SCB_FLAG_AUTOSENSE_MASK         0x0060
#define HSCSI_SCB_FLAG_DISABLE_AUTOSENSE      0x0000
#define HSCSI_SCB_FLAG_SENSE_DATA_IN_MESSAGE  0x0020
#define HSCSI_SCB_FLAG_SENSE_DATA_IN_BUFFER   0x0060


typedef U16     HSCSI_BSA_CTL_FLAGS;

#define HSCSI_BSA_FLAG_PROGRESS_REPORT    0x0080

/* I2O BSA Block Read Message Control Flags */

typedef U16     HSCSI_BSA_READ_FLAGS;
#define HSCSI_BSA_RD_FLAG_DONT_RETRY      0x0001
#define HSCSI_BSA_RD_FLAG_SOLO            0x0002
#define HSCSI_BSA_RD_FLAG_CACHE_READ      0x0004
#define HSCSI_BSA_RD_FLAG_READ_PREFETCH   0x0008
#define HSCSI_BSA_RD_FLAG_CACHE_DATA      0x0010

/* I2O BSA Block Write Message Control Flags */

typedef U16     HSCSI_BSA_WRITE_FLAGS;
#define HSCSI_BSA_WR_FLAG_DONT_RETRY      0x0001
#define HSCSI_BSA_WR_FLAG_SOLO            0x0002
#define HSCSI_BSA_WR_FLAG_DONT_CACHE      0x0004
#define HSCSI_BSA_WR_FLAG_WRITE_THRU      0x0008
#define HSCSI_BSA_WR_FLAG_WRITE_TO        0x0010


//============================================================================
// Define payloads used for Bsa and Scsi Messages
//
// Need a struct to hold the payload section of the
// SCSI SCB EXEC message
typedef struct _SCB_PAYLOAD {
	IDLUN					IdLun;
    U8                      CDBLength;
    U8                      Reserved;
    HSCSI_SCB_FLAGS			SCBFlags;
    U8                      CDB[HSCSI_SCSI_CDB_LENGTH];
    U32                     ByteCount;
} SCB_PAYLOAD, *PSCB_PAYLOAD;

// Need a struct to hold the payload section for a
// SCB Error Reply message
typedef struct _SCB_REPLY_PAYLOAD {
    U32                     TransferCount;
    U32                     AutoSenseTransferCount;
    U8                      SenseData[HSCSI_SCSI_SENSE_DATA_SZ];
} SCB_REPLY_PAYLOAD, *PSCB_REPLY_PAYLOAD;


// Need a struct to hold the payload section for a
// BSA message
typedef struct _BSA_RW_PAYLOAD {
    HSCSI_BSA_READ_FLAGS      ControlFlags;		// same size for R/W
    U8                      TimeMultiplier;
    U8                      FetchAhead;			// reserved for WRITE
    U32                     TransferByteCount;
    U32                     LogicalBlockAddress;
//  HSCSI_SG_ELEMENT          SGL;
} BSA_RW_PAYLOAD, *PBSA_RW_PAYLOAD;

// Need a struct to hold the payload section for a
// BSA Reply message
typedef struct _BSA_REPLY_PAYLOAD {
    U32                     TransferCount;
    U32                     LogicalBlockAddress;
    
    // these two fields are added to pass the sense data to
    // raid software.
    U32                     AutoSenseTransferCount;
    U8                      SenseData[HSCSI_SCSI_SENSE_DATA_SZ];
} BSA_REPLY_PAYLOAD, *PBSA_REPLY_PAYLOAD;

typedef union {
	SCB_PAYLOAD			p1;
	SCB_REPLY_PAYLOAD	p2;
	BSA_RW_PAYLOAD		p3;
	BSA_REPLY_PAYLOAD	p4;
} HSCSI_MSG_SIZE;		// to get the max length

#ifdef          __cplusplus
}                               /* C declarations in C++     */
#endif

#endif /* MessageFormats_H  */
