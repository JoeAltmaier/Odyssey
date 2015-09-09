#include "stdafx.h"
#include "LexicalAnalyser.h"

//////////////////////////////////////////////////////////////////////
// CCtToken Class
//////////////////////////////////////////////////////////////////////

CCtToken::CCtToken(): TokenType(nilT), TokenInt(0), TokenUintT(0), TokenSintT(0), TokenI64(0)
{
}

CCtToken::~CCtToken()
{
}

//////////////////////////////////////////////////////////////////////
// CSymbol Class
//////////////////////////////////////////////////////////////////////

CSymbol::CSymbol(CString string, int type):	SymbolString(string),
											TokenType(type)
{
}

CSymbol::~CSymbol()
{
}

bool CSymbol::operator < (CSymbol &Symbol)
{
	return SymbolString.GetLength() < Symbol.SymbolString.GetLength();
}

bool CSymbol::operator <= (CSymbol &Symbol)
{
	return SymbolString.GetLength() <= Symbol.SymbolString.GetLength();
}

bool CSymbol::operator > (CSymbol &Symbol)
{
	return SymbolString.GetLength() > Symbol.SymbolString.GetLength();
}

bool CSymbol::operator >= (CSymbol &Symbol)
{
	return SymbolString.GetLength() >= Symbol.SymbolString.GetLength();
}

//////////////////////////////////////////////////////////////////////
// CLexicalAnalyser Class
//////////////////////////////////////////////////////////////////////

CLexicalAnalyser::CLexicalAnalyser(): CurrentPosition(NULL)
{
}

CLexicalAnalyser::~CLexicalAnalyser()
{
}

void CLexicalAnalyser::setSymbol(CSymbol Symbol)
{
	if(SymbolTable.GetSize()==0)
		SymbolTable.Add(Symbol);
	else
	{
		for(int i=0; i<SymbolTable.GetSize()-1; i++)
		{
			if(SymbolTable[i]>Symbol && SymbolTable[i+1] <= Symbol)
			{
				SymbolTable.InsertAt(i+1, Symbol);
				return;
			}
			else if(SymbolTable[i] <= Symbol)
			{
				SymbolTable.InsertAt(i,Symbol);
				return;
			}
		}
		SymbolTable.Add(Symbol);
	}

}

CSymbol::CSymbol()
{
}

