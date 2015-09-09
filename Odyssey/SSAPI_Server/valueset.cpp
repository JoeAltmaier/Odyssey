//ValueSet.cpp

#include "valueset.h"
#include "byte_array_utils.h"

//------------Value--------------
Value::Value() {
	m_pValue = NULL;
	Clear();
}

Value::~Value() {
	Clear();
}

void Value::Clear() {
	if(m_pValue) {
		delete[] m_pValue;
		m_pValue = NULL;
	}
	m_iType = 0;
	m_iCode = 0;
}

//creates a new value with the same contents as this one
Value* Value::Create() {
	Value* pValue = new Value();
	pValue->Set(m_iType, m_pValue, m_iCode);
	return pValue;
}

const Value& Value::operator= (const Value &value ){
	SetInternal(value);
	return value;
}

void Value::SetInternal(const Value &value)  {
	Clear();
	Set(value.m_iType, value.m_pValue, value.m_iCode);
}

//checks for atomic equality
bool Value::operator == (const Value &val) {
	if(m_iType == val.m_iType &&
		m_iCode == val.m_iCode) {
		switch(m_iType) {
			case SSAPI_TYPE_STRING:
				{
					UnicodeString us((void*)m_pValue);
					UnicodeString v_us((void*)(val.m_pValue));
					if(us == v_us)
						return true;
					break;
				}
			case SSAPI_TYPE_FLOAT:
			case SSAPI_TYPE_INT:
			case SSAPI_TYPE_U32:
				{
					for(int i=0;i<sizeof(int);i++) {
						if(m_pValue[i] != val.m_pValue[i])
							return false;
					}
					return true;
				}
			case SSAPI_TYPE_INT_64:
				{
					for(int i=0;i<8;i++) {
						if(m_pValue[i] != val.m_pValue[i])
							return false;
					}
					return true;
				}
			case SSAPI_TYPE_GENERIC:
			case SSAPI_TYPE_ROW_ID:
			{
				int iThisSize;
				//byte_array_utils::Write((char*)(&iThisSize), m_pValue, sizeof(int));
				memcpy((char*)(&iThisSize), m_pValue, sizeof(int));

				int iThatSize;
				//byte_array_utils::Write((char*)(&iThatSize), val.m_pValue, sizeof(int));
				memcpy((char*)(&iThatSize), val.m_pValue, sizeof(int));

				if(iThisSize == iThatSize) {
					for(int i=sizeof(int);i<iThisSize + (signed)sizeof(int);i++) {
						if(m_pValue[i] != val.m_pValue[i])
							return false;
					}
					return true;
				}
			}
		};
	}
	return false;
}

//returns false if size not adequate, copies only the value data, not the size info
BOOL Value::GetGenericValue(char* pBuffer, int iSize) {

	if(m_iType != SSAPI_TYPE_GENERIC && m_iType != SSAPI_TYPE_ROW_ID)
		return FALSE;

	int iThisSize;
	memcpy((char*)(&iThisSize), m_pValue, sizeof(int));

	if(iSize < iThisSize)
		return FALSE;

	memcpy(pBuffer, m_pValue + sizeof(int), iThisSize);
	return TRUE;
}

//returns size needed to write this value in bytes
int Value::GetSize() {
	//the header is used to describe the field and type 
	int iHeaderSize = SSAPI_INTERNAL_VALUE_HEADER_SIZE;//iCode, cType
	switch(m_iType) {
	case SSAPI_TYPE_STRING:
		{
			UnicodeString us((void*)m_pValue);
			return us.GetSize() + iHeaderSize;
		}
	case SSAPI_TYPE_U32:
	case SSAPI_TYPE_INT:
	case SSAPI_TYPE_FLOAT:
		return 4 + iHeaderSize;
	case SSAPI_TYPE_GENERIC:
	case SSAPI_TYPE_ROW_ID:
		{
			int iThisSize;
			memcpy((char*)(&iThisSize), m_pValue, sizeof(int));
			return iThisSize + sizeof(int) + iHeaderSize;
		}
	case SSAPI_TYPE_INT_64:
	case SSAPI_TYPE_U64:
		return 8 + iHeaderSize;
	case SSAPI_TYPE_U8:
	case SSAPI_TYPE_INT8:
		return 1 + iHeaderSize;
	case SSAPI_TYPE_U16:
	case SSAPI_TYPE_INT16:
		return 2 + iHeaderSize;
	default:
		printf("Value::GetSize() default reached\n");
		return 0;
	}
}

