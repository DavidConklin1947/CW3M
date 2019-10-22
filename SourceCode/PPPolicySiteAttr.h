#pragma once

#include "resource.h"
#include "afxwin.h"
#include "afxcmn.h"

#include "EnvEngine\policy.h"

#include <TabPageSSL.h>

class PolEditor;
class MapLayer;

// PPPolicySiteAttr dialog

class PPPolicySiteAttr : public CTabPageSSL
{
	DECLARE_DYNAMIC(PPPolicySiteAttr)

public:
	PPPolicySiteAttr( PolEditor*, Policy *&pPolicy );
	virtual ~PPPolicySiteAttr();

   // Dialog Data
	enum { IDD = IDD_SITEATTR };
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
   Policy *&m_pPolicy;
   PolEditor *m_pParent;
   MapLayer  *m_pLayer;

public:
   CComboBox m_fields;
   CComboBox m_ops;
   CComboBox m_values;
   CComboBox m_layers;
   CEdit m_query;

public:
   void LoadPolicy();
   bool StoreChanges();
   void LoadFieldInfo();

protected:
   void LoadFieldValues();
   bool CompilePolicy();
   void MakeDirty();

protected:
   virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedAdd();
   afx_msg void OnCbnSelchangeFields();
   afx_msg void OnBnClickedAnd();
   afx_msg void OnBnClickedOr();
   afx_msg void OnBnClickedSpatialops();
   afx_msg void OnEnChangeQuery();
   afx_msg void OnBnClickedUpdate();
	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnCbnSelchangeLayers();
   afx_msg void OnBnClickedCheck();
   };
