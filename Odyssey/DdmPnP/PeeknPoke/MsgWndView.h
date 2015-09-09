#if !defined(AFX_MSGWNDVIEW_H__0A486094_D747_11D2_9BF8_00105A2459CB__INCLUDED_)
#define AFX_MSGWNDVIEW_H__0A486094_D747_11D2_9BF8_00105A2459CB__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MsgWndView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// MsgWndView view
#include "PeeknPokeDoc.h"

class MsgWndView : public CEditView
{
protected:
	MsgWndView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(MsgWndView)

// Attributes
public:
	CPeeknPokeDoc* GetDocument();

// Operations
public:
	void DisplayMessage(LPCTSTR msg);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MsgWndView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~MsgWndView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(MsgWndView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LONG OnOtherMessage(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in MsgWndView.cpp
inline CPeeknPokeDoc* MsgWndView::GetDocument()
   { return (CPeeknPokeDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MSGWNDVIEW_H__0A486094_D747_11D2_9BF8_00105A2459CB__INCLUDED_)
