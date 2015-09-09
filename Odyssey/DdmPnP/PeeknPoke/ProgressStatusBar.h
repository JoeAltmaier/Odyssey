#if !defined(AFX_PROGRESSSTATUSBAR_H__C7D2E311_2824_11D3_9C0A_00105A2459CB__INCLUDED_)
#define AFX_PROGRESSSTATUSBAR_H__C7D2E311_2824_11D3_9C0A_00105A2459CB__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ProgressStatusBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProgressStatusBar window

const int PROGRESS_CTRL_CX = 160;
const int X_MARGIN = 5;
const int Y_MARGIN = 2;

class CProgressStatusBar : public CStatusBar
{
// Construction
public:
	CProgressStatusBar();

// Attributes
protected:
	CProgressCtrl	m_ProgressCtrl;
	CStatic			m_ProgressLabel;
	BOOL			m_bProgressMode;
	int				m_nProgressCtrlWidth;
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgressStatusBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	void ShowProgressDisplay( BOOL bShow = TRUE );
	void SetProgressLabel( LPCSTR lpszProgressLabel );
	void RecalcProgressDisplay();
	void SetProgressCtrlWidth( UINT nWidth = PROGRESS_CTRL_CX );
	CProgressCtrl *GetProgressCtrl() { return &m_ProgressCtrl; }
	virtual ~CProgressStatusBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CProgressStatusBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESSSTATUSBAR_H__C7D2E311_2824_11D3_9C0A_00105A2459CB__INCLUDED_)
