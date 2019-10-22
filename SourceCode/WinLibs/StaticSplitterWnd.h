#pragma once


// StaticSplitterWnd

class WINLIBSAPI StaticSplitterWnd : public CWnd
{
	DECLARE_DYNAMIC(StaticSplitterWnd)

public:
	StaticSplitterWnd();
	virtual ~StaticSplitterWnd();

   void LeftRight( CWnd *pLeft, CWnd *pRight );
   void MoveBar( int newLocation );
   int GetBarLoc() const { return m_bar; }
   int GetBarWidth() const { return m_barWidth; }
   void Draw();

protected:
    CWnd *m_pLeft;
    CWnd *m_pRight;

    int  m_bar;
    int  m_barWidth;
    bool m_inBarMove;
    int  m_mouseBarOffset;

protected:
	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
   virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnMouseMove(UINT nFlags, CPoint point);
   afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
   afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};


