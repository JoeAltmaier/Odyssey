// PeeknPokeView.h : interface of the CPeeknPokeView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PEEKNPOKEVIEW_H__FBA88F03_D28B_11D2_9BF6_00105A2459CB__INCLUDED_)
#define AFX_PEEKNPOKEVIEW_H__FBA88F03_D28B_11D2_9BF6_00105A2459CB__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CPeeknPokeDoc;

class CPeeknPokeView : public CScrollView
{
protected: // create from serialization only
	CPeeknPokeView();
	DECLARE_DYNCREATE(CPeeknPokeView)

// Attributes
protected:
	int		MARGINX;
	int		iLineNum;
	POINT	m_CaretPos;
	int		m_XCaret, m_YCaret;
	int		m_Caret;
	CSize	m_ViewCharSize;
	CSize	m_DocSize;

public:
	CPeeknPokeDoc* GetDocument();

// Operations
public:
	void DisplayMessage(LPCTSTR msg);
	CSize GetDocSize() const { return m_DocSize; }
	CSize GetCharSize() const { return m_ViewCharSize; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPeeknPokeView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	//CPoint GetCaretPos(CDC *pDC, long xoffset);
	virtual ~CPeeknPokeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	void ComputeVisibleLines(CDC* pDC, int& nFirst, int& nLast);
	//{{AFX_MSG(CPeeknPokeView)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in PeeknPokeView.cpp
inline CPeeknPokeDoc* CPeeknPokeView::GetDocument()
   { return (CPeeknPokeDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PEEKNPOKEVIEW_H__FBA88F03_D28B_11D2_9BF6_00105A2459CB__INCLUDED_)
