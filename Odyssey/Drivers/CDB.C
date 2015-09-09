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
// This file implements interfaces for for the SCSI Command Data Block.
// 
// Update Log 
// 
// 4/14/98 Jim Frandeen: Create file
// 5/5/98 Jim Frandeen: Use C++ comment style
// 7/28/98 Michael Panas: need explicit casting for U8 since it is an 8 bit
//			signed value. Also add in all the 12 byte values.
// 8/11/98 Michael G. Panas: Add some special cases; Reserve6, Release6
// 8/13/98 Michael G. Panas: Handle alignment problems, add new function to
//			return the CDB Length
/*************************************************************************/
#include "CDB.h"

#define	SPECIAL_6				256		// special case for 6 byte CDB with 16 bit len
#define	SPECIAL_6_NO_LEN		257		// special case for 6 byte CDB with no len field
#define	SPECIAL_10_NO_LEN		258		// special case for 10 byte CDB with no len field
#define	SPECIAL_12_NO_LEN		259		// special case for 12 byte CDB with no len field
#define	SPECIAL_256				260		// special for 6 byte block commands
#define	SPECIAL_10_24_BIT_LEN	261		// special case for 10 byte CDB with 24 Bit len field
#define	SPECIAL_10_8_BIT_LEN	262		// special case for 10 byte CDB with 8 Bit len field

