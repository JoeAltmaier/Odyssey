// WriteSequential.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWriteSequential dialog

class CWriteSequential : public CDialog
{
// Construction
public:
	CWriteSequential(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWriteSequential)
	enum { IDD = IDD_DIALOG3 };
	UINT	m_starting_page;
	UINT	m_num_pages;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWriteSequential)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWriteSequential)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
