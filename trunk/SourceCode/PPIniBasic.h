#pragma once

#include "resource.h"

#include <TabPageSSL.h>
#include "afxwin.h"
#include "afxcmn.h"

class IniFileEditor;


// PPIniBasic dialog

class PPIniBasic : public CTabPageSSL
{
	DECLARE_DYNAMIC(PPIniBasic)

public:
	PPIniBasic(IniFileEditor* pParent);   // standard constructor
	virtual ~PPIniBasic();

// Dialog Data
	enum { IDD = IDD_INIBASIC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   void UpdateCounts();

	DECLARE_MESSAGE_MAP()
public:
   IniFileEditor *m_pParent;

   bool StoreChanges();

   virtual BOOL OnInitDialog();
   
   afx_msg void OnBnClickedImportPolicies();
   afx_msg void OnBnClickedImportActors();
   afx_msg void OnBnClickedImportScenarios();
   afx_msg void OnBnClickedImportLulcTree();


   afx_msg void MakeDirty();
   afx_msg void OnBnClickedRunpolicyeditor();
   afx_msg void OnBnClickedRunactoreditor();
   afx_msg void OnBnClickedRunscenarioeditor();
   afx_msg void OnBnClickedRunlulceditor();
   };
