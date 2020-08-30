#include "stdafx.h"

#pragma hdrstop

#include "GlobalMethods.h"
#include "Flow.h"

#include <UNITCONV.H>
#include <omp.h>

extern FlowProcess *gpFlow;
extern FlowModel *gpModel;



bool ReachRouting::Step( FlowContext *pFlowContext )
{    
   bool rtnflag = false;

   // handle NONE, EXTERNAL cases if defined
   rtnflag = GlobalMethod::Step(pFlowContext);    // true means GlobalMethod handled it.
   if (!rtnflag) switch( m_method )
   {
      case GM_EULER:
      case GM_RK4:
      case GM_RKF:
         rtnflag = gpModel->m_reachBlock.Integrate( gpModel->m_currentTime, gpModel->m_currentTime+this->m_reachTimeStep, GetReachDerivatives, pFlowContext );  // NOTE: GetReachDerivatives does not work
         break;
      case GM_KINEMATIC:
         rtnflag = SolveReachKinematicWave( pFlowContext );
         break;
      case GM_NONE:
         rtnflag =  true;
         break;
      default:
         ASSERT( 0 );
         rtnflag = false;
   } // end of switch(m_method)

   // Ensure that every discharge is > 0 and every volume is > 0.
   for (int reach_ndx = 0; reach_ndx < gpModel->m_reachArray.GetSize(); reach_ndx++)
   {
      Reach * pReach = gpModel->m_reachArray[reach_ndx];
      int subnode_count = pReach->GetSubnodeCount();
      for (int subnode_ndx = 0; subnode_ndx < subnode_count; subnode_ndx++)
      {
         ReachSubnode *pNode = pReach->GetReachSubnode(subnode_ndx);
         if (isnan(pNode->m_discharge)) { pNode->m_nanOccurred = true; pNode->m_discharge = 0; }
         if (pNode->m_discharge <= 0)
         {
            float node_current_added_discharge_cms = NOMINAL_LOW_FLOW_CMS - pNode->m_discharge;
            pNode->m_addedDischarge_cms += node_current_added_discharge_cms;
            pNode->m_discharge = NOMINAL_LOW_FLOW_CMS;
         }
         if (isnan(pNode->m_volume)) { pNode->m_nanOccurred = true; pNode->m_volume = 0; }
         if (pNode->m_volume <= 0)
         {
            float depth = GetDepthFromQ(pReach, pNode->m_discharge, pReach->m_wdRatio);
            float width = pReach->m_wdRatio * depth;
            float volume = (width * depth * pReach->m_length / subnode_count);

            float node_current_added_volume_m3 = (float)(volume - pNode->m_volume);
            pNode->m_addedVolume_m3 += node_current_added_volume_m3;
            pNode->m_volume = volume;
         }
      } // end of loop thru subnodes
   } // end of logic to ensure that every discharge is > 0 and every volume is > 0

   return(rtnflag);
} // end of ReachRouting::Step()


bool ReachRouting::SolveReachKinematicWave( FlowContext *pFlowContext )
   {
   pFlowContext->Reset();

   int reachCount = gpModel->GetReachCount();

   clock_t start = clock();

   // basic idea - for each Reach, estimate it's outflow bases on lateral flows and any fluxes
   for ( int i=0; i < reachCount; i++ )
      {
      Reach *pReach = gpModel->GetReach( i );     // Note: these are guaranteed to be non-phantom

      for (int l = 0; l < pReach->GetSubnodeCount(); l++)
         {
         pFlowContext->pReach = pReach;
         ReachSubnode *pNode = pReach->GetReachSubnode(l);
         float lateralInflow = GetLateralInflow(pReach);
         WaterParcel newLateralInflowWP = GetLateralInflowWP(pReach); 
         float externalFluxes = GetReachFluxes(pFlowContext, pReach);
         float new_lateralInflow = lateralInflow + externalFluxes;

         if (new_lateralInflow != new_lateralInflow)
            {
            CString msg;
            msg.Format("*** SolveReachKinematicWave(): i = %d, l = %d, lateralInflow = %f, externalFluxes = %f", i, l, lateralInflow, externalFluxes);
            Report::LogMsg(msg);
            msg.Format("Setting new_lateralInflow to %f cms and continuing.", NOMINAL_LOW_FLOW_CMS); Report::LogMsg(msg);
            new_lateralInflow = NOMINAL_LOW_FLOW_CMS;
            }

         float outflow = EstimateReachOutflow(pReach, l, pFlowContext->timeStep, new_lateralInflow); // m3/day
         WaterParcel OutflowWP(0,0);
         OutflowWP = EstimateReachOutflowWP(pReach, l, pFlowContext->timeStep, newLateralInflowWP); // returns a pointer to a newly created WaterParcel
         ASSERT(outflow == pOutflowWP->m_volume_m3);
         pNode->m_discharge = outflow / SEC_PER_DAY;  //convert units to m3/s;  

         if (pNode->m_discharge < 0 || pNode->m_discharge > 1.e10f || pNode->m_discharge != pNode->m_discharge)
            {
            CString msg;
            msg.Format("*** SolveReachKinematicWave(): COMID = %d, i = %d, l = %d, m_discharge = %f", 
               pReach->m_reachID, i, l, pNode->m_discharge);
            Report::LogMsg(msg);
            msg.Format("Setting pNode->m_discharge to %f cms and continuing.", NOMINAL_LOW_FLOW_CMS); Report::LogMsg(msg);
            pNode->m_discharge = NOMINAL_LOW_FLOW_CMS;
            }

         for (int k=0; k < pFlowContext->svCount;k++)
            {
            float ss = 0;
            ss = GetLateralSVInflow( pReach , k);//kg/d
           // ss+=GetReachSVInflow(pReach,0,k);//kg/m3

            pNode->SetStateVar(k, pNode->GetStateVar(k) + (ss*pFlowContext->timeStep));
            }
         }
      }

   clock_t finish = clock();
   double duration = (float)(finish - start) / CLOCKS_PER_SEC;   
   gpModel->m_reachFluxFnRunTime += (float) duration;   

   return true;
   }


