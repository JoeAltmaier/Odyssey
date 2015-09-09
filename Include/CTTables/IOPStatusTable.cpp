/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Archive: /Gemini/Include/CTTables/IOPStatusTable.cpp $
// 
// Description:
// This file contains the PTS field definitions used to create the
// IOP Status table.
// 
// $Log: /Gemini/Include/CTTables/IOPStatusTable.cpp $
// 
// 8     9/08/99 11:26a Ewedel
// Added debug flag to suppress warning on CHECKFIELDDEFS() macro usage.
// 
// 7     9/03/99 5:10p Ewedel
// Changed to use CPtsRecordBase, and to be friendly to PTS message
// templates.  Also added cross-check of field defs (PTS-evaluated size ==
// C++ class instance size).
// 
// 6     8/26/99 1:42p Tnelson
// 
// 5     8/11/99 7:45p Ewedel
// Modest tweaks to reflect final (I hope!) AVR EEPROM data.
// 
// 4     6/14/99 11:54p Ewedel
// Removed some obsoleted fields.
// 
// 3     6/03/99 5:58p Ewedel
// Updated state values, added separate "desired state" enum, corrected
// serial number fields (16 bytes for CMA-based ones), added PCI window
// parameters.  Also changed Iop_Type to use enum, instead of simple
// integer.
// 
// 2     5/12/99 4:18p Ewedel
// Split IOP state field into two:  current and desired (a.k.a. status &
// control).
// Removed obsolete second set of temperature parameters.
// 
// 1     3/11/99 7:12p Ewedel
// Initial checkin.
//
/*************************************************************************/

#define _DEBUG    /* needed by CHECKFIELDDEFS(), otherwise harmless */
#include  "IOPStatusTable.h"



//  verify that field defs agree with record def
CHECKFIELDDEFS (IOPStatusRecord);


const fieldDef aIopStatusTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName               Size  Type         Persist yes/no

   CPTS_RECORD_BASE_FIELDS (Persistant_PT),
   CT_IOPST_IOP_TYPE,            0, U32_FT,      NotPersistant_PT, // IopType
   CT_IOPST_SLOT,                0, U32_FT,      Persistant_PT,    // TySlot
   CT_IOPST_REDUNDANTSLOT,       0, U32_FT,      Persistant_PT,    // TySlot
   CT_IOPST_MANUFACTURER,       32, STRING32_FT, NotPersistant_PT,
   CT_IOPST_AVRSWVERSION,        0, U32_FT,      Persistant_PT,
   CT_IOPST_AVRSWREVISION,       0, U32_FT,      Persistant_PT,
   CT_IOPST_HWPARTNO,           16, STRING16_FT, Persistant_PT,
   CT_IOPST_HWREVISION,          0, U32_FT,      Persistant_PT,
   CT_IOPST_HWMFGDATE,           0, U32_FT,      Persistant_PT,
   CT_IOPST_SERIALNUMBER,       16, STRING16_FT, NotPersistant_PT,
   CT_IOPST_CHASSISSERIALNUM,   16, STRING16_FT, NotPersistant_PT,
   CT_IOPST_IOPEPLDREVISION,     0, U32_FT,      Persistant_PT,
   CT_IOPST_IOPMIPSSPEED,        0, U32_FT,      Persistant_PT,
   CT_IOPST_IOPPCISPEED,         0, U32_FT,      Persistant_PT,
   CT_IOPST_IOPCURRENTSTATE,     0, U32_FT,      NotPersistant_PT, // IopCurrentState
   CT_IOPST_TEMP,                0, S32_FT,      NotPersistant_PT,
   CT_IOPST_TEMPHITHRESHOLD,     0, S32_FT,      Persistant_PT,
   CT_IOPST_TEMPLOWTHRESHOLD,    0, S32_FT,      Persistant_PT,
 };


//  size of field definition table, in bytes
const U32 cbIopStatusTable_FieldDefs  =  sizeof (aIopStatusTable_FieldDefs);


IOPStatusRecord::IOPStatusRecord (void) :
                        CPtsRecordBase (sizeof (IOPStatusRecord),
                                        CT_IOPST_TABLE_VER)
{


   Clear ();

}  /* end of IOPStatusRecord::IOPStatusRecord */

void  IOPStatusRecord::Clear (void)
{


   IOP_Type = (IopType) 0;
   Slot = RedundantSlot = (TySlot) 0;
   *Manufacturer = 0;
   ulAvrSwVersion = ulAvrSwRevision = 0;
   *strHwPartNo = 0;
   ulHwRevision = ulHwMfgDate = 0;
   *SerialNumber = 0;
   *ChassisSerialNumber = 0;

   ulIopEpldRevision = ulIopMipsSpeed = ulIopPciSpeed = 0;

   eIOPCurrentState = IOPS_UNKNOWN;

   Temp = TempHiThreshold = TempNormThreshold = 0;

}  /* end of IOPStatusRecord::Clear */


//  here are the standard field defs for our row/table
/* static */
const fieldDef *IOPStatusRecord::FieldDefs (void)
{
   return (aIopStatusTable_FieldDefs);
}

//  and here is the size, in bytes, of our field defs
/* static */
const U32 IOPStatusRecord::FieldDefsSize (void)
{
   return (cbIopStatusTable_FieldDefs);
}

//  here is the name of the PTS table whose rows we define
/* static */
const char *IOPStatusRecord::TableName (void)
{
   return (CT_IOPST_TABLE_NAME);
}


