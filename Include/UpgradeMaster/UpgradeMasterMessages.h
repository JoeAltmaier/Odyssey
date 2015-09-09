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
// $Log: /Gemini/Include/UpgradeMaster/UpgradeMasterMessages.h $
// 
// 11    1/26/00 2:33p Joehler
// Split out some functionality to UpgradeMasterMessages.cpp and allocate
// memory from big heap.
// 
// 10    12/09/99 4:10p Joehler
// Added Modify default image message
// 
// 9     11/17/99 7:30p Agusev
// Fixed GetImages if there are no images in system
// 
// 8     11/17/99 3:23p Joehler
// Add command queue to upgrade master
// 
// 7     10/15/99 2:07p Joehler
// Modified so type is not an arg to AssignDefaultImage
// 
// 6     10/15/99 10:58a Joehler
// Moved upgrade master error codes to *.mc files to localize
// 
// 5     10/14/99 5:28p Joehler
// Fixed bug when returning opened image.
// 
// 4     10/12/99 11:14a Joehler
// Modifications for Query to assocaite IOP with Image
// 
// 3     10/07/99 9:56a Joehler
// Modified comments
// 
// 2     10/06/99 4:21p Joehler
// added error checking
// 
// 1     9/30/99 7:43a Joehler
// First cut of Upgrade Master
// 
//			

#ifndef UpgradeMasterMessages_h
#define UpgradeMasterMessages_h

#include "Trace_Index.h"		// include the trace module numbers

#undef TRACE_INDEX
#define	TRACE_INDEX		TRACE_UPGRADE	// set this modules index to your index	
#include "Odyssey_Trace.h"			// include trace macros

#include "CTTypes.h"
#include "message.h"
#include "RequestCodes.h"
#include "FileSystemInfo.h"
#include "ImageIterator.h"
#include "UpgradeImageType.h"

class MsgAddImage : public Message
{
	friend DdmUpgrade;

public:
	// SGL indexes for the data being sent
	enum {IMAGE_SGL};

	// Ctor: add the SGLs for the image	
	MsgAddImage(U32 cbImage_, void *pImage_);

	// D'tor can't be virtual, so the message must be cast prior to deletion.
	~MsgAddImage();
	
	RowId GetImageKey() { return imageKey; }

protected:

	// allocate storage for the alarm context, and copy the data.  The
	// caller supplies the pointer and is responsible for deleting it.
	U32 GetImage(void **ppImage_);

	U32 GetImageSize() { return cbImage; }
			
	void SetImageKey(RowId imageKey_) { imageKey = imageKey_; }

private:
	// send data:
	U32 cbImage;
	// reply data:
	RowId imageKey;
};

class MsgQueryImages : public Message
{
	friend DdmUpgrade;

public:	
	// SGL indexes for the data being sent
	enum {IMAGES_SGL};
		
	// Ctor
	MsgQueryImages()
		: Message(REQ_UPGRADE_QUERY_IMAGES), type(ALL_IMAGES)
	{
		// allocate the SGLS for reply
		AddSgl(IMAGES_SGL, NULL, 0, SGL_DYNAMIC_REPLY);
	}

	MsgQueryImages(ImageType type_)
		: Message(REQ_UPGRADE_QUERY_IMAGES), type(type_)
	{
		// allocate the SGLS for reply
		AddSgl(IMAGES_SGL, NULL, 0, SGL_DYNAMIC_REPLY);
	}

	// D'tor can't be virtual, so the message must be cast prior to deletion.
	~MsgQueryImages() { if (IsLast()) CleanAllSgl(); }  // delete client allocated SGLs
	
	void GetImages(ImageIterator** ppImages)
	{
		if (cbData != 0)
		{
			void* pData = new (tBIG) char[cbData];
			CopyFromSgl(IMAGES_SGL, 0, pData, cbData);
			*ppImages = new ImageIterator(pData);
			assert(*ppImages);
			delete pData;
		}
		else
			*ppImages = new ImageIterator();
			
	}

protected:

	ImageType GetType() { return type; }