void CLexicalAnalyser::String2TokenSequence(CString sequence)
{
	CCtToken token;

	TokenSequence.RemoveAll();
	CurrentPosition=TokenSequence.GetHeadPosition();

	sequence.TrimLeft();//trim off leading whitespaces, tabs and newlines
	sequence.TrimRight();//trim off ending whitespaces, tabs and newlines

	//This loop goes to every character and tokenise the string
	for(int i=0; i< sequence.GetLength(); i++)
	{
		token.TokenType = nilT;
 
		//if character is a space or tab, skip
		if(sequence[i] == ' ' || sequence[i] == '\t')
			continue;
		//else if the character is a " character...
		else if(sequence[i] == '\"') 
		{
			i++;
			token.TokenType = stringT;
			token.TokenName.Empty();

			while(i< sequence.GetLength() && sequence[i] != '\"') //while the closing " is not there
			{
				token.TokenName += sequence[i];
				i++;
			}
			AddToken(token);
		}
		//else if the character is a newline...
		else if(sequence[i] == '\r' && sequence[i+1] == '\n')
		{
			token.TokenType = endlineT;
			i++;
			AddToken(token);
		}
		//else if the character is a digit or a decimal point...
		else if(isdigit(sequence[i]) || sequence[i] == '.' || sequence[i] == '+' || sequence[i] == '-')
		{
			token.TokenType = intT;
			token.TokenName.Empty();
			//while the character is within bounds and is either a 
			//digit or decimal point...
			if( sequence[i] == '-' )
			{
				token.TokenType = sintT;
				token.TokenName += sequence[i++];
			}
			while(i< sequence.GetLength() && (isdigit(sequence[i]) || sequence[i] == '.' || sequence[i] == '+' || sequence[i] == '-'))
			{
				if(sequence[i] == '.')
					token.TokenType = realT;
				token.TokenName += sequence[i];
				i++;
			}
			i--;//Need to decrement it because in the for loop, i will be incremented
			if(token.TokenType == intT || token.TokenType == sintT)
			{
				if( token.TokenName.GetLength() < 10 )
					token.TokenInt = atol((LPCSTR)token.TokenName);
				else
					if( _atoi64((LPCSTR)token.TokenName) > LONG_MAX || _atoi64((LPCSTR)token.TokenName) < LONG_MIN)
					{
						token.TokenType = I64T;
						token.TokenI64 = _atoi64((LPCSTR)token.TokenName);
					}
					else
						token.TokenInt = atol((LPCSTR)token.TokenName);
			}
			else if( token.TokenType == realT )
				token.TokenReal=atof((LPCSTR)token.TokenName);

			//if invalid decimal number is given
			if(token.TokenType == realT &&token.TokenReal==0.0)
				token.TokenType = errorT;
			AddToken(token);
		}
		//else if the character is still neither number, newline or quotes,
		//it may be one of the symbols
		else if(token.TokenType == nilT)
		{
			//This loop look up the symbol table and assign the correct token type
			for(int j=0; j < SymbolTable.GetSize(); j++)
			{
				CString str = sequence.Mid(i,SymbolTable[j].SymbolString.GetLength());
				str.MakeUpper();
				//if the following sequence string matches the string of one of
				//the symbol in the symbol table...
				if( (sequence.Mid(i,SymbolTable[j].SymbolString.GetLength()) == SymbolTable[j].SymbolString ||
					str == SymbolTable[j].SymbolString )
					&& !IsAllAplhaOrDigit(sequence,i,SymbolTable[j].SymbolString.GetLength()+1) )
				{//...assign the correct token type
					token.TokenType=SymbolTable[j].TokenType;
					i += SymbolTable[j].SymbolString.GetLength()-1;
					AddToken(token);
					break;
				}
			}
		}
		//still, if the character is still not one of the symbols, check
		//whether it is an identifier
		if(token.TokenType == nilT)
		{
			token.TokenName.Empty();
			
			//if character at sequence[i] is not either digit or alphabets, 
			//or space don't even bother scanning- it must be an error
			if(i<sequence.GetLength() && (isalpha(sequence[i]) || isdigit(sequence[i]) ) )
			{
				//condition: i within bounds and the character is either a character or digit or '_'
				while(i<sequence.GetLength()  && (isalpha(sequence[i]) || isdigit(sequence[i]) || sequence[i] == '_') )
				{
					token.TokenType=idT;
					token.TokenName+=sequence[i];
					i++;
				}
				if(i<sequence.GetLength()) //if i is within bounds of the string
					if(sequence[i] != ' '/* && !isalpha(sequence[i]) 
						&& !isdigit(sequence[i])*/ ) 
						i--;
				if(token.TokenType != nilT) 
					AddToken(token);
			}
		}
		//still, if the character is not an identifier, an error has occured
		if(token.TokenType == nilT)
		{
			token.TokenType = errorT;
			AddToken(token);
		}
	}
	token.TokenType = endfileT;
	AddToken(token);
	CurrentPosition=TokenSequence.GetHeadPosition();
}

CCtToken CLexicalAnalyser::GetCurrentToken()
{
	CCtToken NIL;
	NIL.TokenType = nilT;

	if(CurrentPosition == NULL) 
		return NIL;
	else
		return TokenSequence.GetAt(CurrentPosition);
}

CCtToken CLexicalAnalyser::NextToken()
{
	CCtToken NIL;
	NIL.TokenType = nilT;

	if(CurrentPosition == NULL) 
		return NIL;
	else
		return TokenSequence.GetNext(CurrentPosition);
}

void CLexicalAnalyser::AddToken(CCtToken token)
{
	CurrentPosition=TokenSequence.AddTail(token);
}

BOOL CLexicalAnalyser::IsSequenceEmpty()
{
	return TokenSequence.IsEmpty();
}

bool CLexicalAnalyser::IsAllAplhaOrDigit(CString s, int begin, int length)
{
	//if the length is overshot
	if(begin+length>=s.GetLength())
		return false;
	for(int i=begin; i<begin+length; i++)
	{
		if(!isalpha(s[i]) && !isdigit(s[i])) 
			return false;
	}

	return true;
}

void CLexicalAnalyser::ResetPosition()
{
	CurrentPosition = TokenSequence.GetHeadPosition();	
}

void CLexicalAnalyser::ClearAllTokens()
{
	while(!TokenSequence.IsEmpty())
		TokenSequence.RemoveHead();
}

POSITION CLexicalAnalyser::GetCurrentPosition()
{
	return CurrentPosition;
}

void CLexicalAnalyser::SetCurrentPosition(POSITION pos)
{
	CurrentPosition=pos;
}