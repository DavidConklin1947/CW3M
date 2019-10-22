// PPPlotAxes.cpp : implementation file
//

#include "winlibs.h"
#include "PPPlotAxes.h"
#include ".\ppplotaxes.h"
#include <graphwnd.h>



// PPPlotAxes dialog

IMPLEMENT_DYNAMIC(PPPlotAxes, CPropertyPage)
PPPlotAxes::PPPlotAxes(GraphWnd* pGraph)
	: CPropertyPage(PPPlotAxes::IDD),
   m_pGraph( pGraph ),
   m_axis( -1 )
{
}

PPPlotAxes::~PPPlotAxes()
{
}

void PPPlotAxes::DoDataExchange(CDataExchange* pDX)
{
CPropertyPage::DoDataExchange(pDX);
DDX_Control(pDX, IDC_SCALEMIN, m_scaleMin);
DDX_Control(pDX, IDC_SCALEMAX, m_scaleMax);
DDX_Control(pDX, IDC_TICMAJOR, m_ticMinor);
DDX_Control(pDX, IDC_TICMINOR, m_ticMajor);
DDX_Control(pDX, IDC_TICDECIMALS, m_decimals);
DDX_Control(pDX, IDC_AUTOSCALE, m_autoScale);
DDX_Control(pDX, IDC_AUTOINCR, m_autoIncr);
DDX_Control(pDX, IDC_AXIS, m_axes);
}


BEGIN_MESSAGE_MAP(PPPlotAxes, CPropertyPage)
   ON_EN_CHANGE(IDC_SCALEMIN, &PPPlotAxes::OnEnChangeScalemin)
   ON_EN_CHANGE(IDC_SCALEMAX, &PPPlotAxes::OnEnChangeScalemax)
   ON_EN_CHANGE(IDC_TICMAJOR, &PPPlotAxes::OnEnChangeTicmajor)
   ON_EN_CHANGE(IDC_TICMINOR, &PPPlotAxes::OnEnChangeTicminor)
   ON_EN_CHANGE(IDC_TICDECIMALS, &PPPlotAxes::OnEnChangeTicdecimals)
END_MESSAGE_MAP()


BOOL PPPlotAxes::OnInitDialog()
   {
   CPropertyPage::OnInitDialog();

   //-- initialization --//
   char buffer[ 120 ];
   //m_axes.AddString( "X-axis" );
   //m_axes.AddString( "Y-axis" );
   m_axes.SetCurSel( 0 );
   m_axis = XAXIS;

   //-- scale factors --//
   float min, max;
   m_pGraph->GetExtents( (MEMBER) m_axis, &min, &max );

   sprintf_s( buffer, 120, "%g", min );
	SetDlgItemText( IDC_SCALEMIN, buffer );
	sprintf_s( buffer, 120, "%g", max );
	SetDlgItemText(  IDC_SCALEMAX, buffer );

   BOOL autoScale;

   if ( m_axis == XAXIS )
      autoScale = m_pGraph->xAxis.autoScale;
   else
      autoScale = m_pGraph->yAxis.autoScale;
      
   CheckDlgButton( IDC_AUTOSCALE, autoScale );

   //-- tick marks --//
   float majorIncr, minorIncr;
   m_pGraph->GetAxisTickIncr( (MEMBER) m_axis, &majorIncr, &minorIncr );

   sprintf_s( buffer, 120, "%g", majorIncr );
	SetDlgItemText( IDC_TICMAJOR, buffer );
	sprintf_s( buffer, 120, "%g", minorIncr );
	SetDlgItemText( IDC_TICMINOR, buffer );

   BOOL autoIncr;
   if ( m_axis == XAXIS )
      autoIncr = m_pGraph->xAxis.autoIncr;
   else
      autoIncr = m_pGraph->yAxis.autoIncr;
      
   CheckDlgButton( IDC_AUTOINCR, autoIncr );

   int digits = m_pGraph->GetAxisDigits( (MEMBER) m_axis );
   SetDlgItemInt( IDC_TICDECIMALS, digits, FALSE );

   //-- label text --//
   //m_pGraph->GetLabelText( axis, (LPSTR) buffer, 120 );   // axis title
   //SetDlgItemText( 130, (LPSTR) buffer );

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
   }


