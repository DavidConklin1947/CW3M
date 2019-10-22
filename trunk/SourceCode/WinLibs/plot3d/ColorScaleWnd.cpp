// ColorScaleWnd.cpp : implementation file
//

#include "winlibs.h"
#include "plot3d/ColorScaleWnd.h"


// ColorScaleWnd

IMPLEMENT_DYNAMIC(ColorScaleWnd, CWnd)
ColorScaleWnd::ColorScaleWnd()
{
m_colorScale.SetBaseColor( RGB(0,255,0) );
m_font.CreatePointFont( 90, "Arial" );
}

ColorScaleWnd::~ColorScaleWnd()
{
}


BEGIN_MESSAGE_MAP(ColorScaleWnd, CWnd)
   ON_WM_SIZE()
   ON_WM_PAINT()
END_MESSAGE_MAP()



// ColorScaleWnd message handlers


void ColorScaleWnd::OnSize(UINT nType, int cx, int cy)
   {
   CWnd::OnSize(nType, cx, cy);

   m_height = cy;
   m_width = cx;
   }

void ColorScaleWnd::OnPaint()
   {
   CPaintDC dc(this); // device context for painting
   // TODO: Add your message handler code here
   // Do not call CWnd::OnPaint() for painting messages

   dc.SelectObject( &m_font );

   CSize zSize = dc.GetTextExtent( "Z", 1 );
   //int textHeight = zSize.cy;

   int brushHeight = zSize.cy;
   static int brushWidth  = 30;
   static int num = 25;

   for ( int i=0; i<num; i++ )
      {
      CBrush brush( m_colorScale.GetColor( m_colorScale.GetMax()*(1.0f-float(i)/float(num)) ) );
      dc.FillRect( CRect( 0, i*brushHeight, brushWidth, (i+1)*brushHeight ), &brush );
      }

   dc.SetTextAlign( TA_TOP );

   CString label;
   label.Format( "%g", m_colorScale.GetMax() );
   dc.TextOut( brushWidth, 0, label );

   label.Format( "%g", m_colorScale.GetMin() );
   dc.TextOut( brushWidth, (num-1)*brushHeight , label );

   }

BOOL ColorScaleWnd::PreCreateWindow(CREATESTRUCT& cs)
   {
   cs.lpszClass = AfxRegisterWndClass(NULL, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

   return CWnd::PreCreateWindow(cs);
   }
