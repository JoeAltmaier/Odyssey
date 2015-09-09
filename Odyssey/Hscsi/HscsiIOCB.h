/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiIOCB.h
// 
// Description:
// This file describes interfaces for for the QLogic I/O control block (IOCB).
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiIOCB.h $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#if !defined(HscsiIOCB_H)
#define HscsiIOCB_H

#include "CDB.h"
#include "Nucleus.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


// Public Methods implemented in HscsiIOCB.c

STATUS	HSCSI_Enable_LUNs();


#pragma pack(1)

// Define suspend timeout.
// If we wait longer than this, a fatal error will occur.
#define HSCSI_TIMEOUT 100

/*************************************************************************/
//    IOCB type values
/*************************************************************************/
typedef enum
{
	HSCSI_IOCB_TYPE_COMMAND					= 0x01,
    HSCSI_IOCB_TYPE_CONTINUATION	    	= 0x02,
    HSCSI_IOCB_TYPE_STATUS		          	= 0x03,
    HSCSI_IOCB_TYPE_MARKER                 	= 0x04,
    HSCSI_IOCB_TYPE_EXTENDED_COMMAND    	= 0x05,
    HSCSI_IOCB_TYPE_SCAM_OPERATION			= 0x08,
    HSCSI_IOCB_TYPE_COMMAND_A64				= 0x09,
    HSCSI_IOCB_TYPE_CONTINUATION_A64		= 0x0A,
    HSCSI_IOCB_TYPE_STATUS_CONTINUATION    	= 0x10
} HSCSI_IOCB_TYPE;

/*************************************************************************/
//    status values
//	Note: the status fields are 16 bit little endian values so, we use
//        reverse zero filled values to speed things up
/*************************************************************************/
typedef enum
{
	STATUS_COMPLETE						= 0x0000,
	STATUS_INCOMPLETE					= 0x0100,
	STATUS_DMA							= 0x0200,
	STATUS_TRANSPORT					= 0x0300,
	STATUS_RESET						= 0x0400,
	STATUS_ABORTED						= 0x0500,
	STATUS_TIMEOUT						= 0x0600,
	STATUS_DATA_OVERRUN					= 0x0700,
	STATUS_COMMAND_OVERRUN				= 0x0800,
	STATUS_STATUS_OVERRUN				= 0x0900,
	STATUS_BAD_MESSAGE					= 0x0A00,
	STATUS_NO_MESSAGE_OUT				= 0x0B00,
	STATUS_EXTENDED_ID_FAILED			= 0x0C00,
	STATUS_IDE_MESSAGE_FAILED			= 0x0D00,
	STATUS_ABORT_MESSAGE_FAILED			= 0x0E00,
	STATUS_REJECT_MESSAGE_FAILED		= 0x0F00,
	STATUS_NOP_MESSAGE_FAILED			= 0x1000,
	STATUS_PARITY_ERROR_MESSAGE_FAILED	= 0x1100,
	STATUS_DEVICE_RESET_MESSAGE_FAILED	= 0x1200,
	STATUS_ID_MESSAGE_FAILED			= 0x1300,
	STATUS_UNEXPECTED_BUS_FREE			= 0x1400,
	STATUS_DATA_UNDERRUN				= 0x1500,
	STATUS_COMMAND_UNDERRUN				= 0x1600,
	STATUS_TRANSACTION_ERROR1			= 0x1800,
	STATUS_TRANSACTION_ERROR2			= 0x1900,
	STATUS_TRANSACTION_ERROR3			= 0x1A00,
	STATUS_INVALID_ENTRY_TYPE			= 0x1B00,
	STATUS_DEVICE_QUEUE_FULL			= 0x1C00,
	STATUS_SCSI_PHASE_SKIPPED			= 0x1D00,
	STATUS_ARS_FAILED					= 0x1E00,
	STATUS_SYNCHRONOUS_TRANSFER_FAILED	= 0x2000
} HSCSI_IOCB_STATUS;

#define	STATUS_MASK		0x7F00		// mask only the bits of interest

/*************************************************************************/
//    control_flags values for IOCB_TYPE_COMMAND
/*************************************************************************/
#define	HSCSI_REQ_FLAG_NODISCON		0x0001
#define	HSCSI_REQ_FLAG_HEAD_TAG		0x0002
#define	HSCSI_REQ_FLAG_ORDERED_TAG	0x0004
#define	HSCSI_REQ_FLAG_SIMPLE_TAG	0x0008
#define	HSCSI_REQ_FLAG_USE_TRN		0x0010
#define	HSCSI_REQ_FLAG_DATA_READ	0x0020
#define	HSCSI_REQ_FLAG_DATA_WRITE	0x0040
#define HSCSI_REQ_FLAG_NOAUTOREQS	0x0100
#define HSCSI_REQ_FLAG_FORCE_ASYNC	0x0200
#define HSCSI_REQ_FLAG_FORCE_SYNC	0x0400
#define HSCSI_REQ_FLAG_NOPARITY		0x1000
#define HSCSI_REQ_FLAG_STOP_QUEUE	0x2000
#define HSCSI_REQ_FLAG_PRIORITY		0x8000


