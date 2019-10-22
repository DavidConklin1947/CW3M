#pragma once


// ResultsTreeCtrl

class ResultsTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNAMIC(ResultsTreeCtrl)

public:
	ResultsTreeCtrl();
	virtual ~ResultsTreeCtrl();

   HTREEITEM m_lastItem;

protected:
	DECLARE_MESSAGE_MAP()

   bool OpenItem( HTREEITEM hItem );

public:
   afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
   afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};




// ResultsTree

class ResultsTree : public CWnd
{
	DECLARE_DYNCREATE(ResultsTree)

public:
	ResultsTree();
	virtual ~ResultsTree();

   ResultsTreeCtrl m_tree;
   CMFCButton   m_play;
   CMFCButton   m_pause;
   CMFCButton   m_rewind;

   //CMFCButton   m_options;

   int  m_buttonHeight;
   int  m_editHeight;
   HBITMAP m_hRewind;
   HBITMAP m_hPause;
   HBITMAP m_hPlay;

   //HICON m_iRewind;
   
   CFont m_font;

   // filter related dta
   CEdit   m_filter;
   CString m_filterStr;
   CFont   m_filterFont;
   CFont   m_noFilterFont;
    
protected:
	DECLARE_MESSAGE_MAP()

public:
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnButton( UINT nID );
   afx_msg void OnChangeFilter();
   afx_msg void OnSetFocusFilter();
   afx_msg void OnTvnItemexpanded(NMHDR *pNMHDR, LRESULT *pResult );
   afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);  
   afx_msg void OnDestroy();
   };

