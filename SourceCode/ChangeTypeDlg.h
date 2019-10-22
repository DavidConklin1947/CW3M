#pragma once
#include "afxwin.h"


// ChangeTypeDlg dialog

class ChangeTypeDlg : public CDialog
{
	DECLARE_DYNAMIC(ChangeTypeDlg)

public:
	ChangeTypeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~ChangeTypeDlg();

// Dialog Data
	enum { IDD = IDD_CHANGETYPE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   void LoadFields();
   void LoadField();
   void EnableCtrls();

	DECLARE_MESSAGE_MAP()
public:
   CComboBox m_layers;
   CComboBox m_fields;
   CString m_oldType;
   CComboBox m_type;
   int m_width;
   CString m_format;

   virtual BOOL OnInitDialog();
   afx_msg void OnCbnSelchangeType();
   afx_msg void OnCbnSelchangeLayers();
   afx_msg void OnCbnSelchangeFields();
   virtual void OnOK();
   BOOL m_useFormat;
   };
