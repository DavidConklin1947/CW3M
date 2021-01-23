#pragma once
#include "libs.h"

#include "randgen/Randunif.hpp"
#include "PtrArray.h"
#include <../EnvEngine/EnvContext.h>

#ifdef Yield
#undef Yield
#endif

#ifndef NO_MFC
inline
void YieldMsg()
   {
   MSG  msg;
   while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
      {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      }
   }
#endif
// e.g MAKELONG type macro
// 
// Usage:
// 
// WORD wLo = 0xAA55U, wHi = 0x55AAU;
// ASSERT (MAKE<DWORD> (wLo, wHi) == 0x55AAAA55);

template<typename DOUBLE, typename SINGLE> 
DOUBLE MAKE (const SINGLE lo, const SINGLE hi)
   {
   static_assert (sizeof (DOUBLE) == (2 * sizeof (SINGLE)), "Unmatched");
   return ((static_cast<DOUBLE> (hi) << (CHAR_BIT * sizeof (SINGLE))) | lo);
   }


template <class T>
void ShuffleArray(T *a, INT_PTR size, RandUniform *pRand )
   {
   //--- Shuffle elements by randomly exchanging each with one other.
   for ( int i=0; i < size-1; i++ )
      {
      int randVal = (int) pRand->RandValue( 0, size-i-0.0001f );

      int r = i + randVal; // Random remaining position.
      T temp = a[ i ];
      a[i] = a[r];
      a[r] = temp;
      }
   }

inline
int MonteCarloDraw( RandUniform *pRand, float probs[], int count, bool shuffleProbs=false )
   {
   if ( shuffleProbs )
      ShuffleArray( probs, count, pRand );

   float randVal = (float) pRand->RandValue();

   float sumSoFar = 0;
   for ( int i=0; i < count; i++ )
      {
      sumSoFar += probs[ i ];

      if ( randVal <= sumSoFar )
         return i;
      }

   return -1; 
   }


inline
int MonteCarloDraw( RandUniform *pRand, CArray< float, float > &probs, bool shuffleProbs=false )
   {
   if ( shuffleProbs )
      ShuffleArray( probs.GetData(), probs.GetCount(), pRand );

   float randVal = (float) pRand->RandValue();

   float sumSoFar = 0;
   for ( int i=0; i < (int) probs.GetCount(); i++ )
      {
      sumSoFar += probs[ i ];

      if ( randVal <= sumSoFar )
         return i;
      }

   return -1; 
   }


inline
bool IsPrime(int number)
   {	// Given:   num an integer > 1
	// Returns: true if num is prime
	// 			false otherwise.	
   for (int i=2; i < number; i++)
	   {
		if (number % i == 0)
			return false;
	   }
	
	return true;	
   }



#ifdef __cplusplus
extern "C" {
#endif

void LIBSAPI GetTileParams( int count, int &rows, CArray<int,int> &colArray );
int  LIBSAPI CleanFileName(LPCTSTR filename);
int  LIBSAPI Tokenize( const TCHAR* str, const TCHAR* delimiters, CStringArray &tokens);
void LIBSAPI ApplySubstituteStrings(CString& str, PtrArray<SubstituteString> substitutes);

#ifdef __cplusplus
}
#endif
