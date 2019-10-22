// ==========================================================================
// LogbookGUI.h
//
// Author : Marquet Mike
//
// Company : /
//
// Date of creation  : 14/04/2003
// Last modification : 14/04/2003
// ==========================================================================

#if !defined(AFX_LOGBOOKGUI_H__E250301A_BB42_42CE_B91F_D36C05BBA3D2__INCLUDED_)
#define AFX_LOGBOOKGUI_H__E250301A_BB42_42CE_B91F_D36C05BBA3D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ==========================================================================
// Les Includes
// ==========================================================================

#include "Logbook.h"

/////////////////////////////////////////////////////////////////////////////
// CLogbookGUI window

class CLogbookGUIItem;

class CLogbookGUI : public CListCtrl
 {
  protected :
              CLogbook   *m_pLogbook;
              CFont       m_cLogbookFont[2];
              CImageList  m_cImageList;
              BOOL        m_bAutoScroll;
              int         m_nCurrentTimeColumnWidth;
              int         m_nCurrentTypeColumnWidth;

              void DrawNormalLine(CDC *pDC, LPDRAWITEMSTRUCT lpDIS, LVITEM *pstLVI, CLogbookGUIItem *pLogbookGUIItem);

              void DrawSeparatorLine(CDC *pDC, LPDRAWITEMSTRUCT lpDIS, LVITEM *pstLVI, CLogbookGUIItem *pLogbookGUIItem);

              int Init();
              
              void RecalcColumns(int nPixelAdd = 0);

              // INLINE
              inline LOGBOOK_INIT_GUI_INFOS *GetGUIInfos() { return m_pLogbook ? &m_pLogbook->m_stGuiInfos : NULL; }

  public :
           CLogbookGUI(CLogbook *pLogbook);
           virtual ~CLogbookGUI();

           int AddLine(const LOGBOOKITEM_INFO &stInfo, const SYSTEMTIME &stST, BOOL bDisplayTime, BOOL bDisplayType);

           BOOL Create(CWnd *pParentWnd, UINT nID = 9876);

  public :
	//{{AFX_VIRTUAL(CLogbookGUI)
  public:
  virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
  protected:
  virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
  virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

  protected :
	//{{AFX_MSG(CLogbookGUI)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

  DECLARE_MESSAGE_MAP()
 };

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGBOOKGUI_H__E250301A_BB42_42CE_B91F_D36C05BBA3D2__INCLUDED_)
