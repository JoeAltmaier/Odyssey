/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This is the Odyssey Storage Box header file. This includes all system
// global structs, defines and enums.
// 
// Update Log:
//	$Log: /Gemini/Include/Odyssey.h $
// 
// 12    9/16/99 4:27p Mpanas
// need one more DriveStatus value
// 
// 11    9/10/99 7:10p Mpanas
// New CTDiskType to specify where storage is.
// 
// 10    7/22/99 11:17a Jlane
// Readded StateInvalid.
// 
// 9     7/21/99 3:05p Jlane
// Updated CTReadyState enum values per Mike Panas.  Fixed arroneous
// comment.
// 
// 8     7/21/99 2:57p Jlane
// Updated CTReadyState enum values per Mike Panas.
// 
// 7     7/14/99 12:17p Jlane
// Added CTReadyState::Quiesced.
// 
// 6     5/28/99 4:53p Mpanas
// Add fields used by LoopDescriptor Table
//
// 9/9/98 Michael G. Panas: Create file
// 9/30/98 Michael G. Panas: Move all the Max values to this file, add RAID
//          storage types and states.
// 11/13/98 Michael G. Panas: Lower the number of LUNS per initiator to 256,
//          this should be enough for everyone.
// 01/11/99 Michael G. Panas: Add status "Invalid" to DriveStatus and
//          CTReadystate enums.
/*************************************************************************/

#ifndef __Odyssey_h
#define __Odyssey_h

// Systemwide defines
#define	MAX_SERIAL_NUMBER_LENGTH		32
#define	MAX_TABLE_NAME_LENGTH			64

// number of drives scanned on the RAC
#define	MAX_DRIVES						20

// max number of Fibre Channel target IDs
#define	MAX_FC_IDS						126

// Define the maximum number of initiators we will keep track of
#define FCP_INITIATOR_MAX				8

// Define the maximum number of LUNs that we will keep track of
#define FCP_LUNS_MAX					256



// Typedef to specify an Odyssey Persistant Data Store Table Name
struct TableName {
	U8		Name[MAX_TABLE_NAME_LENGTH];
};

// Systemwide enum defs

// Disk Type used for the Descriptors
enum CTDiskType {
	TypeUnknown = 0,		// don't know where
	TypeFCDisk,				// Internal Odyssey Fibre disk
	TypeExternalFCDisk,		// External Fibre disk
	TypeSCSIDisk,			// Internal Odyssey SCSI disk
	TypeExternalSCSIDisk	// External SCSI disk
};

// Used to specify the status of a Disk Drive
enum DriveStatus {
	DriveInvalid = 0,
	DriveFound,
	DriveNotSpinning,
	DriveSpinningUp,
	DriveReady,
	DriveRemoved,
	DriveNotPresent,
	DriveHardFailure };

// Odyssey Protocol types
enum CTProtocolType {
	ProtocolFibreChannel,
	ProtocolIP
};

// Specifies the state of a Partition, LUN or Storage Device
enum CTReadyState {
	StateInvalid = 0,			// Requests replied to with No Such LUN.
	StateConfiguring,			// Requests replied to with No Such LUN.
	StateConfigured,			// Internal state.  Requests replied to with No Such LUN.
	StateConfiguredNotExported,	// Configure but not exported.  Reqs replied to with No Such LUN.
	StateConfiguredAndExporting,// Internal State Requests replied to with No Such LUN. 
	StateConfiguredAndExported,	// Configured but not active.  Requests deactivate (ie flush) 
								// the alternate path (if active) and activate this one
	StateConfiguredAndActive,	// Configured & active.  Requests are dispatched.
	StateOffLine,				// Requests replied to with error.
	StateQuiesced				// Requests replied to with busy.
};

// Storage types
enum CTStorageType {
	TyMirror,
	TyStripe,
	TyRaid5,
	TyConcat,
	TyDisk
};

// LoopState 
enum LoopState {
	LoopInvalid = 0,
	LoopUp,
	LoopDown,
	LoopQuiesce
};

#endif