/*************************************************************************/
//    The following table maps CDB command code to CDB size
//    for direct access commands.
//    Commands that are not direct access are mapped to zero.
//    For some commands such as Read6 and Write6, a zero
//    length translates to 256. 256 is a 6-byte CDB
/*************************************************************************/
int MapCommandToCDBSize[] =
{
/*
Operation   Command                             CDB
Code        Name                                size    */

/*  00h      TEST UNIT READY                 */  SPECIAL_6_NO_LEN,
/*  01h      REWIND                          */  0,
/*  02h                                      */  0,
/*  03h      REQUEST SENSE                   */  6,
/*  04h      FORMAT UNIT                     */  6,
/*  05h      READ BLOCK LIMITS               */  SPECIAL_6_NO_LEN,
/*  06h                                      */  0,
/*  07h      REASSIGN BLOCKS                 */  6,
/*  08h      READ                            */  SPECIAL_256,
/*  09h                                      */  0,
/*  0Ah      WRITE                           */  SPECIAL_256,
/*  0Bh      SEEK                            */  0,		// Obsolete
/*  0Ch                                      */  0,
/*  0Dh                                      */  0,
/*  0Eh                                      */  0,
/*  0Fh      READ REVERSE                    */  0,
/*  10h      WRITE FILEMARKS                 */  0,
/*  11h      SPACE                           */  0,
/*  12h      INQUIRY                         */  6,
/*  13h      VERIFY                          */  0,
/*  14h      RECOVER BUFFERED DATA           */  0,
/*  15h      MODE SELECT                     */  6,
/*  16h      RESERVE                         */  SPECIAL_6,
/*  17h      RELEASE                         */  SPECIAL_6_NO_LEN,
/*  18h      COPY                            */  6,
/*  19h      ERASE                           */  0,
/*  1Ah      MODE SENSE                      */  6,
/*  1Bh      START STOP UNIT                 */  SPECIAL_6_NO_LEN,
/*  1Ch      RECEIVE DIAGNOSTIC RESULTS      */  SPECIAL_6,
/*  1Dh      SEND DIAGNOSTIC                 */  SPECIAL_6,
/*  1Eh      PREVENT ALLOW MEDIUM REMOVAL    */  SPECIAL_6_NO_LEN,
/*  1Fh                                      */  0,
/*  20h                                      */  0,
/*  21h                                      */  0,
/*  22h                                      */  0,
/*  23h                                      */  0,
/*  24h      SET WINDOW                      */  SPECIAL_10_NO_LEN,
/*  25h      READ CAPACITY                   */  SPECIAL_10_NO_LEN,
/*  26h                                      */  0,
/*  27h                                      */  0,
/*  28h      READ                            */  10,
/*  29h      READ GENERATION                 */  SPECIAL_10_NO_LEN,
/*  2Ah      WRITE                           */  10,
/*  2Bh      SEEK                            */  10,
/*  2Ch      ERASE                           */  10,
/*  2Dh      READ UPDATED BLOCK              */  0,
/*  2Eh      WRITE AND VERIFY                */  10,
/*  2Fh      VERIFY                          */  10,
/*  30h      SEARCH DATA HIGH                */  0,
/*  31h      SEARCH DATA EQUAL               */  0,
/*  32h      SEARCH DATA LOW                 */  0,
/*  33h      SET LIMITS                      */  10,
/*  34h      PRE-FETCH                       */  10,
/*  35h      SYNCHRONIZE CACHE               */  10,
/*  36h      LOCK UNLOCK CACHE               */  10,
/*  37h      READ DEFECT DATA                */  10,
/*  38h      MEDIUM SCAN                     */  0,
/*  39h      COMPARE                         */  SPECIAL_10_NO_LEN,
/*  3Ah      COPY AND VERIFY                 */  SPECIAL_10_NO_LEN,
/*  3Bh      WRITE BUFFER                    */  SPECIAL_10_24_BIT_LEN,
/*  3Ch      READ BUFFER                     */  SPECIAL_10_24_BIT_LEN,
/*  3Dh      UPDATE BLOCK                    */  0,
/*  3Eh      READ LONG                       */  10,
/*  3Fh      WRITE LONG                      */  10,
/*  40h      CHANGE DEFINITION               */  SPECIAL_10_8_BIT_LEN,
/*  41h      WRITE SAME                      */  10,
/*  42h      READ SUB-CHANNEL                */  0,
/*  43h      READ TOC                        */  0,
/*  44h      REPORT DENSITY SUPPORT          */  0,
/*  45h      PLAY AUDIO                      */  0,
/*  46h                                      */  0,
/*  47h      PLAY AUDIO MSF                  */  0,
/*  48h      PLAY AUDIO TRACK INDEX          */  0,
/*  49h      PLAY TRACK RELATIVE             */  0,
/*  4Ah                                      */  0,
/*  4Bh      PAUSE/RESUME                    */  0,
/*  4Ch      LOG SELECT                      */  10,
/*  4Dh      LOG SENSE                       */  10,
/*  4Eh                                      */  0,
/*  4Fh                                      */  0,
/*  50h      XDWRITE                         */  10,
/*  51h      XPWRITE                         */  10,
/*  52h      XDREAD                          */  10,
/*  53h                                      */  0,
/*  54h                                      */  0,
/*  55h      MODE SELECT                     */  10,
/*  56h      RESERVE                         */  10,
/*  57h      RELEASE                         */  10,
/*  58h                                      */  0,
/*  59h                                      */  0,
/*  5Ah      MODE SENSE                      */  10,
/*  5Bh                                      */  0,
/*  5Ch                                      */  0,
/*  5Dh                                      */  0,
/*  5Eh      PERSISTENT RESERVATION IN       */  10,
/*  5Fh      PERSISTENT RESERVATION OUT      */  10,
/*  60h                                      */  0,
/*  61h                                      */  0,
/*  62h                                      */  0,
/*  63h                                      */  0,
/*  64h                                      */  0,
/*  65h                                      */  0,
/*  66h                                      */  0,
/*  67h                                      */  0,
/*  68h                                      */  0,
/*  69h                                      */  0,
/*  6Ah                                      */  0,
/*  6Bh                                      */  0,
/*  6Ch                                      */  0,
/*  6Dh                                      */  0,
/*  6Eh                                      */  0,
/*  6Fh                                      */  0,
/*  70h                                      */  0,
/*  71h                                      */  0,
/*  72h                                      */  0,
/*  73h                                      */  0,
/*  74h                                      */  0,
/*  75h                                      */  0,
/*  76h                                      */  0,
/*  77h                                      */  0,
/*  78h                                      */  0,
/*  79h                                      */  0,
/*  7Ah                                      */  0,
/*  7Bh                                      */  0,
/*  7Ch                                      */  0,
/*  7Dh                                      */  0,
/*  7Eh                                      */  0,
/*  7Fh                                      */  0,
/*  80h      XDWRITE EXTENDED                */  16,
/*  81h      REBUILD                         */  16,
/*  82h      REGENERATE                      */  16,
/*  83h                                      */  0,
/*  84h                                      */  0,
/*  85h                                      */  0,
/*  86h                                      */  0,
/*  87h                                      */  0,
/*  88h                                      */  0,
/*  89h                                      */  0,
/*  8Ah                                      */  0,
/*  8Bh                                      */  0,
/*  8Ch                                      */  0,
/*  8Dh                                      */  0,
/*  8Eh                                      */  0,
/*  8Fh                                      */  0,
/*  90h                                      */  0,
/*  91h                                      */  0,
/*  92h                                      */  0,
/*  93h                                      */  0,
/*  94h                                      */  0,
/*  95h                                      */  0,
/*  96h                                      */  0,
/*  97h                                      */  0,
/*  98h                                      */  0,
/*  99h                                      */  0,
/*  9Ah                                      */  0,
/*  9Bh                                      */  0,
/*  9Ch                                      */  0,
/*  9Dh                                      */  0,
/*  9Eh                                      */  0,
/*  9Fh                                      */  0,
/*  A0h      REPORT LUNS                     */  12,
/*  A1h                                      */  0,
/*  A2h                                      */  0,
/*  A3h                                      */  0,
/*  A4h                                      */  0,
/*  A5h                                      */  0,
/*  A6h                                      */  0,
/*  A7h                                      */  0,
/*  A8h                                      */  0,
/*  A9h                                      */  0,
/*  AAh                                      */  0,
/*  ABh                                      */  0,
/*  ACh                                      */  0,
/*  ADh                                      */  0,
/*  AEh                                      */  0,
/*  AFh                                      */  0,
/*  B0h                                      */  0,
/*  B1h                                      */  0,
/*  B2h                                      */  0,
/*  B3h                                      */  0,
/*  B4h                                      */  0,
/*  B5h                                      */  0,
/*  B6h                                      */  0,
/*  B7h                                      */  0,
/*  B8h                                      */  0,
/*  B9h                                      */  0,
/*  BAh                                      */  0,
/*  BBh                                      */  0,
/*  BCh                                      */  0,
/*  BDh                                      */  0,
/*  BEh                                      */  0,
/*  BFh                                      */  0,
/*  C0h                                      */  0,
/*  C1h                                      */  0,
/*  C2h                                      */  0,
/*  C3h                                      */  0,
/*  C4h                                      */  0,
/*  C5h                                      */  0,
/*  C6h                                      */  0,
/*  C7h                                      */  0,
/*  C8h                                      */  0,
/*  C9h                                      */  0,
/*  CAh                                      */  0,
/*  CBh                                      */  0,
/*  CCh                                      */  0,
/*  CDh                                      */  0,
/*  CEh                                      */  0,
/*  CFh                                      */  0,
/*  D0h                                      */  0,
/*  D1h                                      */  0,
/*  D2h                                      */  0,
/*  D3h                                      */  0,
/*  D4h                                      */  0,
/*  D5h                                      */  0,
/*  D6h                                      */  0,
/*  D7h                                      */  0,
/*  D8h                                      */  0,
/*  D9h                                      */  0,
/*  DAh                                      */  0,
/*  DBh                                      */  0,
/*  DCh                                      */  0,
/*  DDh                                      */  0,
/*  DEh                                      */  0,
/*  DFh                                      */  0,
/*  E0h                                      */  0,
/*  E1h                                      */  0,
/*  E2h                                      */  0,
/*  E3h                                      */  0,
/*  E4h                                      */  0,
/*  E5h                                      */  0,
/*  E6h                                      */  0,
/*  E7h                                      */  0,
/*  E8h                                      */  0,
/*  E9h                                      */  0,
/*  EAh                                      */  0,
/*  EBh                                      */  0,
/*  ECh                                      */  0,
/*  EDh                                      */  0,
/*  EEh                                      */  0,
/*  EFh                                      */  0,
/*  F0h                                      */  0,
/*  F1h                                      */  0,
/*  F2h                                      */  0,
/*  F3h                                      */  0,
/*  F4h                                      */  0,
/*  F5h                                      */  0,
/*  F6h                                      */  0,
/*  F7h                                      */  0,
/*  F8h                                      */  0,
/*  F9h                                      */  0,
/*  FAh                                      */  0,
/*  FBh                                      */  0,
/*  FCh                                      */  0,
/*  FDh                                      */  0,
/*  FEh                                      */  0,
/*  FFh                                      */  6
};

