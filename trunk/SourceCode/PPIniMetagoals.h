#pragma once


#include <TabPageSSL.h>
#include "afxwin.h"
#include "resource.h"

class IniFileEditor;
struct METAGOAL;



// PPIniMetagoals dialog

class PPIniMetagoals : public CTabPageSSL
{
	DECLARE_DYNAMIC(PPIniMetagoals)

public:
   PPIniMetagoals(IniFileEditor *pParent );
	virtual ~PPIniMetagoals();

// Dialog Data
	enum { IDD = IDD_INIMETAGOALS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
   
   void LoadMetagoal();
   int  SaveMetagoal();
   void EnableCtrls();

   bool m_isMetagoalDirty;

public:
   bool StoreChanges();
   void AddEvalModel( LPCTSTR string );
   void RemoveEvalModel( int index );

public:
   IniFileEditor *m_pParent;
   METAGOAL *m_pInfo;

   CCheckListBox m_metagoals;
   CComboBox m_models;
   CString m_label;
   BOOL m_useSelfInterested;
   BOOL m_useAltruism;
  
   bool m_addRemove;

   afx_msg void OnBnClickedBrowse();
   afx_msg void OnBnClickedAdd();
   afx_msg void OnBnClickedRemove();
   afx_msg void OnEnChangeLabel();

   afx_msg void OnLbnChkchangeMetagoals();

   virtual BOOL OnInitDialog();

   afx_msg void MakeDirty();
   afx_msg void MakeClean();

   afx_msg void OnLbnSelchangeMetagoals();
   afx_msg void OnCbnSelchangeModel();
   };
