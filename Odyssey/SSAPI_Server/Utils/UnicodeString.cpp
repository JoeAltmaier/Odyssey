//************************************************************************
// FILE:		UnicodeString.cpp
//
// PURPOSE:		Implements a unicode string object. Provided to wrap all
//				operations on wide character strings.
//
//************************************************************************

#include "UnicodeString.h"
#include "byte_array_utils.h"

#ifdef WIN32
	#define PRIMARY_BYTE 1
	#define SECONDARY_BYTE 0
#else
	#define PRIMARY_BYTE 1
	#define SECONDARY_BYTE 0
#endif


//************************************************************************
// UnicodeString:
//
// PURPOSE:		Default constructor
//
// RECEIVE:		ascii:	a possible ascii string. will be converted to 
//						Unicode
//************************************************************************

UnicodeString::UnicodeString( StringClass ascii ){

	U32				asciiLength = ascii.SLength();
	UnicodeChar		*pChar;

	if( asciiLength ){
		pData	= new UnicodeChar[asciiLength * 2 + sizeof( UnicodeChar )];
		size	= asciiLength * 2;
		memset( pData, 0, asciiLength * 2 + sizeof( UnicodeChar ) );
		pChar	= (UnicodeChar *)pData;

		for( unsigned i = 0; i < ascii.SLength(); i++, pChar++ ){
			pChar->byte[PRIMARY_BYTE] = ascii[i];
		}
	}
	else{
		pData	= NULL;
		size	= 0;
	}
}


//************************************************************************
// UnicodeString:
//
// PURPOSE:		A constructor. Used to create a string off an integer
//
// RECEIVE:		u32:	a number to convert to string
//************************************************************************

UnicodeString::UnicodeString( U32 u32 ){

	UnicodeString( StringClass( u32 ) );
}


//************************************************************************
// UnicodeString:
//
// PURPOSE:		A constructor. Reads a Unicode string from a buffer.
//				The buffer should contain data that complies to the
//				Unicode string format. In particular, it has to be 
//				terminated with a proper Unicode EOS
//************************************************************************

UnicodeString::UnicodeString( const void *pBuffer ){

	const char* pBuf = (const char*)pBuffer;

	size = 0;
	pData = NULL;

	if( pBuffer ){
		for( size=1; true; size += 2 )
			if((pBuf[size]==0)&&(pBuf[size-1]==0))
				break;

		size--;
		pData = (UnicodeChar *)new char[ size ];
		memcpy( pData, pBuffer, size );
	}
	else{
		size = sizeof(UnicodeChar);
		pData = (UnicodeChar *)new char[ size ];
		memset( pData, 0, size );
	}
}


//************************************************************************
// UnicodeString:
//
// PURPOSE:		A copy constructor
//************************************************************************

UnicodeString::UnicodeString( const UnicodeString &obj ){

	pData	= NULL;
	size	= 0;

	*this = obj;
}


//************************************************************************
// UnicodeString:
//
// PURPOSE:		A constructor. Creates an object and inits it with the
//				character supplied
//************************************************************************

UnicodeString::UnicodeString( UnicodeChar	unicodeChar ){

	pData	=	new UnicodeChar[ 2 ];

	memset( pData, 0, sizeof(UnicodeChar) * 2 );
	memcpy( pData, &unicodeChar, sizeof( UnicodeChar ) );

	size	=	sizeof(UnicodeChar);
}


//************************************************************************
// Logical operators overloaded
//************************************************************************

bool 
UnicodeString::operator ==( const UnicodeString &obj ){

	if( size != obj.size )
		return false;
	
	if( !memcmp( pData, obj.pData, GetSize() - sizeof(UnicodeChar)) )
		return true;
	else
		return false;

}


bool 
UnicodeString::operator !=( const UnicodeString& obj ){

	return !(*this == obj);
}


