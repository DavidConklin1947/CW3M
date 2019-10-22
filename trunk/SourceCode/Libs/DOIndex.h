#pragma once
#include "libs.h"

class DataObj;

class LIBSAPI DOIndex
   {
   public:
      DOIndex( void );

      DOIndex( DataObj*, UINT size );
      //~DOIndex(void);

      DOIndex &operator = ( const DOIndex & );

      int  Build( void );
      bool IsBuilt( void ) { return m_isBuilt; }

      //bool LookupRow( int row, float &value ) { UINT key = GetKey( row ); return Lookup( key, value ); }

      BOOL Lookup( UINT key, int &value )
         {
         ASSERT( IsBuilt() );
         return m_map.Lookup( key, value );
         }
      
      virtual UINT GetKey( int row ) = 0;

   public:
      DataObj  *m_pDataObj;
      int       m_size;
      bool      m_isBuilt;

      CMap< UINT, UINT, int, int > m_map;
   };




