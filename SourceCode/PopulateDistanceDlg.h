#pragma once

#include "resource.h"
#include <map.h>
#include "afxwin.h"
#include "afxcmn.h"

// PopulateDistanceDlg dialog

class PopulateDistanceDlg : public CDialog
{
	DECLARE_DYNAMIC(PopulateDistanceDlg)

public:
	PopulateDistanceDlg( Map *pMap, CWnd* pParent = NULL);   // standard constructor
	virtual ~PopulateDistanceDlg();

// Dialog Data
	enum { IDD = IDD_DISTANCEDLG };

private:
   Map *m_pMap;

   QueryEngine *m_pSourceQueryEngine;
   QueryEngine *m_pTargetQueryEngine;   // IDU
   
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CComboBox m_targetLayerCombo;
   CComboBox m_sourceLayerCombo;
   CComboBox m_targetFieldCombo;
   CComboBox m_indexFieldCombo;
   CComboBox m_valueFieldCombo;
   CComboBox m_sourceValueFieldCombo;
   virtual BOOL OnInitDialog();
   afx_msg void OnCbnSelchangePddTargetlayer();
   afx_msg void OnCbnSelchangePddSourcelayer();
   afx_msg void OnCalculate();
   afx_msg void OnExit();
   CEdit m_qTarget;
   CEdit m_qSource;
   afx_msg void OnBnClickedPopulatenearestindex();
   afx_msg void OnBnClickedPopulatenearestvalue();
   float m_thresholdDistance;
   afx_msg void OnBnClickedPopulatecount();
   CEdit m_editDistance;
   CProgressCtrl m_progress;
   CStatic m_status;
   afx_msg void OnBnClickedBrowse();
   };
