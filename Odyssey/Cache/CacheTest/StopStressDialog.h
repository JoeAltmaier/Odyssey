#if !defined(AFX_STOPSTRESSDIALOG_H__15C1A8E1_64E4_11D2_A15A_C510913BF96D__INCLUDED_)
#define AFX_STOPSTRESSDIALOG_H__15C1A8E1_64E4_11D2_A15A_C510913BF96D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// StopStressDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStopStressDialog dialog

class CStopStressDialog : public CDialog
{
// Construction
public:
	CStopStressDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CStopStressDialog)
	enum { IDD = IDD_DIALOG9 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStopStressDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStopStressDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STOPSTRESSDIALOG_H__15C1A8E1_64E4_11D2_A15A_C510913BF96D__INCLUDED_)
