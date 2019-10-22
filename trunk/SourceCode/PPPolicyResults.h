#if !defined(AFX_PPPOLICYRESULTS_H__8367DD63_49AC_439C_ACBB_E7C7FF530653__INCLUDED_)
#define AFX_PPPOLICYRESULTS_H__8367DD63_49AC_439C_ACBB_E7C7FF530653__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PPPolicyResults.h : header file
//


#include "resource.h"
#include "afxcmn.h"


/////////////////////////////////////////////////////////////////////////////
// PPPolicyResults dialog

class PPPolicyResults : public CPropertyPage
{
	DECLARE_DYNCREATE(PPPolicyResults)

// Construction
public:
   PPPolicyResults() { }
	PPPolicyResults(int cell, int run=-1 );
	~PPPolicyResults();

// Dialog Data
	enum { IDD = IDD_POLICYRESULTS };
	CButton	m_prevCell;
	CButton	m_nextCell;
	CButton	m_flashCell;

   int   m_cell;
   int   m_run;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(PPRuleResults)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
   void Load( void );

	// Generated message map functions
	//{{AFX_MSG(PPRuleResults)
	virtual BOOL OnInitDialog();
	afx_msg void OnNextcell();
	afx_msg void OnPrevcell();
	afx_msg void OnFlashcell();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
   CListCtrl m_results;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PPRULERESULTS_H__8367DD63_49AC_439C_ACBB_E7C7FF530653__INCLUDED_)
