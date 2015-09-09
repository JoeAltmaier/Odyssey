// CacheTestView.cpp : implementation of the CCacheTestView class
//

#include "Simple.h"
#include "stdafx.h"
#include "CacheTest.h"

#include "CacheTestDoc.h"
#include "CacheTestView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "CmTest.h"

/////////////////////////////////////////////////////////////////////////////
// CCacheTestView

IMPLEMENT_DYNCREATE(CCacheTestView, CScrollView)

BEGIN_MESSAGE_MAP(CCacheTestView, CScrollView)
	//{{AFX_MSG_MAP(CCacheTestView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
	ON_WM_SIZE()
	ON_WM_TIMER()
    ON_WM_VSCROLL ()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCacheTestView construction/destruction

CCacheTestView::CCacheTestView()
{
	// TODO: add construction code here

}

CCacheTestView::~CCacheTestView()
{
}

BOOL CCacheTestView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CCacheTestView drawing

void CCacheTestView::OnDraw(CDC* pDC)
{
	CCacheTestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
	CRect rect;
	GetClientRect(rect);
	pDC->SetBkMode(TRANSPARENT);

	// Create a font
	CFont font;

	// Create a logical font structure for a fixed-pitch font.
	LOGFONT logical_font;
	memset(&logical_font, 0, sizeof(logical_font));
	logical_font.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
	font.CreateFontIndirect(&logical_font);

	TEXTMETRIC tm;
	CClientDC dc (this);
	dc.GetTextMetrics (&tm);
	int cyChar = tm.tmHeight;

	// Set size of rectangle so that scrolling will work.
	// Eventually, get this from the document.  For now, a constant.
	int num_scroll_lines_in_window = 50;
	rect.SetRect(
		rect.TopLeft().x, //  x-coordinate of the upper-left corner
		rect.TopLeft().y, //  y-coordinate of the upper-left corner.
		rect.BottomRight().x, //  x-coordinate of the lower-right corner
		cyChar * num_scroll_lines_in_window);// y-coordinate of the lower-right corner
	pDC->SelectObject( &font );
	pDC->DrawText(pDoc->m_statistics, -1, &rect, 0);

}

/////////////////////////////////////////////////////////////////////////////
// CCacheTestView printing

BOOL CCacheTestView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CCacheTestView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CCacheTestView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CCacheTestView diagnostics

#ifdef _DEBUG
void CCacheTestView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CCacheTestView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CCacheTestDoc* CCacheTestView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCacheTestDoc)));
	return (CCacheTestDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCacheTestView message handlers

void CCacheTestView::OnSize(UINT nType, int cx, int cy)
{
	CCacheTestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// find out how many lines are to be displayed.
	// Eventually, get this from the document.  For now, a constant.
	int num_scroll_lines_in_window = 50;

	TEXTMETRIC tm;
	CClientDC dc (this);
	dc.GetTextMetrics (&tm);
	int cyChar = tm.tmHeight;

	SetScrollSizes(MM_TEXT, CSize(0, cyChar * num_scroll_lines_in_window),
		CSize(0, cy), CSize (0, cyChar));

}

void CCacheTestView::OnTimer(UINT nTimerID)
{
	CCacheTestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Make the statistics string empty
	pDoc->m_statistics.Empty();

	STATUS status = Get_Cache_Stats(&pDoc->m_statistics, pDoc->m_cache_number);
	//UpdateAllViews(NULL);	

}
