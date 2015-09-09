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
// This is the definition and declaration of the Card Status Record.
// 
// $Log: /Gemini/Include/CTTables/IOPStatusTable.h $
// 
// 20    12/14/99 8:02p Ewedel
// Added new state IOPS_UPDATING_BOOT_ROM, to stay in sync with low-level
// CMB definitions.
// 
// 19    11/21/99 4:22p Jlane
// Added IopState IOPS_QUIESCING  foruse by  BootMgr/ QuiesceMaster.
// 
// 18    10/08/99 11:56a Ewedel
// Added new IOPS_SUSPENDED state, and mirror states for CMB "image
// corrupt" and "insufficient RAM" states.
// 
// 17    9/15/99 1:27p Ewedel
// Updated IopState values to reflect results from Jaymie's upgrade / IOP
// meeting.
// 
// 16    9/14/99 2:35p Tnelson
// Added remaining PTS interface typedefs
// 
// 15    9/08/99 9:10a Vnguyen
// Change temperature threshold from U32 to S32.
// 
// 14    9/03/99 5:12p Ewedel
// Changed Temp1 to Temp, since there's only one temperature now.  Changed
// to use CPtsRecordBase, and made friendly to PTS message templates.
// 
// 13    8/26/99 1:42p Tnelson
// 
// 12    8/11/99 7:44p Ewedel
// Modest tweaks to reflect final (I hope!) AVR EEPROM data.
// 
// 11    6/14/99 11:54p Ewedel
// Rearranged states to be closer to final, and removed "desired state"
// and its funny parameter fields.  These are now replaced by the command
// queue DDM interface machinery.
// 
// 10    6/03/99 5:58p Ewedel
// Updated state values, added separate "desired state" enum, corrected
// serial number fields (16 bytes for CMA-based ones), added PCI window
// parameters.  Also changed Iop_Type to use enum, instead of simple
// integer.
// 
// 9     5/12/99 4:17p Ewedel
// Split IOP state field into two:  current and desired (a.k.a. status &
// control).
// Removed obsolete second set of temperature parameters.
// 
// 8     4/05/99 5:37p Ewedel
// Changed logical slot count from 20 to 26, which appears to agree with
// the definition of TySlot in oos\Address.h.
// 
// 7     3/19/99 6:57p Ewedel
// Added a missing #define:  CT_IOPST_TABLE_VER which gives the current
// structure version.
// 
// 6     3/17/99 2:52p Ewedel
// Changed member eIOP_State to have its "true" enum type, instead of U32.
// 
// 5     3/11/99 6:11p Ewedel
// Added IopState enum, #define aliases for field names, and extern refs
// to standard field defs array / size (in IOPStatus.cpp).  Also added
// "max slots" constant, though this might go away.
// 
// 4     3/05/99 3:58p Jlane
// Added IOP_STATUS_TABLE define for the Tablew name.
// 
// 3     3/05/99 3:19p Jlane
// Added include of Address.h for TySlot.  Why wasn't this included with
// CTTypes?  TODO:  Figure that out.
// 
// 2     3/04/99 6:24p Ewedel
// Added missing typedef keyword, removed commented-out #include.
// 
// 1     3/02/99 5:12p Jlane
// Initial checkin.
//
// 03/01/99 JFL Created.  A sunny Monday.  Last Monday before vacation?
// 
/*************************************************************************/

#ifndef _IOPStatusRecord_h
#define _IOPStatusRecord_h


#ifndef _PtsRecordBase_h
# include  "PtsRecordBase.h"
#endif

#ifndef __Address_h
# include  "Address.h"           // for SLOT IDs
#endif

#if !defined(_IopTypes_h_)
# include  "IopTypes.h"
#endif

#ifndef __RqPts_T_h
# include  "RqPts_T.h"
#endif


#pragma pack(4)      // standard packing for PTS things

//  table name
#define  CT_IOPST_TABLE_NAME         "IOPStatusTable"

#define  CT_IOPST_TABLE_VER          (1)     /* current struct version */


//
// IOPStatusRecord
//
// NOTE: We need to make sure we've got what we need for hot swap such as adding stuff for 
// the mechanical "activators" that will be controlling insertion and extraction of drives and IOPs.
// Asked Norm and there's no news yet as 9/24/98..
//

