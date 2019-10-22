// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://msdn.microsoft.com/officeui.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// EnvisionView.cpp : implementation of the CEnvView class
//

#include "stdafx.h"
#include "Envision.h"
#include ".\EnvView.h"
#include "mainfrm.h"
#include "actorDlg.h"
#include "MapPanel.h"
#include "EnvDoc.h"
#include "EnvEngine\EnvModel.h"
#include "SelectActor.h"
#include "wizrun.h"
#include "EnvEngine\DataManager.h"
#include "EnvEngine\EnvException.h"
#include "ExportDataDlg.h"
#include "deltaviewer.h"
#include "querydlg.h"
#include "PolicyColorDlg.h"
#include "ScenarioEditor.h"
#include "databaseInfoDlg.h"
#include "ActiveWnd.h"
#include "ActorDlg.h"
#include "LulcEditor.h"

#include <map.h>
#include <MapWnd.h>
#include <VDataTable.h>
#include <FDataObj.h>
#include <direct.h>
#include <Path.h>

#include "EnvEngine\ColumnTrace.h"
#include "VideoRecorderDlg.h"

#include <raster.h>

#include <AlgLib\AlgLib.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

InputPanel    *gpInputPanel   = NULL;
MapPanel      *gpMapPanel     = NULL;
ResultsPanel  *gpResultsPanel = NULL;
RtViewPanel   *gpRtViewsPanel = NULL;
CEnvView      *gpView         = NULL;

extern MapLayer      *gpCellLayer;
extern CEnvDoc       *gpDoc;
extern PolicyManager *gpPolicyManager;
extern ActorManager  *gpActorManager;
extern EnvModel      *gpModel;
extern CMainFrame    *gpMain;
//__EXPORT__  Map3d    *gpMap3d;      // defined in Map3d.h


// CEnvView

IMPLEMENT_DYNCREATE(CEnvView, CView)

BEGIN_MESSAGE_MAP(CEnvView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CEnvView::OnFilePrintPreview)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_COPYLEGEND, OnEditCopyLegend)
	ON_COMMAND(ID_ZOOMIN, OnZoomin)
 	ON_UPDATE_COMMAND_UI(ID_ZOOMIN, OnUpdateIsMapWnd)
	ON_COMMAND(ID_ZOOMOUT, OnZoomout)
 	ON_UPDATE_COMMAND_UI(ID_ZOOMOUT, OnUpdateIsMapWnd)
	ON_COMMAND(ID_ZOOMTOQUERY, OnZoomToQuery)
 	ON_UPDATE_COMMAND_UI(ID_ZOOMTOQUERY, OnUpdateIsMapWnd)
	ON_COMMAND(ID_SELECT, OnSelect)
 	ON_UPDATE_COMMAND_UI(ID_SELECT, OnUpdateIsMapWnd)
   ON_COMMAND(ID_CLEAR_SELECTION, OnClearSelection)
 	ON_UPDATE_COMMAND_UI(ID_CLEAR_SELECTION, OnUpdateIsSelection)
	ON_COMMAND(ID_QUERYDISTANCE, OnQuerydistance)
 	ON_UPDATE_COMMAND_UI(ID_QUERYDISTANCE, OnUpdateIsMapWnd)
	ON_COMMAND(ID_ZOOMFULL, OnZoomfull)
 	ON_UPDATE_COMMAND_UI(ID_ZOOMFULL, OnUpdateIsMapWnd)
	//ON_COMMAND(ID_DBTABLE, OnDbtable)
	//ON_UPDATE_COMMAND_UI(ID_DBTABLE, OnUpdateIsMapWnd)
	ON_COMMAND(ID_PAN, OnPan)
	ON_UPDATE_COMMAND_UI(ID_PAN, OnUpdateIsMapWnd)
	ON_COMMAND(ID_RESULTS_CASCADE, OnResultsCascade)
	ON_UPDATE_COMMAND_UI(ID_RESULTS_CASCADE, OnUpdateResultsTileCascade)
	ON_COMMAND(ID_RESULTS_TILE, OnResultsTile)
	ON_UPDATE_COMMAND_UI(ID_RESULTS_TILE, OnUpdateResultsTileCascade)
	ON_COMMAND(ID_RESULTS_CLEAR, OnResultsClear)
	ON_UPDATE_COMMAND_UI(ID_RESULTS_CLEAR, OnUpdateResultsTileCascade)
	ON_COMMAND(ID_RESULTS_CAPTURE, OnResultsCapture)
	ON_UPDATE_COMMAND_UI(ID_RESULTS_CAPTURE, OnUpdateResultsCapture)
	ON_COMMAND(ID_RESULTS_SYNC_MAPS, OnResultsSyncMaps)
	ON_UPDATE_COMMAND_UI(ID_RESULTS_SYNC_MAPS, OnUpdateResultsSyncMaps)
   ON_UPDATE_COMMAND_UI(ID_RTVIEW_MAPS, OnUpdateData)

   ON_WM_KEYDOWN()

   ON_COMMAND(ID_VIEW_INPUT, OnViewInput)
   ON_COMMAND(ID_VIEW_MAP, OnViewMap)
   ON_COMMAND(ID_VIEW_TABLE, OnViewTable)
   ON_COMMAND(ID_VIEW_RTVIEWS, OnViewRtViews)
   ON_COMMAND(ID_VIEW_RESULTS, OnViewResults)
   
   ON_COMMAND(ID_VIEW_POLYGONEDGES, OnViewPolygonedges)
	ON_UPDATE_COMMAND_UI(ID_VIEW_POLYGONEDGES, OnUpdateViewPolygonedges)
   ON_COMMAND(ID_VIEW_SHOWSCALE, OnViewShowMapScale)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWSCALE, OnUpdateViewShowMapScale)
   ON_COMMAND(ID_VIEW_SHOWATTRIBUTE, OnViewShowAttribute)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWATTRIBUTE, OnUpdateViewShowAttribute)

   ON_COMMAND(ID_ANALYSIS_SHOWACTORTERRITORY, OnAnalysisShowactorterritory)
	ON_COMMAND(ID_ANALYSIS_SHOWACTORS, OnAnalysisShowactors)
	ON_WM_MOUSEWHEEL()

   // Data menu
   ON_COMMAND(ID_DATA_LULCINFO, OnDataLulcInfo)
   ON_COMMAND(ID_DATA_IDUINFO, OnDataIduInfo)

   // Analysis menu
   ON_COMMAND(ID_ANALYSIS_SETUP_RUN, OnAnalysisSetupRun)
   ON_COMMAND(ID_ANALYSIS_SETUP_MODELS, OnAnalysisSetupModels)
   ON_COMMAND(ID_EDIT_POLICIES, OnEditPolicies)
   ON_COMMAND(ID_EDIT_ACTORS, OnEditActors)
   ON_COMMAND(ID_EDIT_LULC, OnEditLulc)
   ON_COMMAND(ID_EDIT_SCENARIOS, OnEditScenarios)
   
   ON_COMMAND(ID_ANALYSIS_EXPORTCOVERAGES, OnExportCoverages)
   ON_COMMAND(ID_ANALYSIS_EXPORTOUTPUTS, OnExportOutputs)
   ON_COMMAND(ID_ANALYSIS_EXPORTDELTAS, OnExportDeltas)
   ON_COMMAND(ID_ANALYSIS_EXPORTDELTAFIELDS, OnExportDeltaFields)
   ON_COMMAND(ID_ANALYSIS_VIDEORECORDERS, OnVideoRecorders)
 
	ON_UPDATE_COMMAND_UI(ID_ANALYSIS_EXPORTCOVERAGES, OnUpdateExportCoverages)
	ON_UPDATE_COMMAND_UI(ID_ANALYSIS_EXPORTOUTPUTS, OnUpdateExportOutputs)
	ON_UPDATE_COMMAND_UI(ID_ANALYSIS_EXPORTDELTAS, OnUpdateExportDeltas)
	ON_UPDATE_COMMAND_UI(ID_ANALYSIS_EXPORTDELTAFIELDS, OnUpdateExportDeltaFields)
	ON_UPDATE_COMMAND_UI(ID_ANALYSIS_EXPORTINTERVAL,  OnUpdateExportInterval)
	ON_UPDATE_COMMAND_UI(ID_ANALYSIS_STARTYEAR, OnUpdateStartYear)
	ON_UPDATE_COMMAND_UI(ID_ANALYSIS_STOPTIME,  OnUpdateStopTime)
   
   ON_UPDATE_COMMAND_UI(ID_EDIT_POLICIES, OnUpdateData)
   ON_UPDATE_COMMAND_UI(ID_EDIT_ACTORS, OnUpdateData)
   ON_UPDATE_COMMAND_UI(ID_EDIT_LULC, OnUpdateData)
   ON_UPDATE_COMMAND_UI(ID_EDIT_SCENARIOS, OnUpdateData)
   
   ON_COMMAND(ID_STATUSBAR_ZOOMSLIDER, OnZoomSlider)
   ON_COMMAND(ID_STATUSBAR_ZOOMBUTTON, OnZoomButton)
   
   ON_UPDATE_COMMAND_UI(ID_ANALYSIS_SHOWACTORTERRITORY, OnUpdateData)
   ON_UPDATE_COMMAND_UI(ID_FILE_RESET, OnUpdateData)
   ON_COMMAND(ID_SAVE_SHAPEFILE, OnSaveShapefile)
   ON_UPDATE_COMMAND_UI(ID_SAVE_SHAPEFILE, OnUpdateSaveShapefile)
