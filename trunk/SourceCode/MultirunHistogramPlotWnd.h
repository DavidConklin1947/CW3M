#pragma once

#include <Histogram.h>
#include "BarPlotWnd.h"


// MultirunHistogramPlotWnd

class MultirunHistogramPlotWnd : public CWnd
{
DECLARE_DYNAMIC(MultirunHistogramPlotWnd)

public:
	MultirunHistogramPlotWnd( int multirun );
	virtual ~MultirunHistogramPlotWnd();

   int m_multirun;
   int m_year;
   int m_totalYears;

   BarPlotWnd m_plot;

   CSliderCtrl m_yearSlider;
   CEdit       m_yearEdit;
   CComboBox   m_resultCombo;
   
   static bool Replay( CWnd *pWnd, int flag );
   bool _Replay( int flag );

   void ComputeFrequencies( std::vector< float > xVector, int binCount );

   bool m_showLine;     // determines if a interpolation line is shown

protected:
   void RefreshData( void );

	DECLARE_MESSAGE_MAP()
  	afx_msg void OnPaint();

public:
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
   afx_msg void OnResultChange();

protected:
   virtual BOOL PreCreateWindow(CREATESTRUCT& cs);


   // child Windows
   enum 
      {
      HPW_PLOT = 8452,
      HPW_YEARSLIDER,
      HPW_YEAREDIT,
      HPW_RESULTCOMBO,
      };

};