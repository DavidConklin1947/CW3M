#pragma once

//#include <afxwin.h>


/*============================================================================*/
// CDragCheckListBox

class CDragCheckListBox : public CCheckListBox
{
	DECLARE_DYNAMIC(CDragCheckListBox)

// Constructors
public:
	CDragCheckListBox();

// Attributes

	// find item index at given point
	int ItemFromPt(_In_ CPoint pt, _In_ BOOL bAutoScroll = TRUE) const;

// Operations

#pragma push_macro("DrawInsert")
#undef DrawInsert
	// draws insertion line
	virtual void DrawInsert(_In_ int nItem);
#pragma pop_macro("DrawInsert")

// Overridables

	// Override to respond to beginning of drag event.
	virtual BOOL BeginDrag(_In_ CPoint pt);

	// Overrdie to react to user cancelling drag.
	virtual void CancelDrag(_In_ CPoint pt);

	// Called as user drags. Return constant indicating cursor.
	virtual UINT Dragging(_In_ CPoint pt);

	// Called when user releases mouse button to end drag event.
	virtual void Dropped(_In_ int nSrcIndex, _In_ CPoint pt);

// Implementation
public:
	int m_nLast;
	void DrawSingle(_In_ int nIndex);
	virtual void PreSubclassWindow();
	virtual ~CDragCheckListBox();

protected:
	virtual BOOL OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*);
};
