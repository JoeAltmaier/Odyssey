/* i2oIntLib.h - interrupt library header file */

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


#ifndef __INCi2oIntLibh
#define __INCi2oIntLibh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "i2o/i2o.h"
#include "i2o/i2oAdapterLib.h"
#include "i2o/shared/i2otypes.h"
#include "i2o/i2oEvtQLib.h"
#include "i2o/i2oObjLib.h"

/* interrupt typedefs */

typedef struct i2oInt *		I2O_INT_ID;		/* interrupt ID */
typedef BOOL	(*I2O_ISR_HANDLER) (I2O_OBJ_CONTEXT, I2O_ARG);	/* isr func */
typedef int			I2O_INT_LOCK_KEY;	/* isr func */

/* function declarations */

extern I2O_INT_ID	i2oIntCreate (I2O_OWNER_ID ownerId, 
				      I2O_OBJ_CONTEXT context,
				      I2O_ADAPTER_ID adapterId,
				      I2O_ISR_HANDLER isrHandler, I2O_ARG isrArg, 
				      I2O_EVENT_QUEUE_ID evtQId, I2O_COUNT maxEvts,
				      I2O_STATUS * pStatus);
extern BOOL		i2oIntInIsr (void);
extern I2O_INT_LOCK_KEY	i2oIntLock (void);
extern void		i2oIntUnlock (I2O_INT_LOCK_KEY key);
extern void		i2oIntEventPost (I2O_INT_ID intId, I2O_EVENT_PRI evtPri,
				         I2O_EVENT_HANDLER evtHandler, 
				         I2O_ARG evtArg,
				         I2O_STATUS * pStatus);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCi2oIntLibh */