//   ON_COMMAND(ID_INTERACTIVE_CONNECTTOHOST, OnConnectToHost)
//   ON_UPDATE_COMMAND_UI(ID_INTERACTIVE_CONNECTTOHOST, OnUpdateConnectToHost)
//   ON_COMMAND(ID_INTERACTIVE_HOSTINTERACTIVESESSION, OnHostSession)
//   ON_UPDATE_COMMAND_UI(ID_INTERACTIVE_HOSTINTERACTIVESESSION, OnUpdateHostSession)
   ON_COMMAND(ID_ANALYSIS_SHOWDELTALIST, OnAnalysisShowdeltalist)
   ON_UPDATE_COMMAND_UI(ID_ANALYSIS_SHOWDELTALIST, OnUpdateAnalysisShowdeltalist)
   ON_COMMAND(ID_ANALYSIS_SETUPPOLICYMETAPROCESS, OnAnalysisSetuppolicymetaprocess)
   ON_UPDATE_COMMAND_UI(ID_ANALYSIS_SETUPPOLICYMETAPROCESS, OnUpdateAnalysisSetuppolicymetaprocess)
   ON_COMMAND(ID_DATA_STORERUNDATA, OnDataStorerundata)
   ON_COMMAND(ID_DATA_LOADRUNDATA, OnDataLoadrundata)
   ON_COMMAND(ID_RUNQUERY, OnRunquery)
   ON_UPDATE_COMMAND_UI(ID_RUNQUERY, OnUpdateData)
   ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
   ON_UPDATE_COMMAND_UI(ID_EDIT_COPYLEGEND, OnUpdateIsMapWnd)
   ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
   ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateFilePrint)
   ON_WM_CLOSE()
   ON_COMMAND(ID_EDIT_SETPOLICYCOLORS, OnEditSetpolicycolors)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEnvView construction/destruction

CEnvView::CEnvView()
: m_mode( MM_NORMAL )
, m_policyEditor( this )
, m_activePanel( 1 )
, m_pDefaultZoom( NULL )
   {
   gpView = this;  // initially
   }

CEnvView::~CEnvView()
{
}


BOOL CEnvView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CEnvView drawing

void CEnvView::OnDraw(CDC* /*pDC*/)
{
	CEnvDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CEnvView printing


void CEnvView::OnFilePrintPreview()
{
	AFXPrintPreview(this);
}

BOOL CEnvView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CEnvView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CEnvView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CEnvView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CEnvView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}


// CEnvView diagnostics

#ifdef _DEBUG
void CEnvView::AssertValid() const
{
	CView::AssertValid();
}

void CEnvView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CEnvDoc* CEnvView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CEnvDoc)));
	return (CEnvDoc*)m_pDocument;
}
#endif //_DEBUG



/////////////////////////////////////////////////////////////////////////////
// CEnvView message handlers

int CEnvView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
   {
   // basic idea - CEnvView maintains a set of windows:
   //  1) Map Panel - GIS front end for IDU Display
   //  2) Data Panel - Tabular access to loaded GIS datatables
   //  3) RtViews Panel - for displaying dynamic results with plug-in views during a simulation run
   //  4) Results Panel - where all results are displayed
   //
   // all are create here, only one visible initially
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

   RECT rect;
   GetClientRect( &rect );

   BOOL ok = m_inputPanel.Create( NULL, "Input Panel", WS_CHILD, rect, this, 100 );
   ASSERT ( ok );

   ok = m_mapPanel.Create( NULL, "Map View", WS_CHILD | WS_VISIBLE, rect, this, 101 );
   ASSERT ( ok );

   ok = m_dataPanel.Create( this );
   ASSERT( ok );

   ok = m_viewPanel.Create( NULL, "RtViews Panel", WS_CHILD | WS_CLIPCHILDREN, rect, this, 102 );
   ASSERT( ok );

   ok = m_resultsPanel.Create( NULL, "Results View", WS_CHILD | WS_CLIPCHILDREN, rect, this, 104 );
   ASSERT( ok );

   gpInputPanel = &m_inputPanel;
   gpMapPanel = &m_mapPanel;
   gpResultsPanel = &m_resultsPanel;
   gpRtViewsPanel = &m_viewPanel;

   ActiveWnd::SetActiveWnd( &(gpMapPanel->m_mapFrame) );

   // create modeless dialogs
   //m_policyEditor.Create( IDD_POLICYEDITOR, this );   // initially invisible
 
	return 0;
   }


void CEnvView::OnInitialUpdate()
   {
   CView::OnInitialUpdate();
   }


void CEnvView::OnSize(UINT nType, int cx, int cy) 
   {
	CView::OnSize(nType, cx, cy);
	
   // size the embedded views
   if ( m_inputPanel.m_hWnd ) m_inputPanel.MoveWindow( 0, 0, cx, cy, TRUE );

   if ( m_mapPanel.m_hWnd ) m_mapPanel.MoveWindow( 0, 0, cx, cy, TRUE );

   if ( m_dataPanel.m_hWnd ) m_dataPanel.MoveWindow( 0, 0, cx, cy, TRUE );

   if( m_resultsPanel.m_hWnd ) m_resultsPanel.MoveWindow( 0, 0, cx, cy, TRUE );

   if ( m_viewPanel.m_hWnd ) m_viewPanel.MoveWindow( 0, 0, cx, cy, TRUE );
   }


BOOL CEnvView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
   {
/*
   //-- handle the toolbar control beiing pushed --//
   NMHDR *pNotify = (NMHDR*) lParam;

   switch( pNotify->idFrom )
      {
      case 100:   // m_tab control
         {
         if ( pNotify->code == TCN_SELCHANGE )
            {
            int index = TabCtrl_GetCurSel( pNotify->hwndFrom );
            ChangeTabSelection( index );
            }  // end of:  code == TCN_SELCHANGE
         }
      break;
      }
*/
	return CView::OnNotify(wParam, lParam, pResult);
   }


void CEnvView::OnEditCopy() 
   {
   ActiveWnd::OnEditCopy();
   }


void CEnvView::OnEditCopyLegend() 
   {
   ActiveWnd::OnEditCopyLegend();
   }


void CEnvView::OnZoomin() 
   {
   m_mode = MM_ZOOMIN;
   }

void CEnvView::OnZoomout() 
   {
   m_mode = MM_ZOOMOUT;
   }


