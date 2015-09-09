// Parser.cpp: implementation of the Parser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <string.h>
#include "Parser.h"
#include "PeeknPokeDoc.h"
#include "PeeknPokeView.h"
#include "PnPMsg.h"

const createKeywordT		= endfileT + 1;
const getdefKeywordT		= endfileT + 2;
const enumKeywordT			= endfileT + 3;
const listKeywordT			= endfileT + 4;
const insertKeywordT		= endfileT + 5;
const equalOperatorT		= endfileT + 6;
const helpKeywordT			= endfileT + 7;

const modifyrowKeywordT		= endfileT + 8;
const readrowKeywordT		= endfileT + 9;
const deleterowKeywordT		= endfileT + 10;
const modifyfieldKeywordT	= endfileT + 11;
const deleteKeywordT		= endfileT + 12;
const listenKeywordT		= endfileT + 13;
const tablesKeywordT		= endfileT + 14;
const storeKeywordT			= endfileT + 15;
const downloadKeywordT		= endfileT + 16;
const runKeywordT			= endfileT + 17;
const persistentKeywordT	= endfileT + 18;
const startrowKeywordT		= endfileT + 19;
const rowIDKeywordT			= endfileT + 20;
const tableIDKeywordT		= endfileT + 21;
const HiPartKeywordT		= endfileT + 22;
const LoPartKeywordT		= endfileT + 23;
const stoplistenKeywordT	= endfileT + 24;

//////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////
CParser::CParser()
{
	// My own define token
	lex.setSymbol( CSymbol("create"	, createKeywordT) );
	lex.setSymbol( CSymbol("getdef"	, getdefKeywordT) );
	lex.setSymbol( CSymbol("enum"	, enumKeywordT  ) );
	lex.setSymbol( CSymbol("list"	, listKeywordT  ) );
	lex.setSymbol( CSymbol("insert"	, insertKeywordT) );
	lex.setSymbol( CSymbol("="		, equalOperatorT) );
	lex.setSymbol( CSymbol("help"	, helpKeywordT) );

	lex.setSymbol( CSymbol("modifyrow"	, modifyrowKeywordT) );
	lex.setSymbol( CSymbol("readrow"	, readrowKeywordT) );
	lex.setSymbol( CSymbol("deleterow"	, deleterowKeywordT) );
	lex.setSymbol( CSymbol("modifyfield"	, modifyfieldKeywordT) );
	lex.setSymbol( CSymbol("delete"	, deleteKeywordT) );
	lex.setSymbol( CSymbol("listen"	, listenKeywordT) );
	lex.setSymbol( CSymbol("tables"	, tablesKeywordT) );
	lex.setSymbol( CSymbol("store"	, storeKeywordT) );
	lex.setSymbol( CSymbol("download"	, downloadKeywordT) );
	lex.setSymbol( CSymbol("run"	, runKeywordT) );
	lex.setSymbol( CSymbol("persistent"	, persistentKeywordT) );
	lex.setSymbol( CSymbol("startrow"	, startrowKeywordT) );
	lex.setSymbol( CSymbol("rowID"	, rowIDKeywordT) );
	lex.setSymbol( CSymbol("tableID", tableIDKeywordT) );
	lex.setSymbol( CSymbol("HiPart"	, HiPartKeywordT) );
	lex.setSymbol( CSymbol("LoPart"	, LoPartKeywordT) );
	lex.setSymbol( CSymbol("stoplisten", stoplistenKeywordT) );

	lex.setSymbol( CSymbol("BINARY_FT", BINARY_FT) );
	lex.setSymbol( CSymbol("S32_FT" , S32_FT) );
	lex.setSymbol( CSymbol("U32_FT" , U32_FT) );
	lex.setSymbol( CSymbol("S64_FT" , S64_FT) );
	lex.setSymbol( CSymbol("U64_FT" , U64_FT) );
	lex.setSymbol( CSymbol("STRING16_FT", STRING16_FT) );
	lex.setSymbol( CSymbol("STRING32_FT", STRING32_FT) );
	lex.setSymbol( CSymbol("STRING64_FT", STRING64_FT) );
	lex.setSymbol( CSymbol("ROWID_FT", ROWID_FT) );
}

