/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Scsi.h
// 
// Description:
// This file contains all the SCSI structures and data for the Odyssey.
// The structures and defines used here will conform to the SCSI-2/SCSI-3
// specifications described in the ANSI T10 suite of specifications. 
// 
// 
// Update Log
//  $Log: /Gemini/Include/Fcp/Scsi.h $
// 
// 14    1/24/00 6:57p Mpanas
// Correct typo B->E
// 
// 05/18/98 Michael G. Panas: Create file
// 05/21/98 Jim Frandeen: Add SCSI Status values and new commands
// 05/29/98 Michael G. Panas: Add structs for commands, more details
// 09/02/98 Michael G. Panas: add C++ stuff
// 10/08/98 Michael G. Panas: correct typo in CDB6, add Inquiry Serial
//                            number page
// 01/20/99 Michael G. Panas: add base set of Mode Pages, correct typos
/*************************************************************************/

#if !defined(SCSI_H)
#define SCSI_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#pragma pack(1)

#define	SYSTEM_BLOCK_SIZE		0x200		// natural SCSI drive block size


// Define the SCSI commands used by the Odyssey System
// Any command not on this list will cause an error.
typedef enum
{									// Input Structure
	CMD_TEST_UNIT		= 0x00,		// CDB6
	CMD_REQUEST_SENSE	= 0x03,		// CDB6
	CMD_FORMAT_UNIT		= 0x04,		// FORMAT_UNIT
	CMD_READ6			= 0x08,		// CDB6
	CMD_WRITE6			= 0x0A,		// CDB6
	CMD_INQUIRY			= 0x12,		// INQUIRY
	CMD_MODE_SELECT6	= 0x15,		// MODE_SELECT6
	CMD_RESERVE6		= 0x16,		// RESERVE6
	CMD_RELEASE6		= 0x17,		// RESERVE6
	CMD_MODE_SENSE6		= 0x1A,		// MODE_SENSE6
	CMD_START_UNIT		= 0x1B,		// CDB6
	CMD_RECEIVE_DIAG	= 0x1C,		// RECEIVE_DIAG
	CMD_SEND_DIAG		= 0x1D,		// SEND_DIAG
	CMD_READ_CAPACITY	= 0x25,		// READ_CAP
	CMD_READ10			= 0x28,		// CDB10
	CMD_WRITE10			= 0x2A,		// CDB10
	CMD_VERIFY10		= 0x2F,		// CDB10
	CMD_MODE_SELECT10	= 0x55,		// MODE_SELECT10
	CMD_RESERVE10		= 0x56,		// RESERVE10
	CMD_RELEASE10		= 0x57,		// RESERVE10
	CMD_MODE_SENSE10	= 0x5A,		// MODE_SENSE10
	CMD_REPORT_LUNS		= 0xA0		// CDB12
} SCSI_COMMANDS;

// General Command formats
// NOTE: all reserved fields should be set to zero
typedef struct _Gen_Cmd_6 {
	U8				Cmd;
	U8				reserved:3;		// old LUN
	U8				MSB:5;
	U16				BlockAddress;
	U8				Length;
	U8				Control;
} CDB6, *PCDB6;

typedef struct _Gen_Cmd_10 {
	U8				Cmd;
	U8				Reserved;
	U32				BlockAddress;
	U8				Res2;
	U16				Length;
	U8				Control;
} CDB10, *PCDB10;

typedef struct _Gen_Cmd_12 {
	U8				Cmd;
	U8				Reserved;
	U32				BlockAddress;
	U32				Length;
	U8				Res2;
	U8				Control;
} CDB12, *PCDB12;


typedef struct _Gen_Cmd_16 {
	U8				Cmd;
	U8				Reserved;
	U32				BlockAddress;
	U32				AdditionalLength;
	U32				Length;
	U8				Res2;
	U8				Control;
} CDB16, *PCDB16;

