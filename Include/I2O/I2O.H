#if 0
/* i2o.h - i2o header file */



/****************************************************************************

All software on this website is made available under the following terms

and conditions. By downloading this software, you agree to abide by these

terms and conditions with respect to this software.



I2O SIG All rights reserved.



These header files are provided, pursuant to your I2O SIG membership

agreement, free of charge on an as-is basis without warranty of any kind, 

either express or implied, including but not limited to, implied 

warranties or merchantability and fitness for a particular purpose.  

I2O SIG does not warrant that this program will meet the user's 

requirements or that the operation of these programs will be uninterrupted 

or error-free. Acceptance and use of this program constitutes the user's 

understanding that he will have no recourse to I2O SIG for any actual 

or consequential damages including, but not limited to, loss profits 

arising out of use or inability to use this program.



Member is permitted to create derivative works to this header-file

program.  However, all copies of the program and its derivative works 

must contain the I2O SIG copyright notice.

**************************************************************************/
#endif


#ifndef __INCi2oh

#define __INCi2oh



#define I2OAPI_REV 1_5_2



#include "i2otypes.h"

#include "i2omsg.h"



/* version info */



/* I2O Architecture specification version 1.5d. */



#define I2O_VERSION_15D		1



/* I2O version number for this release. */



#define I2O_VERSION_NUM		I2O_VERSION_15D



/* Message frame header version. */



#define I2O_FRAME_HDR_VERSION	I2O_VERSION_11



#define I2O_NO_WAIT		0

#define I2O_WAIT_FOREVER	((I2O_USECS) -1)



#endif /* __INCi2oh */