void CEnvView::OnZoomfull() 
   {
   // check on the active window to see if it is a map
   MapFrame *pMapFrame = ActiveWnd::GetActiveMapFrame();

   if ( pMapFrame != NULL )
      {
      gpMain->SetStatusZoom( 100 );
      pMapFrame->m_pMapWnd->ZoomFull();
      }
   }

void CEnvView::OnZoomToQuery() 
   {
   // check on the active window to see if it is a map
   MapFrame *pMapFrame = ActiveWnd::GetActiveMapFrame();

   if ( pMapFrame != NULL )
      {
      gpMain->SetStatusZoom( 100 );
      MapLayer *pActiveLayer = pMapFrame->m_pMap->GetActiveLayer();
      pMapFrame->m_pMapWnd->ZoomToSelection( pActiveLayer, true );
      }
   }


void CEnvView::SetZoom( int zoomIndex ) 
   {
   // check on the active window to see if it is a map
   MapFrame *pMapFrame = ActiveWnd::GetActiveMapFrame();

   if ( pMapFrame != NULL )
      {
      ZoomInfo *pInfo = GetZoomInfo( zoomIndex );

      //gpMain->SetStatusZoom( 100 );?????
      pMapFrame->m_pMapWnd->ZoomRect(  COORD2d( pInfo->m_xMin, pInfo->m_yMin ), COORD2d( pInfo->m_xMax, pInfo->m_yMax ), true );

      //if ( 
      }
   }




void CEnvView::OnUpdateIsSelection( CCmdUI *pCmdUI )
   {
   if ( gpCellLayer == NULL )
      {
      pCmdUI->Enable( 0 );
      return;
      }

   // map exists, is there anything selected?
   Map *pMap = gpCellLayer->GetMapPtr();
   for ( int i=0; i < pMap->GetLayerCount(); i++ )
      {
      MapLayer *pLayer = pMap->GetLayer( i );

      if ( pLayer->GetSelectionCount() > 0 )
         {
         pCmdUI->Enable( 1 );
         return;
         }
      }

   pCmdUI->Enable( 0 );
   }


void CEnvView::OnUpdateIsMapWnd(CCmdUI* pCmdUI) 
   {
   pCmdUI->Enable( ActiveWnd::IsMapFrame() ? 1 : 0 );
   }


void CEnvView::OnClearSelection()
   {
   MapWindow *pMapWnd = this->m_mapPanel.m_pMapWnd;
   Map *pMap = this->m_mapPanel.m_pMap;
   if ( pMap == NULL )
      return;

   for ( int i=0; i < pMap->GetLayerCount(); i++ )
      pMap->GetLayer( i )->ClearSelection();

   pMapWnd->RedrawWindow();
   }


void CEnvView::OnSelect() 
   {
   m_mode = MM_SELECT;
   }


void CEnvView::OnQuerydistance() 
   {
   m_mode = MM_QUERYDISTANCE;
   }


void CEnvView::OnPan() 
   {
   m_mode = MM_PAN;
   }


void CEnvView::OnUpdateResultsTileCascade(CCmdUI* pCmdUI) 
   {
   BOOL enable = FALSE;

   if ( m_resultsPanel.IsWindowVisible() && m_resultsPanel.m_pResultsWnd->m_wndArray.GetSize() > 0 )
      enable = TRUE;

   pCmdUI->Enable( enable );	
   }


void CEnvView::OnResultsCascade() 
   {
   if ( m_resultsPanel.IsWindowVisible() )
      m_resultsPanel.m_pResultsWnd->Cascade();
   }


void CEnvView::OnResultsTile() 
   {
   if ( m_resultsPanel.IsWindowVisible() )
      m_resultsPanel.m_pResultsWnd->Tile();
   }

void CEnvView::OnResultsClear() 
   {
   if ( m_resultsPanel.IsWindowVisible() )
      m_resultsPanel.m_pResultsWnd->Clear();
   }

void CEnvView::OnResultsCapture() 
   {
   bool status = m_resultsPanel.m_pResultsWnd->m_createMovie;
   m_resultsPanel.m_pResultsWnd->m_createMovie = ( status == true ? false : true );
   }


void CEnvView::OnUpdateResultsCapture( CCmdUI *pCmdUI )
   {
   if ( m_resultsPanel.m_pResultsWnd )
      {
      bool setCheck = ( m_resultsPanel.m_pResultsWnd->m_createMovie ? true : false );
      pCmdUI->SetCheck( setCheck ? 1 : 0 );
      }
   }


void CEnvView::OnResultsSyncMaps() 
   {
   bool status = m_resultsPanel.m_pResultsWnd->m_syncMapZoom;
   m_resultsPanel.m_pResultsWnd->m_syncMapZoom = ( status == true ? false : true );
   }


void CEnvView::OnUpdateResultsSyncMaps( CCmdUI *pCmdUI )
   {
   if ( m_resultsPanel.m_pResultsWnd )
      {
      pCmdUI->Enable( 1 );

      if ( m_resultsPanel.m_pResultsWnd->m_syncMapZoom )
         pCmdUI->SetCheck( 1 );
      else
         pCmdUI->SetCheck( 0 );
      }
   }

