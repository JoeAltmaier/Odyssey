/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpProto.h
// 
// Description:
// This file defines interfaces for the FCP methods with forward reference
// loops.  Many of the create and destroy methods are included here.
// 
// Update Log: 
// 8/25/98 Michael G. Panas: Create file
// 9/2/98 Michael G. Panas: add C++ stuff
// 11/30/98 Michael G. Panas: New memory allocation methods
// 12/02/98 Michael G. Panas: add new methods defined in DDMs external
//                            to the FCP Library
// 07/16/99 Michael G. Panas: Use the LoopMonitor to handle AE
/*************************************************************************/

#if !defined(FcpProto_H)
#define FcpProto_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


/************************************************************************/
//	Public methods implemented in FcpISP.c
/************************************************************************/
STATUS	FCP_ISP_Create(PINSTANCE_DATA Id);
void	FCP_ISP_Destroy();

/************************************************************************/
//	Public methods implemented in FcpIOCB.c
/************************************************************************/
STATUS	FCP_IOCB_Create(PINSTANCE_DATA Id);
void	FCP_IOCB_Destroy();

/************************************************************************/
//	Public methods implemented in FcpBuffer.c
/************************************************************************/
STATUS	FCP_Buffer_Create(PINSTANCE_DATA Id);
void	FCP_Buffer_Destroy();

/************************************************************************/
// Public Methods implemented in FcpIOCB.c
// Can't define in FcpIOCB.h because FcpEvent.h depends on FcpIOCB.h
/************************************************************************/
STATUS	FCP_Send_Command_IOCB(
			FCP_EVENT_CONTEXT *p_context, 
			FCP_EVENT_ACTION next_action,
			IOCB_COMMAND_TYPE2 *p_command_IOCB);
			
/************************************************************************/
//	Public methods implemented in DDMs that are called from the FCP Lib
/************************************************************************/
STATUS	LM_Handle_FC_AE(FCP_EVENT_CONTEXT *p_context);


#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* FcpProto_H  */
