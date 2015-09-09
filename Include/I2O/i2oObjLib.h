/* i2oObjLib.h - i2o object library header file */

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


#ifndef __INCi2oObjLibh
#define __INCi2oObjLibh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "i2oErrorLib.h"
#include "i2otypes.h"

/* i2o object typedefs */

typedef struct i2o_obj_core * 	I2O_OBJ_ID;		/* i2o object ID */
typedef char 			I2O_OBJ_NAME; 		/* obj name */
typedef void *			I2O_OBJ_CONTEXT;	/* obj context */
typedef I2O_OBJ_ID		I2O_OWNER_ID;		/* owner ID */

/* function declarations */

extern void		i2oObjDestroy (I2O_OBJ_ID objectId, 
				       I2O_STATUS * pStatus);
extern I2O_OWNER_ID	i2oObjOwnerGet (I2O_OBJ_ID objectId,
					I2O_STATUS * pStatus);
extern void 		i2oObjOwnerSet (I2O_OBJ_ID objectId, 
					I2O_OWNER_ID ownerId, 
					I2O_STATUS * pStatus);
extern I2O_OBJ_NAME *	i2oObjNameGet (I2O_OBJ_ID objectId, 
			 	       I2O_STATUS * pStatus);
extern void 		i2oObjNameSet (I2O_OBJ_ID objectId, 
				       I2O_OBJ_NAME * pName,
				       I2O_STATUS * pStatus);
extern I2O_OBJ_CONTEXT	i2oObjContextGet (I2O_OBJ_ID objectId,
					  I2O_STATUS * pStatus);
extern void 		i2oObjContextSet (I2O_OBJ_ID objectId, 
					  I2O_OBJ_CONTEXT context,
					  I2O_STATUS * pStatus);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCi2oObjLibh */

