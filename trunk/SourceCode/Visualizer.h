#pragma once

#include "EnvEngine\EnvContext.h"
#include <PtrArray.h>


class VizManager;

// VisualizerWnd 
//
// This provides a container window for visualizers.  The visualizer window
//  consists of several windows:
//  1) the container window for the ENV_VISUALIZER,
//  2) a slider bar and edit control for representing/changing the current display period.
//

class VisualizerWnd : public CWnd
{
friend class VizManager;

DECLARE_DYNAMIC(VisualizerWnd)

protected:
	VisualizerWnd( ENV_VISUALIZER *pViz, int run );    // use VizManager::CreateWnd()

public:
   virtual ~VisualizerWnd();

public:
   ENV_VISUALIZER *m_pVizInfo;

   EnvContext m_envContext;

public:
   BOOL InitWindow  ( EnvContext *pContext=NULL );    // NULL means use internal
   BOOL UpdateWindow( EnvContext *pContext=NULL );

   int GetRun( void ) { return m_envContext.run; }

	DECLARE_MESSAGE_MAP()

public:
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnSize(UINT nType, int cx, int cy);

protected:
   virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

public:
   afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
   afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
};


class VizManager
{
public:
   VizManager( void ) : m_nextID( 80000 ) { }

   int AddVisualizer( VisualizerWnd *pWnd ) { return (int) m_visualizers.Add( pWnd ); }
   int GetVisualizerCount( void ) { return (int) m_visualizers.GetSize(); }
   VisualizerWnd *GetVisualizerWnd( int i ) { return m_visualizers[ i ]; } 

   VisualizerWnd *CreateWnd( CWnd *pParent, ENV_VISUALIZER *pInfo, int run, BOOL &success );
   
protected:
   PtrArray< VisualizerWnd > m_visualizers;
   
   int m_nextID;
};


inline
VisualizerWnd *VizManager::CreateWnd( CWnd *pParent, ENV_VISUALIZER *pInfo, int run, BOOL &success )
   {
   VisualizerWnd *pWnd = new VisualizerWnd( pInfo, run );

   RECT rect;
   pParent->GetClientRect( &rect );
   
   success = pWnd->Create( NULL, pInfo->name, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                           rect, pParent, m_nextID++ );

   m_visualizers.Add( pWnd );
  
   pWnd->m_envContext.pWnd = pWnd;
   pWnd->m_envContext.run  = run;
   pWnd->m_envContext.yearOfRun = 0;

   return pWnd;
   }