bool 
UnicodeString::operator <( const UnicodeString& obj ){

	U16		wordI, wordJ, i, j;
	
	for( i = j = 0; (i < size) && (j < obj.size); i++, j++ ){
		memcpy( &wordI, (UnicodeChar *)&pData + i, 2 );
		memcpy( &wordJ, (UnicodeChar *)&pData + j, 2 );

		if( wordI >= wordJ )
			return false;
	}

	if( ( i == size ) && ( j == obj.size ) )
		return false;

	if( i != size )			// means that j == obj.size, thus right string is shorter 
		return false;

	return true;
}


bool 
UnicodeString::operator <=( const UnicodeString& obj ){

	return ( (*this == obj) | (*this < obj ) )? true : false;
}


bool
UnicodeString::operator >( const UnicodeString& obj ){

	return (*this <= obj )? false : true;
}


bool 
UnicodeString::operator >=( const UnicodeString& obj ){

	return ( (*this == obj) | (*this > obj ) )? true : false;
}


//************************************************************************
// operator=
// 
// PURPOSE:		overloads operator =
//************************************************************************

const UnicodeString& 
UnicodeString::operator=( const UnicodeString &obj ){

	this->~UnicodeString();

	pData	= new UnicodeChar[ obj.GetLength() + 1 ];
	memset( pData, 0, obj.GetSize() );
	memcpy( pData, obj.pData, obj.GetSize() - sizeof(UnicodeChar));

	size	= obj.size;

	return obj;
}


//************************************************************************
// operator[]
//
// PURPOSE:		Extractor
//
// RECEIVE:		index:		index of the unicode character to extract
//
// RETURN:		the character
//************************************************************************

UnicodeString::UnicodeChar 
UnicodeString::operator[]( U32 index ){

	UnicodeChar	a;

	memcpy( &a, pData + index, sizeof( a ) );

	return a;
}


//************************************************************************
// Substring:
//
// PURPOSE:		Returns a substring
//
// RECEIVE:		start:		index of the character to start at
//				end:		index of the character to end at
//************************************************************************

UnicodeString 
UnicodeString::Substring( U32 start, U32 end ){

	UnicodeString			tempString;
	

	if( start < end )
		for( unsigned i = start; i < end; i++ )
			tempString = tempString + (*this)[ i ];

	return tempString;
}


//************************************************************************
// UnicodeString:
//
// PURPOSE:		The destructor
//************************************************************************

UnicodeString::~UnicodeString(){

	if( pData ){
		delete [] pData;
		size = 0;
	}
}


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

bool 
UnicodeString::CString( void *pBuff, U32 bufferSize ){

	char* pBuf = (char*)pBuff;

	if( size + 2 > bufferSize )
		return false;
	
	memset( pBuff, 0, size + 2 );
	memcpy( pBuff, pData, size );

	return true;
}


//************************************************************************
// operator+:
//
// PURPOSE:		Concatenates two strings
//
// RETURN:		The concatenated string
//************************************************************************

UnicodeString 
UnicodeString::operator+( const UnicodeString &obj ){
	
	UnicodeString		savedString;


	savedString.size = size + obj.size;
	savedString.pData = new UnicodeChar[ size/2 + obj.size/2 + 2];

	memset( savedString.pData, 0 , size/2 + obj.size/2 + 2 );
	memcpy( savedString.pData, pData, GetSize() - 2 );
	memcpy( savedString.pData + GetLength(), obj.pData, obj.GetSize() - 2);

	return savedString;		
}


//************************************************************************
// GetAsciiString:
//
// PURPOSE:		Attempts to converts a Unicode string into an Ascii string
//
// OUTPUT:		On success, 'ascii' will contain the ascii string
//
// RETURN:		true:		success
//************************************************************************

bool 
UnicodeString::GetAsciiString( StringClass &ascii ){

	UnicodeChar		*pUChar;
	U32				index;
	
	ascii = "";
	
	for( index = 0, pUChar = pData; index < GetLength(); index++, pUChar++ )
		ascii = ascii + StringClass( 1, pUChar->byte[PRIMARY_BYTE] );

	return true;
}