void CEnvView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
   {
/*
   if ( gpMap3d && gpMap3d->IsWindowVisible() )
      gpMap3d->OnKeyDown( nChar, nRepCnt, nFlags );
*/
   char lsChar = char(nChar);
   WORD ctrl = HIWORD( (DWORD) GetKeyState( VK_CONTROL ) );

   // CTL + Page Down
   if ( ctrl && nChar == VK_NEXT )
      {
      //ChangeTabSelection( m_tabIndex + 1 );
      }
   
   // CTL + Page Up
   if ( ctrl && nChar == VK_PRIOR )
      {
      //ChangeTabSelection( m_tabIndex - 1 );
      }

   // toggle output window
   if ( ctrl && ( lsChar == 'o' || lsChar == 'O' ) ) 
      gpMain->OnViewOutput();

   ///////////////////////////
   // this is to debug whatever
   if ( ctrl && lsChar == 'R' )
      {
      //// distribute population density based on FID_BLOCKS
      //
      //int colBlock = gpCellLayer->GetFieldCol( "FID_BLOCKS" );
      //int colArea  = gpCellLayer->GetFieldCol( "AREA" );
      //int colPopDens  = gpCellLayer->GetFieldCol( "POPDENS" );
      //
      //CUIntArray idArray;  // this has unique block numbers
      //gpCellLayer->GetUniqueValues( colBlock, idArray );
      //
      //QueryEngine qe( gpCellLayer );
      //
      //CArray< float > cuAreaArray;   // store total area of each block
      //cuAreaArray.SetSize( idArray.GetSize() );
      //
      ////CArray< float > cuPopArray;    // total population of each block
      ////cuPopArray.SetSize( idArray.GetSize() );
      //
      //for ( int i=0; i < (int) idArray.GetSize(); i++ )
      //   {
      //   cuAreaArray[ i ] = 0;
      //   //cuPopArray[ i ] = 0;
      //   }
      //
      //CString msg;
      //msg.Format( "Initializing PopDens - %i Census units found", (int) idArray.GetSize() );
      //Report::LogMsg( msg );
      //
      //for ( int i=0; i < (int) idArray.GetSize(); i++ )
      //   {
      //   if ( i % 10 == 0 )
      //      {
      //      CString msg;
      //      msg.Format( "PopDens: processing %i of %i census units", i, (int) idArray.GetSize() );
      //      Report::StatusMsg( msg );
      //      }
      //   
      //   CString query;
      //   query.Format( "FID_BLOCKS=%i", idArray[ i ] );
      //
      //   Query *pQuery = qe.ParseQuery( query, 0, "" );
      //
      //   int selCount = pQuery->Select( true, false );
      //
      //   for ( int j=0; j < selCount; j++ )
      //      {
      //      int idu = gpCellLayer->GetSelection( j );
      //       
      //      float area = 0;
      //      gpCellLayer->GetData( idu, colArea, area );
      //
      //      //float popDens = 0;
      //      //gpCellLayer->GetData( idu, colPopDens, popDens );
      //
      //      cuAreaArray[ i ] += area;
      //      //cuPopArray[ i ]  += popDens * area;
      //      }
      //
      //   // have sums, now distribute to IDUs
      //   //float popDens = cuPopArray[ i ] / cuAreaArray[ i ];
      //
      //   float blockPopDens = 0;
      //   float blockPop = 0;
      //   for ( int j=0; j < selCount; j++ )
      //      {
      //      int idu = gpCellLayer->GetSelection( j );
      //      
      //      if ( j == 0 )
      //         {
      //         float blockPopDens = 0;
      //         gpCellLayer->GetData( idu, colPopDens, blockPopDens );
      //
      //         float blockPop = blockPopDens * cuAreaArray[ i ];
      //         //blockPopDens = blockPop / cuAreaArray[ i ];
      //         }
      //
      //      float area = 0;
      //      //gpCellLayer->GetData( idu, colArea, area );
      //
      //      float popDens = ( blockPop * area / cuAreaArray[ i ] ) / area;
      //      gpCellLayer->SetData( idu, colPopDens, popDens );
      //      }
      //   }
      //
      //gpCellLayer->ClearSelection();

      

      //CMap< int, int, LPCTSTR, LPCTSTR > map;
      //
      //int colp = gpCellLayer->GetFieldCol( "PRPCLASS" );
      //int cols = gpCellLayer->GetFieldCol( "PRPCLSDSC" );
      //for ( int i=0; i < gpCellLayer->GetRecordCount(); i++ )
      //   {
      //   // get idu info
      //   int p;
      //   CString s;
      //   gpCellLayer->GetData( i, colp, p );
      //   gpCellLayer->GetData( i, cols, s );
      //
      //   LPCTSTR str = NULL;
      //   BOOL found = map.Lookup( p, str );
      //
      //   if ( ! found )
      //      map.SetAt( p, (LPCTSTR) s );
      //   }
      //
      //FILE *fp = fopen( "\\envision\\StudyAreas\\Tillamook\\test.txt", "wt" );
      //for ( int i=0; i < 10000; i++ )
      //   {
      //   LPCTSTR str;
      //   BOOL found = map.Lookup( i, str );
      //
      //   if ( found )
      //      fprintf( fp, "%4i %s\n", i, str );
      //   }
      //fclose( fp );
      }


      /*
      CArray< VData, VData& >oldValues;
      CArray< VData, VData& >newValues;

      // temporary
      oldValues.Add(VData(201));  newValues.Add(VData(200));
      oldValues.Add(VData(202));  newValues.Add(VData(201));
      oldValues.Add(VData(203));  newValues.Add(VData(202));
      oldValues.Add(VData(204));  newValues.Add(VData(203));
      oldValues.Add(VData(205));  newValues.Add(VData(204));
      oldValues.Add(VData(206));  newValues.Add(VData(210));
      oldValues.Add(VData(207));  newValues.Add(VData(211));
      oldValues.Add(VData(208));  newValues.Add(VData(212));
      oldValues.Add(VData(209));  newValues.Add(VData(220));
      oldValues.Add(VData(210));  newValues.Add(VData(221));
      oldValues.Add(VData(212));  newValues.Add(VData(230));
      oldValues.Add(VData(213));  newValues.Add(VData(231));
      oldValues.Add(VData(214));  newValues.Add(VData(232));
      oldValues.Add(VData(215));  newValues.Add(VData(233));
      oldValues.Add(VData(216));  newValues.Add(VData(234));
      oldValues.Add(VData(211));  newValues.Add(VData(239));
      oldValues.Add(VData(218));  newValues.Add(VData(240));
      oldValues.Add(VData(219));  newValues.Add(VData(241));
      oldValues.Add(VData(220));  newValues.Add(VData(242));
      oldValues.Add(VData(221));  newValues.Add(VData(243));
      oldValues.Add(VData(222));  newValues.Add(VData(244));
      oldValues.Add(VData(223));  newValues.Add(VData(245));
      oldValues.Add(VData(224));  newValues.Add(VData(246));
      oldValues.Add(VData(225));  newValues.Add(VData(247));
      oldValues.Add(VData(217));  newValues.Add(VData(259));
      oldValues.Add(VData(232));  newValues.Add(VData(260));
      oldValues.Add(VData(233));  newValues.Add(VData(261));
      oldValues.Add(VData(235));  newValues.Add(VData(262));
      oldValues.Add(VData(236));  newValues.Add(VData(263));
      oldValues.Add(VData(237));  newValues.Add(VData(264));
      oldValues.Add(VData(234));  newValues.Add(VData(265));
      oldValues.Add(VData(231));  newValues.Add(VData(269));
      oldValues.Add(VData(238));  newValues.Add(VData(270));
      oldValues.Add(VData(239));  newValues.Add(VData(271));
      oldValues.Add(VData(226));  newValues.Add(VData(280));
      oldValues.Add(VData(227));  newValues.Add(VData(281));
      oldValues.Add(VData(228));  newValues.Add(VData(282));
      oldValues.Add(VData(229));  newValues.Add(VData(285));
      oldValues.Add(VData(230));  newValues.Add(VData(286));
      oldValues.Add(VData(280));  newValues.Add(VData(300));
      oldValues.Add(VData(281));  newValues.Add(VData(301));
      oldValues.Add(VData(282));  newValues.Add(VData(302));
      oldValues.Add(VData(283));  newValues.Add(VData(303));
      oldValues.Add(VData(284));  newValues.Add(VData(304));
      oldValues.Add(VData(285));  newValues.Add(VData(311));
      oldValues.Add(VData(286));  newValues.Add(VData(312));
      oldValues.Add(VData(287));  newValues.Add(VData(320));
      oldValues.Add(VData(288));  newValues.Add(VData(321));
      oldValues.Add(VData(289));  newValues.Add(VData(333));
      oldValues.Add(VData(290));  newValues.Add(VData(334));
      oldValues.Add(VData(291));  newValues.Add(VData(343));
      oldValues.Add(VData(292));  newValues.Add(VData(344));
      oldValues.Add(VData(293));  newValues.Add(VData(346));
      oldValues.Add(VData(294));  newValues.Add(VData(347));
      oldValues.Add(VData(297));  newValues.Add(VData(363));
      oldValues.Add(VData(298));  newValues.Add(VData(364));
      oldValues.Add(VData(299));  newValues.Add(VData(370));
      oldValues.Add(VData(300));  newValues.Add(VData(371));
      oldValues.Add(VData(295));  newValues.Add(VData(385));
      oldValues.Add(VData(296));  newValues.Add(VData(386));
      oldValues.Add(VData(320));  newValues.Add(VData(440));
      oldValues.Add(VData(321));  newValues.Add(VData(441));
      oldValues.Add(VData(322));  newValues.Add(VData(442));
      oldValues.Add(VData(323));  newValues.Add(VData(443));
      oldValues.Add(VData(324));  newValues.Add(VData(444));
      oldValues.Add(VData(325));  newValues.Add(VData(447));
      oldValues.Add(VData(326));  newValues.Add(VData(448));
      oldValues.Add(VData(330));  newValues.Add(VData(500));
      oldValues.Add(VData(331));  newValues.Add(VData(501));
      oldValues.Add(VData(332));  newValues.Add(VData(502));
      oldValues.Add(VData(333));  newValues.Add(VData(503));
      oldValues.Add(VData(334));  newValues.Add(VData(504));
      oldValues.Add(VData(335));  newValues.Add(VData(505));
      oldValues.Add(VData(336));  newValues.Add(VData(506));
      oldValues.Add(VData(340));  newValues.Add(VData(520));
      oldValues.Add(VData(341));  newValues.Add(VData(521));
      oldValues.Add(VData(342));  newValues.Add(VData(522));
      oldValues.Add(VData(343));  newValues.Add(VData(523));
      oldValues.Add(VData(344));  newValues.Add(VData(524));
      oldValues.Add(VData(345));  newValues.Add(VData(525));
      oldValues.Add(VData(346));  newValues.Add(VData(526));
      oldValues.Add(VData(347));  newValues.Add(VData(527));
      oldValues.Add(VData(350));  newValues.Add(VData(540));
      oldValues.Add(VData(351));  newValues.Add(VData(541));
      int col = gpCellLayer->GetFieldCol( "VEGCLASS" );

      int count = gpCellLayer->Reclass( col, oldValues, newValues );

      CString msg;
      msg.Format( "Reclassed %i records", count );
      MessageBox( msg );
      }
      */

   //if ( ctrl && lsChar == 'J' )
   //   {
   //   //Raster r( gpCellLayer, false );
   //   //int col = gpCellLayer->GetFieldCol( "LULC_B" );
   //   //ASSERT( r.Rasterize( col, 120, RT_32BIT, -1 ) );
   //   //ASSERT( r.SaveDIB( "c:\\envision\\test.bmp" ) );
   //   Map *pMap = gpCellLayer->GetMapPtr();
   //   MapLayer *pLayer = pMap->AddLayer( LT_POLYGON );
   //
   //   for ( int row=0; row < 2; row++ )
   //      {
   //      for ( int col=0; col < 2; col++ )
   //         {
   //         Poly *pPoly = new Poly;  // memory is managed by map layer once the poly is added
   //         float x = col*100.0f;
   //         float y = row*100.0f;
   //         pPoly->AddVertex( Vertex( x, y ) ); // i's should be replaced with real-world coordinates
   //         pPoly->AddVertex( Vertex( x+100, y ) );
   //         pPoly->AddVertex( Vertex( x+100, y+100 ) );
   //         pPoly->AddVertex( Vertex( x, y+100 ) );
   //         pLayer->AddPolygon( pPoly );
   //         }
   //      }
   //
   //   int count = pLayer->GetPolygonCount();
   //   ASSERT( count == 4 );
   //
   //   DataObj  *pDataObj = pLayer->CreateDataTable( 4, 1 );
   //   pLayer->m_pDbTable->SetField( 0, "Test", TYPE_INT, 10, 0, true );
   //   pDataObj->Set( 0, 0, 10 );
   //   pDataObj->Set( 0, 1, 20 );
   //   pDataObj->Set( 0, 2, 30 );
   //   pDataObj->Set( 0, 3, 40 );
   //   pLayer->SaveShapeFile( "c:\\envision\\StudyAreas\\Boxes\\boxes.shp" );
   //   }
   //
   //if ( ctrl && lsChar == 'K' )
   //   {
   //   }
   //
   //if ( ctrl && lsChar == 'L' )
   //   {
   //   //RandomizeLulcLevel( 3 );
   //   //RandomizeLulcLevel( 4 );
   //   }
   //
   //if ( ctrl && lsChar == 'R' )
   //   {
   //   ALRadialBasisFunction3D rbf;
   //   rbf.Create( 8 );  // number of outputs
   //
   //   Report::LogMsg( "opening input file..." );
   //   FDataObj inputData;
   //   inputData.ReadAscii( "\\AlgLib\\AlgLib\\Inputs\\OffshoreConditions.csv" );
   //   
   //   Report::LogMsg( "setting points...\n" );
   //   rbf.SetPoints( inputData );  // rows are input points, col0=x, col1=y, col2=z, col3=f0, col4=f1 etc...
   //   
   //   // optionally set type of model (default is QNN)
   //   // (radius should be about avg distance between neighboring points)
   //   rbf.SetML( 1.0, 5, 0.005 );   // radius, layers, lambdaV 
   //
   //   // build the RBF
   //   clock_t start = clock();
   //
   //   Report::LogMsg( "Building network...\n" );
   //   rbf.Build();
   //
   //   clock_t finish = clock();
   //   double buildTime = (float)(finish - start) / CLOCKS_PER_SEC;   
   //   CString msg;
   //   msg.Format( "RBF built (%.1f seconds)", (float) buildTime );
   //   Report::LogMsg( msg );
   //
   //   // ready to query for values
   //   Report::LogMsg( "reading observation file...\n" );
   //   FDataObj bouyObsData;
   //   bouyObsData.ReadAscii( "\\AlgLib\\AlgLib\\Inputs\\BouyObsData.csv" );
   //   
   //   Report::LogMsg( "writing output file...\n" );
   //   FILE *fp = fopen( "\\AlgLib\\AlgLib\\Inputs\\output.csv", "wt" );
   //
   //   start = clock();
   //
   //
   //   int outputCount = inputData.GetColCount() - 3;
   //
   //   double *outputs = new double[ outputCount ];
   //   
   //   for ( int i=0; i < bouyObsData.GetRowCount(); i++ )
   //      {
   //      float x, y, z;
   //      bouyObsData.Get( 0, i, x );
   //      bouyObsData.Get( 1, i, y );
   //      bouyObsData.Get( 2, i, z );
   //
   //      int count = rbf.GetAt( x, y, z, outputs );
   //      ASSERT( count == outputCount );
   //
   //      fprintf( fp, "%f, %f, %f, ", (float) x, (float) y, (float) z );
   //
   //      for ( int j=0; j < outputCount; j++ )
   //         {
   //         fprintf( fp, "%f", outputs[ j ] );
   //
   //         if ( j != outputCount-1 )
   //            fprintf( fp, ", " );
   //         else
   //            fprintf( fp, "\n" );
   //         }
   //      }
   //
   //   fclose( fp );
   //
   //   
   //   finish = clock();
   //   double outputTime = (float)(finish - start) / CLOCKS_PER_SEC;   
   //   msg.Format( "Output file written (%.1f seconds)", (float) outputTime );
   //   Report::LogMsg( msg );
   //
   //   delete [] outputs;
   //   }

	//
   ///////////////////////////

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
   }


