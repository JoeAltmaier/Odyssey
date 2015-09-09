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
// Description:
// This is the definition and declaration of the EVC/Enclosure Status table.
// Note that this table has only one row instance.
// 
// $Log: /Gemini/Include/CTTables/EVCStatusRecord.h $ 
// 
// 20    12/14/99 8:10p Ewedel
// Removed obsolete EVC Master stuff.  [VN]
// 
// 19    12/01/99 4:48p Ewedel
// Changed to derive from CPtsRecordBase.  Added real constructor.
// 
// 18    10/21/99 4:46p Vnguyen
// Add entry for EvcMasterSlot per change in EvcRawParameters.h
// 
// 17    10/08/99 4:44p Vnguyen
// Change FieldDefSize to constant type.
// 
// 16    9/08/99 9:54a Vnguyen
// Add fPrimaryEnable array.
// 
// 15    9/08/99 9:16a Vnguyen
// Change temperature from U32 to S32.
// 
// 14    9/03/99 5:07p Ewedel
// Updated comments to clarify units (lsbs).
// 
// 13    8/23/99 12:28p Ewedel
// Added FieldDefs() / FieldDefsSize() support.
// 
// 12    8/20/99 3:30p Vnguyen
// Add key, RefreshRate fields for PHS Reporter.
// 
// 11    8/18/99 7:56a Vnguyen
// Delete ACTIVEIOPSMASK field.  Add afEVCReachable[2] field.
// 
// 10    8/12/99 7:09p Ewedel
// Changed battery charge current to signed (thanks, Vuong!).  Also
// removed duplicate CT_EVCST_ACTIVEIOPSMASK which prior merge added.
// 
// 9     8/11/99 7:42p Ewedel
// Changed to reflect consolidated EVC reporting, and removed serial
// number fields (they are in the IOP Status table, which also reports on
// EVCs and DDHs).
// Detailed per-EVC data may now be found in EvcRawParameters.h, .cpp.
// 
// 8     8/08/99 11:07a Jlane
// Added ActiveIOPsMask to EVCStatus record.  This word has TySlot bits
// set indicating active IOPs.  Maintained by OS/BootMgr and used by
// Masters instantiating DDMs.
// 
// 7     7/30/99 3:47p Ewedel
// Grouped various global params, and array-ized them to reflect separate
// readings from each EVC.
// 
// 6     7/27/99 6:43p Ewedel
// Updated to use arrays consistently, and for latest EVC hardware defs
// [E2].  Also added keyswitch position enum.
// 
// 5     6/03/99 5:54p Ewedel
// Renumber exhaust air temp readings for consistency with EVC IDs.
// Changed fan speed setting array per current hardware (only two
// independent controls).  Added full complement of serial numbers (two
// per each EVC: board and chassis).
// 
// 4     3/16/99 5:49p Ewedel
// Changed some entries back to arrays (where they're compatible with PTS
// fielddefs).  Removed non-EVC ambient temp items.  Stripped tabs.  Added
// copyright string per present template file.  Added #define values for
// use as field names.  Added externs for standard field defs in
// EVCStatusRecord.cpp.
// 
// 3     3/05/99 3:29p Jlane
// Added typedef and a missing semicolon.
// 
// 2     3/04/99 11:51a Jlane
// Expanded arrays into multiple inline fields.  Changed everything to be
// 4 bytes too.
// 
// 1     3/02/99 5:12p Jlane
// Initial checkin.
// 03/01/99 JFL Created.  A sunny Monday.  Last Monday before vacation?
/*************************************************************************/

#ifndef _EVCStatusRecord_h
#define _EVCStatusRecord_h


#if !defined(PtsCommon_H)
# include  "PtsCommon.h"
#endif

#ifndef _PtsRecordBase_h
# include  "PtsRecordBase.h"
#endif

#ifndef __RqPts_T_h
# include  "RqPts_T.h"
#endif


#pragma  pack(4)

#define EVC_STATUS_TABLE      (EVCStatusRecord::TableName())

#define EVC_STATUS_TABLE_VER  (1)      /* current struct version */

//
// EVC Status Table - Data detailing status of enclosure characteristics
//                    such as temperature, power supplies, fans. etc.
// 

//          Just one row entry for the entire Odyssey box.
class EVCStatusRecord : public CPtsRecordBase
{
public:

   // reporter data (is this needed by Env DDM?)
   U32      key;                    // For PHS Reporter Listen key
   U32      RefreshRate;

