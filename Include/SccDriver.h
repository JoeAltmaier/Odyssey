/* SccDriver.H -- Serial Communications Channel Driver
 *
 * Copyright ConvergeNet Technologies (c) 1998 
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Revision History:
 *    10/14/98 Tom Nelson: Created
 *	02/12/99 JFL	#included cttypes.h.
 *	02/15/99 JA 	#included OsTypes.h.
 *
**/

/* WARNING: Polled I/O will not be supported on final release    */

#ifdef __cplusplus
extern "C" {
#endif

#include "OsTypes.h"

STATUS SccInitialize(void);
STATUS SccInitializePolled(void);

BOOL SccIfChar(void);
char SccGetChar(void);
BOOL SccPutChar(char ch);
BOOL SccPutStr(char *pStr);
void SccPutFlush(void);
void SccGetFlush(void);

#ifdef __cplusplus
}
#endif


