#pragma once

#include <FDataObj.h>

// LulcTransitionResultsWnd

class LulcTransitionResultsWnd : public CWnd
{
public:
   LulcTransitionResultsWnd( int run);
   virtual ~LulcTransitionResultsWnd();

   int m_run;
   int m_year;
   int m_totalYears;

   CWnd       *m_pWnd;
   FDataObj   *m_pData;

   CComboBox   m_lulcLevelCombo;
   CComboBox   m_timeIntervalCombo;
   CComboBox   m_plotListCombo;

   CSliderCtrl m_yearSlider;
   CEdit       m_yearEdit;

   bool m_changeTimeIntervalFlag;

public:
   static bool Replay( CWnd *pWnd, int flag );
   bool _Replay( int flag );

protected:
   void RefreshData();

   // child Windows
   enum 
   {
      LTPW_PLOT = 8452,
      LTPW_LULCLEVELCOMBO,
      LTPW_TIMEINTERVALCOMBO,
      LTPW_PLOTLISTCOMBO,
      LTPW_YEARSLIDER,
      LTPW_YEAREDIT,     
   };

protected:
   DECLARE_MESSAGE_MAP()
public:
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
   afx_msg void OnLulcLevelChange();
   afx_msg void OnTimeIntervalChange();
   afx_msg void OnPlotListChange();

protected:
   virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};


