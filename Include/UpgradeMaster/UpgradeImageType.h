/*************************************************************************
*
* This material is a confidential trade secret and proprietary 
* information of ConvergeNet Technologies, Inc. which may not be 
* reproduced, used, sold or transferred to any third party without the 
* prior written consent of ConvergeNet Technologies, Inc.  This material 
* is also copyrighted as an unpublished work under sections 104 and 408 
* of Title 17 of the United States Code.  Law prohibits unauthorized 
* use, copying or reproduction.
*
* Description:
*	This file contains the definition of the Upgrade Master messages.
*************************************************************************/

// Revision History:
// $Log: /Gemini/Include/UpgradeMaster/UpgradeImageType.h $
// 
// 2     2/06/00 2:35p Jfrandeen
// Fix SSD type
// 
// 1     11/17/99 3:23p Joehler
// Add command queue to Upgrade Master
// 

#ifndef UpgradeImageType_h
#define UpgradeImageType_h

// image types
typedef enum {
	ALL_IMAGES = 0,
	HBC_IMAGE,
	NAC_IMAGE, 
	SSD_IMAGE,
	NIC_IMAGE, 
	RAC_IMAGE
} ImageType;

#endif