#pragma once

#include <afxtempl.h>
#include "plot3d/ShapePreviewWnd.h"

class PointCollectionArray;
class ScatterPlot3DWnd;

// ScatterLegendWnd

class ScatterLegendWnd : public CWnd
{
	DECLARE_DYNAMIC(ScatterLegendWnd)

public:
	ScatterLegendWnd();
	virtual ~ScatterLegendWnd();

protected:
   PointCollectionArray *m_pDataArray;
   CArray< ShapePreviewWnd*, ShapePreviewWnd* > m_previewWndArray;

   int m_selection;
   int m_length;

   CFont m_font;
   static const int m_previewLength = 30;
   static const int m_gutter = 5;

   ScatterPlot3DWnd *m_pParent;

   void ClearPreviewWndArray( void );

public:
   void  SetPointCollectionArray( PointCollectionArray *pCollection );
   void  SetSelection( int sel ){ m_selection = sel; }
   CSize GetIdealSize();
protected:
	DECLARE_MESSAGE_MAP()
   virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
   afx_msg void OnPaint();
   afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};


