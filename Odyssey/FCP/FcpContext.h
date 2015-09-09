/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpContext.h
// 
// Description:
// This file describes interfaces for for the QLogic I/O control block.
// 
// Update Log 
// 4/14/98 Jim Frandeen: Create file
// 5/5/98 Jim Frandeen: Use C++ comment style
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
/*************************************************************************/

#if !defined(FcpIOCB_H)
#define FcpIOCB_H

#include "i2otypes.h"
#include "CDB.h"
#include "Nucleus.h"

PRAGMA_ALIGN_PUSH
PRAGMA_PACK_PUSH

STATUS	FCP_IOCB_Create(void **pp_memory);
void	FCP_IOCB_Destroy();

// Define suspend timeout.
// If we wait longer than this, a fatal error will occur.
#define FCP_TIMEOUT 1000

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
    IOCB_TYPE_ACCEPT_TARGET_IO_TYPE_2	= 0x16,
    IOCB_TYPE_CONTINUE_TARGET_IO_TYPE_2	= 0x17,
    IOCB_TYPE_COMMAND_TYPE_2			= 0x11
} IOCB_TYPE;

/*************************************************************************/
//    status values
/*************************************************************************/
typedef enum
{
	STATUS_PATH_INVALID					= 0x07,
	STATUS_CAPABILITY_NOT_AVAILABLE		= 0x16,
	STATUS_BUS_DEVICE_RESET				= 0x17,
	STATUS_CDB_RECEIVED					= 0x3D
} IOCB_STATUS;

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
   U8                           LUN;
   U8                           target;
   U16                          reserved;
   U16                          control_flags;
   U16                          reserved2;
   U16                          timeout;
   U16                          data_segment_count;
   CDB16                        CDB;
   U16                          data_byte_count;
   DATA_SEGMENT_DESCRIPTOR      data_segment_0;
   DATA_SEGMENT_DESCRIPTOR      data_segment_1;
   DATA_SEGMENT_DESCRIPTOR      data_segment_2;
}  IOCB_COMMAND_TYPE2, *PIOCB_COMMAND_TYPE2;

// timeout value, in seconds to use for commands submitted
#define IOCB_TIMEOUT_SECONDS 2

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
   U8                           reserved[8];
   U8                           sense_data[32];
}  IOCB_STATUS_TYPE0, *PIOCB_STATUS_TYPE0;

/*************************************************************************/
//    IOCB Enable LUN
/*************************************************************************/
typedef struct _IOCB_TYPE_ENABLE_LUN {
   U8	entry_type;
   U8	entry_count;
   U8	system_defined;
   U8	entry_status;
   U32	system_defined2;
   U8	LUN;
   U8	reserved;
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
   U8                           LUN;
   U8                           reserved;
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
   U8                           LUN;
   U8                           initiator_ID;
   U8                           reserved2[6];
   U16                          status;
   U16                          task_flags;
   U16                          SequenceIdentifier;
   U8                           reserved5[42];
}  IOCB_IMMEDIATE_NOTIFY, *PIOCB_IMMEDIATE_NOTIFY;

/*************************************************************************/
//    IOCB Accept target I/O Type 2
/*************************************************************************/
typedef struct _IOCB_ATIO_TYPE_2 {
   U8                           entry_type;
   U8                           entry_count;
   U8                           system_defined;
   U8                           entry_status;
   U32                          system_defined2;
   U8                           LUN;
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
   U32                          reserved2;
   U16                          SCSI_status;
   U8                           sense_data[16];
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
   U8                           LUN;
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
   U8                           sense_data[16];
}  IOCB_CTIO_TYPE_2, *PIOCB_CTIO_TYPE_2;

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
   U8                           LUN;
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
   U8                           LUN;
   U8                           initiator_ID;
   U16							RX_ID;
   U16							flags;
   U16							status;
   U16                          timeout;
   U16                          data_segment_count;
   U32                          relative_offset;
   U32							residual_transfer_length;
   U16                          reserved;
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
   U8                           LUN;
   U8                           initiator_ID;
   U16							RX_ID;
   U16							flags;
   U16							status;
   U16                          timeout;
   U16                          data_segment_count;
   U32                          relative_offset;
   U32							residual_transfer_length;
   U8                           reserved[6];
   U32                          transfer_length;
   DATA_SEGMENT_DESCRIPTOR      response_descriptor;
   U8							reserved2[16];
}  IOCB_CTIO_MODE_2, *PIOCB_CTIO_MODE_2;


PRAGMA_PACK_POP
PRAGMA_ALIGN_POP


#endif //   FcpIOCB_H