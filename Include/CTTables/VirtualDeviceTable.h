/* VirtualDeviceTable.h -- PTS VirtualDeviceTable Definitions
 *
 * Copyright (C) ConvergeNet Technologies, 1999
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
**/
 
// Revision History: 
//  8/06/99 Jerry Lane: Create file
//  ** Log at end-of-file **
//

#ifndef __VirtualDeviceTable_h
#define __VirtualDeviceTable_h

#include "RqPts_T.h"
#include "PtsRecordBase.h"

#pragma	pack(4)

#define PTS_VIRTUAL_DEVICE_TABLE 			"VirtualDeviceTable"
#define	PTS_VIRTUAL_DEVICE_TABLE_VERSION	1

// Virtual Device Attributes (Persistant)
enum EnumVDAttrs {
	eVDAttrMask_None		= 0x0,
	eVDAttrMask_AutoStart	= 0x00400000,	// AutoStart flag
	eVDAttrMask_BuildSys	= 0x00800000	// Defined by BuildSys
};

// Virtual Device Flags (Not Persistant)
// Initial flags must be all zeros.
enum EnumVDFlags {
	eVDFlagMask_None 		  = 0x0,
	eVDFlagMask_Alternate	  = 0x00000001,
	eVDFlagMask_HasPrimary	  = 0x00000002,	// didPrimary is set
	eVDFlagMask_HasSecondary  = 0x00000004,	// didSecondary is set
	eVdFlagMask_Instanciated  = 0x00000010,	// Device(s) instanciated
	eVDFlagMask_RoutesComplete= 0x00000020,	// All Slots saw this device
	eVDFlagMask_Delete		  = 0x00000080	// Request Device Delete
};

class VirtualDeviceRecord : public CPtsRecordBase {
private:
	static const fieldDef rgFieldDefs[];
	static const U32      cbFieldDefs;

	// Non-Persistant fields must be initialized to zero
	void Initialize(const char *_pszClassName,TySlot _slotP,TySlot _slotS, BOOL _fStart,RowId _ridCfg,RowId _ridVDOwnerUse) {
	   	strncpy(szClassName,(char*)_pszClassName,sCLASSNAME);
	   	szClassName[sCLASSNAME-1] = EOS;
	   	
		slotPrimary = _slotP;
		slotSecondary = _slotS;
		ridDdmCfgRec = _ridCfg;
		ridVDOwnerUse = _ridVDOwnerUse;
 		eVDAttrs = (EnumVDAttrs) (_fStart ? eVDAttrMask_AutoStart : eVDAttrMask_None);

		eVDFlags = eVDFlagMask_None;
 		fIOPHasDID = 0;
		nServes = 0;
		didPrimary = DIDNULL;
		didSecondary = DIDNULL;
		didActive = DIDNULL;

		key.Set(szClassName,ridDdmCfgRec,ridVDOwnerUse);
	}
	
	// Key to catch PTS duplicates.
	// Allows:
	//		Each class may have one VD with no config record if no ridVDOwnerUse.
	//		Each class may have multiple VD with no config if ridVDOwnerUse is specified
	//		
	class Key {
		RowId			ridDdmCfgRec;
		RowId			ridVDOwnerUse;
		String32		szClassName;
	public:	
		Key() {
			Set("",0,0);
		}
		Key(char *_pszClassName,RowId _ridDdmCfgRec,RowId _ridVDOwnerUse) {
			Set(_pszClassName,_ridDdmCfgRec,_ridVDOwnerUse);
		}
		
		void Set(char *_pszClassName,RowId _ridDdmCfgRec,RowId _ridVDOwnerUse) {
			strncpy(szClassName,_pszClassName,sCLASSNAME);
			ridDdmCfgRec = _ridDdmCfgRec;
			ridVDOwnerUse = _ridVDOwnerUse;
		}
	};
	
public:
	static const  fieldDef *FieldDefs()		{ return rgFieldDefs;}
	static const U32 FieldDefsSize() 		{ return cbFieldDefs;  }
	static const U32 FieldDefsTotal()		{ return FieldDefsSize() / sizeof(fieldDef); }
	static const char *TableName()			{ return PTS_VIRTUAL_DEVICE_TABLE; }

