/* _chaos.readme.h -- Chaos Build Notes
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99
 * Copyright (C) Dell Computer, 2000
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
**/

#if false

- CHAOS Build Info -

The CHAOS library is divided into several areas which logically makeup the 
operating system.  The system may be used at serveral different levels with
or without the additional components.

CORE: The CHAOS core.
	This level allows basic OS functionality.  Only single processor images
	may be linked and run.
	
	Purpose:
		Build and test basic single board CHAOS based images.

	Platforms:
		Odyssey
		Windows
		Evals
	
	Caviat:
		No PTS or proxy services are required at this level.
	
	BuildSys Support:
		CLASSNAME		DEVICENAME
		SERVELOCAL		SERVEVIRTUAL
		CLASSENTRY		DEVICEENTRY
		SYSTEMENTRY		SYSTEMMASTER
		BOOTENTRY
		
BASE Services:
	Adds additional optional basic OS services not required by the core.
	
	Purpose:
		Build and test base single board CHAOS images.

	Platforms:
		Odyssey
		Windows
		Evals
		
	Caviat:
		No PTS or proxy services are required at this level.
		
	BuildSys Support:
		CLASSNAME		DEVICENAME
		SERVELOCAL		SERVEVIRTUAL
		CLASSENTRY		DEVICEENTRY
		SYSTEMENTRY		SYSTEMMASTER
		BOOTENTRY	

PERSISTENT Services:
	Adds additional optional PTS based OS services.
	
	Purpose:
		Build and test base single board CHAOS images which may include
		VirtualDevice(s) and other PTS based services.

	Platforms:
		Odyssey
		Windows
		Evals
		
	Caviat:
		Must link with PTS and the DdmCmbNull proxy service.
		Need to #define FLASH_STORAGE to make PTS run ram based only.
		
	BuildSys Support:
		CLASSNAME		DEVICENAME
		SERVELOCAL		SERVEVIRTUAL
		CLASSENTRY		DEVICEENTRY
		SYSTEMENTRY		SYSTEMMASTER
		BOOTENTRY		VIRTUALENTRY	
				
TRANSPORT Servies:
	Adds PCI and Network transport services to the OS. 
	
	Purpose:
		Multiboard OS features maybe implemented and/or tested.

	Platforms:
		Odyssey only
		
	Caviat:
		Must link with real CDdmCmb and the DdmBootProxy service.
		Need to #define FLASH_STORAGE to make PTS run ram based only.
	
	BuildSys Support:
		CLASSNAME		DEVICENAME
		SERVELOCAL		SERVEVIRTUAL
		CLASSENTRY		DEVICEENTRY
		SYSTEMENTRY		SYSTEMMASTER
		BOOTENTRY		VIRTUALENTRY
		SUSPENDNAME		FAILNAME
		SUSPENDENTRY	FAILENTRY

DEBUG Services:
	These services provide additional OS support to aid in debugging unit
	testing images or complete application images.

	Platforms:
		Depends on the service.
	
PROXY Services:
	These services allow you to build CHAOS application with will bring
	up smaller images for unit testing the OS or some applications.
	
	Platforms:
		Depends on the service.

#endif

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/_chaos.readme.h $
// 
// 1     2/15/00 6:13p Tnelson
// Proxy services for testing

