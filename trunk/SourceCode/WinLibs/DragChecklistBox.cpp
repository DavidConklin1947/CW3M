// DragChecklistBox.cpp : implementation file
//

#ifdef _WINDOWS

#include "winlibs.h"
#include "Libs.h"
#include "DragCheckListBox.h"


// DragChecklistBox

IMPLEMENT_DYNAMIC(CDragCheckListBox, CCheckListBox)

CDragCheckListBox::CDragCheckListBox()
   {
   m_nLast = -1;
   }

int CDragCheckListBox::ItemFromPt(CPoint pt, BOOL bAutoScroll) const
	{
   ASSERT(::IsWindow(m_hWnd)); 
   return ::AfxLBItemFromPt(m_hWnd, pt, bAutoScroll);
   }

// DragChecklistBox message handlers
//BEGIN_MESSAGE_MAP(CDragCheckListBox, CCheckListBox)
//END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CDragCheckListBox

CDragCheckListBox::~CDragCheckListBox()
{
	DestroyWindow();
}

void CDragCheckListBox::PreSubclassWindow()
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT((GetStyle() & (LBS_MULTIPLESEL|LBS_SORT)) == 0);
	MakeDragList(m_hWnd);
}

#pragma push_macro("DrawInsert")
#undef DrawInsert

BOOL CDragCheckListBox::BeginDrag(CPoint pt)
{
	m_nLast = -1;
	DrawInsert(ItemFromPt(pt));
	return TRUE;
}

void CDragCheckListBox::CancelDrag(CPoint)
{
	DrawInsert(-1);
}

UINT CDragCheckListBox::Dragging(CPoint pt)
{
	int nIndex = ItemFromPt(pt, FALSE); // don't allow scrolling just yet
	DrawInsert(nIndex);
	ItemFromPt(pt);
	return (nIndex == LB_ERR) ? DL_STOPCURSOR : DL_MOVECURSOR;
}

void CDragCheckListBox::Dropped(int nSrcIndex, CPoint pt)
{
	ASSERT(!(GetStyle() & (LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE)) ||
		(GetStyle() & LBS_HASSTRINGS));

	DrawInsert(-1);
	int nDestIndex = ItemFromPt(pt);

	if (nSrcIndex == -1 || nDestIndex == -1)
		return;
	if (nDestIndex == nSrcIndex || nDestIndex == nSrcIndex+1)
		return; //didn't move
	CString str;
	UINT_PTR dwData;
	GetText(nSrcIndex, str);
	dwData = GetItemData(nSrcIndex);
	DeleteString(nSrcIndex);
	if (nSrcIndex < nDestIndex)
		nDestIndex--;
	nDestIndex = InsertString(nDestIndex, str);
	SetItemData(nDestIndex, dwData);
	SetCurSel(nDestIndex);
}

void CDragCheckListBox::DrawInsert(int nIndex)
{
	if (m_nLast != nIndex)
	{
		DrawSingle(m_nLast);
		DrawSingle(nIndex);
		m_nLast = nIndex;
	}
}

#pragma pop_macro("DrawInsert")

void CDragCheckListBox::DrawSingle(int nIndex)
{
	if (nIndex == -1)
		return;
	CBrush* pBrush = CDC::GetHalftoneBrush();
	CRect rect;
	GetClientRect(&rect);
	CRgn rgn;
	rgn.CreateRectRgnIndirect(&rect);

	CDC* pDC = GetDC();
	// prevent drawing outside of listbox
	// this can happen at the top of the listbox since the listbox's DC is the
	// parent's DC
	pDC->SelectClipRgn(&rgn);

	GetItemRect(nIndex, &rect);
	rect.bottom = rect.top+2;
	rect.top -= 2;
	CBrush* pBrushOld = pDC->SelectObject(pBrush);
	//draw main line
	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATINVERT);

	pDC->SelectObject(pBrushOld);
	ReleaseDC(pDC);
}

BOOL CDragCheckListBox::OnChildNotify(UINT nMessage, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (nMessage != m_nMsgDragList)
		return CListBox::OnChildNotify(nMessage, wParam, lParam, pResult);

	ASSERT(pResult != NULL);
	LPDRAGLISTINFO pInfo = (LPDRAGLISTINFO)lParam;
	ASSERT(pInfo != NULL);
	switch (pInfo->uNotification)
	{
	case DL_BEGINDRAG:
		*pResult = BeginDrag(pInfo->ptCursor);
		break;
	case DL_CANCELDRAG:
		CancelDrag(pInfo->ptCursor);
		break;
	case DL_DRAGGING:
		*pResult = Dragging(pInfo->ptCursor);
		break;
	case DL_DROPPED:
		Dropped(GetCurSel(), pInfo->ptCursor);
		break;
	}
	return TRUE;
}


#endif  // _WINDOWS