   U32      afEvcReachable[2];  // (bool) FALSE -> disregard all other params for the EVC
                           // e.g. if afEvcReachable[0] = False, then ignore
                           // all paramaters that are unique to EVC#0.  At this
                           // time, they are the Auxiliary Power Supply values.

   //  cabinet temperature & related info (temperatures have lsb of degrees C)
   S32      ExitAirTemp[2];         // Temperature of air exiting the enclosure,
                                    //  read by EVC's sensor.  Indexed by EVC #.
   S32      ExitTempFanUpThresh;    // Exit air temp at which fans are
                                    //  auto-forced by EVCs to full speed
   S32      ExitTempFanNormThresh;  // Exit air temp at which fans are restored
                                    //  to their FanSpeedSet[] settings
   U32      FanSpeed[4];            // Current RPMs for each fan (read only)
   U32      FanSpeedSet[2];         // Set target RPMs for each fan pair:
                                    //  [0] = upper pair, [1] = lower pair

   //  primary power supply-provided status values (all read-only),
   //  indexed by primary power supply ID.
   U32      fInputOK[3];            // Input (A/C) power OK (bool)
   U32      fOutputOK[3];           // Power Supply Output OK (bool)
   U32      fFanFailOrOverTemp[3];  // combination fan failure / over temp
                                    // alert. (bool)
   U32      fPrimaryEnable[3];      // which supplies has EVC enabled?  (bool)

   //  auxiliary power supply-provided status values (all read-only),
   //  indexed by auxiliary power supply ID.
   U32      DCtoDC33Current[2];     // Current output of 3.3V converter (1ma lsb)
   U32      DCtoDC5Current[2];      // Current output of 5V converter (1ma lsb)
   U32      DCtoDC12ACurrent[2];    // Current output of 12V 'A' conv. (1ma lsb)
                                    //  (presently unreadable, left at 0)
   U32      DCtoDC12BCurrent[2];    // Current output of 12V 'B' conv. (1ma lsb)
   U32      DCtoDC12CCurrent[2];    // Current output of 12V 'C' conv. (1ma lsb)
   S32      DCtoDC33Temp[2];        // Temperature of 3.3V converter (degrees C)
   S32      DCtoDC5Temp[2];         // Temperature of 5V converter (degrees C)
   S32      DCtoDC12ATemp[2];       // Temperature of 12V 'A' conv. (degrees C)
   S32      DCtoDC12BTemp[2];       // Temperature of 12V 'B' conv. (degrees C)
   S32      DCtoDC12CTemp[2];       // Temperature of 12V 'C' conv. (degrees C)
   U32      fDCtoDCEnable[2];       // Is given aux supply's output enabled
                                    //  (3.3, 5, 12 combined; flag value is bool)

   //  SMP power status.  The same SMP lines are read and reported
   //  separately by each EVC; the values below are the consolidated
   //  results of the separate EVC readings.
   U32      SMP48Voltage;           // combined 48V primary supply output (millivolts)
   U32      DCtoDC33Voltage;        // combined output of both auxes (millivolts)
   U32      DCtoDC5Voltage;         // combined output of both auxes (millivolts)
   U32      DCtoDC12Voltage;        // combined output of both auxes (millivolts)
   U32      KeyPosition;            // enum CT_EVC_KEYPOS

   //  Odyssey battery / charge status (all read-only),
   //  indexed by battery module (FRU) ID.
   U32      fBatteryInstalled[2];   // Are batteries present (bool)
   S32      BatteryTemperature[2];  // temperature inside each battery pack
                                    //  (degrees C, somewhere between 2 cells)
   S32      BatteryCurrent[2];      // > 0 charging, < 0 discharging? (lsb = 1ma)

   
   //  constructor for basic initialization
   EVCStatusRecord (void);

   //  members used to access our record field definition

   //  here is the standard table which defines EVC Status table PTS fields
   static const fieldDef *FieldDefs (void);

   //  and here is the size, in bytes, of the EVC Status table field defs
   static const U32 FieldDefsSize (void);

   //  here is the name of the PTS table whose rows we define
   static const char *TableName (void);

    //  some PTS interface message typedefs
   typedef RqPtsDefineTable_T<EVCStatusRecord>     RqDefineTable;
   typedef RqPtsDeleteTable_T<EVCStatusRecord>     RqDeleteTable;
   typedef RqPtsQuerySetRID_T<EVCStatusRecord>     RqQuerySetRID;
   typedef RqPtsEnumerateTable_T<EVCStatusRecord>  RqEnumerateTable;
   typedef RqPtsInsertRow_T<EVCStatusRecord>       RqInsertRow;
   typedef RqPtsModifyRow_T<EVCStatusRecord>       RqModifyRow;
   typedef RqPtsModifyField_T<EVCStatusRecord>     RqModifyField;
   typedef RqPtsModifyBits_T<EVCStatusRecord>      RqModifyBits;
   typedef RqPtsTestAndSetField_T<EVCStatusRecord> RqTestAndSetField;
   typedef RqPtsReadRow_T<EVCStatusRecord>         RqReadRow;
   typedef RqPtsDeleteRow_T<EVCStatusRecord>       RqDeleteRow;
   typedef RqPtsListen_T<EVCStatusRecord>          RqListen;

};  /* end of class EVCStatusRecord */


