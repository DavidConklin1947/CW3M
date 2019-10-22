#pragma once
#include "libs.h"


class LIBSAPI MovingAvg
{
public:   
   MovingAvg( int length );
   ~MovingAvg( void );

   void AddValue( float value );
   
   float GetValue( void );
      
protected:
   int m_length;
   float *m_values;

};

