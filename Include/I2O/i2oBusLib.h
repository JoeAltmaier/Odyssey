/* i2oBusLib.h - i2o bus library header file */

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

#ifndef __INCi2oBusLibh
#define __INCi2oBusLibh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "i2otypes.h"
#include "i2oErrorLib.h"
#include "i2o.h"

/* privates */

typedef struct i2oBus*		I2O_BUS_ID;

/* spaces */

typedef enum 
    {
    I2O_BUS_MEMORY_SPACE = 1,
    I2O_BUS_IO_SPACE = 2
    } I2O_BUS_SPACE;

/* function declarations */

extern U8 i2oBusRead8 (I2O_BUS_ID         busId,
                       I2O_BUS_SPACE      busSpace,
                       I2O_ADDR32         busAddr,
                       I2O_STATUS*        pStatus);

extern U16 i2oBusRead16 (I2O_BUS_ID       busId,
                         I2O_BUS_SPACE    busSpace,
                         I2O_ADDR32       busAddr,
                         I2O_STATUS*      pStatus);

extern U32 i2oBusRead32 (I2O_BUS_ID       busId,
                         I2O_BUS_SPACE    busSpace,
                         I2O_ADDR32       busAddr,
                         I2O_STATUS*      pStatus);

extern U64 i2oBusRead64 (I2O_BUS_ID       busId,
                         I2O_BUS_SPACE    busSpace,
                         I2O_ADDR32       busAddr,
                         I2O_STATUS*      pStatus);

extern void i2oBusWrite8 (I2O_BUS_ID      busId,
                          I2O_BUS_SPACE   busSpace,
                          I2O_ADDR32      busAddr,
                          U8              value,
                          I2O_STATUS*     pStatus);

extern void i2oBusWrite16 (I2O_BUS_ID     busId,
                           I2O_BUS_SPACE  busSpace,
                           I2O_ADDR32     busAddr,
                           U16            value,
                           I2O_STATUS*    pStatus);

extern void i2oBusWrite32 (I2O_BUS_ID     busId,
                           I2O_BUS_SPACE  busSpace,
                           I2O_ADDR32     busAddr,
                           U32            value,
                           I2O_STATUS*    pStatus);

extern void i2oBusWrite64 (I2O_BUS_ID     busId,
                           I2O_BUS_SPACE  busSpace,
                           I2O_ADDR32     busAddr,
                           U64            value,
                           I2O_STATUS*    pStatus);


extern I2O_ADDR32 i2oBusTranslate (I2O_BUS_ID     busId1,
                                   I2O_BUS_SPACE  busSpace1,
                                   I2O_ADDR32     busAddr1,
                                   I2O_BUS_ID     busId2,
                                   I2O_BUS_SPACE  busSpace2,
                                   I2O_STATUS*    pStatus);

extern  I2O_BUS_ID i2oBusLocal(I2O_STATUS*      pStatus);

extern  I2O_BUS_ID i2oBusSystem(I2O_STATUS*     pStatus);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCi2oBusLibh */
