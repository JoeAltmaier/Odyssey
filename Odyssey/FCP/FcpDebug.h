/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpDebug.h
// 
// Description:
// This file defines the external interfaces to FcpDebug
// 
// Update Log 
// 8/17/98 Michael G. Panas: Create file
// 11/30/98 Michael G. Panas: Add create/destroy for consistency
/*************************************************************************/

#if !defined(FcpDebug_H)
#define FcpDebug_H


#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

STATUS	FCP_Debug_Create(PINSTANCE_DATA Id);
void	FCP_Debug_Destroy();

STATUS	FCP_Dump_RISC_RAM(PINSTANCE_DATA Id, U8 *Address, U32 Length, U16 Start_Addr);
STATUS	FCP_Get_Initialization_Control_Block(PINSTANCE_DATA Id, FCP_EVENT_CONTEXT *p_context);
void	FCP_ISP_Print_Regs(PINSTANCE_DATA Id);
STATUS	FCP_Load_RISC_RAM_DBG(PINSTANCE_DATA Id, U8 *Address, U32 Length, U16 Start_Addr);
void	FCP_Print_RAM_Word(PINSTANCE_DATA Id, U16 index);
void	FCP_Verify_RISC_RAM(PINSTANCE_DATA Id);
void	FCP_Verify_RISC_RAM_2200(PINSTANCE_DATA Id, U16 Start_Addr);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif /* FcpDebug_H  */