/*************************************************************************/
//	flags values for IOCB_CTIO_TYPE_2, IOCB_CTIO_MODE_0,
//	IOCB_CTIO_MODE_1, IOCB_CTIO_MODE_2
/*************************************************************************
#define ISP_CTIO_FLAGS_SEND_SCSI_STATUS 	0x8000
#define ISP_CTIO_FLAGS_FAST_POST			0x0200
#define ISP_CTIO_FLAGS_DATA_IN				0x0040
#define ISP_CTIO_FLAGS_DATA_OUT				0x0080
#define ISP_CTIO_FLAGS_NO_DATA_XFR			0x00c0
#define ISP_CTIO_FLAGS_INC_RESOURCE_COUNT	0x0100
*/


#define		IOCB_SIZE 64
#define		IOCB_SHIFT	6	// number of zeroes to right of address

/*************************************************************************/
//	IOCB common header
/*************************************************************************/
typedef struct _HSCSI_IOCB_TYPE_HEADER {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           entry_status;
}  HSCSI_IOCB_HEADER, *PHSCSI_IOCB_HEADER;

/*************************************************************************/
//	Data Segment Descriptor
/*************************************************************************/

typedef struct _HSCSI_DATA_SEGMENT_DESCRIPTOR {
   U32                          Address;
   U32                          Length;
} HSCSI_DATA_SEGMENT_DESCRIPTOR, *PHSCSI_DATA_SEGMENT_DESCRIPTOR;


/*************************************************************************/
//	IOCB Command (01)
/*************************************************************************/
typedef struct _HSCSI_IOCB_TYPE_COMMAND {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           flags;
   U32                          system_defined2;
   U8                           LUN;
   U8                           target;
   U16                          CDB_length;
   U16                          control_flags;
   U16                          reserved;
   U16                          timeout;
   U16                          data_segment_count;
   CDB12                        CDB;
   HSCSI_DATA_SEGMENT_DESCRIPTOR      data_segment_0;
   HSCSI_DATA_SEGMENT_DESCRIPTOR      data_segment_1;
   HSCSI_DATA_SEGMENT_DESCRIPTOR      data_segment_2;
   HSCSI_DATA_SEGMENT_DESCRIPTOR	  data_segment_3;
}  HSCSI_IOCB_COMMAND, *PHSCSI_IOCB_COMMAND;

// timeout value, in seconds to use for commands submitted
#define IOCB_TIMEOUT_SECONDS	2

/*************************************************************************/
//	IOCB Continuation (02)
/*************************************************************************/
typedef struct _HSCSI_IOCB_TYPE_CONTINUATION {
	U8							entry_type;
	U8							entry_count;
	U8							system_defined;
	U8							flags;
	U32							system_defined2;
	HSCSI_DATA_SEGMENT_DESCRIPTOR	data_segment_0;
	HSCSI_DATA_SEGMENT_DESCRIPTOR	data_segment_1;
	HSCSI_DATA_SEGMENT_DESCRIPTOR	data_segment_2;
	HSCSI_DATA_SEGMENT_DESCRIPTOR	data_segment_3;
	HSCSI_DATA_SEGMENT_DESCRIPTOR	data_segment_4;
	HSCSI_DATA_SEGMENT_DESCRIPTOR	data_segment_5;
	HSCSI_DATA_SEGMENT_DESCRIPTOR	data_segment_6;
}	HSCSI_IOCB_CONTINUATION, *PHSCSI_IOCB_CONTINUATION;

	
/*************************************************************************/
//	IOSB Status (03)
/*************************************************************************/
typedef struct _IOCB_STATUS_ENTRY {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           flags;
   U32                          system_defined2;
   U16                          SCSI_status;
   U16                          status;
   U16                          state_flags;
   U16                          status_flags;
   U16                          timeout;
   U16                          sense_data_length;
   U32                          residual_transfer_length;
   U8                           response_info[8];
   U8                           sense_data[32];
}  IOCB_STATUS_ENTRY, *PIOCB_STATUS_ENTRY;

// Flags used for SCSI_status
#define	IOCB_SCSI_FLAGS_RESIDUAL_UNDER				0x0800
#define	IOCB_SCSI_FLAGS_RESIDUAL_OVER				0x0400
#define	IOCB_SCSI_FLAGS_SENSE_VALID					0x0200
#define	IOCB_SCSI_FLAGS_RESPONSE_DATA_VALID			0x0100
#define	IOCB_SCSI_FLAGS_SCSI_STATUS_MASK			0x00FF

// Flags used for state_flags
#define	IOCB_STATE_FLAGS_XFER_COMPLETE				0x4000
#define	IOCB_STATE_FLAGS_GOT_STATUS					0x1000
#define	IOCB_STATE_FLAGS_XFERED_DATA				0x0800
#define	IOCB_STATE_FLAGS_SENT_COMMAND				0x0400

