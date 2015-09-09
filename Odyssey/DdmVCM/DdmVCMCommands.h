/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: DdmVCMCommands.h
// 
// Description:
//    Virtual Circuit Manager DDM control interface definitions.
//    These are the commands submitted to the VCM DDM by other
//    DDMs in the Odyssey system.
//
//    Commands are submitted via class CmdSender, see include\CmdSender.h
//    and friends for the particulars.
// 
// $Log: /Gemini/Include/CmdQueues/DdmVCMCommands.h $
// 
// 10    12/17/99 5:26p Dpatel
// added vc modify
// 
// 9     9/06/99 8:03p Jlane
// Yet another interim checkin.
// 
// 8     9/06/99 1:42p Jlane
// Changes to status structure to support event handling by identifying
// the action resulting in the event.
// 
// 7     9/05/99 4:40p Jlane
// Compiles and is theoretically ready to try.
// 
// 6     8/12/99 11:47a Jlane
// Changes with Andrey to add lergacy support.  What about tape and SES?
// 
// 5     8/08/99 12:20p Jlane
// Interim checkin upon compiling
// 
// 4     8/05/99 11:36a Jlane
// Interim checkin to share the code.
// 
// 3     7/26/99 1:22p Jlane
// INterim checkin before rewrite supporting new HostConnectionDescriptor.
// 
// 2     7/26/99 1:22p Jlane
// INterim checkin before rewrite supporting new HostConnectionDescriptor.
// 
// 1     7/21/99 5:53p Jlane
// Initial Checkin of as yet unfinished work.
// 
// 
/*************************************************************************/

#ifndef _DdmVcmCommands_h_
#define _DdmVcmCommands_h_

#ifndef __Address_h
# include  "Address.h"        // for TySlot
#endif

#ifndef Simple_H
# include  "Simple.h"         // for BOOL, U32, etc.
#endif

#ifndef CTtypes_H
# include  "CtTypes.h"        // for rowID
#endif

#ifndef __ExportTable_h
# include  "ExportTable.h"    // for Export Table Entry
#endif



//  common VCM control queue name root
#define     VCM_CONTROL_QUEUE    "VCMControlQueue"

//  size of VCM control queue's command messages
#define     VCM_CONTROL_COMMAND_SIZE   (sizeof (VCRequest))

//  size of VCM control queue's status messages (unused)
#define     VCM_CONTROL_STATUS_SIZE    (sizeof (VCStatus))


//  here are the supported VCM command codes:
typedef enum  {
	k_eCreateVC = 1,		// Params are VCMCmdCreateVC.
	k_eDeleteVC,			// Params are VCMCmdDeleteVC.
	k_eVCExportCtl,			// Params are VCMCmdVCExportCtl.
	k_eModifyVC,
	k_eVCQuiesceCtl,		// Params are VCMcmdVCQuiesceCtl.
	K_eNextVCMCommand		// Still unused.
} VCCommand;


//  VCCreateCommand - Use to Create a new Virtual Circuit.
class VCCreateCommand
{
public:
	rowID		ridSRCElement;			// Row ID of Storage Roll Call Element to export.
	rowID		ridHCDR;				// Row ID of Host Connection Descriptor Record.
	rowID		ridUserInfo;			// Row ID of User INfo record.  Gets put in export table.
	U32			TargetID;				// Export TargetID for this Virtual Circuit.
	U32			LUN;					// Export LUN for this Virtual Circuit.
	U32			fExport;				// Export the new Virtual Circuit immediately?
	U32			fExport2AllInitiators;	// All Initiators are allowed to see this Virtual Circuit.
	
};


//  VCCreateResult - Returned results from teh Virtual Circuit Create Command.
class VCCreateResult
{
public:
	ExportTableEntry	VCExportRecRet[32];	// Returned export records for VC.
};


//  VCDeleteCommand - Use to Delete a new Virtual Circuit.
class VCDeleteCommand
{
public:
	rowID		ridVcId;		// Virtual Circuit ID.
};


//  VCExportCtlCommand - Status Record returned from Delete a Virtual Circuit.
class VCExportCtlCommand
{
public:
	U32			fAllVCs;		// Apply export command to All Virtual Circuits?
	rowID		ridVcId;		// Virtual Circuit ID.
	U32			fExport;		// True -> Export.  False -> UnExport
};

//  VCQuiesceCtlCommand - Use to quiesce a Virtual Circuit.
class VCQuiesceCtlCommand
{
public:
	U32			fAllVCs;		// Apply export command to All Virtual Circuits?
	rowID		ridVcId;		// Virtual Circuit ID.
	U32			fQuiesce;		// True -> Quiesce.  False -> UnQuiesce.
};


//  VCModifyCtlCommand - Use to modify a Virtual Circuit.
class VCModifyCtlCommand
{
public:
	rowID		ridVcId;		// Virtual Circuit ID.
	VDN			vdNext;			// the new downstream vd
	rowID		srcNext;		// the new downstream src
};

//  Here's our master command parameter struct.  This struct is used
//  both for command and status transfers.  We have no separate
//  "results" struct, we simply loop back the full command packet.
// NOte that this Struct will be returned in the status structure.
typedef struct {
	rowID		rid;      		// By convention all PTS Tables start with rid, version and size.
	U32			version;		//
	U32			size;			//
	VCCommand	eCommand;  		// what this request is for -- "tags" member (u)
	
	//  and a nice little union, "tagged" by eCommand.
	//  This union lets us use the size of our overall struct to define
	//  our maximum "queue struct" parameter size by a simple sizeof().
	union {
	  VCCreateCommand		VCCreateParms;
	  VCDeleteCommand		VCDeleteParms;
	  VCExportCtlCommand	VCExportParms;
	  VCQuiesceCtlCommand	VCQuiesceParms;
	  VCModifyCtlCommand	VCModifyParms;
	} u;
	
}  VCRequest;

// Event enum used in VC Status and event replies.
enum VCEvtEnum{
	VCCreated,					// 
	VCDeleted,
	VCQuiesced,
	VCUnQuiesced,
	VCExported,
	VCUnExported,
	VCModified
};


//  VCStatus - Status Record returned from all Virtual Circuit commands.
typedef struct {
public:
	rowID		rid;      		// By convention all PTS Tables start with rid, version and size.
	U32			version;		// version of this struct.
	U32			size;			// size of this struct.
	rowID		ridVcId;		// VC Identifier
	VCEvtEnum	eVCEvent;		// Event code
	STATUS		status;			// The status code returned from the command.
	
	//  This union lets us use the size of our overall struct to define
	//  our maximum "queue struct" parameter size by a simple sizeof().
	union {
		STATUS			detailedstatus;	// Possible secondary status code.
		VCCreateResult	VCCreateResults;
	} u;
}  VCStatus;


	
#endif   // #ifndef _CmbDdmCommands_h_


