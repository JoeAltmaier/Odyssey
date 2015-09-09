// CacheConfig.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCacheConfig dialog

class CCacheConfig : public CDialog
{
// Construction
public:
	CCacheConfig(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCacheConfig)
	enum { IDD = IDD_DIALOG1 };
	UINT	m_page_size;
	UINT	m_num_pages;
	UINT	m_num_reserve_pages;
	UINT	m_memory_size_to_allocate;
	UINT	m_dirty_page_error_threshold;
	UINT	m_dirty_page_writeback_threshold;
	UINT	m_hash_table_size;
	int		m_write_back;
	int		m_write_through;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCacheConfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCacheConfig)
	afx_msg void OnWriteBack();
	afx_msg void OnWriteThrough();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
