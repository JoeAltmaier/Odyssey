/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CDB.h
// 
// Description:
// This file describes interfaces for for the SCSI Command Data Block.
// 
// Update Log 
// 
// 4/14/98 Jim Frandeen: Create file
// 5/5/98 Jim Frandeen: Use C++ comment style
// 8/3/98 Michael G. Panas: Remove all structs (use scsi.h)
// 9/2/98 Michael G. Panas: add C++ stuff
/*************************************************************************/


/*************************************************************************/

#if !defined(CDB_H)
#define CDB_H

#include "CTtypes.h"
#include "scsi.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


/*************************************************************************/
//    Accessor methods
/*************************************************************************/
U32 CDB_Get_CDB_Length(CDB16*);
U32 CDB_Get_Logical_Block_Address(CDB16*);
void CDB_Set_Logical_Block_Address(CDB16*, U32);

U32 CDB_Get_Transfer_Length(CDB16*);
void CDB_Set_Transfer_Length(CDB16*, U32);

U8 CDB_Get_Control(CDB16*);
void CDB_Set_Control(CDB16*, U8);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif // CDB_H