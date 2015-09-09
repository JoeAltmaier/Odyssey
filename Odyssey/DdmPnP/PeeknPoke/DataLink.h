#if !defined __DataLink__
#define __DataLink__

#include <afxtempl.h>
#include "PnPMsg.h"
#include "LexicalAnalyser.h"

class CPeeknPokeDoc;

struct TokenNode {
	CString name;
	CCtToken	token;
};

class CDataLink {
protected:
	SP_REPLY_PAYLOAD m_spReply;

	BOOL ReceiveCreateData(DWORD signature);
	BOOL ReceiveInsertData(DWORD signature);
	BOOL ReceiveGetDefData(DWORD signature, BOOL DisplayData);
	BOOL ReceiveEnumData(DWORD signature);

	BOOL ReceiveDeleteTableData(DWORD signature);
	BOOL ReceiveReadRowData(DWORD signature);
	BOOL ReceiveDeleteRowData(DWORD signature);
	BOOL ReceiveModifyRowData(DWORD signature);
	BOOL ReceiveModifyFieldData(DWORD signature);
	BOOL ReceiveListenData(DWORD signature);
	BOOL ReceiveConnectData(DWORD signature);

	void SetPayloadNull() { m_spReply.cbData = 0; m_spReply.Data.gtR.pFieldDefsRet = NULL; };

public:
	CPeeknPokeDoc *pDoc;

	BOOL BuildCreateMessageAndSend(CView *pView, CList<TokenNode, TokenNode&>& List);
	BOOL BuildInsertMessageAndSend(CView *pView, CList<CCtToken, CCtToken&>& List);
	BOOL BuildGetDefMessageAndSend(CView *pView, CString& tableName, BOOL DisplayData);
	BOOL BuildEnumMessageAndSend  (CView *pView, CString& tableName);

	BOOL BuildDeleteTableMessageAndSend(CView *pView, CString& tableName);
	BOOL BuildReadRowMessageAndSend    (CView *pView, CList<CCtToken, CCtToken&>& List);
	BOOL BuildDeleteRowMessageAndSend  (CView *pView, CList<CCtToken, CCtToken&>& List);
	BOOL BuildModifyRowMessageAndSend  (CView *pView, CList<CCtToken, CCtToken&>& List);
	BOOL BuildModifyFieldMessageAndSend(CView *pView, CList<CCtToken, CCtToken&>& List);
	BOOL BuildListenMessageAndSend     (CView *pView, CList<CCtToken, CCtToken&>& List);
	BOOL BuildConnectMessageAndSend    (CView *pView);
} ;

#endif //__DataLink__