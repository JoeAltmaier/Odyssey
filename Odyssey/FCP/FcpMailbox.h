/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpMailbox.h
// 
// Description:
// This file describes interfaces to the ISP mailbox. 
// 
// Update Log 
// 5/18/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 9/2/98 Michael G. Panas: add C++ stuff
// 10/6/98 Michael G. Panas: changes to support interrupt driven mailbox commands
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/

#if !defined(FcpMailbox_H)
#define FcpMailbox_H

#include "FcpConfig.h"
#include "FcpData.h"
#include "FCPError.h"
#include "FcpISP.h"
#include "Nucleus.h"


#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

STATUS	FCP_Mailbox_Create(PINSTANCE_DATA Id);
void	FCP_Mailbox_Destroy();

U16		FCP_Mailbox_Command(PINSTANCE_DATA Id);
STATUS	FCP_Mailbox_Wait_Ready_Intr(PINSTANCE_DATA Id);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* FcpMailbox_H  */
