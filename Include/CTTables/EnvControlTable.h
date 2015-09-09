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
//   This file defines the Environment Control record and table.
//   There is one instance of the table, and it contains one row.
//   The single row controls settings fed to both EVCs in the system.
//
//       NOTE:  All columns in this table are read-only!  To change
//              any control settings, use the CMB DDM's control interface.
//
//       Note:  This table is persistent: it is the CMB DDM's means of
//              saving control settings across reboots, failovers, etc.
// 
// $Log: /Gemini/Include/CTTables/EnvControlTable.h $
// 
// 3     10/27/99 12:13p Ewedel
// Corrected description of lsb for exit-temp fan control thresholds.
// 
// 2     9/08/99 9:06a Vnguyen
// Change Temperature Threshold from U32 to S32.
// 
// 1     9/03/99 5:39p Ewedel
// Initial revision.
//
/*************************************************************************/

#ifndef _EnvControlTable_h
#define _EnvControlTable_h


#ifndef _PtsRecordBase_h
# include  "PtsRecordBase.h"
#endif

#ifndef __RqPts_T_h
# include  "RqPts_T.h"
#endif


#pragma  pack(4)

#define CT_ENV_CONTROL_TABLE_VER    (1)      /* current layout version */

//
// EVC Raw Parameters table - Reflects raw data read from one EVC.
//                   Also indicates whether EVC was present to read from,
//                   and if particular classes of data could not be read.
//          NOTE:  All columns in this table are read-only!  To change
//                 any control settings, use the CMB DDM's control interface.
// 
//          There are two entries per table, one for each EVC slot
//
class CtEnvControlRecord : public CPtsRecordBase
{
public:

   //  explicit fan speed settings
   U32      FanSpeedSet[2];         // Set target RPMs for each fan pair:
                                    //  [0] = upper pair, [1] = lower pair

   //  automatic EVC fan speed control thresholds.  The EVCs compare these
   //  values to their DS1720 temperature sensor readings.
   //  These values are given in units (lsb) of 0.5 degrees C, which
   //  coincidentally are just the units desired by the EVC AVR.

   S32      ExitTempFanUpThresh;    // Exit air temp at which fans are
                                    //  auto-forced by EVCs to full speed
   S32      ExitTempFanNormThresh;  // Exit air temp at which fans are restored
                                    //  to their FanSpeedSet[] settings


   //  our constructor, establishes constant fields in the record,
   //  and zeros the rest:
   CtEnvControlRecord (void);

   //  here is the standard table which defines Env Control Table PTS fields
   static const fieldDef *FieldDefs (void);

   //  and here is the size, in bytes, of the Env Control Table field defs
   static const U32 FieldDefsSize (void);

   //  here is the PTS name of our Env Control Table
   static const char *TableName (void);

   //  some PTS interface message typedefs
   typedef RqPtsDefineTable_T    <CtEnvControlRecord>    RqDefineTable;
   typedef RqPtsInsertRow_T      <CtEnvControlRecord>    RqInsertRow;
   typedef RqPtsModifyRow_T      <CtEnvControlRecord>    RqModifyRow;
   typedef RqPtsReadRow_T        <CtEnvControlRecord>    RqReadRow;
   typedef RqPtsEnumerateTable_T <CtEnvControlRecord>    RqEnumTable;

};  /* end of class CtEnvControlRecord */



//  here are compiler-checkable aliases for PTS field definition names

   //  fan control info
#define  CT_ENVCR_EXITTEMPFANUPTHRESH     "ExitTempFanUpThresh"
#define  CT_ENVCR_EXITTEMPFANNORMTHRESH   "ExitTempFanNormThresh"
#define  CT_ENVCR_FANSPEEDSET             "aulFanSpeedSet"



#endif  // #ifndef _EnvControlTable_h

