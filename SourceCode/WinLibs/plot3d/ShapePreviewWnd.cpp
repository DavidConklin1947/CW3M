// ShapePreviewWnd.cpp : implementation file
//

#include "winlibs.h"
#include "plot3d/ScatterPlot3DWnd.h"
#include "plot3d/ShapePreviewWnd.h"


// ShapePreviewWnd

ShapePreviewWnd::ShapePreviewWnd()
   : m_pPointCollection( NULL )
   {
   //m_pointType       = P3D_ST_SPHERE;
   //m_pointColor      = RGB(255,255,255);
   SetFrameType( P3D_FT_NONE );
   }

ShapePreviewWnd::~ShapePreviewWnd()
{
}


BEGIN_MESSAGE_MAP(ShapePreviewWnd, Plot3DWnd)
END_MESSAGE_MAP()

void ShapePreviewWnd::OnCreateGL()
{
   Plot3DWnd::OnCreateGL();
}


void ShapePreviewWnd::DrawData()
{
   if ( m_pPointCollection->IsTrajectory() )
      {
      COLORREF rgb = m_pPointCollection->GetTrajColor();
      int r = GetRValue( rgb );
      int g = GetGValue( rgb );
      int b = GetBValue( rgb );

      glReal3 vector = -4.0*cross( m_eye, m_up );

      SetColorEmission( m_pPointCollection->GetTrajColor() );
      glBegin( GL_LINES );
         glVertex3d( vector.x, vector.y, vector.z );
         vector = -1.0*vector;
         glVertex3d( vector.x, vector.y, vector.z );
      glEnd();
      }

   SetColorObject( m_pPointCollection->GetPointColor() );
   DrawShape( m_pPointCollection->GetPointType(), glReal3(0.5,0.5,0.5), 200 );
   
}