float ReachRouting::GetLateralInflow( Reach *pReach )
   {
   float inflow = 0;
   inflow = pReach->GetFluxValue();
   inflow /= pReach->GetSubnodeCount();  // m3/day

   if (inflow != inflow)
      {
      CString msg; msg.Format("*** GetLateralInflow(): inflow = %f, pReach->GetFluxValue() = %f, pReach->GetSubnodeCount() = %d",
         inflow, pReach->GetFluxValue(), pReach->GetSubnodeCount());
      Report::LogMsg(msg);
      msg.Format("Setting inflow to 0.0 cms and continuing"); Report::LogMsg(msg);
      inflow = 0.f;
      }
   for (int i = 0; i < pReach->GetSubnodeCount(); i++)
   {
      ReachSubnode *pSubnode = pReach->GetReachSubnode(i);
      pSubnode->m_lateralInflow = inflow;
   }

   return inflow;
   } // end of GetLateralFlow()


WaterParcel ReachRouting::GetLateralInflowWP(Reach* pReach)
{
   WaterParcel inflowWP(0, 0);
   inflowWP.m_volume_m3 = pReach->GetFluxValue();
   WaterParcel subnodeInflowWP(inflowWP.m_volume_m3 / pReach->GetSubnodeCount(), inflowWP.WaterTemperature());

   if (inflowWP.m_volume_m3 != inflowWP.m_volume_m3)
   {
      CString msg; msg.Format("*** GetLateralInflowWP(): inflowWP.m_volume_m3 = %f, pReach->GetFluxValue() = %f, pReach->GetSubnodeCount() = %d",
         inflowWP.m_volume_m3, pReach->GetFluxValue(), pReach->GetSubnodeCount());
      Report::LogMsg(msg);
      msg.Format("Setting inflowWP.m_volume_m3 to 0.0 cms and continuing"); Report::LogMsg(msg);
      inflowWP.m_volume_m3 = 0.f;
   }

   for (int i = 0; i < pReach->GetSubnodeCount(); i++)
   {
      ReachSubnode* pSubnode = pReach->GetReachSubnode(i);
      pSubnode->m_lateralInflowWP = subnodeInflowWP;
      pSubnode->m_lateralInflow = (float)subnodeInflowWP.m_volume_m3; 
   }

   return subnodeInflowWP;
} // end of GetLateralInflowWP()



float ReachRouting::GetReachFluxes( FlowContext *pFlowContext, Reach *pReach )
   {
   float totalFlux = 0;

   pFlowContext->pReach = pReach;

   for ( int i=0; i < pReach->GetFluxCount(); i++ )
      {
      Flux *pFlux = pReach->GetFlux( i );
      ASSERT( pFlux != NULL );

      pFlowContext->pFlux = pFlux;
      pFlowContext->pFluxInfo = pFlux->m_pFluxInfo;
      
      float flux = pFlux->Evaluate( pFlowContext );
      
      // Note: this is a rate, applied over a time step

      // figure out which state var this flux is associated with
      //int sv = pFlux->m_pStateVar->m_index;

      // source or sink?  If sink, flip sign  
      if ( pFlux->IsSource() )
         totalFlux += flux;
      else
         totalFlux -= flux;
      }

   return totalFlux;
   }


