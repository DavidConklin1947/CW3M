#pragma once

#include "resource.h"
#include <afxtempl.h>
#include "afxcmn.h"

// PPMetaGoals dialog

class PPMetaGoals : public CPropertyPage
{
	DECLARE_DYNAMIC(PPMetaGoals)

public:
	PPMetaGoals();   // standard constructor
	virtual ~PPMetaGoals();

// Dialog Data
	enum { IDD = IDD_RUNMETAGOALS };

   CArray <CSliderCtrl*, CSliderCtrl* > m_sliderArray;
   CArray <CStatic*, CStatic* > m_staticArray;

   CSliderCtrl m_altruismSlider;
   float       m_altruismWt;     // 0-1
   CSliderCtrl m_selfInterestSlider;
   float       m_selfInterestWt;
   CSliderCtrl m_policyPrefSlider;
   float       m_policyPrefWt;

   float GetGoalWeight( int i );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

   afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
   afx_msg void OnDestroy();
   afx_msg void OnBnClickedBalance();
   afx_msg void OnWtSlider(UINT nSBCode, UINT nPos, CSliderCtrl* pSlider);
};
