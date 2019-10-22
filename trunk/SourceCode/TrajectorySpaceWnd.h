#pragma once

#include <plot3d\scatterPlot3dWnd.h>
#include "deltaArray.h"


// TrajectorySpaceWnd

class TrajectorySpaceWnd : public CWnd
{
DECLARE_DYNAMIC(TrajectorySpaceWnd)

public:
	TrajectorySpaceWnd( int multiRun );
	virtual ~TrajectorySpaceWnd();

protected:
   int m_multiRun;      // multirun attached to this plot
   int m_run;           // current run in the multirun that is highlighted
   int m_year;          // current year being "sliced"
   int m_totalYears;    

   ScatterPlot3DWnd m_plot;

   CSliderCtrl m_yearSlider;
   CComboBox   m_luClassCombo;
   CComboBox   m_evalModelCombo;
   CComboBox   m_runCombo;
   CEdit       m_yearEdit;

   DeltaArray  *m_pDeltaArray;

public:
   static bool Replay( CWnd *pWnd, int flag );
   bool _Replay( int flag );

protected:
   void RefreshData();

   // child Windows
   enum 
   {
      TSW_PLOT = 7472,
      TSW_YEARSLIDER,
      TSW_YEAREDIT,
      TSW_LUCLASSCOMBO,
      TSW_EVALMODELCOMBO,
      TSW_RUNCOMBO
   };

protected:
	DECLARE_MESSAGE_MAP()
public:
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
   afx_msg void OnComboChange();
   afx_msg void OnRunComboChange();
   //afx_msg void OnEvalModelChange();
   //afx_msg void OnRunChange();

protected:
   virtual BOOL PreCreateWindow(CREATESTRUCT& cs);




};


