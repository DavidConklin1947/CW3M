#pragma once

#include "stdafx.h"

#include "afxwin.h"
#include "afxcmn.h"
#include "resource.h"
#include "EnvEngine\policy.h"

#include <TabPageSSL.h>

class PolEditor;

// PPPolicyBasic dialog

class PPPolicyBasic : public CTabPageSSL
{
	DECLARE_DYNAMIC(PPPolicyBasic)

public:
	PPPolicyBasic( PolEditor*, Policy *&pPolicy  );
	virtual ~PPPolicyBasic();

// Dialog Data
	enum { IDD = IDD_POLICYBASIC };
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
   Policy    *&m_pPolicy;
   PolEditor  *m_pParent;

   CStatic     m_color;    // the control
   COLORREF    m_colorRef; // the selected color
   CString     m_narrative;
   int         m_persistenceMin;
   int         m_persistenceMax;
   BOOL        m_isScheduled;
   BOOL        m_mandatory;
   BOOL        m_exclusive;
   BOOL        m_isShared;
   BOOL        m_isEditable;
   BOOL        m_useStartDate;
   BOOL        m_useEndDate;
   int         m_startDate;
   int         m_endDate;

public:
   void LoadPolicy();
   bool StoreChanges();

protected:
   void EnableControls();
   void MakeDirty();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSetcolor();
   afx_msg void OnBnClickedIsScheduled();
   afx_msg void OnBnClickedMandatory();
   afx_msg void OnBnClickedPersistcheck();
   afx_msg void OnBnClickedUsestartdate();
   afx_msg void OnBnClickedUseenddate();
   afx_msg void OnBnClickedIsshared();
   afx_msg void OnBnClickedShowhide();
	DECLARE_MESSAGE_MAP()

};
