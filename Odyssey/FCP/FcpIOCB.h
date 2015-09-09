/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpIOCB.h
// 
// Description:
// This file describes interfaces for for the QLogic I/O control block.
// 
// Update Log 
// 4/14/98 Jim Frandeen: Create file
// 5/5/98 Jim Frandeen: Use C++ comment style
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 8/25/98 Michael G. Panas: Implemented 16 Bit LUN support
// 9/2/98 Michael G. Panas: add C++ stuff
// 10/1/98 Michael G. Panas: Add more status and flags to support NOTIFY
/*************************************************************************/

#if !defined(FcpIOCB_H)
#define FcpIOCB_H

#include "CDB.h"
#include "Nucleus.h"
#include "FcpISP.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


// Public Methods implemented in FcpIOCB.c

STATUS	FCP_Enable_LUNs();


#pragma pack(1)

// Define suspend timeout.
// If we wait longer than this, a fatal error will occur.
#define FCP_TIMEOUT 100

/*************************************************************************/
//    IOCB type values
/*************************************************************************/
typedef enum
{
    IOCB_TYPE_CONTINUATION_TYPE_0    	= 0x02,
    IOCB_TYPE_STATUS_TYPE_0          	= 0x03,
    IOCB_TYPE_MARKER                 	= 0x04,
    IOCB_TYPE_CONTINUATION_TYPE_1    	= 0x0A,
    IOCB_TYPE_ENABLE_LUN             	= 0x0B,
    IOCB_TYPE_MODIFY_LUN             	= 0x0C,
    IOCB_TYPE_IMMEDIATE_NOTIFY       	= 0x0D,
    IOCB_TYPE_NOTIFY_ACKNOWLEDGE     	= 0x0E,
    IOCB_TYPE_STATUS_CONTINUATION    	= 0x10,
    IOCB_TYPE_COMMAND_TYPE_2			= 0x11,
    IOCB_TYPE_COMMAND_TYPE_4			= 0x15,
    IOCB_TYPE_ACCEPT_TARGET_IO_TYPE_2	= 0x16,
    IOCB_TYPE_CONTINUE_TARGET_IO_TYPE_2	= 0x17,
    IOCB_TYPE_CONTINUE_TARGET_IO_TYPE_4	= 0x1E,
	IOCB_TYPE_RIO_TYPE_1				= 0x21,
    // ISP2200 specific IOCBs
    IOCB_TYPE_VP_CTL                	= 0x30,
    IOCB_TYPE_MODIFY_PORT_CFG          	= 0x31,
    IOCB_TYPE_REPORT_ID_ACQ            	= 0x32,
    IOCB_TYPE_ABORT_OPEN_XCHG          	= 0x33
} IOCB_TYPE;

/*************************************************************************/
//    status values
//	Note: the status fields are 16 bit little endian values so, we use
//        reverse zero filled values to speed things up
/*************************************************************************/
typedef enum
{
	STATUS_REQUEST_COMPLETE				= 0x0100,
	STATUS_REQUEST_ABORTED				= 0x0200,
	STATUS_COMP_WITH_ERROR				= 0x0400,
	STATUS_INVALID_REQUEST				= 0x0600,
	STATUS_PATH_INVALID					= 0x0700,
	STATUS_INVALID_RX_ID				= 0x0800,
	STATUS_RESELECTION_TIMEOUT			= 0x0A00,
	STATUS_COMMAND_TIMEOUT				= 0x0B00,
	STATUS_LIP_RESET					= 0x0E00,
	STATUS_CAPABILITY_NOT_AVAILABLE		= 0x1600,
	STATUS_BUS_DEVICE_RESET				= 0x1700,
	STATUS_QUEUE_FULL					= 0x1C00,
	STATUS_ABORT_TASK					= 0x2000,
	STATUS_PORT_UNAVAILABLE				= 0x2800,
	STATUS_PORT_LOGGED_OUT				= 0x2900,
	STATUS_PORT_CONFIG_CHANGED			= 0x2A00,
	STATUS_RESOURCE_UNAVALIABLE			= 0x3400,
	STATUS_UNACK_EVENT_BY_HOST			= 0x3500,
	STATUS_MESSAGE_RECEIVED				= 0x3600,
	STATUS_CDB_RECEIVED					= 0x3D00
} IOCB_STATUS;