//reads from stream...external
int Value::Parse(int iType, char* pValue, int iCode) {

	Clear();

	int iRet = 0;
	switch(iType) {
	case SSAPI_TYPE_STRING:
		{
			UnicodeString us((void*)pValue);
			m_pValue = new char[us.GetSize()];
			memcpy(m_pValue, pValue, us.GetSize());
			iRet = us.GetSize();
			break;
		}
	case SSAPI_TYPE_INT:
	case SSAPI_TYPE_FLOAT:
	case SSAPI_TYPE_U32:
		m_pValue = new char[4];
		iRet = byte_array_utils::Write(m_pValue, pValue, 4);
		break;
	case SSAPI_TYPE_U8:
	case SSAPI_TYPE_INT8:
		m_pValue = new char[1];
		iRet = byte_array_utils::Write(m_pValue, pValue, 1);
		break;
	case SSAPI_TYPE_U16:
	case SSAPI_TYPE_INT16:
		m_pValue = new char[2];
		iRet = byte_array_utils::Write(m_pValue, pValue, 2);
		break;
	case SSAPI_TYPE_INT_64:
	case SSAPI_TYPE_U64:
		m_pValue = new char[8];
		iRet = byte_array_utils::Write(m_pValue, pValue, 8);
		break;
	case SSAPI_TYPE_GENERIC:
	case SSAPI_TYPE_ROW_ID:
		int iGenericSize;
		byte_array_utils::Write((char*)(&iGenericSize), pValue, sizeof(int));
		m_pValue = new(tSMALL) char[iGenericSize + sizeof(int)];
		memcpy(m_pValue, &iGenericSize, sizeof(int));
		if(iType == SSAPI_TYPE_GENERIC) {
			memcpy(m_pValue + sizeof(int), pValue + sizeof(int), iGenericSize);
		}
		else {
			//need to pack in the values
			byte_array_utils::Write(m_pValue + sizeof(int), pValue + sizeof(int), 2);
			byte_array_utils::Write(m_pValue + sizeof(int) + 2, pValue + sizeof(int) + 2, 2);
			byte_array_utils::Write(m_pValue + sizeof(int) + 4, pValue + sizeof(int) + 4, 4);
		}
		iRet = iGenericSize + sizeof(int);
		break;
	default:
		printf("Value:: Bad value type\n");
		m_pValue = NULL;
		break;
	};
	m_iType = iType;
	m_iCode = iCode;
	return iRet;
}

//writes this value to the buffer... returns size written
int Value::Write(char* pBuf) {
	int iSize = 0;

	byte_array_utils::Write(pBuf + iSize, (char*)(&m_iCode), sizeof(int));
	iSize += sizeof(int);

	pBuf[iSize++] = (char)m_iType;
	switch(m_iType) {
	case SSAPI_TYPE_STRING:
		{
			UnicodeString us((void*)m_pValue);
			memcpy(pBuf + iSize, m_pValue, us.GetSize());
			iSize += us.GetSize();
			break;
		}
	case SSAPI_TYPE_INT:
	case SSAPI_TYPE_FLOAT:
	case SSAPI_TYPE_U32:
		byte_array_utils::Write(pBuf + iSize, m_pValue, 4);
		iSize += 4;
		break;
	case SSAPI_TYPE_INT_64:
	case SSAPI_TYPE_U64:
		byte_array_utils::Write(pBuf + iSize, m_pValue, 8);
		iSize += 8;
		break;
	case SSAPI_TYPE_U8:
	case SSAPI_TYPE_INT8:
		byte_array_utils::Write(pBuf + iSize, m_pValue, 1);
		iSize += 1;
		break;
	case SSAPI_TYPE_U16:
	case SSAPI_TYPE_INT16:
		byte_array_utils::Write(pBuf + iSize, m_pValue, 2);
		iSize += 2;
		break;
	case SSAPI_TYPE_GENERIC:
	case SSAPI_TYPE_ROW_ID:
		{
			int iValueSize;
			memcpy((char*)(&iValueSize), m_pValue, sizeof(int));
			iSize += byte_array_utils::Write(pBuf + iSize, (char*)(&iValueSize), sizeof(int));
			if(m_iType == SSAPI_TYPE_GENERIC) {
				memcpy(pBuf + iSize, m_pValue + sizeof(int), iValueSize);
			}
			else {
				byte_array_utils::Write(pBuf + iSize, m_pValue + sizeof(int), 2);
				byte_array_utils::Write(pBuf + iSize + 2, m_pValue + sizeof(int) + 2, 2);
				byte_array_utils::Write(pBuf + iSize + 4, m_pValue + sizeof(int) + 4, 4);
			}
			iSize += iValueSize;
		}
		break;
	};
	return iSize;
}

