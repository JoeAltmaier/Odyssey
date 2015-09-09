/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfConfig.cpp
// 
// Description:
// This file sets up the Flash File driver variables from the 
// configuration information. 
// 
// 8/12/98 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "ErrorLog.h"
#include "FfInterface.h"
#include "String.h"
#include "TraceMon.h"

/*************************************************************************/
// FF_Interface::Initialize_Config
// Create FF_Config object.
/*************************************************************************/
Status FF_Interface::Initialize_Config(FF_CONFIG *p_config)
{	
	
	// Check version of FF_CONFIG record.
	if (p_config->version != FF_CONFIG_VERSION)
		return FF_ERROR(INVALID_CONFIG_VERSION);
	
	// Configuration data is valid.
	return OK;
		
} // FF_Interface::Initialize_Config



