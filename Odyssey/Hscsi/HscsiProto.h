/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiProto.h
// 
// Description:
// This file defines interfaces for the HSCSI methods with forward reference
// loops.  Many of the create and destroy methods are included here.
// 
// Update Log:
//	$Log: /Gemini/Odyssey/Hscsi/HscsiProto.h $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#if !defined(HscsiProto_H)
#define HscsiProto_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


/************************************************************************/
//	Public methods implemented in HscsiISP.c
/************************************************************************/
STATUS	HSCSI_ISP_Create(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_ISP_Destroy();

/************************************************************************/
//	Public methods implemented in HscsiIOCB.c
/************************************************************************/
STATUS	HSCSI_IOCB_Create(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_IOCB_Destroy();

/************************************************************************/
//	Public methods implemented in HscsiBuffer.c
/************************************************************************/
STATUS	HSCSI_Buffer_Create(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_Buffer_Destroy();

/************************************************************************/
// Public Methods implemented in HscsiIOCB.c
// Can't define in HscsiIOCB.h because HscsiEvent.h depends on HscsiIOCB.h
/************************************************************************/
STATUS	HSCSI_Send_Command_IOCB(
			HSCSI_EVENT_CONTEXT *p_context, 
			HSCSI_EVENT_ACTION next_action,
			HSCSI_IOCB_COMMAND *p_command_IOCB);

STATUS	HSCSI_Send_IOCB( // function in HscsiIOCB.c
			HSCSI_EVENT_CONTEXT *p_context, 
			HSCSI_EVENT_ACTION next_action,
			HSCSI_IOCB_COMMAND *p_command_IOCB);
						
/************************************************************************/
//	Public methods implemented in the DDMs
/************************************************************************/
STATUS	HSCSI_Handle_AE_Target(HSCSI_EVENT_CONTEXT *p_context);
STATUS	HSCSI_Handle_AE_Initiator(HSCSI_EVENT_CONTEXT *p_context);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* HscsiProto_H  */
