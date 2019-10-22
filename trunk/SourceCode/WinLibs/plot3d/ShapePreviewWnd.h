#pragma once

#include "plot3d/Plot3DWnd.h"

class PointCollection;

// ShapePreviewWnd

class ShapePreviewWnd : public Plot3DWnd
{

public:
	ShapePreviewWnd();
	virtual ~ShapePreviewWnd();

protected:
   PointCollection *m_pPointCollection;

   void DrawPoint( void );

public:
   void SetBackgroundColor( COLORREF rgb ){ m_backgroundColor = rgb; }
   void SetPointCollection( PointCollection *pPointCollection ){ m_pPointCollection = pPointCollection; }

protected:
	DECLARE_MESSAGE_MAP()

public:
   virtual void OnCreateGL(); 
   virtual void DrawData();
};


