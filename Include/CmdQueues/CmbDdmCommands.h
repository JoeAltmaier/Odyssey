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
// File: CmbDdmCommands.h
// 
// Description:
//    Card Management Bus DDM control interface definitions.
//    These are the commands submitted to the CMB DDM by other
//    DDMs in the Odyssey system.
//
//    Commands are submitted via class CmdSender, see include\CmdSender.h
//    and friends for the particulars.
// 
// $Log: /Gemini/Include/CmdQueues/CmbDdmCommands.h $
// 
// 7     9/03/99 5:02p Ewedel
// Removed "poll environment" command - this is now done via standard PHS
// Reporter machinery.
// 
// 6     8/11/99 7:38p Ewedel
// Added several more CMB request types.
// 
// 5     7/30/99 7:41p Ewedel
// Added environment poll, IOP slot and drive bay lock control requests.
// 
// 4     6/24/99 7:26p Ewedel
// Added command/status structure size #defines.
// 
// 3     6/15/99 7:06p Ewedel
// Removed derived queue names, they're no longer needed.
// 
// 2     6/14/99 11:53p Ewedel
// Added standard 'e' prefix on enum member values.
// 
// 1     6/12/99 1:22a Ewedel
// Command / status queue definitions for the CMB DDM.
// 
/*************************************************************************/

#ifndef _CmbDdmCommands_h_
#define _CmbDdmCommands_h_

#ifndef __Address_h
# include  "Address.h"        // for TySlot
#endif

#ifndef Simple_H
# include  "Simple.h"         // for BOOL, U32, etc.
#endif

#ifndef CTtypes_H
# include  "CtTypes.h"        // for rowID
#endif


//  common CMB control queue name root
#define     CMB_CONTROL_QUEUE    "CmbControlQueue"

//  size of CMB control queue's command messages
#define     CMB_CONTROL_COMMAND_SIZE   (sizeof (CmbCtlRequest))

//  size of CMB control queue's status messages (unused)
#define     CMB_CONTROL_STATUS_SIZE    (0)


//  here are the support CMB control command codes:
enum CmbCtlCommand {
   k_eCmbCtlPower = 1,        // params are CCmbCtlPower
   k_eCmbCtlPciWindow,        // params are CCmbCtlPciWindow
   k_eCmbCtlCmbCtlBoot,       // params are CCmbCtlBoot
   k_eCmbCtlIopLockCtl,       // params are CCmbCtlIopLock
   k_eCmbCtlDriveLockCtl,     // params are CCmbCtlDriveLock
   k_eCmbCtlDriveBypassCtl,   // params are CCmbCtlDriveBypass
   k_eCmbCtlFanSpeed,         // params are CCmbCtlSetFanSpeed
   k_eCmbCtlOneChassisSN,     // params are CCmbCtlSetChassisSN
   k_eCmbCtlPciBusAccess,     // params are CCmbCtlPciBusEnable
   k_eCmbCtlSpiResetEnable,   // params are CCmbCtlSpiResetEnable
   k_eCmbCtlHbcMaster,        // params are CCmbCtlHbcMaster
   k_eCmbCtl
};


//  use to turn power to an IOP's MIPS CPU on or off
class CCmbCtlPower
{
public:
   BOOL     bPowerOn;      // TRUE to turn on power, FALSE to turn it off
};  /* end of class CCmbCtlPower */


//  use to tell an IOP's MIPS boot ROM what PCI window it should set
//  (Legal only when IOP is in IOPS_AWAITING_BOOT state -- see
//   CtTables\IopStatusTable.h.  This command doesn't change IOP's state.)
class CCmbCtlPciWindow
{
public:
   U32      ulPciWinSize;     // IOP's PCI window size (lsb = 64kB)
   U32      ulPciWinPciBase;  // IOP's PCI window base addr on PCI bus
                              //  (lsb = 64kB)
   U32      ulPciWinIopBase;  // IOP's PCI window base addr on IOP
                              //  (lsb = 64kB)
};  /* end of CCmbCtlPciWindow */


//  use to tell an IOP's MIPS boot ROM what sort of "boot" course to take
//  (Legal only when IOP is in IOPS_AWAITING_BOOT state -- see
//   CtTables\IopStatusTable.h.  This command may change the IOP's state.)
class CCmbCtlBoot {
public:

   enum Action {
      k_eNone = 0,            // no-op
      k_eDiagnostic = 1,      // enter diagnostic mode (params tbd)
      k_ePCI = 2,             // boot an image loaded via IOP's PCI window
      k_eCrashDump = 3        // do a crash dump - action tbd
   };

   Action   eAction;          // what sort of "boot" to do

   //  PCI boot params
   U32      ulImageOffset;    // offset of boot image from start of
                              //  PCI window - lsb == 64kB
   U32      ulParamOffset;    // offset of boot image param area from start
                              //  of PCI window - lsb == 64kB

};  /* end of CCmbCtlBoot */


