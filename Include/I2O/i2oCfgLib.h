/* i2oConfigLib.h - DDM configuration support library header file */

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


#ifndef __INCi2oConfigLibh
#define __INCi2oConfigLibh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "i2o/shared/i2omsg.h"
#include "i2o/i2oObjLib.h"
#include "i2o/i2oDdmLib.h"

/* Field set/get, row add/delete function */

typedef U8 (*I2O_PARAMS_FIELD_ACCESS_FUNC) ();

/* Group clear function */

typedef U8 (*I2O_PARAMS_CLEAR_FUNC)
    (
    I2O_OBJ_CONTEXT context,   /* User-defined context */
    U16             groupNum   /* Group number */
    );

/* Get keys function */

typedef U8 (*I2O_PARAMS_GET_KEYS_FUNC)
    (
    I2O_OBJ_CONTEXT context,          /* User-defined context */
    U16             groupNum,         /* Group number */
    I2O_ADDR32      pPrevKey,         /* Previous key value */
    I2O_COUNT       resultBufLen,     /* Length of result buffer */
    I2O_ADDR32      pResultBuf,       /* Result buffer */
    U16*            pNumKeysReturned, /* Number of keys returned */
    U16*            pNumKeysTotal     /* Total number of keys in group */
    );

/* I2O field info structure */

typedef struct _I2O_PARAMS_FIELD_DEF
    {
    I2O_PARAMS_FIELD_ACCESS_FUNC addSetFunc;  /* write field or add a row */
    I2O_PARAMS_FIELD_ACCESS_FUNC delGetFunc;  /* read field or delete a row */
    I2O_COUNT                    size;        /* Size in bytes of this field */
    U32                          constant;    /* Const associated with field */
} I2O_PARAMS_FIELD_DEF;

/* I2O group info structure */

typedef struct _I2O_PARAMS_GROUP_DEF
    {
    U16                      groupNumber;   /* Group ID */
    U16                      fieldCount;    /* Number of fields in group */
    I2O_PARAMS_FIELD_DEF*    pFieldArray;   /* Array of field definitions */
    I2O_PARAMS_GET_KEYS_FUNC getKeysFunc;   /* Func returning keys of group */
    I2O_PARAMS_CLEAR_FUNC    clearFunc;     /* Func to delete all rows */
} I2O_PARAMS_GROUP_DEF;

#define I2O_PARAMS_FIELD_COUNT(X) (sizeof(X) / sizeof(I2O_PARAMS_FIELD_DEF))
#define I2O_PARAMS_GROUP_COUNT(X) (sizeof(X) / sizeof(I2O_PARAMS_GROUP_DEF))

/* Event client info structure */

typedef struct _I2O_EVENT_CLIENT_INFO
{
    I2O_TID               TargetAddress;
    I2O_TID               InitiatorAddress;
    U64                   InitiatorContext;
    U32                   Flags;
    U64                   TransactionContext;
    U32                   EventMask;
} I2O_EVENT_CLIENT_INFO;

/* Event client info flags bits */

#define I2O_EVENT_CLIENT_FLAGS_CONTEXT_64  0x00000002

/* Event client list structure */

typedef struct _I2O_EVENT_CLIENT_LIST
{
    I2O_COUNT             clientCount;
    U32                   reserved;
    I2O_EVENT_CLIENT_INFO clientList[1];
} I2O_EVENT_CLIENT_LIST;

/* function prototypes */

extern void i2oCfgParamMsgReply (I2O_DEV_ID              devId,
				 I2O_OBJ_CONTEXT         context,
				 I2O_MESSAGE_FRAME *     pMsg,
				 I2O_COUNT               numGroups,
				 I2O_PARAMS_GROUP_DEF *  pGroupArray,
				 I2O_EVENT_CLIENT_LIST * pEventClientList,
				 I2O_STATUS *            pStatus);

extern void i2oCfgDialogMsgReply (I2O_DEV_ID              devId,
				  I2O_OBJ_CONTEXT         context,
				  I2O_COUNT               setNum,
				  I2O_MESSAGE_FRAME *     pMsg,
				  I2O_COUNT               numGroups,
				  I2O_PARAMS_GROUP_DEF *  pGroupArray,
				  I2O_EVENT_CLIENT_LIST * pEventClientList,
				  I2O_STATUS *            pStatus);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCi2oConfigLibh */
