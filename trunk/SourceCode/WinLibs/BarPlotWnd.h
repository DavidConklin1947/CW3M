#pragma once

#include "graphwnd.h"


// BarPlotWnd

// Notes about Bar Plot Styles:
//   1) each BAR GROUP on the plot corresponds to a ROW in the attached data object.
//   2) STACKS for each bar correspond to LINES (DataObj cols) defiends for the plot

enum BPSTYLE 
   { 
   BPS_GROUPED,   // stacks are placed groups adjacent to each other
   BPS_STACKED,   // stacks are placed on top of each other
   BPS_SINGLE     // only on stack is display, selected from combo box (if more than one)
   };


#define MS_BAR2DSOLID   MS_SOLIDSQUARE
#define MS_BAR2DHOLLOW  MS_HOLLOWSQUARE


class WINLIBSAPI BarPlotWnd : public GraphWnd
{
	DECLARE_DYNAMIC(BarPlotWnd)

public:
	BarPlotWnd();
   BarPlotWnd( DataObj *pData, bool makeDataLocal=false );

	virtual ~BarPlotWnd();

   void SetBarPlotStyle( BPSTYLE, int extra=-1 );
   void LoadCombo();


protected:
	DECLARE_MESSAGE_MAP()
   //afx_msg void OnPaint();

   BPSTYLE m_style;     // defaults to BPS_STACKED
   CComboBox m_combo;

   virtual void DrawPlotArea( CDC *pDC );     // note: hDC should be initMapped when this function is called
   virtual bool UpdatePlot( CDC *pDC, bool, int startIndex=0, int numPoints=-1 );

   void DrawBar( HDC hDC, int left, int top, int right, int bottom, MSTYLE style, COLORREF color );

public:
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnComboChange();
   };


