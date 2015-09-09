/* i2oFrameLib.h - I2O HW queue interface header file */

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


#ifndef __INCi2oFrameLibh
#define __INCi2oFrameLibh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "i2oEvtQLib.h"
#include "i2omsg.h"
#include "i2oErrorLib.h"

typedef struct 
    {
    I2O_MESSAGE_FRAME     i2oMsgFrameHdr;
    } I2O_FRAME;

/* function declarations */
extern void                	i2oFrameFree (I2O_FRAME* pFrame,
                                              I2O_STATUS* pStatus);
extern I2O_FRAME*   		i2oFrameAlloc (I2O_STATUS* pStatus);
extern void     		i2oFrameSend (I2O_FRAME* pFrame,
                                              I2O_STATUS* pStatus);
extern I2O_INITIATOR_CONTEXT	i2oInitiatorContextBuild
                                             (I2O_EVENT_HANDLER	func,
                                              I2O_EVENT_PRI	pri,
                                              I2O_STATUS*	pStatus);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCi2oFifoLibh */
