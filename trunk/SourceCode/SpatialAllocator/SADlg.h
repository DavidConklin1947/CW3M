#pragma once
#include <winlibs.h>

#include "afxcmn.h"
#include "afxwin.h"

#include "resource.h"
#include "SpatialAllocator.h"
#include <EditTreeCtrl\EditTreeCtrl.h>

#include <Maplayer.h>


class SATreeCtrl : public CEditTreeCtrl
{
protected:
   //afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
   // If derived from CEditTreeCtrl:
   virtual bool CanDragItem( TVITEM & item );
   virtual bool CanDropItem( HTREEITEM hDrag, HTREEITEM hDrop, EDropHint hint);

   virtual void DragMoveItem(HTREEITEM hDrag, HTREEITEM hDrop, EDropHint hint, bool bCopy );
   virtual void DisplayContextMenu( CPoint & point);
};



// SADlg dialog

class SADlg : public CDialogEx
{
	DECLARE_DYNAMIC(SADlg)

public:
	SADlg(SpatialAllocator*, MapLayer*, CWnd* pParent = NULL);   // standard constructor
	virtual ~SADlg();

// Dialog Data
	enum { IDD = IDD_SPATIALALLOCATOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   void LoadFields( void );
   void LoadTreeCtrl( void );
   
	DECLARE_MESSAGE_MAP()

public:
   SpatialAllocator *m_pSpatialAllocator;
   MapLayer         *m_pMapLayer;

   AllocationSet *m_pCurrentSet;
   Allocation    *m_pCurrentAllocation;

   SATreeCtrl m_tree;
   CString m_allocationSetName;
   CComboBox m_allocationSetField;
   BOOL m_inUse;
   BOOL m_shuffle;
   BOOL m_useSequences;
   CComboBox m_sequenceField;
   CString m_allocationName;
   int m_allocationCode;
   float m_rate;
   CString m_timeSeriesValues;
   CString m_timeSeriesFile;
   BOOL m_allowExpansion;
   BOOL m_limitToQuery;
   CString m_expandQuery;
   BOOL m_limitSize;
   float m_expandMaxSize;
   afx_msg void OnBnClickedNewallocationset();
   afx_msg void OnBnClickedRemoveallocationset();
   afx_msg void OnBnClickedNewallocation();
   afx_msg void OnBnClickedRemoveallocation();
   virtual BOOL OnInitDialog();
   virtual void OnOK();
   };
