#pragma once

#include "Vdata.h"

class LIBSAPI PropertyList
   {
   public:
      PropertyList(void) { }
      ~PropertyList(void) { }


   protected:
      CMap< CString, LPCTSTR, VData, VData& > m_lookupTable;

   public:
      bool GetPropValue( LPCTSTR propName, VData   &value ) { return m_lookupTable.Lookup( propName, value ) ? true : false; }
      bool GetPropValue( LPCTSTR propName, float   &value );
      bool GetPropValue( LPCTSTR propName, int     &value );
      bool GetPropValue( LPCTSTR propName, bool    &value );
      bool GetPropValue( LPCTSTR propName, CString &value );

      void SetPropValue( LPCTSTR propName, VData   &value ) { m_lookupTable.SetAt( propName, value ); }
      void SetPropValue( LPCTSTR propName, float    value ) { m_lookupTable.SetAt( propName, VData( value ) ); }
      void SetPropValue( LPCTSTR propName, int      value ) { m_lookupTable.SetAt( propName, VData( value ) ); }
      void SetPropValue( LPCTSTR propName, bool     value ) { m_lookupTable.SetAt( propName, VData( value ) ); }
      void SetPropValue( LPCTSTR propName, CString &value ) { m_lookupTable.SetAt( propName, VData( value ) ); }
   };


inline
bool PropertyList::GetPropValue( LPCTSTR propName, float &value )  
   {
   VData v; 
   if ( m_lookupTable.Lookup( propName, v ) == 0 )
      return false;
   
   return v.GetAsFloat( value ); 
   }


inline
bool PropertyList::GetPropValue( LPCTSTR propName, int &value )  
   {
   VData v; 
   if ( m_lookupTable.Lookup( propName, v ) == 0 )
      return false;
   
   return v.GetAsInt( value ); 
   }


inline
bool PropertyList::GetPropValue( LPCTSTR propName, bool &value )  
   {
   VData v; 
   if ( m_lookupTable.Lookup( propName, v ) == 0 )
      return false;
   
   return v.GetAsBool( value ); 
   }


inline
bool PropertyList::GetPropValue( LPCTSTR propName, CString &value )  
   {
   VData v; 
   if ( m_lookupTable.Lookup( propName, v ) == 0 )
      return false;
   
   return v.GetAsString( value ); 
   }
