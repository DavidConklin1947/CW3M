#pragma once

#include "resource.h"

// RemoveDbColsDlg dialog

class RemoveDbColsDlg : public CDialog
{
	DECLARE_DYNAMIC(RemoveDbColsDlg)

public:
	RemoveDbColsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~RemoveDbColsDlg();

// Dialog Data
	enum { IDD = IDD_REMOVEDBCOL };

   CComboBox m_layers;
   CCheckListBox m_fields;

   virtual BOOL OnInitDialog();
   virtual void OnOK();

   afx_msg void OnCbnSelchangeLayers();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   void LoadFields();
	DECLARE_MESSAGE_MAP()
};
