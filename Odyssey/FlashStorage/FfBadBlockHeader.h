/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FfBadBlockHeader.h
// 
// Description:
// This file defines the FF_Bad_Block_Header for the Flash Storage 
// 
// 6/2/99 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FfBadBlockHeader_H)
#define FfBadBlockHeader_H

#include "FfCommon.h"

#define FF_BAD_BLOCK_TABLE_COOKIE 0XBADBADBADBADBAD	
#define FF_BAD_BLOCK_TABLE_VERSION 3

/*************************************************************************/
// Internal bad block error codes.
/*************************************************************************/
typedef enum
{
	FF_ERROR_INVALID_BAD_BLOCK_TABLE_A		= 1,
	FF_ERROR_INVALID_BAD_BLOCK_TABLE_B		= 2,
	FF_ERROR_INVALID_BAD_BLOCK_VERSION_A	= 3,
	FF_ERROR_INVALID_BAD_BLOCK_VERSION_B	= 4
	
} FF_BAD_BLOCK_ERROR_CODE;

/*************************************************************************/
// FF_Bad_Block_Header class
// When the bad block table is written, a header is written to precede it.  
// The header is designed such that the data is duplicated 
// in card A and card B
/*************************************************************************/
class FF_Bad_Block_Header
{
public: // methods

	Status Open();
	Status Create();

private: // member data
	// The first two doublewords go to card A.
	// The next two doublewords go to card B.

	// Version
	UI64		m_version_A;

	// Test this value to make sure we have a valid object.
	UI64		m_cookie_A;

	// Version
	UI64		m_version_B;

	// Test this value to make sure we have a valid object.
	UI64		m_cookie_B;

}; // FF_Bad_Block_Header

/*************************************************************************/
// FF_Bad_Block_Header::Create 
/*************************************************************************/
inline Status FF_Bad_Block_Header::Create()
{
	m_version_A = FF_BAD_BLOCK_TABLE_VERSION;
	m_version_B = FF_BAD_BLOCK_TABLE_VERSION;
	m_cookie_A = FF_BAD_BLOCK_TABLE_COOKIE;
	m_cookie_B = FF_BAD_BLOCK_TABLE_COOKIE;
	return OK;
}

#endif // FfBadBlockTable_H