//////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////
CParser::~CParser()
{
	IdList.RemoveAll();
	ValueMap.RemoveAll();
	Value.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// ProcessCommand
//////////////////////////////////////////////////////////////////////
BOOL CParser::ProcessCommand(CString& m_StrCmd)
{
	ERROR_CODE erc;
	BOOL result;

	if( !pDoc->m_Connected )
		pDoc->pPnPView->DisplayMessage("WARNING: no connection, process command only\n");

	switch( erc = ParseLine(m_StrCmd) )
	{
	case CREATE_OK:
	if( pDoc->m_Connected )
		m_Link.BuildCreateMessageAndSend(pDoc->pPnPView, ValueMap);
		result = TRUE;
		break;

	case GETDEF_OK:
	if( pDoc->m_Connected )
		m_Link.BuildGetDefMessageAndSend(pDoc->pPnPView, sTokenName, TRUE);
		result = TRUE;
		break;

	case ENUM_OK:
	if( pDoc->m_Connected )
		m_Link.BuildEnumMessageAndSend(pDoc->pPnPView, sTokenName);
		result = TRUE;
		break;

	case DEL_OK:
	if( pDoc->m_Connected )
		m_Link.BuildDeleteTableMessageAndSend(pDoc->pPnPView, sTokenName);
		result = TRUE;
		break;

	case INSERT_OK:
	if( pDoc->m_Connected )
		m_Link.BuildInsertMessageAndSend(pDoc->pPnPView, Value);
		result = TRUE;
		break;

	case RROW_OK:
	if( pDoc->m_Connected )
		m_Link.BuildReadRowMessageAndSend(pDoc->pPnPView, Value);
		result = TRUE;
		break;

	case DROW_OK:
	if( pDoc->m_Connected )
		m_Link.BuildDeleteRowMessageAndSend(pDoc->pPnPView, Value);
		result = TRUE;
		break;

	case MODROW_OK:
	if( pDoc->m_Connected )
		m_Link.BuildModifyRowMessageAndSend(pDoc->pPnPView, Value);
		result = TRUE;
		break;

	case MODFLD_OK:
	if( pDoc->m_Connected )
		m_Link.BuildModifyFieldMessageAndSend(pDoc->pPnPView, Value);
		result = TRUE;
		break;

	case LISTEN_OK:
	//if( pDoc->m_Connected )
	//	m_Link.BuildListenMessageAndSend(pDoc->pPnPView, Value);
		result = TRUE;
		break;

	case TABLES_OK:
	//if( pDoc->m_Connected )
	//	m_Link.BuildTablesMessageAndSend(pDoc->pPnPView, Value);
		result = TRUE;
		break;

	case INVALID:
		pDoc->pPnPView->DisplayMessage("Invalid command\n");
		result = FALSE;
		break;

	case VAR_NOT_FOUND:
		pDoc->pPnPView->DisplayMessage("Variable not found\n");
		result = FALSE;
		break;

	case UNREG_TOKEN:
		pDoc->pPnPView->DisplayMessage("Unrecognize command\n");
		result = FALSE;
		break;

	case MISSING_PARAM:
		pDoc->pPnPView->DisplayMessage("Not enough parameters\n");
		result = FALSE;
		break;
	}

	ValueMap.RemoveAll();
	Value.RemoveAll();
	return result;
}

//////////////////////////////////////////////////////////////////////
// ProcessCommand
//////////////////////////////////////////////////////////////////////
BOOL CParser::CompileLine(CString& m_StrCmd, int& line)
{
	ERROR_CODE erc;
	CString str;
	BOOL result = TRUE;

	str.Format("Line %d: ", line);
	switch( erc = ParseLine(m_StrCmd) )
	{
	case CREATE_OK:
	case GETDEF_OK:
	case ENUM_OK:
	case INSERT_OK:
	case DEL_OK:
	case RROW_OK:
	case DROW_OK:
	case MODROW_OK:
	case MODFLD_OK:
	case LISTEN_OK:
	case TABLES_OK:
		break;

	case INVALID:
		str += _T("Invalid command\n");
		pDoc->pPnPView->DisplayMessage(str);
		result = FALSE;
		break;

	case VAR_NOT_FOUND:
		str += _T("Variable not found\n");
		pDoc->pPnPView->DisplayMessage(str);
		result = FALSE;
		break;

	case UNREG_TOKEN:
		str += _T("Unrecognize command\n");
		pDoc->pPnPView->DisplayMessage(str);
		result = FALSE;
		break;

	case MISSING_PARAM:
		str += _T("Not enough parameters\n");
		pDoc->pPnPView->DisplayMessage(str);
		result = FALSE;
		break;
	}

	ValueMap.RemoveAll();
	Value.RemoveAll();
	return result;
}

//////////////////////////////////////////////////////////////////////
// ParseLine
//////////////////////////////////////////////////////////////////////
ERROR_CODE CParser::ParseLine(CString& m_StrCmd)
{
	CCtToken* pTokenValue;
	ERROR_CODE erc;
	BOOL GetDefState = FALSE;
	TokenNode _tokenNode;

	// Make sure the token list is empty
	if( !lex.IsSequenceEmpty() )
		lex.ClearAllTokens();

	// Give the CString to the lexical analyser
	lex.String2TokenSequence( m_StrCmd );

	// Start parsing
	do
	{
		// While current token is newlines
		while( lex.GetCurrentToken().getTokenType() == endlineT )
			lex.NextToken();	// Get next token

		switch( lex.GetCurrentToken().getTokenType() )
		{
		case getdefKeywordT:
		case enumKeywordT:
			if( lex.GetCurrentToken().getTokenType() == getdefKeywordT )
				GetDefState = TRUE;
			else
				GetDefState = FALSE;

			lex.NextToken();
			if( lex.GetCurrentToken().getTokenType() != idT &&
				lex.GetCurrentToken().getTokenType() != stringT )
				return erc =  INVALID;

			sTokenName = lex.GetCurrentToken().getTokenName();
			if( lex.GetCurrentToken().getTokenType() == idT )
			{
				if( IdList.Lookup(sTokenName, (CObject *&)pTokenValue) )
					sTokenName =  pTokenValue->getTokenName();
				else
				{
					lex.NextToken();
					if( lex.GetCurrentToken().getTokenType() == equalOperatorT )
					{
						lex.NextToken();
						IdList.SetAt(sTokenName, (CObject *)new CCtToken(lex.GetCurrentToken()));
						sTokenName = lex.GetCurrentToken().getTokenName();
					}
					else
						return erc = INVALID;
				}
			}
			//else?  This case is already taken of by sTokenName

			if( GetDefState )
				erc = GETDEF_OK;
			else
				erc = ENUM_OK;
			break;

		// Lis all the variable
		case listKeywordT:
			lex.NextToken();
			if( lex.GetCurrentToken().getTokenType() == idT ||
				lex.GetCurrentToken().getTokenType() == endfileT )
			{
				if( lex.GetCurrentToken().getTokenType() == endfileT )
				{
					// Print out all the variable
					POSITION pos = IdList.GetStartPosition();
					while( pos != NULL )
					{
						IdList.GetNextAssoc(pos, sTokenName, (CObject *&)pTokenValue);
						CString str;
						switch( pTokenValue->getTokenType() )
						{
						case intT:
							str.Format(sTokenName ," = %d\n", pTokenValue->getTokenInt());
							break;
						case realT:
							str.Format(sTokenName ," = %8.2f\n", pTokenValue->getTokenReal());
							break;
						case stringT:
							str = sTokenName + " = " + pTokenValue->getTokenName();
							break;
						}
						pDoc->pPnPView->DisplayMessage(str+"\n");
					}
					erc = LIST_OK;
				}
				else
				{
					sTokenName = lex.GetCurrentToken().getTokenName();
					if( IdList.Lookup(sTokenName, (CObject *&)pTokenValue) )
					{
						// Print out the variable
						CString str;
						switch( pTokenValue->getTokenType() )
						{
						case intT:
							str.Format(sTokenName ," = %d\n", pTokenValue->getTokenInt());
							break;
						case realT:
							str.Format(sTokenName ," = %8.2f\n", pTokenValue->getTokenReal());
							break;
						case stringT:
							str = sTokenName + " = " + pTokenValue->getTokenName();
							break;
						}
						pDoc->pPnPView->DisplayMessage(str+"\n");
					}
					else
						erc = VAR_NOT_FOUND;
				}
			}
			else
				return erc = INVALID;
			break;

		case createKeywordT:
			// TO DO: delete data from new CCtToken!!!
			lex.NextToken();
			if( lex.GetCurrentToken().getTokenType() != idT &&
				lex.GetCurrentToken().getTokenType() != stringT )
				return erc = INVALID;

			sTokenName = lex.GetCurrentToken().getTokenName();
			if( lex.GetCurrentToken().getTokenType() == idT )
			{
				if( IdList.Lookup(sTokenName, (CObject *&)pTokenValue) )
				{
					_tokenNode.name = pTokenValue->getTokenName();
					pTokenValue = new CCtToken(lex.GetCurrentToken());
					_tokenNode.token = *pTokenValue;
					ValueMap.AddHead(_tokenNode);
				}
				else
				{
					lex.NextToken();
					if( lex.GetCurrentToken().getTokenType() == equalOperatorT )
					{
						lex.NextToken();
						IdList.SetAt(sTokenName, (CObject *)new CCtToken(lex.GetCurrentToken()));
						_tokenNode.name = sTokenName;
						pTokenValue = new CCtToken(lex.GetCurrentToken());
						_tokenNode.token = *pTokenValue;
						ValueMap.AddHead(_tokenNode);
					}
					else
						return erc = INVALID;
				}
			}
			else  // This is the case when user type in a string for table name
			{
				_tokenNode.name = sTokenName;
				_tokenNode.token = lex.GetCurrentToken();
				ValueMap.AddHead(_tokenNode);
			}

			lex.NextToken();
			while( lex.GetCurrentToken().getTokenType() != endfileT )
			{
				sTokenName = lex.GetCurrentToken().getTokenName();
				if( IdList.Lookup(sTokenName, (CObject *&)pTokenValue) )
				{
					_tokenNode.name = pTokenValue->getTokenName();
					_tokenNode.token = *pTokenValue;
					ValueMap.AddTail(_tokenNode);
				}
				else
				{
					lex.NextToken();
					if( lex.GetCurrentToken().getTokenType() == equalOperatorT )
					{
						lex.NextToken();
						_tokenNode.name = sTokenName;
						pTokenValue = new CCtToken(lex.GetCurrentToken());
						_tokenNode.token = *pTokenValue;
						ValueMap.AddTail(_tokenNode);
					}
					else
						return erc = INVALID;
				}
				lex.NextToken();
			}
			if( ValueMap.GetCount() < 2 )
				erc =  MISSING_PARAM;
			else
				erc = CREATE_OK;
			break;

		case insertKeywordT:
			lex.NextToken();
			if( lex.GetCurrentToken().getTokenType() != idT &&
				lex.GetCurrentToken().getTokenType() != stringT )
				return erc =  INVALID;

			sTokenName = lex.GetCurrentToken().getTokenName();
			if( lex.GetCurrentToken().getTokenType() == idT )
			{
				if( IdList.Lookup(sTokenName, (CObject *&)pTokenValue) )
					Value.AddTail(*pTokenValue);
				else
				{
					lex.NextToken();
					if( lex.GetCurrentToken().getTokenType() == equalOperatorT )
					{
						lex.NextToken();
						IdList.SetAt(sTokenName, (CObject *)new CCtToken(lex.GetCurrentToken()));
						Value.AddTail( lex.GetCurrentToken() );
					}
					else
						return erc = INVALID;
				}
			}
			else
				Value.AddTail( lex.GetCurrentToken() );

			lex.NextToken();
			while( lex.GetCurrentToken().getTokenType() != endfileT )
			{
				Value.AddTail( lex.GetCurrentToken() );
				lex.NextToken();
			}

			if( Value.GetCount() < 2 )
				erc =  MISSING_PARAM;
			else
				erc = INSERT_OK;
			break;
			
		case deleteKeywordT:
			lex.NextToken();
			if( lex.GetCurrentToken().getTokenType() != idT &&
				lex.GetCurrentToken().getTokenType() != stringT )
				return erc =  INVALID;

			sTokenName = lex.GetCurrentToken().getTokenName();
			if( lex.GetCurrentToken().getTokenType() == idT )
			{
				if( IdList.Lookup(sTokenName, (CObject *&)pTokenValue) )
					sTokenName =  pTokenValue->getTokenName();
				else
				{
					lex.NextToken();
					if( lex.GetCurrentToken().getTokenType() == equalOperatorT )
					{
						lex.NextToken();
						IdList.SetAt(sTokenName, (CObject *)new CCtToken(lex.GetCurrentToken()));
						sTokenName = lex.GetCurrentToken().getTokenName();
					}
					else
						return erc = INVALID;
				}
			}
			//else?  This case is already taken of by sTokenName

			erc = DEL_OK;
			break;

		case deleterowKeywordT:
			lex.NextToken();			// Table name
			if( lex.GetCurrentToken().getTokenType() != idT &&
				lex.GetCurrentToken().getTokenType() != stringT )
				return erc =  INVALID;

			sTokenName = lex.GetCurrentToken().getTokenName();
			if( lex.GetCurrentToken().getTokenType() == idT )
			{
				if( IdList.Lookup(sTokenName, (CObject *&)pTokenValue) )
					Value.AddTail(*pTokenValue);
				else
				{
					lex.NextToken();
					if( lex.GetCurrentToken().getTokenType() == equalOperatorT )
					{
						lex.NextToken();
						IdList.SetAt(sTokenName, (CObject *)new CCtToken(lex.GetCurrentToken()));
						Value.AddTail( lex.GetCurrentToken() );
					}
					else
						return erc = INVALID;
				}
			}
			else
				Value.AddTail( lex.GetCurrentToken() );

			lex.NextToken();			// KeyFieldName
			if( lex.GetCurrentToken().getTokenType() != stringT )
				return erc =  INVALID;
			else
				Value.AddTail( lex.GetCurrentToken() );

			lex.NextToken();
			if( lex.GetCurrentToken().getTokenType() == equalOperatorT )
			{
				lex.NextToken();			// KeyFieldValue
				Value.AddTail( lex.GetCurrentToken() );
			}
			else
				return erc = INVALID;

			if( Value.GetCount() < 3 )
				erc =  MISSING_PARAM;
			else
				erc = DROW_OK;
			break;

		case readrowKeywordT:
			lex.NextToken();			// Table name
			if( lex.GetCurrentToken().getTokenType() != idT &&
				lex.GetCurrentToken().getTokenType() != stringT )
				return erc =  INVALID;

			sTokenName = lex.GetCurrentToken().getTokenName();
			if( lex.GetCurrentToken().getTokenType() == idT )
			{
				if( IdList.Lookup(sTokenName, (CObject *&)pTokenValue) )
					Value.AddTail(*pTokenValue);
				else
				{
					lex.NextToken();
					if( lex.GetCurrentToken().getTokenType() == equalOperatorT )
					{
						lex.NextToken();
						IdList.SetAt(sTokenName, (CObject *)new CCtToken(lex.GetCurrentToken()));
						Value.AddTail( lex.GetCurrentToken() );
					}
					else
						return erc = INVALID;
				}
			}
			else
				Value.AddTail( lex.GetCurrentToken() );

			lex.NextToken();			// KeyFieldName
			if( lex.GetCurrentToken().getTokenType() != stringT )
				return erc =  INVALID;
			else
				Value.AddTail( lex.GetCurrentToken() );

			lex.NextToken();
			if( lex.GetCurrentToken().getTokenType() == equalOperatorT )
			{
				lex.NextToken();			// KeyFieldValue
				Value.AddTail( lex.GetCurrentToken() );
			}
			else
				return erc = INVALID;

			if( Value.GetCount() < 3 )
				erc =  MISSING_PARAM;
			else
				erc = RROW_OK;
			break;

		case modifyrowKeywordT:
			lex.NextToken();			// Table name
			if( lex.GetCurrentToken().getTokenType() != idT &&
				lex.GetCurrentToken().getTokenType() != stringT )
				return erc =  INVALID;

			sTokenName = lex.GetCurrentToken().getTokenName();
			if( lex.GetCurrentToken().getTokenType() == idT )
			{
				if( IdList.Lookup(sTokenName, (CObject *&)pTokenValue) )
					Value.AddTail(*pTokenValue);
				else
				{
					lex.NextToken();
					if( lex.GetCurrentToken().getTokenType() == equalOperatorT )
					{
						lex.NextToken();
						IdList.SetAt(sTokenName, (CObject *)new CCtToken(lex.GetCurrentToken()));
						Value.AddTail( lex.GetCurrentToken() );
					}
					else
						return erc = INVALID;
				}
			}
			else
				Value.AddTail( lex.GetCurrentToken() );

			lex.NextToken();			// KeyFieldName
			if( lex.GetCurrentToken().getTokenType() != stringT )
				return erc =  INVALID;
			else
				Value.AddTail( lex.GetCurrentToken() );

			lex.NextToken();
			if( lex.GetCurrentToken().getTokenType() == equalOperatorT )
			{
				lex.NextToken();			// KeyFieldValue
				Value.AddTail( lex.GetCurrentToken() );
			}
			else
				return erc = INVALID;

			lex.NextToken();
			while( lex.GetCurrentToken().getTokenType() != endfileT )
			{
				Value.AddTail( lex.GetCurrentToken() );
				lex.NextToken();
			}

			if( Value.GetCount() < 4 )
				erc =  MISSING_PARAM;
			else
				erc = MODROW_OK;
			break;

		case modifyfieldKeywordT:
			lex.NextToken();			// Table name
			if( lex.GetCurrentToken().getTokenType() != idT &&
				lex.GetCurrentToken().getTokenType() != stringT )
				return erc =  INVALID;

			sTokenName = lex.GetCurrentToken().getTokenName();
			if( lex.GetCurrentToken().getTokenType() == idT )
			{
				if( IdList.Lookup(sTokenName, (CObject *&)pTokenValue) )
					Value.AddTail(*pTokenValue);
				else
				{
					lex.NextToken();
					if( lex.GetCurrentToken().getTokenType() == equalOperatorT )
					{
						lex.NextToken();
						IdList.SetAt(sTokenName, (CObject *)new CCtToken(lex.GetCurrentToken()));
						Value.AddTail( lex.GetCurrentToken() );
					}
					else
						return erc = INVALID;
				}
			}
			else
				Value.AddTail( lex.GetCurrentToken() );

			lex.NextToken();			// KeyFieldName
			if( lex.GetCurrentToken().getTokenType() != stringT )
				return erc =  INVALID;
			else
				Value.AddTail( lex.GetCurrentToken() );

			lex.NextToken();
			if( lex.GetCurrentToken().getTokenType() == equalOperatorT )
			{
				lex.NextToken();			// KeyFieldValue
				Value.AddTail( lex.GetCurrentToken() );
			}
			else
				return erc = INVALID;

			lex.NextToken();			// FieldName
			if( lex.GetCurrentToken().getTokenType() != stringT )
				return erc =  INVALID;
			else
				Value.AddTail( lex.GetCurrentToken() );

			lex.NextToken();
			if( lex.GetCurrentToken().getTokenType() == equalOperatorT )
			{
				lex.NextToken();			// FieldValue
				Value.AddTail( lex.GetCurrentToken() );
			}
			else
				return erc = INVALID;

			if( Value.GetCount() < 5 )
				erc =  MISSING_PARAM;
			else
				erc = MODFLD_OK;
			break;

		case listenKeywordT:
			lex.NextToken();			// Table name
			if( lex.GetCurrentToken().getTokenType() != idT &&
				lex.GetCurrentToken().getTokenType() != stringT )
				return erc =  INVALID;
			erc = INVALID;
			lex.NextToken();
			break;

		case stoplistenKeywordT:
			lex.NextToken();			// Table name
			if( lex.GetCurrentToken().getTokenType() != idT &&
				lex.GetCurrentToken().getTokenType() != stringT )
				return erc =  INVALID;
			erc = INVALID;
			lex.NextToken();
			break;

		case tablesKeywordT:
			erc = INVALID;
			break;

		case storeKeywordT:
			erc = INVALID;
			break;

		case downloadKeywordT:
			erc = INVALID;
			break;

		case runKeywordT:
			erc = INVALID;
			break;

		case idT:
			sTokenName = lex.GetCurrentToken().getTokenName();

			if( IdList.Lookup(sTokenName, (CObject *&)pTokenValue) )
			{
				_tokenNode.name = pTokenValue->getTokenName();
				_tokenNode.token = *pTokenValue;
				ValueMap.AddTail(_tokenNode);
			}
			else
			{
				lex.NextToken();
				if( lex.GetCurrentToken().getTokenType() == equalOperatorT )
				{
					lex.NextToken();
					CString strTemp = lex.GetCurrentToken().getTokenName();
					IdList.SetAt(sTokenName, (CObject *)new CCtToken(lex.GetCurrentToken()));
				}
				else
					return erc = INVALID;
			}
			break;

		case startrowKeywordT:
		case persistentKeywordT:
		case rowIDKeywordT:
		case tableIDKeywordT:
		case HiPartKeywordT:
		case LoPartKeywordT:
		case BINARY_FT:
		case S32_FT:
		case U32_FT:
		case S64_FT:
		case U64_FT:
		case STRING16_FT:
		case STRING32_FT:
		case STRING64_FT:
		case ROWID_FT:
		case equalOperatorT:
		case intT:
		case realT:
		case stringT:
			erc = INVALID;
			break;

		case helpKeywordT:
			pDoc->pPnPView->DisplayMessage("create - create a new table\n");
			pDoc->pPnPView->DisplayMessage("create <\"TableName\"> <\"FieldName\"=FieldType> [<\"FieldName\"=FieldType> ...]\n");
			pDoc->pPnPView->DisplayMessage("\n");
			pDoc->pPnPView->DisplayMessage("insert - insert a row into a table\n");
			pDoc->pPnPView->DisplayMessage("insert <\"TableName\"> <value> [<value> ...]\n");
			pDoc->pPnPView->DisplayMessage("\n");
			pDoc->pPnPView->DisplayMessage("getdef - get the structure of a table\n");
			pDoc->pPnPView->DisplayMessage("getdef <\"TableName\">\n");
			pDoc->pPnPView->DisplayMessage("\n");
			pDoc->pPnPView->DisplayMessage("enum - list the content of the table\n");
			pDoc->pPnPView->DisplayMessage("enum <\"TableName\">\n");
			pDoc->pPnPView->DisplayMessage("\n");
			pDoc->pPnPView->DisplayMessage("list - list one or all variables\n");
			pDoc->pPnPView->DisplayMessage("list [variable]\n");
			pDoc->pPnPView->DisplayMessage("\n");
			pDoc->pPnPView->DisplayMessage("delete - delete table including its contents\n");
			pDoc->pPnPView->DisplayMessage("delete <\"TableName\">\n");
			pDoc->pPnPView->DisplayMessage("\n");
			pDoc->pPnPView->DisplayMessage("readrow - read the content of rows that matches the field value\n");
			pDoc->pPnPView->DisplayMessage("readrow <\"TableName\"> <\"FieldName\"=value>\n");
			pDoc->pPnPView->DisplayMessage("\n");
			pDoc->pPnPView->DisplayMessage("deleterow - delete the specified rows that matches the field value\n");
			pDoc->pPnPView->DisplayMessage("deleterow <\"TableName\"> <\"FieldName\"=value>\n");
			pDoc->pPnPView->DisplayMessage("\n");
			pDoc->pPnPView->DisplayMessage("modifyrow - modify the content of rows that matches the field value\n");
			pDoc->pPnPView->DisplayMessage("modifyrow <\"TableName\"> <\"FieldName\"=value> <value> [<value> ...]\n");
			pDoc->pPnPView->DisplayMessage("\n");
			pDoc->pPnPView->DisplayMessage("modifyfield - modify the content of a field\n");
			pDoc->pPnPView->DisplayMessage("modifyfield <\"TableName\"> <\"FieldName\"=value> <value>\n");

			//pDoc->pPnPView->DisplayMessage("listen - listen\n");
			//pDoc->pPnPView->DisplayMessage("listen <\"TableName\"> [variable]\n");
			//pDoc->pPnPView->DisplayMessage("tables - list all tables\n");
			//pDoc->pPnPView->DisplayMessage("tables\n");
			//pDoc->pPnPView->DisplayMessage("store - \n");
			//pDoc->pPnPView->DisplayMessage("store [variable]\n");
			//pDoc->pPnPView->DisplayMessage("download - \n");
			//pDoc->pPnPView->DisplayMessage("download [variable]\n");
			//pDoc->pPnPView->DisplayMessage("run - \n");
			//pDoc->pPnPView->DisplayMessage("run\n");
			break;

		// Unrecognize token
		case errorT:
			// Invalid token, report error
			erc = UNREG_TOKEN;
		}

		// Get next token
		lex.NextToken();
	} while( lex.GetCurrentToken().getTokenType() != endfileT && lex.GetCurrentToken().getTokenType() != nilT );

	lex.ClearAllTokens();
	return erc;
}

BOOL CParser::Connect()
{
	return m_Link.BuildConnectMessageAndSend(pDoc->pPnPView);
}
// End of file