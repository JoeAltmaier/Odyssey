// ReadSequential.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CReadSequential dialog

class CReadSequential : public CDialog
{
// Construction
public:
	CReadSequential(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CReadSequential)
	enum { IDD = IDD_DIALOG4 };
	UINT	m_starting_page;
	UINT	m_num_pages;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReadSequential)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CReadSequential)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
