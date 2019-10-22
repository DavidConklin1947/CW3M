#pragma once
#include "EnvEngine\EnvModel.h"

 class EvalModelLearningStats;

class Sandbox : protected EnvModel
   {
   public:
      Sandbox( EnvModel *pModel, int years = 10, float percentCoverage = 1.0f, bool callInitRun = false );
      ~Sandbox(void);

   private:
      CArray< float, float > m_initialMetrics;  // state of the landscape at creation of sandbox
      
      float m_percentCoverage;
   
   public:
      void  SetYearsToRun( int years ){ m_yearsToRun = years; }
      void  SetPercentCoverage( float percentCoverage ){ m_percentCoverage = percentCoverage; }
      int   GetYearsToRun(){ return m_yearsToRun; }
      float GetPercentCoverage(){ return m_percentCoverage; }

      void CalculateGoalScores( Policy *pPolicy, bool *useMetagoalsArray ); // NULL says update all policy metagoals; else just where true
      void CalculateGoalScores( CArray<Policy *>  & schedPolicy, Policy *pPolicy, EvalModelLearningStats * emStats, bool *useMetagoalsArray ); // NULL says update all policy metagoals; else just where true

   private:
      // Used Base Methods
      UINT_PTR AddDelta( int cell, int col, int year, VData newValue, int type ){ return EnvModel::AddDelta( cell, col, year, newValue, type ); }
      UINT_PTR ApplyDeltaArray( MapLayer *pLayer ){ return EnvModel::ApplyDeltaArray( pLayer ); }
      UINT_PTR UnApplyDeltaArray( MapLayer *pLayer ){ return EnvModel::UnApplyDeltaArray(pLayer); }
      int   SelectPolicyOutcome( MultiOutcomeArray &m ){ return EnvModel::SelectPolicyOutcome( m ); }
      int   DoesPolicyApply( Policy *pPolicy, int cell ){ return EnvModel::DoesPolicyApply( pPolicy, cell ); }
           
      float GetLandscapeMetric( int i ){ return EnvModel::GetLandscapeMetric( i ); }

      // Overridden Base Methods
      bool Reset( void );

      // Sandbox Methods
      void InitRunModels();
      void PushModels();
      void PopModels();
   };
