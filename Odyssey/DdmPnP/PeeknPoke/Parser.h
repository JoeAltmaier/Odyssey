// Parser.h: interface for the Parser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARSER_H__C3B13E86_0A2C_11D3_9C08_00105A2459CB__INCLUDED_)
#define AFX_PARSER_H__C3B13E86_0A2C_11D3_9C08_00105A2459CB__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "PeeknPoke.h"

#include "StringEx.h"
#include "LexicalAnalyser.h"
#include "DataLink.h"

class CPeeknPokeView;

class CPeeknPokeDoc;

typedef enum {
	CREATE_OK,
	GETDEF_OK,
	ENUM_OK,
	DEL_OK,
	INSERT_OK,
	RROW_OK,
	DROW_OK,
	MODROW_OK,
	MODFLD_OK,
	LISTEN_OK,
	TABLES_OK,
	LIST_OK,
	INVALID,
	VAR_NOT_FOUND,
	UNREG_TOKEN,
	MISSING_PARAM
} ERROR_CODE;

class CParser
{
protected:
	CLexicalAnalyser	lex;
	CMapStringToOb		IdList;
	CList<TokenNode, TokenNode&> ValueMap;
	CList<CCtToken, CCtToken&> Value;
	CDataLink			m_Link;
	CString				sTokenName;

public:
	CPeeknPokeDoc*  pDoc;

protected:
	// The main process module
	BOOL Send();
	ERROR_CODE ParseLine(CString& cmd);

public:
	CParser();
	virtual ~CParser();
	BOOL ProcessCommand(CString& cmd);
	BOOL CompileLine(CString& cmd,int& line);
	void SetDoc(CPeeknPokeDoc*  _pDoc) { m_Link.pDoc = _pDoc; pDoc = _pDoc; };
	BOOL Connect();
};

#endif // !defined(AFX_PARSER_H__C3B13E86_0A2C_11D3_9C08_00105A2459CB__INCLUDED_)
