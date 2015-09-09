//************************************************************************
// FILE:			StringClass.cpp
//
// PURPOSE:			The file contains function definitions
//					for class StringClass
//************************************************************************

#ifdef WIN32
#include <stdio.h>
#endif // WIN32

#include <stdlib.h>

#include "StringClass.h"

StringClass::StringClass(const char *inString){

	if ( !inString){
   		length    =  0;
		pString = NULL;
		return;
	}

	length     = strlen(inString);

	if ( !( pString  = new char[ length +1 ] ) ){
		length    =  0;
		pString = NULL;
		return;
	}

	strcpy(pString, (char *)inString);
}  // ENDOF StringClass(const char*)




char* GetCharByDigit( int i ){
	switch( i ){
		case 0:	return "0";
		case 1:	return "1";
		case 2:	return "2";
		case 3:	return "3";
		case 4:	return "4";
		case 5:	return "5";
		case 6:	return "6";
		case 7:	return "7";
		case 8:	return "8";
		case 9:	return "9";
	}

	return "-";
}

void itoa( unsigned i, StringClass& s ){
	unsigned index = 1;

	s.~StringClass();

	do {
		s = StringClass( GetCharByDigit( i % 10 ) ) + s;
		i -= i % 10;
		i /= 10;
	}while( i );
}


//************************************************************************
// StringClass: constructor
//
// Input:       int value     - value to init to
//
// Locals:      char buff[4]  - temp buff to convert
//                              int to char
//
// Calls:       strlen
//              strcpy
//************************************************************************

StringClass::StringClass( int value ){

  pString = NULL;
  length  = 0;
  itoa( (unsigned int) value, *this); 	

}

/**************************************************************************
StringClass:   A copy constractor for the class

Pre:           An instance has been declared. To be
               initialized with the value of another
               already existing instance of the class.

Receive:       String Class inString - A ref to the
               instance to get data from.

Post:          The instance is intialized to be of the
					same value as the other instance.

Usage:         StringClass		s(anotherstring);
**************************************************************************/

StringClass::StringClass( const StringClass &inString){

   length     = inString.SLength();
   if ( !length ){
      pString = NULL;
      return;
   }

   if ( !( pString  = new char[ length + 1 ] ) ){
   	length    =  0;
      pString = NULL; 
      return;
   }

   strcpy( pString, inString.pString );
} // ENDOF StringClass(const StringClass)


/**************************************************************************
~StringClass:  Destructor for the class

Pre:           An instance has been declared or inited

Post:          All allocated memory is freed.
**************************************************************************/
StringClass::~StringClass( void ){

   if ( pString )
      delete[ ]  pString;

   pString = NULL;
   length  = 0;
}


/**************************************************************************
CString:        Returns address of a newly created
                copy of the string conatined in the
                instance.

Pre:            Instance is declared / inited

Post:            A new chunk of mem is allocated and
                the string is copied into it.

Return:         char*
                    ptr to the mem allocated

Loval vars:     char *temp - temp ptr to allocated chunk

Approach:       Alloc mem
                copy string, return ptr

Calls:          strcpy()

Usage:          charPtr = mystr.CString()

Note:           Memory MUST be deallocated by the caller 
**************************************************************************/

char *
StringClass::CString( void ){

    char     *temp;

    if ( !SLength() )
        return NULL;
       
    temp = new char [ SLength() + 1 ];
 
    if ( !temp )
        return NULL;

    strcpy( temp, pString );

    return temp;

} // ENDOF CString(void)


/**************************************************************************
CString:       Copies the string at the address specified.
               Provided in order to convert to char*

Pre:           An instance is initialized

Receive:       char  *buff - buffer to copy the string to
               U32
                  buffSize - size of the buff

Return:        BOOL
                  TRUE  - successfuly copied
                  FALSE - could not copy (out of mem)

Calls:         strcpy()

Usage:         flag = mystring.CString(charBuff, sizeof(charBuff) )

**************************************************************************/
BOOL
StringClass::CString( char* buff, U32 buffSize ) const {

	if ( ( buffSize < length + 1) || ( !buff ) )
      return FALSE;

   if ( !length )
      return FALSE;

   strcpy(buff,pString);

   return ( TRUE );

} // ENDOF CString




/**************************************************************************
=:             Assignment operator for the class
               (string and string)

Pre:           An instance is declared, may be intialized

Receive:       StringClass  inString - string to assign to

Return:        StringClass

Approach:      Delete previusly allocated memory. Allocate
               another chunk and copy the contents of the other
               string into it.

Calls:         strcpy()

Usage:         string1 = string2
**************************************************************************/
StringClass
StringClass::operator = ( const StringClass &inString ){

   this->~StringClass();

   length = inString.SLength();
   if ( !length )
       return inString;

   pString = new char [ length + 1 ];

   if ( !pString){
       length    =  0;
       pString = NULL;
       return inString;
   }

   strcpy( pString, inString.pString );

   return inString;

} // ENDOF operator =


