/*************************************************************************
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// 
*************************************************************************/
#include "ImageIterator.h"

Image::Image(
		RowId imageKey_,
		U32 majorVersion_, 
		U32 minorVersion_,
		U32 cbImage_,
		UnicodeString* name_, 
		I64* timeCreated_,
		I64* timeLoaded_,
		U32 type_,
		U32 iopCount_)
{
	ii.imageKey = imageKey_;
	ii.majorVersion = majorVersion_;
	ii.minorVersion = minorVersion_;
	ii.cbImage = cbImage_;
	name_->CString(ii.name, sizeof(ii.name));
	ii.timeCreated = *timeCreated_;
	ii.timeLoaded = *timeLoaded_;
	ii.type = type_;
	ii.iopCount = iopCount_;
	ii.isDefault = FALSE;
	iterator = NULL;
}

Image::Image(void* pData, ImageIterator* iterator_)
{
	memcpy(&ii, pData, sizeof(ii));
	iterator = iterator_;
}

//const 
ImageDesc* Image::GetAssociation(TySlot slot) const
{
	//const 
	ImageDesc* imageDesc = NULL;
	IOPDesc* iopDesc = iterator->GetIOPDesc(slot);

	if (iopDesc != NULL && 
		(ii.imageKey == iopDesc->ImageOne() ||
		ii.imageKey == iopDesc->ImageTwo()))
	{
		BOOL current = (ii.imageKey == iopDesc->CurrentImage());
		I64 timeBooted = 0;
		if (current)
			iopDesc->TimeBooted(&timeBooted);
		BOOL primary = (ii.imageKey == iopDesc->PrimaryImage());
		BOOL accepted = iopDesc->Accepted(ii.imageKey);
		imageDesc = new ImageDesc(iopDesc->Key(), current, 
			&timeBooted, primary, accepted);
	}

	return imageDesc;
}

IOPDesc::IOPDesc(RowId key,
		TySlot slot,
		RowId primaryImage,
		RowId currentImage,
		I64* timeBooted,
		RowId imageOne,
		RowId imageTwo,
		BOOL imageOneAccepted,
		BOOL imageTwoAccepted) 
{
	iopi.key = key;
	iopi.slot = slot;
	iopi.primaryImage = primaryImage;
	iopi.currentImage = currentImage;
	iopi.timeBooted = *timeBooted;
	iopi.imageOne = imageOne;
	iopi.imageTwo = imageTwo;
	iopi.imageOneAccepted = imageOneAccepted;
	iopi.imageTwoAccepted = imageTwoAccepted;
}

IOPDesc::IOPDesc(void* pData)
{
	memcpy(&iopi, pData, sizeof(iopi));
}

const U16 IMAGE_ITERATOR_ARRAY_GROW_SIZE = 2;

ImageIterator::ImageIterator() 
{
	iii.numberOfImages = 0;
	pImages = NULL;
	iii.numberOfIOPDescs = 0;
	pIOPDescs = NULL;
}

ImageIterator::ImageIterator(void *pData) 		// a blob containing everything about the event
{
	Set(pData);
}

void ImageIterator::Set(void *pData)
{
	/* <total bytes><iii><IOPDesc1>...<IOPDescN><ImageBlob1>...<ImageBlobN>*/

	U16 i;
	U32 cb = 0;
	
	char *p = (char *)pData;
	memcpy(&cb, p, sizeof(cb));	// total bytes
	p += sizeof(cb);
	memcpy(&iii, p, sizeof(iii));
	p += sizeof(iii);

	if (iii.numberOfIOPDescs > 0)
	{
		pIOPDescs = new IOPDesc *[(iii.numberOfIOPDescs)];
		assert (pIOPDescs);
		for (i = 0; i < iii.numberOfIOPDescs; i++)
		{
			pIOPDescs[i] = new IOPDesc(p);
			assert(pIOPDescs[i]);
			p += sizeof(IOPDesc);
		}
	}
	else
		pIOPDescs = NULL;
	
	if (iii.numberOfImages > 0)
	{
		pImages = new Image *[(iii.numberOfImages)];
		assert(pImages);
		for (i = 0; i < iii.numberOfImages; i++)
		{
			pImages[i] = new Image(p, this);
			assert(pImages[i]);
			p += sizeof(Image);
		}
	}
	else
		pImages = NULL;
}


