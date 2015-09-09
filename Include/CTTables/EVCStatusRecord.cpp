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
// $Archive: /Gemini/Include/CTTables/EVCStatusRecord.cpp $
// 
// Description:
//   This file contains the PTS field definitions used to create the
//   EVC Status table.
// 
// $Log: /Gemini/Include/CTTables/EVCStatusRecord.cpp $
// 
// 16    12/14/99 8:11p Ewedel
// Removed obsolete EVC Master stuff.  [VN]
// 
// 15    12/01/99 4:48p Ewedel
// Changed to derive from CPtsRecordBase.  Added real constructor.
// 
// 14    10/21/99 4:46p Vnguyen
// Add entry for EvcMasterSlot per change in EvcRawParameters.h
// 
// 13    10/08/99 4:44p Vnguyen
// Change FieldDefSize to constant type.
// 
// 12    9/08/99 9:54a Vnguyen
// Add fPrimaryEnable array.
// 
// 11    9/08/99 9:16a Vnguyen
// Change temperature from U32 to S32.
// 
// 10    8/23/99 12:27p Ewedel
// Added FieldDefs() / FieldDefsSize() support.
// 
// 9     8/20/99 3:30p Vnguyen
// Add key, RefreshRate fields for PHS Reporter.
// 
// 8     8/18/99 7:56a Vnguyen
// Delete ACTIVEIOPSMASK field.  Add afEVCReachable[2] field.
// 
// 7     8/12/99 7:08p Ewedel
// Changed battery charge current to signed (thanks, Dipam!).
// 
// 6     8/11/99 7:43p Ewedel
// Changed to reflect consolidated EVC reporting, and removed serial
// number fields (they are in the IOP Status table, which also reports on
// EVCs and DDHs).
// Detailed per-EVC data may now be found in EvcRawParameters.h, .cpp.
// 
// 5     8/08/99 11:07a Jlane
// Added ActiveIOPsMask to EVCStatus record.  This word has TySlot bits
// set indicating active IOPs.  Maintained by OS/BootMgr and used by
// Masters instantiating DDMs.
// 
// 4     7/30/99 3:47p Ewedel
// Grouped various global params, and array-ized them to reflect separate
// readings from each EVC.
// 
// 3     7/27/99 6:43p Ewedel
// Updated to reflect latest [E2] EVC hardware defs.
// 
// 2     6/03/99 5:54p Ewedel
// Renumber exhaust air temp readings for consistency with EVC IDs.
// Changed fan speed setting array per current hardware (only two
// independent controls).  Added full complement of serial numbers (two
// per each EVC: board and chassis).
// 
// 1     3/16/99 5:50p Ewedel
// Initial checkin.
//
/*************************************************************************/

#include  "EVCStatusRecord.h"


//  verify that field defs agree with record def
CHECKFIELDDEFS (EVCStatusRecord);


//  should be private (static), but left public for legacy support:
const fieldDef aEvcStatusTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName                  Size  Type         Persist yes/no

   CPTS_RECORD_BASE_FIELDS (Persistant_PT),
   CT_EVCST_KEY,                    0, U32_FT,      NotPersistant_PT,
   CT_EVCST_REFRESHRATE,            0, U32_FT,      Persistant_PT,

   CT_EVCST_EVC_REACHABLE,         	8, U32_FT,      NotPersistant_PT,	// 2xU32
   CT_EVCST_EXITAIRTEMP,            8, S32_FT,      NotPersistant_PT,   // 2xS32
   CT_EVCST_EXITTEMPFANUPTHRESH,    0, S32_FT,      Persistant_PT,
   CT_EVCST_EXITTEMPFANNORMTHRESH,  0, S32_FT,      Persistant_PT,
   CT_EVCST_FANSPEED,              16, U32_FT,      NotPersistant_PT,   // 4xU32
   CT_EVCST_FANSPEEDSET,            8, U32_FT,      Persistant_PT,      // 2xU32

   CT_EVCST_FINPUTOK,              12, U32_FT,      NotPersistant_PT,   // 3xU32
   CT_EVCST_FOUTPUTOK,             12, U32_FT,      NotPersistant_PT,   // 3xU32
   CT_EVCST_FFANFAILOROVERTEMP,    12, U32_FT,      NotPersistant_PT,   // 3xU32
   CT_EVCST_FPRIMARYENABLE,        12, U32_FT,      NotPersistant_PT,   // 3xU32

   CT_EVCST_DCTODC33CURRENT,        8, U32_FT,      NotPersistant_PT,   // 2xU32
   CT_EVCST_DCTODC5CURRENT,         8, U32_FT,      NotPersistant_PT,   // 2xU32
   CT_EVCST_DCTODC12ACURRENT,       8, U32_FT,      NotPersistant_PT,   // 2xU32
   CT_EVCST_DCTODC12BCURRENT,       8, U32_FT,      NotPersistant_PT,   // 2xU32
   CT_EVCST_DCTODC12CCURRENT,       8, U32_FT,      NotPersistant_PT,   // 2xU32
   CT_EVCST_DCTODC33TEMP,           8, S32_FT,      NotPersistant_PT,   // 2xS32
   CT_EVCST_DCTODC5TEMP,            8, S32_FT,      NotPersistant_PT,   // 2xS32
   CT_EVCST_DCTODC12ATEMP,          8, S32_FT,      NotPersistant_PT,   // 2xS32
   CT_EVCST_DCTODC12BTEMP,          8, S32_FT,      NotPersistant_PT,   // 2xS32
   CT_EVCST_DCTODC12CTEMP,          8, S32_FT,      NotPersistant_PT,   // 2xS32
   CT_EVCST_FDCTODCENABLE,          8, U32_FT,      NotPersistant_PT,   // 2xU32

   CT_EVCST_SMP48VOLTAGE,           0, U32_FT,      NotPersistant_PT,
   CT_EVCST_DCTODC33VOLTAGE,        0, U32_FT,      NotPersistant_PT,
   CT_EVCST_DCTODC5VOLTAGE,         0, U32_FT,      NotPersistant_PT,
   CT_EVCST_DCTODC12VOLTAGE,        0, U32_FT,      NotPersistant_PT,
   CT_EVCST_KEYPOSITION,            0, U32_FT,      NotPersistant_PT,

   CT_EVCST_FBATTERYINSTALLED,      8, U32_FT,      NotPersistant_PT,   // 2xU32
   CT_EVCST_BATTERYTEMPERATURE,     8, S32_FT,      NotPersistant_PT,   // 2xS32
   CT_EVCST_BATTERYCURRENT,         8, S32_FT,      NotPersistant_PT    // 2xS32
};


