// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnFlashConfig();
	afx_msg void OnFlashTestconfig();
	afx_msg void OnFlashWritetest();
	afx_msg LONG OnFlashWriteComplete(WPARAM, LPARAM);
	afx_msg LONG OnFlashReadComplete(WPARAM, LPARAM);
	afx_msg LONG OnFlashFlushComplete(WPARAM, LPARAM);
	afx_msg LONG OnFlashCloseComplete(WPARAM, LPARAM);
	afx_msg LONG OnFlashStressComplete(WPARAM, LPARAM);
	afx_msg void OnFlashReadtest();
	afx_msg void OnFlashFlushcache();
	afx_msg void OnFlashClose();
	afx_msg void OnCacheStats();
	afx_msg void OnStressTest();
	afx_msg void OnClose( );
	afx_msg void OnDestroy( );
	afx_msg void OnTimer(UINT nTimerID);
	afx_msg void OnStopStressTest();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
