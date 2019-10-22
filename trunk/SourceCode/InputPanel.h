#pragma once

#include <PtrArray.h>
#include "EnvEngine\EnvContext.h"
#include <CustomTabCtrl\CustomTabCtrl.h>


// InputPanel is the main container  it contains an...
//    InputTab control. This tab control manages...
//       InputWnd's, which are the parent window for Input Visualizers

class InputWnd : public CWnd
{
public:
   ENV_VISUALIZER *m_pViz;

public:
   DECLARE_MESSAGE_MAP()
   afx_msg void OnSize(UINT nType, int cx, int cy);
 
};


class InputPanel : public CWnd
{
public:
   InputPanel() : CWnd(), m_pActiveWnd( NULL ) { }
   ~InputPanel();

   InputWnd *AddInputWnd( ENV_VISUALIZER *pViz );
   InputWnd *GetActiveWnd();

protected:
   PtrArray< InputWnd > m_inputWndArray;
   InputWnd *m_pActiveWnd;

   CCustomTabCtrl m_tabCtrl;
   CFont m_font;

protected:
   void GetViewRect( RECT *rect );
   void ChangeTabSelection( int index );
 
	DECLARE_MESSAGE_MAP()

   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

public:
   afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg BOOL OnEraseBkgnd(CDC* pDC);
   afx_msg void OnAddWindow();

protected:
   //virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
public:
   afx_msg void OnPaint();
   };