/*************************************************************************/
//	CDB_Get_CDB_Length
//	Return the 32-bit logical block address from the CDB.
/*************************************************************************/
U32 CDB_Get_CDB_Length(CDB16 *pCDB)
{
    switch (MapCommandToCDBSize[pCDB->Cmd])
    {
        case SPECIAL_256: 
        case SPECIAL_6: 
        case SPECIAL_6_NO_LEN: 
        case 6: 
            return 6;
        case SPECIAL_10_NO_LEN: 
        case SPECIAL_10_24_BIT_LEN: 
        case SPECIAL_10_8_BIT_LEN: 
        case 10: 
            return 10;
        case SPECIAL_12_NO_LEN: 
        case 12: 
            return 12;
        case 16: 
            return 16;
        default:
            return 0;
    }
} // CDB_Get_Logical_Block_Address

/*************************************************************************/
//	CDB_Get_Logical_Block_Address
//	Return the 32-bit logical block address from the CDB.
/*************************************************************************/
U32 CDB_Get_Logical_Block_Address(CDB16 *pCDB)
{
    switch (MapCommandToCDBSize[pCDB->Cmd])
    {
        case SPECIAL_256: 
        case 6: 
            return (U32)(((PCDB6)pCDB)->BlockAddress
                + (((PCDB6)pCDB)->MSB << 16));
        case 10: 
            return ((PCDB10)pCDB)->BlockAddress;
        case 12: 
            return ((PCDB12)pCDB)->BlockAddress;
        default:
            return 0;
    }
} // CDB_Get_Logical_Block_Address