// FlowModel::EstimateOutflow() ------------------------------------------
//
// solves the KW equations for the Reach downstream from pNode
//------------------------------------------------------------------------
float ReachRouting::EstimateReachOutflow( Reach *pReach, int subnode, double timeStep, float lateralInflow_m3)
   {
   // Test for the case where the reach runs dry
   float Qin_m3 = GetReachInflow(pReach, subnode) * SEC_PER_DAY;
   if (subnode == 0 && Qin_m3 + lateralInflow_m3 < 0.)
   { // More water is leaving the subnode laterally than is entering from upstream; the volume in the subnode will be drawn down.
/*
      float net_subnode_outflow_m3 = Qin_m3 + lateralInflow_m3;
      CString msg;
      msg.Format("EstimateReachOutflow()1 COMID = %d, subnode = %d, Qin_m3 = %f, lateralInflow_m3 = %f, net_subnode_outflow_m3 = %f",
         pReach->m_reachID, subnode, Qin_m3, lateralInflow_m3, net_subnode_outflow_m3);
      if (pReach->IsHeadwaterNode()) msg = msg + "  This is a headwater reach.";
      Report::LogMsg(msg);
*/
   }

   float beta  = 3.0f/5.0f;
   // compute Qin for this reach
   float dx = pReach->m_deltaX;
   double dt = timeStep* SEC_PER_DAY;
   float lateral = float( lateralInflow_m3/timeStep );//m3/d
   float qsurface = float( lateral/dx*timeStep );  //m2

   float Qnew = 0.0f;      
   float Qin = 0.0f;

   ReachSubnode *pSubnode = pReach->GetReachSubnode( subnode );
   ASSERT( pSubnode != NULL );

   float Q = pSubnode->m_discharge;   // m3/sec
   ASSERT(Q > 0);
   SetGeometry(pReach, Q);
   float width = pReach->m_width;
   float depth = pReach->m_depth;
   float n = pReach->m_n;
   float slope = pReach->m_slope;
   //if (slope < 0.001f)
   //   slope = 0.05f;
   float wp = width + depth + depth;

   float alph =  n * (float)pow( (long double) wp, (long double) (2/3.)) / (float)sqrt( slope );

   float alpha = (float) pow((long double) alph, (long double) 0.6);

   // Qin is the value upstream at the current time
   // Q is the value at the current location at the previous time
   Qin = GetReachInflow( pReach, subnode ); // m3/sec

   // If there is more water already in this reach than the sum of what is flowing in from upstream plus the lateral inflow,
   // then treat some of what is already in the reach as if it had flowed in from upstream.
   if (pSubnode->m_volume > (Qin * dt + lateralInflow_m3))
   {
      double H2O_to_move_m3 = pSubnode->m_volume - (Qin * dt + lateralInflow_m3);
      double H2O_to_move_cms = H2O_to_move_m3 / dt;
      Qin += (float)H2O_to_move_cms;
      pSubnode->m_volume -= H2O_to_move_m3;
   }

   if (Qin <= 0. && subnode == 0 && pReach->m_pLeft == NULL && pReach->m_pRight == NULL)
   { // This is the upstream subnode of a headwater reach.
      Catchment *pCatchment = NULL;
      if (pReach->m_catchmentArray.GetSize() > 0) pCatchment = pReach->m_catchmentArray[0];
      if (pCatchment == NULL || pCatchment->m_id <= 0)
      { // This is a headwater reach which is not associated with an HRU, so there is no inflow from soil.
         Qin = NOMINAL_LOW_FLOW_CMS; pSubnode->m_addedVolume_m3 += Qin * SEC_PER_DAY;
         if (lateralInflow_m3 < 0.)
         { // Negative lateral inflow is probably an irrigation withdrawal.
            CString msg; msg.Format("EstimateReachOutflow()2 Trying to withdraw water from a headwater reach which is not associated with an HRU\n"
               "pReach->m_reachID = %d, lateralInflow_m3 = %f", pReach->m_reachID, lateralInflow_m3);
            Report::LogMsg(msg);
         }
      }
      else if (lateralInflow_m3 > 0 && pSubnode->m_volume > lateralInflow_m3)
      { // For the purpose of the outflow calculation, treat any volume in this headwater node in excess of 
         // of one timestep's lateral inflow as if it had flowed in from an imaginary upstream reach.
         // This is to prevent the volume in this node from increasing without limit.
         double H2O_to_move_m3_per_timestep = pSubnode->m_volume - lateralInflow_m3;
         Qin = (float)(H2O_to_move_m3_per_timestep / dt); // m3/sec
         pSubnode->m_volume = lateralInflow_m3;
      }
   }

   float Qstar = ( Q + Qin  ) / 2.0f;   // from Chow, eqn. 9.6.4 (m3/sec)
   if (Qstar > 0.)
   {
      float z = alpha * beta * (float)pow(Qstar, beta - 1.0f);
      //// start computing new Q value ///
      // next, inflow term
      float Qin_dx = (Qin / dx) * (float)dt;   // (m3/sec)*sec/m = m2
                                               // next, current flow rate
      float Qcurrent_z_dt = Q * z;              // (m3/sec)*(Sec/m) = m2
                                                // last, divisor
      float divisor = z + (float)dt / dx;          // sec/m ???

                                                   // compute new Q
      Qnew = (qsurface + Qin_dx + Qcurrent_z_dt) / divisor; // m2/(sec/m) = m3/s
/*
      if (Qnew <= 0)
      {
         CString msg; msg.Format("EstimateReachOutflow()3 Qnew = %f, qsurface = %f, Qin_dx = %f, Qcurrent_z_dt = %f, divisor = %f\n"
            "lateral = %f, dx = %f, timeStep = %f, lateralInflow_m3 = %f",
            Qnew, qsurface, Qin_dx, Qcurrent_z_dt, divisor, lateral, dx, timeStep, lateralInflow_m3);
         Report::LogMsg(msg);
      }
*/
   }
   else
   { // Qstar is <= 0.  This condition may occur if we are withdrawing water from the reach.  This solution impacts the mass balance.
      Qnew = 0; // Will be set to > 0 just below.
      CString msg; msg.Format("EstimateReachOutflow()4 pReach->m_reachID = %d, subnode = %d Qstar = %f.",
         pReach->m_reachID, subnode, Qstar);
      if (subnode == 0) Report::LogMsg(msg);
   }

   if (isnan(Qnew) || Qnew <= 0.)
   { // Qnew should always be greater than zero.
      float original_Qnew = Qnew;
      if (isnan(Qnew)) { pSubnode->m_nanOccurred = true; Qnew = 0.; }
      pSubnode->m_addedDischarge_cms += NOMINAL_LOW_FLOW_CMS - Qnew;
      Qnew = NOMINAL_LOW_FLOW_CMS;
      CString msg; msg.Format("EstimateReachOutflow()5 pReach->m_reachID = %d, subnode = %d original Qnew = %f cms. Setting Qnew = %f cms. pSubnode->m_addedDischarge_cms = %f.",
         pReach->m_reachID, subnode, original_Qnew, Qnew, pSubnode->m_addedDischarge_cms);
      if (!isnan(original_Qnew) && original_Qnew < -0.01) Report::LogMsg(msg);
   }

   pSubnode->m_previousVolume = pSubnode->m_volume;
   // (Qin - Qnew) * dt has units of m3/day

   // Constrain the amount of water leaving the reach to no more than what is already there plus
   // what is coming in, less at least a small amount to keep the volume positive and greater than zero.
   float min_volume_m3 = (NOMINAL_LOW_WATER_LITERS_PER_METER * pReach->m_length / pReach->GetSubnodeCount()) / LITERS_PER_M3;
   double incoming_volume_m3 = Qin*dt + lateral*timeStep;
   double max_outgoing_volume_m3 = (pSubnode->m_previousVolume + incoming_volume_m3) - min_volume_m3;
   double max_Qnew_cms = max_outgoing_volume_m3 / dt;
   if (max_Qnew_cms <= 0) max_Qnew_cms = NOMINAL_LOW_FLOW_CMS;
   if (Qnew > max_Qnew_cms) Qnew = (float)max_Qnew_cms;
/*
   {
      Qnew = (float)max_Qnew_cms;
      SVTYPE delta_volume = (Qin*dt + lateral*timeStep) - (Qnew*dt); // (Qin - Qnew) * dt + lateral *  timeStep
      CString msg; msg.Format("EstimateReachOutflow()6 subnode volume prevented from going negative.  m_previousVolume = %lf, delta_volume = %lf, subnode = %d, orig_Qnew = %f, max_Qnew_cms = %f, Qnew = %f",
            pSubnode->m_previousVolume, delta_volume, subnode, orig_Qnew, max_Qnew_cms, Qnew);
      Report::LogMsg(msg);
      msg.Format("dx = %f, dt = %f, lateral = %f, qsurface = %f, Q = %f, width = %f, depth = %f, n = %f, slope = %f, Qin = %f, Qstar = %f, Qin_dx = %f, Qcurrent_z_dt = %f, divisor = %f",
            dx, dt, lateral, qsurface, Q, width, depth, n, slope, Qin, Qstar, Qin_dx, Qcurrent_z_dt, divisor);
      Report::LogMsg(msg);
      msg.Format("incoming_volume_m3 = %f, max_outgoing_volume_m3 = %f, max_Qnew_cms = %f", incoming_volume_m3, max_outgoing_volume_m3, max_Qnew_cms);
      Report::LogMsg(msg);
   }
*/
   pSubnode->m_volume += ( Qin - Qnew ) * dt + lateral *  timeStep;  // (m3/sec)*sec = m3
   if (pSubnode->m_volume <= 0)
   {
      pSubnode->m_addedVolume_m3 += min_volume_m3 - pSubnode->m_volume;
      pSubnode->m_volume = min_volume_m3;
   }

   SetGeometry(pReach, Qnew);
 
   return(SEC_PER_DAY*Qnew);    // return m3/day
   } // end of EstimateReachOutflow()


