// CacheTestDoc.h : interface of the CCacheTestDoc class
//
/////////////////////////////////////////////////////////////////////////////

class CCacheTestDoc : public CDocument
{
protected: // create from serialization only
	CCacheTestDoc();
	DECLARE_DYNCREATE(CCacheTestDoc)

// Attributes
public:
	CString	m_statistics;
	UINT m_cache_number;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCacheTestDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCacheTestDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CCacheTestDoc)
	afx_msg void OnCacheStats();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
