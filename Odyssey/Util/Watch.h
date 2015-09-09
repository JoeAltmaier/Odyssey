/* Watch.h -- RM7000 Watchpoint support
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
**/

// Revision History:
//  8/4/98 Joe Altmaier: Create file


#ifndef __Watch_h
#define __Watch_h

typedef enum {WATCH1=1, WATCH2=2} WATCH;
typedef enum {WATCHSTORE=8, WATCHLOAD=4, WATCHEXECUTE=2} WATCHMODE;

#ifdef  __cplusplus
extern "C"
#else
extern
#endif
void Watch(WATCH watch, void *pWatch, int cb, WATCHMODE mode);

#endif