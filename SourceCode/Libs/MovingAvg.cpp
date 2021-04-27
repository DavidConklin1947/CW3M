#include "libs.h"
#pragma hdrstop

#include "MovingAvg.h"
#include "Report.h"

MovingAvg::MovingAvg( int length ) 
: m_length( length )
, m_values( NULL )
   {
   ASSERT( length > 0 );

   m_values = new float[ length];

   for ( int i=0; i < length; i++ )
      m_values[ i ] = 0.0f;
   }
   

MovingAvg::~MovingAvg( void )
   {
   if ( m_values != NULL ) 
      delete [] m_values;
   }

void MovingAvg::AddValue( float value )  // push on back, pop front
{
   if (m_length < 1)
   {
      CString msg;
      msg.Format("MovingAvg::AddValue() m_length = %d", m_length);
      Report::ErrorMsg(msg);
   }

   for ( int i=0; i < m_length-1; i++ ) 
      m_values[ i ] = m_values[ i+1 ]; 
      
   m_values[ m_length-1 ] = value;
} // end of AddValue()

float MovingAvg::GetValue( void )
   {
   ASSERT(m_length >= 1);

   float value = 0.0f; 
   for (int i=0; i < m_length; i++ ) 
      value += m_values[ i ]; 
   
   return value / m_length; 
   }
 