// QueryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Envision.h"
#include ".\querydlg.h"
#include "EnvEngine\EnvModel.h"
#include "SpatialOperatorsDlg.h"
//#include "EnvMsg.h"
#include "queryViewer.h"
#include "MapPanel.h"

#include <queryengine.h>
#include <maplayer.h>
#include <MAP.h>
#include <UNITCONV.H>

//extern QueryEngine *gpQueryEngine;

extern MapPanel *gpMapPanel;

CStringArray QueryDlg::m_queryStringArray;


// QueryDlg dialog

IMPLEMENT_DYNAMIC(QueryDlg, CDialog)
QueryDlg::QueryDlg(MapLayer *pLayer, int record /*=-1*/, CWnd* pParent /*=NULL*/)
: CDialog(QueryDlg::IDD, pParent)
, m_pLayer( pLayer )
, m_record( record )
, m_runGlobal(FALSE)
, m_clearPrev(TRUE)
   { }


QueryDlg::~QueryDlg()
{ }


void QueryDlg::DoDataExchange(CDataExchange* pDX)
{
CDialog::DoDataExchange(pDX);
DDX_Control(pDX, IDC_FIELDS, m_fields);
DDX_Control(pDX, IDC_OPERATORS, m_ops);
DDX_Control(pDX, IDC_VALUES, m_values);
DDX_Control(pDX, IDC_QUERY, m_query);
DDX_Check(pDX, IDC_RUNGLOBAL, m_runGlobal);
DDX_Check(pDX, IDC_CLEARPREV, m_clearPrev);
DDX_Control(pDX, IDC_HISTORY, m_historyCombo);
DDX_Control(pDX, IDC_LAYER, m_layers);
   }


BEGIN_MESSAGE_MAP(QueryDlg, CDialog)
   ON_CBN_SELCHANGE(IDC_LAYER, OnCbnSelchangeLayers)
   ON_BN_CLICKED(IDC_ADD, OnBnClickedAdd)
   ON_CBN_SELCHANGE(IDC_FIELDS, OnCbnSelchangeFields)
   ON_BN_CLICKED(IDC_AND, OnBnClickedAnd)
   ON_BN_CLICKED(IDC_OR, OnBnClickedOr)
   ON_BN_CLICKED(IDC_SPATIALOPS, OnBnClickedSpatialops)
   ON_BN_CLICKED(IDC_QUERYVIEWER, &QueryDlg::OnBnClickedQueryviewer)
   ON_BN_CLICKED(IDCANCEL, &QueryDlg::OnBnClickedCancel)
   ON_CBN_SELCHANGE(IDC_HISTORY, &QueryDlg::OnCbnSelchangeHistory)
END_MESSAGE_MAP()


// QueryDlg message handlers

BOOL QueryDlg::OnInitDialog()
   {
   CDialog::OnInitDialog();

   m_query.SetWindowText( _T("") );

   // load layers 
   Map *pMap = m_pLayer->m_pMap;
   int index = -1;

   for ( int i=0; i < pMap->GetLayerCount(); i++ )
      {
      MapLayer *pLayer = pMap->GetLayer( i );
      m_layers.AddString( pLayer->m_name );

      if ( pLayer == m_pLayer )
         index = i;
      }

   m_layers.SetCurSel( index );

   LoadLayer();

   LoadHistory();

   index = -1;
   
   if ( EnvModel::m_lulcTree.GetLevels() > 0 )
      index = m_fields.FindString( -1, EnvModel::m_lulcTree.GetFieldName( 1 ) );

   if ( index >= 0 )
      m_fields.SetCurSel( index );
   else 
      m_fields.SetCurSel( 0 );

   m_ops.AddString( _T("=") );
   m_ops.AddString( _T("!=") );
   m_ops.AddString( _T("<") );
   m_ops.AddString( _T(">") );
   m_ops.SetCurSel( 0 );

   LoadFieldValues();

   if ( m_record == -1 )
      {
      CheckDlgButton( IDC_RUNGLOBAL, 1 );
      GetDlgItem( IDC_RUNGLOBAL )->EnableWindow( 0 );
      }

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
   }


