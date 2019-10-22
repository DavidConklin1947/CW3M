#if !defined(AFX_PPRUNSETUP_H__3C45B339_F359_42BE_B268_E43D6BEA3224__INCLUDED_)
#define AFX_PPRUNSETUP_H__3C45B339_F359_42BE_B268_E43D6BEA3224__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PPRunSetup.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PPRunSetup dialog

class PPRunSetup : public CPropertyPage
{
	DECLARE_DYNCREATE(PPRunSetup)

// Construction
public:
	PPRunSetup();
	~PPRunSetup();

// Dialog Data
	//{{AFX_DATA(PPRunSetup)
	enum { IDD = IDD_RUNSETUP };
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(PPRunSetup)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

protected:
   CCheckListBox m_models;
   CCheckListBox m_aps;

public:
   int SetModelsToUse(void);

public:
   //bool m_culturalEnabled;
   //int  m_culturalFrequency;
   //bool m_policyEnabled;
   //int  m_policyFrequency;

protected:
//   void EnableControls(void);
public:
//   afx_msg void OnBnClicked();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PPRUNSETUP_H__3C45B339_F359_42BE_B268_E43D6BEA3224__INCLUDED_)
