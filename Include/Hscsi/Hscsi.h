/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Hscsi.h
// 
// Description:
// This file defines the external interfaces to the HSCSI driver. 
// 
// Update Log:
//	$Log: /Gemini/Include/Hscsi/Hscsi.h $ 
// 
// 1     9/15/99 11:11a Cchan
// Includes required for the HSCSI library (QL1040B support)
//
/*************************************************************************/

#if !defined(Hscsi_H)
#define Hscsi_H

#include "Nucleus.h"
#include "HscsiData.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

/* Declared in HscsiData.h
typedef enum {
	HSCSI_STATE_RESET,
	HSCSI_STATE_INIT,
	HSCSI_STATE_ACTIVE,
	HSCSI_STATE_QUIET
} HSCSI_STATE;
*/

STATUS	HSCSI_Start(PHSCSI_INSTANCE_DATA Id);
STATUS	HSCSI_Quiet(PHSCSI_INSTANCE_DATA Id);
STATUS	HSCSI_Restart(PHSCSI_INSTANCE_DATA Id);
STATUS	HSCSI_Reset(PHSCSI_INSTANCE_DATA Id);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif /* Hscsi_H  */
