// PeeknPokeView.cpp : implementation of the CPeeknPokeView class
//

#include "stdafx.h"
#include "PeeknPoke.h"

//#include "ProgressStatusBar.h"
#include "PeeknPokeDoc.h"
#include "PeeknPokeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPeeknPokeView

IMPLEMENT_DYNCREATE(CPeeknPokeView, CScrollView)

BEGIN_MESSAGE_MAP(CPeeknPokeView, CScrollView)
	//{{AFX_MSG_MAP(CPeeknPokeView)
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPeeknPokeView construction/destruction

CPeeknPokeView::CPeeknPokeView() : m_ViewCharSize(0, 0), m_DocSize(0, 0)
{
	// TODO: add construction code here
	m_CaretPos.x = 0;
	m_CaretPos.y = 0;
	m_Caret = 0;
}

CPeeknPokeView::~CPeeknPokeView()
{
}

BOOL CPeeknPokeView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CPeeknPokeView drawing

void CPeeknPokeView::OnDraw(CDC* pDC)
{
	CPeeknPokeDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CClientDC ClientDC(this);
	TEXTMETRIC	TM;

	ClientDC.GetTextMetrics( &TM );
	MARGINX = ClientDC.GetTextExtent(">").cx;

	int nFirstLn, nLastLn;
	ComputeVisibleLines(pDC, nFirstLn, nLastLn);
	CStringList *pLineList = GetDocument()->GetLineList();
	CString strLine;
	POSITION pos;

	int i = 0;
	for( pos = pLineList->FindIndex( nFirstLn ); pos != NULL; i++)
	{
		strLine = pLineList->GetNext( pos );
		pDC->TabbedTextOut(0, i*TM.tmHeight + TM.tmExternalLeading, strLine, strLine.GetLength()-1, 0, NULL, 0);
	}
	
	CPoint	scrollPos= GetScrollPosition();
	HideCaret();

	// Calculate the position of the caret
	m_CaretPos.x = pDC->GetTextExtent(pDoc->m_TextLine, m_Caret).cx+ MARGINX;
	m_CaretPos.y = pLineList->GetCount()*TM.tmHeight + TM.tmExternalLeading;

	// Display the caret
	pDC->TextOut( 0, m_CaretPos.y, ">");
	pDC->TextOut( MARGINX, m_CaretPos.y, pDoc->m_TextLine, pDoc->m_TextLine.GetLength());

	CPoint caret = CPoint(m_CaretPos);
	if( CPoint(caret - scrollPos) != m_CaretPos )
		m_CaretPos = CPoint(caret - scrollPos);
	SetCaretPos(m_CaretPos);
	ShowCaret();
}

void CPeeknPokeView::ComputeVisibleLines(CDC * pDC, int & nFirst, int & nLast)
{
	int nLineCount = GetDocument()->GetLineList()->GetCount();

	// Get the viewpoint origin, converrt to logical coordinates
	CPoint pt = pDC->GetViewportOrg();
	pDC->DPtoLP(&pt, 1);

	// Get the clipping region
	CRect rc;
	pDC->GetClipBox(&rc);

	// Get the logical line height
	CSize CharSize = GetCharSize();

	// Compute the first visible line
	nFirst = min(abs((rc.top - pt.y)/CharSize.cy), nLineCount);//-1
	// Compute the last visible line
	nLast = min(abs(rc.Height())/CharSize.cy + nFirst + 1, nLineCount);// - 1
}

void CPeeknPokeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class

	//CPeeknPokeDoc*	pDoc = GetDocument();
	CStringList *pLineList = GetDocument()->GetLineList();
	CDC* pDC = CDC::FromHandle(::GetDC(NULL));
	CSize Size;
	CRect	rect;
	TEXTMETRIC TM;

	GetClientRect(&rect);

	// Calculate view character size
	m_ViewCharSize.cy = TM.tmHeight + TM.tmExternalLeading;
	m_ViewCharSize.cx = TM.tmAveCharWidth;

	// convert to device units to minimize round off error
	pDC->LPtoDP(&m_ViewCharSize);

	if( pLineList->GetCount() == 0 )
	{
		m_DocSize.cx = m_DocSize.cy = 0;
		SetScrollSizes(MM_TEXT, m_DocSize);
	}
	else
	{
		pDC->GetTextMetrics( &TM );
		POSITION pos = pLineList->GetHeadPosition();
		CString Line;
		while( pos != NULL )
		{
			Line = pLineList->GetNext( pos );
			Size = pDC->GetTextExtent(Line, Line.GetLength() );
			m_DocSize.cx = max(Size.cx, m_DocSize.cx );
		}

		m_DocSize.cx += MARGINX;
		m_DocSize.cy = (TM.tmHeight + TM.tmExternalLeading)*(pLineList->GetCount()+1);
		if( m_DocSize.cx < rect.Width() )
			m_DocSize.cx = 0;
	}

	if(m_DocSize.cy > rect.Height() )
	{
		SetScrollSizes(MM_TEXT, GetDocSize());
		ScrollToPosition(CPoint(0, m_DocSize.cy));
	}

	//Invalidate();
	CScrollView::OnUpdate(pSender, lHint, pHint);
}

