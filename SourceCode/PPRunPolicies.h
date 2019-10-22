#pragma once

#include "resource.h"
#include "afxwin.h"

class Policy;

// PPolicyUse dialog

class PPRunPolicies : public CPropertyPage
{
	DECLARE_DYNAMIC(PPRunPolicies)

public:
	PPRunPolicies();
	virtual ~PPRunPolicies();

// Dialog Data
	enum { IDD = IDD_RUNPOLICIES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:
   CCheckListBox m_policies;

public:
   virtual BOOL OnInitDialog();
   int SetPoliciesToUse(void);
   afx_msg void OnBnClickedSelectall();
   afx_msg void OnBnClickedSelectnone();

protected:
   bool ShowPolicy( Policy *pPolicy );
   void LoadPolicies();

public:
   afx_msg void OnBnClickedShowmypolicies();
   afx_msg void OnBnClickedShowsharedpolicies();
};
