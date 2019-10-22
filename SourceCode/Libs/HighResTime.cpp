#include "libs.h"
#pragma hdrstop

#include "HighResTime.h"

#include <float.h>
#include <limits.h>

HighResTime::HighResTime( double *pAccum /*= NULL*/ )
:  m_pAccum( pAccum )
   {
   m_frequency.QuadPart = 0; 
   m_count.QuadPart = 0;
   m_secsElapsed = DBL_MAX;

   if (! QueryPerformanceFrequency( &m_frequency ) )
      m_frequency.QuadPart = 0;

   if ( m_pAccum ) 
      Start();
   }

HighResTime::~HighResTime()
   {
   if ( m_pAccum )
      *m_pAccum += Stop();
   }


void HighResTime::Start() 
   {
   m_secsElapsed = DBL_MAX;
   m_count.QuadPart = 0;
   if ( ! QueryPerformanceCounter( &m_count ) ) 
      {
      m_count.QuadPart = 0;
      }
   }

double HighResTime::Stop() 
   {
   ASSERT( sizeof(__int64) == sizeof(m_count.QuadPart) );
   LARGE_INTEGER countNext;
   countNext.QuadPart = 0;
   if ( ! QueryPerformanceCounter( &countNext ) ) 
      {
      m_secsElapsed = DBL_MAX;
      return m_secsElapsed;
      }

   if (countNext.QuadPart > m_count.QuadPart) 
      {
      m_elapsed.QuadPart = (countNext.QuadPart - m_count.QuadPart);
      }
   else 
      {
      m_elapsed.QuadPart = ( _I64_MAX - countNext.QuadPart + m_count.QuadPart);
      }

   m_secsElapsed = (double) m_elapsed.QuadPart / (double) m_frequency.QuadPart;

   return m_secsElapsed;
   }
