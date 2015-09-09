#if !defined(AFX_STRESSDIALOG_H__5055D2E1_5960_11D2_A15A_444553540000__INCLUDED_)
#define AFX_STRESSDIALOG_H__5055D2E1_5960_11D2_A15A_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// StressDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStressDialog dialog

class CStressDialog : public CDialog
{
// Construction
public:
	CStressDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CStressDialog)
	enum { IDD = IDD_DIALOG8 };
	UINT	m_num_threads;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStressDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStressDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STRESSDIALOG_H__5055D2E1_5960_11D2_A15A_444553540000__INCLUDED_)