//  tells the CMB to lock or unlock an IOP's card locking solenoid
class CCmbCtlIopLock {
public:
   enum Action {
      k_eNone = 0,            // no-op
      k_eLockIop,          // lock IOP in its slot
      k_eUnlockIop         // unlock IOP, to allow its removal from slot
   };

   Action   eAction;

   //  slot comes from our wrapper class, CmbCtlRequest

};  /* end of CCmbCtlIopLock */


//  tells the CMB to lock or unlock an internal drive bay's
//  drive locking solenoid.
class CCmbCtlDriveLock {
public:
   enum Action {
      k_eNone = 0,         // no-op
      k_eLockDrive,        // lock drive in its bay
      k_eUnlockDrive       // unlock drive, to allow its removal from bay
   };

   Action   eAction;

   U32      ulDriveBay;    // drive bay number to operate on
                           // (see odyssey_hw_params.doc for mapping)

};  /* end of CCmbCtlDriveLock */


//  tells a DDH to enable access to a drive, or to bypass it
//  so that the drive is not accessible via the FC loops
//  (and so the drive doesn't pollute the loops with bad traffic)
class CCmbCtlDriveBypass {
public:
   enum Action {
      k_eNone = 0,         // no-op
      k_eEnableDrive,      // include drive in FC loops
      k_eBypassDrive       // exclude drive from FC loops
   };

   Action   eAction;

   U32      ulDriveBay;    // drive bay number to operate on
                           // (see odyssey_hw_params.doc for mapping)

};  /* end of CCmbCtlDriveBypass */

//  tell the EVCs to set target fan speeds.  The requestor can say to
//  set only intake, only exhaust, or both speeds if desired.
//  [Or neither, if they're being really silly.]
class CCmbCtlSetFanSpeed {
public:

   U32   ulIntake;         // desired intake (lower) fan speed
   BOOL  fUpdateIntake;    // should we change intake setting

   U32   ulExhaust;        // desired exhaust (upper) fan speed
   BOOL  fUpdateExhaust;   // should we change exhaust setting

};  /* end of CCmbCtlSetFanSpeed */

//  tell some CMB denizen to set its chassis serial number data
//  (used by FRU assimilation code)
class CCmbCtlSetChassisSN {
public:

   String16 strSerialNumber;

};  /* end of CCmbCtlSetChassisSN */

//  tell an IOP to enable or disable its PCI bus
class CCmbCtlPciBusEnable {
public:

   enum Action {
      k_eCtlPciBusNone = 0,      // do nothing
      k_eCtlPciBusEnable = 1,    // enable PCI bus access from IOP
      k_eCtlPciBusIsolate = 2    // isolate IOP from its PCI bus(es)
   };

   Action   eAction;

};  /* end of CCmbCtlPciBusEnable */

class CCmbCtlSpiResetEnable {
public:

   enum Action {
      k_eCtlSpiEnableReset = 1,  // enable MIPS-asserted SPI reset lines
      k_eCtlSpiDisableReset = 2  // disable  "      "     "    "     "
   };

   Action   eAction;

};  /* end of class CCmbCtlSpiResetEnable */

class CCmbCtlHbcMaster {
public:

};  /* end of class CCmbCtlHbcMaster */



//  here's our master command parameter struct.  This struct is used
//  both for command and status transfers.  We have no separate
//  "results" struct, we simply loop back the full command packet.

//  As presently defined, classes CmdSender & CmdServer required
//  that the first parameter be a PTS rowID.  Blecchhh!
//  Other than that little problem, this is a nice generic
//  parameter transport mechanism.

typedef struct {
   rowID          rid;        // oops - required by CmbServer/CmbSender

   CmbCtlCommand  eCommand;   // what this request is for -- "tags" member (u)

   TySlot         eSlot;      // of IOP to update (not used for Drive cmds)

   //  and a nice little union, "tagged" by eCommand.  This union
   //  lets us use the size of our overall struct to define our
   //  maximum "queue struct" parameter size by a simple sizeof().
   union {
      CCmbCtlPower            Power;
      CCmbCtlPciWindow        PciWindow;
      CCmbCtlBoot             Boot;
      CCmbCtlIopLock          IopLock;
      CCmbCtlDriveLock        DriveLock;
      CCmbCtlDriveBypass      DriveBypass;
      CCmbCtlSetFanSpeed      FanSpeed;
      CCmbCtlSetChassisSN     ChassisSN;
      CCmbCtlPciBusEnable     PciBus;
      CCmbCtlSpiResetEnable   SpiResetEnable;
      CCmbCtlHbcMaster        SetHbcMaster;
   } u;

}  CmbCtlRequest;


#endif   // #ifndef _CmbDdmCommands_h_