BOOL PPPlotAxes::OnCommand(WPARAM wParam, LPARAM lParam)
   {
   int ctrlID   = LOWORD( wParam );
   int ctrlCode = HIWORD( wParam );

   switch( ctrlID )
      {
      case IDC_AUTOSCALE:   // autoScale check box
         if ( ctrlCode == BN_CLICKED )
            {
            BOOL checked = IsDlgButtonChecked( IDC_AUTOSCALE );
            GetDlgItem( IDC_SCALEMIN )->EnableWindow( ! checked );
            GetDlgItem( IDC_SCALEMAX )->EnableWindow( ! checked );
            GetDlgItem( IDC_SCALEMINLABEL )->EnableWindow( ! checked );
            GetDlgItem( IDC_SCALEMAXLABEL )->EnableWindow( ! checked );
            SetModified( TRUE );
            }
         return TRUE;

      case IDC_AUTOINCR:   // autoIncr check box
         if ( ctrlCode == BN_CLICKED )
            {
            BOOL checked = IsDlgButtonChecked( 125 );
            GetDlgItem( IDC_TICMAJOR )->EnableWindow( ! checked );
            GetDlgItem( IDC_TICMINOR )->EnableWindow( ! checked );
            GetDlgItem( IDC_TICMAJORLABEL )->EnableWindow( ! checked );
            GetDlgItem( IDC_TICMINORLABEL )->EnableWindow( ! checked );
            SetModified( TRUE );
            }
         return TRUE;
      }
  
   return CPropertyPage::OnCommand(wParam, lParam);
   }


bool PPPlotAxes::Apply( bool redraw /*=true*/ )
   {
   if ( m_axes.GetCurSel() == 0 )
      m_axis = XAXIS;
   else
      m_axis = YAXIS;

   //-- scale factors --//
   float scaleMin, scaleMax;
   CString _min, _max;
   m_scaleMin.GetWindowText( _min );
   m_scaleMax.GetWindowText( _max );
   scaleMin = (float) atof( _min );
   scaleMax = (float) atof( _max );

   BOOL autoScale = m_autoScale.GetCheck();
   BOOL autoIncr  = m_autoIncr.GetCheck();

   float ticMajor, ticMinor;
   CString _ticMajor, _ticMinor;
   m_ticMajor.GetWindowText( _ticMajor );
   m_ticMinor.GetWindowText( _ticMinor );
   ticMajor = (float) atof( _ticMajor );
   ticMinor = (float) atof( _ticMinor );

   BOOL translateFlag;
   int decimals = GetDlgItemInt(  IDC_TICDECIMALS, &translateFlag, FALSE );

   if ( translateFlag )
      m_pGraph->SetAxisDigits( (MEMBER) m_axis, decimals );

   AXIS *pAxis = &(m_pGraph->yAxis);
   if ( m_axis == XAXIS )
      pAxis = &(m_pGraph->xAxis);

   pAxis->autoScale = autoScale ? true : false;
   pAxis->autoIncr = autoIncr ? true : false;

   // more needed

   if ( redraw )
      m_pGraph->RedrawWindow();

   return true;
   }



/*

BOOL GraphAxisDlg::Close( void )
   {
   BOOL translateFlag;
   //-- get current values and load into box --//
   char buffer[ 120 ];

   //-- scaling --//
   BOOL checked = IsChecked( 105 );

   if ( axis == XAXIS )
      pGraph->xAxis.autoScale = checked;
   else
      pGraph->yAxis.autoScale = checked;

   if ( ! checked )
      {
      float min, max;
      ::GetDlgItemText( GetHandle(), 102, (LPSTR) buffer, 59 );
      min = (float) atof( buffer );
      ::GetDlgItemText( GetHandle(), 104, (LPSTR) buffer, 59 );
      max = (float) atof( buffer );
      pGraph->SetExtents( axis, min, max );
      }

   //-- tics --//
   checked = IsChecked( 125 );

   if ( axis == XAXIS )
      pGraph->xAxis.autoIncr = checked;
   else
      pGraph->yAxis.autoIncr = checked;

   if ( ! checked )
      {
      float majorIncr, minorIncr;
      ::GetDlgItemText( GetHandle(), 122, (LPSTR) buffer, 59 );
      majorIncr = (float) atof( buffer );
      ::GetDlgItemText( GetHandle(), 124, (LPSTR) buffer, 59 );
      minorIncr = (float) atof( buffer );
      pGraph->SetAxisTickIncr( axis, majorIncr, minorIncr );
      }

   int digits = ::GetDlgItemInt( GetHandle(), 126, &translateFlag, FALSE );

   if ( translateFlag )
      pGraph->SetAxisDigits( axis, digits );

   ::GetDlgItemText( GetHandle(), 130, (LPSTR) buffer, 120 );
   pGraph->SetLabelText( axis, (LPSTR) buffer );

   return TRUE;
   }
   */



void PPPlotAxes::OnEnChangeScalemin()
   {
   SetModified( TRUE );
   }


void PPPlotAxes::OnEnChangeScalemax()
   {
   SetModified( TRUE );
   }


void PPPlotAxes::OnEnChangeTicmajor()
   {
   SetModified( TRUE );
   }


void PPPlotAxes::OnEnChangeTicminor()
   {
   SetModified( TRUE );
   }


void PPPlotAxes::OnEnChangeTicdecimals()
   {
   SetModified( TRUE );
   }