#define	STATUS_MASK		0x7F00		// mask only the bits of interest

/*************************************************************************/
//    control_flags values for IOCB_TYPE_COMMAND_TYPE2
/*************************************************************************/
#define	ISP_REQ_FLAG_NODISCON		0x0001
#define	ISP_REQ_FLAG_HEAD_TAG		0x0002
#define	ISP_REQ_FLAG_ORDERED_TAG	0x0004
#define	ISP_REQ_FLAG_SIMPLE_TAG		0x0008
#define	ISP_REQ_FLAG_USE_TRN		0x0010
#define	ISP_REQ_FLAG_DATA_READ		0x0020
#define	ISP_REQ_FLAG_DATA_WRITE		0x0040

/*************************************************************************/
//	flags values for IOCB_CTIO_TYPE_2, IOCB_CTIO_MODE_0,
//	IOCB_CTIO_MODE_1, IOCB_CTIO_MODE_2
/*************************************************************************/
#define ISP_CTIO_FLAGS_SEND_SCSI_STATUS 	0x8000
#define ISP_CTIO_FLAGS_FAST_POST			0x0200
#define ISP_CTIO_FLAGS_DATA_IN				0x0040
#define ISP_CTIO_FLAGS_DATA_OUT				0x0080
#define ISP_CTIO_FLAGS_NO_DATA_XFR			0x00c0
#define ISP_CTIO_FLAGS_INC_RESOURCE_COUNT	0x0100



#define		IOCB_SIZE 64
#define		IOCB64_SIZE	8
#define		IOCB_SHIFT	6	// number of zeroes to right of address

/*************************************************************************/
//    IOCB common header
/*************************************************************************/
typedef struct _IOCB_TYPE_HEADER {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           entry_status;
}  IOCB_HEADER, *PIOCB_HEADER;

/*************************************************************************/
//    Data Segment Descriptor
/*************************************************************************/
typedef struct _DATA_SEGMENT_DESCRIPTOR {
   U32                          Address;
   U32                          Length;
} DATA_SEGMENT_DESCRIPTOR, *PDATA_SEGMENT_DESCRIPTOR;


/*************************************************************************/
//    IOCB Command Type 2
/*************************************************************************/
typedef struct _IOCB_TYPE_COMMAND_TYPE2 {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           entry_status;
   U32                          system_defined2;
   U8                           reserved2;
   U8                           target;
   U16                          LUN;
   U16                          control_flags;
   U16                          reserved;
   U16                          timeout;
   U16                          data_segment_count;
   CDB16                        CDB;
   U32                          data_byte_count;
   DATA_SEGMENT_DESCRIPTOR      data_segment_0;
   DATA_SEGMENT_DESCRIPTOR      data_segment_1;
   DATA_SEGMENT_DESCRIPTOR      data_segment_2;
}  IOCB_COMMAND_TYPE2, *PIOCB_COMMAND_TYPE2;

/*************************************************************************/
//    IOCB Command Type 4
/*************************************************************************/
typedef struct _IOCB_TYPE_COMMAND_TYPE4 {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           entry_status;
   U32                          system_defined2;
   U8	                        reserved2;
   U8                           target;
   U16                          LUN;
   U16                          control_flags;
   U16                          reserved;
   U16                          timeout;
   U16                          data_segment_count;
   CDB16                        CDB;
   U32                          data_byte_count;
   U16							dsd_list_type;
   U32							dsd_list_base_address;
   U32							dsd_list_address;
   U32							dsd_list_address_lo;
   U8							reserved3[10];

}  IOCB_COMMAND_TYPE4, *PIOCB_COMMAND_TYPE4;

/*************************************************************************/
//    IOCB RIO Type 1
/*************************************************************************/
typedef struct _IOCB_TYPE_RIO_TYPE1 {
   U8                           entry_type;
   U8                           entry_count;
   U8                           handle_count;
   U8                           entry_status;
   U32                          handle[15];

}  IOCB_RIO_TYPE1, *PIOCB_RIO_TYPE1;

// timeout value, in seconds to use for commands submitted
#define IOCB_TIMEOUT_SECONDS	4

