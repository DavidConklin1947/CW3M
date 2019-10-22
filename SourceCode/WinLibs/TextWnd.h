#pragma once

///////////////////////////////////////////////////////////////////////
// TextWnd

class WINLIBSAPI TextWnd : public CWnd
{

public:
	TextWnd();
	virtual ~TextWnd();

protected:
	CFont m_font;
	int   m_cxChar;
	int   m_cyChar;
   int   m_indent;
   int   m_spacesPerIndent;

   bool m_current;
   int  m_width;
   int  m_height;
   int  m_xOffset;
   int  m_yOffset;

   bool m_showNewText;   //If true, automaticly scrolls to show appended text

	CStringArray m_strArray;
   
public:
	void AppendText( LPCSTR text );
   void SetText( LPCSTR text ){ ClearAll(); AppendText(text); }
   void ClearAll( void );

   void UpIndent();
   void DownIndent();
   void ResetIndent(){ m_indent = 0; }

   int  GetCharWidth() { return m_cxChar; } // returns 0 before a paint
   int  GetCharHeight(){ return m_cyChar; } // returns 0 before a paint

protected:
   void UpdateDimensions();

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
	afx_msg void OnPaint();
   afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
   afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
   afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
   afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