double ReachRouting::KinematicWave(double oldQ_cms /* Q */, double upstreamInflow_cms /* Qin */, double lateralInflow_cms, Reach * pReach)
{
   double beta = 3.0 / 5.0;
   double wp = pReach->m_width + pReach->m_depth + pReach->m_depth;
   double alph = pReach->m_n * pow((long double)wp, (long double)(2 / 3.)) / sqrt(pReach->m_slope);
   double alpha = pow((long double)alph, (long double) 0.6);
      
   double Qstar_cms = (oldQ_cms + upstreamInflow_cms) / 2.0f; ASSERT(Qstar_cms > 0.);  // from Chow, eqn. 9.6.4 (m3/sec)
   double z = alpha * beta * pow(Qstar_cms, beta - 1.0); // s/m
   float dx_m = pReach->m_deltaX;

   double Qin_m2 = (upstreamInflow_cms * SEC_PER_DAY) / dx_m; // inflow term
   double Qcurrent_m2 = oldQ_cms * z; // current flow rate
   double divisor_s_per_m = z + SEC_PER_DAY / dx_m; // divisor        
   double qsurface_m2 = (lateralInflow_cms * SEC_PER_DAY) / dx_m;  //m2
   double newQ_cms = (qsurface_m2 + Qin_m2 + Qcurrent_m2) / divisor_s_per_m; ASSERT(newQ_cms > 0.);

   return(newQ_cms);
} // end of KinematicWave()


