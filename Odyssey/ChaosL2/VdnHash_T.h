/* VdnHash_T.h -- VDN Hashtable Template Definition
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99
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
 *		Simplistic hashtable that allows indexing by VDN.  This could be
 *		made more generic, but I want it tied to the VDN type so only this
 *		module needs to be changed if the VDN type someday becomes more complex
 *
 * Important: MyItem may need to overload the '=' operator.
 *
**/
 
// Revision History:
//  10/05/99 Bob Butler: Created
//

#ifndef __VdnHash_T_h
#define __VdnHash_T_h

#include "Array_t.h"

template <class T> class VdnHash_T {
// hash is the lower 10 bits of the VDN, which means we have 1024 buckets
// to stick the hashed VDNs into.
	enum { cBuckets = 1024, hashMask = 0x3FF };  
	
	class VhMap {
	public:
		VhMap() : key(VDNNULL) {}
		VhMap(const VhMap &v) : key(v.key), value(v.value) {}
		VhMap(VDN key_, T value_) : key(key_), value(value_) {}
		VhMap &operator=(const VhMap &map_) {
			if (&map_ != this) {
				key = map_.key;
				value = map_.value;
			}
			return *this;
		}
	private:
		friend VdnHash_T<T>;

		VDN key;
		T	value;
		};
		
private:
	Array_T< Array_T<VhMap> > atElements;
	
public:
	VdnHash_T() { atElements.Size(cBuckets); }
	VdnHash_T(const VdnHash_T &v) : atElements(v.atElements) { }
	~VdnHash_T()	{ }

	void Clear()	{ atElements.Clear(); }
	
	void Set(VDN key_,const T& value_);
	
	// returns NULL if not found
	T *Get(VDN key_);
	
	void Remove(VDN key_);
	
	void CopyFrom(const VdnHash_T<T>&);
	void CopyTo(Array_T<VDN> &at);	
	void CopyTo(Array_T<T> &at);	

	const VdnHash_T<T>& operator=(const VdnHash_T<T>& t)	{ CopyFrom(t); return *this;  }	// =  Assignment Operator
	void Dump();
};


// CopyFrom VdnHash_T
template <class T> void VdnHash_T<T>::CopyFrom(const VdnHash_T<T> &fromT) 
{
	if (&fromT != this) {	// Not us!
		atElements = fromT.atElements;
	}
}

// CopyTo Array_T<VDN>
template <class T> void VdnHash_T<T>::CopyTo(Array_T<VDN> &atVdn) 
{
	atVdn.Clear();
	for (U32 ii=0; ii < cBuckets; ii++)
		for (int jj=0; jj < atElements[ii].NextIndex(); ++jj)
			atVdn.Append(atElements[ii][jj].key);
}
#ifndef WIN32
// CopyTo Array_T<T>
template <class T> void VdnHash_T<T>::CopyTo(Array_T<T> &atT) 
{
	atT.Clear();
	for (U32 ii=0; ii < cBuckets; ii++)
		for (int jj=0; jj < atElements[ii].NextIndex(); ++jj)
			atT.Append(atElements[ii][jj].value);
}
#endif
	
// Set Element.  
template <class T> void VdnHash_T<T>::Set(VDN key_, const T& value_) 
{
	U32 buckNum = (U32)key_ & hashMask;
	Array_T<VhMap> &atBucket = atElements[buckNum];
	
	for (U32 ii = 0; ii < atBucket.NextIndex(); ++ii)
	{
		if (atBucket[ii].key == key_)
		{
			atBucket[ii].value = value_;  // replace an existing value
			return;
		}
	}
	VhMap map(key_, value_);
	atBucket.Append(map);
}		

// Get element
template <class T> T *VdnHash_T<T>::Get(VDN key_) 
{
	U32 buckNum = (U32)key_ & hashMask;
	Array_T<VhMap> &atBucket = atElements[buckNum];
	
	for (U32 ii = 0; ii < atBucket.NextIndex(); ++ii)
		if (atBucket[ii].key == key_)
			return &atBucket[ii].value;
	return NULL;
}

// Get element
template <class T> void VdnHash_T<T>::Remove(VDN key_) 
{
	U32 buckNum = (U32)key_ & hashMask;
	Array_T<VhMap> &atBucket = atElements[buckNum];
	
	for (U32 ii = 0; ii < atBucket.NextIndex(); ++ii)
		if (atBucket[ii].key == key_)
		{
			atBucket[ii] = atBucket[atBucket.NextIndex() - 1];
			atBucket.RemoveLast();
			return;
		}
	return;
}
#ifndef WIN32

// Dump Elements
template <class T> void Array_T<T>::Dump() 
{
	for (U32 ii=0; ii<Size(); ii++) {
		Tracef("<%u>",ii);
		pElements[ii].Dump();
	}
	Tracef("\n");
}
#endif

#endif 	// __Array_T_h