// Specific Commands
// Inquiry Command
typedef struct _Inquiry_Cmd {
	U8				Cmd;
	U8				Res:7;
	U8				EVPD:1;
	U8				PageCode;
	U8				Res2;
	U8				Length;					// Allocation length
	U8				Control;
} INQUIRY_CMD, *PINQUIRY_CMD;

// Vital Product Data Page Codes
#define	INQ_PAGE_CODE_SUPPORTED_PAGES			0x00
#define	INQ_PAGE_CODE_SERIAL_NUMBER				0x80
#define	INQ_PAGE_CODE_OPER_DEFINITIONS			0x81
#define	INQ_PAGE_CODE_ASCII_OPER_DEFINITIONS	0x82
#define	INQ_PAGE_CODE_DEVICE_IDENTIFICATION		0x83
// Vendor specific
#define	INQ_PAGE_CODE_FIRMWARE_NUMBERS			0xC0
#define	INQ_PAGE_CODE_DATE_CODE					0xC1
#define	INQ_PAGE_CODE_JUMPER_SETTINGS			0xC2
#define	INQ_PAGE_CODE_DEVICE_BEHAVIOR			0xC3		// not available on some

// Format Unit
typedef struct _Format_Unit {
	U8				Cmd;
	U8				Res1:3;
	U8				FmtData:1;				// format data
	U8				CmpList:1;				// complete list
	U8				DefectFmt:3;			// defect list format
	U8				VendorSpecific;
	U16				Interleave;
	U8				Control;
} FORMAT_UNIT, *PFORMAT_UNIT;

// Read Capacity
typedef struct _Read_Capacity {
	U8				Cmd;
	U8				Reserved:7;
	U8				RelAdr:1;
	U32				BlockAddress;
	U8				Res2[3];
	U8				Control;
} READ_CAP, *PREAD_CAP;

// Reserve and Release commands
typedef struct _Reserve_Release_6 {
	U8				Cmd;
	U8				Reserved:7;
	U8				Extent:1;
	U8				ResId;					// reservation identification
	U16				Length;
	U8				Control;
} RESERVE6, *PRESERVE6;

typedef struct _Reserve_10 {
	U8				Cmd;
	U8				Reserved:3;
	U8				ThirdPty:1;
	U8				Reserved1:2;
	U8				LongID:1;
	U8				Extent:1;
	U8				ResId;					// reservation identification
	U8				deviceId;				// third-party device identification
	U8				Reserved2[3];
	U16				Length;
	U8				Control;
} RESERVE10, *PRESERVE10;


// Mode Sense
typedef struct _Mode_Sense_6 {
	U8				Cmd;
	U8				Res1:4;
	U8				DBD:1;					// disable block descriptors
	U8				Res2:3;
	U8				PC:2;					// page control
	U8				PageCode:6;				// page code
	U8				Length;					// parameter list length
	U8				Control;
} MODE_SENSE6, *PMODE_SENSE6;

typedef struct _Mode_Sense_10 {
	U8				Cmd;
	U8				Res1:4;
	U8				DBD:1;					// disable block descriptors
	U8				Res2:3;
	U8				PC:2;					// page control
	U8				PageCode:6;				// page code
	U8				Res3[4];
	U16				Length;					// parameter list length
	U8				Control;
} MODE_SENSE10, *PMODE_SENSE10;

// Mode Select
typedef struct _Mode_Select_6 {
	U8				Cmd;
	U8				Res1:3;
	U8				PF:1;					// page format
	U8				Res2:3;
	U8				SP:1;					// save pages
	U8				Length;					// parameter list length
	U8				Control;
} MODE_SELECT6, *PMODE_SELECT6;

typedef struct _Mode_Select_10 {
	U8				Cmd;
	U8				Res1:3;
	U8				PF:1;					// page format
	U8				Res2:3;
	U8				SP:1;					// save pages
	U8				Res3[5];
	U16				Length;					// parameter list length
	U8				Control;
} MODE_SELECT10, *PMODE_SELECT10;

// Receive Diagnostic Results
typedef struct _Receive_Diag {
	U8				Cmd;
	U8				Reserved[2];
	U16				Length;					// Allocation Length
	U8				Control;
} RECEIVE_DIAG, *PRECEIVE_DIAG;