void CEnvView::OnViewInput()
   {
   ActivatePanel( 0 );
   }

void CEnvView::OnViewMap()
   {
   ActivatePanel( 1 );
   }

void CEnvView::OnViewTable()
   {
   ActivatePanel( 2 );
   }

void CEnvView::OnViewRtViews()
   {
   ActivatePanel( 3 );
   }

void CEnvView::OnViewResults()
   {
   ActivatePanel( 4 );
   }

void CEnvView::ActivatePanel( int index )
   {
   //gpMain->SetRibbonContextCategory( 0 );
   if ( m_activePanel != index )
      {
      gpMain->SetRibbonContextCategory( 0 );

      m_inputPanel.ShowWindow  ( index==0 ? SW_SHOW : SW_HIDE );
      m_mapPanel.ShowWindow    ( index==1 ? SW_SHOW : SW_HIDE );
      m_dataPanel.ShowWindow   ( index==2 ? SW_SHOW : SW_HIDE );
      m_viewPanel.ShowWindow   ( index==3 ? SW_SHOW : SW_HIDE );
      m_resultsPanel.ShowWindow( index==4 ? SW_SHOW : SW_HIDE );
      m_activePanel = index;
      //
      switch( index )
         {
         case 0:     // input panel
            gpInputPanel->RedrawWindow();
            ActiveWnd::SetActiveWnd( gpInputPanel );
            break;

         case 1:     // map panel
            ActiveWnd::SetActiveWnd( &gpMapPanel->m_mapFrame );
            break;

         case 2:     // data panel
            {
            DataGrid *pGrid = &(m_dataPanel.m_grid);
            ActiveWnd::SetActiveWnd( pGrid );
            }
            break;

         case 3:     // 
            ActiveWnd::SetActiveWnd( m_viewPanel.GetActiveView() );
            gpMain->SetRibbonContextCategory( IDCC_RTVIEWS );
            break;

         case 4:  // results
            ActiveWnd::SetActiveWnd( m_resultsPanel.GetActiveView() );
            gpMain->SetRibbonContextCategory( IDCC_RESULTS );
            break;
         }
      }
   }