/////////////////////////////////////////////////////////////////////////////
// CPeeknPokeView diagnostics

#ifdef _DEBUG
void CPeeknPokeView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CPeeknPokeView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CPeeknPokeDoc* CPeeknPokeView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPeeknPokeDoc)));
	return (CPeeknPokeDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPeeknPokeView message handlers

void CPeeknPokeView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	CPeeknPokeDoc* pDoc = GetDocument();
	CClientDC	ClientDC(this);
	char str[256];

	switch( nChar )
		{
		case VK_LEFT:
			if( m_Caret > 0 )
				m_Caret--;
			break;

		case VK_RIGHT:
			if( m_Caret < pDoc->m_TextLine.GetLength())
				m_Caret++;
			break;

		case VK_DELETE:
			if( m_Caret >= 0 && pDoc->m_TextLine.GetLength() > m_Caret )
				pDoc->m_TextLine.Delete(m_Caret, 1);
			break;

		case VK_UP:
			iLineNum? iLineNum--: 0;
			strcpy(str, pDoc->m_CmdArray[iLineNum]);
			m_Caret = strlen(str)-1;
			str[m_Caret] = '\0';
			pDoc->m_TextLine = str;
			break;

		case VK_DOWN:
			if( (pDoc->iCmdIndex == iLineNum+1) || (pDoc->iCmdIndex == iLineNum) )
			{
				iLineNum = pDoc->iCmdIndex;
				pDoc->m_TextLine.Empty();
				m_Caret = 0;
			}
			else
			{
				iLineNum++;
				strcpy(str, pDoc->m_CmdArray[iLineNum]);
				m_Caret = strlen(str)-1;
				str[m_Caret] = '\0';
				pDoc->m_TextLine = str;
			}
			break;

		case VK_HOME:
			m_Caret = 0;
			break;

		case VK_END:
			m_Caret = pDoc->m_TextLine.GetLength();
			break;

		case VK_PRIOR:			// PgUp key
			break;

		case VK_NEXT:
			break;
	} 
	pDoc->UpdateAllViews(NULL);

	CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CPeeknPokeView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CPeeknPokeDoc* pDoc = GetDocument();
	CClientDC ClientDC( this );

	switch( nChar )
	{
		case  8:			// backspace
			if( m_Caret > 0 )
			{
				m_Caret--;
				pDoc->m_TextLine.Delete(m_Caret, 1);
			}
			break;

		case 13:			// CR
			pDoc->m_TextLine.Insert(pDoc->m_TextLine.GetLength(), nChar);//m_Caret

			// If the previous character is \ then display and continue
			//if( pDoc->m_TextLine[GetLength()-1] == "\" )
			// Else then process the command

			// Add to the document
			pDoc->m_CmdArray[pDoc->iCmdIndex++] = pDoc->m_TextLine;
			pDoc->m_LineList.AddTail( ">" + pDoc->m_TextLine );

			// Send the command to the parser
			BeginWaitCursor();
			pDoc->m_Parser.ProcessCommand(pDoc->m_TextLine);
			EndWaitCursor();

			pDoc->m_TextLine.Empty();
			m_Caret = 0;
			pDoc->SetModifiedFlag(TRUE);
			iLineNum = pDoc->iCmdIndex;
			break;

		default:
			pDoc->m_TextLine.Insert(m_Caret, nChar);
			m_Caret++;
			break;
	}
	pDoc->UpdateAllViews(NULL);

	CScrollView::OnChar(nChar, nRepCnt, nFlags);
}

void CPeeknPokeView::OnSetFocus(CWnd* pOldWnd) 
{
	CScrollView::OnSetFocus(pOldWnd);
	
	// TODO: Add your message handler code here
	CreateSolidCaret(m_XCaret, m_YCaret);
	SetCaretPos(m_CaretPos);
	ShowCaret();
}

void CPeeknPokeView::OnKillFocus(CWnd* pNewWnd) 
{
	CScrollView::OnKillFocus(pNewWnd);
	
	// TODO: Add your message handler code here
	::DestroyCaret();
}

int CPeeknPokeView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO: Add your specialized creation code here
	CPeeknPokeDoc* pDoc = GetDocument();
	pDoc->pPnPView = this;
	CClientDC ClientDC(this);
	TEXTMETRIC TM;

	m_XCaret = ::GetSystemMetrics( SM_CXBORDER) * 2;
	ClientDC.GetTextMetrics( &TM );
	m_YCaret = TM.tmHeight + TM.tmExternalLeading;

	return 0;
}

void CPeeknPokeView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CScrollView::OnLButtonDown(nFlags, point);
}

void CPeeknPokeView::DisplayMessage(LPCTSTR msg)
{
	CPeeknPokeDoc* pDoc = GetDocument();
	CString str = "    " + CString(msg);

	pDoc->m_LineList.AddTail( str );
	pDoc->UpdateAllViews(NULL);
}
