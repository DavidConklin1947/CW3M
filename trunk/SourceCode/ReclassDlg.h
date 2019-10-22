#pragma once
#include "afxwin.h"

class MapLayer;


// ReclassDlg dialog

class ReclassDlg : public CDialog
{
	DECLARE_DYNAMIC(ReclassDlg)

public:
	ReclassDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~ReclassDlg();

// Dialog Data
	enum { IDD = IDD_RECLASS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   MapLayer *m_pLayer;

   void LoadFieldInfo();
   
	DECLARE_MESSAGE_MAP()
public:
   CComboBox m_layers;
   CComboBox m_fields;
   //CEdit m_pairs;
   afx_msg void OnCbnSelchangeLayer();
   afx_msg void OnCbnSelchangeField();
   afx_msg void OnBnClickedExtract();
   virtual BOOL OnInitDialog();
   virtual void OnOK();
   CString m_fieldName;
   CString m_reclassPairs;
   afx_msg void OnBnClickedLoadfromfile();
   afx_msg void OnEnChangeReclassvalues();
   };
