/***************************************************************************
FILE:               StringClass.h


TYPES:				class	StringClass

CONSTANTS:		    ENTER_KEY    =   10
                    EOS          =   '\0'

VARS:               char   *pString  - ptr to string
                    int    length    - the length of the string

PUBLIC METHODS:
                    StringClass            -- constractor
                    StringClass            -- constractor
                    StringClass            -- constructor
                    StringClass            -- copy constractor
                    ~StringClass           -- destructor
                    void CString           -- for conversion
					char* CString		   -- for conversion
                    operator= (StringClass)-- assignment
                    operator= (float)      -- assignment
                    operator= (int)        -- assignment
                    operator+ (char*)      -- assignment
                    char operator []       -- get character
                    BOOL operator ==       -- equal
                    BOOL operator !=       -- not equal
                    BOOL operator <=       -- less or equal
                    BOOL operator >=       -- greater or equal
                    BOOL operator <        -- less then
                    BOOL operator >        --  greater then
                    U32 SLength()		   -- get length
                    friend operator <<     -- output
                    friend opeator  >>     -- input
                    StringClass operator + -- concatenation
                    StringClass operator(
                       U32, U32 )		   -- get a substring
                    int SPostion           -- position of a
                                              substring
                    BOOL SFloat(float&)    -- return string
                                              as a float
                    BOOL SInt(int&)        -- return string as
                                              an int

***************************************************************************/

#ifndef	 __STRING_CLASS_H__
#define  __STRING_CLASS_H__


#define  ENTER_KEY    10
#define  EOS          '\0'



#ifdef WIN32
#include <iostream.h>
#endif

#include "CTtypes.h"
#include "Simple.h"
#include <string.h>