//  here's the keyswitch position enum
enum CT_EVC_KEYPOS {
   CT_KEYPOS_SERVICE  = 1,       // "service" mode (e.g., diags)
   CT_KEYPOS_OFF      = 0,       // power off
   CT_KEYPOS_ON       = 3,       // normal operation
   CT_KEYPOS_SECURITY = 2        // security override
};


//  here are compiler-checkable aliases for PTS field definition names

   // reporter data (is this needed by Env DDM?)
#define  CT_EVCST_KEY            "key"
#define  CT_EVCST_REFRESHRATE    "RefreshRate"

   // Is EVC accessible via the CMB?
#define  CT_EVCST_EVC_REACHABLE  "afEvcReachable"

   //  temperature & related info
#define  CT_EVCST_EXITAIRTEMP             "ExitAirTemp"
#define  CT_EVCST_EXITTEMPFANUPTHRESH     "ExitTempFanUpThresh"
#define  CT_EVCST_EXITTEMPFANNORMTHRESH   "ExitTempFanNormThresh"
#define  CT_EVCST_FANSPEED                "FanSpeed"
#define  CT_EVCST_FANSPEEDSET             "FanSpeedSet"

   //  primary power supply-provided status values
#define  CT_EVCST_FINPUTOK             "afInputOK"
#define  CT_EVCST_FOUTPUTOK            "afOutputOK"
#define  CT_EVCST_FFANFAILOROVERTEMP   "afFanFailOrOverTemp"
#define  CT_EVCST_FPRIMARYENABLE       "afPrimaryEnable"
#define  CT_EVCST_SMP48VOLTAGE         "SMP48Voltage"

   //  auxiliary power supply-provided status values
#define  CT_EVCST_FDCTODCOK            "afDCtoDCOK"
#define  CT_EVCST_DCTODC33CURRENT      "aulDCtoDC33Current"
#define  CT_EVCST_DCTODC5CURRENT       "aulDCtoDC5Current"
#define  CT_EVCST_DCTODC12ACURRENT     "aulDCtoDC12ACurrent"
#define  CT_EVCST_DCTODC12BCURRENT     "aulDCtoDC12BCurrent"
#define  CT_EVCST_DCTODC12CCURRENT     "aulDCtoDC12CCurrent"
#define  CT_EVCST_DCTODC33TEMP         "alDCtoDC33Temp"
#define  CT_EVCST_DCTODC5TEMP          "alDCtoDC5Temp"
#define  CT_EVCST_DCTODC12ATEMP        "alDCtoDC12ATemp"
#define  CT_EVCST_DCTODC12BTEMP        "alDCtoDC12BTemp"
#define  CT_EVCST_DCTODC12CTEMP        "alDCtoDC12CTemp"
#define  CT_EVCST_DCTODC33VOLTAGE      "ulDCtoDC33Voltage"
#define  CT_EVCST_DCTODC5VOLTAGE       "ulDCtoDC5Voltage"
#define  CT_EVCST_DCTODC12VOLTAGE      "ulDCtoDC12Voltage"
#define  CT_EVCST_FDCTODCENABLE        "afDCtoDCEnable"

   //  Odyssey battery / charge status
#define  CT_EVCST_FBATTERYINSTALLED    "afBatteryInstalled"
#define  CT_EVCST_BATTERYTEMPERATURE   "alBatteryTemperature"
#define  CT_EVCST_BATTERYCURRENT       "alBatteryCurrent"
   
   //  misc. Odyssey cabinet features
#define  CT_EVCST_KEYPOSITION          "KeyPosition"


//  these vars are obsolete, and should not be used!
//  Instead, use members FieldDefs() and FieldDefsSize() in class above.

//  here is the standard table which defines EVC Status table PTS fields
extern const fieldDef aEvcStatusTable_FieldDefs[];

//  and here is the size, in bytes, of the EVC Status table field defs
extern const U32 cbEvcStatusTable_FieldDefs;


#endif  // #ifndef _EVCStatusRecord_h

