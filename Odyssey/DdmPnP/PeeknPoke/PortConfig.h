#if !defined(AFX_PORTCONFIG_H__BBBB3863_D80F_11D2_9BF8_00105A2459CB__INCLUDED_)
#define AFX_PORTCONFIG_H__BBBB3863_D80F_11D2_9BF8_00105A2459CB__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PortConfig.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PortConfig dialog
#include "SerialPort.h"

class PortConfig : public CDialog
{
protected:

// Construction
public:
	PortConfig(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(PortConfig)
	enum { IDD = IDD_DIALOG_CONFIG };
	//CString	m_Baud;
	int		m_ComPort;
	//CString	m_Data;
	//CString	m_Parity;
	//CString	m_StopBit;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PortConfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(PortConfig)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORTCONFIG_H__BBBB3863_D80F_11D2_9BF8_00105A2459CB__INCLUDED_)