class StringClass {
    char     *pString;
    U32		 length;


public:


/*************************************************************************
StringClass:   Constractor to initialize an instance
               of the class to some const character
               string.

Pre:           An instance has been declared. To be
               intialized with some character constant
               value.

Receive:       inString - A ptr to the const char string

Post:          The instance is intialized to the string
					passed in.

Usage:         StringClass		s("whatever string");
*************************************************************************/

StringClass(const char *inString = 0);


/*************************************************************************
StringClass:   A copy constractor for the class

Pre:           An instance has been declared. To be
               initialized with the value of another
               already existing instance of the class.

Receive:       String Class inString - A ref to the
               instance to get data from.

Post:          The instance is intialized to be of the
					same value as the other instance.

Usage:         StringClass		s(anotherstring);
*************************************************************************/
StringClass( const StringClass &inString);


/*************************************************************************
// StringClass: constructor
//
// Input:       int value  - value to init to
*************************************************************************/

StringClass( int value );


/*************************************************************************
// StringClass:  constructor
//
// Purpose:      create a string of size 'size' and
//               fills it with char 'character'
//
// Input:        int size  - size of the new string
//               char character
//                         - contents of the string
*************************************************************************/

StringClass( int size, char character );


/*************************************************************************
~StringClass:  Destructor for the class

Pre:           An instance has been declared or inited

Post:          All allocated memory is freed.
*************************************************************************/

~StringClass( void );


/*************************************************************************
CString:       Copies the string at the address specified.
               Provided in order to convert to char*

Pre:           An instance is initialized

Receive:       char  *buff - buffer to copy the string to
               U32
                  buffSize - size of the buff

Return:        BOOL
                  TRUE  - successfuly copied
                  FALSE - could not copy (out of mem)

Usage:         flag = mystring.CString(charBuff, sizeof(charBuff) )

*************************************************************************/

BOOL CString( char* buff, U32 buffSize ) const;


/*************************************************************************
CString        : Returns ptr to a newly-created copy
               of the string contained in the instance

Pre:           Instance is declared / inited

Post:          A new chunk of mem is allocated and
               the string is copied into it

Return         char *
                   ptr to mem with the copy of string

Usage:         pChar = string1.CString() ;
*************************************************************************/

char* CString( void ); 


/*************************************************************************
=:             Assignment operator for the class




Pre:           An instance is declared, may be intialized

Receive:       StringClass  &inString - string to assign to

Post:          Instance has another value. If it was inited before,
               memory used is freed prior to assignment.

Return:        StringClass&
                    ref received.

Usage:         string1 = string2
*************************************************************************/

StringClass   operator = (const StringClass &inString);


/*************************************************************************
=:             Assignment operator for a class
               instance and char*

Pre:           An instance is declared, may be intialized.

Receive:       char*  inString - string to assign to

Post:          Instance has another value. If it was inited before,
               memory used is freed prior to assignment.

Return:        char* inString

Usage:         string = charString
*************************************************************************/

char*   operator = (char *inString);


int     operator = (const int &value);


/*************************************************************************
char []:       Operator to access (read-only) particular
					character in the string.

Pre:           An instance is declared and inited

Receive:			U32    index  -  index of the caracter

Return:       char
                  The caracter with index = 'index'

Usage:         myChar = myString[some_meaningful_index]
*************************************************************************/

char operator [] (U32 index);


/*************************************************************************
==, !=, <, >,  <=, >=:        Relation operators for
              StringClass and StringClass

Pre:          Both strings have been declared AND inited!

Receive:      StringClass  &inString - ref to string to
              compare against

Return:       BOOL
                  TRUE
                  FALSE

Usage:        s1 == s2
              s1 >  s2
              s1 <  s2
              s1 != s2
              s1 <= s2
              s1 >= s2
*************************************************************************/

BOOL operator == ( const StringClass &inString);
BOOL operator != ( const StringClass &inString);
BOOL operator <= ( const StringClass &inString);
BOOL operator >= ( const StringClass &inString);
BOOL operator <  ( const StringClass &inString);
BOOL operator >  ( const StringClass &inString);


/*************************************************************************
SLength:       The function returns the length of the
              string.
Note:         Unlike ANSI C library, this function returns
              length of the string including(!) '\0'

Pre:          An instance has been declared

Return:       U32
                 Size of memory used to allocate the string
                 (in bytes)
*************************************************************************/

U32 SLength( void ) const;


/*************************************************************************
==, !=, <, >,
<=, >=:      Relation Operators for StringClass and char*

Pre:         And instance is declared AND inited!

Receive:     char  *pChar - ptr to the char string

Return:      BOOL
                 TRUE
                 FALSE
*************************************************************************/

BOOL operator == ( char *pChar );

BOOL operator != ( char *pChar );

BOOL operator <= ( char *pChar );

BOOL operator >= ( char *pChar );

BOOL operator >  ( char *pChar );

BOOL operator <  ( char *pChar );


/*************************************************************************
+:           Concatenation operator for StringClass and
             StringClass

Pre:         The strings have been declared/buit

Receive:     StringClass   &inString  - the string to conc.

Return:      StringClass
                concatenated string
*************************************************************************/

StringClass operator + ( const StringClass &inString );


/***************************************************************************
():            Get a substring of a StringClass instance.

Pre:           Instance is declared and inited

Receive:       U32 first - index to start with
               U32 last  - index to finish at

Return:        StringClass
                    A substring that starts at position=first  and
                    ends at position=last

Usage:         s1 = s2.(1,3);

***************************************************************************/

StringClass operator() (U32 first, U32 last);


/*************************************************************************
SPosition:    Returns position of the first occurence
             of the substring.

Pre:         Both instances dec;ared and inited

Receive      StringClass  &inString - substring

Return:      int
                  index  if found
                  -1     if not found
*************************************************************************/

int SPosition( const StringClass &inString);


/*************************************************************************
SFloat
SInt:     Functions return string as a number

Pre:      Instance is declared and inited

Receive:  float &value        ||       int &value

Return:   BOOL
            TRUE  - string was successfully translated
                    into number
            FALSE - could not translate

Usage:   flag = str.SFloat(floatVar)  ||
         flag = str.SInt(intvAR)
*************************************************************************/

BOOL SInt   ( int   &var );

BOOL SFloat ( float &var );


#ifdef WIN32

/*************************************************************************
<<:          Output operator for the class

Pre:         An instance is declared/built

Receive      ostream       &os       - ref to ostream.
             StringClass   &inString - string to send to
                                       the stream

Post:        returns ios - the ref to stream

Usage:       <instance of ostream> << string
*************************************************************************/

friend ostream& operator << ( ostream &os, const StringClass &inString );


/*************************************************************************
>>:          Input operator for the class

Pre:         An instance is declared/built.

Receive      ostream       &is       - ref to istream.
             StringClass   &inString - instance of the
                                       class to store
                                       data in.

Post:        returns ios - the ref to stream

Usage:       <instance of ostream> << string
*************************************************************************/

friend istream& operator >> ( istream &is, StringClass &inString );

#endif // WIN32

};    

#endif //__STRING_CLASS_H__
