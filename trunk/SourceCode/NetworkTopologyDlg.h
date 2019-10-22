#pragma once
#include "afxwin.h"

#include "resource.h"
#include <ReachTree.h>
#include <NetworkTree.h>
#include "afxcmn.h"


// NetworkTopologyDlg dialog

class NetworkTopologyDlg : public CDialog
{
	DECLARE_DYNAMIC(NetworkTopologyDlg)

public:
	NetworkTopologyDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~NetworkTopologyDlg();

// Dialog Data
	enum { IDD = IDD_REACHTREE };

public:
   //NetworkTree *m_pTree;
   ReachTree  *m_pTree;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   void LoadFields();
   void UpdateInterface();

	DECLARE_MESSAGE_MAP()
public:
   CComboBox m_layers;
   CComboBox m_colNodeID;
   CComboBox m_colToNode;
   CComboBox m_colOrder;
   CComboBox m_colTreeID;
   CComboBox m_colReachIndex;

   CEdit m_flowDirGrid;
   CButton m_populateOrder;
   CButton m_populateTreeID;
   CButton m_populateReachIndex;
   CStatic m_nodesGenerated;
   CStatic m_singleNodeReaches;
   CStatic m_phantomNodesGenerated;
   afx_msg void OnBnClickedGenerate();
   afx_msg void OnCbnSelchangeLayers();
   afx_msg void OnBnClickedPopulateorder();
   afx_msg void OnBnClickedUseattr();
   afx_msg void OnBnClickedUsevertices();
   virtual BOOL OnInitDialog();

   afx_msg void OnBnClickedBrowse();
   CProgressCtrl m_progress;
   CStatic m_progressText;
   };