	// Persistant fields
	RowId			ridVDOwnerUse;		// For internal use by VD Owner/Creator. ** KEY FIELD **
	String32		szClassName;		// Class type name of the VD.
	TySlot			slotPrimary;		// Slot of primary device
	TySlot			slotSecondary;		// Slot of secondary device
	RowId			ridDdmCfgRec;		// Configuration data for the VD.
	EnumVDAttrs  	eVDAttrs;			// Attributes of the Virtual Device
	U32				nServes;			// Number of Virtual Serves

	// Non-Persistant fields
	EnumVDFlags  	eVDFlags;			// Attributes of the Virtual Device
	U32				fIOPHasDID;			// Flag array indicating IOPs Know of DID.
	DID				didPrimary;			// DID of Primary DDM.
	DID				didSecondary;		// DID of secondary DDM.
	DID 			didActive;			// DID of current active DDM

	// Persistant Non-Duplicate Key
	Key				key;				// Duplicate Key
	
	// Constructors
#define VDT_DEFAULT_CTOR
#ifdef VDT_DEFAULT_CTOR
	VirtualDeviceRecord() 
	: CPtsRecordBase(sizeof(VirtualDeviceRecord),PTS_VIRTUAL_DEVICE_TABLE_VERSION) {
		Initialize("",IOP_NONE,IOP_NONE,FALSE,0,0);
	}
#endif
	
	// Sets all fields
	VirtualDeviceRecord(const char *_pszClassName,TySlot _slotP,TySlot _slotS, BOOL fStart=0, RowId _ridCfg=0,RowId _ridVDOwnerUse=0) 
 	: CPtsRecordBase(sizeof(VirtualDeviceRecord),PTS_VIRTUAL_DEVICE_TABLE_VERSION) {
	   	Initialize(_pszClassName,_slotP,_slotS,fStart,_ridCfg,_ridVDOwnerUse);
	}

	// PTS Interface Classes.  EG: VirtualDeviceRecord::RqDefineTable *pMsg;
	typedef RqPtsDefineTable_T<VirtualDeviceRecord> RqDefineTable;
	typedef RqPtsDeleteTable_T<VirtualDeviceRecord> RqDeleteTable;
	typedef RqPtsGetTableDef_T<VirtualDeviceRecord> RqGetTableDef;
	typedef RqPtsQuerySetRID_T<VirtualDeviceRecord> RqQuerySetRID;
	typedef RqPtsEnumerateTable_T<VirtualDeviceRecord> RqEnumerateTable;
	typedef RqPtsInsertRow_T<VirtualDeviceRecord> RqInsertRow;
	typedef RqPtsModifyRow_T<VirtualDeviceRecord> RqModifyRow;
	typedef RqPtsModifyField_T<VirtualDeviceRecord> RqModifyField;
	typedef RqPtsModifyBits_T<VirtualDeviceRecord> RqModifyBits;
	typedef RqPtsTestAndSetField_T<VirtualDeviceRecord> RqTestAndSetField;
	typedef RqPtsReadRow_T<VirtualDeviceRecord> RqReadRow;
	typedef RqPtsDeleteRow_T<VirtualDeviceRecord> RqDeleteRow;
	typedef RqPtsListen_T<VirtualDeviceRecord> RqListen;
};

// Jerry's typedefs
typedef VirtualDeviceRecord VirtualDeviceTable[];
typedef VirtualDeviceRecord Vdef;
typedef VirtualDeviceRecord **pPVdef;

// Persistant Field Names
#define VDT_RID_VDOWNERUSE_FIELD	"ridVDOwnerUse"
#define VDT_CLASSNAME_FIELD			"szClassName"
#define	VDT_SLOT_PRIMARY_FIELD		"slotPrimary"
#define	VDT_SLOT_SECONDARY_FIELD	"slotSecondary"
#define VDT_RID_DDM_CFG_REC			"ridDdmCfgRec"
#define VDT_ATTRS_FIELD				"eVDAttrs"
#define VDT_NSERVES_FIELD			"nServes"
#define VDT_KEY						"key"

