#pragma once
#include "afxwin.h"
#include "resource.h"

#include "EnvEngine\policy.h"

#include <TabPageSSL.h>

class PolEditor;

// PPPolicyScenarios dialog

class PPPolicyScenarios : public CTabPageSSL
{
	DECLARE_DYNAMIC(PPPolicyScenarios)

public:
	PPPolicyScenarios( PolEditor*, Policy *&pPolicy );
	virtual ~PPPolicyScenarios();

// Dialog Data
	enum { IDD = IDD_POLICY_SCENARIOS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
   Policy *&m_pPolicy;
   PolEditor *m_pParent;

public:
   void LoadPolicy();
   bool StoreChanges();

protected:
//   void MakeDirty();

	DECLARE_MESSAGE_MAP()
public:
   CCheckListBox m_scenarios;
   virtual BOOL OnInitDialog();

   afx_msg void OnCheckScenario();
   };
