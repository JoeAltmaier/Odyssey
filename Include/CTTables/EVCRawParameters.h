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
//   This file defines the EVC Raw Parameters record and table.
//   There is one instance of the table, and it contains two rows,
//   one reflecting data read from each physical EVC.
//   Note that both rows are always present, even if one of the EVCs
//   is not installed in the Odyssey.
//
//       NOTE:  All columns in this table are read-only!  To change
//              any control settings, use the CMB DDM's control interface.
//
//       Note:  This table is entirely non-persistent.  This is because
//              it is meant to mirror exactly what is in the EVC's AVR
//              processors at any given time.  No tuning settings are
//              stored in this table.  The few columns which suggest that
//              are actually reflections of the true values in the AVRs.
// 
// $Log: /Gemini/Include/CTTables/EVCRawParameters.h $
// 
// 9     12/13/99 1:45p Ewedel
// Removed "evc master" support, since it's gone from the CMB level.
// 
// 8     11/18/99 7:21p Ewedel
// Updated fan speed comments.
// 
// 7     11/08/99 7:50p Ewedel
// Added BatteryPresent[] flag.
// 
// 6     10/19/99 4:13p Ewedel
// Added new "EVC Master slot" field, echoing yet another EVC parameter.
// 
// 5     9/08/99 9:14a Vnguyen
// Change temperature threshold from U32 to S32.
// 
// 4     9/03/99 5:05p Ewedel
// Changed to use CPtsRecordBase, made friendly to PTS message templates,
// and updated units (lsb) notes.
// 
// 3     8/23/99 12:25p Ewedel
// Changed to use new FieldDefs() / FieldDefsSize() scheme.
// 
// 2     8/12/99 7:08p Ewedel
// Changed battery charge current to signed (thanks, Dipam!).
// 
// 1     8/11/99 7:54p Ewedel
// Initial revision.
//
/*************************************************************************/

#ifndef _EVCRawParameters_h
#define _EVCRawParameters_h


#ifndef _PtsRecordBase_h
# include  "PtsRecordBase.h"
#endif

#ifndef PtsCommon_H
# include  "PtsCommon.h"
#endif


#pragma  pack(4)

#define CT_EVC_RAW_PARAM_TABLE   "EVC_Raw_Param_Table"

#define CT_EVC_RAW_PARAM_TABLE_VER  (1)      /* current struct version */

//
// EVC Raw Parameters table - Reflects raw data read from one EVC.
//                   Also indicates whether EVC was present to read from,
//                   and if particular classes of data could not be read.
//          NOTE:  All columns in this table are read-only!  To change
//                 any control settings, use the CMB DDM's control interface.
// 
//          There are two entries per table, one for each EVC slot
//
class CtEVCRawParameterRecord : public CPtsRecordBase
{
public:

   //  CMB slot ID of this record's EVC
   U32      EvcSlotId;              // a TySlot value: CMB_EVC0 or CMB_EVC1

   //  global flag:  is EVC accessible via CMB bus?
   U32      fEvcReachable;          // FALSE -> disregard all other params in row

   //  cabinet temperature & related info
   S32      ExitAirTemp;            // Temperature of air exiting the enclosure
                                    //  (read by EVC's DS1720 sensor)
   S32      ExitTempFanUpThresh;    // Exit air temp at which fans are
                                    //  auto-forced by EVCs to full speed
   S32      ExitTempFanNormThresh;  // Exit air temp at which fans are restored
                                    //  to their FanSpeedSet[] settings
   U32      FanSpeed[4];            // Current % of full speed for each fan
   U32      FanSpeedSet[2];         // Current target % of full speed for
                                    //  each fan pair:
                                    //  [0] = upper pair, [1] = lower pair

   //  primary power supply-related status values (all read-only)
   U32      fInputOK[3];            // Input (A/C) power OK (bool)
   U32      fOutputOK[3];           // Power Supply Output OK (bool)
   U32      fFanFailOrOverTemp[3];  // combination fan failure / over temp
                                    // alert. (bool)
   U32      fPrimaryEnable[3];      // which supplies has EVC enabled?  (bool)

   //  auxiliary power supply-provided status values (all read-only)
   U32      DCtoDC33Current;        // Current output of 3.3V converter (1ma lsb)
   U32      DCtoDC5Current;         // Current output of 5V converter (1ma lsb)
   U32      DCtoDC12ACurrent;       // Current output of 12V 'A' conv. (1ma lsb)
                                    //  (presently unreadable, left at 0)
   U32      DCtoDC12BCurrent;       // Current output of 12V 'B' conv. (1ma lsb)
   U32      DCtoDC12CCurrent;       // Current output of 12V 'C' conv. (1ma lsb)
   S32      DCtoDC33Temp;           // Temperature of 3.3V converter (degrees C)
   S32      DCtoDC5Temp;            // Temperature of 5V converter (degrees C)
   S32      DCtoDC12ATemp;          // Temperature of 12V 'A' conv. (degrees C)
   S32      DCtoDC12BTemp;          // Temperature of 12V 'B' conv. (degrees C)
   S32      DCtoDC12CTemp;          // Temperature of 12V 'C' conv. (degrees C)
   U32      fDCtoDCEnable[2];       // Is given aux supply's output enabled
                                    //  (3.3, 5, 12 combined; flag value is bool)

