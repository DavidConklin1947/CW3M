#pragma once
#include "afxwin.h"


// SaveProjectDlg dialog

class SaveProjectDlg : public CDialogEx
{
	DECLARE_DYNAMIC(SaveProjectDlg)

public:
	SaveProjectDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~SaveProjectDlg();

// Dialog Data
	enum { IDD = IDD_SAVEPROJECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CString m_projectPath;
   CString m_lulcPath;
   CString m_policyPath;
   CString m_scenarioPath;
   afx_msg void OnBnClickedBrowseenvx();
   afx_msg void OnBnClickedBrowselulc();
   afx_msg void OnBnClickedBrowsepolicy();
   afx_msg void OnBnClickedBrowsescenario();
   virtual BOOL OnInitDialog();
   virtual void OnOK();

   CButton m_cbLulcNoSave;
   BOOL m_lulcNoSave;
   CButton m_cbPolicyNoSave;
   BOOL m_policyNoSave;
   CButton m_cbScenarioNoSave;
   BOOL m_scenarioNoSave;

   afx_msg void UpdateUI();
   };
