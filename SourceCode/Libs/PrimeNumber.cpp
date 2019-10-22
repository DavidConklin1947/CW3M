#include "libs.h"

#pragma hdrstop

#include "PrimeNumber.h"


//--------------------------------------------------------------------
// overload the output << operator of stream i/o to print prime number 
// use a reference to an ostream and this class 'prime' and return an 
// ostream reference
//--------------------------------------------------------------------
//ostream& operator << (ostream& strm_ref, prime &prm_in)
//{
//   return strm_ref << prm_in.get_prime();
//}  

//--------------------------------------------------------------------
// here where we create an new prime number upon invocation
// the overloaded ++ operator returns a new prime number
// To the user of the class it is as easy as adding one to integer.
//--------------------------------------------------------------------

PrimeNumber& PrimeNumber::operator++()
   {  
   ++m_primeCount;  // how many prime number so far?
   // test for 2
   if ( m_primeNumber == 2 )
      {
   	++m_primeNumber;
	   return (*this);
      }

   for( ; ; )
      {
      m_primeNumber += 2;
      int prime_found = 1;
      
      int max_times = ( m_primeNumber / 2 );
      
      for ( int i = 3; i < max_times; ++i )
	      {
	      if ( ( ( m_primeNumber / i ) * i ) == m_primeNumber )
	         {
		      prime_found = 0;
		      break;
	         }
	      }
      
      if ( prime_found ) 
	      break;
      }

   return ( *this );
   }