/*************************************************************************/
//    IOCB status Type 0
/*************************************************************************/
typedef struct _IOCB_STATUS_TYPE0 {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           entry_status;
   U32                          system_defined2;
   U16                          SCSI_status;
   U16                          status;
   U16                          state_flags;
   U16                          status_flags;
   U16                          response_info_length;
   U16                          sense_data_length;
   U32                          residual_transfer_length;
   U8                           response_info[8];
   U8                           sense_data[32];
}  IOCB_STATUS_TYPE0, *PIOCB_STATUS_TYPE0;

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
//    IOCB Enable LUN
/*************************************************************************/
typedef struct _IOCB_TYPE_ENABLE_LUN {
   U8	entry_type;
   U8	entry_count;
   U8	system_defined;
   U8	entry_status;
   U32	system_defined2;
   U8	reserved0;
   U8	target_ID;
   U8	reserved2[6];
   U8	status;
   U8	reserved3;
   U8	command_count;
   U8	immediate_notify_count;
   U16	reserved4;
   U16	timeout;
   U8	reserved5[40];
}  IOCB_ENABLE_LUN, *PIOCB_ENABLE_LUN;

/*************************************************************************/
//    IOCB Modify LUN
/*************************************************************************/
typedef struct _IOCB_MODIFY_LUN {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           entry_status;
   U32                          system_defined2;
   U8                           reserved0;
   U8                           target_ID;
   U8                           operators;
   U8                           reserved2[5];
   U8                           status;
   U8                           reserved3;
   U8                           command_count;
   U8                           immediate_notify_count;
   U16                          reserved4;
   U16                          timeout;
   U8                           reserved5[40];
}  IOCB_MODIFY_LUN, *PIOCB_MODIFY_LUN;

/*************************************************************************/
//    IOCB Immediate Notify
/*************************************************************************/
typedef struct _IOCB_IMMEDIATE_NOTIFY {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           entry_status;
   U32                          system_defined2;
   U8                           reserved0;
   U8                           initiator_ID;
   U16							LUN;
   U8							target_ID;
   U8                           reserved2[3];
   U16                          status;
   U16                          task_flags;
   U16                          SequenceIdentifier;
   U8                           reserved5[42];
}  IOCB_IMMEDIATE_NOTIFY, *PIOCB_IMMEDIATE_NOTIFY;

// Notify Task Flags (these are valid with STATUS_MESSAGE_RECEIVED)
#define	ISP_TASK_FLAGS_ABORT_TASK_SET		0x0200
#define	ISP_TASK_FLAGS_CLEAR_TASK_SET		0x0400
#define	ISP_TASK_FLAGS_TARGET_RESET			0x2000
#define	ISP_TASK_FLAGS_CLEAR_ACA			0x4000
#define	ISP_TASK_FLAGS_TERMINATE_TASK		0x8000


/*************************************************************************/
//    IOCB Immediate Notify Acknowledge
/*************************************************************************/
typedef struct _IOCB_NOTIFY_ACKNOWLEDGE {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           entry_status;
   U32                          system_defined2;
   U8                           reserved0;
   U8                           initiator_ID;
   U8							target_ID;
   U8                           reserved1;
   U16							flags;
   U8                           reserved2[2];
   U16                          status;
   U16                          task_flags;
   U16                          SequenceIdentifier;
   U8                           reserved5[42];
}  IOCB_NOTIFY_ACKNOWLEDGE, *PIOCB_NOTIFY_ACKNOWLEDGE;

// Notify Acknowledge Flags
#define	ISP_ACK_FLAGS_CLEAR_LIP_RESET		0x0020
#define	ISP_ACK_FLAGS_INC_NOTIFY_RES_CNT	0x0100


/*************************************************************************/
//    IOCB Accept target I/O Type 2
/*************************************************************************/
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
   U8							target_ID;
   U8                           reserved2[21];
}  IOCB_ATIO_TYPE_2, *PIOCB_ATIO_TYPE_2;

/*************************************************************************/
//    IOCB Continue target I/O Type 2
//    This CTIO is returned to the target driver by the ISP.
/*************************************************************************/
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

