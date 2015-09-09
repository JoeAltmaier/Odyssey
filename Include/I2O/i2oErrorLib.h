/* i2oErrorLib.h - error library header file */



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





#ifndef __INCi2oErrorLibh

#define __INCi2oErrorLibh



#ifdef __cplusplus

extern "C" {

#endif /* __cplusplus */



/* i2o error/status typedefs */



typedef	enum		

    {

    I2O_STS_OK			 	= 0,

    I2O_STS_INVALID_OBJ_ID		= 1,

    I2O_STS_INVALID_OWNER_ID		= 2,

    I2O_STS_NOT_ISR_CALLABLE		= 3,

    I2O_STS_TIMER_IN_USE		= 4,

    I2O_STS_TIMER_EVT_IN_USE		= 5,

    I2O_STS_TIMER_INACTIVE		= 6,

    I2O_STS_INVALID_OBJ_NAME		= 7,

    I2O_STS_NOT_IMPLEMENTED		= 8,

    I2O_STS_ALLOCATION_FAILED		= 9,

    I2O_STS_MAX_EVENTS_OUTSTANDING	= 10,

    I2O_STS_INVALID_ISR_FUNC		= 11,

    I2O_STS_LIB_NOT_INSTALLED		= 12,

    I2O_STS_UNKNOWN_ERROR		= 13,

    I2O_STS_FUNC_NOT_IMPLEMENTED	= 14,

    I2O_STS_FRAME_ALREADY_FREE		= 15,

    I2O_STS_FRAME_ALLOCATION_FAILED	= 16,

    I2O_STS_FRAME_POST_ERROR		= 17,

    I2O_STS_FRAME_INVALID		= 18,

    I2O_STS_FRAME_DISPATCH_ERROR	= 19,

    I2O_STS_DMA_UNSUPPORTED_BUS		= 20,

    I2O_STS_DMA_INVALID			= 21,

    I2O_STS_DMA_ERROR			= 22,

    I2O_STS_DMA_CHAIN_ERROR		= 23,

    I2O_STS_TOO_MANY_ENTRIES		= 24,

    I2O_STS_TID_ERROR			= 25,

    I2O_STS_ARG_NOT_IMPLEMENTED		= 26,

    I2O_STS_DMA_PAGE_SIZE_ERROR		= 27,

    I2O_STS_ARG_ERROR			= 28,

    I2O_STS_INVALID_USER_FUNC		= 29,

    I2O_STS_INVALID_BLOCK		= 30,

    I2O_STS_SEM_INIT_ERROR		= 31,

    I2O_STS_SEM_TAKE_ERROR		= 32,

    I2O_STS_SEM_GIVE_ERROR		= 33,

    I2O_STS_PIPE_SEND_ERROR		= 34,

    I2O_STS_PIPE_RECEIVE_ERROR		= 35,

    I2O_STS_DMA_FULL  			= 36,

    I2O_STS_INVALID_EVENT_PRI		= 37,

    I2O_STS_INVALID_ERR_ACT		= 38,

    I2O_STS_INSUFFICIENT_MEMORY		= 39,

    I2O_STS_INVALID_PAGE_ARRAY		= 40,

    I2O_STS_INVALID_CONFIG_MESSAGE      = 41,

    I2O_STS_DMA_CANCEL_FAILED		= 42,

    I2O_STS_DMA_RESUME_FAILED		= 43,

    I2O_STS_DMA_OBJECT_SUSPENDED	= 44,

    I2O_STS_MPB_SIZE_ERROR              = 45,

    I2O_STS_NO_NVS			= 46,

    I2O_STS_NVS_CREATE_ERROR		= 47,

    I2O_STS_NVS_OPEN_ERROR		= 48,

    I2O_STS_NVS_CLOSE_ERROR		= 49,

    I2O_STS_NVS_DELETE_ERROR		= 50,

    I2O_STS_NVS_WRITE_ERROR		= 51,

    I2O_STS_BBU_NOT_PRESENT		= 52,

    I2O_STS_INSUFFICIENT_NVRAM		= 53,

    I2O_STS_NVRAM_ACCESS_ERROR		= 54,

    I2O_STS_INVALID_EVENT_HANDLER	= 55,

    I2O_STS_OBJ_DESTROY_ERROR		= 56,

    I2O_STS_UNAVAILABLE			= 57,

    I2O_STS_TIMEOUT			= 58

    } I2O_STATUS;					



typedef void		(*I2O_ERROR_FUNC) (I2O_STATUS);  /* I2O_ERROR_FUNC */

    

#define	I2O_NO_STATUS	(I2O_STATUS *) NULL	/* I2O_NO_STATUS */



/* i2o error action codes */



typedef	enum					/* I2O_ERROR_ACTION */

    {

    I2O_ERR_ACT_DEFAULT		= 0,

    I2O_ERR_ACT_PRINTMSG	= 1,

    I2O_ERR_ACT_IGNORE		= 2,

    I2O_ERR_ACT_USER		= 3,

    I2O_ERR_ACT_DEBUG		= 4

    } I2O_ERROR_ACTION;



/* function declarations */



extern void		i2oErrorSet (I2O_STATUS errorCode, 

			  	     I2O_STATUS * status);

extern void		i2oErrorAction (I2O_STATUS errorCode, 

					I2O_ERROR_ACTION errorAction);



#ifdef __cplusplus

}

#endif /* __cplusplus */



#endif /* __INCi2oErrorLibh */



