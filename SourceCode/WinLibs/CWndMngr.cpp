#include "winlibs.h"
#pragma hdrstop


#include <cwndmngr.h>



//------------------- CWndManager methods --------------------//

//-- constructor --//
CWndManager::CWndManager( void ) 
  : CArray< CWnd*, CWnd*>(),
    pActive( NULL )
  {
  deleteOnDelete = TRUE;
  }


CWndManager::~CWndManager()
   {
   for ( int i=0; i < GetSize(); i++ )
      Remove( GetAt( i ) );

   RemoveAll();
   }


void CWndManager::Add( CWnd *pWnd ) 
   {
   CArray<CWnd*, CWnd*>::Add( pWnd );
   pActive = pWnd;
   }
   
   
void CWndManager::Remove(CWnd *pWnd )
   {
   int i;
   bool found = false;

   for ( i=0; i < GetSize(); i++ )
      {
      if ( GetAt( i ) == pWnd )
         {
         found = true;
         break;
         }
      }

   if ( found )
      {
      RemoveAt( i );

      if ( deleteOnDelete )
         delete pWnd;
      }

   if ( pWnd == pActive )
      {
      if ( GetSize() > 0 )
         pActive = GetAt( 0 );
      else
         pActive = NULL;
      }
   }
         

