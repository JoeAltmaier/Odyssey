#if 0
/* i2oDdmLib.h - i2o DDM support library header file */



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
#endif




#ifndef __INCi2oDdmLibh

#define __INCi2oDdmLibh



#ifdef __cplusplus

extern "C" {

#endif /* __cplusplus */



#include "i2o.h"

#include "i2otypes.h"

#include "i2oEvtQLib.h"

#include "i2oObjLib.h"

#include "i2oThreadLib.h"

#include "i2oErrorLib.h"

#include "i2oAdapterLib.h"

#include "i2omsg.h"



/* privates */

typedef struct 	i2oDdm* 	I2O_DDM_ID;	     /* DDM object */

typedef struct 	i2oDev* 	I2O_DEV_ID;	     /* DEV object */

typedef struct 	i2oDispatchId* 	I2O_DISPATCH_TBL_ID; /* Dispatch object */



typedef U16  I2O_ORG_ID;

typedef U16  I2O_MOD_ID;



typedef struct

    {

    I2O_ORG_ID  orgId;

    I2O_MOD_ID  modId;

    } I2O_DDM_TAG;





/* I2O DDM message codes */

#define I2O_DDM_ADAPTER_ATTACH      0xE0

#define I2O_DDM_ADAPTER_RECONFIG    0xE1

#define I2O_DDM_ADAPTER_RELEASE     0xE2

#define I2O_DDM_ADAPTER_RESUME      0xE3

#define I2O_DDM_ADAPTER_SUSPEND     0xE4

#define I2O_DDM_DEVICE_ATTACH       0xE5

#define I2O_DDM_DEVICE_RELEASE      0xE6

#define I2O_DDM_DEVICE_RESET        0xE7

#define I2O_DDM_DEVICE_RESUME       0xE8

#define I2O_DDM_DEVICE_SUSPEND      0xE9

#define I2O_DDM_SELF_RESET          0xEA

#define I2O_DDM_SELF_RESUME         0xEB

#define I2O_DDM_SELF_SUSPEND        0xEC

#define I2O_DDM_SYSTEM_CHANGE       0xED

#define I2O_DDM_SYSTEM_ENABLE       0xEE

#define I2O_DDM_SYSTEM_HALT         0xEF



#define I2O_USER_TID_UNCLAIMED	0xFFF



#define I2O_DDM_ATTACH_DEVICE_RESERVE_SZ	(32 - I2O_TID_SZ)



typedef U32           I2O_TID;

typedef U32           I2O_DEV_FLAGS;

typedef I2O_CLASS_ID  I2O_DEV_CLASS;

typedef U32           I2O_DEV_SUBCLASS;

typedef U8            I2O_DEV_IDENTITY_TAG;

typedef U32           I2O_DEV_EVENT_CAPABILITIES;

typedef U8            I2O_DEV_BIOS_INFO;

typedef U32           I2O_DEV_CHANGE_INDICATOR;



/* I2O adapter attach message structure */

typedef struct

    {

    I2O_MESSAGE_FRAME           StdMessageFrame;

    I2O_TRANSACTION_CONTEXT     TransactionContext;

    I2O_ADAPTER_ID              AdapterId;

} I2O_DDM_ADAPTER_ATTACH_MESSAGE, *PI2O_DDM_ADAPTER_ATTACH_MESSAGE;



/* I2O adapter reconfig message structure */

typedef struct

    {

    I2O_MESSAGE_FRAME           StdMessageFrame;

    I2O_TRANSACTION_CONTEXT     TransactionContext;

    I2O_ADAPTER_ID              AdapterId;

} I2O_DDM_ADAPTER_RECONFIG_MESSAGE, *PI2O_DDM_ADAPTER_RECONFIG_MESSAGE;



/* I2O adapter release message structure */

typedef struct

    {

    I2O_MESSAGE_FRAME           StdMessageFrame;

    I2O_TRANSACTION_CONTEXT     TransactionContext;

    I2O_ADAPTER_ID              AdapterId;

} I2O_DDM_ADAPTER_RELEASE_MESSAGE, *PI2O_DDM_ADAPTER_RELEASE_MESSAGE;



/* I2O adapter resume message structure */

typedef struct

    {

    I2O_MESSAGE_FRAME           StdMessageFrame;

    I2O_TRANSACTION_CONTEXT     TransactionContext;

    I2O_ADAPTER_ID              AdapterId;

} I2O_DDM_ADAPTER_RESUME_MESSAGE, *PI2O_DDM_ADAPTER_RESUME_MESSAGE;



/* I2O adapter suspend message structure */

typedef struct

    {

    I2O_MESSAGE_FRAME           StdMessageFrame;

    I2O_TRANSACTION_CONTEXT     TransactionContext;

    I2O_ADAPTER_ID              AdapterId;

} I2O_DDM_ADAPTER_SUSPEND_MESSAGE, *PI2O_DDM_ADAPTER_SUSPEND_MESSAGE;



/* I2O device attach message structure */

typedef struct

    {

    I2O_MESSAGE_FRAME           StdMessageFrame;

    I2O_TRANSACTION_CONTEXT     TransactionContext;

    U32                         Reserved:I2O_DDM_ATTACH_DEVICE_RESERVE_SZ;

    U32                         DeviceTID:I2O_TID_SZ;

} I2O_DDM_DEVICE_ATTACH_MESSAGE, *PI2O_DDM_DEVICE_ATTACH_MESSAGE;



/* I2O device release message structure */

typedef struct

    {

    I2O_MESSAGE_FRAME           StdMessageFrame;

    I2O_TRANSACTION_CONTEXT     TransactionContext;

    U32                         Reserved:I2O_DDM_ATTACH_DEVICE_RESERVE_SZ;

    U32                         DeviceTID:I2O_TID_SZ;

} I2O_DDM_DEVICE_RELEASE_MESSAGE, *PI2O_DDM_DEVICE_RELEASE_MESSAGE;



/* I2O device reset message structure */

typedef struct

    {

    I2O_MESSAGE_FRAME           StdMessageFrame;

    I2O_TRANSACTION_CONTEXT     TransactionContext;

    U32                         Reserved:I2O_DDM_ATTACH_DEVICE_RESERVE_SZ;

    U32                         DeviceTID:I2O_TID_SZ;

} I2O_DDM_DEVICE_RESET_MESSAGE, *PI2O_DDM_DEVICE_RESET_MESSAGE;



/* I2O device resume message structure */

typedef struct

    {

    I2O_MESSAGE_FRAME           StdMessageFrame;

    I2O_TRANSACTION_CONTEXT     TransactionContext;

    U32                         Reserved:I2O_DDM_ATTACH_DEVICE_RESERVE_SZ;

    U32                         DeviceTID:I2O_TID_SZ;

} I2O_DDM_DEVICE_RESUME_MESSAGE, *PI2O_DDM_DEVICE_RESUME_MESSAGE;



/* I2O device suspend message structure */

typedef struct

    {

    I2O_MESSAGE_FRAME           StdMessageFrame;

    I2O_TRANSACTION_CONTEXT     TransactionContext;

    U32                         Reserved:I2O_DDM_ATTACH_DEVICE_RESERVE_SZ;

    U32                         DeviceTID:I2O_TID_SZ;

} I2O_DDM_DEVICE_SUSPEND_MESSAGE, *PI2O_DDM_DEVICE_SUSPEND_MESSAGE;



/* I2O self reset message structure */

typedef struct

    {

    I2O_MESSAGE_FRAME           StdMessageFrame;

    I2O_TRANSACTION_CONTEXT     TransactionContext;

} I2O_DDM_SELF_RESET_MESSAGE, *PI2O_DDM_SELF_RESET_MESSAGE;



/* I2O self resume message structure */

typedef struct

    {

    I2O_MESSAGE_FRAME           StdMessageFrame;

    I2O_TRANSACTION_CONTEXT     TransactionContext;

} I2O_DDM_SELF_RESUME_MESSAGE, *PI2O_DDM_SELF_RESUME_MESSAGE;



/* I2O self suspend message structure */

typedef struct

    {

    I2O_MESSAGE_FRAME           StdMessageFrame;

    I2O_TRANSACTION_CONTEXT     TransactionContext;

} I2O_DDM_SELF_SUSPEND_MESSAGE, *PI2O_DDM_SELF_SUSPEND_MESSAGE;



/* I2O system change message structure */

typedef struct

    {

    I2O_MESSAGE_FRAME           StdMessageFrame;

    I2O_TRANSACTION_CONTEXT     TransactionContext;

} I2O_DDM_SYSTEM_CHANGE_MESSAGE, *PI2O_DDM_SYSTEM_CHANGE_MESSAGE;



/* I2O system enable message structure */

typedef struct

    {

    I2O_MESSAGE_FRAME           StdMessageFrame;

    I2O_TRANSACTION_CONTEXT     TransactionContext;

} I2O_DDM_SYSTEM_ENABLE_MESSAGE, *PI2O_DDM_SYSTEM_ENABLE_MESSAGE;



/* I2O system halt message structure */

typedef struct

    {

    I2O_MESSAGE_FRAME           StdMessageFrame;

    I2O_TRANSACTION_CONTEXT     TransactionContext;

} I2O_DDM_SYSTEM_HALT_MESSAGE, *PI2O_DDM_SYSTEM_HALT_MESSAGE;



typedef struct

    {

    I2O_EVENT_HANDLER	func;

    U8		msgCode;

    U8		pri;

    } I2O_FUNC_ENTRY;



typedef struct 

    {

    I2O_TID                        LocalTID;

    I2O_DEV_FLAGS                  Flags;

    I2O_DEV_CLASS                  Class;

    I2O_DEV_SUBCLASS               Subclass;

    I2O_TID                        UserTID;

    I2O_TID                        ParentTID;

    I2O_DEV_IDENTITY_TAG           IdentityTag[I2O_IDENTITY_TAG_SZ];

    I2O_DEV_EVENT_CAPABILITIES     EventCapabilities;

    I2O_DEV_BIOS_INFO              BiosInfo;

    I2O_DEV_CHANGE_INDICATOR       ChangeIndicator;

    } I2O_LCT_INFO;





/* function declarations */

extern I2O_DDM_ID	i2oDdmCreate (I2O_DDM_TAG               ddmTag, 

                                      int                       i2oVersion,

                                      I2O_LCT_INFO*             pLctInfo,

                                      I2O_OBJ_CONTEXT           devContext,

                                      I2O_THREAD_OPTIONS        tOptions,

                                      I2O_SIZE                  tStackSize,

                                      I2O_FUNC_ENTRY*           pFuncArray,

                                      I2O_COUNT                 numFuncs,

                                      I2O_STATUS*               pStatus);



extern I2O_DEV_ID	i2oDdmDevGet (I2O_DDM_ID                ddmId,

                                      I2O_STATUS*               pStatus);



extern void  		i2oDdmMpbStore (I2O_DDM_ID              ddmId,

                                        I2O_SG_ELEMENT*	        pMpb,

                                        I2O_STATUS*             pStatus);



extern void  		i2oDdmTidRelease (I2O_DDM_ID            ddlTid,

                                          I2O_TID               tid,

                                          I2O_STATUS*           pStatus);



extern I2O_DEV_ID	i2oDevCreate (I2O_OWNER_ID              ownerId,

                                      I2O_OBJ_CONTEXT           devContext,

                                      I2O_EVENT_QUEUE_ID            evtQId, 

                                      I2O_DISPATCH_TBL_ID       dispatchId,

                                      I2O_LCT_INFO*             pLctInfo,

                                      I2O_STATUS*               pStatus);



extern I2O_TID		i2oDevTidGet   (I2O_DEV_ID              devId,

                                        I2O_STATUS*             pStatus);



extern I2O_EVENT_QUEUE_ID   i2oDevEventQGet (I2O_DEV_ID             devId,

                                         I2O_STATUS*            pStatus);



extern I2O_DISPATCH_TBL_ID i2oDevDispatchTblGet (I2O_DEV_ID         devId,

                                             I2O_STATUS*        pStatus);



extern void		i2oDevLctInfoGet (I2O_DEV_ID            devId,

                                          I2O_LCT_INFO*         pLctInfo,

                                          I2O_STATUS*           pStatus);



extern void		i2oDevLctFlagsSet(I2O_DEV_ID            devId,

                                          I2O_DEV_FLAGS         devFlags,

                                          I2O_DEV_FLAGS         devMask,

                                          I2O_STATUS*           pStatus);



extern void		i2oDevUserTidSet (I2O_DEV_ID		devId,

                                          I2O_TID		userTid,

                                          I2O_DEV_FLAGS         claimFlags,

                                          I2O_STATUS*		pStatus);



extern I2O_DISPATCH_TBL_ID  i2oDispatchTblCreate 

                                  (I2O_OWNER_ID                 ownerId,

                                   I2O_FUNC_ENTRY*              pFuncArray,

                                   I2O_COUNT                    numFuncs,

                                   I2O_STATUS*                  pStatus);

#ifdef __cplusplus

}

#endif /* __cplusplus */



#endif /* __INCi2oDdmLibh */

