/* i2oMemSetLib.h - page library header file */

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


#ifndef __INCi2oMemSetLibh
#define __INCi2oMemSetLibh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "i2o/shared/i2otypes.h"
#include "i2o/i2oObjLib.h"
#include "i2o/i2oBusLib.h"
#include "i2o/i2oEvtQLib.h"
#include "i2o/i2oPageLib.h"

/* mem set typedefs */

typedef struct i2oMemSet * 	I2O_MEM_SET_ID;	/* mem set ID */

/* defines */

#define I2O_MEM_ALIGNMENT_DEFAULT		16

/* function declarations */

extern I2O_MEM_SET_ID	i2oMemSetCreate (I2O_OWNER_ID ownerId,
					 I2O_MEM_CACHE_ATTR cacheAttr,
					 I2O_MEM_ACCESS_ATTR accessAttr,
					 I2O_BUS_ID busId,
					 I2O_STATUS * pStatus);
extern I2O_ADDR32	i2oMemAlloc (I2O_MEM_SET_ID memSetId, I2O_SIZE size,
				     I2O_SIZE alignment, I2O_STATUS * pStatus);
extern void		i2oMemFree (I2O_MEM_SET_ID memSetId,
				    I2O_ADDR32 addr, I2O_STATUS * pStatus);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCi2oMemSetLibh */

