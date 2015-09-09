// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__FBA88EFF_D28B_11D2_9BF6_00105A2459CB__INCLUDED_)
#define AFX_MAINFRM_H__FBA88EFF_D28B_11D2_9BF6_00105A2459CB__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//#include "ProgressStatusBar.h"

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only

	CMainFrame();
	void OnUpdateTime(CCmdUI *pCmdUI);
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	//CProgressStatusBar *GetStatusBar()  { return & m_wndStatusBar; }
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	//CProgressStatusBar  m_wndStatusBar;
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CSplitterWnd	m_SplitterWnd;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__FBA88EFF_D28B_11D2_9BF6_00105A2459CB__INCLUDED_)
