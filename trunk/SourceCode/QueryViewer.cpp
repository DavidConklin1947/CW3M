// QueryViewer.cpp : implementation file
//

#include "stdafx.h"
#include "QueryViewer.h"

#include "maplayer.h"
#include "queryengine.h"


// QueryViewer dialog

IMPLEMENT_DYNAMIC(QueryViewer, CDialog)

QueryViewer::QueryViewer( LPCTSTR query, MapLayer *pLayer, CWnd* pParent /*=NULL*/)
	: CDialog(QueryViewer::IDD, pParent)
   , m_pLayer( pLayer )
   , m_query( query )
{ }


QueryViewer::~QueryViewer()
{ }


void QueryViewer::DoDataExchange(CDataExchange* pDX)
{
CDialog::DoDataExchange(pDX);
DDX_Text(pDX, IDC_QUERY, m_query);
DDX_Control(pDX, IDC_TREE, m_tree);
}


BEGIN_MESSAGE_MAP(QueryViewer, CDialog)
END_MESSAGE_MAP()


// QueryViewer message handlers

void QueryViewer::OnOK()
   {
   UpdateData( 1 );

   m_tree.DeleteAllItems();

   QueryEngine qEngine( m_pLayer );

   qEngine.m_debug = true;

   Query *pQuery = qEngine.ParseQuery( m_query, 0, "Query Viewer Setup" );

   if ( pQuery == NULL )
      {
      MessageBox( "Error Parsing query", "Error" );
      return;
      }

   HTREEITEM hItem = m_tree.InsertItem( _T( "Query" ) );

   QNode *pNode = pQuery->m_pRoot;

   AddNode( hItem, pNode );

   m_tree.Expand( hItem, TVE_EXPAND );
   }


void QueryViewer::AddNode( HTREEITEM hParent, QNode *pNode )
   {
   CString label;
   pNode->GetLabel( label, 0 );
   HTREEITEM hItem = m_tree.InsertItem( label, hParent );

   if ( pNode->m_pLeft != NULL )
      AddNode( hItem, pNode->m_pLeft );

   if ( pNode->m_pRight != NULL )
      AddNode( hItem, pNode->m_pRight );

   m_tree.Expand( hItem, TVE_EXPAND );
   }
