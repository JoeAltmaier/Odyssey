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
// File: CmbHwIntfMsgs.h
// 
// Description:
//    Card Management Bus interface messages, as sent between the
//    CMA Atmel microcontroller and the MIPS processor.
//    These messages are documented in the CMB Design Specification.
//    [These are also mostly the same messages as are sent on the
//     CMB itself.  We do this so that the Atmel part doesn't have
//     to spend code space on message reformatting.]
// 
// $Log: /Gemini/Odyssey/DdmCmb/CmbHwIntfMsgs.h $
// 
// 17    1/20/00 4:15p Eric_wedel
// Added new set / get boot conditions codes.
// 
// 16    12/13/99 1:47p Ewedel
// Removed EVC Master stuff, added "GetAttnStatus" command.
// 
// 15    11/24/99 4:56p Ewedel
// Fixed up MakeReply() so it knows a little about ack/nak replies.
// 
// 14    10/27/99 12:16p Ewedel
// A bit of cleanup.
// 
// 13    10/22/99 10:55a Ewedel
// Updated to reflect new CMB definitions.
// 
// 10    9/10/99 4:31p Ewedel
// Changed CmbAddrMips definition to use new Address.h global flag
// CMB_ADDR_MIPS_FLAG.
// 
// 9     8/11/99 7:52p Ewedel
// Added various extra command and parameter codes to flesh out the
// current CMB command set.
// 
// 8     8/02/99 7:27p Ewedel
// Changed to track CMB Design spec 0.092.  (Mostly EVC revisions.)
// 
// 7     7/15/99 4:21p Ewedel
// Changed CmbPacket so that default type is full-sized (32 bytes), and so
// that header data is more clearly segregated.  Also added handy-dandy
// Size() member.
// 
// 6     6/24/99 7:23p Ewedel
// Modified command codes, added parameter enums for new commands, and
// modified form of constant names.
// 
// 5     6/14/99 11:56p Ewedel
// Added "boot type" parameter enum, NAK packet "reason" enum, and cleaned
// up "MIPS" states a bit.
// 
// 4     6/12/99 1:26a Ewedel
// Clarified comments, added initialize of packet "status" byte in
// constructor.
// 
// 3     6/03/99 7:14p Ewedel
// Updated per latest CMB Design spec.
// 
// 2     5/14/99 3:35p Ewedel
// Added "GetBoardInfo" and "EnableUnsolicited" command codes, and updated
// a few comments.
// 
// 1     5/14/99 12:04p Ewedel
// Initial revision.
// 
/*************************************************************************/

#ifndef _CmbHwIntfMsgs_h_
#define _CmbHwIntfMsgs_h_


#ifndef Simple_H
# include  "Simple.h"
#endif


//  pack following structures to a byte boundary
#pragma pack(1)


//  possible CMB packet command codes (these are written into request packets
//  sent from the MIPS to the AVR).
//  These values are written into CmbPacket.bCommand (see below).
enum CmbHwCmdCode {
   k_eCmbCmdStatusPoll      = 3,    // get CMA's status byte
   k_eCmbCmdGetCommand      = 4,    // get 3rd party CMA command
   k_eCmbCmdCmbSlotStatus   = 5,    // read CMB master's status for some CMB slot
   k_eCmbCmdGetAttnStatus   = 7,    // read CMA's "attention status" bits
   k_eCmbCmdGetBoardInfo    = 8,    // read board's type, slot ID, and HBC master
   k_eCmbCmdSetPciWindow    = 9,    // set IOP's PCI window parameters
   k_eCmbCmdSetBootParams   = 10,   // set IOP's boot type, and initiate boot
   k_eCmbCmdSetMipsState    = 11,   // set IOP's reported MIPS state (in status)
   k_eCmbCmdSetAutoPoll     = 12,   // enable or disable CMB master's auto-poll

   k_eCmbCmdPowerControl    = 0x10, // control power to an IOP's MIPS CPU
   k_eCmbCmdPciBusAccessCtl = 0x11, // set an IOP's PCI quickswitches (on or off)
   k_eCmbCmdResetCpu        = 0x12, // reset an IOP's MIPS (strobe or leave on)
   k_eCmbCmdNmiCpu          = 0x13, // send NMI to an IOP's MIPS CPU
   k_eCmbCmdSpiResetEnable  = 0x14, // enable SPI Reset assertion by HBC MIPS
   k_eCmbCmdChangeHbcMaster = 0x15, // tell EVC to change HBC master line
   k_eCmbCmdIopSlotLockCtl  = 0x16, // define IOP's locking solenoid enable flag

