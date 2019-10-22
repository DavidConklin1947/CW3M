#pragma once

// ResultsGraphWnd
#include <ScatterWnd.h>


class ResultsGraphWnd : public CWnd
{
public:
	ResultsGraphWnd( int run, bool isDifference );
   virtual ~ResultsGraphWnd();

   static bool Replay( CWnd *pWnd, int flag );
   bool _Replay( int flag );

   static bool SetYear( CWnd *pWnd, int oldYear, int newYear );
   bool _SetYear( int oldYear, int newYear );

public:
   GraphWnd *m_pGraphWnd;  // pointers to the two contained window
   int       m_graphType;  // 0=scatter, 1=bar

protected:
   int m_year;
   int m_startYear;
   int m_endYear;
   int m_run;

protected:
   void Shift( int fromYear, int toYear );

protected:
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