/**************************************************************************
char []:       Operator to access (read-only) particular
					character in the string.

Pre:           An instance is declared and inited

Receive:			U32    index  -  index of the caracter

Return:          The (reference to)caracter with
               index = 'index'

Usage:         myChar = myString[some_meaningful_index]
**************************************************************************/
char
StringClass::operator [] (U32 index){

    if ( ( !length ) || ( index > length ) )
       return '?';

    return *(pString + index);
}


/**************************************************************************
==, !=, <, >,  <=, >=:        Relation operators for
              StringClass and StringClass

Pre:          Both strings have been declared AND inited!

Receive:      StringClass  &inString - ref to string to
              compare against

PostReturn:   BOOL
                 TRUE
                 FALSE

Usage:        s1 == s2
              s1 >  s2
              s1 <  s2
              s1 != s2
              s1 <= s2
              s1 >= s2

Calls:        strcmp()

Note:         If either string has a NULL, 0 is returned
**************************************************************************/
BOOL
StringClass::operator == ( const StringClass &inString){

	 if ( !pString  || !inString.pString)
        return FALSE;

    if ( !strcmp( pString, inString.pString ) )
        return TRUE;

    return FALSE;
}


BOOL
StringClass::operator != ( const StringClass &inString){

   if ( !pString  || !inString.pString)
        return FALSE;

   return ( !( *this == inString ) );
}


BOOL
StringClass::operator < ( const StringClass &inString){

   if ( !pString  || !inString.pString)
        return FALSE;

	return ( strcmp( pString, inString.pString ) <= 0) ?
              TRUE : FALSE;
}


BOOL
StringClass::operator > ( const StringClass &inString){

   if ( !pString  || !inString.pString)
        return FALSE;

	return ( strcmp( pString, inString.pString ) >= 0) ?
              TRUE : FALSE;
}

BOOL
StringClass::operator >= ( const StringClass &inString){

   if ( !pString  || !inString.pString)
        return FALSE;

	return ( *this == inString ) || ( *this > inString );
}

BOOL
StringClass::operator <= ( const StringClass &inString){

   if ( !pString  || !inString.pString)
        return FALSE;

   return ( *this == inString ) || ( *this < inString );
}

/**************************************************************************
SLength:      The function returns the length of the
              string.

Note:         Just like withANSI C library, this function
              returns length of the string exluding '\0'

Pre:          An instance has been declared

Return:         Size of memory used to allocate the string
**************************************************************************/

U32
StringClass::SLength( void ) const {
   return length;
}


/**************************************************************************
==,    Relation Operators for StringClass and char*

Pre:         And instance is declared AND inited!

Receive:     char  *pChar - ptr to the char string

Return:      BOOL
                TRUE
                FALSE

Note:        If either string has a NULL, 0 is returned

Approach:    All functions utilize corresponding
 				 relation operators defined for StrngClass
             and StringClass as well the class's
             constractor StringClass(char*)

**************************************************************************/

BOOL
StringClass::operator == ( char *pChar ){
	return StringClass(pChar) == *this;
}



/**************************************************************************
+:           Concatenation operator for StringClass and
             StringClass

Pre:         The strings have been declared/buit

Receive:     StringClass   &inString  - the string to conc.
             with this instance's string.

Return:      StringClass
                 concatenated string

Approach:    Create new instance, unit it with concatenated
             string

Local vars   StringClass newString -  concatenated string to
                                      be returned
Calls:       strcpy()
             strcat()

Usage:       s1 = s2 + s3
**************************************************************************/

StringClass
StringClass::operator + ( const StringClass &inString ){

   StringClass newString;

   newString.pString = new char[ SLength() + inString.SLength() + 1 ];

   if ( !newString.pString ){
      newString.length = 0;
   }
   else{
	  newString.length = SLength() + inString.SLength();
	  memset( (void *)newString.pString, 0, newString.length );

	  if( pString )
		strcpy( newString.pString, pString );

      if( inString.pString )
		  strcat( newString.pString, inString.pString );
   }

   return newString;
} // ENDOF operator +



/***************************************************************************
():            Get a substring of a StringClass instance.

Pre:           Instance is declared and inited

Receive:       U32 first - index to start with
               U32 last  - index to finish at

Local vars:    StringClass  newString - temp instance to
                                        be returned

Return:          StringClass
                    A substring that starts at position=first  and
                    ends at position=last

Usage:        s1 = s2(1,3);

Note:         expressions as s1 = s2(1,1) is legal!

Calls:        memset()
              strncpy()
***************************************************************************/

