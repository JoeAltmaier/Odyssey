/* i2oTimerLib.h - timer library header file */

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


#ifndef __INCi2oTimerLibh
#define __INCi2oTimerLibh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "i2o/shared/i2otypes.h"
#include "i2o/i2oEvtQLib.h"
#include "i2o/i2oObjLib.h"
#include "i2o/i2oErrorLib.h"

/* timer typedefs */

typedef struct i2oTimer *I2O_TIMER_ID;	/* timer ID */

/* function declarations */

extern I2O_TIMER_ID	i2oTimerCreate (I2O_OWNER_ID ownerId,
					I2O_OBJ_CONTEXT context,
					I2O_EVENT_QUEUE_ID eventQId,
					I2O_STATUS * pStatus);
extern void		i2oTimerStart (I2O_TIMER_ID timerId, I2O_USECS usecs,
				       I2O_EVENT_PRI evtPri,
				       I2O_EVENT_HANDLER evtHandler, 
				       I2O_ARG evtArg,
				       I2O_STATUS * pStatus);
extern void		i2oTimerRepeat (I2O_TIMER_ID timerId, I2O_USECS usecs,
				        I2O_EVENT_PRI evtPri,
				        I2O_EVENT_HANDLER evtHandler, 
					I2O_ARG evtArg,
				        I2O_STATUS * pStatus);
extern void		i2oTimerCancel (I2O_TIMER_ID timerId, 
					I2O_STATUS * pStatus);
extern I2O_USECS	i2oTimerElapsed (I2O_TIMER_ID timerId, 
					 I2O_COUNT * pCount,
					 I2O_STATUS * pStatus);
extern I2O_USECS	i2oTimerEventRes ();
extern I2O_USECS	i2oTimerStampRes ();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCi2oTimerLibh */

