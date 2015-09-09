/* i2oIopLib.h - I2O IOP Information functions. */

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


#ifndef __INCi2oIopLibh
#define __INCi2oIopLibh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "i2o/shared/i2otypes.h"
#include "i2o/i2oErrorLib.h"
#include "i2o/i2oDdmLib.h"

typedef U32 I2O_IOP_CONFIG_FLAGS;

typedef struct
    {
    I2O_SIZE                configInfoLen;
    I2O_COUNT               i2oVersion;
    I2O_IOP_CONFIG_FLAGS    iopFlags;
    I2O_SIZE                localFrameSize;
    I2O_SIZE                sysPageSize;
    I2O_ADDR32              bitBucketAddr;
    I2O_COUNT               bitBucketSize;
    } I2O_IOP_CONFIG_INFO;

/* function declarations */
extern I2O_IOP_CONFIG_INFO*     i2oIopConfigGet(void);
extern BOOL                     i2oIopTidIsLocal(I2O_TID tid, 
                                                 I2O_STATUS* pStatus);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCi2oIopLibh */
