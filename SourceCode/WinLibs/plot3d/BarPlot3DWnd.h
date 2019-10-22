#pragma once

#include "Plot3DWnd.h"
#include <afxtempl.h>
#include <math.h>
#include <gl/glReal.h>

#include "FDataobj.h"

#include "plot3d/ColorScaleWnd.h"
#include "WinLib_resource.h"



// BarPlot3DWnd

class WINLIBSAPI BarPlot3DWnd : public Plot3DWnd
{

public:
	BarPlot3DWnd();
	virtual ~BarPlot3DWnd();

protected:
   FDataObj *m_pDataObj;
   bool      m_delOnDel;

   ColorScaleWnd m_colorScaleWnd;
   bool m_showLegend;
   bool m_showLabels;
   bool m_showOutline;

   // Plot Properties
   COLORREF m_axesColor;
   COLORREF m_backgroundColor;
   bool m_whiteBackground;

   void DrawData( void );
   void DrawAxes( void );

   enum B3D_MENUS
      {
      SHOW_LEGEND      = 9000,
      SHOW_OUTLINE,
      DATA_SET_PROPS
      };
   virtual void BuildPopupMenu( CMenu* pMenu );
   void OnSelectOption( UINT nID );

public:
   void SetDataObj( FDataObj *pDataObj, bool delOnDel, bool onlyIncreaseScale = false );

protected:
	DECLARE_MESSAGE_MAP()
public:
   virtual void OnCreateGL(); // override to set bg color, activate z-buffer, and other global settings
   virtual void OnSizeGL(int cx, int cy); // override to adapt the viewport to the window

   afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};


