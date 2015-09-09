// MainFrm.cpp : implementation of the CMainFrame class
//
#include "stdafx.h"
#include "PeeknPoke.h"

#include "MainFrm.h"

#include "MsgWndView.h"
#include "PeeknPokeDoc.h"
#include "PeeknPokeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_TIME, OnUpdateTime)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	//ID_RUNNING,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
	ID_INDICATOR_TIME,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	// Update the clock
	SetTimer(123, 1000, NULL);
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	return CFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	// create a splitter with 2 rows, 1 column
	if (m_SplitterWnd.CreateStatic(this, 2, 1, WS_CHILD))
	{
		CRect	rect;
		GetClientRect(&rect);
		CSize	sizeWnd = rect.Size();

		sizeWnd.cy = rect.Height()*3/4;

		if (!m_SplitterWnd.CreateView(0, 0, RUNTIME_CLASS(CPeeknPokeView), sizeWnd, pContext) )
		{
			TRACE0("Failed to create first pane\n");
			return FALSE;
		}

		// add the second splitter pane - an input view in column 1
		if (!m_SplitterWnd.CreateView(1, 0, RUNTIME_CLASS(MsgWndView), CSize(0,0), pContext) )
		{
			TRACE0("Failed to create second pane\n");
			return FALSE;
		}
	}

	// activate the input view
	SetActiveView((CView *)m_SplitterWnd.GetPane(0,0));

	//show the splitter
	m_SplitterWnd.ShowWindow(SW_SHOWNORMAL);
	m_SplitterWnd.UpdateWindow();

	return TRUE;
}

void CMainFrame::OnUpdateTime(CCmdUI *pCmdUI)
{
	CTime	time = CTime::GetCurrentTime();
	CString	sTime = time.Format("%I:%M %p");
	pCmdUI->SetText(sTime);
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default

	CFrameWnd::OnTimer(nIDEvent);
}