void QueryDlg::LoadLayer( void )
   {
   int layer = m_layers.GetCurSel();

   Map *pMap = m_pLayer->m_pMap;
   m_pLayer = pMap->GetLayer( layer );

   m_fields.ResetContent();

   for ( int i=0; i < m_pLayer->GetFieldInfoCount(1); i++ )
      {
      MAP_FIELD_INFO *pInfo = m_pLayer->GetFieldInfo( i );
      if ( pInfo->mfiType != MFIT_SUBMENU )
         m_fields.AddString( pInfo->fieldname );
      }
   }


void QueryDlg::LoadFieldValues()
   {
   int index = m_fields.GetCurSel();

   if ( index < 0 )
      return;

   char fieldName[ 64 ];
   m_fields.GetLBText( index, fieldName );
   SetDlgItemText( IDC_DESCRIPTION, _T("") );

   m_values.ResetContent();
   int colCellField = m_pLayer->GetFieldCol( fieldName );
   ASSERT( colCellField >= 0 );

   // check field info
   MAP_FIELD_INFO *pInfo = m_pLayer->FindFieldInfo( fieldName );

   if ( pInfo != NULL )  // found field info.  get attributes to populate box
      {
      for ( int i=0; i < pInfo->attributes.GetSize(); i++ )
         {
         FIELD_ATTR &attr = pInfo->GetAttribute( i );

         CString value;
         value.Format( "%s {%s}", attr.value.GetAsString(), attr.label );

         m_values.AddString( value );
         }

      m_values.SetCurSel( 0 );
      }

   else  // no field info exists...
      {
      // If it is a string type, get uniue values from the database
      switch( m_pLayer->GetFieldType( colCellField ) )
         {
         case TYPE_STRING:
         case TYPE_DSTRING:
            {
            CStringArray valueArray;
            m_pLayer->GetUniqueValues( colCellField, valueArray );
            for ( int i=0; i < valueArray.GetSize(); i++ )
               m_values.AddString( valueArray[ i ] );
            }
            m_values.SetCurSel( 0 );
            return;

         case TYPE_BOOL:
            m_values.AddString( _T("0 {False}") );
            m_values.AddString( _T("1 {True}") );
            m_values.SetCurSel( 0 );
            return;
         }

      // at this point, we have no attributes defined in the database, and the field is not a string
      // field or a boolean field.  No further action required - user has to type an entry into the
      // combo box     
      }
   return;
   }


void QueryDlg::LoadHistory( void )
   {
   m_historyCombo.ResetContent();

   int count = (int) m_queryStringArray.GetSize();

   for ( int i=count-1; i >= 0; i-- )
      {
      m_historyCombo.AddString( m_queryStringArray.GetAt( i ) );
      }

   if ( count > 0 )
      m_historyCombo.SetCurSel( 0 );
   }


void QueryDlg::OnBnClickedAdd()
   {
   // get text for underlying representation and add it to the query edit control at the current insertion point.
   CString field, op, value, query;
   m_fields.GetWindowText( field );
   m_ops.GetWindowText( op );
   m_values.GetWindowText( value );

   query += field + _T(" ") + op + _T(" ") + value;
   
   m_query.ReplaceSel( query, TRUE );

   int end = m_query.GetWindowTextLength();
   m_query.SetSel( end, end, 1 );
   }


void QueryDlg::OnCbnSelchangeLayers()
   {
   LoadLayer();
   }


void QueryDlg::OnCbnSelchangeFields()
   {
   LoadFieldValues();
   }