StringClass
StringClass::operator() (U32 first, U32 last){

    StringClass     newString;

    if ( ( last < first ) || ( last > SLength() ) )
        return newString;


    newString.pString = new char[ last - first + 2 ];
    if ( !newString.pString )
        return newString;

    newString.length =  last - first + 1;
    memset( newString.pString, 0, newString.SLength() +1 );

    strncpy( newString.pString, pString + first, newString.SLength() );

    return newString;
} // ENDOF operator(U32, U32)



/**************************************************************************
SPosition:    Returns position of the first occurence
             of the substring.

Pre:         Both instances dec;ared and inited

Receive      StringClass  &inString - substring

Return:      int
                  index  if found
                  -1     if not found

Local vars:  char *p - ptr to the substring, temp

Calls:       strstr()

Usage:       pos = s1( s2 )
**************************************************************************/

int
StringClass::SPosition( const StringClass &inString){

    char     *p;

    if ( !pString || !inString.pString)
        return -1;

    p = strstr( pString, inString.pString);

    if ( p )
       return ( (int) (p - pString) );
    
    return -1;

} // ENDOF Position


/**************************************************************************
=:             Assignment operator for a class
               instance and char*

Pre:           An instance is declared, may be intialized.

Receive:       char*  inString - string to assign to

Post:          Instance has another value. If it was inited before,
               memory used is freed prior to assignment.

Return:        char* inString

Usage:         string = charString
**************************************************************************/

char*
StringClass::operator = (char *inString){

    this->~StringClass();

    if ( !inString )
        return inString;

    *this = *this + inString;

    return ( inString );
} // ENDOF operator + ( char* )


/**************************************************************************
=:             Assignment operator for a class
               instance and a number

Pre:           An instance is declared, may be intialized.

Receive:       const float&  value  || const int &value


Post:          Instance has another value. If it was inited before,
               memory used is freed prior to assignment.

Return:        float &value         || int &value

Local Vars:    char  temp[4] - used to transfer number to


Usage:         string = floatVar    || string = intVar
**************************************************************************/


int
StringClass::operator = (const int &value){

   this->~StringClass();

   itoa( (unsigned)value, *this);

   return value;
} // ENDOF operator = (int)


/**************************************************************************
SFloat
SInt:     Functions return string as a number

Pre:      Instance is declared and inited

Receive:  float &value        ||       int &value

Return:   BOOL
            TRUE  - string was successfully translated
                    into number
            FALSE - could not translate

Calls:   atoi() || atof()

Usage:   flag = str.SFloat(floatVar)  ||
         flag = str.SInt(intvAR)
**************************************************************************/


BOOL
StringClass::SInt   ( int   &var ){

	if ( !SLength() )
       return FALSE;

   if ( !( var = atoi( pString ) ) )
       return FALSE;

   return TRUE;
} //ENDOF SInt()

BOOL
StringClass::SFloat ( float &var ){

   if ( !SLength() )
       return FALSE;

   var = (float) atof( pString );

   return TRUE;
} //ENDOF SFloat()


//*********************************************************************************************
// StringClass:  constructor
//
// Purpose:      create a string of size 'size' and
//               fills it with char 'character'
//
// Input:        int size  - size of the new string
//               char character
//                         - contents of the string
//*********************************************************************************************

StringClass::StringClass( int size, char character ){

  length  = size;
  pString = new char[ length + 1 ];

  if( pString ){
     memset( pString, 0, size + 1 );	
     memset( pString, character, size );
  }
}


#ifdef WIN32

/**************************************************************************************
>>:          Input operator for the class

Pre:         An instance is declared/built.

Receive      ostream       &is       - instance of istream.
             StringClass   &inString - instance of the
                                       class to store
                                       data in.

Post:        returns ios - the ref to stream

Approach:    Clear the buffer of this instance.
             Read characters one by one as they are
             being entered concatinating them to the
             string until ENTER_KEY is reached.

Notes:       1.Each character entered is interrupted with EOS
             2.length is decremented 'cause we count '\0' when
               using operator +

Local Vars:  char ch[2]   - temp storage to put the byte entered
                            to. Second byte is a termination symbol

Calls:       getchar()

Usage:       cin >> string
****************************************************************************************/

istream&
operator >> ( istream &is, StringClass &inString ){

   char          ch[2];

   inString.~StringClass();

   do {

      ch[ 0 ] = (char) getchar();
      ch[ 1 ] = EOS;
      inString = (inString + StringClass((char*)ch));

   } while (  ch[ 0 ] != ENTER_KEY );

   inString.length--;
   inString.pString[ inString.length ] = 0;

   return is;
} //ENDOF operator >>


/**************************************************************************
<<:          Output operator for the class

Pre:         An instance is declared/built

Receive      ostream       &os       - instance of ostream.
             StringClass   &inString - string to send to
                                       the stream

Post:        returns ios - the ref to stream

Usage:       cout << string
**************************************************************************/

ostream&
operator << ( ostream &os, const StringClass &inString){
   if ( inString.SLength() )
      os << inString.pString;

   return os;
}

#endif	// WIN32