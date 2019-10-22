#include "libs.h"
#pragma hdrstop

#include "DOIndex.h"
#include "DATAOBJ.H"
#include "PrimeNumber.h"


DOIndex::DOIndex( void )
   : m_pDataObj( NULL )
   , m_size( -1 )
   , m_isBuilt( false )
   { }


DOIndex::DOIndex( DataObj *pDataObj, UINT size /*=-1*/ )
   : m_pDataObj( pDataObj )
   , m_size( size )
   , m_isBuilt( false )
   {
   if ( m_size > 0 )
      m_map.InitHashTable( m_size );
   }

DOIndex& DOIndex::operator = ( const DOIndex &index )
   {
   m_pDataObj  = index.m_pDataObj;   // no storage, so no copy needed
   m_size      = index.m_size;
   m_isBuilt   = index.m_isBuilt;

   POSITION pos = index.m_map.GetStartPosition();
   m_map.RemoveAll();
   m_map.InitHashTable( index.m_map.GetHashTableSize() );
   UINT  key = 0;
   int   value = 0;
   
   while (pos != NULL)
      {
      index.m_map.GetNextAssoc(pos, key, value );
      m_map.SetAt( key, value );
      }

   return *this;
   }


int DOIndex::Build( void )
   {
   if ( m_pDataObj == NULL )
      return -1;
   
   // start  building index.  Basic idea is to build a hash table with keys that can be
   // defined externally and used to look up specific values based on the key.
   int rows = m_pDataObj->GetRowCount();

   if ( m_size < 0 )
      {
      int maxEntries = rows * 14 / 10;  // increase to 140%
      
      PrimeNumber primeNo;

      while ( primeNo.GetPrime() <= maxEntries )
         {
         ++primeNo;
         }

      m_size = primeNo.GetPrime();

      m_map.InitHashTable( m_size );
      }

   for ( int i=0; i < rows; i++ )
      {
      UINT key = GetKey( i );

      //float value = m_pDataObj->GetAsFloat( m_lookupCol, i );
      m_map.SetAt( key, i );
      }
   
   m_isBuilt = true;

   return rows;
   }