// Send Diagnostic
typedef struct _Send_Diag {
	U8				Cmd;
	U8				Res1:3;
	U8				PF:1;					// page format
	U8				Res2:1;
	U8				SelfTest:1;
	U8				DevOfL:1;				// device off-line
	U8				UnitOfL:1;				// unit off-line
	U8				Reserved;
	U16				Length;
	U8				Control;
} SEND_DIAG, *PSEND_DIAG;




// Return data Structures

// Inquiry structure
typedef struct _Inquiry {
	U8				qual:3;
	U8				type:5;					// Peripheral device type
	U8				RMB:1;					// Removable
	U8				Res1:7;					// reserved
	U8				Version;				// Versions
	U8				AERC:1;
	U8				TrmTsk:1;
	U8				NormACA:1;
	U8				HiSupport:1;			// New for T10/1236-D Rev 2a
	U8				RespFmt:4;
	U8				AdditionalLength;		// Additional length following
	U8				SCCS:1;					// New for T10/1236-D Rev 2a
	U8				Res2:7;					// reserved
	U8				BQue:1;					// New for T10/1236-D Rev 2a
	U8				EncServ:1;
	U8				VS:1;
	U8				MultiP:1;
	U8				MChngr:1;
	U8				ACKREQQ:1;
	U8				Addr32:1;
	U8				Addr16:1;
	U8				RelAdr:1;
	U8				WBus32:1;
	U8				WBus16:1;
	U8				Sync:1;
	U8				Linked:1;
	U8				TranDis:1;
	U8				CmdQue:1;
	U8				VS2:1;
	U8				VendorId[8];			// Vendor identification
	U8				ProductId[16];			// Product identification
	U8				ProductRevision[4];		// Product revision level
	U8				VendorSpecific[20];		// Probably not used (drive Serial Number)
//	U8				Res3[40];				// reserved
} INQUIRY, *PINQUIRY;

// Peripheral device type
#define SCSI_DEVICE_TYPE_DIRECT         0x00
#define SCSI_DEVICE_TYPE_SEQUENTIAL     0x01
#define SCSI_DEVICE_TYPE_PRINTER        0x02
#define SCSI_DEVICE_TYPE_PROCESSOR      0x03
#define SCSI_DEVICE_TYPE_WORM           0x04
#define SCSI_DEVICE_TYPE_CDROM          0x05
#define SCSI_DEVICE_TYPE_SCANNER        0x06
#define SCSI_DEVICE_TYPE_OPTICAL        0x07
#define SCSI_DEVICE_TYPE_MEDIA_CHANGER  0x08
#define SCSI_DEVICE_TYPE_COMM           0x09
#define SCSI_DEVICE_GRAPHICS_1          0x0A
#define SCSI_DEVICE_GRAPHICS_2          0x0B
#define SCSI_DEVICE_TYPE_ARRAY_CONT     0x0C
#define SCSI_DEVICE_TYPE_ENCLOSURE_SERVICES     0x0D
#define SCSI_DEVICE_TYPE_UNKNOWN        0x1F

#define	INQUIRY_ADDITIONAL_LENGTH			51
#define	INQUIRY_VENDOR_ID					"DELL    "
#define	INQUIRY_PRODUCT_ID					"PV SDM         "
#define	INQUIRY_REVISION_LEVEL				"0.20"

// Scsi Extended Inquiry Supported Vital product pages header
typedef struct _SUPPORTED_PAGES {
	U8				DeviceType;
	U8				PageCode;
	U8				Res1;
	U8				PageLength;
} SUPPORTED_PAGES;

// Scsi Extended Inquiry Unit Serial Number Page
typedef struct _INQUIRY_SERIAL_NUMBER {
	U8				DeviceType;
	U8				PageCode;
	U8				Res1;
	U8				PageLength;
	U8				ProductSerialNumber[8];		// ASCII data
	U8				BoardSerialNumber[6];
} INQUIRY_SERIAL_NUMBER;

