#pragma once
#include "libs.h"

// USE CASE 1:  pAccum != NULL.
//              Timing begins when constructor is called.
//              Timing ends when destructor is called.
//              Elapsed time is added to *pAccum

// USE CASE 2:  pAccum == NULL.
//              Timing begins when Start() is called.
//              Timing ends when Stop() is called.
//              Elapsed time is stored in m_secsElapsed and returned by Stop()
//                Additional calls Start/Stop calls resets m_secsElapsed

class LIBSAPI HighResTime
   {
   public:
      HighResTime( double *pAccum = NULL );
      ~HighResTime();

      LARGE_INTEGER m_frequency;
      LARGE_INTEGER m_count;
      LARGE_INTEGER m_elapsed;
      double m_secsElapsed;  // seconds between most recent Start()/Stop() call

      // Starts the count.
      void Start(); 

      // Stops the count and returns elapsed secs.
      double Stop(); 

      double *m_pAccum;
   };
