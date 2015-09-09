/* i2oPipeLib.h - pipe library header file */

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


#ifndef __INCi2oPipeLibh
#define __INCi2oPipeLibh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "i2o/shared/i2otypes.h"
#include "i2o/i2oObjLib.h"

/* pipe typedefs */

typedef struct i2oPipe *  	I2O_PIPE_ID;		/* pipe ID */
typedef int			I2O_PIPE_OPTIONS; 	/* pipe options */

/* pipe options */

#define I2O_PIPE_OPT_Q_FIFO	0x00   	/* 0x0 first in first out queue */
#define I2O_PIPE_OPT_Q_PRIORITY	0x01  	/* 0x1 priority sorted queue */

/* pipe priorities */

typedef enum		
    {
    I2O_PIPE_PRI_NORMAL	= 0,		/* normal priority */
    I2O_PIPE_PRI_URGENT	= 1		/* urgent priority */
    } I2O_PIPE_PRI;			

/* function declarations */

extern I2O_PIPE_ID	i2oPipeCreate (I2O_OWNER_ID ownerId, I2O_COUNT maxMsgs,
				       I2O_SIZE maxMsgLen, 
				       I2O_PIPE_OPTIONS pipeOptions, 
				       I2O_STATUS * pStatus);
extern void		i2oPipeSend (I2O_PIPE_ID pipeId, I2O_ADDR32 buf, 
				     I2O_SIZE nBytes, I2O_PIPE_PRI pri,
				     I2O_USECS timeout, I2O_STATUS * pStatus);
extern I2O_SIZE		i2oPipeReceive (I2O_PIPE_ID pipeId, I2O_ADDR32 buf,
					I2O_SIZE maxBytes, I2O_USECS timeout,
					I2O_STATUS * pStatus);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCi2oPipeLibh */

