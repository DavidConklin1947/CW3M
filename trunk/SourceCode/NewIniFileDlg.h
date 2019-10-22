#if !defined(AFX_NewIniFileDlg_H__45446545_7C09_11D3_95B9_00A076B0010A__INCLUDED_)
#define AFX_NewIniFileDlg_H__45446545_7C09_11D3_95B9_00A076B0010A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewIniFileDlg.h : header file
//


#include "PPNewIniStep1.h"
#include "PPNewIniStep2.h"
#include "PPNewIniStep3.h"
#include "PPNewIniFinish.h"

/////////////////////////////////////////////////////////////////////////////
// NewIniFileDlg

class NewIniFileDlg : public CPropertySheet
{
	DECLARE_DYNAMIC(NewIniFileDlg)

// Construction
public:
	NewIniFileDlg(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	NewIniFileDlg(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

   PPNewIniStep1      m_step1;
   PPNewIniStep2      m_step2;
   //PPNewIniStep3      m_step3;
   PPNewIniFinish     m_finish;
   
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NewIniFileDlg)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
   virtual ~NewIniFileDlg() {}

	// Generated message map functions
protected:
	//{{AFX_MSG(NewIniFileDlg)
	afx_msg void OnApplyNow();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NewIniFileDlg_H__45446545_7C09_11D3_95B9_00A076B0010A__INCLUDED_)
