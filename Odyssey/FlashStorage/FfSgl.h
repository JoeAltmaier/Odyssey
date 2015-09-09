/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FfSgl.h
// 
// Description:
// 
// 1/3/00 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FF_SGL_H)
#define FF_SGL_H

#include "Message.h"
#include "Dma.h"

/*************************************************************************/
// FF_SGL
/*************************************************************************/
class FF_SGL
{
public: // methods

	// constructors
	FF_SGL();
	FF_SGL(void *pb, U32 cb);
	FF_SGL(SGE_SIMPLE_ELEMENT	*p_element);

	// Initialize from pb/cb
	void Initialize(void *pb, U32 cb);

	// Set offset
	void Offset(U32 offset);

	// Get address from specified element.
	void *Address(U32 element_index);

	// Get count of bytes from specified element.
	U32 Count(U32 element_index);
	
	// Is this the last element?
	int Is_Last(U32 element_index);

	typedef enum {
		Transfer_Address_Is_Source,
		Transfer_Address_Is_Dest
	} Transfer_Direction;

	// Build a TyDma
	Status Build_TyDma(void *transfer_address, U32 transfer_byte_count, 
						Transfer_Direction transfer_direction,
						DmaCallback p_callback, void *p_context, U32 flags, 
						TyDma **pp_TyDma);

    // operator =
    FF_SGL& operator=(const FF_SGL& right_side);

private: // member data

	SGE_SIMPLE_ELEMENT	 m_element;
	SGE_SIMPLE_ELEMENT	*m_p_element;
	U32					 m_offset;

}; // class FF_SGL

/*************************************************************************/
// Offset
/*************************************************************************/
inline void FF_SGL::Offset(U32 offset)
{
	m_offset = offset;
}

/*************************************************************************/
// Get address from specified element.
/*************************************************************************/
inline void *FF_SGL::Address(U32 element_index)
{
	return (void *)m_p_element[element_index].address;
}

/*************************************************************************/
// Get count of bytes from specified element.
/*************************************************************************/
inline U32 FF_SGL::Count(U32 element_index)
{
	return m_p_element[element_index].count;
}

/*************************************************************************/
// Is this the last element.
/*************************************************************************/
inline int FF_SGL::Is_Last(U32 element_index)
{
	return (m_p_element[element_index].flags & SGL_FLAGS_LAST_ELEMENT);
}

/*************************************************************************/
// Initialize from simple pointer and count.
/*************************************************************************/
inline void FF_SGL::Initialize(void * pb, U32 cb)
{
	m_p_element = &m_element;
	m_element.count = cb;
	m_element.address = (U32)pb;
	m_element.flags = SGL_FLAGS_LAST_ELEMENT | SGL_FLAGS_LOCAL_ADDRESS;
	m_offset = 0;
}

#endif // FF_SGL_H