//  size of field definition table, in bytes
const U32 cbEvcStatusTable_FieldDefs  =  sizeof (aEvcStatusTable_FieldDefs);


EVCStatusRecord::EVCStatusRecord () : CPtsRecordBase(sizeof (EVCStatusRecord),
                                                     EVC_STATUS_TABLE_VER)
{

   //  we initialize our whole record to zero.

   key = 0;
   RefreshRate = 0;
   afEvcReachable[0] = afEvcReachable[1] = FALSE;

   ExitAirTemp[0] = ExitAirTemp[1] = 0;
   ExitTempFanUpThresh = 0;
   ExitTempFanNormThresh = 0;
   FanSpeed[0] = FanSpeed[1] = FanSpeed[2] = FanSpeed[3] = 0;
   FanSpeedSet[0] = FanSpeedSet[1] = 0;

   fInputOK[0]           = fInputOK[1]           = fInputOK[2]           = 0;
   fOutputOK[0]          = fOutputOK[1]          = fOutputOK[2]          = 0;
   fFanFailOrOverTemp[0] = fFanFailOrOverTemp[1] = fFanFailOrOverTemp[2] = 0;
   fPrimaryEnable[0]     = fPrimaryEnable[1]     = fPrimaryEnable[2]     = 0;

   DCtoDC33Current[0]  = DCtoDC33Current[1]  = 0;
   DCtoDC5Current[0]   = DCtoDC5Current[1]   = 0;
   DCtoDC12ACurrent[0] = DCtoDC12BCurrent[1] = 0;
   DCtoDC12CCurrent[0] = DCtoDC12CCurrent[1] = 0;
   DCtoDC33Temp[0]     = DCtoDC33Temp[1]     = 0;
   DCtoDC5Temp[0]      = DCtoDC5Temp[1]      = 0;
   DCtoDC12ATemp[0]    = DCtoDC12ATemp[1]    = 0;
   DCtoDC12BTemp[0]    = DCtoDC12BTemp[1]    = 0;
   DCtoDC12CTemp[0]    = DCtoDC12CTemp[1]    = 0;
   fDCtoDCEnable[0]    = fDCtoDCEnable[1]    = 0;

   SMP48Voltage    = 0;
   DCtoDC33Voltage = 0;
   DCtoDC5Voltage  = 0;
   DCtoDC12Voltage = 0;
   KeyPosition     = CT_KEYPOS_ON;     // a safe default

   fBatteryInstalled[0]  = fBatteryInstalled[1]  = 0;
   BatteryTemperature[0] = BatteryTemperature[1] = 0;
   BatteryCurrent[0]     = BatteryCurrent[1]     = 0;

}  /* end of EVCStatusRecord::EVCStatusRecord */


//  here is the standard table which defines EVC Status table PTS fields
const fieldDef *EVCStatusRecord::FieldDefs (void)
{
   return (aEvcStatusTable_FieldDefs);
}


//  and here is the size, in bytes, of the EVC Status table field defs
const U32 EVCStatusRecord::FieldDefsSize (void)
{
   return (sizeof (aEvcStatusTable_FieldDefs));
}


//  here is the name of the PTS table whose rows we define
const char *EVCStatusRecord::TableName (void)
{
   return ("EVC_Status_Table");
}