// flags used for status_flags
#define	IOCB_STATUS_FLAGS_TIMEOUT					0x0040
#define	IOCB_STATUS_FLAGS_ABORTED					0x0020


/*************************************************************************/
//	IOCB Marker (04)
/*************************************************************************/
typedef struct _IOCB_MARKER_ENTRY {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           flags;
   U32                          system_defined2;
   U8							LUN;
   U8							target;
   U8		                	modifier;
   U8							reserved;
   U32                          reserved1;
   U32							reserved2;
   U32							reserved3;
   U32							reserved4;
   U32							reserved5;
   U32							reserved6;
   U32							reserved7;
   U32							reserved8;
   U32							reserved9;
   U32							reserved10;
   U32							reserved11;
   U32							reserved12;
   U32							reserved13;
}  IOCB_MARKER_ENTRY, *PIOCB_MARKER_ENTRY;

/*************************************************************************/
//	IOCB Extended Command (05)
/*************************************************************************/

/*************************************************************************/
//	IOSB Continuation Status (0x10)
/*************************************************************************/

/*************************************************************************/
//    IOCB Accept target I/O Type 2
/*************************************************************************
typedef struct _IOCB_ATIO_TYPE_2 {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           entry_status;
   U32                          system_defined2;
   U8                           reserved0;
   U8                           initiator_ID;
   U16							RX_ID;
   U16							flags;
   U16							status;
   U8                           reserved;
   U8                           task_codes;
   U8                           task_flags;
   U8							execution_codes;
   CDB16                        CDB;
   U32                          transfer_length;
   U16							LUN;
   U16                          reserved2;
   U16                          SCSI_status;
   U8                           sense_data[18];
}  IOCB_ATIO_TYPE_2, *PIOCB_ATIO_TYPE_2;
*/

/*************************************************************************/
//    IOCB Continue target I/O Type 2
//    This CTIO is returned to the target driver by the ISP.
/*************************************************************************
typedef struct _IOCB_CTIO_TYPE_2 {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           entry_status;
   U32                          system_defined2;
   U8                           reserved0;
   U8                           initiator_ID;
   U16							RX_ID;
   U16							flags;
   U16							status;
   U16                          timeout;
   U16                          data_segment_count;
   U32                          relative_offset;
   U32							residual_transfer_length;
   U8	                        reserved[16];
   U16                          SCSI_status;
   U8                           sense_data[18];
}  IOCB_CTIO_TYPE_2, *PIOCB_CTIO_TYPE_2;
*/

/*************************************************************************/
//    IOCB Continue target I/O Type 2 Mode 0
//    This CTIO is sent to the ISP by the target driver
/*************************************************************************
typedef struct _IOCB_CTIO_MODE_0 {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           entry_status;
   U32                          system_defined2;
   U8                           reserved0;
   U8                           initiator_ID;
   U16							RX_ID;
   U16							flags;
   U16							status;
   U16                          timeout;
   U16                          data_segment_count;
   U32                          relative_offset;
   U32							residual_transfer_length;
   U8                           reserved[6];
   U16                          SCSI_status;
   U32                          transfer_length;
   HSCSI_DATA_SEGMENT_DESCRIPTOR      segment_0;
   HSCSI_DATA_SEGMENT_DESCRIPTOR      segment_1;
   HSCSI_DATA_SEGMENT_DESCRIPTOR      segment_2;
}  IOCB_CTIO_MODE_0, *PIOCB_CTIO_MODE_0;
*/

/*************************************************************************/
//    IOCB Continue target I/O Type 2 Mode 1
//    This CTIO is sent to the ISP by the target driver
/*************************************************************************
typedef struct _IOCB_CTIO_MODE_1 {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           entry_status;
   U32                          system_defined2;
   U8                           reserved0;
   U8                           initiator_ID;
   U16							RX_ID;
   U16							flags;
   U16							status;
   U16                          timeout;
   U16                          data_segment_count;
   U32                          relative_offset;
   U32							residual_transfer_length;
   U32                          reserved;
   U16                          sense_length;
   U16                          SCSI_status;
   U16                          response_length;
   U8							response_information[26];
}  IOCB_CTIO_MODE_1, *PIOCB_CTIO_MODE_1;
*/
/*************************************************************************/
//    IOCB Continue target I/O Type 2 Mode 2
//    This CTIO is sent to the ISP by the target driver
/*************************************************************************
typedef struct _IOCB_CTIO_MODE_2 {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           entry_status;
   U32                          system_defined2;
   U8                           reserved0;
   U8                           initiator_ID;
   U16							RX_ID;
   U16							flags;
   U16							status;
   U16                          timeout;
   U16                          data_segment_count;
   U32                          relative_offset;
   U32							residual_transfer_length;
   U8                           reserved[8];
   U32                          transfer_length;
   HSCSI_DATA_SEGMENT_DESCRIPTOR      response_descriptor;
   U8							reserved2[16];
}  IOCB_CTIO_MODE_2, *PIOCB_CTIO_MODE_2;
*/

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif //   HscsiIOCB_H