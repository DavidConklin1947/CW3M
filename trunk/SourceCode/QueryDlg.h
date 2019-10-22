#pragma once
#include "afxwin.h"
#include "resource.h"

class MapLayer;

// QueryDlg dialog

class QueryDlg : public CDialog
{
	DECLARE_DYNAMIC(QueryDlg)

public:
	QueryDlg(MapLayer *pLayer, int record = -1, CWnd* pParent = NULL);   // standard constructor
	virtual ~QueryDlg();

// Dialog Data
	enum { IDD = IDD_QUERYBUILDER };

   int m_record;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   void LoadLayer();
   void LoadFieldValues();
   void LoadHistory();

   MapLayer *m_pLayer;

   DECLARE_MESSAGE_MAP()
public:
   CString m_queryString;

   static CStringArray m_queryStringArray;

   CComboBox m_layers;
   CComboBox m_ops;
   CComboBox m_values;
   CEdit m_query;
   CComboBox m_fields;
   CComboBox m_historyCombo;

   virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedAdd();
   afx_msg void OnCbnSelchangeFields();
   afx_msg void OnCbnSelchangeLayers();
   afx_msg void OnBnClickedAnd();
   afx_msg void OnBnClickedOr();

protected:
   virtual void OnOK();
public:
   BOOL m_runGlobal;
   BOOL m_clearPrev;
   afx_msg void OnBnClickedSpatialops();
   afx_msg void OnBnClickedQueryviewer();
   afx_msg void OnBnClickedCancel();
   afx_msg void OnCbnSelchangeHistory();
   };