   k_eCmbCmdGetLastValue    = 0x20, // one param - reads last recorded value
   k_eCmbCmdGetCurValue     = 0x21, //  "    "   - reads value *now*
   k_eCmbCmdSetFanSpeed     = 0x22, //  "    "
   k_eCmbCmdSetThreshold    = 0x23, //  "    "   - auto-fan speed thresholds
   k_eCmbCmdSetSysSerialNum = 0x24, // one param - chassis serial number
   k_eCmbCmdSetSmpV54       = 0x25, //
   k_eCmbCmdSetPriSupplyEna = 0x26, // set primary supply enable flags
   k_eCmbCmdSetAuxSupplyEna = 0x27, // set aux supply enable flags
   k_eCmbCmdSetBootCondit   = 0x28, // set boot condition flags

   k_eCmbCmdDdmMessage      = 0x30, // DDM-level messaging envelope code
   k_eCmbCmdSendLogEntry    = 0x31, // specialized envelope for event log xmit

   k_eCmbCmdGetDdhStatus    = 0x40, // zero params
   k_eCmbCmdSetDriveLock    = 0x41, // mess with drive bay locking solenoid
   k_eCmbCmdSetPortBypass   = 0x42  // FC loop port bypass @ DDH

};


//  these are the defined parameter type codes used with command codes
//  k_CmbCmdGetLastValue and k_CmbCmdGetCurValue.
//  These are written as a request packet's first data byte.
enum CmbHwParamType {
   k_eCmbParamEvcPrimStatus      = 1,    // read primary supply status
   k_eCmbParamEvcPrimVoltage     = 2,    // read primary supply voltage (comb.)
   k_eCmbParamReserved1          = 3,    // 
   k_eCmbParamEvcAuxCurrents     = 4,    // read aux supply currents
   k_eCmbParamEvcAuxTemperatures = 5,    // read aux supply converter temps
   k_eCmbParamEvcAuxVoltages     = 6,    // read aux supply voltages
   k_eCmbParamEvcAuxEnables      = 7,    // read aux supply enable flags
   k_eCmbParamEvcBattFuseDrops   = 8,    // read battery fuse voltage drops
   k_eCmbParamEvcBattTemperatures= 9,    // read battery temperatures
   k_eCmbParamReserved2          = 0xA,  // 
   k_eCmbParamEvcFanSpeeds       = 0xB,  // read fan speeds
   k_eCmbParamEvcKeyPosition     = 0xC,  // read rotary keyswitch position
   k_eCmbParamGetBootConditions  = 0xD,  // read boot condition flags

   k_eCmbParamTemperature        = 0x10, // temperature read by EVC or IOP's CMA
   k_eCmbParamIopType            = 0x11, // read IOP's type (enum IopType)
   k_eCmbParamChassisSerNum      = 0x12, // read Odyssey chassis serial number
   k_eCmbParamBoardSerNum        = 0x13, // read IOP's board serial number
   k_eCmbParamBoardHwBuildInfo   = 0x14, // read IOP's board hardware build info
   k_eCmbParamBoardMfgName       = 0x15, // read IOP's manufacturer name (US ASCII)
   k_eCmbParamCmaFirmwareInfo    = 0x16, // read CMA firmware's version info
   k_eCmbParamIopMipsHwParams    = 0x17  // read MIPS params used by boot ROM
};


//  here is the "boot type" code used with command code k_CmbCmdSetBootParams:
enum CmbBootType {
   k_eCmbBootNone = 0,        // no boot type given (reserved)
   k_eCmbBootDiags,           // begin running diags (param defs tbd)
   k_eCmbBootPCI,             // boot PCI image
   k_eCmbBootCrashDump        // perform crash dump (param defs tbd)
};


//  here is the "power control action" code used with command code
//  k_CmbCmdPowerControl:
enum CmbPowerControlAction {
   k_eCmbPowerOn = 1,         // enable power to IOP's MIPS CPU
   k_eCmbPowerOff = 2         // disable power to IOP's MIPS CPU
};


//  here is the "PCI bus access control action" code used with command
//  code k_eCmbCmdPciBusAccessCtl:
enum CmbPciBusAccessControlAction {
   k_eCmbPciAccessEnable = 1,    // enable MIPS access to its PCI bus(es)
   k_eCmbPciAccessIsolate = 2    // isolate IOP / MIPS from PCI bus(es)
};


//  here is the "solenoid control action" code used with command
//  code k_eCmbCmdIopSlotLockCtl:
enum CmbSlotLockControlAction {
   k_eCmbLockEnable = 0,      // disable extraction of IOP / drive from slot
   k_eCmbLockDisable = 1      // enable      "       "  "      "    "    "
};


