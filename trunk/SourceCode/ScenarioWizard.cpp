// ScenarioWizard.cpp : implementation file
//

#include "stdafx.h"


#pragma hdrstop

#include "Envision.h"
#include "ScenarioWizard.h"
#include "ActiveWnd.h"
#include "EnvView.h"
#include "EnvDoc.h"
#include "EnvEngine\EnvModel.h"
#include "EnvEngine\policy.h"
#include "MainFrm.h"

#include <Maplayer.h>

extern CEnvDoc       *gpDoc;
extern CEnvView      *gpView;
extern PolicyManager *gpPolicyManager;
extern MapLayer      *gpCellLayer;
extern CMainFrame    *gpMain;
// ScenarioWizard


//int MapProc( Map *pMap, NOTIFY_TYPE type, int a0, LONG_PTR a1, LONG_PTR extra );
//int MapTextProc( Map *pMap, NOTIFY_TYPE type, LPCTSTR msg );


IMPLEMENT_DYNAMIC(ScenarioWizard, CWnd)

ScenarioWizard::ScenarioWizard()
: CWnd()
, m_currentPage( -1 )
{
}


ScenarioWizard::~ScenarioWizard()
{
}


BEGIN_MESSAGE_MAP(ScenarioWizard, CWnd)
   ON_WM_CREATE()
	ON_WM_SIZE()
  	//ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	//ON_WM_ACTIVATE()
	//ON_COMMAND(ID_CELLPROPERTIES, OnCellproperties)
	//ON_COMMAND(ID_SHOWSAMEATTRIBUTE, OnShowSameAttribute)
   //ON_COMMAND(ID_RUNQUERY, OnRunQuery)
   //ON_COMMAND(ID_SHOWPOLYGONVERTICES, OnShowPolygonVertices)
   //ON_COMMAND(ID_ZOOMTOPOLY, OnZoomToPoly)
	//ON_COMMAND(ID_RECLASSIFYDATA, OnReclassifydata)
	//ON_WM_MOUSEACTIVATE()
   //ON_COMMAND_RANGE(40000, 60000, OnSelectField)
END_MESSAGE_MAP()


// ScenarioWizard message handlers


// ScenarioWizard message handlers

int ScenarioWizard::OnCreate(LPCREATESTRUCT lpCreateStruct)
   {
   if (CWnd::OnCreate(lpCreateStruct) == -1)
      return -1;

   // page 1
   ScenarioPage *pPage = new ScenarioPage;
   pPage->m_name = _T("Select a Scenario" );










   // m_mapWnd.Create( NULL, _T("MapWnd"), WS_CHILD | WS_VISIBLE, CRect( 0,0,0,0), this, 100, NULL );
   // 
   // m_pMap = m_mapWnd.m_pMap;
   // 
   // m_pMap->InstallNotifyHandler( MapProc, (LONG_PTR) &m_mapWnd );
   // m_pMap->InstallNotifyTextHandler( MapTextProc );
   // 
   // MapWnd::m_setupBinsFn = SetupBins;
   // 
   // ActiveWnd::SetActiveWnd( &m_mapWnd );
   // 
   // // Map Text definitions       X    Y     position flags,                                           fontSize
   // m_pMap->AddMapText( "",      20,   20,   MTP_ALIGNRIGHT | MTP_FROMRIGHT | MTP_FROMTOP,     180 );    // text
   // m_pMap->AddMapText( "",      20,   45,   MTP_ALIGNRIGHT | MTP_FROMRIGHT | MTP_FROMTOP,     100 );    // subtext 
   // m_pMap->AddMapText( "",      20,   20,   MTP_ALIGNLEFT  | MTP_FROMLEFT  | MTP_FROMBOTTOM,  100 );    // lower right
   // m_pMap->AddMapText( "",      20,   20,   MTP_ALIGNLEFT  | MTP_FROMLEFT  | MTP_FROMTOP,     180 );    // upper right

   return 0;
   }


void ScenarioWizard::OnSize(UINT nType, int cx, int cy) 
   {
	CWnd::OnSize(nType, cx, cy);

   //if ( IsWindow( m_mapWnd.m_hWnd ) )
   //   {
   //   m_mapWnd.MoveWindow( 0, 0, cx, cy, TRUE );
   //   }	
   }


//int MapTextProc( Map *pMap, NOTIFY_TYPE type, LPCTSTR msg )
//   {
//   gpMain->SetStatusMsg( msg );
//
//   return 1;
//   }




void ScenarioWizard::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
   {
	CWnd::OnActivate(nState, pWndOther, bMinimized);
	
   if ( nState != WA_INACTIVE )
      ;  // ActiveWnd::SetActiveWnd( &(this->m_mapWnd) );   
   }
  
