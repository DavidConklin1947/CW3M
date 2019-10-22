#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// OptionsDlg dialog

class OptionsDlg : public CDialog
{
	DECLARE_DYNAMIC(OptionsDlg)

public:
	OptionsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~OptionsDlg();

// Dialog Data
	enum { IDD = IDD_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   BOOL m_showTips;
   BOOL m_checkForUpdates;
   bool m_isDirty;

protected:
   virtual void OnOK();
   bool SaveRegistryKeys();

   void LoadAnalysisModules();
   void LoadVisualizationModules();
   void LoadDataManagementModules();

public:
   CListBox m_addIns;
   virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedAdd();
   afx_msg void OnBnClickedDelete();
   afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
   CTabCtrl m_tabCtrl;
   afx_msg void OnBnClickedShowtips();
   afx_msg void OnBnClickedCheckforupdates();
   };