void QueryDlg::OnOK()
   {
   UpdateData();
   CString query;
   m_query.GetWindowText( query );

   QueryEngine qe( m_pLayer );

   Query *pQuery = qe.ParseQuery( query, 0, "Query Setup" );

   if ( pQuery == NULL )
      {
      Report::ErrorMsg( _T("Unable to parse query") );
      return;
      }
   
   //qe.GetLastQuery();
   //bool result;

   CWaitCursor c;

   if ( m_runGlobal )
      {
      // if ( m_pLayer->GetSpatialIndex() == NULL )
       //  m_pLayer->CreateSpatialIndex( NULL, 10000, 500, SIM_NEAREST );

      qe.SelectQuery( pQuery, m_clearPrev ? true : false );
      gpMapPanel->m_mapFrame.m_pMapWnd->RedrawWindow();
      }
   else
      {
      bool result = false; 
      if ( pQuery->Run( m_record, result ) == false )
         {
         Report::ErrorMsg( _T("Error solving query") );
         return;
         }
      }

   // get area of selection
   int selCount = m_pLayer->GetSelectionCount();
   float area = 0;
   for ( int i=0; i < selCount; i++ )
      {
      int idu = m_pLayer->GetSelection( i );

      switch( m_pLayer->m_layerType )
         {
         case LT_POLYGON:
            area += m_pLayer->GetPolygon( idu )->GetArea();
            break;

         case LT_LINE:
            area += m_pLayer->GetPolygon( idu )->GetEdgeLength();
            break;
         }
      }

   Map *pMap = m_pLayer->m_pMap;
   MAP_UNITS mu = pMap->GetMapUnits();

   float aggArea = 0;
   if ( mu == MU_FEET )
      aggArea = area / FT2_PER_ACRE;
   else if ( mu == MU_METERS )
      aggArea = area / M2_PER_ACRE;

   float totalArea = m_pLayer->GetTotalArea();
   float pctArea = 100 * area / totalArea;

   CString resultStr;
   switch( m_pLayer->m_layerType )
      {
      case LT_POLYGON:
         {
         if ( mu == MU_FEET || mu == MU_METERS )
            resultStr.Format( "Query [%s] returned %i polygons, with an area of %g sq. %s (%g acres), %4.1f%% of the total study area",
               (LPCSTR) query, selCount, area, pMap->GetMapUnitsStr(), aggArea, pctArea );
         else
            resultStr.Format( "Query [%s] returned %i polygons, with an area of %g, %4.1f%% of the total study area",
                  (LPCSTR) query, selCount, area, pctArea );
         }
         break;

      case LT_LINE:
         {
         if ( mu == MU_FEET || mu == MU_METERS )
            resultStr.Format( "Query [%s] returned %i lines, with a length of %g sq. %s (%g acres), %4.1f%% of the total study area",
               (LPCSTR) query, selCount, area, pMap->GetMapUnitsStr(), aggArea, pctArea );
         else
            resultStr.Format( "Query [%s] returned %i lines, with a length of %g, %4.1f%% of the total study area",
                  (LPCSTR) query, selCount, area, pctArea );
         }
         break;
      }
   
   MessageBox( resultStr, _T("Query Result") );

   m_queryStringArray.Add( query );
   LoadHistory();
   }


void QueryDlg::OnBnClickedAnd()
   {
   m_query.ReplaceSel( _T(" and "), TRUE );

   int end = m_query.GetWindowTextLength();
   m_query.SetSel( end, end, 1 );   
   }


void QueryDlg::OnBnClickedOr()
   {
   m_query.ReplaceSel( _T(" or "), TRUE );

   int end = m_query.GetWindowTextLength();
   m_query.SetSel( end, end, 1 );
   }


void QueryDlg::OnBnClickedSpatialops()
   {
   SpatialOperatorsDlg dlg;
   if ( dlg.DoModal() == IDOK )
      {
      switch( dlg.m_group )
         {
         case 0: 
            m_query.ReplaceSel( _T("NextTo( <query> )"), TRUE );
            break;

         case 1: 
            {
            CString text;
            text.Format( _T("Within( <query>, %g )"), dlg.m_distance );
            m_query.ReplaceSel( text, TRUE );
            }
            break;

         case 2: 
            {
            CString text;
            text.Format( _T("WithinArea( <query>, %g, %g )"), dlg.m_distance, dlg.m_areaThreshold );
            m_query.ReplaceSel( text, TRUE );
            }
            break;
         }

      CString qstr;
      m_query.SetFocus();
      m_query.GetWindowText( qstr );
      int pos = qstr.Find( _T("<query>") );
      m_query.SetSel( pos, pos+7, TRUE );
      }
   }


void QueryDlg::OnBnClickedQueryviewer()
   {
   CString qstr;
   m_query.GetWindowText( qstr );
  
   QueryViewer dlg( qstr, m_pLayer, this );
   dlg.DoModal();
   }


void QueryDlg::OnBnClickedCancel()
   {
   UpdateData();
   m_query.GetWindowText( m_queryString );
   OnCancel();
   }


void QueryDlg::OnCbnSelchangeHistory()
   {
   //int index = m_historyCombo.GetCurSel();

   CString query;
   m_historyCombo.GetWindowText( query );

   m_query.SetWindowText( query );
   }
