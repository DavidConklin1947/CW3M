// FieldSorterDlg.cpp : implementation file
//


#include "winlibs.h"

#ifdef _WINDOWS

#include "Libs.h"
#include "FieldSorterDlg.h"


// FieldSorterDlg dialog

IMPLEMENT_DYNAMIC(FieldSorterDlg, CDialog)

FieldSorterDlg::FieldSorterDlg(MapLayer *pLayer, CWnd* pParent /*=NULL*/)
	: CDialog(FieldSorterDlg::IDD, pParent)
   , m_pMapLayer( pLayer )
{

}

FieldSorterDlg::~FieldSorterDlg()
{
}

void FieldSorterDlg::DoDataExchange(CDataExchange* pDX)
{
CDialog::DoDataExchange(pDX);
DDX_Control(pDX, IDC_LIB_FIELDS, m_fields);
 }


BEGIN_MESSAGE_MAP(FieldSorterDlg, CDialog)
END_MESSAGE_MAP()


// FieldSorterDlg message handlers
BOOL FieldSorterDlg::OnInitDialog() 
   {
	CDialog::OnInitDialog();

   if ( m_pMapLayer == NULL )
      return FALSE;

   for( int i=0; i < m_pMapLayer->GetColCount(); i++ )
      m_fields.AddString( m_pMapLayer->GetFieldLabel( i ));

   return TRUE;
   }

/* sort example
start  end
-----  -----
aaa    bbb
bbb    ccc
ccc    aaa
ddd    eee
eee    ddd

Algorithm - iterate though end list, swapping end value with label in same position in current list

Start             After swap1             After swap2            after swap3             after swap4
--------          -----------            -------------         --------------            ------------
aaa <<< swap b,a     bbb                     bbb                   bbb                         bbb
bbb                  aaa   <<< swap c,a      ccc                   ccc                         ccc
ccc                  ccc                     aaa <<< swap a,a      aaa                         aaa
ddd                  ddd                     ddd     (no-op)       ddd   <<< swap e,d          eee 
eee                  eee                     eee                   eee                         ddd  << last should always be a no-op

*/

void FieldSorterDlg::OnOK()
   {
   /*
   // test
   TCHAR labels[ 5 ] = { 'a', 'b', 'c', 'd', 'e' };
   TCHAR end   [ 5 ] = { 'e', 'a', 'd', 'b', 'c' };

   for( int i=0; i < 5; i++ )
      {
      // remember what is at the current position (i)
      TCHAR oldVal = labels[ i ];

      // find where this end value was in the starting list
      int endPos = 0;
      while ( labels[ endPos ] != end[ i ] )
         {
         ASSERT( endPos < 5 );
         endPos++;
         }

      // write the end value to the current position
      labels[ i ] = end[ i ];

      // write the starting value to the position where the end used to be
      labels[ endPos ] = oldVal;
      }

   for ( int i=0; i < 5; i++ )
      TRACE2( "%c  %c\n", labels[ i ], end[ i ] );
   // test
   */
   // iterate through final list
   for ( int i=0; i < m_pMapLayer->GetColCount(); i++ )
      {
      // get the end value
      CString label;
      m_fields.GetText( i, label );

      // find where end value was in the starting list
      int swapCol = m_pMapLayer->GetFieldCol( label );
      ASSERT( swapCol >= 0 );

      if ( i != swapCol )
         m_pMapLayer->SwapCols( i, swapCol );
      }

   //////// final order
   for ( int i=0; i < m_pMapLayer->GetColCount(); i++ )
      {
      CString value( "" );
      MAP_FIELD_INFO *pInfo = m_pMapLayer->FindFieldInfo( m_pMapLayer->GetFieldLabel( i ) );

      if ( pInfo )
         value = pInfo->label;

      TRACE2( "%s (%s)", m_pMapLayer->GetFieldLabel( i ), (LPCTSTR) value ); 
      }

   CDialog::OnOK();
   }


#endif  // _WINDOWS

