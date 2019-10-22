#pragma once

#include "resource.h"

#include <TabPageSSL.h>
#include "afxwin.h"


struct ENV_AUTO_PROCESS;
class IniFileEditor;


// PPIniProcesses dialog

class PPIniProcesses : public CTabPageSSL
{
	DECLARE_DYNAMIC(PPIniProcesses)

public:
	PPIniProcesses(IniFileEditor* pParent);   // standard constructor
	virtual ~PPIniProcesses();

// Dialog Data
	enum { IDD = IDD_INIPROCESSES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

   bool m_isProcessDirty;
   bool m_isProcessesDirty;

   ENV_AUTO_PROCESS *GetAP();


public:
   IniFileEditor *m_pParent;

   CCheckListBox m_aps;
   CString m_label;
   CString m_path;
   int m_timing;
   int m_ID;
   BOOL m_useCol;
   CString m_fieldName;
   CString m_initStr;

   void LoadAP();
   bool StoreChanges();

   afx_msg void OnBnClickedBrowse();
   afx_msg void OnBnClickedAdd();
   afx_msg void OnBnClickedRemove();

   virtual BOOL OnInitDialog();

   afx_msg void OnLbnSelchangeAps();
   afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
   afx_msg void OnEnChangeLabel();
   afx_msg void OnEnChangePath();
   afx_msg void OnEnChangeID();
   afx_msg void OnEnChangeField();
   afx_msg void OnEnChangeInitStr();
   afx_msg void OnBnClickedTiming();
   afx_msg void OnBnClickedUsecol();

   afx_msg void OnLbnChkchangeAps();

   afx_msg void MakeDirty();
   };