// Scsi Extended Inquiry Device Identification Page header
typedef struct _DEVICE_ID_HEADER {
	U8				DeviceType;
	U8				PageCode;
	U8				Res1;
	U8				PageLength;
} DEVICE_ID_HEADER;

// Device Identification Descriptor
typedef struct _DEVICE_ID_DESCRIPTOR {
	U8				Res1:4;
	U8				CodeSet:4;		// should be 1 for us
	U8				Res2:2;
	U8				Association:2;	// 0 = phys or logical device, 1 = port
	U8				IdType:4;
	U8				Res3;
	U8				DescLength;
} DEVICE_ID_DESCRIPTOR;

#define	DEVICE_CODE_SET				1		// Binary (always)

#define	DEVICE_ASSO_LOGICAL			0		// Physical or Logical device
#define	DEVICE_ASSO_PORT			1		// Port

#define	DEVICE_ID_TYPE_NO_AUTH		0		// No assignment authority
#define	DEVICE_ID_TYPE_VENDOR_ID	1		// SCSI Vendor ID
#define	DEVICE_ID_TYPE_EUI_64		2		// IEEE Extended Unique (EUI-64)
#define	DEVICE_ID_TYPE_IEEE_WWN		3		// Any FC-PH Identifier (WWW)


// Scsi Extended Inquiry Device Identification Page
typedef struct _INQUIRY_DEVICE_ID {
	DEVICE_ID_HEADER		Header;
	DEVICE_ID_DESCRIPTOR	Descriptor;		// WWN follows
} INQUIRY_DEVICE_ID;

// Read Capacity structure
typedef struct _Read_Cap {
	U32				BlockAddress;
	U32				BlockLength;
} READ_CAPACITY, *PREAD_CAPACITY;


// Mode Page Definitions

// Page Codes for direct access devices
// Not all of these pages will be supported by Odyssey
typedef enum
{
	RETURN_ALL_PAGES					= 0x3f,		// required if any mode support
	RW_ERROR_RECOVERY					= 0x01,
	DISCONNECT_RECONNECT				= 0x02,
	FORMAT_DEVICE						= 0x03,
	RIGID_DISK_GEOMETRY					= 0x04,
	VERIFY_ERROR_RECOVERY				= 0x07,
	CACHING								= 0x08,
	CONTROL_MODE						= 0x0a
} PAGE_CODE;

// values for the Page Control (PC) field of the mode sense command
typedef enum
{
	CURRENT_VALUES						= 0,
	CHANGEABLE_VALUES					= 1,
	DEFAULT_VALUES						= 2,
	SAVED_VALUES						= 3
} MODE_PC;

// Mode Parameter Header (6)
typedef struct _Mode_Header6 {
	U8				Length;
	U8				MediumType;
	U8				DeviceSpecific;
	U8				BlockDescLength;
} MODE_HEADER6, *PMODE_HEADER;

// Mode Parameter Header (10)
typedef struct _Mode_Header10 {
	U16				Length;
	U8				MediumType;
	U8				DeviceSpecific;
	U8				Reserved[2];
	U16				BlockDescLength;
} MODE_HEADER10, *PMODE_HEADER10;

// Block Descriptor
typedef struct _Block_Desc {
	U8				Density;
	U8				NumBlocks[3];
	U8				Reserved;
	U8				BlockLength[3];
} BLOCK_DESC, *PBLOCK_DESC;

typedef struct _Mode_Format {
	U8				PS:1;					// Parameters saveable
	U8				Reserved:1;
	PAGE_CODE		PageCode:6;
	U8				PageLength;
} MODE_FORMAT, *PMODE_FORMAT;

// Mode Page Data Structures
#define	PAGE1_LEN		10
#define	PAGE2_LEN		14
#define PAGE3_LEN		22
#define PAGE4_LEN		22

