#pragma once
#include "afxwin.h"
#include "resource.h"

class MapLayer;

// FieldCalculatorDlg dialog

class FieldCalculatorDlg : public CDialog
{
	DECLARE_DYNAMIC(FieldCalculatorDlg)

public:
	FieldCalculatorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~FieldCalculatorDlg();

// Dialog Data
	enum { IDD = IDD_CALCULATEFIELD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

   MapLayer *m_pLayer;

   void LoadFieldInfo();

public:
   CComboBox m_layers;
   CComboBox m_fields;
   CEdit m_formula;
   CListBox m_fields1;
   CListBox m_functions;
   CListBox m_operators;
   CEdit m_query;
   afx_msg void OnCbnSelchangeLayers();
   afx_msg void OnCbnSelchangeLayers1();
   afx_msg void OnLbnDblclkFields1();
   afx_msg void OnLbnDblclkFunctions();
   afx_msg void OnLbnDblclkOps();
   virtual BOOL OnInitDialog();
protected:
   virtual void OnOK();
   };
