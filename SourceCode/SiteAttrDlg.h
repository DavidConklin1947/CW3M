#pragma once
#include "afxwin.h"
#include "resource.h"
#include "EnvEngine\policy.h"


// SiteAttrDlg dialog

class SiteAttrDlg : public CDialog
{
	DECLARE_DYNAMIC(SiteAttrDlg)

public:
	SiteAttrDlg(Policy *pPolicy, CWnd* pParent = NULL);   // standard constructor
	virtual ~SiteAttrDlg();

// Dialog Data
	enum { IDD = IDD_SITEATTR };

protected:
   Policy *m_pPolicy;

   void LoadFieldValues();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedAdd();
   afx_msg void OnCbnSelchangeFields();
   CComboBox m_fields;
   CComboBox m_ops;
   CComboBox m_values;
   CEdit m_query;
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedAnd();
   afx_msg void OnBnClickedOr();
   afx_msg void OnBnClickedSpatialops();
};