//returns the size of the field set... a Unicode String of type SSAPI_type_string
int Value::Set(UnicodeString *pUS, int iCode) {

	Clear();

	m_iType = SSAPI_TYPE_STRING;
	m_pValue = new char[pUS->GetSize()];
	pUS->CString(m_pValue, pUS->GetSize());
	m_iCode = iCode;
	return pUS->GetSize();
}

//returns the size of the value field
int Value::Set(int iType, char* pValue, int iCode) {

	Clear();

	int iRet = 0;
	switch(iType) {
	case SSAPI_TYPE_STRING:
		{
			UnicodeString us((void*)pValue);
			m_pValue = new char[us.GetSize()];
			memcpy(m_pValue, pValue, us.GetSize());
			iRet = us.GetSize();
			break;
		}
	case SSAPI_TYPE_INT:
	case SSAPI_TYPE_U32:
	case SSAPI_TYPE_FLOAT:
		m_pValue = new char[4];
		memcpy(m_pValue, pValue, 4);
		iRet += 4;
		break;
	case SSAPI_TYPE_INT_64:
	case SSAPI_TYPE_U64:
		m_pValue = new char[8];
		memcpy(m_pValue, pValue, 8);
		iRet += 8;
		break;
	case SSAPI_TYPE_GENERIC:
	case SSAPI_TYPE_ROW_ID:
		int iGenericSize;
		memcpy((char*)(&iGenericSize), pValue, sizeof(int));
		m_pValue = new(tSMALL) char[iGenericSize + sizeof(int)];
		memcpy(m_pValue, pValue, iGenericSize + sizeof(int));
		iRet = iGenericSize + sizeof(int);
		break;
	case SSAPI_TYPE_U8:
	case SSAPI_TYPE_INT8:
		m_pValue = new char[1];
		memcpy(m_pValue, pValue, 1);
		iRet += 1;
		break;
	case SSAPI_TYPE_U16:
	case SSAPI_TYPE_INT16:
		m_pValue = new char[2];
		memcpy(m_pValue, pValue, 2);
		iRet += 2;
		break;
	default:
		printf("Value:: Bad value type\n");
		m_pValue = NULL;
		break;
	};
	m_iType = iType;
	m_iCode = iCode;
	return iRet;
}


//------------ValueSet--------------
//a value set is a vector of values, the constructor parses its self if pBuf != NULL
ValueSet::ValueSet(char* pBuf, int iSize) : Value() {
	m_apValueSet = NULL;
	m_iValueSetSize = 0;
	m_iType = SSAPI_TYPE_VECTOR;
	if(pBuf) {
		Parse(pBuf, iSize);
	}
}

ValueSet::~ValueSet() {
	Clear();
}

//used to create a new value set with the same contents as this one.
Value* ValueSet::Create() {
	ValueSet* pNew = new ValueSet(); 
	*pNew = *this;
	return pNew;
}

//used to empty this value set, used by destructor
void ValueSet::Clear() {
	if(m_apValueSet) {
		for(int i=0;i<m_iValueSetSize;i++) {
			if(m_apValueSet[i])
				delete m_apValueSet[i];
		}
		delete[] m_apValueSet;
		m_apValueSet = NULL;
	}
	m_iValueSetSize = 0;
	m_iType = SSAPI_TYPE_VECTOR;
}

//checks for equality
bool ValueSet::operator == (const Value &vs) {
	if(m_iValueSetSize != ((ValueSet&)vs).m_iValueSetSize)
		return false;
	for(int i=0;i<m_iValueSetSize;i++) {
		if(!(*(m_apValueSet[i]) == *(((ValueSet&)vs).m_apValueSet[i])))
			return false;
	}
	return true;
}

