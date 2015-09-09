/* i2oSemLib.h - sem library header file */

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


#ifndef __INCi2oSemLibh
#define __INCi2oSemLibh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "i2o/shared/i2otypes.h"
#include "i2o/i2oErrorLib.h"
#include "i2o/i2oObjLib.h"

/* sem typedefs */

typedef struct i2oSem *  	I2O_SEM_ID;		/* sem ID */
typedef int			I2O_SEM_OPTIONS; 	/* sem options */

/* semaphore options */

#define I2O_SEM_OPT_Q_FIFO	0x0    	/* 0x0 first in first out queue */
#define I2O_SEM_OPT_Q_PRIORITY	0x1   	/* 0x1 priority sorted queue */
#define I2O_SEM_OPT_DELETE_SAFE	0x4 	/* 0x4 holder del safe (mutx) */
#define I2O_SEM_OPT_INVERSION_SAFE 0x8 	/* 0x8 no pri invn (mutx)*/

/* semaphore initial states */

typedef enum
    {
    I2O_SEM_EMPTY	= 0,
    I2O_SEM_FULL	= 1
    } I2O_SEM_B_STATE;

/* function declarations */


extern I2O_SEM_ID 	i2oSemBCreate (I2O_OWNER_ID  ownerId, 
				       I2O_SEM_OPTIONS semOptions, 
				       I2O_SEM_B_STATE intialState,
				       I2O_STATUS * pStatus);
extern I2O_SEM_ID 	i2oSemCCreate (I2O_OWNER_ID  ownerId, 
				       I2O_SEM_OPTIONS semOptions, 
				       I2O_COUNT intialCount,
				       I2O_STATUS * pStatus);
extern I2O_SEM_ID 	i2oSemMCreate (I2O_OWNER_ID  ownerId, 
				       I2O_SEM_OPTIONS semOptions, 
				       I2O_STATUS * pStatus);
extern void		i2oSemTake (I2O_SEM_ID semId, I2O_USECS timeout, 
				    I2O_STATUS * pStatus);
extern void		i2oSemGive (I2O_SEM_ID semId, I2O_STATUS * pStatus);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCi2oSemLibh */