WaterParcel ReachRouting::EstimateReachOutflowWP(Reach *pReach, int subnode, double timeStep, WaterParcel lateralInflowWP) 
   // WaterParcel version of EstimateReachOutflow()
   {
      ReachSubnode *pSubnode = pReach->GetReachSubnode(subnode); ASSERT(pSubnode != NULL);

      double original_energy_kJ = pSubnode->m_waterParcel.m_thermalEnergy_kJ;
      WaterParcel upstream_inflowWP = GetReachInflowWP(pReach, subnode);
      pSubnode->m_waterParcel.MixIn(upstream_inflowWP);

      pSubnode->m_waterParcel.MixIn(lateralInflowWP);

      double lateralInflow_m3 = lateralInflowWP.m_volume_m3;

      // Use yesterday's outflow rate to determine the geometry of this subreach.
      double old_Q_cms = pSubnode->m_discharge; ASSERT(old_Q_cms > 0); // old_Q_cms is yesterday's outflow rate from this subreach.
      SetGeometry(pReach, (float)old_Q_cms);
      float width = pReach->m_width;
      float depth = pReach->m_depth;

      // If there is more water already in this reach than the sum of what is flowing in from upstream plus the lateral inflow,
      // then treat some of what is already in the reach as if it had flowed in from upstream.
      double upstream_inflow_m3 = SEC_PER_DAY * GetReachInflow(pReach, subnode);
      double total_inflow_m3 = upstream_inflow_m3 + lateralInflow_m3;
      if (pSubnode->m_volume > total_inflow_m3)
      {
         double H2O_to_move_m3 = pSubnode->m_volume - total_inflow_m3;
         upstream_inflow_m3 += H2O_to_move_m3;
         pSubnode->m_volume -= H2O_to_move_m3;
      }

      if (upstream_inflow_m3 <= 0. && subnode == 0 && pReach->m_pLeft == NULL && pReach->m_pRight == NULL)
      { // This is the upstream subnode of a headwater reach.
         Catchment *pCatchment = NULL;
         if (pReach->m_catchmentArray.GetSize() > 0) pCatchment = pReach->m_catchmentArray[0];
         ASSERT(pCatchment != NULL);
         if (lateralInflow_m3 > 0 && pSubnode->m_volume > lateralInflow_m3)
         { // For the purpose of the outflow calculation, treat any volume in this headwater node in excess of 
           // of one timestep's lateral inflow as if it had flowed in from an imaginary upstream reach.
           // This is to prevent the volume in this node from increasing without limit.
            double H2O_to_move_m3 = pSubnode->m_volume - lateralInflow_m3;
            pSubnode->m_volume = lateralInflow_m3;
         }
      }

      double Qnew_cms = KinematicWave(old_Q_cms, upstream_inflow_m3 / SEC_PER_DAY, lateralInflow_m3 / SEC_PER_DAY, pReach);
      if (isnan(Qnew_cms) || Qnew_cms <= 0.)
      { // Qnew_cms should always be greater than zero.
         float original_Qnew = (float)Qnew_cms;
         if (isnan(Qnew_cms)) { pSubnode->m_nanOccurred = true; Qnew_cms = 0.; }
         pSubnode->m_addedDischarge_cms += (float)(NOMINAL_LOW_FLOW_CMS - Qnew_cms);
         Qnew_cms = NOMINAL_LOW_FLOW_CMS;
         CString msg; msg.Format("KinematicWave() pReach->m_reachID = %d, original Qnew = %f cms. Setting Qnew = %f cms. pSubnode->m_addedDischarge_cms = %f.",
            pReach->m_reachID, original_Qnew, Qnew_cms, pSubnode->m_addedDischarge_cms);
         if (!isnan(original_Qnew) && original_Qnew < -0.01) Report::LogMsg(msg);
      }

      pSubnode->m_previousVolume = pSubnode->m_volume;

      // Constrain the amount of water leaving the reach to no more than what is already there plus
      // what is coming in, less at least a small amount to keep the volume positive and greater than zero.
      float min_volume_m3 = (NOMINAL_LOW_WATER_LITERS_PER_METER * pReach->m_length / pReach->GetSubnodeCount()) / LITERS_PER_M3;
      double incoming_volume_m3 = upstream_inflow_m3 + lateralInflow_m3;
      double max_outgoing_volume_m3 = (pSubnode->m_previousVolume + incoming_volume_m3) - min_volume_m3;
      double max_Qnew_cms = max_outgoing_volume_m3 / SEC_PER_DAY;
      if (max_Qnew_cms < NOMINAL_LOW_FLOW_CMS) max_Qnew_cms = NOMINAL_LOW_FLOW_CMS;
      if (Qnew_cms > max_Qnew_cms) Qnew_cms = max_Qnew_cms;

      pSubnode->m_volume += upstream_inflow_m3 - (SEC_PER_DAY * Qnew_cms) + lateralInflow_m3;  
      if (pSubnode->m_volume <= 0)
      {
         double min_volume_m3 = (NOMINAL_LOW_WATER_LITERS_PER_METER * pReach->m_length / pReach->GetSubnodeCount()) / LITERS_PER_M3;
         pSubnode->m_addedVolume_m3 += min_volume_m3 - pSubnode->m_volume;
         pSubnode->m_volume = min_volume_m3;
      }

      SetGeometry(pReach, (float)Qnew_cms);

      return(pSubnode->m_waterParcel);   
   } // end of EstimateReachOutflowWP()


