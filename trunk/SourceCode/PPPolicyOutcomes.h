#pragma once

#include "afxwin.h"
#include "resource.h"

#include "EnvEngine\policy.h"

#include <TabPageSSL.h>

class PolEditor;


// PPPolicyOutcomes dialog

class PPPolicyOutcomes : public CTabPageSSL
{
	DECLARE_DYNAMIC(PPPolicyOutcomes)

public:
	PPPolicyOutcomes( PolEditor*, Policy *&pPolicy );
	virtual ~PPPolicyOutcomes();

// Dialog Data
	enum { IDD = IDD_POLICYOUTCOME };
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   
protected:
   Policy *&m_pPolicy;
   PolEditor *m_pParent;

   CListBox m_outcomeList;
   CEdit    m_outcomes;

public:
   void LoadPolicy();
   void RefreshOutcomes();

   bool StoreChanges();

protected:
//   void MakeDirty();

protected:
   virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedAdd();
   afx_msg void OnBnClickedEdit();
   afx_msg void OnLbnSelchangeOutcomelist();
   afx_msg void OnEnChangeOutcomes();
	DECLARE_MESSAGE_MAP()

public:
   afx_msg void OnBnClickedDelete();
   afx_msg void OnBnClickedUpdate();
   };

