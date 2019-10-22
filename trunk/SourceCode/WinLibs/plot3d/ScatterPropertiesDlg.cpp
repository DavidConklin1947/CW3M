
//  ScatterPropertiesDlg.cpp

#include "winlibs.h"
#include "plot3d/ScatterPropertiesDlg.h"


ScatterPropertiesDlg::ScatterPropertiesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(ScatterPropertiesDlg::IDD, pParent)
   {
   //m_pParent = (ScatterPlot3D*) pParent;

   ScatterPlot3DWnd* parent = (ScatterPlot3DWnd*) pParent;

   m_previewWnd.SetBackgroundColor( parent->GetBackgroundColor() );

   m_pParentDataArray = parent->GetPointCollectionArray();

   for ( int i=0; i<m_pParentDataArray->GetCount(); i++ )
      {
      PointCollection *pCol = m_pParentDataArray->GetAt(i);
      m_dataArray.Add( new PointCollection( *pCol ) );
      }

   
   }

ScatterPropertiesDlg::~ScatterPropertiesDlg()
{
}

void ScatterPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
CDialog::DoDataExchange(pDX);
DDX_Control(pDX, IDC_DATA_SET_COMBO, m_dataSetCombo);
DDX_Control(pDX, IDC_LINE_SIZE_COMBO, m_lineWidthCombo);
DDX_Control(pDX, IDC_POINT_SIZE_COMBO, m_pointSizeCombo);
DDX_Control(pDX, IDC_POINT_TYPE_COMBO, m_pointTypeCombo);
DDX_Control(pDX, IDC_COLOR_STATIC, m_colorStatic);
DDX_Control(pDX, IDC_COLOR_STATIC_LINES, m_colorStaticLines);
DDX_Control(pDX, IDC_PREVIEW_STATIC, m_previewStatic);
}


BEGIN_MESSAGE_MAP(ScatterPropertiesDlg, CDialog)
   ON_BN_CLICKED(IDC_SET_COLOR_BUTTON, OnBnClickedSetColorButton)
   ON_BN_CLICKED(IDC_SET_COLOR_BUTTON_LINES, OnBnClickedSetColorButtonLines)
   ON_WM_PAINT()
   ON_CBN_SELCHANGE(IDC_DATA_SET_COMBO, OnCbnSelchangeDataSetCombo)
   ON_CBN_SELCHANGE(IDC_LINE_SIZE_COMBO, OnCbnSelchangeLineWidthCombo)
   ON_CBN_SELCHANGE(IDC_POINT_SIZE_COMBO, OnCbnSelchangePointSizeCombo)
   ON_CBN_SELCHANGE(IDC_POINT_TYPE_COMBO, OnCbnSelchangePointTypeCombo)
END_MESSAGE_MAP()


// ScatterPropertiesDlg message handlers

BOOL ScatterPropertiesDlg::OnInitDialog()
   {
   CDialog::OnInitDialog();

   m_dataSetCombo.AddString( "-- All Datasets --" );

   for ( int i=0; i<m_dataArray.GetCount(); i++ )
      {
      PointCollection *pCol = m_dataArray.GetAt(i);
      m_dataSetCombo.AddString( pCol->GetName() );
      }
   m_dataSetCombo.SetCurSel(0);

   CString str;
   for ( int i=0; i<10; i++ )
      {
      str.Format( "%d", i );
      m_lineWidthCombo.AddString( str );
      }
   m_lineWidthCombo.SetCurSel( 0 );

   for ( int i=0; i<10; i++ )
      {
      str.Format( "%d", i );
      m_pointSizeCombo.AddString( str );
      }
   m_pointSizeCombo.SetCurSel( 0 );

   for ( int i=0; i<P3D_ST_LAST; i++ )
      m_pointTypeCombo.AddString( P3D_SHAPE_TYPE_NAME[i] );
   m_pointTypeCombo.SetCurSel( 0 );

   CRect rect;
   m_previewStatic.GetWindowRect( rect );
   ScreenToClient( &rect );

   m_previewWnd.Create( NULL, NULL, WS_VISIBLE | WS_BORDER, rect, this, 9876 );

   UpdateProperties();

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
   }

void ScatterPropertiesDlg::OnBnClickedSetColorButton()
   {
   int col = m_dataSetCombo.GetCurSel();

   if ( col > 0 )
      col--;

   PointCollection *pCol = m_dataArray.GetAt(col);

   CColorDialog dlg;
   dlg.m_cc.rgbResult = pCol->GetPointColor();
   dlg.m_cc.Flags |= CC_RGBINIT;

   if ( dlg.DoModal() != IDOK )
      return;

   // selected new color, get it and refresh dialog box
   col = m_dataSetCombo.GetCurSel();

   if ( col == 0 )
      {
      for ( int i=0; i < m_dataArray.GetCount(); i++ )
         {
         PointCollection *pCol = m_dataArray.GetAt( i );
         pCol->SetPointColor( dlg.m_cc.rgbResult );
         }
      }
   else
      pCol->SetPointColor( dlg.m_cc.rgbResult );
   
   UpdateProperties();
   }