/*************************************************************************/
//	CDB_Set_Logical_Block_Address
//	Set the logical block address in the CDB.
/*************************************************************************/
void CDB_Set_Logical_Block_Address(CDB16 *pCDB, U32 logical_block_address)
{
    switch (MapCommandToCDBSize[pCDB->Cmd])
    {
        case SPECIAL_256: 
        case 6: 
            ((PCDB6)pCDB)->BlockAddress = (U16)logical_block_address;
            ((PCDB6)pCDB)->MSB = logical_block_address >> 16;
            break;
        case 10: 
            ((PCDB10)pCDB)->BlockAddress = logical_block_address;
            break;
        case 12: 
            ((PCDB12)pCDB)->BlockAddress = logical_block_address;
            break;
        default:
            return;
    }
} // CDB_Set_Logical_Block_Address

/*************************************************************************/
//	CDB_Get_Transfer_Length
//	Return the 32-bit transfer length from the CDB.
/*************************************************************************/
U32 CDB_Get_Transfer_Length(CDB16 *pCDB)
{
    switch (MapCommandToCDBSize[pCDB->Cmd])
    {
        case SPECIAL_256: 
         	// For some commands such as Read6 and Write6, a zero
        	// length translates to 256. 
           if (((PCDB6)pCDB)->Length)
           		return (U32)(unsigned char)((PCDB6)pCDB)->Length;
            return 256;
        case 6: 
           	return (U32)(unsigned char)((PCDB6)pCDB)->Length;
        case 10: 
        	// not aligned on a word address
            return (U32) (*(unsigned char *)( &((PCDB10)pCDB)->Length) << 8) |
            				(*(((unsigned char *)( &((PCDB10)pCDB)->Length)) + 1));
        case SPECIAL_10_8_BIT_LEN: 
        	// only an 8 bit length
            return (U32) (*(unsigned char *)( &((PCDB10)pCDB)->Length+1));
        case SPECIAL_10_24_BIT_LEN: 
        	// not aligned on a word address and 24 bit Length
            return (U32) (unsigned char)((PCDB10)pCDB)->Res2 << 16 |
            				(*(unsigned char *)( &((PCDB10)pCDB)->Length) << 8) |
            				(*(((unsigned char *)( &((PCDB10)pCDB)->Length)) + 1));
        case 12: 
            return (U32)((PCDB12)pCDB)->Length;
        case SPECIAL_6: 
        	// not aligned on a word address
            return (U32) (*(unsigned char *)( &((PRESERVE6)pCDB)->Length) << 8) |
            				(*(((unsigned char *)( &((PRESERVE6)pCDB)->Length)) + 1));
        default:
            return 0;
    }
}