   //  SMP power status.  The same SMP lines are read and reported
   //  separately by each EVC.
   U32      SMP48Voltage;           // combined 48V primary supply output (millivolts)
   U32      DCtoDC33Voltage;        // combined output of both auxes (millivolts)
   U32      DCtoDC5Voltage;         // combined output of both auxes (millivolts)
   U32      DCtoDC12Voltage;        // combined output of both auxes (millivolts)
   U32      KeyPosition;            // enum CT_EVC_KEYPOS (EvcStatusRecord.h)

   //  Odyssey battery / charge status, one set per battery FRU (all read-only)
   S32      BatteryTemperature[2];  // temperature inside each battery pack
                                    //  (degrees C, somewhere between 2 cells)
   S32      BatteryCurrent[2];      // > 0 charging, < 0 discharging? (lsb = 1ma)
   U32      BatteryPresent[2];      // BOOL: TRUE = present, FALSE = not

   //  our constructor, establishes constant fields in the record,
   //  and zeroes the rest:
   CtEVCRawParameterRecord (void);

   //  here is the standard table which defines EVC Raw Param table PTS fields
   static const fieldDef *FieldDefs (void);

   //  and here is the size, in bytes, of the EVC Raw Param table field defs
   static const U32 FieldDefsSize (void);

   //  here is the name of the PTS table whose rows we define
   static const char *TableName (void);

};  /* end of class CtEVCRawParameterRecord */



//  here are compiler-checkable aliases for PTS field definition names

#define  CT_EVCRP_EVCSLOTID      "ulEvcSlotId"
#define  CT_EVCRP_EVC_REACHABLE  "fEvcReachable"

   //  temperature & related info
#define  CT_EVCRP_EXITAIRTEMP             "ExitAirTemp"
#define  CT_EVCRP_EXITTEMPFANUPTHRESH     "ExitTempFanUpThresh"
#define  CT_EVCRP_EXITTEMPFANNORMTHRESH   "ExitTempFanNormThresh"
#define  CT_EVCRP_FANSPEED                "aulFanSpeed"
#define  CT_EVCRP_FANSPEEDSET             "aulFanSpeedSet"

   //  primary power supply-related status values
#define  CT_EVCRP_FINPUTOK             "afInputOK"
#define  CT_EVCRP_FOUTPUTOK            "afOutputOK"
#define  CT_EVCRP_FFANFAILOROVERTEMP   "afFanFailOrOverTemp"
#define  CT_EVCRP_FPRIMARYENABLE       "afPrimaryEnable"
#define  CT_EVCRP_SMP48VOLTAGE         "SMP48Voltage"

   //  auxiliary power supply-provided status values
#define  CT_EVCRP_DCTODC33CURRENT      "ulDCtoDC33Current"
#define  CT_EVCRP_DCTODC5CURRENT       "ulDCtoDC5Current"
#define  CT_EVCRP_DCTODC12ACURRENT     "ulDCtoDC12ACurrent"
#define  CT_EVCRP_DCTODC12BCURRENT     "ulDCtoDC12BCurrent"
#define  CT_EVCRP_DCTODC12CCURRENT     "ulDCtoDC12CCurrent"
#define  CT_EVCRP_DCTODC33TEMP         "lDCtoDC33Temp"
#define  CT_EVCRP_DCTODC5TEMP          "lDCtoDC5Temp"
#define  CT_EVCRP_DCTODC12ATEMP        "lDCtoDC12ATemp"
#define  CT_EVCRP_DCTODC12BTEMP        "lDCtoDC12BTemp"
#define  CT_EVCRP_DCTODC12CTEMP        "lDCtoDC12CTemp"
#define  CT_EVCRP_DCTODC33VOLTAGE      "ulDCtoDC33Voltage"
#define  CT_EVCRP_DCTODC5VOLTAGE       "ulDCtoDC5Voltage"
#define  CT_EVCRP_DCTODC12VOLTAGE      "ulDCtoDC12Voltage"
#define  CT_EVCRP_FDCTODCENABLE        "afDCtoDCEnable"

   //  Odyssey battery / charge status
#define  CT_EVCRP_BATTERYTEMPERATURE   "alBatteryTemperature"
#define  CT_EVCRP_BATTERYCURRENT       "alBatteryCurrent"
#define  CT_EVCRP_BATTERYPRESENT       "alBatteryPresent"
   
   //  misc. Odyssey cabinet features
#define  CT_EVCRP_KEYPOSITION          "KeyPosition"



#endif  // #ifndef _EVCRawParameters_h