float ReachRouting::GetReachInflow( Reach *pReach, int subNode )
   {
   float Q = 0;

   ///////
   if ( subNode == 0 )  // look upstream?
      {
      Reservoir *pRes = pReach->m_pReservoir;

      if ( pRes == NULL )
         {
         Q = GetReachOutflow( pReach->m_pLeft );
         Q += GetReachOutflow( pReach->m_pRight );
         }
      else
         //Q = pRes->GetResOutflow( pRes, m_flowContext.dayOfYear );
       Q = pRes->m_outflow/SEC_PER_DAY;  //m3/day to m3/s
      }
   else
      {
      ReachSubnode *pNode = (ReachSubnode*) pReach->m_subnodeArray[ subNode-1 ];  ///->GetReachSubnode( subNode-1 );
      ASSERT( pNode != NULL );
      Q = pNode->m_discharge;
      }

   return Q;
   } // end of GetReachInflow()


WaterParcel ReachRouting::GetReachInflowWP(Reach* pReach, int subNode)
{
   WaterParcel inflowWP(0,0);
   if (subNode == 0)  // look upstream?
   {
      Reservoir* pRes = pReach->m_pReservoir;
      if (pRes != NULL)
      {
         inflowWP = pRes->m_outflowWP;
         ASSERT(pReach->m_pRight == NULL);
      }
      else
      {
         inflowWP = GetReachOutflowWP(pReach->m_pLeft);
         inflowWP.MixIn(GetReachOutflowWP(pReach->m_pRight));
      }
   }
   else
   {
      ReachSubnode* pNode = (ReachSubnode*)pReach->m_subnodeArray[subNode - 1];  ///->GetReachSubnode( subNode-1 );
      ASSERT(pNode != NULL);
      inflowWP.m_volume_m3 = pNode->m_discharge * SEC_PER_DAY; inflowWP.m_thermalEnergy_kJ = inflowWP.ThermalEnergy(pNode->m_waterParcel.WaterTemperature());
   }

   return(inflowWP);
} // end of GetReachInflowWP()

