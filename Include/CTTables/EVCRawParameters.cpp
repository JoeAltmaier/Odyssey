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
// $Archive: /Gemini/Include/CTTables/EVCRawParameters.cpp $
// 
// Description:
//   This file contains the PTS field definitions used to create the
//   EVC Raw Parameters table.
// 
// $Log: /Gemini/Include/CTTables/EVCRawParameters.cpp $
// 
// 10    1/26/00 1:22p Eric_wedel
// Whoops, fixed CT_EVCRP_FDCTODCENABLE definition: it is really two U32s,
// not one.  Thanks to Huy & Vuong for finding this one!
// 
// 9     12/13/99 1:45p Ewedel
// Removed "evc master" support, since it's gone from the CMB level.
// 
// 8     11/08/99 7:51p Ewedel
// Added BatteryPresent[] flag.
// 
// 7     10/19/99 4:42p Ewedel
// Fixed stupid missing comma that should never have been checked in!
// 
// 6     10/19/99 4:14p Ewedel
// Added new "EVC Master slot" field, echoing yet another EVC parameter.
// Also added nice CHECKFIELDDEFS() verifier macro.
// 
// 5     9/08/99 9:14a Vnguyen
// Change temperature threshold from U32 to S32.
// 
// 4     9/03/99 5:04p Ewedel
// Changed to use CPtsRecordBase, made friendly to PTS message templates.
// 
// 3     8/23/99 12:25p Ewedel
// Changed to use new FieldDefs() / FieldDefsSize() scheme.
// 
// 2     8/12/99 7:07p Ewedel
// Changed battery charge current to signed (thanks, Dipam!).
// 
// 1     8/11/99 7:54p Ewedel
// Initial revision.
//
/*************************************************************************/

#include  "EVCRawParameters.h"

#include  "EvcStatusRecord.h"    // for key position enum CT_EVC_KEYPOS

#include  <string.h>


//  verify that our fields defs are as big as our record:
CHECKFIELDDEFS(CtEVCRawParameterRecord);


//  here's the raw array, which we keep private:

static const fieldDef aEvcRawParamTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName                  Size  Type         Persist yes/no

   CPTS_RECORD_BASE_FIELDS (NotPersistant_PT),
   CT_EVCRP_EVCSLOTID,              0, U32_FT,      NotPersistant_PT,
   CT_EVCRP_EVC_REACHABLE,          0, U32_FT,      NotPersistant_PT,

   CT_EVCRP_EXITAIRTEMP,            0, S32_FT,      NotPersistant_PT,
   CT_EVCRP_EXITTEMPFANUPTHRESH,    0, S32_FT,      NotPersistant_PT,
   CT_EVCRP_EXITTEMPFANNORMTHRESH,  0, S32_FT,      NotPersistant_PT,
   CT_EVCRP_FANSPEED,              16, U32_FT,      NotPersistant_PT,   // 4xU32
   CT_EVCRP_FANSPEEDSET,            8, U32_FT,      NotPersistant_PT,   // 2xU32

   CT_EVCRP_FINPUTOK,              12, U32_FT,      NotPersistant_PT,   // 3xU32
   CT_EVCRP_FOUTPUTOK,             12, U32_FT,      NotPersistant_PT,   // 3xU32
   CT_EVCRP_FFANFAILOROVERTEMP,    12, U32_FT,      NotPersistant_PT,   // 3xU32
   CT_EVCRP_FPRIMARYENABLE,        12, U32_FT,      NotPersistant_PT,   // 3xU32

   CT_EVCRP_DCTODC33CURRENT,        0, U32_FT,      NotPersistant_PT,
   CT_EVCRP_DCTODC5CURRENT,         0, U32_FT,      NotPersistant_PT,
   CT_EVCRP_DCTODC12ACURRENT,       0, U32_FT,      NotPersistant_PT,
   CT_EVCRP_DCTODC12BCURRENT,       0, U32_FT,      NotPersistant_PT,
   CT_EVCRP_DCTODC12CCURRENT,       0, U32_FT,      NotPersistant_PT,
   CT_EVCRP_DCTODC33TEMP,           0, S32_FT,      NotPersistant_PT,
   CT_EVCRP_DCTODC5TEMP,            0, S32_FT,      NotPersistant_PT,
   CT_EVCRP_DCTODC12ATEMP,          0, S32_FT,      NotPersistant_PT,
   CT_EVCRP_DCTODC12BTEMP,          0, S32_FT,      NotPersistant_PT,
   CT_EVCRP_DCTODC12CTEMP,          0, S32_FT,      NotPersistant_PT,
   CT_EVCRP_FDCTODCENABLE,          8, U32_FT,      NotPersistant_PT,   // 2xBOOL

   CT_EVCRP_SMP48VOLTAGE,           0, U32_FT,      NotPersistant_PT,
   CT_EVCRP_DCTODC33VOLTAGE,        0, U32_FT,      NotPersistant_PT,
   CT_EVCRP_DCTODC5VOLTAGE,         0, U32_FT,      NotPersistant_PT,
   CT_EVCRP_DCTODC12VOLTAGE,        0, U32_FT,      NotPersistant_PT,
   CT_EVCRP_KEYPOSITION,            0, U32_FT,      NotPersistant_PT,

   CT_EVCRP_BATTERYTEMPERATURE,     8, S32_FT,      NotPersistant_PT,   // 2xS32
   CT_EVCRP_BATTERYCURRENT,         8, S32_FT,      NotPersistant_PT,   // 2xS32
   CT_EVCRP_BATTERYPRESENT,         8, U32_FT,      NotPersistant_PT,   // 2xBOOL
};


