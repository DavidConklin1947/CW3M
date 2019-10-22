#pragma once

#include <EasySize.h>
#include <ScatterWnd.h>
#include <FDataObj.h>

#include "Resource.h"
#include "afxwin.h"

// PolicyAppPlot dialog

class PolicyAppPlot : public CDialog
{
	DECLARE_DYNAMIC(PolicyAppPlot)

   DECLARE_EASYSIZE

public:
	PolicyAppPlot( int run );   // standard constructor
	virtual ~PolicyAppPlot();

   int m_run;

   CComboBox m_targetMetagoalCombo;
   CComboBox m_toleranceCombo;

   ScatterWnd *m_pPlot;
   FDataObj   *m_pData;

// Dialog Data
	enum { IDD = IDD_POLICYAPPPLOT };

protected:
   void RefreshPlot();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   virtual BOOL Create( const RECT &rect, CWnd* pParentWnd = NULL);
   virtual BOOL OnInitDialog();
   afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnChangeEffectiveness();
   afx_msg void OnChangeTolerance();
protected:
   virtual void OnCancel();
   virtual void OnOK();   
   
};