// Mode Page 1 - Error Recovery parameters
typedef struct _Mode_Page_1 {
	MODE_FORMAT		Page1;
	U8				AWRE:1;
	U8				ARRE:1;
	U8				TB:1;
	U8				RC:1;
	U8				ERR1:1;
	U8				PER:1;
	U8				DTE:1;
	U8				DCR:1;
	U8				RdRetry;
	U8				Span;
	U8				HeadOffset;
	U8				DataStrbOffset;
	U8				Res1;
	U8				WrRetry;
	U8				Res2;
	U16				RecoveryTimeLimit;
} MODE_PAGE_1, *PMODE_PAGE_1;

// Mode Page 2 - Disconnect/Reconnect Control
typedef struct _Mode_Page_2 {
	MODE_FORMAT		Page2;
	U8				BufferFullRatio;
	U8				BufferEmptyRatio;
	U16				BusInactivityLimit;
	U16				DisconnectTimeLimit;
	U16				ConnectTimeLimit;
	U16				MaxBurst;
	U8				EMDP:1;
	U8				Res:7;
	U8				Res2[3];
} MODE_PAGE_2, *PMODE_PAGE_2;

// Mode Page 3 - Format Device parameters
typedef struct _Mode_Page_3 {
	MODE_FORMAT		Page3;
	U16				TracksPerZone;
	U16				AltSectPerZone;
	U16				AltTracksPerZone;
	U16				AltTracksPerLUN;
	U16				SectorsPerTrack;
	U16				BytesPerPhysSect;
	U16				Interleave;
	U16				TrackSkew;
	U16				CylSkew;
	U8				SSEC:1;
	U8				HSECT:1;
	U8				RMB:1;
	U8				SURF:1;
	U8				Res1:4;
	U8				Res2[3];
} MODE_PAGE_3, *PMODE_PAGE_3;

// Mode Page 4 - Rigid Disk Drive Geometry parameters
typedef struct _Mode_Page_4 {
	MODE_FORMAT		Page4;
	U8				NumberofCylinders[3];
	U8				NumberofHeads;
	U8				WrPrecompStartCyl[3];
	U8				ReducedWrCurrentStartCyl[3];
	U16				DriveStepRate;
	U8				LandingZoneCyl[3];
	U8				Res1:6;
	U8				RPL:2;
	U8				RotatinalOffset;
	U8				Res2;
	U16				MediumRotationRate;
	U8				Res3[2];
} MODE_PAGE_4, *PMODE_PAGE_4;


// Report Luns
typedef struct _Report_Luns {
	U32				Length;
	U32				Reserved;
} REPORT_LUNS, *PREPORT_LUNS;

// Request Sense Data
typedef struct _Request_Sense {
	U8				ResponseCode;
	U8				Segment;
	U8				SenseKey;				// other bits here too, all 0
	U8				Information[4];
	U8				AdditionalLength;
	U8				CmdSpecificInfo[4];
	U16				ASC_ASCQ;
	U8				Fru;
	U8				SenseKeySpecific[3];
	U8				AdditionalSense[1];
} REQUEST_SENSE, *PREQUEST_SENSE;

#define	RESPONSE_CODE		0x70			// Current errors
#define	ADDITIONAL_LENGTH	11				// num bytes after "AdditionalLength"

// SCSI Sense Key Defines
typedef enum
{
	SENSE_NO_SENSE			= 0,
	SENSE_RECOVERED_ERROR	= 1,
	SENSE_NOT_READY			= 2,
	SENSE_MEDIUM_ERROR		= 3,
	SENSE_HARDWARE_ERROR	= 4,
	SENSE_ILLEGAL_REQUEST	= 5,
	SENSE_UNIT_ATTENTION	= 6,
	SENSE_DATA_PROTECT		= 7,
	SENSE_BLANK_CHECK		= 8,
	SENSE_VENDOR_SPECIFIC	= 9,
	SENSE_COPY_ABORTED		= 0xA,
	SENSE_ABORTED_COMMAND	= 0xB,
	SENSE_EQUAL				= 0xC,
	SENSE_VOLUME_OVERFLOW	= 0xD,
	SENSE_MISCOMPARE		= 0xE,
	SENSE_RESERVED			= 0xF
} SENSE_KEY;
	