enum IopState {
   IOPS_UNKNOWN = 0,       // default when we know nothing
   IOPS_EMPTY,             // no card in slot
   IOPS_BLANK,             // blank card in slot
   IOPS_POWERED_ON,        // fresh state when IOP is just powered on
   IOPS_AWAITING_BOOT,     // IOP awaiting PCI window / boot instructions
   IOPS_DIAG_MODE,         // running diagnostic "image"
   IOPS_BOOTING,           // IOP boot ROM handing off control to boot image
   IOPS_LOADING,           // IOP image running, loading "system entries"
   IOPS_OPERATING,         // normally operating (OS / app-level code)
   IOPS_SUSPENDED,         // IOP's MIPS is suspended (no PCI accesses)
   IOPS_FAILING,           // IOP failure detected, IOP is being failed away
   IOPS_FAILED,            // IOP failed away from, may now shut down or reboot
   IOPS_QUIESCING,         // IOP is gbeing Quiesced.
   IOPS_QUIESCENT,         // in quiesced state
   IOPS_POWERED_DOWN,      // MIPS cluster is powered down
   IOPS_UNLOCKED,          // IOP's card locking solenoid has been released,
                           //  ready for card extraction (only possible when
                           //  IOP's MIPS is also powered down)
   IOPS_IMAGE_CORRUPT,     // IOP was told to boot an image, & found it corrupt
   IOPS_INSUFFICIENT_RAM,  // IOP has insufficient RAM to perform requested boot
   IOPS_UPDATING_BOOT_ROM  // IOP is presently reprogramming its BOOT ROM
};


// One entry for each CMB-addressable entity.

class IOPStatusRecord : public CPtsRecordBase
{
public:
    IopType         IOP_Type;           // read from CMB.  See IopTypes.h.
    TySlot          Slot;               // Slot # I live in .  See Oos\Address.h.
    TySlot          RedundantSlot;      // Slot # of my redundant counterpart.
    String32        Manufacturer;       // Manufacturer of this card "ConvergeNet"?
    U32             ulAvrSwVersion;     // AVR firmware version (a byte)
    U32             ulAvrSwRevision;    // AVR firmware revision (a byte)
    String16        strHwPartNo;        // board's part number (xx-xxxxx-xx, sans '-'s)
    U32             ulHwRevision;       // board revision number (a byte)
    U32             ulHwMfgDate;        // original board mfg date: ANSI timestamp,
                                        //  (seconds since 1/1/70).
    String16        SerialNumber;       // Board Serial Number.
    String16        ChassisSerialNumber;// Board's last known chassis serial #.

    U32             ulIopEpldRevision;  // IOP: EPLD revision (two bytes)
    U32             ulIopMipsSpeed;     // IOP: MIPS clock freq (2 bytes, format?)
    U32             ulIopPciSpeed;      // IOP: PCI speed (2 bytes, format?)

#if false // These items require binary def'n'
    DateTime        Installation;       // Date and time of installation.
    OneOrTwo        WhichImageIsActive; // which system image is active.
    // will we know the version of an image that hasn't run yet?
    Status          Image1Status,       // Suspect, Obsolete, Active?  See I2O spec
    Status          Image2Status,
#endif
    IopState        eIOPCurrentState;   // IOP's current state

    S32             Temp;              // Temperature near the CPU.
    S32             TempHiThreshold;   // Threshold value for above to turn fans up.
    S32             TempNormThreshold; // Threshold value for above to turn fans down.
                                       // (delta between Hi and Norm is hysteresis)

    //  constructor for basic initialization
    IOPStatusRecord (void);

    //  helper for clearing an existing record
    void  Clear (void);

    //  here are the standard field defs for our row/table
    static const fieldDef *FieldDefs (void);

    //  and here is the size, in bytes, of our field defs
    static const U32 FieldDefsSize (void);

    //  here is the name of the PTS table whose rows we define
    static const char *TableName (void);

