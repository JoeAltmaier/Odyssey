/* Array_T.h -- Array Template Definition
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
 *		Create an Array_T class that is a resizable array of specified type.
 *
 * Usage:
 *		typedef Array_T<MyItem> MyArray;
 *
 * Important: MyItem may need to overload the '=' operator.
 *
**/
 
// Revision History:
//  3/12/99 Tom Nelson: Create file
//  4/06/99 Tom Nelson: Added operator = overload
//  6/04/99 Bob Butler: Added Append() and NextIndex() 
//  7/08/99 Joe Altmaier: Set, Append const ref
//

#ifndef __Array_T_h
#define __Array_T_h

#define ARRAY_T_SIZE	16

template <class T>
class Array_T {
private:
	T  *pElements;
	U32 nElements;
	U32 iNextIndex;
	
public:
	Array_T(U32 _nElements=0) : pElements(new T[_nElements]),nElements(_nElements),iNextIndex(0) {}
	Array_T(const Array_T<T> & t) : pElements(NULL), nElements(0), iNextIndex(0) { CopyFrom(t); }
	~Array_T()	{ delete [] pElements; }

	void Clear()	{ delete [] pElements; pElements = new T[nElements]; iNextIndex = 0;}
	
	U32 Factor(U32 _size) 	{ return ((_size + (ARRAY_T_SIZE-1)) / ARRAY_T_SIZE) * ARRAY_T_SIZE; }
	void Size(U32);
	void Set(U32 iElement,const T& data);
	void Append(const T& data);
	void RemoveLast();
	U32 Size() const { return nElements; }
	U32 NextIndex() const { return iNextIndex; }
	
	void CopyFrom(const Array_T<T>&);
	T& operator[](U32 iElement) { return pElements[iElement];  }	// Does not resize
	Array_T<T>& operator=(const Array_T<T>& t)	{ CopyFrom(t); return *this;  }	// =  Assignment Operator
	void Dump();
};

// CopyFrom Array_T
template <class T>
void Array_T<T>::CopyFrom(const Array_T<T> &fromT) {
	if (&fromT != this) {	// Not us!
		delete [] pElements;				// Destruct old Array.
		U32 _sNew = fromT.Size() ? Factor(fromT.Size()) : 0;  // if old array is size zero, 
															  // avoid extra allocations and make this zero too
		pElements = new T[_sNew];	// Don't check for NULL 
		nElements = _sNew;

		for (U32 ii=0; ii < Size(); ii++)
			pElements[ii] = fromT.pElements[ii];
		iNextIndex = fromT.iNextIndex;
	}
}
		
// Resize Array_T
template <class T>
void Array_T<T>::Size(U32 _sNew) {
	_sNew = Factor(_sNew);
	if (_sNew != Size()) {
		T* _pNew = new T[_sNew];	// Don't check for NULL 

		U32 nCopy = (_sNew < Size()) ? _sNew : Size();
		for (U32 ii=0; ii < nCopy; ii++)
			_pNew[ii] = pElements[ii];
				
		delete [] pElements;
		pElements = _pNew;
		nElements = _sNew;
	}
}
		
// Set Element.  Resize if needed.	
template <class T>
void Array_T<T>::Set(U32 _iElement,const T& data) {
	if (_iElement >= Size())
		Size(_iElement+1);

	pElements[_iElement] = data;
	if (iNextIndex < _iElement + 1)
		iNextIndex = _iElement + 1;
}		

// Set Next Element.  Resize if needed.	
template <class T>
void Array_T<T>::Append(const T& data) {
	Set(iNextIndex, data);
}		

// Remove Last Element.  Resize if needed.	
template <class T>
void Array_T<T>::RemoveLast() {
	T emptyT;
	Set(iNextIndex-1, emptyT);
	--iNextIndex;
}		


// Dump Elements
template <class T>
void Array_T<T>::Dump() {
	for (U32 ii=0; ii<Size(); ii++) {
		Tracef("<%u>",ii);
		pElements[ii].Dump();
	}
	Tracef("\n");
}

#endif 	// __Array_T_h