// Non-Persistant Field Names
#define VDT_FLAGS_FIELD				"eVDFlags"
#define VDT_FIOPHASDID_FIELD		"fIOPHasDID"
#define	VDT_DID_PRIMARY_FIELD		"didPrimary"
#define	VDT_DID_SECONDARY_FIELD		"didSecondary"
#define VDT_DID_ACTIVE_FIELD		"didActive"

#ifdef OBSOLETE
#define VDT_STATE_DESIRED_FIELD		"eVdStateDesired"
#define VDT_STATE_ACTUAL_FIELD		"eVdStateActual"
#define VDT_EVD_DESIRED_STATE_FIELD	"eVDStateDesired"
#define VDT_EVD_ACTUAL_STATE_FIELD	"eVDStateActual"
#define VDT_FIOPHASVDR_FIELD			"fIOPHasVdr"
#endif

#endif


/*************************************************************************/
// Update Log:
//	$Log: /Gemini/Include/CTTables/VirtualDeviceTable.h $
// 
// 30    2/15/00 6:04p Tnelson
// Fixed invalid flag value
// 
// 29    12/09/99 1:28a Iowa
// 
// 28    11/04/99 4:37p Jlane
// Roll in Tom's changes
// 
// 27    10/14/99 3:49a Iowa
// Iowa merge
// 
// 25    9/16/99 4:00p Tnelson
// 
// 24    9/14/99 7:39p Jlane
// Add eVDState_Deleted
// 
// 23    9/03/99 3:13p Tnelson
// 
// 22    9/02/99 3:40p Jlane
// Added eVDState_Deleted.
// 
// 21    9/02/99 3:38p Jlane
// Added #defines for desired and actual state fields.
// 
// 20    8/26/99 3:46p Tnelson
// Latest and Greatest!
// 
// 19    8/26/99 3:28p Tnelson
// Added remaining Pts requests
// 
// 18    8/26/99 2:09p Tnelson
// Couple of fixes
// 
// 16    8/26/99 2:04p Tnelson
// Moved PTS Request Classes into VirtualDeviceRecord as per Eric's
// request
// 
// 15    8/20/99 3:12p Tnelson
// Remove #defines from previous check-in since it will not solve the
// externs for those names
// 
// 13    8/20/99 3:10p Tnelson
// Add #define for VirtualDeviceTable_FieldDefs and
// cbVirtualDeviceTable_FieldDefs for compatibility with old code
// 
// 12    8/20/99 2:59p Tnelson
// Move rgFieldDefs[] and cbFieldDefs into VirtualDeviceRecord as statics
// 
// 11    8/19/99 6:32p Tnelson
// 
// 10    8/19/99 6:31p Tnelson
// 
// 9     8/19/99 6:27p Tnelson
// 
// 8     8/17/99 12:45p Tnelson
// 
// 7     8/17/99 12:44p Tnelson
// Moved SourceSafe log to end of file
// 
// 6     8/17/99 12:41p Tnelson
// No, I mean renamed .didStandby to .didSecondary
// 
// 5     8/17/99 12:40p Tnelson
// Renamed .didAlternate to .didSecondary
// 
// 4     8/14/99 7:39p Tnelson
// Make VirtualDeviceTable_FieldDefs[] use the #define field names that
// are in VirtualDeviceTable.h.
// VirtualDeviceTable_FieldDefs[] was missing fields that were defined
// in the VirtualDeviceRecord struct
// 
// 3     8/14/99 6:08p Tnelson
// Added static access methods the VirtualDeviceRecord
// 
// 2     8/08/99 11:22a Jlane
// Compile fixes - case errors etc.
// 
// 1     8/06/99 10:48a Jlane
// Initial Checkin.
// 