float ReachRouting::GetReachOutflow( ReachNode *pReachNode )   // recursive!!! for pahntom nodes
   {
   if ( pReachNode == NULL )
      return 0;

   if ( pReachNode->IsPhantomNode() )
      {
      float q = GetReachOutflow( pReachNode->m_pLeft );
      q += GetReachOutflow( pReachNode->m_pRight );

      return q;
      }
   else
      {
      Reach *pReach = gpModel->GetReachFromNode( pReachNode );

      if ( pReach != NULL )
         return pReach->GetDischarge();
      else
         {
         ASSERT( 0 );
         return 0;
         }
      }
   }


WaterParcel ReachRouting::GetReachOutflowWP(ReachNode* pReachNode)
{
   WaterParcel outflowWP(0, 0);
   if (pReachNode == NULL) return(outflowWP);

   ASSERT(!pReachNode->IsPhantomNode());

   Reach* pReach = gpModel->GetReachFromNode(pReachNode);
   ASSERT(pReach != NULL);
   outflowWP = pReach->GetDischargeWP();
   return(outflowWP);
} // end of GetReachOutflowWP()


float ReachRouting::GetLateralSVInflow( Reach *pReach, int sv )
   {
   float inflow = 0;
   ReachSubnode *pNode = pReach->GetReachSubnode( 0 );
   inflow=pNode->GetExtraSvFluxValue(sv);
   inflow /= pReach->GetSubnodeCount();  // kg/d

   return inflow;
   }


float ReachRouting::GetReachSVOutflow( ReachNode *pReachNode, int sv )   // recursive!!! for pahntom nodes
   {
   if ( pReachNode == NULL )
      return 0;

   if ( pReachNode->IsPhantomNode() )
      {
      float flux = GetReachSVOutflow( pReachNode->m_pLeft, sv );
      flux += GetReachSVOutflow( pReachNode->m_pRight, sv );

      return flux;
      }
   else
      {
      Reach *pReach = gpModel->GetReachFromNode( pReachNode );
      ReachSubnode *pNode = (ReachSubnode*) pReach->m_subnodeArray[ 0 ]; 

      if ( pReach != NULL && pNode->m_volume>0.0f)
         return float( pNode->GetStateVar( sv ) / pNode->m_volume*pNode->m_discharge*86400.0f );//kg/d 
      else
         {
         //ASSERT( 0 );
         return 0;
         }
      }
   }


float ReachRouting::GetReachSVInflow( Reach *pReach, int subNode, int sv )
   {
    float flux=0.0f;
    float Q=0.0f;

   if ( subNode == 0 )  // look upstream?
      {
      Reservoir *pRes = pReach->m_pReservoir;

      if ( pRes == NULL )
         {
         flux = GetReachSVOutflow( pReach->m_pLeft, sv );
         flux += GetReachSVOutflow( pReach->m_pRight, sv );
         }
      else
         flux = pRes->m_outflow/SEC_PER_DAY;  //m3/day to m3/s
      }
   else
      {
      ReachSubnode *pNode = (ReachSubnode*) pReach->m_subnodeArray[ subNode-1 ];  ///->GetReachSubnode( subNode-1 );
      ASSERT( pNode != NULL );
     
      if ( pNode->m_svArray != NULL)
         flux  = float( pNode->m_svArray[sv]/pNode->m_volume*  pNode->m_discharge*86400.0f );//kg/d 
      }

   return flux; //kg/d;
   }