//asignment, used by Create()
const ValueSet& ValueSet::operator = (const ValueSet& vs) {

	Clear();

	if(vs.m_apValueSet) {
		for(int i=0;i<vs.m_iValueSetSize;i++) {
			if(vs.m_apValueSet[i]) {
				AddValue(vs.m_apValueSet[i]);
			}
		}
	}

	m_iType = vs.m_iType;
	m_iCode = vs.m_iCode;

	return vs;
}

//this method reads a value set sent from the client
void ValueSet::Parse(char* pBuf, int iSize) {
	int iPos = 0;
	int iCount = 0;

	if(iSize >= sizeof(int)) {
		byte_array_utils::Write((char*)(&iCount), pBuf, sizeof(int));
		iPos += sizeof(int);
	}

	for(int i=0;i<iCount;i++) {
		int iReqCode;
		iPos += byte_array_utils::Write((char*)(&iReqCode), pBuf + iPos, sizeof(int));

		char cType = pBuf [iPos++];

		int iRet = ParseValue(cType, pBuf + iPos, iSize - iPos, iReqCode);

		if(iRet)
			iPos += iRet - 5;//code + type
		else //we didn't understand the buffer, terminate
			break;
	}
}

//called to parse a new value
int ValueSet::ParseValue(int iType, char* pValueData, int iSize, int iCode) {

	//handle the nested case
	if(iType == SSAPI_TYPE_VECTOR) {
		ValueSet* pVS = new ValueSet(pValueData, iSize);
		pVS->m_iCode = iCode;
		return AddValueInternal(pVS);
	}

	Value* pValue = new Value();
	pValue->Parse(iType,pValueData, iCode);

	return AddValueInternal(pValue);
}

//writes the vector to the array
int ValueSet::Write(char* pBuf) {

	int iSize = byte_array_utils::Write(pBuf, (char*)(&m_iCode), sizeof(int));

	pBuf[iSize++] = m_iType;

	byte_array_utils::Write(pBuf + iSize, (char*)(&m_iValueSetSize), sizeof(int));
	iSize += sizeof(int);
	
	for(int i=0;i<m_iValueSetSize;i++) {
		iSize += m_apValueSet[i]->Write(pBuf + iSize);
	}
	return iSize;
}

//return the size in bytes this value set will take to write
int ValueSet::GetSize() {
	int iSize = 5 + sizeof(int);//cType, iCode, iCount
	for(int i=0;i<m_iValueSetSize;i++) {
		iSize += m_apValueSet[i]->GetSize();
	}
	return iSize;
}

//pick the value out and return it... NOT A COPY
Value* ValueSet::GetValue(int iCode) {
	for(int i=0;i<m_iValueSetSize;i++) {
		if(m_apValueSet[i]->m_iCode == iCode)
			return m_apValueSet[i];
	}
	return NULL;
}

BOOL ValueSet::GetGenericValue(char* pBuffer, int iSize, int iCode) {
	Value* pValue = GetValue(iCode);
	if(!pValue || (pValue->m_iType != SSAPI_TYPE_GENERIC && pValue->m_iType != SSAPI_TYPE_ROW_ID))
		return FALSE;
	return pValue->GetGenericValue(pBuffer, iSize);
}

//returns false if value not found
BOOL ValueSet::GetString(int iCode, UnicodeString* pUS) {
	Value* pValue = GetValue(iCode);
	if(!pValue || (pValue->m_iType != SSAPI_TYPE_STRING) )
		return FALSE;
	UnicodeString us(pValue->m_pValue);
	*pUS = us;
	return TRUE;
}

BOOL ValueSet::GetInt64(int iCode, I64* pInt64) {
	Value* pValue = GetValue(iCode);
	if(!pValue || (pValue->m_iType != SSAPI_TYPE_INT_64))
		return FALSE;
	memcpy( pInt64, pValue->m_pValue, sizeof(I64) );
	return TRUE;
}

BOOL ValueSet::GetInt(int iCode, int* pInt) {
	Value* pValue = GetValue(iCode);
	if(!pValue || (pValue->m_iType != SSAPI_TYPE_INT) || !(pValue->m_pValue))
		return FALSE;
	*pInt = *((int*)(pValue->m_pValue));
	return TRUE;
}