/*************************************************************************/
//	CDB_Set_Transfer_Length
//	Set the transfer length in the CDB.
/*************************************************************************/
void CDB_Set_Transfer_Length(CDB16 *pCDB, U32 transfer_length)
{
    switch (MapCommandToCDBSize[pCDB->Cmd])
    {
        case SPECIAL_256:
        	// For some commands such as Read6 and Write6, a zero
        	// length translates to 256. 
        	if (transfer_length == 256)
            	((PCDB6)pCDB)->Length = 0;
          	else
            	((PCDB6)pCDB)->Length = (U8)transfer_length;
            break;
        case 6: 
            ((PCDB6)pCDB)->Length = (U8)transfer_length;
            break;
        case 10:
        	// not aligned on a word address
            *(unsigned char *)( &((PCDB10)pCDB)->Length) = transfer_length >> 8;
            *((unsigned char *)( &((PCDB10)pCDB)->Length)+1) = transfer_length & 0xff;
            break;
        case SPECIAL_10_8_BIT_LEN: 
            *((unsigned char *)( &((PCDB10)pCDB)->Length)+1) = transfer_length & 0xff;
            break;
        case SPECIAL_10_24_BIT_LEN: 
        	// not aligned on a word address and 24 bit length
            ((PCDB10)pCDB)->Res2 = (U8)(transfer_length >> 16) & 0xff;
            *(unsigned char *)( &((PCDB10)pCDB)->Length) = (transfer_length >> 8) & 0xff;
            *((unsigned char *)( &((PCDB10)pCDB)->Length)+1) = transfer_length & 0xff;
            break;
        case 12: 
            ((PCDB12)pCDB)->Length = (U32)transfer_length;
            break;
         case SPECIAL_6: 
        	// not aligned on a word address
            *(unsigned char *)( &((PRESERVE6)pCDB)->Length) = transfer_length >> 8;
            *((unsigned char *)( &((PRESERVE6)pCDB)->Length)+1) = transfer_length & 0xff;
            break;
            
        default:
            return;
    }
} // CDB_Set_Transfer_Length

/*************************************************************************/
//	CDB_Get_Control
//	Return the control field from the CDB.
/*************************************************************************/
U8 CDB_Get_Control(CDB16 *pCDB)
{
    switch (MapCommandToCDBSize[pCDB->Cmd])
    {
        case SPECIAL_256: 
        case SPECIAL_6: 
        case 6: 
            return ((PCDB6)pCDB)->Control;
        case 10: 
            return ((PCDB10)pCDB)->Control;
        case 12: 
            return ((PCDB12)pCDB)->Control;
        default:
            return 0;
    }
} // CDB_Get_Control

/**************************************************************************
CDB_Set_Control
Set the control field in the CDB.
**************************************************************************/
void CDB_Set_Control(CDB16 *pCDB, U8 Control)
{
    switch (MapCommandToCDBSize[pCDB->Cmd])
    {
        case SPECIAL_256: 
        case 6: 
            ((PCDB6)pCDB)->Control = Control;
            break;
        case 10: 
            ((PCDB10)pCDB)->Control = Control;
            break;
        case 12: 
            ((PCDB12)pCDB)->Control = Control;
            break;
        default:
            return;
    }
} // CDB_Set_Control