    //  some PTS interface message typedefs
	typedef RqPtsDefineTable_T<IOPStatusRecord> RqDefineTable;
	typedef RqPtsDeleteTable_T<IOPStatusRecord> RqDeleteTable;
	typedef RqPtsQuerySetRID_T<IOPStatusRecord> RqQuerySetRID;
	typedef RqPtsEnumerateTable_T<IOPStatusRecord> RqEnumerateTable;
	typedef RqPtsInsertRow_T<IOPStatusRecord> RqInsertRow;
	typedef RqPtsModifyRow_T<IOPStatusRecord> RqModifyRow;
	typedef RqPtsModifyField_T<IOPStatusRecord> RqModifyField;
	typedef RqPtsModifyBits_T<IOPStatusRecord> RqModifyBits;
	typedef RqPtsTestAndSetField_T<IOPStatusRecord> RqTestAndSetField;
	typedef RqPtsReadRow_T<IOPStatusRecord> RqReadRow;
	typedef RqPtsDeleteRow_T<IOPStatusRecord> RqDeleteRow;
	typedef RqPtsListen_T<IOPStatusRecord> RqListen;
};  /* end of class IOPStatusRecord */

#if false
// We're NOT going to use the following in the card status table.
enum VerifyCode {       // This is from SES SCSI Enclosure Services.
Error = 0,      // 0 = "An error occurred; check status code."
IdontExist,     // 1 = "This component does not exist."  I love this one - it's the default too - JFL.
IdontVerify,        // 2 = "Verification is not supported."
Reserved,       // 3 = "Reserved."
IExistUntested,     // 4 = "This component exists, but the functionality is untested."
IExistUnKnown,      // 5 = "This component exists, but the functionality is unknown."I
ExistBroken,        // 6 = "This component exists, and is not functioning correctly."
IexistWorking       // 7 = "This component exists, and is functioning correctly."
};
VerifyCode  Verify;         // See enum values above.
#endif


//  compiler-checkable aliases for table / field names

//  field defs
#define  CT_IOPST_REC_VERSION        CT_PTS_VER_FIELD_NAME // Version of IOP_Status record.
#define  CT_IOPST_SIZE               CT_PTS_SIZE_FIELD_NAME// # of bytes in record.
#define  CT_IOPST_IOP_TYPE           "IOP_Type"            // read from CMB.
#define  CT_IOPST_SLOT               "Slot"                // Slot # I live in .  See DeviceId.h.
#define  CT_IOPST_REDUNDANTSLOT      "RedundantSlot"       // Slot # of my redundant counterpart.
#define  CT_IOPST_MANUFACTURER       "Manufacturer"        // Manufacturer of this card "ConvergeNet"?
#define  CT_IOPST_AVRSWVERSION       "ulAvrSwVersion"
#define  CT_IOPST_AVRSWREVISION      "ulAvrSwRevision"
#define  CT_IOPST_HWPARTNO           "strHwPartNum"
#define  CT_IOPST_HWREVISION         "ulHwRevision"
#define  CT_IOPST_HWMFGDATE          "ulHwMfgDate"
#define  CT_IOPST_SERIALNUMBER       "SerialNumber"        // Board Serial Number.
#define  CT_IOPST_CHASSISSERIALNUM   "ChassisSerialNumber" // Board Serial Number.
#define  CT_IOPST_IOPEPLDREVISION    "ulIopEpldRevision"
#define  CT_IOPST_IOPMIPSSPEED       "ulIopMipsSpeed"
#define  CT_IOPST_IOPPCISPEED        "ulIopPciSpeed"
#define  CT_IOPST_IOPCURRENTSTATE    "eIOPCurrentState"    // known-size alias for enum IopState
#define  CT_IOPST_TEMP               "Temp"                // Temperature near the CPU.
#define  CT_IOPST_TEMPHITHRESHOLD    "TempHiThreshold"     // Threshold value for above to turn fans up.
#define  CT_IOPST_TEMPLOWTHRESHOLD   "TempLowThreshold"    // Threshold value for above to turn fans up.



//  here is the standard table which defines IOP Status table fields
extern const fieldDef aIopStatusTable_FieldDefs[];

//  and here is the size, in bytes, of the IOP Status field defs
extern const U32 cbIopStatusTable_FieldDefs;


#endif  /* #ifndef _IOPStatusRecord_h */