//  our constructor - a simple beast
CtEVCRawParameterRecord::CtEVCRawParameterRecord (void) :
                        CPtsRecordBase (sizeof (CtEVCRawParameterRecord),
                                        CT_EVC_RAW_PARAM_TABLE_VER)
{

   EvcSlotId = (TySlot) 0;
   fEvcReachable = FALSE;
   ExitAirTemp = 0;
   ExitTempFanUpThresh = ExitTempFanNormThresh = 0;
   memset (FanSpeed, 0, sizeof (FanSpeed));
   memset (FanSpeedSet, 0, sizeof (FanSpeedSet));

   memset (fInputOK, 0, sizeof (fInputOK));
   memset (fOutputOK, 0, sizeof (fOutputOK));
   memset (fFanFailOrOverTemp, 0, sizeof (fFanFailOrOverTemp));
   memset (fPrimaryEnable, 0, sizeof (fPrimaryEnable));

   DCtoDC33Current = DCtoDC5Current = 0;
   DCtoDC12ACurrent = DCtoDC12BCurrent = DCtoDC12CCurrent = 0;
   DCtoDC33Temp = DCtoDC5Temp = 0;
   DCtoDC12ATemp = DCtoDC12BTemp = DCtoDC12CTemp = 0;
   memset (fDCtoDCEnable, 0, sizeof (fDCtoDCEnable));

   SMP48Voltage = 0;
   DCtoDC33Voltage = DCtoDC5Voltage = DCtoDC12Voltage = 0;
   KeyPosition = (CT_EVC_KEYPOS) 0;

   memset (BatteryTemperature, 0, sizeof (BatteryTemperature));
   memset (BatteryCurrent, 0, sizeof (BatteryCurrent));

   return;

}  /* end of CtEVCRawParameterRecord::CtEVCRawParameterRecord */


//  here are the routines which make the table defs publicly visible:

//  the name of the PTS table whose rows we define
const char *CtEVCRawParameterRecord::TableName (void)
{
   return (CT_EVC_RAW_PARAM_TABLE);
}

//  here is the standard table which defines EVC Raw Param table PTS fields
const fieldDef *CtEVCRawParameterRecord::FieldDefs (void)
{
   return (aEvcRawParamTable_FieldDefs);
}


//  and here is the size, in bytes, of the EVC Raw Param table field defs
const U32 CtEVCRawParameterRecord::FieldDefsSize (void)
{
   return (sizeof (aEvcRawParamTable_FieldDefs));
}




