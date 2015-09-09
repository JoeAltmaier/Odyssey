/* i2oAdapterLib.h - i2o Adapter library header file */

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

#ifndef __INCi2oAdapterLibh
#define __INCi2oAdapterLibh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "i2o.h"
#include "i2oBusLib.h"
#include "i2omsg.h"

/* privates */
typedef struct i2oAdapter*		I2O_ADAPTER_ID;

typedef struct
    {
    union 
        {
        /* PCI Bus */
        I2O_PCI_BUS_INFO        pci;

        /* Local Bus */
        I2O_LOCAL_BUS_INFO      local;

        /* ISA Bus */
        I2O_ISA_BUS_INFO        isa;

        /* EISA Bus */
        I2O_EISA_BUS_INFO       eisa;

        /* MCA Bus */
        I2O_MCA_BUS_INFO        mca;

        /* Other. */
        I2O_OTHER_BUS_INFO      other;
        } u;

    } I2O_PHYSICAL_LOCATION, *PI2O_PHYSICAL_LOCATION;

/* function declarations */

extern I2O_BUS_ID             i2oAdapterBusGet (I2O_ADAPTER_ID  adapterId,
                                                I2O_STATUS*     pStatus);

extern void i2oAdapterPhysLocGet (I2O_ADAPTER_ID                adapterId,
                                  I2O_PHYSICAL_LOCATION*        pPhys,
                                  I2O_STATUS*                   pStatus);

extern void i2oAdapterIntLock (I2O_ADAPTER_ID  adapterId,
                               I2O_STATUS*     pStatus);

extern void i2oAdapterIntUnlock (I2O_ADAPTER_ID  adapterId,
                                 I2O_STATUS*     pStatus);

extern U8        i2oAdapterConfigRead8(I2O_ADAPTER_ID        adapterId,
                                       I2O_ADDR32         offset,
                                       I2O_STATUS*        pStatus);
extern U16       i2oAdapterConfigRead16(I2O_ADAPTER_ID       adapterId,
                                        I2O_ADDR32        offset,
                                        I2O_STATUS*       pStatus);
extern U32       i2oAdapterConfigRead32(I2O_ADAPTER_ID       adapterId,
                                        I2O_ADDR32        offset,
                                        I2O_STATUS*       pStatus);
extern U64       i2oAdapterConfigRead64(I2O_ADAPTER_ID       adapterId,
                                        I2O_ADDR32        offset,
                                        I2O_STATUS*       pStatus);
extern void      i2oAdapterConfigWrite8(I2O_ADAPTER_ID    adapterId,
                                        I2O_ADDR32        offset,
                                        U8                value,
                                        I2O_STATUS*       pStatus);
extern void      i2oAdapterConfigWrite16(I2O_ADAPTER_ID   adapterId,
                                         I2O_ADDR32       offset,
                                         U16              value,
                                         I2O_STATUS*      pStatus);
extern void      i2oAdapterConfigWrite32(I2O_ADAPTER_ID   adapterId,
                                         I2O_ADDR32       offset,
                                         U32              value,
                                         I2O_STATUS*      pStatus);
extern void      i2oAdapterConfigWrite64(I2O_ADAPTER_ID   adapterId,
                                         I2O_ADDR32       offset,
                                         U64              value,
                                         I2O_STATUS*      pStatus);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCi2oAdapterLibh */