void CEnvView::OnViewPolygonedges() 
   {
   MapFrame *pMapFrame = ActiveWnd::GetActiveMapFrame();

   if ( pMapFrame == NULL )
      {
      MessageBeep( 0 );
      return;
      }

   MapLayer *pLayer = pMapFrame->m_pMap->GetActiveLayer();
   if ( pLayer == NULL )
      return;

   long edge = pLayer->GetOutlineColor();

   if ( edge == NO_OUTLINE )
      pLayer->SetOutlineColor( RGB( 127, 127, 127 ) );
   else
      pLayer->SetNoOutline();

   pMapFrame->m_pMapWnd->RedrawWindow();
   }


void CEnvView::OnUpdateViewPolygonedges(CCmdUI* pCmdUI) 
   {
   MapFrame *pMapFrame = ActiveWnd::GetActiveMapFrame();

   if ( pMapFrame == NULL || pMapFrame->m_pMap->GetActiveLayer() == NULL )
      {
      pCmdUI->Enable( FALSE );
      return;
      }

   MapLayer *pLayer = pMapFrame->m_pMap->GetActiveLayer();
   
   if ( pLayer->GetOutlineColor() == NO_OUTLINE )
      {
      pCmdUI->Enable();
      pCmdUI->SetCheck( 0 );
      }
   else
      {
      pCmdUI->Enable();
      pCmdUI->SetCheck( 1 );
      }	
   }


void CEnvView::OnViewShowMapScale() 
   {
   if ( gpMapPanel == NULL || gpMapPanel->m_pMapWnd == NULL )
      return;

   gpMapPanel->m_pMapWnd->ShowScale( gpMapPanel->m_pMapWnd->IsScaleShowing() ? false : true );
   gpMapPanel->m_pMapWnd->RedrawWindow();
   }


void CEnvView::OnUpdateViewShowMapScale(CCmdUI* pCmdUI) 
   {
   if ( gpMapPanel == NULL || gpMapPanel->m_pMapWnd == NULL )
      {
      pCmdUI->Enable( 0 );
      return;
      }
   
   pCmdUI->Enable();
   pCmdUI->SetCheck( gpMapPanel->m_pMapWnd->IsScaleShowing() ? 1 : 0 );
   }


void CEnvView::OnViewShowAttribute() 
   {
   if ( gpMapPanel == NULL || gpMapPanel->m_pMapWnd == NULL )
      return;

   gpMapPanel->m_pMapWnd->ShowAttributeOnHover( gpMapPanel->m_pMapWnd->IsShowAttributeOnHover() ? false : true );

   // set for results maps as well?

   }


void CEnvView::OnUpdateViewShowAttribute(CCmdUI* pCmdUI) 
   {
   if ( gpMapPanel == NULL || gpMapPanel->m_pMapWnd == NULL )
      {
      pCmdUI->Enable( 0 );
      return;
      }
   
   pCmdUI->Enable();
   pCmdUI->SetCheck( gpMapPanel->m_pMapWnd->IsShowAttributeOnHover() ? 1 : 0 );
   }



void CEnvView::OnAnalysisShowactorterritory() 
   {
   SelectActor dlg;
   if ( dlg.DoModal() == IDOK )
      {
      int index = dlg.m_index;
      Actor *pActor = gpActorManager->GetActor( index );

      gpCellLayer->ClearSelection();

      for ( int i=0; i < pActor->GetPolyCount(); i++ )
         {
         ASSERT( ! gpCellLayer->IsDefunct( pActor->GetPoly( i ) ) );
         gpCellLayer->AddSelection( pActor->GetPoly( i ) );
         }

      gpMapPanel->RedrawWindow();
      }	
   }

void CEnvView::OnAnalysisShowactors() 
   {
   ActorDlg dlg;
   dlg.DoModal();	
   }


BOOL CEnvView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
   {
	// TODO: Add your message handler code here and/or call default
   // gpMap3d->OnMouseWheel( nFlags, zDelta, pt );
	
	return CView::OnMouseWheel(nFlags, zDelta, pt);
   }


void CEnvView::OnUpdateData(CCmdUI* pCmdUI) 
   {
	switch ( pCmdUI->m_nID )
	   {
      case ID_FILE_RESET:
         pCmdUI->Enable( /*gpDoc->m_currentRun > 0 ? TRUE :*/ FALSE );
         break;
      case ID_DATA_EXPORTDATA:
         pCmdUI->Enable( gpDoc->m_model.m_currentRun > 0 ? TRUE : FALSE );
         break;
	   default:
         pCmdUI->Enable( gpCellLayer != 0 ? TRUE : FALSE );
         break;
	   }
   }


void CEnvView::OnEditPolicies()
   {
   if ( m_policyEditor.GetSafeHwnd() == 0 )
      return;
   else
      {
      if ( m_policyEditor.IsWindowVisible() )
         m_policyEditor.ShowWindow( SW_HIDE );
      else
         {
         m_policyEditor.ShowWindow( SW_SHOW );

         if ( gpPolicyManager->GetPolicyCount() == 0 )
            m_policyEditor.AddNew();
         }
      }
   }


void CEnvView::OnEditActors()
   {
   ActorDlg dlg;
   dlg.DoModal();
   }


void CEnvView::OnEditLulc()
   {
   LulcEditor dlg;
   dlg.DoModal();
   }

void CEnvView::OnDataLulcInfo()
   {
	//LulcInfoDialog dlg;
	//dlg.DoModal();

   if ( !IsWindow( m_lulcInfoDialog.m_hWnd ) )
      {
      m_lulcInfoDialog.Create( this );
      m_lulcInfoDialog.ShowWindow( SW_SHOW );
      }
   else
      m_lulcInfoDialog.SetFocus();
   }


void CEnvView::OnDataIduInfo()
   {
	DatabaseInfoDlg dlg( gpCellLayer );
   dlg.DoModal();   
   }


void CEnvView::OnAnalysisSetupRun()
   {
   WizRun wizRun( "Setup Run", this );
   wizRun.DoModal();

   // create a new run and run
   if ( wizRun.m_runState == 1 ) // do a single run
      {
      //if ( gpModel->m_currentRun == 0 
      //if ( gpModel->m_currentYear > 0 )
      //   m_outputTab.m_pViewTab->NewRun();

      ///???V5 m_outputTab.m_pViewTab->OnAnalysisRun();
      }

   else if ( wizRun.m_runState == 2 )  // do a multirun
      gpDoc->OnAnalysisRunmultiple();
   }



void CEnvView::OnVideoRecorders()
   {
   VideoRecorderDlg dlg;
   dlg.DoModal();
   }


void CEnvView::OnSaveShapefile()
   {
   char *cwd = _getcwd( NULL, 0 );

   CFileDialog dlg( FALSE, "shp", gpCellLayer->m_path, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
         "Shape Files|*.shp|All files|*.*||");

   if ( dlg.DoModal() == IDOK )
      {
      CString filename( dlg.GetPathName() );
      int count = gpCellLayer->SaveShapeFile( filename, FALSE );

      if ( count <= 0 )
         MessageBox( "Unable to save file", "File Error" );
      }

   _chdir( cwd );
   free( cwd );
   }


void CEnvView::OnUpdateSaveShapefile(CCmdUI *pCmdUI)
   {
   if ( gpCellLayer )
      pCmdUI->Enable( TRUE );
   else
      pCmdUI->Enable( FALSE );
   }

/*
void CEnvView::OnConnectToHost()
   {
   CPropertySheet sheet( "Interaction Session", this, 1 );
   PPClient clientPage;
   PPHost   hostPage;
   sheet.AddPage( &hostPage );
   sheet.AddPage( &clientPage );
   sheet.DoModal();
   }

void CEnvView::OnUpdateConnectToHost(CCmdUI *pCmdUI)
   {
   }

void CEnvView::OnHostSession()
   {
   CPropertySheet sheet( "Interaction Session", this, 0 );
   PPClient clientPage;
   PPHost   hostPage;
   sheet.AddPage( &hostPage );
   sheet.AddPage( &clientPage );
   sheet.DoModal();
   }

void CEnvView::OnUpdateHostSession(CCmdUI *pCmdUI)
   {
   }
*/

