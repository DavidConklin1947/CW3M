#pragma once
#include "afxwin.h"
#include "resource.h"

class MapLayer;

// PolQueryDlg dialog

class PolQueryDlg : public CDialog
{
	DECLARE_DYNAMIC(PolQueryDlg)

public:
	PolQueryDlg(MapLayer *pLayer, CWnd* pParent = NULL);   // standard constructor
	virtual ~PolQueryDlg();

// Dialog Data
	enum { IDD = IDD_POLQUERYBUILDER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   void LoadFieldValues();

   MapLayer *m_pLayer;

   DECLARE_MESSAGE_MAP()
public:
   CString m_queryString;

   CComboBox m_ops;
   CComboBox m_values;
   CEdit m_query;
   CComboBox m_fields;

   virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedAdd();
   afx_msg void OnCbnSelchangeFields();
   afx_msg void OnBnClickedAnd();
   afx_msg void OnBnClickedOr();

protected:
   virtual void OnOK();
public:
   afx_msg void OnBnClickedSpatialops();
   afx_msg void OnBnClickedQueryviewer();
   };