	void SetImages(ImageIterator* pImages)
	{
		void* pData;
		cbData = pImages->GetImageInfoData(&pData);
		AllocateSgl(IMAGES_SGL, cbData);
		CopyToSgl(IMAGES_SGL, 0, pData, cbData);
		delete pData;
	}

private:
	// send data:
	ImageType type;
	// return data:
	U32 cbData;
};

class MsgOpenImage : public Message
{
	friend DdmUpgrade;

public:	
	// SGL indexes for the data being sent
	enum {IMAGE_SGL};
		
	// Ctor
	MsgOpenImage(RowId imageKey_)
		: Message(REQ_UPGRADE_OPEN_IMAGE), imageKey(imageKey_)
	{
		// allocate the SGLS for reply
		AddSgl(IMAGE_SGL, NULL, 0, SGL_DYNAMIC_REPLY);
	}

	// D'tor can't be virtual, so the message must be cast prior to deletion.
	~MsgOpenImage() { if (IsLast()) CleanAllSgl(); }  // delete client allocated SGLs
	

	U32 GetImage(void** ppImage)
	{
		*ppImage = new (tBIG) char[cbImage];
		CopyFromSgl(IMAGE_SGL, 0, *ppImage, cbImage);
		return cbImage;
	}

protected:

	RowId GetImageKey() { return imageKey; }
	
	void SetImage(void* pImage, U32 cbImage_)
	{
		cbImage = cbImage_;
		AllocateSgl(IMAGE_SGL, cbImage);
		CopyToSgl(IMAGE_SGL, 0, pImage, cbImage);
	}

private:
	// input data
	RowId imageKey;
	// return data:
	U32 cbImage;
};

class MsgAssociateImage : public Message
{
	friend DdmUpgrade;

public:
	MsgAssociateImage(RowId imageKey_, TySlot slot_)
	 : Message(REQ_UPGRADE_ASSOCIATE_IMAGE), slot(slot_), 
	 imageKey(imageKey_) { }

	~MsgAssociateImage() { }  // delete client allocated SGLs
	
protected:

	RowId GetImageKey() { return imageKey; }
	
	TySlot GetSlot() { return slot; }

private:
	// send data:
	RowId imageKey;
	TySlot slot;
};

class MsgMakePrimary : public Message
{
	friend DdmUpgrade;

public:
	MsgMakePrimary(RowId imageKey_, TySlot slot_)
	 : Message(REQ_UPGRADE_MAKE_PRIMARY), slot(slot_), 
	 imageKey(imageKey_) { }

	~MsgMakePrimary() { }  // delete client allocated SGLs
	
protected:

	RowId GetImageKey() { return imageKey; }
	
	TySlot GetSlot() { return slot; }

private:
	// send data:
	RowId imageKey;
	TySlot slot;
};

class MsgModifyDefaultImage : public Message
{
	friend DdmUpgrade;

public:
	// SGL indexes for the data being sent
	enum {IMAGE_SGL};
	
	// Ctor: add the SGLs for the image	
	MsgModifyDefaultImage(RowId imageKey_, U32 cbImage_, void *pImage_)
	 : Message(REQ_UPGRADE_MODIFY_DEFAULT_IMAGE), cbImage(cbImage_), 
	 imageKey(imageKey_)
	{
		void* pImage = new (tBIG) char[cbImage];
		memcpy(pImage, pImage_, cbImage);
		AddSgl(IMAGE_SGL, pImage, cbImage);
	}

	// D'tor can't be virtual, so the message must be cast prior to deletion.
	~MsgModifyDefaultImage() { if (IsLast()) CleanAllSgl(); }  // delete client allocated SGLs
	
protected:

	RowId GetImageKey() { return imageKey; }

	// allocate storage for the alarm context, and copy the data.  The
	// caller supplies the pointer and is responsible for deleting it.
	U32 GetImage(void **ppImage_) 
	{
		*ppImage_ = new (tBIG) char[cbImage];
		CopyFromSgl(IMAGE_SGL, 0, *ppImage_, cbImage);
		return cbImage;
	}

	U32 GetImageSize() { return cbImage; }
			
	void SetImageKey(RowId imageKey_) { imageKey = imageKey_; }

private:
	// send data:
	RowId imageKey;
	U32 cbImage;
};

#endif