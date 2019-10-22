// SADlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpatialAllocator.h"
#include "SADlg.h"
#include "afxdialogex.h"



bool SATreeCtrl::CanDragItem(TVITEM & item) 
   {
   return true;
   }


bool SATreeCtrl::CanDropItem(HTREEITEM hDrag, HTREEITEM hDrop, EDropHint hint)
   {
   /*
   // basic rules 
   //  1) You can drop a non-submenu MAP_FIELD_INFO anywhere
   //  2) You can only drop a submenu item at the root level

   MAP_FIELD_INFO *pDragInfo = (MAP_FIELD_INFO*) this->GetItemData( hDrag );
   MAP_FIELD_INFO *pDropInfo = (MAP_FIELD_INFO*) this->GetItemData( hDrop );

   CString msg;
   CString _hint;

   switch( hint )
      {
      case DROP_BELOW:   _hint="Below";   break;
      case DROP_ABOVE:   _hint="Above";   break;
      case DROP_CHILD:   _hint="Child";   break;
      case DROP_NODROP:  _hint="No Drop";   break;
      }
   msg.Format( "drop target: %s, hint=%s", pDropInfo->label, _hint );
   pFieldInfoDlg->SetDlgItemText( IDC_TYPE, msg );

   switch( hint )
      {
      case DROP_CHILD:   // only allow drop child events on submenu
         return pDropInfo->IsSubmenu() ? true : false;

      case DROP_BELOW:   // these are always ok
      case DROP_ABOVE:
         return true;
         
      case DROP_NODROP:  _hint="No Drop";   break;
         return false;
      }
    */   
   return false;
   }


void SATreeCtrl::DragMoveItem(HTREEITEM hDrag, HTREEITEM hDrop, EDropHint hint, bool bCopy )
   {
   /*
   // let the parent control handle the move
   CEditTreeCtrl::DragMoveItem( hDrag, hDrop, hint, false );     // never copy, always move
   
   HTREEITEM hNew = this->GetSelectedItem();

   // fix up the pointers for the moved items.
   MAP_FIELD_INFO *pNewInfo = (MAP_FIELD_INFO*) this->GetItemData( hNew ); 

   // the drag item need to have it's parent pointer fixed up
   HTREEITEM hParent = this->GetParentItem( hNew );
   if ( hParent == NULL )
      pNewInfo->SetParent( NULL );
   else
      {
      MAP_FIELD_INFO *pParent = (MAP_FIELD_INFO*) this->GetItemData( hParent );
      pNewInfo->SetParent( pParent );
      }
      */
   }


void SATreeCtrl::DisplayContextMenu(CPoint & point)
   {
   /*
   CPoint pt(point);
   ScreenToClient(&pt);
   UINT flags;
   HTREEITEM hItem = HitTest(pt, &flags);
   bool bOnItem = (flags & TVHT_ONITEM) != 0;

   CMenu menu;
   VERIFY(menu.CreatePopupMenu());

   VERIFY(menu.AppendMenu( MF_STRING, ID_ADD_SUBMENU, _T("New Submenu\tShift+INS")));

   // maybe the menu is empty...
   if(menu.GetMenuItemCount() > 0)
      menu.TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);
   */
   }




// SADlg dialog

IMPLEMENT_DYNAMIC(SADlg, CDialogEx)

SADlg::SADlg(SpatialAllocator *pSA, MapLayer *pLayer, CWnd* pParent /*=NULL*/)
	: CDialogEx(SADlg::IDD, pParent)
   , m_pSpatialAllocator( pSA )
   , m_pMapLayer( pLayer )
   , m_pCurrentSet( NULL )
   , m_pCurrentAllocation( NULL )
   , m_allocationSetName(_T(""))
   , m_inUse(FALSE)
   , m_shuffle(FALSE)
   , m_useSequences(FALSE)
   , m_allocationName(_T(""))
   , m_allocationCode(0)
   , m_rate(0)
   , m_timeSeriesValues(_T(""))
   , m_timeSeriesFile(_T(""))
   , m_allowExpansion(FALSE)
   , m_limitToQuery(FALSE)
   , m_expandQuery(_T(""))
   , m_limitSize(FALSE)
   , m_expandMaxSize(0)
   {

}

SADlg::~SADlg()
{
}

