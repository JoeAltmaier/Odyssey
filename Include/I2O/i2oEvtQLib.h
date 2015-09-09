/* i2oEvtQLib.h - event queue library header file */

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


#ifndef __INCi2oEvtQLibh
#define __INCi2oEvtQLibh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "i2oObjLib.h"
#include "i2otypes.h"
#include "i2oThreadLib.h"
#include "i2oErrorLib.h"

/* event queue typedefs */

typedef struct i2oEventQ *	I2O_EVENT_QUEUE_ID;	/* event queue ID */
typedef U8			I2O_EVENT_PRI;		/* event priority */
typedef void	(*I2O_EVENT_HANDLER) (I2O_OBJ_CONTEXT, I2O_ARG);/* event func */
typedef U32			I2O_EVENT_PRI_MASK;	/* event pri mask */

/* function declarations */

extern I2O_EVENT_QUEUE_ID	i2oEventQCreate (I2O_OWNER_ID ownerId, 
					 I2O_THREAD_OPTIONS options, 
				         I2O_SIZE stackSize, 
					 I2O_STATUS * pStatus);
extern void		i2oEventQPost (I2O_EVENT_QUEUE_ID evtQId, 
				       I2O_EVENT_PRI evtPri,
				       I2O_EVENT_HANDLER evtHandler, 
				       I2O_ARG evtArg1, I2O_ARG evtArg2,
				       I2O_STATUS * pStatus);
extern BOOL		i2oEventQPriEnableGet (I2O_EVENT_QUEUE_ID evtQId, 
					       I2O_EVENT_PRI evtPriLevel, 
					       I2O_STATUS * pStatus);
extern void		i2oEventQPriEnableSet (I2O_EVENT_QUEUE_ID evtQId, 
					       I2O_EVENT_PRI evtPriLevel, 
					       BOOL enableFlag,
					       I2O_STATUS * pStatus);
extern I2O_EVENT_PRI_MASK	i2oEventQPriMaskGet (I2O_EVENT_QUEUE_ID evtQId, 
					     	     I2O_STATUS * pStatus);
extern void		i2oEventQPriMaskSet (I2O_EVENT_QUEUE_ID evtQId, 
					     I2O_EVENT_PRI_MASK mask,
					     I2O_STATUS * pStatus);
extern I2O_EVENT_PRI_MASK	i2oEventQPriPending (I2O_EVENT_QUEUE_ID evtQId,
					             I2O_STATUS * pStatus);
extern I2O_THREAD_ID	i2oEventQThreadGet (I2O_EVENT_QUEUE_ID evtQId, 
					    I2O_STATUS * pStatus);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCi2oEvtQLibh */