/*************************************************************************/
//    IOCB Continue target I/O Type 4
//    This CTIO is sent to the ISP by the target driver to complete the data phase.
/*************************************************************************/
typedef struct _IOCB_CTIO_TYPE_4 {
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
   U8	                        reserved[6];
   U16                          SCSI_status;
   U32                          transfer_length;
   U16							dsd_list_type;
   U32							dsd_list_base_address;
   U32							dsd_list_address;
   U32							dsd_list_address_lo;
   U8							reserved1[10];

}  IOCB_CTIO_TYPE_4, *PIOCB_CTIO_TYPE_4;

/*************************************************************************/
//    IOCB Continue target I/O Type 2 Mode 0
//    This CTIO is sent to the ISP by the target driver
/*************************************************************************/
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
   DATA_SEGMENT_DESCRIPTOR      segment_0;
   DATA_SEGMENT_DESCRIPTOR      segment_1;
   DATA_SEGMENT_DESCRIPTOR      segment_2;
}  IOCB_CTIO_MODE_0, *PIOCB_CTIO_MODE_0;

/*************************************************************************/
//    IOCB Continue target I/O Type 2 Mode 1
//    This CTIO is sent to the ISP by the target driver
/*************************************************************************/
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

/*************************************************************************/
//    IOCB Continue target I/O Type 2 Mode 2
//    This CTIO is sent to the ISP by the target driver
/*************************************************************************/
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
   DATA_SEGMENT_DESCRIPTOR      response_descriptor;
   U8							reserved2[16];
}  IOCB_CTIO_MODE_2, *PIOCB_CTIO_MODE_2;

// ISP2200 specific IOCB structures

/*************************************************************************/
//    IOCB Virtual Port Control
/*************************************************************************/
typedef struct _IOCB_VP_CTL {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           entry_status;
   U32                          system_defined2;
   U16                          reserved0;
   U16                          status;
   U8							command;
   U8                           vp_count;
   U8                           reserved1;
   U8                           vp_index[31];
   U8                           reserved2[18];
}  IOCB_VP_CTL, *PIOCB_VP_CTL;

// VP_CTL values for command
#define	ISP_VP_CTL_ENABLE_VPS				0x00
#define	ISP_VP_CTL_DISABLE_VPS				0x08
#define	ISP_VP_CTL_DISABLE_VPS_LIP			0x09
#define	ISP_VP_CTL_DISABLE_VPS_LOGOUT		0x0A

/*************************************************************************/
//    IOCB Modify Virtual Port config
/*************************************************************************/
typedef struct _IOCB_MODIFY_VP_CFG {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           entry_status;
   U32                          system_defined2;
   U16                          reserved0;
   U16                          status;
   U8							command;
   U8                           vp_count;
   U8                           vp_index1;
   U8                           vp_index2;
   VP_CONFIG					vp1;
   VP_CONFIG                    vp2;
   U8                           reserved2[12];
}  IOCB_MODIFY_VP_CFG, *PIOCB_MODIFY_VP_CFG;

// MODIFY_VP_CFG values for command
#define	ISP_MOD_VP_CFG_MODIFY_VPS		0x00
#define	ISP_MOD_VP_CFG_MODIFY_ENABLE	0x01

// MODIFY_VP_CFG values for status (byte swap)
#define	ISP_MOD_VP_CFG_STS_CFG_CMPL		0x00
#define	ISP_MOD_VP_CFG_STS_VP_STS_ERR	0x01
#define	ISP_MOD_VP_CFG_STS_VP_CNT_ERR	0x02
#define	ISP_MOD_VP_CFG_STS_GEN_ERR		0x03
#define	ISP_MOD_VP_CFG_STS_VP_IDX_ERR	0x04

/*************************************************************************/
//    IOCB Modify Virtual Port config
/*************************************************************************/
typedef struct _IOCB_REPORT_ID_ACQ {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           entry_status;
   U32                          system_defined2;
   U8                           ID_count;
   U8                           reserved1;
   U8                           primary_ID;
   U8                           vp_acquired_ID[31];
   U8                           reserved2[22];
}  IOCB_REPORT_ID_ACQ, *PIOCB_REPORT_ID_ACQ;

// Valid Loop ID values 0x00 - 0x7D plus these:
#define	ISP_VP_NOT_REPORTING				0xFF
#define	ISP_VP_NON_PARTICIPATIING			0xFE


#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif //   FcpIOCB_H