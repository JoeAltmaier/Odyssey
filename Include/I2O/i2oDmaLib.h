/* i2oDmaLib.h - i2o dma library header file */

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


#ifndef __INCi2oDmaLibh
#define __INCi2oDmaLibh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "i2o/i2oBusLib.h"
#include "i2o/i2oEvtQLib.h"
#include "i2o/shared/i2otypes.h"
#include "i2o/shared/i2omsg.h"

/* privates */
typedef struct i2oDma*		I2O_DMA_ID;

/* Used for status that is passed to DMA event. */
typedef U32           I2O_DMA_CREATE_FLAGS;
typedef U32           I2O_DMA_XFER_FLAGS;

/* cancel modes */
typedef enum
    {
    I2O_DMA_CANCEL_MATCHING = 1,
    I2O_DMA_CANCEL_ALL = 2
    } I2O_DMA_CANCEL_MODE;

/* Create flags */
#define I2O_DMA_BUS_1_DEMAND_MODE	0x1
#define I2O_DMA_BUS_2_DEMAND_MODE	0x2
#define I2O_DMA_SERIAL_MODE		0x4

/* xfer flags */
#define I2O_DMA_DIR_REVERSE		0x1
#define I2O_DMA_NO_EVENT		0x2
#define I2O_DMA_SRC_SGL_CONTEXT_64	0x4
#define I2O_DMA_DST_SGL_CONTEXT_64	0x8

/* function declarations */
extern I2O_DMA_ID      i2oDmaCreate  
                            (
                            I2O_OWNER_ID           ownerId,
                            I2O_BUS_ID             busId1,
                            I2O_BUS_SPACE          busSpace1,
                            I2O_BUS_ID             busId2,
                            I2O_BUS_SPACE          busSpace2, 
                            I2O_COUNT              maxXfers,
                            I2O_DMA_CREATE_FLAGS   createFlags,
                            I2O_EVENT_QUEUE_ID     evtQId,
                            I2O_EVENT_PRI          evtPri,
                            I2O_STATUS*            pStatus
                            );

extern I2O_STATUS      i2oDmaXfer 
                            (
                            I2O_DMA_ID             dmaId, 
                            I2O_ADDR32             addr1, 
                            I2O_ADDR32             addr2, 
                            I2O_SIZE               len, 
                            I2O_DMA_XFER_FLAGS     xferFlags,
                            I2O_OBJ_CONTEXT        xferContext,
                            I2O_EVENT_HANDLER      pEvtHandler,
                            I2O_STATUS*            pStatus
                            );

extern I2O_STATUS       i2oDmaXferList 
                            (
                            I2O_DMA_ID                  dmaId, 
                            I2O_SG_ELEMENT*             pList1, 
                            I2O_SG_ELEMENT*             pList2, 
                            I2O_DMA_XFER_FLAGS          xferFlags,
                            I2O_OBJ_CONTEXT             xferContext,
                            I2O_EVENT_HANDLER 		pEvtHandler,
                            I2O_STATUS*			pStatus
                            );

extern I2O_STATUS       i2oDmaXferFrag 
                            (
                            I2O_DMA_ID                  dmaId, 
                            I2O_SG_ELEMENT*             pList1, 
                            I2O_SIZE                    offset1,
                            I2O_SG_ELEMENT*             pList2, 
                            I2O_SIZE                    offset2,
                            I2O_SIZE                    maxBytes,
                            I2O_DMA_XFER_FLAGS          xferFlags,
                            I2O_OBJ_CONTEXT             xferContext,
                            I2O_EVENT_HANDLER 		pEvtHandler,
                            I2O_STATUS*			pStatus
                            );

extern void             i2oDmaCancel 
                            (
                            I2O_DMA_ID           dmaId, 
                            I2O_DMA_CANCEL_MODE  mode,
                            I2O_OBJ_CONTEXT      xferContext,
                            I2O_STATUS*          pStatus
                            );

extern void             i2oDmaResume
                            (
                            I2O_DMA_ID           dmaId, 
                            I2O_STATUS*          pStatus
                            );
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCi2oDmaLibh */