BOOL ValueSet::GetFloat(int iCode, float* pFloat) {
	Value* pValue = GetValue(iCode);
	if(!pValue || (pValue->m_iType != SSAPI_TYPE_FLOAT) )
		return FALSE;
	*pFloat = *((float*)(pValue->m_pValue));
	return TRUE;
}

BOOL ValueSet::GetU32(int iCode, U32* pU32) {
	Value* pValue = GetValue(iCode);
	if(!pValue || ((pValue->m_iType != SSAPI_TYPE_U32) && (pValue->m_iType != SSAPI_TYPE_INT)) )
		return FALSE;
	memcpy(pU32, pValue->m_pValue, 4);
	return TRUE;
}

//get a U8
BOOL ValueSet::GetU8(int iCode, U8* pU8) {
	Value* pValue = GetValue(iCode);
	if(!pValue || (pValue->m_iType != SSAPI_TYPE_U8) )
		return FALSE;
	memcpy(pU8, pValue->m_pValue, 1);
	return TRUE;
}

//get a U16
BOOL ValueSet::GetU16(int iCode, U16* pU16) {
	Value* pValue = GetValue(iCode);
	if(!pValue || (pValue->m_iType != SSAPI_TYPE_U16) )
		return FALSE;
	memcpy(pU16, pValue->m_pValue, 2);
	return TRUE;
}

//get a U64
BOOL ValueSet::GetU64(int iCode, U64* pU64) {
	Value* pValue = GetValue(iCode);
	if(!pValue || (pValue->m_iType != SSAPI_TYPE_U64))
		return FALSE;
	memcpy(pU64, pValue->m_pValue, 8);
	return TRUE;
}

//get a 8 bit Int
BOOL ValueSet::GetInt8(int iCode, S8* pInt8) {
	Value* pValue = GetValue(iCode);
	if(!pValue || (pValue->m_iType != SSAPI_TYPE_INT8) )
		return FALSE;
	memcpy(pInt8, pValue->m_pValue, 1);
	return TRUE;
}

//get a 16 bit Int
BOOL ValueSet::GetInt16(int iCode, S16* pInt16) {
	Value* pValue = GetValue(iCode);
	if(!pValue || (pValue->m_iType != SSAPI_TYPE_INT16) )
		return FALSE;
	memcpy(pInt16, pValue->m_pValue, 2);
	return TRUE;
}

BOOL ValueSet::GetRowID(int iCode, char* pRowID) {
	return GetGenericValue(pRowID, 8, iCode);
}

BOOL ValueSet::GetASCIIString(int iCode, StringClass* pString) {
	Value* pValue = GetValue(iCode);
	if(!pValue || (pValue->m_iType != SSAPI_TYPE_STRING))
		return FALSE;
	UnicodeString us(pValue->m_pValue);
	us.GetAsciiString(*pString);
	return TRUE;
}

//pString is pre-allocated
BOOL ValueSet::GetASCIIString(int iCode, char* pString, int iSize) {
	StringClass stData;
	if(! GetASCIIString(iCode, &stData))
		return FALSE;
	if(iSize <= 0) {
		pString = new char[stData.SLength() + 1];
	}
	return stData.CString(pString,(U32)iSize);
}

//pString is to be allocated
BOOL ValueSet::GetASCIIString(int iCode, char* pString) {
	return GetASCIIString(iCode, pString, 0);
}

int ValueSet::AddASCIIString(StringClass stData, int iCode) {
	UnicodeString usData(stData);
	return AddString(&usData, iCode);
}

int ValueSet::AddASCIIString(char* pString, int iCode) {
	StringClass stData(pString);
	return AddASCIIString(stData, iCode);
}

int ValueSet::AddRowID(char* pRowID, int iCode) {
	return AddValue(SSAPI_TYPE_ROW_ID, pRowID, 8, iCode);
}

int ValueSet::AddString(UnicodeString* pusValue, int iCode) {
	return AddValue(pusValue, iCode);
}

int ValueSet::AddInt(int iValue, int iCode) {
	return AddValue(SSAPI_TYPE_INT,(char*)&iValue, iCode);
}

int ValueSet::AddInt64(I64 &iValue, int iCode) {
	I64	temp;
	
	memcpy( &temp, &iValue, sizeof(I64) );

	return AddValue(SSAPI_TYPE_INT_64, (char*)&temp, iCode);
}

