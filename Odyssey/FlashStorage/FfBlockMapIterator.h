/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FfBlockMapIterator.h
// 
// Description:
// This file defines the FF_Block_Map_Entry for the Flash File 
// There is one FF_Block_Map_Entry for each bad block in the system.
// 
// 5/11/99 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FfBlockMapIterator_H)
#define FfBlockMapIterator_H

#include "FfBlockMapEntry.h"

/*************************************************************************/
// FF_Block_Map_Iterator class
// used to iterate through the entries in the bad block table.
/*************************************************************************/
class FF_Block_Map_Iterator
{

public: // methods

	FF_Block_Map_Iterator();

    FF_Block_Map_Iterator(FF_Block_Map_Entry *p_FF_Block_Map_Entry);
    ~FF_Block_Map_Iterator();

    // operator ++
    FF_Block_Map_Iterator& operator++(int);

    // operator ==
    int operator==(const FF_Block_Map_Iterator& rightSide);

    friend int operator==(const FF_Block_Map_Iterator& leftSide,
                    const FF_Block_Map_Iterator& rightSide);

    // operator !=
    int operator!=(const FF_Block_Map_Iterator& rightSide);

    friend int operator!=(const FF_Block_Map_Iterator& leftSide,
                    const FF_Block_Map_Iterator& rightSide);

    // operator* returns reference to FF_Block_Map_Entry
    FF_Block_Map_Entry& operator*();

private: // member data

	FF_Block_Map_Entry		*m_iterator;

}; // FF_Block_Map_Iterator

/*************************************************************************/
// FF_Block_Map_Iterator default constructor
/*************************************************************************/
inline FF_Block_Map_Iterator::FF_Block_Map_Iterator() :
m_iterator(0)
{}

/*************************************************************************/
// FF_Block_Map_Iterator constructor takes pointer to a FF_Block_Map_Entry.
/*************************************************************************/
inline FF_Block_Map_Iterator::FF_Block_Map_Iterator(FF_Block_Map_Entry *p_FF_Block_Map_Entry) :
m_iterator(p_FF_Block_Map_Entry)
{}

inline FF_Block_Map_Iterator::~FF_Block_Map_Iterator() {}

/*************************************************************************/
// FF_Block_Map_Iterator operator ++
/*************************************************************************/
inline FF_Block_Map_Iterator& FF_Block_Map_Iterator::operator++(int)
{
    m_iterator++;
    return *this;
}

/*************************************************************************/
// FF_Block_Map_Iterator operator ==
/*************************************************************************/
inline int FF_Block_Map_Iterator::operator==(const FF_Block_Map_Iterator&
    rightSide)
{
    return m_iterator == rightSide.m_iterator;
}

inline int operator==(const FF_Block_Map_Iterator& leftSide,
                const FF_Block_Map_Iterator& rightSide)
{
    return leftSide.m_iterator == rightSide.m_iterator;
}

/*************************************************************************/
// FF_Block_Map_Iterator operator !=
/*************************************************************************/
inline int FF_Block_Map_Iterator::operator!=(const FF_Block_Map_Iterator&
    rightSide)
{
    return m_iterator != rightSide.m_iterator;
}

inline int operator!=(const FF_Block_Map_Iterator& leftSide,
                const FF_Block_Map_Iterator& rightSide)
{
    return leftSide.m_iterator != rightSide.m_iterator;
}

/*************************************************************************/
// FF_Block_Map_Iterator operator *
/*************************************************************************/
inline FF_Block_Map_Entry& FF_Block_Map_Iterator::operator* ()
{
    return (*m_iterator);
}



#endif // FfBlockMapIterator_H

