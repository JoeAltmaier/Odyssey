#ifndef __LEXICALANALYSER_H__
#define __LEXICALANALYSER_H__

#include <afxtempl.h>
#include "PnPMsg.h"

enum {nilT=100, idT, intT, uintT, sintT, I64T, realT, stringT, rowIDT, errorT, endlineT, endfileT};//

class CLexicalAnalyser;

class CCtToken  
{
	friend class CLexicalAnalyser;
public:
	CCtToken();
	virtual ~CCtToken();

	int		getTokenType()		{ return TokenType;}
	CString getTokenName()		{ return TokenName;}
	int		getTokenInt()		{ return TokenInt; }
	double	getTokenReal()		{ return TokenReal;}

	unsigned long getTokenUL()	{ return TokenUintT; }
	signed long getTokenSL()	{ return TokenSintT; }
	__int64	getTokenI64()		{ return TokenI64; }
	rowID	getTokenRowID()		{ return TokenrowID;}

protected:
	int TokenType;

	CString TokenName;
	union
	{
		int				TokenInt;
		unsigned long	TokenUintT;
		signed long		TokenSintT;
		double			TokenReal;
		__int64			TokenI64;
		rowID			TokenrowID;
	};
};

class CSymbol  
{
public:
	CSymbol();
	CSymbol( CString string, int type);
	virtual ~CSymbol();

	CString SymbolString;
	int TokenType;

	bool operator < (CSymbol &Symbol);
	bool operator <= (CSymbol &Symbol);

	bool operator > (CSymbol &Symbol);
	bool operator >= (CSymbol &Symbol);

};

class CLexicalAnalyser  
{
public:
	void	SetCurrentPosition(POSITION pos);
	POSITION GetCurrentPosition();
	void	ResetPosition();

	bool	IsAllAplhaOrDigit(CString s,int begin, int length);
	BOOL	IsSequenceEmpty();
	void	String2TokenSequence(CString sequence);
	void	setSymbol(CSymbol Symbol);

	void	ClearAllTokens();
	void	AddToken(CCtToken token);
	CCtToken	NextToken();
	CCtToken	GetCurrentToken();

	CLexicalAnalyser();
	virtual ~CLexicalAnalyser();

protected:
	POSITION CurrentPosition;
	CArray<CSymbol,CSymbol &> SymbolTable;
	CList<CCtToken, CCtToken &> TokenSequence;

};

#endif // __LEXICALANALYSER_H__