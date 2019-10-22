#pragma once
#include "afxwin.h"
#include "resource.h"

// AddDbColDlg dialog

class AddDbColDlg : public CDialog
{
	DECLARE_DYNAMIC(AddDbColDlg)

public:
	AddDbColDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~AddDbColDlg();

// Dialog Data
	enum { IDD = IDD_ADDDBCOL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   void EnableCtrls();

	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnBnClickedAdd();
   virtual BOOL OnInitDialog();
   CComboBox m_type;
   afx_msg void OnCbnSelchangeType();
   CEdit m_width;
   CComboBox m_layers;
   afx_msg void OnCbnSelchangeLayers();
   };