void CEnvView::OnAnalysisShowdeltalist()
   {
   DeltaViewer viewer( this );
   viewer.DoModal();
   }

void CEnvView::OnUpdateAnalysisShowdeltalist(CCmdUI *pCmdUI)
   {
   pCmdUI->Enable( gpModel->m_pDataManager->GetDeltaArrayCount() > 0 ? TRUE : FALSE );
   }

void CEnvView::OnAnalysisSetuppolicymetaprocess()
   {
   //gpDoc->m_model.m_policyMetaProcess.Setup(this);
   }

void CEnvView::OnUpdateAnalysisSetuppolicymetaprocess(CCmdUI *pCmdUI)
   {
   if ( gpCellLayer == NULL )
      pCmdUI->Enable( FALSE );
   else
      pCmdUI->Enable( TRUE );
   }


void CEnvView::OnDataStorerundata()
   {
   char *cwd = _getcwd( NULL, 0 );

   CFileDialog dlg( FALSE, "edd", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "EnvDataDump Files|*.edd|All files|*.*||" );

   if ( dlg.DoModal() == IDOK )
      {
      CString fileName = dlg.GetPathName();
      if ( ! gpModel->m_pDataManager->SaveRun( fileName ) )
         Report::ErrorMsg( "Error Saving Run." );
      }   

   _chdir( cwd );
   free( cwd );
   }

void CEnvView::OnDataLoadrundata()
   {
   char *cwd = _getcwd( NULL, 0 );

   CFileDialog dlg( TRUE, "edd", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "EnvDataDump Files|*.edd|All files|*.*||" );

   if ( dlg.DoModal() == IDOK )
      {
      CWaitCursor c;
      if ( ! gpModel->m_pDataManager->LoadRun( dlg.GetPathName() ) )
         {
         MessageBox( "Error Loading Run.", 0, MB_ICONERROR | MB_OK );
         }
      }

   _chdir( cwd );
   free( cwd );
   }

void CEnvView::OnRunquery()
   {
   MapLayer *pLayer = m_mapPanel.m_pMap->GetActiveLayer();
   QueryDlg dlg( pLayer );
   dlg.DoModal();
   }



void CEnvView::OnUpdateEditCopy(CCmdUI *pCmdUI)
   {
   ActiveWnd::OnUpdateEditCopy( pCmdUI );
   }


void CEnvView::OnFilePrint()
   {
   ActiveWnd::OnFilePrint();
   }


void CEnvView::OnUpdateFilePrint(CCmdUI *pCmdUI)
   {
   ActiveWnd::OnUpdateFilePrint( pCmdUI );
   }


void CEnvView::OnClose()
   {
   //this->m_outputTab.m_scenarioTab.OnBnClickedOk();      // check to see is scenarios need to be saved   
   CView::OnClose();
   }

void CEnvView::OnEditSetpolicycolors()
   {
   PolicyColorDlg dlg;
   dlg.DoModal();
   }


void CEnvView::OnEditScenarios()
   {
   ScenarioEditor dlg;
   dlg.DoModal();
   }




void CEnvView::OnExportCoverages()
   {
   gpDoc->m_model.m_exportMaps = ! gpDoc->m_model.m_exportMaps; //gpMain->m_pExportCoverages->IsChecked() ? true : false;
   }


void CEnvView::OnUpdateExportCoverages(CCmdUI *pCmdUI)
   {
   pCmdUI->Enable();
   pCmdUI->SetCheck( gpDoc->m_model.m_exportMaps ? 1 : 0 );
   }


void CEnvView::OnUpdateExportInterval(CCmdUI *pCmdUI)
   {
   //TCHAR buffer[ 32 ];
   //_itoa_s( gpDoc->m_model.m_exportMapInterval, buffer, 32, 10 );
   //gpMain->SetExportInterval( gpDoc->m_model.m_exportMapInterval ); ////pCmdUI->SetText( buffer );

   pCmdUI->Enable( gpDoc->m_model.m_exportMaps ? 1 : 0 );
   }


void CEnvView::OnExportOutputs()
   {
   gpDoc->m_model.m_exportOutputs = ! gpDoc->m_model.m_exportOutputs;
   }


void CEnvView::OnUpdateExportOutputs(CCmdUI *pCmdUI)
   {
   pCmdUI->Enable();
   pCmdUI->SetCheck( gpDoc->m_model.m_exportOutputs ? 1 : 0 );
   }

void CEnvView::OnExportDeltas()
   {
   gpDoc->m_model.m_exportDeltas = ! gpDoc->m_model.m_exportDeltas;
   }


void CEnvView::OnUpdateExportDeltas(CCmdUI *pCmdUI)
   {
   pCmdUI->Enable();
   pCmdUI->SetCheck( gpDoc->m_model.m_exportDeltas ? 1 : 0 );
   }

void CEnvView::OnExportDeltaFields()
   {
   ;  //gpDoc->m_model.m_exportDeltas = ! gpDoc->m_model.m_exportDeltas;
   }


void CEnvView::OnUpdateExportDeltaFields(CCmdUI *pCmdUI)
   {
   pCmdUI->Enable( gpDoc->m_model.m_exportDeltas ? 1 : 0 ); 
   }


void CEnvView::OnUpdateStartYear(CCmdUI *pCmdUI)
   {
   pCmdUI->Enable(); 
   }

void CEnvView::OnUpdateStopTime(CCmdUI *pCmdUI)
   {
   pCmdUI->Enable(); 
   }




void CEnvView::OnZoomSlider()
   {
   float zoomPct = gpMain->GetZoomSliderPos()/100.0f;

   MapFrame *pMapFrame = ActiveWnd::GetActiveMapFrame();

   if ( pMapFrame && pMapFrame->m_pMapWnd )
      {
      // a larger zoomFactor zooms in, a smaller zoomFactor zooms out.
      // a zoom factor of 1.0 (actually, 0.9) zoom full
      // for a map, the "most zoomed" state is a zoom factor of 100, corresponding to
      // a zoomPct of 1.0.
      // the "least zoomed" state is full zoom, meaning a zoomFactor of 0.9, correspinding
      // to a zoomPct of 0;
      double r = log( 50.0-0.9 );
      float zoomFactor = float( 0.9 + ( exp( r * zoomPct )-1 ) );
      pMapFrame->m_pMapWnd->SetZoomFactor( zoomFactor, true );
      }
   }


void CEnvView::OnZoomButton()
   {
   
   }



void CEnvView::RandomizeLulcLevel( int level /*one-based*/)
   {
   // level 0 = root  (illegal)
   // level 1 = LULC_A  (illegal)
   // level 2 = LULC_B  
   // ...etc..

   LulcTree &lulcTree = EnvModel::m_lulcTree;

   int levels = lulcTree.GetLevels();  // doesn't include root level
   ASSERT( level <= levels );
   ASSERT( level > 1 );

   CString &parentField = lulcTree.GetFieldName( level-1 );
   MAP_FIELD_INFO *pInfoParent = gpCellLayer->FindFieldInfo( parentField );
   if ( pInfoParent == NULL )
      return;

   CString &childField = lulcTree.GetFieldName( level );
   MAP_FIELD_INFO *pInfoChild = gpCellLayer->FindFieldInfo( childField );
   if ( pInfoChild == NULL )
      return;

   LulcNode *pNode = lulcTree.GetRootNode();
   
   RandUniform r;

   while( (pNode = lulcTree.GetNextNode()) != NULL )
      {
      if ( pNode->GetNodeLevel() == level )
         {
         FIELD_ATTR *pAttr = pInfoChild->FindAttribute( pNode->m_id );
         if ( pAttr == NULL )
            continue;

         // this is a node we want to get child from.  First, find the color of this node
         LulcNode *pParentNode = pNode->m_pParentNode;
         FIELD_ATTR *pParentAttr = pInfoParent->FindAttribute( pParentNode->m_id );
         if ( pParentAttr == NULL )
            continue;

         COLORREF color = pParentAttr->color;
         int red = GetRValue( color );
         int grn = GetGValue( color );
         int blu = GetBValue( color );

         red += (int) r.RandValue( -25, 25 );
         grn += (int) r.RandValue( -25, 25 );
         blu += (int) r.RandValue( -25, 25 );
         
         pAttr->color = RGB( red, grn, blu );
         }
      }
   }