int ValueSet::AddInt64( void *pValue, int iCode ){
	return AddValue(SSAPI_TYPE_INT_64, (char*)pValue, iCode);
}

int ValueSet::AddFloat(float fValue, int iCode) {
	return AddValue(SSAPI_TYPE_FLOAT, (char*)&fValue, iCode);
}

int ValueSet::AddGenericValue(char* pBuffer, int iSize, int iCode) {
	return AddValue(SSAPI_TYPE_GENERIC, pBuffer, iSize, iCode);
}

int ValueSet::AddValue(Value* pValue, int iCode) {
	pValue->m_iCode = iCode;
	return AddValueInternal(pValue->Create());
}

int ValueSet::AddU32(U32 iValue, int iCode) {
	return AddValue(SSAPI_TYPE_U32, (char*)&iValue, iCode);
}

int ValueSet::AddU8(U8 iValue, int iCode) {
	return AddValue(SSAPI_TYPE_U8, (char*)&iValue, iCode);
}

int ValueSet::AddU16(U16 iValue, int iCode) {
	return AddValue(SSAPI_TYPE_U16, (char*)&iValue, iCode);
}

int ValueSet::AddU64(U64 &iValue, int iCode) {

	U64	temp;
	
	memcpy( &temp, &iValue, sizeof(U64) );

	return AddValue(SSAPI_TYPE_U64, (char*)&temp, iCode);
}

int ValueSet::AddInt8(S8 iValue, int iCode) {
	return AddValue(SSAPI_TYPE_INT8, (char*)&iValue, iCode);
}

int ValueSet::AddInt16(S16 iValue, int iCode) {
	return AddValue(SSAPI_TYPE_INT16, (char*)&iValue, iCode);
}

//called when building a value set
int ValueSet::AddValue(Value* pValue) {
	return AddValueInternal(pValue->Create());
}

//used to add a pre-allocated value
int ValueSet::AddValueInternal(Value* pValue) {
	//try to find this value
	Value* pValuePresent = GetValue(pValue->m_iCode);
	if(pValuePresent) {
		if(pValuePresent->m_iType != pValue->m_iType) {
			Tracef("attempt to add conflicting value types");
		}
		else {
			*pValuePresent = *pValue;
			delete pValue;
			return pValuePresent->GetSize();
		}
	}

	m_iValueSetSize++;
	Value** pValueSetTemp = new Value*[m_iValueSetSize];
	
	if(m_apValueSet) {
		memcpy(pValueSetTemp, m_apValueSet, sizeof(Value*) * (m_iValueSetSize - 1));
		delete[] m_apValueSet;
	}
	
	m_apValueSet = pValueSetTemp;
	m_apValueSet[m_iValueSetSize - 1] = pValue;

	return pValue->GetSize();
}

//pass through to Value creation
int ValueSet::AddValue(int iType, char* pValueData, int iCode) {
	Value* pValue = new Value();
	pValue->Set(iType, pValueData, iCode);
	return AddValueInternal(pValue);
}

int ValueSet::AddValue(UnicodeString* pUS, int iCode) {
	Value* pValue = new Value();
	pValue->Set(pUS, iCode);
	return AddValueInternal(pValue);
}

//called to add generic data pValueData of size iSize...
int ValueSet::AddValue(int iType, char* pValueData, int iSize, int iCode) {

	if(pValueData) {
		//handle the nested case
		if(iType == SSAPI_TYPE_VECTOR) {
			ValueSet* pVS = new ValueSet(pValueData, iSize);
			pVS->m_iCode = iCode;
			return AddValueInternal(pVS);
		}

		Value* pValue = new Value();

		if(iType == SSAPI_TYPE_GENERIC || iType == SSAPI_TYPE_ROW_ID) {
			char* pTemp = new(tSMALL) char[iSize + 4];
			memcpy(pTemp, (char*)(&iSize), sizeof(int));
			memcpy(pTemp + sizeof(int), pValueData, iSize);

			int iRet = pValue->Set(iType, pTemp, iCode);

			delete[] pTemp;
		}
		else {
			pValue->Set(iType,pValueData, iCode);
		}

		return AddValueInternal(pValue);
	}
	return 0;
}
