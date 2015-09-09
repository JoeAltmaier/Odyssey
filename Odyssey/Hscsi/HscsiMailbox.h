/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiMailbox.h
// 
// Description:
// This file describes interfaces to the ISP mailbox. 
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiMailbox.h $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#if !defined(HscsiMailbox_H)
#define HscsiMailbox_H

#include "HscsiConfig.h"
#include "HscsiData.h"
#include "HscsiError.h"
#include "HscsiISP.h"
#include "Nucleus.h"


#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

STATUS	HSCSI_Mailbox_Create(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_Mailbox_Destroy();

U16		HSCSI_Mailbox_Command(PHSCSI_INSTANCE_DATA Id);
STATUS	HSCSI_Mailbox_Wait_Ready_Intr(PHSCSI_INSTANCE_DATA Id);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* HscsiMailbox_H  */
