#pragma once
#include "libs.h"

// usage:  PtrClass< "Class"> - pass the type, arrays store ptointer to the type

template < class T >
class PtrArray : public CArray< T*, T* >
   {
   public:
      bool m_deleteOnDelete;
      PtrArray( bool deleteOnDelete=true) : m_deleteOnDelete( deleteOnDelete ) { }
      PtrArray( PtrArray<T> &ptrArray ) { *this = ptrArray; }

      ~PtrArray() { RemoveAll(); }

      INT_PTR DeepCopy( PtrArray<T> &source ) { *this = source; return CArray< T*, T* >::GetSize(); }

      PtrArray<T> & operator = ( PtrArray<T> &source )
         {
         for ( INT_PTR i=0; i < source.GetSize(); i++ ) 
            this->Add( new T( *source.GetAt( i ) ) ); 
         return *this;
         }
         
      void Clear() { RemoveAll(); }

      virtual void RemoveAll()
         {
         if ( m_deleteOnDelete ) 
            for ( INT_PTR i=0; i < CArray< T*, T* >::GetSize(); i++ )
               {
               T* p = CArray< T*, T* >::GetAt( i );
               delete p; 
               }

         CArray< T*, T* >::RemoveAll(); 
         }

      virtual void RemoveAt( INT_PTR i )
         {
         if ( m_deleteOnDelete )
            delete CArray< T*, T* >::GetAt( i );

         CArray<T*,T*>::RemoveAt( i );
         }

      void RemoveAtNoDelete( INT_PTR i )
         {
         CArray<T*,T*>::RemoveAt( i );
         }

      bool Remove( T* pItem )
         {
         for ( INT_PTR i=0; i < CArray< T*, T* >::GetSize(); i++ )
            {
            if ( CArray< T*, T* >::GetAt( i ) == pItem )
               {
               CArray< T*, T* >::RemoveAt( i );
               return true;
               }
            }
         
         return false;
         }
   };