void ReachRouting::GetReachDerivatives( double time, double timeStep, int svCount, double *derivatives /*out*/, void *extra )
   {
   FlowContext *pFlowContext = (FlowContext*) extra;

   FlowContext flowContext( *pFlowContext );   // make a copy for openMP

   // NOTE: NEED TO INITIALIZE TIME IN FlowContext before calling here...
   // NOTE:  Should probably do a breadth-first search of the tree(s) rather than what is here.  This could be accomplished by
   //  sorting the m_reachArray in InitReaches()

   FlowModel *pModel = pFlowContext->pFlowModel;
   flowContext.timeStep = (float) timeStep;
   
   // compute derivative values for reachs based on any associated subnode fluxes
   int reachCount = (int) pModel->m_reachArray.GetSize();

   omp_set_num_threads( gpFlow->m_processorsUsed );
   #pragma omp parallel for firstprivate( flowContext )
   for ( int i=0; i < reachCount; i++ )
      {
      Reach *pReach = pModel->m_reachArray[ i ];
      flowContext.pReach = pReach;

      for ( int l=0; l < pReach->GetSubnodeCount(); l++ )
         {
         ReachSubnode *pNode = pReach->GetReachSubnode( l );
         int svIndex = pNode->m_svIndex;

         flowContext.reachSubnodeIndex = l;
         derivatives[svIndex]=0;

         // add all fluxes for this reach
         for ( int k=0; k < pReach->GetFluxCount(); k++ )
            {
            Flux *pFlux = pReach->GetFlux( k );
            flowContext.pFlux = pFlux;

            float flux = pFlux->Evaluate( pFlowContext );

            // figure out which state var this flux is associated with
            int sv = pFlux->m_pStateVar->m_index;

            if ( pFlux->IsSource() )
               {
               derivatives[ svIndex + sv ] += flux;
               pModel->m_totalWaterInputRate += flux;
               }
            else
               {
               derivatives[ svIndex + sv ] -= flux;
               pModel->m_totalWaterOutputRate += flux;
               }
            }
         }
      }
   }


ReachRouting *ReachRouting::LoadXml( TiXmlElement *pXmlReachRouting, LPCTSTR filename )
   {
   ReachRouting *pRouting = new ReachRouting( "Reach Routing" );  // defaults to GM_KINEMATIC

   LPTSTR method = NULL;
   LPTSTR query = NULL;   // ignored for now
   LPTSTR fn = NULL;
   LPTSTR db = NULL;

   XML_ATTR attrs[] = {
      // attr                 type          address                     isReq  checkCol
      { "name",               TYPE_CSTRING,   &(pRouting->m_name),      false,   0 },
      { "method",             TYPE_STRING,    &method,                  false,   0 },
      { "query",              TYPE_STRING,    &query,                   false,   0 },
      { NULL,                 TYPE_NULL,     NULL,                      false,  0 } };

   bool ok = TiXmlGetAttributes( pXmlReachRouting, attrs, filename );
   if ( ! ok )
      {
      CString msg; 
      msg.Format( _T("Flow: Misformed element reading <reach_routing> attributes in input file %s"), filename );
      Report::ErrorMsg( msg );
      }

   if ( method )
      {
      if ( lstrcmpi( method, "kinematic" ) == 0 )
         pRouting->SetMethod(  GM_KINEMATIC );
      else if ( lstrcmpi( method, "euler" ) == 0 )
         pRouting->SetMethod(  GM_EULER );
      else if ( lstrcmpi( method, "rkf" ) == 0 )
         pRouting->SetMethod(  GM_RKF );
      else if ( lstrcmpi( method, "rk4" ) == 0 )
         pRouting->SetMethod(  GM_RK4 );
      else if ( lstrcmpi( method, "none" ) == 0 )
         pRouting->SetMethod(  GM_NONE );
      else if ( strncmp( method, "fn", 2 ) == 0 )
         {
         pRouting->SetMethod(  GM_EXTERNAL );
         // source string syntax= fn:<dllpath:function> for functional, db:<datasourcepath:columnname> for datasets
         pRouting->m_extSource = method;
         FLUXSOURCE sourceLocation = ParseSource( pRouting->m_extSource, pRouting->m_extPath, pRouting->m_extFnName,
               pRouting->m_extFnDLL, pRouting->m_extFn );
         
         if ( sourceLocation != FS_FUNCTION )
            {
            Report::ErrorMsg( "Fatal Error on direct reach solution method - no solution will be performed" );
            pRouting->SetMethod( GM_NONE );
            }
         }
      }

   return pRouting;
   }
   