void ScatterPropertiesDlg::OnBnClickedSetColorButtonLines()
   {
   int col = m_dataSetCombo.GetCurSel();

   if ( col > 0 )
      col--;

   PointCollection *pCol = m_dataArray.GetAt(col);

   CColorDialog dlg;
   dlg.m_cc.rgbResult = pCol->GetTrajColor();
   dlg.m_cc.Flags |= CC_RGBINIT;

   if ( dlg.DoModal() != IDOK )
      return;

   // selected new color, get it and refresh dialog box
   col = m_dataSetCombo.GetCurSel();

   if ( col == 0 )
      {
      for ( int i=0; i < m_dataArray.GetCount(); i++ )
         {
         PointCollection *pCol = m_dataArray.GetAt( i );
         pCol->SetTrajColor( dlg.m_cc.rgbResult );
         }
      }
   else
      pCol->SetTrajColor( dlg.m_cc.rgbResult );
   
   UpdateProperties();
   }


void ScatterPropertiesDlg::UpdateProperties()
   {
   int col = m_dataSetCombo.GetCurSel();

   if ( col != CB_ERR )
      {
      if ( col > 0 )
         col--;   // adjust for "All" option  (All uses item 0)

      PointCollection *pCol = m_dataArray.GetAt(col);
      m_lineWidthCombo.SetCurSel( pCol->GetTrajSize() );
      m_pointSizeCombo.SetCurSel( pCol->GetPointSize() );
      m_pointTypeCombo.SetCurSel( pCol->GetPointType() );

      m_previewWnd.SetPointCollection( pCol );
      Invalidate();
      }
   }

void ScatterPropertiesDlg::OnPaint()
   {
   CPaintDC dc(this); // device context for painting
   
   int col = m_dataSetCombo.GetCurSel();

   if ( col > 0 )
      col--;

   PointCollection *pCol = m_dataArray.GetAt(col);

   CRect rect;
   m_colorStatic.GetWindowRect( &rect );
   ScreenToClient( &rect );

   CBrush br;
   br.CreateSolidBrush( pCol->GetPointColor() );
   CBrush *pOldBr = dc.SelectObject( &br );
   dc.Rectangle( &rect );
   dc.SelectObject( pOldBr );

   // same with lines
   m_colorStaticLines.GetWindowRect( &rect );
   ScreenToClient( &rect );

   CBrush br1;
   br1.CreateSolidBrush( pCol->GetTrajColor() );
   CBrush *pOldBr1 = dc.SelectObject( &br1 );
   dc.Rectangle( &rect );
   dc.SelectObject( pOldBr1 );
   }


void ScatterPropertiesDlg::OnCbnSelchangeDataSetCombo()
   {
   UpdateProperties();
   }

void ScatterPropertiesDlg::OnCbnSelchangeLineWidthCombo()
   {
   int col = m_dataSetCombo.GetCurSel();

   if ( col == 0 )   // all
      {
      for ( int i=0; i < m_dataArray.GetCount(); i++ )
         {
         PointCollection *pCol = m_dataArray.GetAt( i );
         pCol->SetTrajSize( m_lineWidthCombo.GetCurSel() );
         }
      }
   else
      {
      PointCollection *pCol = m_dataArray.GetAt(col-1);
      pCol->SetTrajSize( m_lineWidthCombo.GetCurSel() );
      }

   UpdateProperties();
   }


void ScatterPropertiesDlg::OnCbnSelchangePointSizeCombo()
   {
   int col = m_dataSetCombo.GetCurSel();

   if ( col == 0 )   // all
      {
      for ( int i=0; i < m_dataArray.GetCount(); i++ )
         {
         PointCollection *pCol = m_dataArray.GetAt( i );
         pCol->SetPointSize( m_pointSizeCombo.GetCurSel() );
         }
      }
   else
      {
      PointCollection *pCol = m_dataArray.GetAt(col-1);
      pCol->SetPointSize( m_pointSizeCombo.GetCurSel() );
      }

   UpdateProperties();
   }


void ScatterPropertiesDlg::OnCbnSelchangePointTypeCombo()
   {
   int col = m_dataSetCombo.GetCurSel();

   if ( col == 0 )   // all
      {
      for ( int i=0; i < m_dataArray.GetCount(); i++ )
         {
         PointCollection *pCol = m_dataArray.GetAt( i );
         pCol->SetPointType( (P3D_SHAPE_TYPE) m_pointTypeCombo.GetCurSel() );
         }
      }
   else
      {
      PointCollection *pCol = m_dataArray.GetAt(col-1);
      pCol->SetPointType( (P3D_SHAPE_TYPE) m_pointTypeCombo.GetCurSel() );
      }

   UpdateProperties();
   }


void ScatterPropertiesDlg::OnOK()
   {
   for ( int i=0; i < m_pParentDataArray->GetCount(); i++ )
      {
      PointCollection *pCol = m_pParentDataArray->GetAt(i);
      *pCol = *m_dataArray.GetAt(i);
      }

   CDialog::OnOK();
   }
