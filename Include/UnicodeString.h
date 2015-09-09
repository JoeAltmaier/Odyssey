//************************************************************************
// FILE:		UnicodeString.h
//
// PURPOSE:		Defines a unicode string object. Provided to wrap all
//				operations on wide character strings.
//
//************************************************************************

#ifndef __UNICODE_STRING_H__
#define	__UNICODE_STRING_H__

#include "CtTypes.h"
#include "StringClass.h"

class UnicodeString {


public:
	typedef	struct{
		U8		byte[2];
	}UnicodeChar;



private:

	UnicodeChar		*pData;
	U32				size;		// in bytes, excluding the termination (0x0000)


public:



//************************************************************************
// UnicodeString:
//
// PURPOSE:		Default constructor
//
// RECEIVE:		ascii:	a possible ascii string. will be converted to 
//						Unicode
//************************************************************************

UnicodeString( StringClass ascii = "" );


//************************************************************************
// UnicodeString:
//
// PURPOSE:		A constructor. Used to create a string off an integer
//
// RECEIVE:		u32:	a number to convert to string
//************************************************************************

UnicodeString( U32 u32 );


//************************************************************************
// UnicodeString:
//
// PURPOSE:		A copy constructor
//************************************************************************

UnicodeString( const UnicodeString &obj );


//************************************************************************
// UnicodeString:
//
// PURPOSE:		A constructor. Reads a Unicode string from a buffer.
//				The buffer should contain data that complies to the
//				Unicode string format. In particular, it has to be 
//				terminated with a proper Unicode EOS
//************************************************************************

UnicodeString( const void *pBuffer);


//************************************************************************
// UnicodeString:
//
// PURPOSE:		A constructor. Creates an object and inits it with the
//				character supplied
//************************************************************************

UnicodeString( UnicodeChar	unicodeChar );


//************************************************************************
// Logical operators overloaded
//************************************************************************

bool operator ==( const UnicodeString &obj ); 
bool operator <( const UnicodeString& obj );
bool operator <=( const UnicodeString& obj );
bool operator >( const UnicodeString& obj );
bool operator >=( const UnicodeString& obj );
bool operator !=( const UnicodeString& obj );


//************************************************************************
// operator=
// 
// PURPOSE:		overloads operator =
//************************************************************************

const UnicodeString& operator=( const UnicodeString &obj );


//************************************************************************
// operator+:
//
// PURPOSE:		Concatenates two strings
//
// RETURN:		The concatenated string
//************************************************************************

UnicodeString operator+( const UnicodeString &obj );


//************************************************************************
// operator[]
//
// PURPOSE:		Extractor
//
// RECEIVE:		index:		index of the unicode character to extract
//
// RETURN:		the character
//************************************************************************

UnicodeChar operator[]( U32 index );


//************************************************************************
// Substring:
//
// PURPOSE:		Returns a substring
//
// RECEIVE:		start:		index of the character to start at
//				end:		index of the character to end at
//************************************************************************

UnicodeString Substring( U32 start, U32 end );


//************************************************************************
// UnicodeString:
//
// PURPOSE:		The destructor
//************************************************************************

~UnicodeString();


//************************************************************************
// CString:
//
// PURPOSE:		Copies the contents of the string into a buffer.
//
// RECEIVE:		pBuff:		ptr to the buffer
//				bufferSize:	size of the buffer pointed
//
// RETURN:		true:		success
//************************************************************************

bool CString( void *pBuff, U32 bufferSize );


//************************************************************************
// GetLength:
//
// PURPOSE:		An accessor
//
// RETURN:		length of the string in unicode chars exluding the EOS 
//				(which is two bytes long )
//************************************************************************


U32	GetLength() const { return size/2; }



//************************************************************************
// GetAsciiString:
//
// PURPOSE:		Attempts to converts a Unicode string into an Ascii string
//
// OUTPUT:		On success, 'ascii' will contain the ascii string
//
// RETURN:		true:		success
//************************************************************************

bool GetAsciiString( StringClass &ascii );


//************************************************************************
// GetSize:
//
// PURPOSE:		An accessor
//
// RETURN:		Length of the string in bytes it takes in memory
//				
//************************************************************************

U32 GetSize() const { return size + 2; }


};

#endif	// __UNICODE_STRING_H__