void SADlg::DoDataExchange(CDataExchange* pDX)
{
CDialogEx::DoDataExchange(pDX);
DDX_Control(pDX, IDC_ALLOCATIONTREE, m_tree);
DDX_Text(pDX, IDC_ALLOCATIONSETNAME, m_allocationSetName);
DDX_Control(pDX, IDC_ALLOCATIONSETFIELD, m_allocationSetField);
DDX_Check(pDX, IDC_INUSE, m_inUse);
DDX_Check(pDX, IDC_SHUFFLE, m_shuffle);
DDX_Check(pDX, IDC_USESEQUENCES, m_useSequences);
DDX_Control(pDX, IDC_SEQUENCEFIELD, m_sequenceField);
DDX_Text(pDX, IDC_ALLOCATIONNAME, m_allocationName);
DDX_Text(pDX, IDC_ALLOCATIONID, m_allocationCode);
DDX_Text(pDX, IDC_GROWTHRATE, m_rate);
DDX_Text(pDX, IDC_TIMESERIES, m_timeSeriesValues);
DDX_Text(pDX, IDC_FILE, m_timeSeriesFile);
DDX_Check(pDX, IDC_ALLOWEXPANSION, m_allowExpansion);
DDX_Check(pDX, IDC_LIMITTOQUERY, m_limitToQuery);
DDX_Text(pDX, IDC_QUERY, m_expandQuery);
DDX_Check(pDX, IDC_LIMITSIZE, m_limitSize);
DDX_Text(pDX, IDC_MAXAREA, m_expandMaxSize);
   }


BEGIN_MESSAGE_MAP(SADlg, CDialogEx)
   ON_BN_CLICKED(IDC_NEWALLOCATIONSET, &SADlg::OnBnClickedNewallocationset)
   ON_BN_CLICKED(IDC_REMOVEALLOCATIONSET, &SADlg::OnBnClickedRemoveallocationset)
   ON_BN_CLICKED(IDC_NEWALLOCATION, &SADlg::OnBnClickedNewallocation)
   ON_BN_CLICKED(IDC_REMOVEALLOCATION, &SADlg::OnBnClickedRemoveallocation)
END_MESSAGE_MAP()



BOOL SADlg::OnInitDialog()
   {
   CDialogEx::OnInitDialog();

   // TODO:  Add extra initialization here



   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
   }



void SADlg::LoadFields( void )
   {
   m_allocationSetField.ResetContent();
   m_sequenceField.ResetContent();

   // note: only use fields with field info defined
   for ( int i=0; i < m_pMapLayer->GetColCount(); i++ )
      {
      if ( ::IsInteger( m_pMapLayer->GetFieldType( i ) ) )
         {
         LPCTSTR field = m_pMapLayer->GetFieldLabel( i );
         MAP_FIELD_INFO *pInfo = m_pMapLayer->FindFieldInfo( field );

         if ( pInfo != NULL ) // ino found?
            {
            int index = m_allocationSetField.AddString( field );
            m_allocationSetField.SetItemData( index, i );

            index = m_sequenceField.AddString( field );
            m_sequenceField.SetItemData( index, i );      // sotre the column offset with the item
            }
         }
      }
   
   m_allocationSetField.SetCurSel( 0 );
   m_sequenceField.SetCurSel( 0 );
   }


void SADlg::LoadTreeCtrl( void )
   {
   m_tree.DeleteAllItems();

   for ( int i=0; i < (int) m_pSpatialAllocator->m_allocationSetArray.GetSize(); i++ )
      {
      AllocationSet *pSet = m_pSpatialAllocator->m_allocationSetArray.GetAt( i );
      ASSERT( pSet != NULL );

      HTREEITEM hRoot = m_tree.GetRootItem();   // root
         
      // this gets inserted at the root level
      HTREEITEM hItemSet = m_tree.InsertItem( pSet->m_name );
      m_tree.SetItemData( hItemSet, (DWORD_PTR) pSet );


      for ( int j=0; j < pSet->GetAllocationCount(); j++ )
         {
         Allocation *pAlloc = pSet->GetAllocation( j );

         HTREEITEM hItemAlloc = m_tree.InsertItem( pAlloc->m_name, hItemSet );
         m_tree.SetItemData( hItemAlloc, (DWORD_PTR) pAlloc );
         }  // end of:  else ( attached to a submenu )
      }
   }







// SADlg message handlers


void SADlg::OnBnClickedNewallocationset()
   {
   // TODO: Add your control notification handler code here
   }


void SADlg::OnBnClickedRemoveallocationset()
   {
   // TODO: Add your control notification handler code here
   }


void SADlg::OnBnClickedNewallocation()
   {
   // TODO: Add your control notification handler code here
   }


void SADlg::OnBnClickedRemoveallocation()
   {
   // TODO: Add your control notification handler code here
   }


void SADlg::OnOK()
   {
   // TODO: Add your specialized code here and/or call the base class

   CDialogEx::OnOK();
   }
