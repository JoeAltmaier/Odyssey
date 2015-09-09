//ValueSet.h

#ifndef __ValueSet_H
#define __ValueSet_H

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <stdio.h>
#include <String.h>
#include "OsTypes.h"
#include "SSAPI_Codes.h"
#include "UnicodeString.h"
#include "StringClass.h"

class Value {
public:
	Value();
	virtual ~Value();

	//clean up and re-initialize
	virtual void Clear();

	virtual Value* Create();

	//reads its self from the stream
	int Parse(int iType, char* pValue, int iCode);

	virtual bool operator == (const Value &val);
	const Value& operator= (const Value &value );

	//returns the ssapi field code... used to communicate with the client
	virtual int get_ssapi_code() { return m_iCode; }

	virtual void SetInternal(const Value& value);

	//returns the size of the value field
	//the type is a base type the code is the field code (see SSAPI_Codes.h) 
	int Set(int iType, char* pValue, int iCode);
	//for setting a unicode string
	int Set(UnicodeString* pUS, int iCode);

	//returns the size needed to write this value
	virtual int GetSize();

	//writes this value to the pre-allocated buffer of size >= GetSize()
	//returns the size written
	virtual int Write(char* pBuf);

	//returns false if buffer not large enough... copies generic data into buffer
	BOOL GetGenericValue(char* pBuffer, int iSize);

	//internal storage of the value information
	int m_iType;
	char* m_pValue;
	int m_iCode;
};

class ValueSet : public Value {
public:
	//will parse its self if given a buffer
	ValueSet(char* pBuf = NULL, int iSize = 0);
	virtual ~ValueSet();

	virtual void Clear();
	virtual bool operator == (const Value &vs);
	const ValueSet& operator = (const ValueSet &vs);

	virtual void SetInternal(const Value& value) {
		*this = *((ValueSet*)&value);
	}

	//value sets are vectors
	virtual int get_ssapi_code() { return SSAPI_TYPE_VECTOR; }

	//returns the size needed to write this value set
	virtual int GetSize();

	//writes the value set to the pre-allocated buffer that is at least
	//as large as the size returned by GetSize()
	//returns the size written
	virtual int Write(char* pBuf);

	//reads its self from the stream
	void Parse(char* pBuf, int iSize);
	int ParseValue(int iType, char* pValueData, int iSize, int iCode);

	//returns the value, untouched... NOT A COPY
	Value* GetValue(int iCode);

	//returns the string contained by value at cCode
	BOOL GetString(int iCode, UnicodeString* pUS);
	//returns the int contained by the value at cCode
	BOOL GetInt(int iCode, int* pInt);
	//returns the int 64 contained by the value at cCode
	BOOL GetInt64(int iCode, I64* pInt64);
	//get a generic value, returns false if the size specified isn't big enough
	BOOL GetGenericValue(char* pBuffer, int iSize, int iCode);
	//get a float, returns false if not found
	BOOL GetFloat(int iCode, float* pFloat);
	//get a U32
	BOOL GetU32(int iCode, U32* pU32);
	//get a U8
	BOOL GetU8(int iCode, U8* pU8);
	//get a U16
	BOOL GetU16(int iCode, U16* pU16);
	//get a U64
	BOOL GetU64(int iCode, U64* pU64);
	//get a 8 bit Int
	BOOL GetInt8(int iCode, S8* pInt8);
	//get a 16 bit Int
	BOOL GetInt16(int iCode, S16* pInt16);
	//get a RowID
	BOOL GetRowID(int iCode, char* pRowID);
	//get an ASCII String
	BOOL GetASCIIString(int iCode, StringClass* pString);
	//get an ASCII String - memory already allocated
	BOOL GetASCIIString(int iCode, char* pString, int iSize);
	//get an ASCII String - memory to be allocated inside
	BOOL GetASCIIString(int iCode, char* pString);

	//add value wrappers
	int AddASCIIString(StringClass stData, int iCode);
	int AddASCIIString(char* pString, int iCode);
	int AddRowID(char* pRowID, int iCode);
	int AddString(UnicodeString* pusValue, int iCode);
	int AddInt(int iValue, int iCode);
	int AddInt64(I64 &iValue, int iCode);
	int	AddInt64( void *pValue, int iCode );
	int AddFloat(float fValue, int iCode);
	int AddGenericValue(char* pBuffer, int iSize, int iCode);
	int AddU32(U32 iValue, int iCode);
	int AddU8(U8 iValue, int iCode);
	int AddU16(U16 iValue, int iCode);
	int AddU64(U64 &iValue, int iCode);
	int AddInt8(S8 iValue, int iCode);
	int AddInt16(S16 iValue, int iCode);

	//add a well known value to value set... Value pass through
	int AddValue(int iType, char* pValueData, int iCode);
	//Value passthrough for unicode string
	int AddValue(UnicodeString* pUS, int iCode);

	//add a generic value pValueData of size iSize
	int AddValue(int iType, char* pValueData, int iSize, int iCode);
	//add a pre-constructed value
	int AddValue(Value* pValue);
	int AddValue(Value* pValue, int iCode);

private:
	//expects the value to be pre-allocated
	int AddValueInternal(Value* pValue);

public:
	// GetCount:
	//
	// PURPOSE:		Returns number of elements in the valueset
	int GetCount() const { return m_iValueSetSize; }

	//copy a value set
	virtual Value* Create();
	
	//internal value storage
	Value** m_apValueSet;
	int m_iValueSetSize;
};

#endif