//  here is the "reset action" code used with command code k_eCmbCmdResetCpu:
enum CmbCpuResetAction {
   k_eCmbCpuAssertReset = 1,     // assert reset to target MIPS, leave it reset
   k_eCmbCpuDeassertReset = 2,   // deassert reset at target MIPS (let it run)
   k_eCmbCpuStrobeReset = 3      // assert/deassert reset (let MIPS run)
};


//  here is the "drive bypass action" code used with command
//  code k_eCmbCmdSetPortBypass:
enum CmbDriveBypassControlAction {
   k_eCmbDriveEnable = 0,     // enable FC access to drive
   k_eCmbDriveBypass = 1      // isolate drive from FC loops and Odyssey
};


//  here is the "fan speed set" target fan pair selection, used with
//  command code k_eCmbCmdSetFanSpeed:
enum CmbFanSpeedSetFanSelect {
   k_eCmbFanPairExhaust = 0,     // exhaust fans, mounted in EVC FRUs
   k_eCmbFanPairIntake = 1       // intake fans, in bottom of chassis
};

//  here is the fan-speed temperature threshold selection, used with
//  command code k_eCmbCmdSetThreshold
enum CmbFanSpeedTempThreshSelect {
   k_eCmbFanTempThreshFanUp = 0,       // force fans to full speed at this temp.
   k_eCmbFanTempThreshFanRestore = 1   // restore config'd fan speed at this temp
};

//  here is the "SPI Reset Enable" selection, used with command
//  code k_eCmbCmdSpiResetEnable:
enum CmbSpiResetEnableSelect {
   k_eCmbSpiResetEnable = 1,     // allow MIPS to assert reset to AVRs
   k_eCmbSpiResetDisable = 2
};


//  standard CMB packet structure -- note that we make packet's
//  variable-length tail long enough to accomodate the largest
//  supported CMB packet.

//  maximum packet size supported by CMAs
#define  cbCmbPacketMax    (32)


//  the value CmbPacket.bStatus contains multiple bitfields.  The following
//  defs help in accessing them:
#define  CmbStatCmd       (0x80)    /* set when command, clear when response */
#define  CmbStatAck       (0x40)    /* resp. only, set for ack, clear for nak */
#define  CmbStatCmdAvail  (0x20)    /* resp. only, set when slave has cmd to send */
#define  CmbStatMipsState (0x0F)    /* resp. only, slave's MIPS CPU state */


class CmbPacket
{
public:

   //  what packet header always looks like
   typedef struct {
      U8    bDestAddr;     // destination address
      U8    bSrcAddr;      // source address (MIPS sets to zero when creating
                           //  a request packet)
      U8    bCommand;      // request code (a CmbHwCmdCode enum member)
      U8    bStatus;       // status bitfield
      U8    cbData;        // length of data tail, incl. abTail[] and its CRC
                           // (zero if no data included in message)
      U8    bHeaderCRC;    // CRC of five preceding header bytes only - computed
                           // by Atmel, MIPS can leave this blank.
   } Header;

   //  standard packet header
   Header   Hdr;

   //  variable-size payload tail
   //  [actual size is [Hdr.cbData + 1] bytes (tail + 2nd CRC),
   //   or zero iff Hdr.cbData == 0.]
   U8    abTail[cbCmbPacketMax - sizeof (Header)];

   //  a constructor, too
   CmbPacket (U8 bCommand = 0)
      {
      Hdr.bDestAddr = Hdr.bSrcAddr = 0;
      Hdr.bCommand = bCommand;
      Hdr.cbData = Hdr.bHeaderCRC = 0;
      Hdr.bStatus = 0;
      };

   //  make a reply packet, given a command packet
   void  MakeReply (const CmbPacket& pktCmd, BOOL fIsAck = TRUE)
      {
      Hdr.bDestAddr = pktCmd.Hdr.bSrcAddr;   // flip SRC and DST..
      Hdr.bSrcAddr  = pktCmd.Hdr.bDestAddr;
      Hdr.bCommand  = pktCmd.Hdr.bCommand;
      if (fIsAck)
         {
         //  set happy status
         Hdr.bStatus   = CmbStatAck;      //BUGBUG: what is MIPS state value?
         Hdr.cbData    = 0;
         }
      else
         {
         //  whoops, set a NAK
         Hdr.bStatus   = 0;         // does MIPS state matter?
         Hdr.cbData    = 1;         // single payload byte is NAK reason code
         }
      };