// Extended Request Sense Codes used by Odyssey.
// Since not all the possible codes are used,
// these codes are in ASC/ASCQ pairs with ASC in the high order
// and ASCQ in the low order.  More of these may added later.
typedef enum
{
	ASC_NO_ADDITIONAL_SENSE										= 0x0000,
	ASC_IO_PROCESS_TERMINATED									= 0x0006,
	ASC_INQUIRY_DATA_HAS_CHANGED								= 0x3F03,
	ACS_INTERNAL_TARGET_FAILURE									= 0x4400,
	ASC_SYSTEM_RESOURCE_FAILURE									= 0x5500,
	ASC_SYSTEM_BUFFER_FULL										= 0x5501,
	ASC_INVALID_COMMAND_OPERATION_CODE							= 0x2000,
	ASC_INVALID_FIELD_IN_CDB									= 0x2400,
	ASC_INVALID_FIELD_IN_PARAMETER_LIST							= 0x2600,
	ASC_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE						= 0x2100,
	ASC_LOGICAL_UNIT_COMMUNICATION_FAILURE						= 0x0800,
	ASC_LOGICAL_UNIT_COMMUNICATION_PARITY_ERROR					= 0x0802,
	ASC_LOGICAL_UNIT_COMMUNICATION_TIMEOUT						= 0x0801,
	ASC_LOGICAL_UNIT_DOES_NOT_RESPOND_TO_SELECTION				= 0x0500,
	ASC_LOGICAL_UNIT_FAILURED_SELF_CONFIGURATION				= 0x4C00,
	ASC_LOGICAL_UNIT_SELF_CONFIGURATION_NOT_COMPLETE			= 0x3E00,
	ASC_LOGICAL_UNIT_FAILURE									= 0x3E01,
	ASC_LOGICAL_UNIT_TIMEOUT									= 0x3E02,
	ASC_LOGICAL_UNIT_BECOMING_READY								= 0x0401,
	ASC_LOGICAL_UNIT_NOT_READY_CAUSE_NOT_REPORTABLE				= 0x0400,
	ASC_LOGICAL_UNIT_NOT_READY_FORMAT_IN_PROGRESS				= 0x0404,
	ASC_LOGICAL_UNIT_NOT_READY_INITIALIZING_COMMAND_REQUIRED	= 0x0402,
	ASC_LOGICAL_UNIT_NOT_READY_MANUAL_INTERVENTION_REQUIRED		= 0x0403,
	ASC_LOGICAL_UNIT_NOT_SUPPORTED								= 0x2500,
	ASC_PARAMETER_LIST_LENGTH_ERROR								= 0x1A00,
	ASC_PARAMETER_NOT_SUPPORTED									= 0x2601,
	ASC_PARAMETER_VALUE_INVALID									= 0x2602,
	ASC_PARAMETERS_CHANGED										= 0x2A00,
	ASC_POWER_ON_OR_DEVICE_RESET_OCCURED						= 0x2900,
	ASC_POWER_ON_OCCURED										= 0x2901,
	ASC_SCSI_BUS_RESET_OCCURED									= 0x2902,
	ASC_BUS_DEVICE_RESET_OCCURED								= 0x2903,
	ASC_UNSUCCESSFUL_SOFT_RESET									= 0x4600
} ASC_ASCQ_Pairs;
	

//    SCSI Status values
typedef enum
{
	SCSI_STATUS_GOOD					= 0x00,
	SCSI_STATUS_CHECK					= 0x02,
	SCSI_STATUS_BUSY					= 0x08,
	SCSI_STATUS_RESERVATION_CONFLICT	= 0x18,
	SCSI_STATUS_COMMAND_TERMINATED		= 0x22,
	SCSI_STATUS_TASK_SET_FULL			= 0x28,
	SCSI_STATUS_ACA_ACTIVE				= 0x30
} SCSI_STATUS;

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif /* SCSI_H  */
