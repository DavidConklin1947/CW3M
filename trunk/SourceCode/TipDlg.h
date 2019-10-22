// XHTMLTipOfTheDayDlg.h  Version 1.0
//
// This software is released into the public domain.  You are free to use it
// in any way you like, except that you may not sell this source code.
//
// This software is provided "as is" with no expressed or implied warranty.
// I accept no liability for any damage or loss of business that this software
// may cause.
///////////////////////////////////////////////////////////////////////////////

#ifndef XHTMLTIPOFTHEDAYDLG_H
#define XHTMLTIPOFTHEDAYDLG_H

#include "XGlyphButton.h"
#include "XHTMLStatic.h"

///////////////////////////////////////////////////////////////////////////////
// CXHTMLTipOfTheDayDlg dialog

class CXHTMLTipOfTheDayDlg : public CDialog
{
// Construction
public:
	CXHTMLTipOfTheDayDlg(LPCTSTR lpszTipFile, CWnd* pParent = NULL);
	virtual ~CXHTMLTipOfTheDayDlg();

// Dialog Data
	//{{AFX_DATA(CXHTMLTipOfTheDayDlg)
	BOOL			m_bStartup;
	CString			m_strTip;
	//}}AFX_DATA
	CButton			m_chkStartup;
	CXGlyphButton	m_btnNext;
	CXGlyphButton	m_btnPrev;

// Attributes
	void SetStartup(BOOL bStartup) { m_bStartup = bStartup; }
	void SetAppCommands(XHTMLSTATIC_APP_COMMAND * paAppCommands, int nAppCommands)
	{
		m_Tip.SetAppCommands(paAppCommands, nAppCommands);
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXHTMLTipOfTheDayDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void			DisplayHeader(int nHeader);
	void			DoPaint (CDC *pdc);
	int				GetLastTipNo();
	TCHAR *			GetTipLine(TCHAR *buffer, int n);
	BOOL			GetTipString(int* pnTip, CString& str, BOOL bFirstTime);

	BOOL			m_bVisible;
	int				m_nTipNo;
	int				m_nLastTipNo;
	FILE*			m_pStream;
	CString			m_strTipFile;
	CStatic			m_Side;
	CXHTMLStatic	m_Header;
	CXHTMLStatic	m_Tip;
	CBrush			m_SideBrush;

	// Generated message map functions
	//{{AFX_MSG(CXHTMLTipOfTheDayDlg)
	afx_msg void OnNextTip();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnPrevTip();
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};

#endif //XHTMLTIPOFTHEDAYDLG_H