   //  figure out packet's current actual size
   U32  Size (void)  const
         {  return (sizeof(Hdr) + ((Hdr.cbData > 0) ? Hdr.cbData + 1 : 0));  };

   //  how big of a payload we can hold (excl. header & 2nd CRC)
   static U32  PayloadMaxSize (void)
         {  return (cbCmbPacketMax - sizeof (Header) - 1);  };

};  /* end of class CmbPacket */


//  the values in CmbPacket.bDestAddr and .bSrcAddr are TySlot enum members
//  (see include\oos\address.h).  However, bit 7 is used to distinguish
//  between the AVR (CMA) and MIPS processors at a given IOP slot address.
//  Bit 7 is set if the dst/src is the MIPS, or clear if it is the AVR:
#define  CmbAddrMips    (CMB_ADDR_MIPS_FLAG)

//  the bitfield selected by mask CmbStatMipsState can assume any of the
//  following values:
enum CmbMipsState {        
   k_eCmbMStUnknown         = 0, // [Unknown whether CMA even exists at this
                                 //  CMB address]
   k_eCmbMStNotFound        = 1, // [CMA not found at this CMB address]
   k_eCmbMStPoweredOff      = 2, // MIPS CPU in this slot is powered off
   k_eCmbMStPoweredOn       = 3, // EVC or DDH is present and operating, or
                                 //  MIPS CPU is freshly powered-on
   k_eCmbMStAwaitingBoot    = 4, // MIPS is powered on, but has not established
                                 //  a boot type (e.g., the "SetBootParams"
                                 //  command has not been sent to it yet)
   k_eCmbMStRunningDiags    = 5, // MIPS is running its diagnostic image,
                                 //  performing a diagnostic test of some sort
   k_eCmbMStAwaitingDiagCmd = 6, // MIPS is running the diagnostic image, awaiting
                                 //  the next diagnostic test command
   k_eCmbMStBootingPCIImage = 7, // MIPS is beginning to boot a PCI-based image
                                 //  (state reported by the CMA)
   k_eCmbMStLoadingOSImage  = 8, // MIPS is running OS image code, loading
                                 //  "system entries".  This state is set by
                                 //  the CMB DDM.
   k_eCmbMStRunningOSImage  = 9, // MIPS is running OS image code, and has full
                                 //  message routing established.
                                 //  This state is set by some DDM or other.
   k_eCmbMStOSImageSuspended= 10,// MIPS is running OS image code, and image's
                                 //   PCI bus access has been suspended (this
                                 //   is a MIPS state, broken out here for
                                 //   communications purposes only).
   k_eCmbMStImageCorrupt    = 11,// MIPS image is corrupt - boot aborted
                                 //   This state is set by the boot ROM.
   k_eCmbMStInsufficientRam = 12,// Insufficient memory for boot.
                                 //   This state is set by the boot ROM.
   k_eCmbMStUpdatingBootRom = 13,// IOP is presently updating its boot ROM
                                 //   image with one supplied by HBC
   k_eCmbMStAttentionNeeded = 14 // EVC/DDH has detected some event of note
};


//  here are the possible values in a NAK packet (a packet with neither
//  CmbStatCmd nor CmbStatAck flags set).

enum CmbNakReason {
   k_eCmbNRCmdUnknown   = 1,  // command not known/supported by target CMA
   k_eCmbNRBadParam     = 2,  // invalid parameter(s) for command
   k_eCmbNRCmaPktPreempt= 3,  // command from MIPS preempted by unsolicited
                              //  command, which CMA is awaiting response to
   k_eCmbNRCmaTimeout   = 4,  // timeout on CMB: no reply from target CMA
   k_eCmbNRMipsTimeout  = 5,  // timeout at target CMA: no reply from
                              //  destination MIPS CPU (only when CmbAddrMips)
   k_eCmbNRCrcError     = 6,  // CRC error in packet (detected by far end?)
   k_eCmbNRFramingError = 7,  // UART framing error (detected by which end?)
   k_eCmbNRHbcMasterChg = 8   // HBC Master line changed (packet context may
                              //  no longer be valid)
};


//  here are the EVC "attention needed" status flags

#define  CmbAttnFlagEvcKey       (1)   /* key position changed */
#define  CmbAttnFlagEvcBattery   (2)   /* battery current direction changed */

//  here are the DDH "attention needed" status flags

#define  CmbAttnFlagDdhBay       (1)   /* drive inserted / removed */


//  resume default structure packing
//#pragma pack()

#endif   // #ifndef _CmbHwIntfMsgs_h_


