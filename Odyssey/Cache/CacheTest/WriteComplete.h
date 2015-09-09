// WriteComplete.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWriteComplete dialog

// Define message for write complete
#define WM_WRITE_COMPLETE (WM_USER + 1)

class CWriteComplete : public CDialog
{
// Construction
public:
	CWriteComplete(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWriteComplete)
	enum { IDD = 0 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWriteComplete)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWriteComplete)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
