// MsgWndView.cpp : implementation file
//

#include "stdafx.h"
#include "PeeknPoke.h"
#include "MsgWndView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MsgWndView

IMPLEMENT_DYNCREATE(MsgWndView, CEditView)

MsgWndView::MsgWndView()
{
}

MsgWndView::~MsgWndView()
{
}

BEGIN_MESSAGE_MAP(MsgWndView, CEditView)
	//{{AFX_MSG_MAP(MsgWndView)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MsgWndView drawing

void MsgWndView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// MsgWndView diagnostics

#ifdef _DEBUG
void MsgWndView::AssertValid() const
{
	CEditView::AssertValid();
}

void MsgWndView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CPeeknPokeDoc* MsgWndView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPeeknPokeDoc)));
	return (CPeeknPokeDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// MsgWndView message handlers

int MsgWndView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CEditView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	CPeeknPokeDoc* pDoc = GetDocument();
	pDoc->pMWView = this;

	return 0;
}

BOOL MsgWndView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	BOOL ret = CEditView::PreCreateWindow(cs);
	cs.style = AFX_WS_DEFAULT_VIEW | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL |
				ES_MULTILINE | ES_NOHIDESEL | ES_READONLY;

	return ret;
}

void MsgWndView::DisplayMessage(LPCTSTR msg)
{
	CString strTemp = msg;
	strTemp += _T("\r\n");

	int len = GetEditCtrl().GetWindowTextLength();
	GetEditCtrl().SetSel(len, len);
	GetEditCtrl().ReplaceSel(strTemp);
}
