#pragma once

#include <Plot3D\ScatterPlot3DWnd.h>



// GoalSpaceWnd

class GoalSpaceWnd : public CWnd
{
public:
	GoalSpaceWnd( int run);
	virtual ~GoalSpaceWnd();

   int m_run;
   int m_year;
   int m_totalYears;

   ScatterPlot3DWnd m_plot;

   CSliderCtrl m_yearSlider;
   CComboBox   m_goalCombo0;
   CComboBox   m_goalCombo1;
   CComboBox   m_goalCombo2;
   CEdit       m_yearEdit;


public:
   static bool Replay( CWnd *pWnd, int flag );
   bool _Replay( int flag );

protected:
   void RefreshData();

   // child Windows
   enum 
   {
      GSW_PLOT = 7452,
      GSW_YEARSLIDER,
      GSW_YEAREDIT,
      GSW_GOALCOMBO1,
      GSW_GOALCOMBO2,
      GSW_GOALCOMBO3     
   };

protected:
	DECLARE_MESSAGE_MAP()
public:
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
   afx_msg void OnGoalChange();

protected:
   virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};


