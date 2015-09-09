/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FfBadSpotIterator.h
// 
// Description:
// This file defines the FF_Bad_Spot_Entry for the Flash File 
// There is one FF_Bad_Spot_Entry for each bad block in the system.
// 
// 5/11/99 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FfBadSpotIterator_H)
#define FfBadSpotIterator_H

#include "FfBadSpotEntry.h"

/*************************************************************************/
// FF_Bad_Spot_Iterator class
// used to iterate through the entries in the bad block table.
/*************************************************************************/
class FF_Bad_Spot_Iterator
{

public: // methods

	FF_Bad_Spot_Iterator();

    FF_Bad_Spot_Iterator(FF_Bad_Spot_Entry *p_FF_Bad_Spot_Entry);
    ~FF_Bad_Spot_Iterator();

    // operator ++
    FF_Bad_Spot_Iterator& operator++(int);

    // operator ==
    int operator==(const FF_Bad_Spot_Iterator& rightSide);

    friend int operator==(const FF_Bad_Spot_Iterator& leftSide,
                    const FF_Bad_Spot_Iterator& rightSide);

    // operator !=
    int operator!=(const FF_Bad_Spot_Iterator& rightSide);

    friend int operator!=(const FF_Bad_Spot_Iterator& leftSide,
                    const FF_Bad_Spot_Iterator& rightSide);

    // operator* returns reference to FF_Bad_Spot_Entry
    FF_Bad_Spot_Entry& operator*();

private: // member data

	FF_Bad_Spot_Entry		*m_iterator;

}; // FF_Bad_Spot_Iterator

/*************************************************************************/
// FF_Bad_Spot_Iterator default constructor
/*************************************************************************/
inline FF_Bad_Spot_Iterator::FF_Bad_Spot_Iterator() :
m_iterator(0)
{}

/*************************************************************************/
// FF_Bad_Spot_Iterator constructor takes pointer to a FF_Bad_Spot_Entry.
/*************************************************************************/
inline FF_Bad_Spot_Iterator::FF_Bad_Spot_Iterator(FF_Bad_Spot_Entry *p_FF_Bad_Spot_Entry) :
m_iterator(p_FF_Bad_Spot_Entry)
{}

inline FF_Bad_Spot_Iterator::~FF_Bad_Spot_Iterator() {}

/*************************************************************************/
// FF_Bad_Spot_Iterator operator ++
/*************************************************************************/
inline FF_Bad_Spot_Iterator& FF_Bad_Spot_Iterator::operator++(int)
{
    m_iterator++;
    return *this;
}

/*************************************************************************/
// FF_Bad_Spot_Iterator operator ==
/*************************************************************************/
inline int FF_Bad_Spot_Iterator::operator==(const FF_Bad_Spot_Iterator&
    rightSide)
{
    return m_iterator == rightSide.m_iterator;
}

inline int operator==(const FF_Bad_Spot_Iterator& leftSide,
                const FF_Bad_Spot_Iterator& rightSide)
{
    return leftSide.m_iterator == rightSide.m_iterator;
}

/*************************************************************************/
// FF_Bad_Spot_Iterator operator !=
/*************************************************************************/
inline int FF_Bad_Spot_Iterator::operator!=(const FF_Bad_Spot_Iterator&
    rightSide)
{
    return m_iterator != rightSide.m_iterator;
}

inline int operator!=(const FF_Bad_Spot_Iterator& leftSide,
                const FF_Bad_Spot_Iterator& rightSide)
{
    return leftSide.m_iterator != rightSide.m_iterator;
}

/*************************************************************************/
// FF_Bad_Spot_Iterator operator *
/*************************************************************************/
inline FF_Bad_Spot_Entry& FF_Bad_Spot_Iterator::operator* ()
{
    return (*m_iterator);
}



#endif // FfBadSpotIterator_H

