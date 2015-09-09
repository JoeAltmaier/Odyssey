#include "stdafx.h"

#include "PeeknPoke.h"
#include "PeeknPokeDoc.h"
#include "PeeknPokeView.h"
#include "DataLink.h"
#include "LexicalAnalyser.h"

////////////////////////////////////
// BuildCreateMessage
////////////////////////////////////
BOOL CDataLink::BuildCreateMessageAndSend(CView *pView, CList<TokenNode, TokenNode&>& List)
{
	SP_PAYLOAD spPayload;
	CCtToken* pToken;
	POSITION pos= List.GetHeadPosition();
	fieldDef* pFielDefs = new fieldDef[List.GetCount()-1];
	int cEntries = 0;
	char* pData = NULL;
	TokenNode _tokenNode;
	DWORD signature = (DWORD)pView;
	CString str;	// Required! Since the pToken->getTokenName() return NULL

	_tokenNode = List.GetNext( pos );
	pToken = &_tokenNode.token;

	spPayload.cmd = CREATE_TABLE;
	spPayload.chID = (long)pView;
	str = pToken->getTokenName();
	strcpy(spPayload.Data.ct.TableName, str);

	// Iterate through the list
	while( pos != NULL )
	{
		_tokenNode = List.GetNext( pos );
		pToken = &_tokenNode.token;
		str = pToken->getTokenName();
		strcpy(pFielDefs[cEntries].name, str);
		pFielDefs[cEntries].cbField = pToken->getTokenName().GetLength();

		// Convert from S32FT, U32_FT, ... to fieldType
		switch( pToken->getTokenType() )
		{
		case BINARY_FT:
			pFielDefs[cEntries].cbField = 1;
			pFielDefs[cEntries].iFieldType = BINARY_FT;
			break;
		case S32_FT:
			pFielDefs[cEntries].cbField = 4;
			pFielDefs[cEntries].iFieldType = S32_FT;
			break;
		case U32_FT:
			pFielDefs[cEntries].cbField = 4;
			pFielDefs[cEntries].iFieldType = U32_FT;
			break;
		case S64_FT:
			pFielDefs[cEntries].cbField = 8;
			pFielDefs[cEntries].iFieldType = S64_FT;
			break;
		case U64_FT:
			pFielDefs[cEntries].cbField = 8;
			pFielDefs[cEntries].iFieldType = U64_FT;
			break;
		case STRING16_FT:
			pFielDefs[cEntries].cbField = 16;
			pFielDefs[cEntries].iFieldType = STRING16_FT;
			break;
		case STRING32_FT:
			pFielDefs[cEntries].cbField = 32;
			pFielDefs[cEntries].iFieldType = STRING32_FT;
			break;
		case STRING64_FT:
			pFielDefs[cEntries].cbField = 64;
			pFielDefs[cEntries].iFieldType = STRING64_FT;
			break;
		case ROWID_FT:
			pFielDefs[cEntries].cbField = 8;
			pFielDefs[cEntries].iFieldType = ROWID_FT;
			break;
		default:
			pDoc->pPnPView->DisplayMessage("BuildCreateMessageAndSend - Invalid field type\n");
			if( pData )
				delete pData;
			delete [] pFielDefs;
			return FALSE;
		} // switch

		pFielDefs[cEntries].persistFlags = TRUE;
		cEntries++;
	} // while

	spPayload.Data.ct.cbFieldDefs = sizeof(fieldDef)*(List.GetCount()-1);
	spPayload.Data.ct.numFieldDefs = cEntries;
	spPayload.Data.ct.cEntriesRsv = 1;
	spPayload.Data.ct.persistent = TRUE;
	spPayload.Data.ct.cRow = 1;
	spPayload.cbData = sizeof( spPayload ) + spPayload.Data.ct.cbFieldDefs;

	// Make a new buffer to send
	pData = new char[spPayload.cbData];
	memmove( pData, &spPayload, sizeof(spPayload) );
	memmove( pData+sizeof(spPayload), pFielDefs, spPayload.Data.ct.cbFieldDefs );

	// Send to serial port and wait for data back
	if( pDoc->m_Port.WriteABuffer( pData, spPayload.cbData ) )
	{
		if( pData )
			delete pData;
		if( pFielDefs )
			delete [] pFielDefs;
		return ReceiveCreateData(H_TO_N(signature));
	}
	else
	{
		if( pData )
			delete pData;
		if( pFielDefs )
			delete [] pFielDefs;
		pDoc->pPnPView->DisplayMessage("BuildCreateMessageAndSend -  Error writing to port\n");
		return FALSE;
	}
}

////////////////////////////////////
// ReceiveCreateData
////////////////////////////////////
BOOL CDataLink::ReceiveCreateData(DWORD signature)
{
	SP_REPLY_PAYLOAD spReply;
	signed long cbRead = sizeof(SP_REPLY_PAYLOAD);
	char str[256];

	if( pDoc->m_Port.ReadPort(&spReply, &cbRead, signature) )
	{
		switch( spReply.ErrorCode )
		{
		case ercOK:
			sprintf(str, "TableID = %#04x\n", spReply.Data.ctR.Table);
			break;
		case ercTableExists:
			sprintf(str, "Table exists - TableID = %#04x\n", spReply.Data.ctR.Table);
			break;
		case ercNoMoreHeap:
			sprintf(str, "No more heap\n");
			break;
		case ercBadParameter:
			sprintf(str, "Bad parameter\n");
			break;
		case ercNoFreeHeaders:
			sprintf(str, "No free headers\n");
			break;
		default:
			sprintf(str, "Unknown error, error code = %d\n", spReply.ErrorCode);
			break;
		}
		pDoc->pPnPView->DisplayMessage(_T(str));
		return TRUE;
	}
	else
	{
		pDoc->pPnPView->DisplayMessage("ReceiveCreateData - Read port error!\n");
		return FALSE;
	}

}

