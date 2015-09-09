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

#include "UpgradeMasterMessages.h"

MsgAddImage::MsgAddImage(U32 cbImage_, void *pImage_)
	 : Message(REQ_UPGRADE_ADD_IMAGE), cbImage(cbImage_)
	{
		void* pImage = new (tBIG) char[cbImage];
		assert(pImage);
		memcpy(pImage, pImage_, cbImage);
		AddSgl(IMAGE_SGL, pImage, cbImage);
	}

	// D'tor can't be virtual, so the message must be cast prior to deletion.
MsgAddImage::~MsgAddImage() { if (IsLast()) CleanAllSgl(); }  // delete client allocated SGLs
	
U32 MsgAddImage::GetImage(void **ppImage_) 
	{
		*ppImage_ = new (tBIG) char[cbImage];
		assert(*ppImage_);
		CopyFromSgl(IMAGE_SGL, 0, *ppImage_, cbImage);
		return cbImage;
	}
