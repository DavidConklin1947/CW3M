// WW2100AP.cpp 
//

#include "stdafx.h"
#include "APs.h"

#include <Maplayer.h>
#include <Map.h>
#include <math.h>
#include <UNITCONV.H>
#include <path.h>
#include <EnvEngine\EnvModel.h>
#include <EnvInterface.h>
#include <direct.h>
#include <random>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


bool WW2100AP::InitFish( EnvContext *pContext )
   {
   m_reachIndexArray.RemoveAll();

   MapLayer *pIDULayer = (MapLayer*) pContext->pMapLayer;

   MapLayer *pStreamLayer = (MapLayer*)pContext->pReachLayer; 

   for ( MapLayer::Iterator i = pStreamLayer->Begin( ); i != pStreamLayer->End(); i++ )
      {
      int inSlices = 0;
      pStreamLayer->GetData( i, m_colInSlices, inSlices );

      if ( inSlices )
         m_reachIndexArray.Add( i );
      }

   return true;
   }


bool WW2100AP::RunFish( EnvContext *pContext )
   {
   // basic idea - for each species, compute a probability of presence for each reach.
   //  assemble these into.  Limit this to those reaches that are in the slices coverage
   testMessage(pContext, _T("RunFish"));

   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;
   Map *pMap = pLayer->GetMapPtr();

   int sppGroupCount = (int) m_sppGroupArray.GetSize();
   int reachCount    = (int) m_reachIndexArray.GetSize();

   MapLayer *pStreamLayer = (MapLayer*)pContext->pReachLayer;

   for ( int j=0; j < sppGroupCount; j++ )
      m_sppGroupArray[ j ]->m_presentCountTotal = 0;
   
   for ( int i=0; i < reachCount; i++ )
      {
      int reachIndex = m_reachIndexArray[ i ];
      
      float tMax = 0;
      pStreamLayer->GetData( reachIndex, m_colTMax, tMax );

      // for each spp, calculate a presence/absence probability
      for ( int j=0; j < sppGroupCount; j++ )
         {
         SpeciesGroup *pGroup = m_sppGroupArray[ j ];
         pGroup->m_presentCountReach = 0;

         for ( int k=0; k < (int) pGroup->m_sppArray.GetSize(); k++ )
            {
            FishSpecie *pFish = pGroup->m_sppArray[ k ];

            float b0 = pFish->m_intercept;
            float b1 = pFish->m_tMax;

            float px = 1/( exp( -(b0+b1*tMax) ) + 1 );

            if ( px >= m_paThreshold )
               {
               pGroup->m_presentCountReach++;
               pGroup->m_presentCountTotal++;
               }
            }  // end of: for each fish

         // right group results to map. the group score is the number of spp present in the reach
         if ( pGroup->m_colScore )
            {
            int oldScore = 0;
            pStreamLayer->GetData( reachIndex, pGroup->m_colScore, oldScore );

            if ( oldScore != pGroup->m_presentCountReach )
               AddDelta( pContext, reachIndex, pGroup->m_colScore, pGroup->m_presentCountReach );
            }
         }  // end of: for each group
      } // end reach
   
  // for each spp, calculate a presence/absence probability as the average across
  // all reaches
  for ( int j=0; j < sppGroupCount; j++ )
     {
     SpeciesGroup *pGroup = m_sppGroupArray[ j ];
     pGroup->m_presentCountTotal /= reachCount;
     }

  return TRUE;
  }


