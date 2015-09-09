// ProgressStatusBar.cpp : implementation file
//

#include "stdafx.h"
#include "PeeknPoke.h"
//#include "ProgressStatusBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProgressStatusBar

CProgressStatusBar::CProgressStatusBar()
{
	m_bProgressMode = FALSE;
	m_nProgressCtrlWidth = PROGRESS_CTRL_CX;
}

CProgressStatusBar::~CProgressStatusBar()
{
}


BEGIN_MESSAGE_MAP(CProgressStatusBar, CStatusBar)
	//{{AFX_MSG_MAP(CProgressStatusBar)
	ON_WM_CREATE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CProgressStatusBar message handlers

void CProgressStatusBar::SetProgressCtrlWidth(UINT nWidth)
{
	m_nProgressCtrlWidth = nWidth;
}

void CProgressStatusBar::RecalcProgressDisplay()
{
	CRect ControlRect;
	CRect ClientRect;
	GetClientRect(&ClientRect);
	ControlRect = ClientRect;

	// The Progress bar
	ControlRect.left += X_MARGIN;
	ControlRect.right = ControlRect.left + m_nProgressCtrlWidth;
	ControlRect.top += Y_MARGIN;
	ControlRect.left -= Y_MARGIN;

	m_ProgressCtrl.MoveWindow(ControlRect, FALSE);

	// The text label
	ControlRect.left = ControlRect.right + X_MARGIN;
	ControlRect.right = ClientRect.right - X_MARGIN;
	m_ProgressLabel.MoveWindow(ControlRect, FALSE);
}

void CProgressStatusBar::SetProgressLabel(LPCSTR lpszProgressLabel)
{
	m_ProgressLabel.SetWindowText( lpszProgressLabel );
	if( m_bProgressMode )
	{
		RecalcProgressDisplay();
		Invalidate();
		UpdateWindow();
	}
}

void CProgressStatusBar::ShowProgressDisplay(BOOL bShow)
{
	m_bProgressMode = bShow;

	if( m_bProgressMode )
		RecalcProgressDisplay();

	m_ProgressLabel.ShowWindow( m_bProgressMode? SW_SHOW : SW_HIDE );
	m_ProgressCtrl.ShowWindow( m_bProgressMode? SW_SHOW : SW_HIDE );

	Invalidate();
	UpdateWindow();
}

int CProgressStatusBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CStatusBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	// Create the Progress Control
	if( !m_ProgressCtrl.Create(
		0,					// Style
							// Position or percent
		CRect(0, 0, 0, 0),	// Initial position
		this,				// Parent
		0) )				// Child ID
	{
		return -1;
	}

	// Create the Progress Label
	if( !m_ProgressLabel.Create(
		NULL,				// Text
		WS_CHILD | SS_LEFT,	// Style
		CRect(0, 0, 0, 0),	// Initial position
		this) )				// Parent
	{
		return -1;
	}

	m_ProgressLabel.SetFont( GetFont() );

	return 0;
}

void CProgressStatusBar::OnPaint() 
{
	//CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	if( !m_bProgressMode )
		CStatusBar::OnPaint();
	// Do not call CStatusBar::OnPaint() for painting messages
}