BOOL CEnvView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
   {
   if ( CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo) )
      return TRUE;

   // add rtview panel to command handlers
   return this->m_viewPanel.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
   }


bool CEnvView::AddStandardRecorders( void )
   {
   nsPath::CPath path( gpDoc->m_iniFile );
   nsPath::CPath fullPath = path.MakeFullPath();

   fullPath.RemoveFileSpec();
   fullPath.Append( "\\Outputs\\AVIs" );
   SHCreateDirectoryEx( NULL, fullPath, NULL );

   // main window
   CString _path = fullPath + "\\MainWnd.avi";
   VideoRecorder *pVR = new VideoRecorder( _path, gpMain, 30 );
   pVR->m_enabled = false;
   pVR->m_useApplication = true;
   pVR->m_name = "Main Envision Window";
   AddVideoRecorder( pVR );

   // map panel
   _path = CString( fullPath  ) + "\\MapPanel.avi";
   pVR = new VideoRecorder( _path, &m_mapPanel, 30 );
   pVR->m_enabled = false;
   pVR->m_useApplication = true;
   pVR->m_name = "Map Panel";
   AddVideoRecorder( pVR );

   // results panel
   _path = CString( fullPath ) + "\\ResultsPanel.avi";
   pVR = new VideoRecorder( _path, m_resultsPanel.m_pResultsWnd, 30 );
   pVR->m_enabled = false;
   pVR->m_useApplication = true;
   pVR->m_name = "Results Panel";
   AddVideoRecorder( pVR );

   // rtviews panel
   _path = CString( fullPath ) + "\\RtViewsPanel.avi";
   pVR = new VideoRecorder( _path, &m_viewPanel, 30 );
   pVR->m_enabled = false;
   pVR->m_useApplication = true;
   pVR->m_name = "Runtime Views Panel";
   AddVideoRecorder( pVR );

   // input panel
   _path = (CString) fullPath + "\\InputPanel.avi";
   pVR = new VideoRecorder( _path, &m_inputPanel, 30 );
   pVR->m_enabled = false;
   pVR->m_useApplication = true;
   pVR->m_name = "Input Panel";
   AddVideoRecorder( pVR );

   // main map (no legend tree)
   _path = (CString) fullPath + "\\ResultsPanel.avi";
   pVR = new VideoRecorder( _path, m_mapPanel.m_pMapWnd, 30 );
   pVR->m_enabled = false;
   pVR->m_useApplication = true;
   pVR->m_name = "Main Map";
   AddVideoRecorder( pVR );

   return true;
   }



void CEnvView::StartVideoRecorders( void )
   {
   for ( int i=0; i < GetVideoRecorderCount(); i++ )
      {
      VideoRecorder *pVR = GetVideoRecorder( i );

      ASSERT( pVR != NULL );

      if ( pVR->m_enabled && pVR->m_useApplication )
         {
         pVR->StartCapture();
         }
      }
   }


void CEnvView::UpdateVideoRecorders( void )
   {
   for ( int i=0; i < GetVideoRecorderCount(); i++ )
      {
      VideoRecorder *pVR = GetVideoRecorder( i );

      ASSERT( pVR != NULL );

      if ( pVR->m_enabled && pVR->m_useApplication )
         {
         pVR->CaptureFrame();
         }
      }
   }

void CEnvView::StopVideoRecorders( void )
   {
   for ( int i=0; i < GetVideoRecorderCount(); i++ )
      {
      VideoRecorder *pVR = GetVideoRecorder( i );

      ASSERT( pVR != NULL );

      if ( pVR->m_enabled && pVR->m_useApplication )
         {
         pVR->EndCapture();
         }
      }
   }


void CEnvView::UpdateUI( int flags, INT_PTR extra )
   {
   if ( ( gpModel->m_dynamicUpdate & flags ) == 0 )
      return;

   //if ( m_inMultiRun )
   //   return;

   if ( flags & 2 )  // update map.   TODO: Implement dynamic update for multi-run case
      {
      m_mapPanel.m_mapFrame.Update();
      UpdateVideoRecorders();
      }

   if ( flags & 4 ) // show year
      {
      CString msg;

      if ( gpModel->m_inMultiRun )         
         {
         int currentRun = gpModel->m_currentRun % gpModel->m_iterationsToRun;
         msg.Format( "%i%i:%i", gpModel->m_currentMultiRun, currentRun, gpModel->m_currentYear );
         }
      else
         msg.Format( "%i", gpModel->m_currentYear );

      m_mapPanel.UpdateText( msg );
      }

   if ( flags & 8 )    // process - extra pts to EnvExtension
      {
      if ( extra == 0 )
         m_mapPanel.UpdateSubText( "Actor Decision-making" );
      else
         {
         ENV_EXTENSION *pExt = (ENV_EXTENSION*) extra;
         m_mapPanel.UpdateSubText( pExt->name );
         }
      }

   if ( flags & 16 )  // update model msg
      {
      LPCTSTR name = (LPCTSTR) extra;
      gpMain->SetModelMsg( name );
      }
   }


void CEnvView::EndRun()
   {
   //ENV_ASSERT( m_pIDULayer->GetActiveField() >= 0  && gpMapPanel != NULL);
   m_mapPanel.m_mapFrame.OnSelectField( m_mapPanel.m_pMap->GetActiveLayer()->GetActiveField() );

   int vizCount = m_vizManager.GetVisualizerCount();
    
   for ( int i=0; i < vizCount; i++ )
      {
      VisualizerWnd *pVizWnd = m_vizManager.GetVisualizerWnd( i );
   
      ENV_VISUALIZER *pInfo = pVizWnd->m_pVizInfo;
   
      if ( pInfo->use && pInfo->endRunFn != NULL )
         {
         // call the function (eval models should return scores in the range of -3 to +3)
         if ( gpModel->m_showRunProgress )
            gpMain->SetModelMsg( pInfo->name );
   
         gpModel->m_envContext.pExtensionInfo  = pInfo;
         gpModel->m_envContext.id     = pInfo->id;
         gpModel->m_envContext.handle = pInfo->handle;
         gpModel->m_envContext.col    = -1;
         gpModel->m_envContext.firstUnseenDelta = gpModel->GetFirstUnseenDeltaViz( i );
         gpModel->m_envContext.lastUnseenDelta = gpModel->m_envContext.pDeltaArray->GetSize();
   
         gpModel->m_envContext.pWnd   = pVizWnd;
   
         clock_t start = clock();
   
         BOOL ok = pInfo->endRunFn( &gpModel->m_envContext );
   
         clock_t finish = clock();
         double duration = (float)(finish - start) / CLOCKS_PER_SEC;   
         pInfo->runTime += (float) duration;         
   
         if ( gpModel->m_showRunProgress )
            {
            CString msg( pInfo->name );
            msg += " - Completed";
            gpMain->SetModelMsg( msg );
            }
   
         if ( ! ok )
            {
            CString msg = "The ";
            msg += pInfo->name;
            msg += " Visualizer returned FALSE indicating an error.";
            throw new EnvRuntimeException( msg );
            }
         }
      }  // end of:  for( i < vizCount )

   StopVideoRecorders();   // generates AVI files
   return;
   }
