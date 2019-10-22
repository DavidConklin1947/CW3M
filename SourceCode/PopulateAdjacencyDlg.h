#pragma once

#include "resource.h"
#include <map.h>


// PopulateAdjacencyDlg dialog

class PopulateAdjacencyDlg : public CDialog
{
	DECLARE_DYNAMIC(PopulateAdjacencyDlg)

public:
	PopulateAdjacencyDlg(Map *pMap, CWnd* pParent = NULL);   // standard constructor
	virtual ~PopulateAdjacencyDlg();

// Dialog Data
	enum { IDD = IDD_ADJACENCY };

private:
   Map *m_pMap;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

   CComboBox m_sourceLayerCombo;
   CComboBox m_targetLayerCombo;
   CComboBox m_targetFieldCombo;
   virtual BOOL OnInitDialog();
   afx_msg void OnCbnSelchangePapTargetlayer();
   afx_msg void OnCalculate();
   afx_msg void OnExit();
   CEdit m_query;
};