ImageIterator::~ImageIterator()
{
	U16 i;

	for (i=0; i < iii.numberOfImages; ++i)
	{
		delete pImages[i];
	}

	for (i=0; i < iii.numberOfIOPDescs; ++i)
	{
		delete pIOPDescs[i];
	}
	delete []pImages;
}

U32 ImageIterator::GetImageInfoData(void **pData_) const
{
   /*
	* build a blob containing the Image data.  
	* the blob is formatted as follows:
	* <total bytes><iii><IOPDesc1>...<IOPDescN><ImageBlob1>...<ImageBlobN>
	*/
	U16 i;
	U32 cb = 0;

	cb = sizeof(cb) + sizeof(iii) + (iii.numberOfImages * sizeof(Image)) +
		(iii.numberOfIOPDescs * sizeof(IOPDesc));

	*pData_ = new (tBIG) char[cb];  // allocate the blob.  
	assert(*pData_);

	char *p = (char *)*pData_;

	memcpy(p, &cb, sizeof(cb));	// total bytes
	p += sizeof(cb);	

	memcpy(p, &iii, sizeof(iii));	// Image Iterator data
	p += sizeof(iii);

	for (i=0; i < iii.numberOfIOPDescs; i++)
	{
		memcpy(p, pIOPDescs[i], sizeof(IOPDesc));
		p += sizeof(IOPDesc);
	}

	for (i=0; i < iii.numberOfImages; i++)
	{
		memcpy(p, pImages[i], sizeof(Image));
		p += sizeof(Image);
	}

	return cb;
}

IOPDesc* ImageIterator::GetIOPDesc(TySlot slot) const
{
	for (U16 i =0; i < iii.numberOfIOPDescs; i++)
	{
		if (pIOPDescs[i]->Slot() == slot)
			return pIOPDescs[i];
	}
	return NULL;
}


// append an Image to the Image array
void ImageIterator::Append(Image *pImage)
{
	if (iii.numberOfImages % IMAGE_ITERATOR_ARRAY_GROW_SIZE == 0)
	{
		Image** pTemp = pImages;
		if (iii.numberOfImages ==0)
			pImages = new Image *[ IMAGE_ITERATOR_ARRAY_GROW_SIZE];
		else
			pImages = new Image *[iii.numberOfImages + IMAGE_ITERATOR_ARRAY_GROW_SIZE];
		assert(pImages);
		memcpy(pImages, pTemp, (iii.numberOfImages * sizeof(Image*)));
		delete pTemp;
	}
	pImages[iii.numberOfImages++] = pImage;
}

void ImageIterator::Append(IOPDesc *pIOPDesc)
{
	if (iii.numberOfIOPDescs % IMAGE_ITERATOR_ARRAY_GROW_SIZE == 0)
	{
		IOPDesc** pTemp = pIOPDescs;
		if (iii.numberOfIOPDescs ==0)
			pIOPDescs = new IOPDesc *[ IMAGE_ITERATOR_ARRAY_GROW_SIZE];
		else
			pIOPDescs = new IOPDesc *[iii.numberOfIOPDescs + IMAGE_ITERATOR_ARRAY_GROW_SIZE];
		assert(pIOPDescs);
		memcpy(pIOPDescs, pTemp, (iii.numberOfIOPDescs * sizeof(IOPDesc*)));
		delete pTemp;
	}
	pIOPDescs[iii.numberOfIOPDescs++] = pIOPDesc;
}


