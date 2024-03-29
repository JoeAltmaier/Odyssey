// CacheTestView.h : interface of the CCacheTestView class
//
/////////////////////////////////////////////////////////////////////////////

class CCacheTestView : public CScrollView
{
protected: // create from serialization only
	CCacheTestView();
	DECLARE_DYNCREATE(CCacheTestView)

// Attributes
public:
	CCacheTestDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCacheTestView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

    afx_msg void OnSize (UINT, int, int);
	afx_msg void OnTimer(UINT nTimerID);

	// Implementation
public:
	virtual ~CCacheTestView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CCacheTestView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in CacheTestView.cpp
inline CCacheTestDoc* CCacheTestView::GetDocument()
   { return (CCacheTestDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