////////////////////////////////////
// BuildInsertMessage
////////////////////////////////////
BOOL CDataLink::BuildInsertMessageAndSend(CView *pView, CList<CCtToken, CCtToken&>& List)
{
	SP_PAYLOAD spPayload;
	char* pData;
	CCtToken Token = List.GetHead();
	POSITION pos = List.GetHeadPosition();
	DWORD signature = (DWORD)pView;

	spPayload.cmd = INSERT_ROW;
	spPayload.chID = (long)pView;
	strcpy(spPayload.Data.in.TableName, Token.getTokenName());

	BuildGetDefMessageAndSend(pView, Token.getTokenName(), FALSE);
	if( m_spReply.ErrorCode != ercOK )
	{
		char str[256];
		switch( m_spReply.ErrorCode )
		{
		case ercTableNotfound:
			pDoc->pPnPView->DisplayMessage("BuildInsertMessageAndSend - Table not found\n");
			break;
		case ercNoMoreHeap:
			pDoc->pPnPView->DisplayMessage("BuildInsertMessageAndSend - No more heap\n");
			break;
		case ercBadParameter:
			pDoc->pPnPView->DisplayMessage("BuildInsertMessageAndSend - Bad parameter\n");
			break;
		default:
			sprintf(str, "BuildInsertMessageAndSend - Unknown error, error code = %d\n", m_spReply.ErrorCode);
			pDoc->pPnPView->DisplayMessage(str);
			break;
		}
		return FALSE;
	}

	// Make a new buffer to send
	pData = new char[sizeof(spPayload)+m_spReply.Data.gtR.cbRowRet];

	unsigned long	ulNumber;
	__int64 i64Number;
	double	dTemp;
	rowID	rID;
	CString str;
	int		i = 1;	// Skip the first field of pFieldDefsRet which is rid
	spPayload.Data.in.cbRowData = sizeof(rowID);

	// Iterate through the list and copy data, skip the first parameter
	pos = List.GetHeadPosition();
	for( List.GetNext(pos); pos != NULL; i++)
	{
		Token = List.GetNext( pos );
		switch( Token.getTokenType() )
		{
		case intT:
			switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
			{
			case S32_FT:
			case U32_FT:
				ulNumber = Token.getTokenUL();
				ulNumber = N_TO_H(ulNumber);
				memcpy(pData+sizeof(spPayload)+spPayload.Data.in.cbRowData, &ulNumber,
					m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
				break;
			case S64_FT:
			case U64_FT:
				i64Number = Token.getTokenI64();
				memcpy(pData+sizeof(spPayload)+spPayload.Data.in.cbRowData, &i64Number,
					m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
				break;
			default:	// Field mismatch
				pDoc->pPnPView->DisplayMessage("BuildInsertMessageAndSend - Field mismatch\n");
				if( m_spReply.Data.gtR.pFieldDefsRet )
				{
					delete m_spReply.Data.gtR.pFieldDefsRet;
					SetPayloadNull();
				}
				if( pData )
					delete pData;
				return FALSE;
			}
			spPayload.Data.in.cbRowData += m_spReply.Data.gtR.pFieldDefsRet[i].cbField;
			break;
		case sintT:
			switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
			{
			case S32_FT:
				ulNumber = Token.getTokenUL();
				ulNumber = H_TO_N(ulNumber);
				memcpy(pData+sizeof(spPayload)+spPayload.Data.in.cbRowData, &ulNumber,
					m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
				break;
			case S64_FT:
				i64Number = Token.getTokenI64();
				memcpy(pData+sizeof(spPayload)+spPayload.Data.in.cbRowData, &i64Number,
					m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
				break;
			default:	// Field mismatch
				pDoc->pPnPView->DisplayMessage("BuildInsertMessageAndSend - Field mismatch\n");
				if( m_spReply.Data.gtR.pFieldDefsRet )
				{
					delete m_spReply.Data.gtR.pFieldDefsRet;
					SetPayloadNull();
				}
				if( pData )
					delete pData;
				return FALSE;
			}
			spPayload.Data.in.cbRowData += m_spReply.Data.gtR.pFieldDefsRet[i].cbField;
			break;
		case I64T:
			switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
			{
			case S64_FT:
			case U64_FT:
				i64Number = Token.getTokenI64();
				memcpy(pData+sizeof(spPayload)+spPayload.Data.in.cbRowData, &i64Number,
					m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
				break;
			default:	// Field mismatch
				pDoc->pPnPView->DisplayMessage("BuildInsertMessageAndSend - Field mismatch\n");
				if( m_spReply.Data.gtR.pFieldDefsRet )
				{
					delete m_spReply.Data.gtR.pFieldDefsRet;
					SetPayloadNull();
				}
				if( pData )
					delete pData;
				return FALSE;
			}
			spPayload.Data.in.cbRowData += m_spReply.Data.gtR.pFieldDefsRet[i].cbField;
			break;
		case realT:
			dTemp = Token.getTokenReal();
			memcpy(pData+sizeof(spPayload)+spPayload.Data.in.cbRowData, &dTemp, sizeof(double));
			spPayload.Data.in.cbRowData += sizeof(double);
			break;
		case rowIDT:
			rID = Token.getTokenRowID();
			rID.Table = H_TO_NS(rID.Table);
			rID.HiPart = H_TO_NS(rID.HiPart);
			rID.LoPart = H_TO_N(rID.LoPart);
			memcpy(pData+sizeof(spPayload)+spPayload.Data.in.cbRowData, &rID, sizeof(rowID));
			spPayload.Data.in.cbRowData += sizeof(rowID);
			break;
		case stringT:
			switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
			{
			case STRING16_FT:
				str.Format("%-15s", Token.getTokenName());
				break;
			case STRING32_FT:
				str.Format("%-31s", Token.getTokenName());
				break;
			case STRING64_FT:
				str.Format("%-63s", Token.getTokenName());
				break;
			default:	// Field mismatch
				pDoc->pPnPView->DisplayMessage("BuildInsertMessageAndSend - Field mismatch\n");
				if( m_spReply.Data.gtR.pFieldDefsRet )
				{
					delete m_spReply.Data.gtR.pFieldDefsRet;
					SetPayloadNull();
				}
				if( pData )
					delete pData;
				return FALSE;
			}
			str += '\0';
			memcpy(pData+sizeof(spPayload)+spPayload.Data.in.cbRowData, str, str.GetLength() );
			spPayload.Data.in.cbRowData += str.GetLength();
			break;
		default:
			pDoc->pPnPView->DisplayMessage("BuildInsertMessageAndSend - Invalid field type\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
	}

	if( spPayload.Data.in.cbRowData != m_spReply.Data.gtR.cbRowRet )
	{
		pDoc->pPnPView->DisplayMessage("BuildInsertMessageAndSend - Row size mismatch\n");
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		delete pData;
		return FALSE;
	}

	spPayload.cbData = sizeof(spPayload) + spPayload.Data.in.cbRowData;

	memmove(pData, &spPayload, sizeof(spPayload));

	// Send to serial port and wait for data back
	if( pDoc->m_Port.WriteABuffer( pData, spPayload.cbData ) )
	{
		delete pData;
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		return ReceiveInsertData(H_TO_N(signature));
	}
	else
	{
		delete pData;
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		pDoc->pPnPView->DisplayMessage("BuildInsertMessageAndSend - Error writing to port\n");
		return FALSE;
	}
}

////////////////////////////////////
// ReceiveInsertData
////////////////////////////////////
BOOL CDataLink::ReceiveInsertData(DWORD signature)
{
	SP_REPLY_PAYLOAD spReply;
	signed long cbRead = sizeof(SP_REPLY_PAYLOAD);
	char str[256];

	if( pDoc->m_Port.ReadPort(&spReply, &cbRead, signature) )
	{
		switch( spReply.ErrorCode )
		{
		case ercOK:
			sprintf(str, "rowID.Table = %#04x   rowID.HiPart = %#04x   rowID.LoPart = %#08x\n",
					spReply.Data.inR.Table, spReply.Data.inR.HiPart, spReply.Data.inR.LoPart);
			break;
		case ercNoModRowId:
			sprintf(str, "No more rowID available\n");
			break;
		case ercNoMoreHeap:
			sprintf(str, "No more heap\n");
			break;
		case ercBadParameter:
			sprintf(str, "Bad parameter\n");
			break;
		default:
			sprintf(str, "Unknown error, error code = %d\n", spReply.ErrorCode);
			break;
		}

		pDoc->pPnPView->DisplayMessage(_T(str));
		return TRUE;
	}
	else
	{
		pDoc->pPnPView->DisplayMessage("ReceiveInsertData - Read port error!\n");
		return FALSE;
	}
}

////////////////////////////////////
// BuildGetDefMessage
////////////////////////////////////
BOOL CDataLink::BuildGetDefMessageAndSend(CView *pView, CString& tableName, BOOL DisplayData)
{
	SP_PAYLOAD spPayload;
	DWORD signature = (DWORD)pView;

	if( tableName.GetLength() == 0 )
		return FALSE;

	spPayload.cmd = GET_TABLE_DEF;
	spPayload.chID = (long)pView;

	spPayload.cbData = sizeof( spPayload );
	strcpy(spPayload.Data.gt.TableName, tableName);
	spPayload.Data.gt.FieldDefRetMax = sizeof(fieldDef)*16;	// this is hardcode value, need to change later

	// Send to serial port and wait for data back
	if( pDoc->m_Port.WriteABuffer( &spPayload, spPayload.cbData ) )
		return ReceiveGetDefData(H_TO_N(signature), DisplayData);
	else
	{
		pDoc->pPnPView->DisplayMessage("BuildGetDefMessageAndSend - Error writing to port\n");
		m_spReply.ErrorCode = 255;
		return FALSE;
	}
}

////////////////////////////////////
// ReceiveGetDefData
////////////////////////////////////
BOOL CDataLink::ReceiveGetDefData(DWORD signature, BOOL DisplayData)
{
	SP_REPLY_PAYLOAD spReply;
	signed long cbRead = sizeof(SP_REPLY_PAYLOAD);

	// TO DO: need to validate returned data!
	if( pDoc->m_Port.ReadPort(&spReply, &cbRead, signature) )
	{
		if( !DisplayData )
		{
			if( spReply.ErrorCode != ercOK )
				return FALSE;
			m_spReply = spReply;
			return TRUE;
		}

		unsigned long i;
		char str[256];
		CString tab;
		CString FieldName = "Field name\t";
		CString FieldType = "Field Type\t";
		CString FieldSize = "Field size\t";

		switch( spReply.ErrorCode )
		{
		case ercOK:
			if( spReply.Data.gtR.pFieldDefsRet == NULL )
			{
				pDoc->pPnPView->DisplayMessage("ReceiveGetDefData - Can not obtain field definition of the table\n");
				return FALSE;
			}

			for( i = 0; i < spReply.Data.gtR.numFieldDefsRet; i++)
			{
				FieldName += spReply.Data.gtR.pFieldDefsRet[i].name;
				if( strlen(spReply.Data.gtR.pFieldDefsRet[i].name) < 8 )
				{
					FieldName += "\t\t";
					tab = "\t\t";
				}
				else
				{
					FieldName += "\t";
					tab = "\t";
				}
				switch( spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
				{
				case BINARY_FT:
					FieldType += "BINARY_FT\t";
					FieldSize += "1\t\t";
					break;
				case S32_FT:
					FieldType += "S32_FT\t\t";
					FieldSize += "4\t\t";
					break;
				case U32_FT:
					FieldType += "U32_FT\t\t";
					FieldSize += "4\t\t";
					break;
				case S64_FT:
					FieldType += "S64_FT\t\t";
					FieldSize += "8\t\t";
					break;
				case U64_FT:
					FieldType += "U64_FT\t\t";
					FieldSize += "8\t\t";
					break;
				case STRING16_FT:
					FieldType += "STRING16_FT\t";
					FieldSize += "16\t\t";
					break;
				case STRING32_FT:
					FieldType += "STRING32_FT\t";
					FieldSize += "32\t\t";
					break;
				case STRING64_FT:
					FieldType += "STRING64_FT\t";
					FieldSize += "64\t\t";
					break;
				case ROWID_FT:
					FieldType += "ROWID_FT\t";
					FieldSize += "8\t\t";
					break;
				}
			}

			pDoc->pPnPView->DisplayMessage(FieldName+"\n");
			pDoc->pPnPView->DisplayMessage(FieldType+"\n");
			pDoc->pPnPView->DisplayMessage(FieldSize+"\n");
			sprintf( str, "Number of rows in the table = %d\n", spReply.Data.gtR.cRowsRet);
			pDoc->pPnPView->DisplayMessage("\n");
			pDoc->pPnPView->DisplayMessage(_T(str));
			sprintf( str, "Persistent = %s\n", spReply.Data.gtR.persistent? "YES" : "NO");
			pDoc->pPnPView->DisplayMessage(_T(str));
			
			delete [] spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
			break;
		case ercTableNotfound:
			pDoc->pPnPView->DisplayMessage("Table not found\n");
			break;
		case ercNoMoreHeap:
			pDoc->pPnPView->DisplayMessage("No more heap\n");
			break;
		case ercBadParameter:
			pDoc->pPnPView->DisplayMessage("Bad parameter\n");
			break;
		default:
			sprintf(str, "Unknown error, error code = %d\n", spReply.ErrorCode);
			pDoc->pPnPView->DisplayMessage(str);
			break;
		}

		return TRUE;
	}
	else
	{
		pDoc->pPnPView->DisplayMessage("ReceiveGetDefData - Read port error!\n");
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete [] m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		m_spReply.ErrorCode = 256;
		return FALSE;
	}
}

////////////////////////////////////
// BuildEnumMessage
////////////////////////////////////
BOOL CDataLink::BuildEnumMessageAndSend(CView *pView, CString& tableName)
{
	SP_PAYLOAD spPayload;
	DWORD signature = (DWORD)pView;

	if( tableName.GetLength() == 0 )
		return FALSE;

	// Get the table definition first
	BuildGetDefMessageAndSend(pView, tableName, FALSE);
	if( m_spReply.ErrorCode != ercOK )
	{
		char str[256];
		switch( m_spReply.ErrorCode )
		{
		case ercTableNotfound:
			pDoc->pPnPView->DisplayMessage("BuildEnumMessageAndSend - Table not found\n");
			break;
		case ercNoMoreHeap:
			pDoc->pPnPView->DisplayMessage("BuildEnumMessageAndSend - No more heap\n");
			break;
		case ercBadParameter:
			pDoc->pPnPView->DisplayMessage("BuildEnumMessageAndSend - Bad parameter\n");
			break;
		default:
			sprintf(str, "BuildEnumMessageAndSend - Unknown error, error code = %d\n", m_spReply.ErrorCode);
			pDoc->pPnPView->DisplayMessage(str);
			break;
		}
		return FALSE;
	}

	spPayload.cmd = ENUM_TABLE;
	spPayload.chID = (long)pView;

	strcpy( spPayload.Data.et.TableName, tableName );
	spPayload.Data.et.startRow = 0;
	spPayload.Data.et.cbDataRetMax = m_spReply.Data.gtR.cbRowRet * m_spReply.Data.gtR.cRowsRet;  // LATER - minus the start row
	spPayload.cbData = sizeof( spPayload );

	// Send to serial port and wait for data back
	if( pDoc->m_Port.WriteABuffer(&spPayload, spPayload.cbData) )
		return ReceiveEnumData(H_TO_N(signature));
	else
	{
		pDoc->pPnPView->DisplayMessage("BuildEnumMessageAndSend - Error writing to port\n");
		return FALSE;
	}
}

////////////////////////////////////
// ReceiveEnumData
////////////////////////////////////
BOOL CDataLink::ReceiveEnumData(DWORD signature)
{
	SP_REPLY_PAYLOAD spReply;
	signed long cbRead = sizeof(SP_REPLY_PAYLOAD);
	CString str, Line;
	unsigned int i, index, j;
	char *pData;
	__int64 *i64Num;
	unsigned long *upNum;
	signed long *spNum;
	rowID *rID;

	// TO DO: need to validate returned data!
	if( pDoc->m_Port.ReadPort(&spReply, &cbRead, signature) )
	{
		switch( spReply.ErrorCode )
		{
		case ercOK:
			if( spReply.Data.etR.pRowsDataRet == NULL )
			{
				if( m_spReply.Data.gtR.pFieldDefsRet )
				{
					delete [] m_spReply.Data.gtR.pFieldDefsRet;
					SetPayloadNull();
				}
				pDoc->pPnPView->DisplayMessage("No data (ercOK)\n");
				return TRUE;
			}

			// Display the header
			for( i = 0; i < m_spReply.Data.gtR.numFieldDefsRet; i++)
			{
				str.Format("%s\t\t", m_spReply.Data.gtR.pFieldDefsRet[i].name);
				Line += str;
			}
			pDoc->pPnPView->DisplayMessage(Line);

			str.Empty();
			Line.Empty();
			// pData hold one record at a time
			pData = new char[m_spReply.Data.gtR.cbRowRet];
			// Display the record
			for( i = 0; i < m_spReply.Data.gtR.cRowsRet; i++ )
			{
				memcpy(pData, (char *)spReply.Data.etR.pRowsDataRet+m_spReply.Data.gtR.cbRowRet*i,
					m_spReply.Data.gtR.cbRowRet);
				index = 0;
				for( j = 0; j < m_spReply.Data.gtR.numFieldDefsRet; j++ )
				{
					switch( m_spReply.Data.gtR.pFieldDefsRet[j].iFieldType )
					{
					case BINARY_FT:
						str.Format( "%#016x\t\t", *((char *)pData));
						break;
					case S32_FT:
						spNum = (signed long *)(pData+index);
						*spNum = N_TO_H((unsigned long)*spNum);
						str.Format( "%-16ld\t", *spNum);
						break;
					case U32_FT:
						upNum = (unsigned long *)(pData+index);
						*upNum = N_TO_H(*upNum);
						str.Format( "%-16u\t", *upNum);
						break;
					case S64_FT:
						i64Num = (__int64 *)(pData+index);
						str.Format( "%-26ld\t", *i64Num);
						break;
					case U64_FT:
						i64Num = (__int64 *)(pData+index);
						str.Format( "%-26u\t", *i64Num);
						break;
					case STRING16_FT:
						str.Format( "%-16.16s\t", (pData+index));
						break;
					case STRING32_FT:
						str.Format( "%-32.32s\t", (pData+index));
						break;
					case STRING64_FT:
						str.Format( "%-64.64s\t", (pData+index));
						break;
					case ROWID_FT:
						rID = (rowID *)(pData+index);
						rID->Table = H_TO_NS(rID->Table);
						rID->HiPart = H_TO_NS(rID->HiPart);
						rID->LoPart = H_TO_N(rID->LoPart);
						str.Format( "%#06x%06x\t", rID->Table, rID->LoPart);
						break;
					}
					Line += str;
					index += m_spReply.Data.gtR.pFieldDefsRet[j].cbField;
				}
				pDoc->pPnPView->DisplayMessage(Line);
				Line.Empty();
			}
			delete pData;
			if( spReply.Data.etR.pRowsDataRet )
				delete spReply.Data.etR.pRowsDataRet;
			break;
		case ercTableNotfound:
			pDoc->pPnPView->DisplayMessage("Table not found\n");
			break;
		case ercTableEmpty:
			pDoc->pPnPView->DisplayMessage("No data\n");
			break;
		case ercNoMoreHeap:
			pDoc->pPnPView->DisplayMessage("No more heap\n");
			break;
		case ercBadParameter:
			pDoc->pPnPView->DisplayMessage("Bad parameter\n");
			break;
		default:
			str.Format("Unknown error, error code = %d\n", spReply.ErrorCode);
			pDoc->pPnPView->DisplayMessage(str);
			break;
		}
		str.Empty();

		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete [] m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		return TRUE;
	}
	else
	{
		pDoc->pPnPView->DisplayMessage("ReceiveEnumData - Read port error!\n");
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete [] m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}

		return FALSE;
	}
}

////////////////////////////////////
// BuildDeleteTableMessageAndSend
////////////////////////////////////
BOOL CDataLink::BuildDeleteTableMessageAndSend(CView *pView, CString& tableName)
{
	SP_PAYLOAD spPayload;
	DWORD signature = (DWORD)pView;
	spPayload.cmd = DELETE_TABLE;
	spPayload.chID = (long)pView;

	if( tableName.GetLength() == 0 )
		return FALSE;

	spPayload.cbData = sizeof( spPayload );
	strcpy(spPayload.Data.dt.TableName, tableName);

	// Send to serial port and wait for data back
	if( pDoc->m_Port.WriteABuffer( &spPayload, spPayload.cbData ) )
		return ReceiveDeleteTableData(H_TO_N(signature));
	else
	{
		pDoc->pPnPView->DisplayMessage("BuildDeleteTableMessageAndSend - Error writing to port\n");
		return FALSE;
	}
}

////////////////////////////////////
// ReceiveDeleteTableData
////////////////////////////////////
BOOL CDataLink::ReceiveDeleteTableData(DWORD signature)
{
	SP_REPLY_PAYLOAD spReply;
	signed long cbRead = sizeof(SP_REPLY_PAYLOAD);
	char str[256];

	if( pDoc->m_Port.ReadPort(&spReply, &cbRead, signature) )
	{
		switch( spReply.ErrorCode )
		{
		case ercOK:
			sprintf(str, "Table deleted\n");
			break;
		case ercTableNotfound:
			sprintf(str, "Table not found\n");
			break;
		case ercBadParameter:
			sprintf(str, "Bad parameter\n");
			break;
		default:
			sprintf(str, "Unknown error, error code = %d\n", spReply.ErrorCode);
			break;
		}
		pDoc->pPnPView->DisplayMessage(str);
		return TRUE;
	}
	else
	{
		pDoc->pPnPView->DisplayMessage("ReceiveDeleteTableData - Read port error!\n");
		return FALSE;
	}
}

////////////////////////////////////
// BuildReadRowMessageAndSend
////////////////////////////////////
BOOL CDataLink::BuildReadRowMessageAndSend(CView *pView, CList<CCtToken, CCtToken&>& List)
{
	SP_PAYLOAD spPayload;
	DWORD signature = (DWORD)pView;
	char* pData;
	CCtToken Token = List.GetHead();
	POSITION pos = List.GetHeadPosition();
	BOOL foundName = FALSE;
	unsigned long ulNumber;
	__int64 i64Number;
	double	dTemp;
	rowID	rID;
	CString str;

	// Get cbRowRetMax from the table definition
	BuildGetDefMessageAndSend(pView, Token.getTokenName(), FALSE);
	if( m_spReply.ErrorCode != ercOK )
	{
		char str[256];
		switch( m_spReply.ErrorCode )
		{
		case ercTableNotfound:
			pDoc->pPnPView->DisplayMessage("BuildReadRowMessageAndSend - Table not found\n");
			break;
		case ercNoMoreHeap:
			pDoc->pPnPView->DisplayMessage("BuildReadRowMessageAndSend - No more heap\n");
			break;
		case ercBadParameter:
			pDoc->pPnPView->DisplayMessage("BuildReadRowMessageAndSend - Bad parameter\n");
			break;
		default:
			sprintf(str, "BuildReadRowMessageAndSend - Unknown error, error code = %d\n", m_spReply.ErrorCode);
			pDoc->pPnPView->DisplayMessage(str);
			break;
		}
		return FALSE;
	}

	spPayload.cmd = READ_ROW;
	spPayload.chID = (long)pView;
	strcpy(spPayload.Data.rr.TableName, Token.getTokenName());
	Token = List.GetNext(pos);
	Token = List.GetNext(pos);

	// Search for the KeyFieldName	CString
	for( unsigned long i=0; i < m_spReply.Data.gtR.numFieldDefsRet && !foundName; )
	{
		foundName = (Token.getTokenName() == m_spReply.Data.gtR.pFieldDefsRet[i].name);
		if( !foundName)
			i++;
	}

	if( !foundName )
	{
		pDoc->pPnPView->DisplayMessage("BuildReadRowMessageAndSend - Invalid field type\n");
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		return FALSE;
	}
	strcpy(spPayload.Data.rr.KeyFieldName, Token.getTokenName());
	spPayload.Data.rr.cbKeyFieldValue = m_spReply.Data.gtR.pFieldDefsRet[i].cbField;
	spPayload.Data.rr.cbRowRetMax = m_spReply.Data.gtR.cbRowRet;
	spPayload.cbData = sizeof(spPayload) + m_spReply.Data.gtR.pFieldDefsRet[i].cbField;

	// Make a new buffer to send
	pData = new char[spPayload.cbData];
	// Copy the SP_PAYLOAD
	memcpy(pData, &spPayload, sizeof(spPayload));

	// Copy data for pKeyFieldValue
	Token = List.GetNext(pos);
	switch( Token.getTokenType() )
	{
	case intT:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case S32_FT:
		case U32_FT:
			ulNumber = Token.getTokenUL();
			ulNumber = H_TO_N(ulNumber);
			memcpy(pData + sizeof(spPayload), &ulNumber, m_spReply.Data.gtR.pFieldDefsRet[i].cbField);
			break;
		case S64_FT:
		case U64_FT:
			i64Number = Token.getTokenI64();
			memcpy(pData + sizeof(spPayload), &i64Number, m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildReadRowMessageAndSend - Field mismatch\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		break;
	case sintT:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case S32_FT:
			ulNumber = Token.getTokenUL();
			ulNumber = H_TO_N(ulNumber);
			memcpy(pData + sizeof(spPayload), &ulNumber, m_spReply.Data.gtR.pFieldDefsRet[i].cbField);
			break;
		case S64_FT:
			i64Number = Token.getTokenI64();
			memcpy(pData + sizeof(spPayload), &i64Number, m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildReadRowMessageAndSend - Field mismatch\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		break;
	case I64T:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case S64_FT:
		case U64_FT:
			i64Number = Token.getTokenI64();
			memcpy(pData + sizeof(spPayload), &i64Number, m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildReadRowMessageAndSend - Field mismatch\n");
				if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		break;
	case realT:
		dTemp = Token.getTokenReal();
		memcpy(pData + sizeof(spPayload), &dTemp, sizeof(double));
		break;
	case rowIDT:
		rID = Token.getTokenRowID();
		rID.Table = H_TO_NS(rID.Table);
		rID.HiPart = H_TO_NS(rID.HiPart);
		rID.LoPart = H_TO_N(rID.LoPart);
		memcpy(pData + sizeof(spPayload), &rID, sizeof(rowID));
		break;
	case stringT:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case STRING16_FT:
			str.Format("%-15s", Token.getTokenName());
			break;
		case STRING32_FT:
			str.Format("%-31s", Token.getTokenName());
			break;
		case STRING64_FT:
			str.Format("%-63s", Token.getTokenName());
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildInsertMessageAndSend - Field mismatch\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		str += '\0';
		memcpy(pData + sizeof(spPayload), str, str.GetLength() );
		break;
	default:
		pDoc->pPnPView->DisplayMessage("BuildReadRowMessageAndSend - Invalid field type\n");
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		return FALSE;
	}

	// Send to serial port and wait for data back
	if( pDoc->m_Port.WriteABuffer( pData, spPayload.cbData ) )
	{
		delete pData;
		return ReceiveReadRowData(H_TO_N(signature));
	}
	else
	{
		delete pData;
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		pDoc->pPnPView->DisplayMessage("BuildReadRowMessageAndSend - Error writing to port\n");
		return FALSE;
	}
}

////////////////////////////////////
// ReceiveReadRowData
////////////////////////////////////
BOOL CDataLink::ReceiveReadRowData(DWORD signature)
{
	SP_REPLY_PAYLOAD spReply;
	signed long cbRead = sizeof(SP_REPLY_PAYLOAD);
	CString str, Line;
	char *pData = NULL;
	__int64 *i64Num;
	unsigned long *upNum;
	signed long *spNum;
	rowID *rID;
	int index = 0;
	unsigned long i;

	if( pDoc->m_Port.ReadPort(&spReply, &cbRead, signature) )
	{
		switch( spReply.ErrorCode )
		{
		case ercOK:
			if( spReply.Data.rrR.pRowDataRet == NULL )
			{
				if( m_spReply.Data.gtR.pFieldDefsRet )
				{
					delete [] m_spReply.Data.gtR.pFieldDefsRet;
					SetPayloadNull();
				}
				pDoc->pPnPView->DisplayMessage("No data\n");
				return FALSE;
			}
			// Display the header
			for( i = 0; i < m_spReply.Data.gtR.numFieldDefsRet; i++)
			{
				str.Format("%s\t\t", m_spReply.Data.gtR.pFieldDefsRet[i].name);
				Line += str;
			}
			pDoc->pPnPView->DisplayMessage(Line);
			str.Empty();
			Line.Empty();

			pData = (char *)spReply.Data.rrR.pRowDataRet;
			for( i = 0; i < m_spReply.Data.gtR.numFieldDefsRet; i++ )
			{
				switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
				{
				case BINARY_FT:
					str.Format( "%#016x\t", *((__int64 *)pData));
					break;
				case S32_FT:
					spNum = (signed long*)(pData+index);
					*spNum = N_TO_H(unsigned long(*spNum));
					str.Format( "%-16ld\t", *spNum);
					break;
				case U32_FT:
					upNum = (unsigned long *)(pData+index);
					*upNum = N_TO_H(*upNum);
					str.Format( "%-16u\t", *upNum);
					break;
				case S64_FT:
					i64Num = (__int64 *)(pData+index);
					str.Format( "%-26ld\t", *i64Num);
					break;
				case U64_FT:
					i64Num = (__int64 *)(pData+index);
					str.Format( "%-26u\t", *i64Num);
					break;
				case STRING16_FT:
					str.Format( "%-16.16s\t\t", pData+index);
					break;
				case STRING32_FT:
					str.Format( "%-32.32s\t", pData+index);
					break;
				case STRING64_FT:
					str.Format( "%-64.64s\t", pData+index);
					break;
				case ROWID_FT:
					rID = (rowID *)(pData+index);
					rID->Table = H_TO_NS(rID->Table);
					rID->HiPart = H_TO_NS(rID->HiPart);
					rID->LoPart = H_TO_N(rID->LoPart);
					str.Format( "%#06x%06x\t", rID->Table, rID->LoPart);
					break;
				}
				Line += str;
				index += m_spReply.Data.gtR.pFieldDefsRet[i].cbField;
			}
			pDoc->pPnPView->DisplayMessage(Line);
			Line.Empty();
			str.Empty();

			if( spReply.Data.etR.pRowsDataRet )
				delete spReply.Data.etR.pRowsDataRet;
			break;
		case ercTableNotfound:
			pDoc->pPnPView->DisplayMessage("Table not found\n");
			break;
		case ercEOF:
			pDoc->pPnPView->DisplayMessage("Row not found\n");
			break;
		case ercNoMoreHeap:
			pDoc->pPnPView->DisplayMessage("No more heap\n");
			break;
		case ercBadParameter:
			pDoc->pPnPView->DisplayMessage("Bad parameter\n");
			break;
		default:
			str.Format("Unknown error, error code = %d\n", spReply.ErrorCode);
			pDoc->pPnPView->DisplayMessage(str);
			break;
		}
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		return TRUE;
	}
	else
	{
		pDoc->pPnPView->DisplayMessage("ReceiveReadRowData - Read port error!\n");
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		return FALSE;
	}
}

////////////////////////////////////
// BuildDeleteRowMessageAndSend
////////////////////////////////////
BOOL CDataLink::BuildDeleteRowMessageAndSend(CView *pView, CList<CCtToken, CCtToken&>& List)
{
	SP_PAYLOAD spPayload;
	DWORD signature = (DWORD)pView;
	char* pData;
	CCtToken Token = List.GetHead();
	POSITION pos = List.GetHeadPosition();
	BOOL foundName = FALSE;
	unsigned long ulNumber;
	__int64 i64Number;
	double	dTemp;
	rowID	rID;
	CString str;

	BuildGetDefMessageAndSend(pView, Token.getTokenName(), FALSE);
	if( m_spReply.ErrorCode != ercOK )
	{
		char str[256];
		switch( m_spReply.ErrorCode )
		{
		case ercTableNotfound:
			pDoc->pPnPView->DisplayMessage("BuildDeleteRowMessageAndSend - Table not found\n");
			break;
		case ercNoMoreHeap:
			pDoc->pPnPView->DisplayMessage("BuildDeleteRowMessageAndSend - No more heap\n");
			break;
		case ercBadParameter:
			pDoc->pPnPView->DisplayMessage("BuildDeleteRowMessageAndSend - Bad parameter\n");
			break;
		default:
			sprintf(str, "BuildDeleteRowMessageAndSend - Unknown error, error code = %d\n", m_spReply.ErrorCode);
			pDoc->pPnPView->DisplayMessage(str);
			break;
		}
		return FALSE;
	}

	spPayload.cmd = DELETE_ROW;
	spPayload.chID = (long)pView;
	strcpy(spPayload.Data.dr.TableName, Token.getTokenName());
	Token = List.GetNext(pos);
	Token = List.GetNext(pos);

	// Search for the KeyFieldName	CString
	for( unsigned long i=0; i < m_spReply.Data.gtR.numFieldDefsRet && !foundName; )
	{
		foundName = (Token.getTokenName() == m_spReply.Data.gtR.pFieldDefsRet[i].name);
		if( !foundName )
			i++;
	}

	if( !foundName )
	{
		pDoc->pPnPView->DisplayMessage("BuildDeleteRowMessageAndSend - Invalid field type\n");
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		return FALSE;
	}
	strcpy(spPayload.Data.dr.KeyFieldName, Token.getTokenName());
	spPayload.cbData = sizeof(spPayload) + m_spReply.Data.gtR.pFieldDefsRet[i].cbField;
	spPayload.Data.dr.cbKeyFieldValue = m_spReply.Data.gtR.pFieldDefsRet[i].cbField;

	// Make a new buffer to send
	pData = new char[spPayload.cbData];
	//Copy the SP_PAYLOAD
	memcpy(pData, &spPayload, sizeof(spPayload));

	Token = List.GetNext(pos);
	switch( Token.getTokenType() )
	{
	case intT:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case S32_FT:
		case U32_FT:
			ulNumber = Token.getTokenUL();
			ulNumber = H_TO_N(ulNumber);
			memcpy(pData + sizeof(spPayload), &ulNumber, m_spReply.Data.gtR.pFieldDefsRet[i].cbField);
			break;
		case S64_FT:
		case U64_FT:
			i64Number = Token.getTokenI64();
			memcpy(pData + sizeof(spPayload), &i64Number, m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildDeleteRowMessageAndSend - Field mismatch\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		break;
	case sintT:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case S32_FT:
			ulNumber = Token.getTokenUL();
			ulNumber = H_TO_N(ulNumber);
			memcpy(pData + sizeof(spPayload), &ulNumber, m_spReply.Data.gtR.pFieldDefsRet[i].cbField);
			break;
		case S64_FT:
			i64Number = Token.getTokenI64();
			memcpy(pData + sizeof(spPayload), &i64Number, m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildDeleteRowMessageAndSend - Field mismatch\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		break;
	case I64T:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case S64_FT:
		case U64_FT:
			i64Number = Token.getTokenI64();
			memcpy(pData + sizeof(spPayload), &i64Number, m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildDeleteRowMessageAndSend - Field mismatch\n");
				if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		break;
	case realT:
		dTemp = Token.getTokenReal();
		memcpy(pData + sizeof(spPayload), &dTemp, sizeof(double));
		break;
	case rowIDT:
		rID = Token.getTokenRowID();
		rID.Table = H_TO_NS(rID.Table);
		rID.HiPart = H_TO_NS(rID.HiPart);
		rID.LoPart = H_TO_N(rID.LoPart);
		memcpy(pData + sizeof(spPayload), &rID, sizeof(rowID));
		break;
	case stringT:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case STRING16_FT:
			str.Format("%-15s", Token.getTokenName());
			break;
		case STRING32_FT:
			str.Format("%-31s", Token.getTokenName());
			break;
		case STRING64_FT:
			str.Format("%-63s", Token.getTokenName());
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildInsertMessageAndSend - Field mismatch\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		str += '\0';
		memcpy(pData + sizeof(spPayload), str, str.GetLength() );
		break;
	default:
		pDoc->pPnPView->DisplayMessage("BuildDeleteRowMessageAndSend - Invalid field type\n");
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		return FALSE;
	}

	// Send to serial port and wait for data back
	if( pDoc->m_Port.WriteABuffer( pData, spPayload.cbData ) )
	{
		delete pData;
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		return ReceiveDeleteRowData(H_TO_N(signature));
	}
	else
	{
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		pDoc->pPnPView->DisplayMessage("BuildDeleteRowMessageAndSend - Error writing to port\n");
		return FALSE;
	}
}

////////////////////////////////////
// ReceiveDeleteRowData
////////////////////////////////////
BOOL CDataLink::ReceiveDeleteRowData(DWORD signature)
{
	SP_REPLY_PAYLOAD spReply;
	signed long cbRead = sizeof(SP_REPLY_PAYLOAD);
	char str[256];

	if( pDoc->m_Port.ReadPort(&spReply, &cbRead, signature) )
	{
		switch( spReply.ErrorCode )
		{
		case ercOK:
			sprintf(str, "Number of rows deleted = %d\n", spReply.Data.drR);
			break;
		case ercTableNotfound:
			sprintf(str, "Table not found\n");
			break;
		case ercEOF:
			sprintf(str, "Row not found\n");
			break;
		case ercBadParameter:
			sprintf(str, "Bad parameter\n");
			break;
		default:
			sprintf(str, "Unknown error, error code = %d\n", spReply.ErrorCode);
			break;
		}
		pDoc->pPnPView->DisplayMessage(str);
		return TRUE;
	}
	else
	{
		pDoc->pPnPView->DisplayMessage("ReceiveDeleteRowData - Read port error!\n");
		return FALSE;
	}
}

////////////////////////////////////
// BuildModifyRowMessageAndSend
////////////////////////////////////
BOOL CDataLink::BuildModifyRowMessageAndSend(CView *pView, CList<CCtToken, CCtToken&>& List)
{
	SP_PAYLOAD spPayload;
	DWORD signature = (DWORD)pView;
	char* pData;
	CCtToken Token = List.GetHead();
	POSITION pos = List.GetHeadPosition();
	BOOL foundName = FALSE;
	unsigned long ulNumber;
	__int64 i64Number;
	double	dTemp;
	rowID	rID;
	CString str;

	BuildGetDefMessageAndSend(pView, Token.getTokenName(), FALSE);
	if( m_spReply.ErrorCode != ercOK )
	{
		char str[256];
		switch( m_spReply.ErrorCode )
		{
		case ercTableNotfound:
			pDoc->pPnPView->DisplayMessage("BuildModifyRowMessageAndSend - Table not found\n");
			break;
		case ercNoMoreHeap:
			pDoc->pPnPView->DisplayMessage("BuildModifyRowMessageAndSend - No more heap\n");
			break;
		case ercBadParameter:
			pDoc->pPnPView->DisplayMessage("BuildModifyRowMessageAndSend - Bad parameter\n");
			break;
		default:
			sprintf(str, "BuildModifyRowMessageAndSend - Unknown error, error code = %d\n", m_spReply.ErrorCode);
			pDoc->pPnPView->DisplayMessage(str);
			break;
		}
		return FALSE;
	}

	spPayload.cmd = MODIFY_ROW;
	spPayload.chID = (long)pView;
	strcpy(spPayload.Data.mr.TableName, Token.getTokenName());
	Token = List.GetNext(pos);
	Token = List.GetNext(pos);

	// Search for the KeyFieldName	CString
	for( unsigned long i=0; i < m_spReply.Data.gtR.numFieldDefsRet && !foundName; )
	{
		foundName = (Token.getTokenName() == m_spReply.Data.gtR.pFieldDefsRet[i].name);
		if( !foundName )
			i++;
	}

	if( !foundName )
	{
		pDoc->pPnPView->DisplayMessage("BuildModifyRowMessageAndSend - Invalid field type\n");
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		return FALSE;
	}
	strcpy(spPayload.Data.mr.KeyFieldName, Token.getTokenName());
	spPayload.Data.mr.cbKeyFieldValue = m_spReply.Data.gtR.pFieldDefsRet[i].cbField;
	spPayload.Data.mr.cbRowData = sizeof(rowID);
	spPayload.cbData = sizeof(spPayload) + m_spReply.Data.gtR.pFieldDefsRet[i].cbField + m_spReply.Data.gtR.cbRowRet;

	// Make a new buffer to send
	pData = new char[spPayload.cbData];
	// Copy the SP_PAYLOAD
	memcpy(pData, &spPayload, sizeof(spPayload));

	// Copy data for pKeyFieldValue
	Token = List.GetNext(pos);
	switch( Token.getTokenType() )
	{
	case intT:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case S32_FT:
		case U32_FT:
			ulNumber = Token.getTokenUL();
			ulNumber = H_TO_N(ulNumber);
			memcpy(pData + sizeof(spPayload), &ulNumber, m_spReply.Data.gtR.pFieldDefsRet[i].cbField);
			break;
		case S64_FT:
		case U64_FT:
			i64Number = Token.getTokenI64();
			memcpy(pData + sizeof(spPayload), &i64Number, m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildModifyRowMessageAndSend - Field mismatch\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		break;
	case sintT:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case S32_FT:
			ulNumber = Token.getTokenUL();
			ulNumber = H_TO_N(ulNumber);
			memcpy(pData + sizeof(spPayload), &ulNumber, m_spReply.Data.gtR.pFieldDefsRet[i].cbField);
			break;
		case S64_FT:
			i64Number = Token.getTokenI64();
			memcpy(pData + sizeof(spPayload), &i64Number, m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildModifyRowMessageAndSend - Field mismatch\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		break;
	case I64T:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case S64_FT:
		case U64_FT:
			i64Number = Token.getTokenI64();
			memcpy(pData + sizeof(spPayload), &i64Number, m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildDeleteRowMessageAndSend - Field mismatch\n");
				if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		break;
	case realT:
		dTemp = Token.getTokenReal();
		memcpy(pData + sizeof(spPayload), &dTemp, sizeof(double));
		break;
	case rowIDT:
		rID = Token.getTokenRowID();
		rID.Table = H_TO_NS(rID.Table);
		rID.HiPart = H_TO_NS(rID.HiPart);
		rID.LoPart = H_TO_N(rID.LoPart);
		memcpy(pData + sizeof(spPayload), &rID, sizeof(rowID));
		break;
	case stringT:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case STRING16_FT:
			str.Format("%-15s", Token.getTokenName());
			break;
		case STRING32_FT:
			str.Format("%-31s", Token.getTokenName());
			break;
		case STRING64_FT:
			str.Format("%-63s", Token.getTokenName());
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildModifyRowMessageAndSend - Field mismatch\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		str += '\0';
		memcpy(pData + sizeof(spPayload), str, str.GetLength() );
		break;
	default:
		pDoc->pPnPView->DisplayMessage("BuildModifyRowMessageAndSend - Invalid field type\n");
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		return FALSE;
	}

	// Copy data for pRowData
	i = 1;	// Skip the first field of pFieldDefsRet which is rid

	// Iterate through the list and copy data, skip the first parameter
	for( ; pos != NULL; i++)
	{
		Token = List.GetNext( pos );
		switch( Token.getTokenType() )
		{
		case intT:
			switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
			{
			case S32_FT:
			case U32_FT:
				ulNumber = Token.getTokenUL();
				ulNumber = N_TO_H(ulNumber);
				memcpy(pData+sizeof(spPayload) + spPayload.Data.mr.cbKeyFieldValue + spPayload.Data.mr.cbRowData,
					&ulNumber, m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
				break;
			case S64_FT:
			case U64_FT:
				i64Number = Token.getTokenI64();
				memcpy(pData+sizeof(spPayload)+spPayload.Data.mr.cbKeyFieldValue, &i64Number,
					m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
				break;
			default:	// Field mismatch
				pDoc->pPnPView->DisplayMessage("BuildInsertMessageAndSend - Field mismatch\n");
				if( m_spReply.Data.gtR.pFieldDefsRet )
				{
					delete m_spReply.Data.gtR.pFieldDefsRet;
					SetPayloadNull();
				}
				if( pData )
					delete pData;
				return FALSE;
			}
			spPayload.Data.mr.cbRowData += m_spReply.Data.gtR.pFieldDefsRet[i].cbField;
			break;
		case sintT:
			switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
			{
			case S32_FT:
				ulNumber = Token.getTokenUL();
				ulNumber = N_TO_H(ulNumber);
				memcpy(pData+sizeof(spPayload) + spPayload.Data.mr.cbKeyFieldValue + spPayload.Data.mr.cbRowData,
					&ulNumber, m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
				break;
			case S64_FT:
				i64Number = Token.getTokenI64();
				memcpy(pData+sizeof(spPayload)+spPayload.Data.mr.cbKeyFieldValue, &i64Number,
					m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
				break;
			default:	// Field mismatch
				pDoc->pPnPView->DisplayMessage("BuildInsertMessageAndSend - Field mismatch\n");
				if( m_spReply.Data.gtR.pFieldDefsRet )
				{
					delete m_spReply.Data.gtR.pFieldDefsRet;
					SetPayloadNull();
				}
				if( pData )
					delete pData;
				return FALSE;
			}
			spPayload.Data.mr.cbRowData += m_spReply.Data.gtR.pFieldDefsRet[i].cbField;
			break;
		case I64T:
			// This will be the __int64 type in Win32, but it need support from the LexicalAnalyser class
			switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
			{
			case S64_FT:
			case U64_FT:
				i64Number = Token.getTokenI64();
				memcpy(pData+sizeof(spPayload)+spPayload.Data.mr.cbKeyFieldValue, &i64Number,
					m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
				break;
			default:	// Field mismatch
				pDoc->pPnPView->DisplayMessage("BuildInsertMessageAndSend - Field mismatch\n");
				if( m_spReply.Data.gtR.pFieldDefsRet )
				{
					delete m_spReply.Data.gtR.pFieldDefsRet;
					SetPayloadNull();
				}
				if( pData )
					delete pData;
				return FALSE;
			}
			spPayload.Data.mr.cbRowData += m_spReply.Data.gtR.pFieldDefsRet[i].cbField;
			break;
		case realT:
			dTemp = Token.getTokenReal();
			memcpy(pData+sizeof(spPayload) + spPayload.Data.mr.cbKeyFieldValue + spPayload.Data.mr.cbRowData,
				&dTemp, sizeof(double));
			spPayload.Data.mr.cbRowData += sizeof(double);
			break;
		case rowIDT:
			rID = Token.getTokenRowID();
			rID.Table = H_TO_NS(rID.Table);
			rID.HiPart = H_TO_NS(rID.HiPart);
			rID.LoPart = H_TO_N(rID.LoPart);
			memcpy(pData+sizeof(spPayload) + spPayload.Data.mr.cbKeyFieldValue + spPayload.Data.mr.cbRowData,
				&rID, sizeof(rowID));
			spPayload.Data.mr.cbRowData += sizeof(rowID);
			break;
		case stringT:
			switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
			{
			case STRING16_FT:
				str.Format("%-15s", Token.getTokenName());
				break;
			case STRING32_FT:
				str.Format("%-31s", Token.getTokenName());
				break;
			case STRING64_FT:
				str.Format("%-63s", Token.getTokenName());
				break;
			default:	// Field mismatch
				pDoc->pPnPView->DisplayMessage("BuildModifyRowMessageAndSend - Field mismatch\n");
				if( m_spReply.Data.gtR.pFieldDefsRet )
				{
					delete m_spReply.Data.gtR.pFieldDefsRet;
					SetPayloadNull();
				}
				if( pData )
					delete pData;
				return FALSE;
			}
			str += '\0';
			memcpy(pData+sizeof(spPayload) + spPayload.Data.mr.cbKeyFieldValue + spPayload.Data.mr.cbRowData,
				str, str.GetLength() );
			spPayload.Data.mr.cbRowData += str.GetLength();
			break;
		default:
			pDoc->pPnPView->DisplayMessage("BuildModifyRowMessageAndSend - Invalid field type\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			return FALSE;
		}
	}
	if( spPayload.Data.mr.cbRowData != m_spReply.Data.gtR.cbRowRet )
	{
		pDoc->pPnPView->DisplayMessage("BuildModifyRowMessageAndSend - Field size mismatch\n");
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		if( pData )
			delete pData;
		return FALSE;
	}

	// Send to serial port and wait for data back
	if( pDoc->m_Port.WriteABuffer( pData, spPayload.cbData ) )
	{
		delete pData;
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		return ReceiveModifyRowData(H_TO_N(signature));
	}
	else
	{
		if( pData )
			delete pData;
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		pDoc->pPnPView->DisplayMessage("BuildModifyRowMessageAndSend - Write to port error\n");
		return FALSE;
	}
}

////////////////////////////////////
// ReceiveModifyRowData
////////////////////////////////////
BOOL CDataLink::ReceiveModifyRowData(DWORD signature)
{
	SP_REPLY_PAYLOAD spReply;
	signed long cbRead = sizeof(SP_REPLY_PAYLOAD);
	char str[256];

	if( pDoc->m_Port.ReadPort(&spReply, &cbRead, signature) )
	{
		switch( spReply.ErrorCode )
		{
		case ercOK:
			sprintf(str, "rowID.Table = %#04x   rowID.HiPart = %#04x   rowID.LoPart = %#08x\n",
			spReply.Data.mrR.Table, spReply.Data.mrR.HiPart, spReply.Data.mrR.LoPart);
			break;
		case ercTableNotfound:
			sprintf(str, "Table not found\n");
			break;
		case ercKeyNotFound:
			sprintf(str, "Key not found\n");
			break;
		case ercEOF:
			sprintf(str, "Row not found\n");
			break;
		case ercBadParameter:
			sprintf(str, "Bad parameter\n");
			break;
		default:
			sprintf(str, "Unknown error, error code = %d\n", spReply.ErrorCode);
			break;
		}
		pDoc->pPnPView->DisplayMessage(_T(str));
		return TRUE;
	}
	else
	{
		pDoc->pPnPView->DisplayMessage("ReceiveModifyRowData - Read port error!\n");
		return FALSE;
	}
}

////////////////////////////////////
// BuildModifyFieldMessageAndSend
////////////////////////////////////
BOOL CDataLink::BuildModifyFieldMessageAndSend(CView *pView, CList<CCtToken, CCtToken&>& List)
{
	SP_PAYLOAD spPayload;
	DWORD signature = (DWORD)pView;
	char* pData;
	CCtToken Token = List.GetHead();
	POSITION pos = List.GetHeadPosition();
	BOOL foundName = FALSE;
	unsigned long ulNumber;
	__int64 i64Number;
	double	dTemp;
	rowID	rID;
	CString str;

	BuildGetDefMessageAndSend(pView, Token.getTokenName(), FALSE);
	if( m_spReply.ErrorCode != ercOK )
	{
		char str[256];
		switch( m_spReply.ErrorCode )
		{
		case ercTableNotfound:
			pDoc->pPnPView->DisplayMessage("BuildModifyFieldMessageAndSend - Table not found\n");
			break;
		case ercNoMoreHeap:
			pDoc->pPnPView->DisplayMessage("BuildModifyFieldMessageAndSend - No more heap\n");
			break;
		case ercBadParameter:
			pDoc->pPnPView->DisplayMessage("BuildModifyFieldMessageAndSend - Bad parameter\n");
			break;
		default:
			sprintf(str, "BuildModifyFieldMessageAndSend - Unknown error, error code = %d\n", m_spReply.ErrorCode);
			pDoc->pPnPView->DisplayMessage(str);
			break;
		}
		return FALSE;
	}

	spPayload.cmd = MODIFY_FIELD;
	spPayload.chID = (long)pView;
	strcpy(spPayload.Data.mf.TableName, Token.getTokenName());
	List.GetNext(pos);
	Token = List.GetNext(pos);

	// Search for the KeyFieldName	CString
	for( unsigned long i=0; i < m_spReply.Data.gtR.numFieldDefsRet && !foundName; )
	{
		foundName = (Token.getTokenName() == m_spReply.Data.gtR.pFieldDefsRet[i].name);
		if( !foundName )
			i++;
	}

	if( !foundName )
	{
		pDoc->pPnPView->DisplayMessage("BuildModifyFieldMessageAndSend - Key field name not found\n");
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		return FALSE;
	}
	foundName = FALSE;
	strcpy(spPayload.Data.mf.KeyFieldName, Token.getTokenName());
	spPayload.Data.mf.cbKeyFieldValue = m_spReply.Data.gtR.pFieldDefsRet[i].cbField;
	spPayload.cbData = sizeof(spPayload) + m_spReply.Data.gtR.pFieldDefsRet[i].cbField;

	List.GetNext(pos);	// skip the KeyFieldValue
	Token = List.GetNext(pos);
	// Search for the FieldName	CString
	for( i=0; i < m_spReply.Data.gtR.numFieldDefsRet && !foundName; )
	{
		foundName = (Token.getTokenName() == m_spReply.Data.gtR.pFieldDefsRet[i].name);
		if( !foundName )
			i++;
	}

	if( !foundName )
	{
		pDoc->pPnPView->DisplayMessage("BuildModifyFieldMessageAndSend - Field name not found\n");
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		return FALSE;
	}
	strcpy(spPayload.Data.mf.FieldName, Token.getTokenName());
	spPayload.Data.mf.cbFieldValue = m_spReply.Data.gtR.pFieldDefsRet[i].cbField;
	spPayload.cbData += m_spReply.Data.gtR.pFieldDefsRet[i].cbField;

	// Make a new buffer to send
	pData = new char[spPayload.cbData];
	// Copy the SP_PAYLOAD
	memcpy(pData, &spPayload, sizeof(spPayload));

	// Copy data for pKeyFieldValue
	List.GetPrev(pos);
	List.GetPrev(pos);
	Token = List.GetPrev(pos);
	switch( Token.getTokenType() )
	{
	case intT:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case S32_FT:
		case U32_FT:
			ulNumber = Token.getTokenUL();
			ulNumber = H_TO_N(ulNumber);
			memcpy(pData + sizeof(spPayload), &ulNumber, m_spReply.Data.gtR.pFieldDefsRet[i].cbField);
			break;
		case S64_FT:
		case U64_FT:
			i64Number = Token.getTokenI64();
			memcpy(pData + sizeof(spPayload), &i64Number,m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildModifyFieldMessageAndSend - Field mismatch\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		break;
	case sintT:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case S32_FT:
			ulNumber = Token.getTokenUL();
			ulNumber = H_TO_N(ulNumber);
			memcpy(pData + sizeof(spPayload), &ulNumber, m_spReply.Data.gtR.pFieldDefsRet[i].cbField);
			break;
		case S64_FT:
			i64Number = Token.getTokenI64();
			memcpy(pData + sizeof(spPayload), &i64Number,m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildModifyFieldMessageAndSend - Field mismatch\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		break;
	case I64T:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case S64_FT:
		case U64_FT:
			i64Number = Token.getTokenI64();
			memcpy(pData + sizeof(spPayload), &i64Number,m_spReply.Data.gtR.pFieldDefsRet[i].cbField );
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildModifyFieldMessageAndSend - Field mismatch\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		break;
	case realT:
		dTemp = Token.getTokenReal();
		memcpy(pData + sizeof(spPayload), &dTemp, sizeof(double));
		break;
	case rowIDT:
		rID = Token.getTokenRowID();
		rID.Table = H_TO_NS(rID.Table);
		rID.HiPart = H_TO_NS(rID.HiPart);
		rID.LoPart = H_TO_N(rID.LoPart);
		memcpy(pData + sizeof(spPayload), &rID, sizeof(rowID));
		break;
	case stringT:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case STRING16_FT:
			str.Format("%-15s", Token.getTokenName());
			break;
		case STRING32_FT:
			str.Format("%-31s", Token.getTokenName());
			break;
		case STRING64_FT:
			str.Format("%-63s", Token.getTokenName());
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildModifyRowMessageAndSend - Field mismatch\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		str += '\0';
		memcpy(pData + sizeof(spPayload), str, str.GetLength() );
		break;
	default:
		pDoc->pPnPView->DisplayMessage("BuildModifyFieldMessageAndSend - Invalid field type\n");
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		return FALSE;
	}

	// Copy data for pFieldValue
	List.GetNext(pos);
	List.GetNext(pos);
	List.GetNext(pos);
	Token = List.GetNext(pos);
	switch( Token.getTokenType() )
	{
	case intT:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case S32_FT:
		case U32_FT:
			ulNumber = Token.getTokenUL();
			ulNumber = H_TO_N(ulNumber);
			memcpy(pData+sizeof(spPayload)+spPayload.Data.mf.cbKeyFieldValue, &ulNumber, m_spReply.Data.gtR.pFieldDefsRet[i].cbField);
			break;
		case S64_FT:
		case U64_FT:
			i64Number = Token.getTokenI64();
			memcpy(pData+sizeof(spPayload)+spPayload.Data.mf.cbKeyFieldValue, &i64Number, m_spReply.Data.gtR.pFieldDefsRet[i].cbField);
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildModifyFieldMessageAndSend - Field mismatch\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		break;
	case sintT:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case S32_FT:
			ulNumber = Token.getTokenUL();
			ulNumber = H_TO_N(ulNumber);
			memcpy(pData+sizeof(spPayload)+spPayload.Data.mf.cbKeyFieldValue, &ulNumber, m_spReply.Data.gtR.pFieldDefsRet[i].cbField);
			break;
		case S64_FT:
			i64Number = Token.getTokenI64();
			memcpy(pData+sizeof(spPayload)+spPayload.Data.mf.cbKeyFieldValue, &i64Number, m_spReply.Data.gtR.pFieldDefsRet[i].cbField);
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildModifyFieldMessageAndSend - Field mismatch\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		break;
	case I64T:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case S64_FT:
		case U64_FT:
			i64Number = Token.getTokenI64();
			memcpy(pData+sizeof(spPayload)+spPayload.Data.mf.cbKeyFieldValue, &i64Number, m_spReply.Data.gtR.pFieldDefsRet[i].cbField);
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildModifyFieldMessageAndSend - Field mismatch\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		break;
	case realT:
		dTemp = Token.getTokenReal();
		memcpy(pData+sizeof(spPayload)+spPayload.Data.mf.cbKeyFieldValue, &dTemp, m_spReply.Data.gtR.pFieldDefsRet[i].cbField);
		break;
	case rowIDT:
		rID = Token.getTokenRowID();
		rID.Table = H_TO_NS(rID.Table);
		rID.HiPart = H_TO_NS(rID.HiPart);
		rID.LoPart = H_TO_N(rID.LoPart);
		memcpy(pData+sizeof(spPayload)+spPayload.Data.mf.cbKeyFieldValue, &rID, sizeof(rowID));
		break;
	case stringT:
		switch( m_spReply.Data.gtR.pFieldDefsRet[i].iFieldType )
		{
		case STRING16_FT:
			str.Format("%-15s", Token.getTokenName());
			break;
		case STRING32_FT:
			str.Format("%-31s", Token.getTokenName());
			break;
		case STRING64_FT:
			str.Format("%-63s", Token.getTokenName());
			break;
		default:	// Field mismatch
			pDoc->pPnPView->DisplayMessage("BuildModifyFieldMessageAndSend - Field mismatch\n");
			if( m_spReply.Data.gtR.pFieldDefsRet )
			{
				delete m_spReply.Data.gtR.pFieldDefsRet;
				SetPayloadNull();
			}
			if( pData )
				delete pData;
			return FALSE;
		}
		str += '\0';
		memcpy(pData + sizeof(spPayload)+spPayload.Data.mf.cbKeyFieldValue, str, str.GetLength() );
		break;
	default:
		pDoc->pPnPView->DisplayMessage("BuildModifyFieldMessageAndSend - Invalid field type\n");
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		return FALSE;
	}

	// Send to serial port and wait for data back
	if( pDoc->m_Port.WriteABuffer( pData, spPayload.cbData ) )
	{
		delete pData;
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		return ReceiveModifyFieldData(H_TO_N(signature));
	}
	else
	{
		pDoc->pPnPView->DisplayMessage("BuildModifyFieldMessageAndSend - Error writing to port\n");
		if( m_spReply.Data.gtR.pFieldDefsRet )
		{
			delete m_spReply.Data.gtR.pFieldDefsRet;
			SetPayloadNull();
		}
		return FALSE;
	}
}

////////////////////////////////////
// ReceiveModifyFieldData
////////////////////////////////////
BOOL CDataLink::ReceiveModifyFieldData(DWORD signature)
{
	SP_REPLY_PAYLOAD spReply;
	signed long cbRead = sizeof(SP_REPLY_PAYLOAD);
	char str[256];

	if( pDoc->m_Port.ReadPort(&spReply, &cbRead, signature) )
	{
		switch( spReply.ErrorCode )
		{
		case ercOK:
		sprintf(str, "rowID.Table = %#04x   rowID.HiPart = %#04x   rowID.LoPart = %#08x\n",
		spReply.Data.mfR.Table, spReply.Data.mfR.HiPart, spReply.Data.mfR.LoPart);
			break;
		case ercTableNotfound:
			sprintf(str, "Table not found\n");
			break;
		case ercKeyNotFound:
			sprintf(str, "Key not found\n");
			break;
		case ercEOF:
			sprintf(str, "Row not found\n");
			break;
		case ercFieldNotFound:
			sprintf(str, "Key field not found\n");
			break;
		case ercBadParameter:
			sprintf(str, "Bad parameter\n");
			break;
		default:
			sprintf(str, "Unknown error, error code = %d\n", spReply.ErrorCode);
			break;
		}
		pDoc->pPnPView->DisplayMessage(_T(str));
		return TRUE;
	}
	else
	{
		pDoc->pPnPView->DisplayMessage("ReceiveModifyFieldData - Read port error!\n");
		return FALSE;
	}
}

////////////////////////////////////
// BuildListenMessageAndSend
////////////////////////////////////
BOOL CDataLink::BuildListenMessageAndSend(CView *pView, CList<CCtToken, CCtToken&>& List)
{
	SP_PAYLOAD spPayload;
	DWORD signature = (DWORD)pView;

	spPayload.cmd = LISTEN;
	spPayload.chID = (long)pView;

	spPayload.cbData = sizeof( spPayload );
	//strcpy(spPayload.Data.gt.TableName, tableName);

	// Send to serial port and wait for data back
	if( pDoc->m_Port.WriteABuffer( &spPayload, spPayload.cbData ) )
		return ReceiveListenData(H_TO_N(signature));
	else
	{
		pDoc->pPnPView->DisplayMessage("BuildListenMessageAndSend - Error writing to port\n");
		SetPayloadNull();
		return FALSE;
	}
}

////////////////////////////////////
// ReceiveListenData
////////////////////////////////////
BOOL CDataLink::ReceiveListenData(DWORD signature)
{
	SP_REPLY_PAYLOAD spReply;
	signed long cbRead = sizeof(SP_REPLY_PAYLOAD);
	//char str[256];

	if( pDoc->m_Port.ReadPort(&spReply, &cbRead, signature) )
		return TRUE;
	else
	{
		pDoc->pPnPView->DisplayMessage("ReceiveListenData - Read port error!\n");
		return FALSE;
	}
}

////////////////////////////////////
// BuildConnectMessageAndSend
////////////////////////////////////
BOOL CDataLink::BuildConnectMessageAndSend(CView *pView)
{
	SP_PAYLOAD spPayload;
	DWORD signature = (DWORD)pView;

	SetPayloadNull();
	spPayload.cmd = CONNECT;
	spPayload.cbData = sizeof( spPayload );
	spPayload.chID = (long)pView;
	memset(&spPayload.Data, 0, sizeof(spPayload.Data));

	// Send to serial port and wait for data back
	if( pDoc->m_Port.WriteABuffer( &spPayload, spPayload.cbData ) )
		return ReceiveConnectData(H_TO_N(signature));
	else
		return FALSE;
}

////////////////////////////////////
// ReceiveConnectData
////////////////////////////////////
BOOL CDataLink::ReceiveConnectData(DWORD signature)
{
	SP_REPLY_PAYLOAD spReply;
	signed long cbRead = sizeof(SP_REPLY_PAYLOAD);

	if( pDoc->m_Port.ReadPort(&spReply, &cbRead, signature) )
		return (spReply.cmd == REPLY_CONNECT)? TRUE : FALSE;
	else
		return FALSE;
}
