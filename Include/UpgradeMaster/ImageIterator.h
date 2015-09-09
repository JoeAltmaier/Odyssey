//*************************************************************************
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
// 
// Update Log: 
// $Log: /Gemini/Include/UpgradeMaster/ImageIterator.h $
// 
// 7     1/26/00 2:32p Joehler
// memcpy timeBooted and add some pads.
// 
// 6     12/09/99 4:09p Joehler
// Added Modify default image message
// 
// 5     10/14/99 5:28p Joehler
// Added key to ImageDesc
// 
// 4     10/12/99 11:14a Joehler
// Modifications for Query to assocaite IOP with Image
// 
// 3     10/11/99 8:49a Joehler
// Modified definition of GetImageName to accept UnicodeString instead of
// UnicodeString16 as a parameter.
// 
// 2     10/06/99 4:20p Joehler
// added error checking
// 
// 1     9/30/99 7:43a Joehler
// First cut of Upgrade Master
//*************************************************************************

#ifndef _ImageIterator_h_
#define _ImageIterator_h_

#include <string.h>

#include "CTTypes.h"
#include "UnicodeString.h"
#include "assert.h"
#include "Address.h"

// forward declarations
class MsgQueryImages;
class MsgGetDefaultImages;
class DdmUpgrade;
class ImageIterator;
class Image;

class ImageDesc 
{
	friend Image;

protected:
	ImageDesc(RowId key_,
		BOOL isCurrent_, 
		I64* timeBooted_,
		BOOL isPrimary_,
		BOOL isAccepted_) : key(key_), isCurrent(isCurrent_), timeBooted(*timeBooted_),
		isPrimary(isPrimary_), isAccepted(isAccepted_) { };

public:
	BOOL IsCurrent() const { return isCurrent; }

	void TimeBooted(I64* time) const { 
		memcpy(time, timeBooted, sizeof(timeBooted));
	}

	BOOL IsPrimary() const { return isPrimary; }

	BOOL IsAccepted() const { return isAccepted; }

	RowId GetKey() const { return key; }
	
private:
	RowId key;
	BOOL isCurrent;
	U32 pad;
	I64 timeBooted;
	BOOL isPrimary;
	BOOL isAccepted;
};

class Image 
{
	friend DdmUpgrade;
	friend ImageIterator;

protected:

	Image(
		RowId imageKey_,
		U32 majorVersion_, 
		U32 minorVersion_,
		U32 cbImage_,
		UnicodeString* name_, 
		I64* timeCreated_,
		I64* timeLoaded_,
		U32 type_,
		U32 iopCount_);

	Image(void* pData, ImageIterator* iterator_);

public:

	rowID GetKey() const { return ii.imageKey; }

	U32 GetMajorVersion() const { return ii.majorVersion; }

	U32 GetMinorVersion() const { return ii.minorVersion; }

	U32 GetImageSize() const { return ii.cbImage; }

	void GetTimeCreated(I64* time) const { 
		memcpy(time, ii.timeCreated, sizeof(ii.timeCreated));
	}

	void GetTimeLoaded(I64* time) const { 
		memcpy(time, ii.timeLoaded, sizeof(ii.timeLoaded));
	}

	U32 GetType() const { return ii.type; }

	U32 GetIopCount() const { return ii.iopCount; }

	BOOL IsDefault() const { return ii.isDefault; }

	void GetImageName(UnicodeString* fileName) const
	{
		UnicodeString us(ii.name);
		*fileName = us;
	}

	//const 
	ImageDesc* GetAssociation(TySlot slot) const;

protected:

	void SetDefault() { ii.isDefault = TRUE; }

private:
	// Image information
	struct 
	{
		rowID imageKey;
		U32 majorVersion;
		U32 minorVersion;
		U32 cbImage;
		U32 type;
		UnicodeString16 name;
		I64 timeCreated;
		I64 timeLoaded;
		U32 iopCount;
		BOOL isDefault;
	} ii;

	ImageIterator* iterator;
	U32 pad;

};

// utility function for Image Iterator and MsgQueryImages
// not accessible by Upgrade Master clients
class IOPDesc
{
	friend ImageIterator;
	friend Image;
	friend DdmUpgrade;

protected:

	IOPDesc(RowId key,
		TySlot slot,
		RowId primaryImage,
		RowId currentImage,
		I64* timeBooted,
		RowId imageOne,
		RowId imageTwo,
		BOOL imageOneAccepted,
		BOOL imageTwoAccepted);

	IOPDesc(void* pData);

	RowId Key() { return iopi.key; }

	TySlot Slot() { return iopi.slot; }

	RowId PrimaryImage() { return iopi.primaryImage; }

	RowId CurrentImage() { return iopi.currentImage; }

	void TimeBooted(I64* time) { 
		memcpy(time, &iopi.timeBooted, sizeof(iopi.timeBooted));
	}

	RowId ImageOne() { return iopi.imageOne; }

	RowId ImageTwo() { return iopi.imageTwo; }

	BOOL Accepted(rowID image) 
	{ 
		if (image == ImageOne())
			return iopi.imageOneAccepted;
		else 
		{
			assert(image == ImageTwo());
			return iopi.imageTwoAccepted;
		}
	}

private:

	struct {
		rowID key;
		TySlot slot;
		U32	pad;
		rowID primaryImage;
		rowID currentImage;
		I64 timeBooted;
		rowID imageOne;
		rowID imageTwo;
		BOOL imageOneAccepted;
		BOOL imageTwoAccepted;
	} iopi;

};

class ImageIterator
{
	friend MsgQueryImages;
	friend MsgGetDefaultImages;
	friend DdmUpgrade;
	friend Image;

protected:

	ImageIterator();

	// construct an ImageIterator struct from binary blob.
	ImageIterator(void *pData);	

public:

	~ImageIterator();

	U32 GetNumberOfImages() { return iii.numberOfImages; }

	//const 
	Image* GetFirst() 
	{
		placeHolder = 0;
		return GetImage(placeHolder);
	}

	//const 
	Image* GetNext() 
	{
		return GetImage(++placeHolder);
	}

protected:

	void AddImage(Image *Image_)
	{
		Append(Image_);
	}

	void AddIOPDesc(IOPDesc *IOPDesc_)
	{
		Append(IOPDesc_);
	}

	// pack up the data into a blob to be sent as a message
	U32 GetImageInfoData(void **pData_) const;

	IOPDesc* GetIOPDesc(TySlot slot) const;
	
private:

	void Set(void *pData);

	//const 
	Image *GetImage(U16 i) const
	{ 
		if (i < iii.numberOfImages)
			return pImages[i]; 
		else 
			return NULL;
	}
	
private:
	// image iterator information
	struct 
	{
		U32 numberOfImages;
		U32 numberOfIOPDescs;
	} iii;

	// don't allow copy ctor or equals operator.  Probably add later.
	ImageIterator(const ImageIterator &);
	ImageIterator &operator=(const ImageIterator &);

	U32 placeHolder;
	Image** pImages;  // array of pointers to the images	

	IOPDesc** pIOPDescs;

	// append an Image to the ImageIterator 
	void Append(Image *pImage);	

	// append an IOPDesc to the ImageIterator
	void Append(IOPDesc* pIOPDesc);

};


#endif	// _ImageIterator_h_
