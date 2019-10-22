// ResultsGraphWnd.cpp : implementation file
//

#include "stdafx.h"
#include "Envision.h"
#include "ResultsGraphWnd.h"
#include "ActiveWnd.h"
#include "EnvEngine\EnvModel.h"
#include "EnvEngine\DataManager.h"

#include "mainfrm.h"

extern EnvModel   *gpModel;
extern CMainFrame *gpMain;

// ResultsGraphWnd

ResultsGraphWnd::ResultsGraphWnd( int run, bool /* isDifference */ )
 : m_run( run )
 , m_year( -1 )
 , m_startYear( -1 )
 , m_endYear( -1 )
 , m_pGraphWnd( NULL )
 , m_graphType( 0 )        // assume scatter by default
   { }


ResultsGraphWnd::~ResultsGraphWnd( void )
   {
   if ( m_pGraphWnd )
      delete m_pGraphWnd;

   m_pGraphWnd = NULL;
   }


BEGIN_MESSAGE_MAP(ResultsGraphWnd, CWnd)
   ON_WM_CREATE()
   ON_WM_SIZE()
   ON_WM_ACTIVATE()
   ON_WM_MOUSEACTIVATE()
END_MESSAGE_MAP()



// ResultsGraphWnd message handlers

BOOL ResultsGraphWnd::PreCreateWindow(CREATESTRUCT& cs)
   {
   cs.lpszClass = AfxRegisterWndClass( CS_VREDRAW | CS_HREDRAW, 
		::LoadCursor(NULL, IDC_SIZEWE), reinterpret_cast<HBRUSH>(COLOR_WINDOW), NULL);

   return CWnd::PreCreateWindow(cs);
   }


int ResultsGraphWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
   {
   if (CWnd::OnCreate(lpCreateStruct) == -1)
      return -1;

   bool multirun = ( gpModel->m_pDataManager->GetDeltaArrayCount() > 1 );
   
   //m_totalYears = m_pDeltaArray->GetMaxYear() + 1;  // + 1 to point to the end of the DeltaArray
   // adjust for non-zero start year
   //m_totalYears -= gpModel->m_startYear;
   m_startYear = gpModel->m_startYear;
   m_endYear   = gpModel->m_endYear;
   m_year = m_startYear;


   // map and mapList
   //m_graphWnd.Create( NULL, _T("DynamicScatterWnd" ), WS_VISIBLE, CRect( 0,0,0,0 ), this, 101 );

   ActiveWnd::SetActiveWnd( m_pGraphWnd );
   return 0;
   }


void ResultsGraphWnd::OnSize(UINT nType, int cx, int cy)
   {
   CWnd::OnSize(nType, cx, cy);

   if ( m_pGraphWnd != NULL )
      {
      const int gutter = 5;
   
      CSize sz( (cx - 3*gutter)/2, 400 );
      CPoint pt( gutter, gutter );
   
      if ( IsWindow( m_pGraphWnd->m_hWnd ) )
         m_pGraphWnd->MoveWindow( 0, 0, cx, cy - 2*gutter, TRUE );
      }
   }


 bool ResultsGraphWnd::_SetYear( int oldYear, int newYear )
   {
   if ( oldYear != newYear )
      {
      m_year = newYear;
      Shift( oldYear, newYear );
      }

   return true;
   }


void ResultsGraphWnd::Shift( int fromYear, int toYear )
   {
   ASSERT( m_startYear <= fromYear && fromYear <= m_endYear );
   ASSERT( m_startYear <= toYear   && toYear   <= m_endYear );

   if ( m_graphType == 0 )
      {
      ScatterWnd *pScatter = (ScatterWnd*) m_pGraphWnd;
      pScatter->ShowCurrentPosBar( true );
      //pScatter->DrawToCurrentPosOnly( true );
      pScatter->UpdateCurrentPos( (float) toYear );
      pScatter->UpdateWindow();
      }
   }


// static functions
bool ResultsGraphWnd::Replay( CWnd *pWnd,int flag )
   {
   ResultsGraphWnd *pGraphWnd = (ResultsGraphWnd*)pWnd;
   return pGraphWnd->_Replay( flag );
   }

bool ResultsGraphWnd::SetYear( CWnd *pWnd,int oldYear, int newYear )
   {
   ResultsGraphWnd *pGraphWnd = (ResultsGraphWnd*)pWnd;
   return pGraphWnd->_SetYear( oldYear, newYear );
   }


bool ResultsGraphWnd::_Replay( int flag )
   {
   // flag:
   //  -1   : reset to year 0
   // >= 0  : increment year unless at end
   if ( flag >= 0 )
      {
      COLDATA colData = m_pGraphWnd->GetXColData();
      DataObj *pDataObj = colData.pDataObj;
      int xCol    = colData.colNum;
      int lastRow = pDataObj->GetRowCount()-1;
      float lastYear = pDataObj->GetAsFloat( xCol, lastRow );
      if ( (float) m_year >=  lastYear )
         return false;

      m_year++;

      Shift( m_year-1, m_year );
      }
   else  // flag < 0 - reset to start
      {
      COLDATA colData = m_pGraphWnd->GetXColData();
      DataObj *pDataObj = colData.pDataObj;
      int xCol    = colData.colNum;
      float firstYear = pDataObj->GetAsFloat( xCol, 0 );
      float oldYear = (float) m_year;

      if ( oldYear != firstYear )
         {
         m_year = (int) firstYear;
         Shift( (int) oldYear, (int) firstYear );
         }
      }
   
   return true;
   }


void ResultsGraphWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
   {
   ActiveWnd::SetActiveWnd( m_pGraphWnd );
   CWnd::OnActivate(nState, pWndOther, bMinimized);
   }


int ResultsGraphWnd::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
   {
   ActiveWnd::SetActiveWnd( m_pGraphWnd );
   return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
   }


