/* i2oPageLib.h - page library header file */

/****************************************************************************
All software on this website is made available under the following terms
and conditions. By downloading this software, you agree to abide by
these terms and conditions with respect to this software.

I2O SIG All rights reserved.

These header files are provided, pursuant to your I2O SIG membership
agreement, free of charge on an as-is basis without warranty of any
kind, either express or implied, including but not limited to, implied
warranties or merchantability and fitness for a particular purpose.
I2O SIG does not warrant that this program will meet the user's
requirements or that the operation of these programs will be
uninterrupted or error-free. Acceptance and use of this program
constitutes the user's understanding that he will have no recourse to
I2O SIG for any actual or consequential damages including, but not
limited to, loss profits arising out of use or inability to use this
program.

Member is permitted to create derivative works to this header-file
program.  However, all copies of the program and its derivative works
must contain the I2O SIG copyright notice.
**************************************************************************/


#ifndef __INCi2oPageLibh
#define __INCi2oPageLibh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "i2o/shared/i2otypes.h"
#include "i2o/i2oObjLib.h"
#include "i2o/i2oBusLib.h"
#include "i2o/i2oEvtQLib.h"

/* page set typedefs */

typedef struct i2oPageSet * 	I2O_PAGE_SET_ID;	/* page set ID */
typedef int			I2O_MEM_CACHE_ATTR;	/* cache attributes */

/* memory access attributes */

typedef enum
    {
    I2O_MEM_ACCESS_PRIVATE		= 0,
    I2O_MEM_ACCESS_SYSTEM		= 1,
    I2O_MEM_ACCESS_LOCAL_ADAPTERS	= 2,
    I2O_MEM_ACCESS_ALL_ADAPTERS		= 3,
    I2O_MEM_ACCESS_BUS_SPECIFIC		= 4
    } I2O_MEM_ACCESS_ATTR;

/* memory cache attribute bits */

#define I2O_MEM_EXTERNAL_READ_SAFE	0x1
#define I2O_MEM_EXTERNAL_WRITE_SAFE	0x2

/* memory battery backup attributes */

typedef enum	
    {
    I2O_BBU_REQUIRED	= 0,
    I2O_BBU_DESIRED	= 1,
    I2O_BBU_NOT_USED	= 2
    } I2O_BBU_ATTR;		

/* memory battery backup status values */

typedef enum		
    {
    I2O_BBU_UNAVAILABLE	= 0,
    I2O_BBU_UNCHARGED	= 1,
    I2O_BBU_CHARGED	= 2
    } I2O_BBU_STATUS;		

/* function declarations */

extern I2O_PAGE_SET_ID	i2oPageSetCreate (I2O_OWNER_ID ownerId,
					  I2O_OBJ_CONTEXT context,
					  I2O_MEM_CACHE_ATTR cacheAttr,
					  I2O_MEM_ACCESS_ATTR accessAttr,
					  I2O_BUS_ID busId,
					  I2O_BBU_ATTR bbuAttr, 
					  I2O_STATUS * pStatus);
extern void		i2oPageAllocContig (I2O_PAGE_SET_ID pageSetId,
					    I2O_COUNT minPages, 
					    I2O_COUNT maxPages,
					    I2O_EVENT_QUEUE_ID pageEvtQId,
					    I2O_EVENT_PRI evtPri,
					    I2O_EVENT_HANDLER evtHandler, 
					    I2O_ARG evtArg,
					    I2O_STATUS * pStatus);
extern I2O_ADDR32	i2oPageAlloc (I2O_PAGE_SET_ID pageSetId,
				      I2O_STATUS * pStatus);
extern void		i2oPageFree (I2O_PAGE_SET_ID pageSetId,
				     I2O_ADDR32 pageAddr, 
				     I2O_STATUS * pStatus);
extern I2O_COUNT	i2oPageAllocN (I2O_PAGE_SET_ID pageSetId, 
				       I2O_COUNT nPages, 
			   	       I2O_ADDR32 * pageArray,
				       I2O_STATUS * pStatus);
extern void		i2oPageFreeN (I2O_PAGE_SET_ID pageSetId, 
				      I2O_COUNT nPages,
			   	      I2O_ADDR32 * pageArray, 
				      I2O_STATUS * pStatus);
extern I2O_SIZE		i2oPageSizeGet (I2O_PAGE_SET_ID pageSetId,
				        I2O_STATUS * pStatus);
extern I2O_COUNT	i2oPageCountGet (I2O_PAGE_SET_ID pageSetId,
				         I2O_STATUS * pStatus);
extern I2O_ADDR32	i2oPageAddrGet (I2O_PAGE_SET_ID pageSetId,
					I2O_ADDR32 prevPageAddr,
				        I2O_STATUS * pStatus);
extern BOOL		i2oPageBbuEnableGet (I2O_PAGE_SET_ID pageSetId,
				             I2O_STATUS * pStatus);
extern void		i2oPageBbuEnableSet (I2O_PAGE_SET_ID pageSetId,
					     BOOL bbuEnable, 
					     I2O_STATUS * pStatus);
extern I2O_BBU_STATUS	i2oPageBbuStatus (I2O_PAGE_SET_ID pageSetId,
					  I2O_STATUS * pStatus);
extern void 		i2oPageBbuNotify (I2O_PAGE_SET_ID pageSetId,
					  I2O_EVENT_QUEUE_ID evtQId,
					  I2O_EVENT_PRI evtPri,
					  I2O_EVENT_HANDLER bbuEvtHandler, 
					  I2O_ARG bbuEvtArg,
					  I2O_STATUS * pStatus);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCi2oPageLibh */

