/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiDebug.h
// 
// Description:
// This file defines the external interfaces to HscsiDebug
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiDebug.h $ 
// 
// 1     9/14/99 7:23p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#if !defined(HscsiDebug_H)
#define HscsiDebug_H


#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

STATUS	HSCSI_Debug_Create(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_Debug_Destroy();

STATUS	HSCSI_Dump_RISC_RAM(PHSCSI_INSTANCE_DATA Id, U8 *Address, U32 Length, U16 Start_Addr);
void	HSCSI_ISP_Print_Regs(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_Print_RAM_Word(PHSCSI_INSTANCE_DATA Id, U16 index);
void	HSCSI_Verify_RISC_RAM(PHSCSI_INSTANCE_DATA Id);

U16		Read_ISP1040(PHSCSI_INSTANCE_DATA Id, U16 ISP_register);
void	Write_ISP1040(PHSCSI_INSTANCE_DATA Id, U16 ISP_register, U16 value);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif /* HscsiDebug_H  */
