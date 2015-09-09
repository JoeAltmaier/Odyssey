/* i2oThreadLib.h - thread library header file */

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


#ifndef __INCi2oThreadLibh
#define __INCi2oThreadLibh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "i2otypes.h"
#include "i2oErrorLib.h"
#include "i2oObjLib.h"

/* thread typedefs */

typedef struct i2oThread 	*I2O_THREAD_ID;		/* thread ID */
typedef U32			I2O_THREAD_PRI; 	/* thread priority */
typedef U32			I2O_THREAD_OPTIONS; 	/* thread options */
typedef void	(*I2O_THREAD_FUNC) (I2O_ARG); 		/* thread func */

#define I2O_THREAD_OPTS_NONE	0		/* thread options */

/* function declarations */

extern I2O_THREAD_ID i2oThreadCreate (I2O_OWNER_ID  ownerId, 
				      I2O_THREAD_PRI threadPri,
				      I2O_THREAD_OPTIONS threadOptions, 
				      I2O_SIZE threadStacksize, 
				      I2O_THREAD_FUNC threadFunc,
				      I2O_ARG threadArg,
				      I2O_STATUS * pStatus);
extern void		i2oThreadDelay (I2O_USECS usecs, I2O_STATUS * pStatus);
extern void 		i2oThreadLock (I2O_STATUS * pStatus);
extern void 		i2oThreadUnlock (I2O_STATUS * pStatus);
extern I2O_THREAD_ID 	i2oThreadIdSelf (I2O_STATUS * pStatus);
extern void 		i2oThreadPriSet (I2O_THREAD_ID threadId, 
				       	 I2O_THREAD_PRI threadPri,
					 I2O_STATUS * pStatus);
extern I2O_THREAD_PRI 	i2oThreadPriGet (I2O_THREAD_ID threadId, 
				       	 I2O_STATUS * pStatus);
extern void		i2oBusyWait (I2O_USECS usecs, I2O_STATUS * pStatus);
extern I2O_ERROR_ACTION	i2oThreadErrorActionGet (I2O_THREAD_ID threadId,
						 I2O_ERROR_FUNC * userFunc,
						 I2O_STATUS * pStatus);
extern void		i2oThreadErrorActionSet (I2O_THREAD_ID threadId,
						 I2O_ERROR_ACTION errorAction,
						 I2O_ERROR_FUNC userFunc,
						 I2O_STATUS * pStatus);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCi2oThreadLibh */

