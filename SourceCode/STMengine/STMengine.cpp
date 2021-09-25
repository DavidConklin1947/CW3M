// STMengine.cpp 
// James Sulzman wrote a state-and-transition engine for the WW2100 project called DynamicVeg.
// Dave Conklin simplified and extensively rewrote DynamicVeg for the OUWIN and INFEWS projects, and renamed it STMengine.
// Conklin subsequently used STMengine as the starting point for a more generic version called STMengine, adding
// a capability for conditional transitions. Conditional transitions, a generalization of probabilistic transitions,
// are transitions based on the value of a particular IDU attribute, relative to specified thresholds.

#include "stdafx.h"
#pragma hdrstop

#include <PathManager.h>
#include <iostream>
#include "STMengine.h"
#include <Maplayer.h>
#include <map.h>
#include <Vdataobj.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <ctime>
#include <cstdlib>
#include <random>
#include <numeric>
#include <Report.h>
#include <tixml.h>
#include <omp.h>
#include "GDALWrapper.h"
#include "GeoSpatialDataObj.h"
#include <afxtempl.h>
#include <initializer_list>
#include <Deltaarray.h>
#include <EnvInterface.h>
#include <EnvEngine\EnvModel.h>

using namespace std;

IDUlayer * gIDUs = NULL;
FlowModel* gFM = NULL;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


STMengine::STMengine()
   : EnvAutoProcess()
   , m_colVEGTRNTYPE(-1)

   , m_colDetSTARTAGE(-1)
   , m_colDetENDAGE(-1)
   , m_colDetLAI(-1)

{ }

STMengine::~STMengine()
   {
   }

BOOL STMengine::Init(EnvContext *pEnvContext, LPCTSTR initStr)
{
   m_pEnvContext = pEnvContext;
   gIDUs = (IDUlayer *)pEnvContext->pMapLayer;

   bool ok = LoadXml(initStr);
   if (!ok)
   {
      CString msg;
      msg = ("STMengine::LoadXml(%s) returned false.", initStr);
      Report::ErrorMsg(msg);
      return(false);}

   for (int i = 0; i < (int)m_outputArray.GetSize(); i++)
      {
      OUTPUT *pOutput = m_outputArray[i];
      AddOutputVar(pOutput->name, pOutput->value, "");
      }


   int records = m_detTransTable.ReadAscii(m_detTransFileName, ',');
   if (records < 1)
   {
      CString msg;
      msg.Format("STMengine::Init() Unable to load deterministic transition file %s. ReadAscii() returns %d.",
         m_detTransFileName.GetString(), records);
      Report::ErrorMsg(msg);
      return(false);
   }
   m_colDetCURR_STATE = m_detTransTable.GetCol("CURR_STATE");
   m_colDetNEW_STATE = m_detTransTable.GetCol("NEW_STATE");
   m_colDetSTARTAGE = m_detTransTable.GetCol("STARTAGE");
   m_colDetENDAGE = m_detTransTable.GetCol("ENDAGE");
   m_colDetLAI = m_detTransTable.GetCol("LAI");
   if (m_colDetCURR_STATE < 0 || m_colDetNEW_STATE < 0 || m_colDetSTARTAGE < 0 || m_colDetENDAGE < 0 || m_colDetLAI < 0)
   {
      CString msg;
      msg.Format("STMengine::::Init() Missing column in deterministic transition file %s: "
         "m_colDetCURR_STATE = %d, m_colDetNEW_STATE = %d, m_colDetSTARTAGE = %d, m_colDetENDAGE = %d, m_colDetLAI = %d",
         m_vegtransfile.deterministic_filename.GetString(), m_colDetCURR_STATE, m_colDetNEW_STATE, m_colDetSTARTAGE, m_colDetENDAGE, m_colDetLAI);
      Report::ErrorMsg(msg);
      return(false);
   }

   records = m_condTransTable.ReadAscii(m_condTransFileName, ',');
   if (records < 1)
   {
      CString msg;
      msg.Format("STMengine::Init() Unable to load conditional transition file %s. ReadAscii() returns %d.",
         m_condTransFileName.GetString(), records);
      Report::ErrorMsg(msg);
      return(false);
   }
   m_colCondCURR_STATE = m_condTransTable.GetCol("CURR_STATE");
   m_colCondWET_FRACatLeast = m_condTransTable.GetCol("WET_FRACatLeast");
   m_colCondWET_FRAClessThan = m_condTransTable.GetCol("WET_FRAClessThan");
   m_colCondWETLONGESTatLeast = m_condTransTable.GetCol("WETLONGESTatLeast");
   m_colCondWETLONGESTlessThan = m_condTransTable.GetCol("WETLONGESTlessThan");
   m_colCondWETAVGDPTHatLeast = m_condTransTable.GetCol("WETAVGDPTHatLeast");
   m_colCondWETAVGDPTHlessThan = m_condTransTable.GetCol("WETAVGDPTHlessThan");
   m_colCondNEW_STATE = m_condTransTable.GetCol("NEW_STATE");
   m_colCondPROBABILITY = m_condTransTable.GetCol("PROBABILITY");
   if (m_colCondCURR_STATE < 0 || m_colCondWET_FRACatLeast < 0 || m_colCondWET_FRAClessThan < 0 || m_colCondWETLONGESTatLeast < 0 ||
      m_colCondWETLONGESTlessThan < 0 || m_colCondWETAVGDPTHatLeast < 0 || m_colCondWETAVGDPTHlessThan < 0 ||
      m_colCondNEW_STATE < 0 || m_colCondPROBABILITY < 0)
   {
      CString msg;
      msg.Format("STMengine::::Init() Missing column in conditional transition file %s", m_vegtransfile.probability_filename.GetString());
      Report::ErrorMsg(msg);
      return(false);
   }

   Report::LogMsg("STMengine::Init() was successful.");
   return(true);
} // end of STMengine::Init()


BOOL STMengine::InitRun(EnvContext *pEnvContext, bool useInitSeed)
{ // Zero out WET_FRAC, WETLONGEST, WETAVGDPTH. Set STM_INDEX. 
   gIDUs->SetColData(WET_FRAC, 0, true);
   gIDUs->SetColData(WETLONGEST, 0, true);
   gIDUs->SetColData(WETAVGDPTH, 0, true);

   for (MapLayer::Iterator idu = pEnvContext->pMapLayer->Begin(); idu != pEnvContext->pMapLayer->End(); idu++)
   {
      int curr_state = AttInt(idu, VEGCLASS);
      if (STM_CLASS_MIN <= curr_state && curr_state <= STM_CLASS_MAX)
      {
         int stm_index = m_detTransTable.Find(m_colDetCURR_STATE, (VData)curr_state, 0);
         if (stm_index < 0)
         {
            CString msg;
            msg.Format("STMengine::InitRun() stm_index = %d, idu = %d, curr_state = %d", stm_index, (int)idu, curr_state);
            Report::ErrorMsg(msg);
            return(false);
         }
         SetAttInt(idu, STM_INDEX, stm_index);
      }
//X      else SetAttInt(idu, STM_INDEX, -1);
   } // end of loop thru IDUs

   return(true);
} // end of STMengine::InitRun()


BOOL STMengine::Run(EnvContext *pEnvContext)
{
   for (MapLayer::Iterator idu = pEnvContext->pMapLayer->Begin(); idu != pEnvContext->pMapLayer->End(); idu++)
   {
      int curr_state = AttInt(idu, VEGCLASS);
      if (curr_state < STM_CLASS_MIN || STM_CLASS_MAX < curr_state) continue;

      // Check first for conditional transitions, and then for deterministic transitions (aging out of 
      // one class into the next).  Finally, if no transitions occur, simply increment the age (AGECLASS).
      // When a transition does occur, updated values are written to:
      // VEGCLASS, PVT, AGECLASS, LAI, ...
      int curr_stm_ndx = AttInt(idu, STM_INDEX);
      if (curr_stm_ndx < 0)
      {
         CString msg;
         msg.Format("STMengine::Run() curr_stm_ndx = %d, idu = %d", curr_stm_ndx, (int)idu);
         Report::ErrorMsg(msg);
         continue;
      }

      int curr_age = AttInt(idu, AGECLASS);
      int new_state = -1;

      int new_stm_ndx = ConditionalTransition(idu, curr_stm_ndx, curr_age); 
      if (new_stm_ndx < 0)
      {
         CString msg;
         msg.Format("STMengine::Run() new_stm_ndx = %d, idu = %d, curr_stm_ndx = %d", new_stm_ndx, (int)idu, curr_stm_ndx);
         Report::ErrorMsg(msg);
         continue;
      }

      if (new_stm_ndx == curr_stm_ndx) new_stm_ndx = DeterministicTransition(idu, curr_stm_ndx, curr_age);

      if (new_stm_ndx != curr_stm_ndx)
      { // Transition to a new state. Update STM_INDEX, VEGCLASS, AGECLASS, and LAI.
         SetAttInt(idu, STM_INDEX, new_stm_ndx);

         int new_state = m_detTransTable.GetAsInt(m_colDetCURR_STATE, new_stm_ndx);
         UpdateIDU(pEnvContext, idu, VEGCLASS, new_state, true);

         int new_age = m_detTransTable.GetAsInt(m_colDetSTARTAGE, new_stm_ndx);
         SetAttInt(idu, AGECLASS, new_age);

         float new_lai = m_detTransTable.GetAsFloat(m_colDetLAI, new_stm_ndx);
         SetAttFloat(idu, LAI, new_lai);
      }
      else
      { // No transitions. Just increment AGECLASS.
         int new_age = curr_age + 1;
         SetAttInt(idu, AGECLASS, new_age);
      }
   } // end of IDU loop
       
   return(true);

} // end of STMengine::Run()


BOOL STMengine::EndRun(EnvContext*)
   {
   if (m_badVegTransClasses.GetSize() == 0)
      return TRUE;

   int count = (int)m_badVegTransClasses.GetSize();
   CString msg;
   msg.Format("STMengine:  No probabilistic transitions were found for the following %d veg classes: \n"
      "m_vegClass,m_disturb,m_regen,m_si,m_pvt,m_region\n", count);

   for (int i = 0; i <= m_badVegTransClasses.GetUpperBound(); i++)
      {
      msg += "STMengine missing probabilistic transition: ";
      msg += m_badVegTransClasses[i];
      }

   Report::LogMsg(msg);

   m_badVegTransClasses.RemoveAll();

   if (m_badDeterminVegTransClasses.GetSize() == 0)
      return TRUE;

   count = (int)m_badDeterminVegTransClasses.GetSize();

   msg.Format("STMengine:  No deterministic transitions were found for the following %d veg classes: \n"
      "m_vegClass,m_disturb,m_regen,m_si,m_pvt,m_region\n", count);

   for (int i = 0; i <= m_badDeterminVegTransClasses.GetUpperBound(); i++)
      {
      msg += "STMengine missing deterministic transition: ";
      msg += m_badDeterminVegTransClasses[i];
      }

   Report::LogMsg(msg);

   m_badDeterminVegTransClasses.RemoveAll();

   return TRUE;
} // end of STMengine::EndRun()



int STMengine::DeterministicTransition(int idu, int currSTMndx, int currAge) // Returns index of new state in the DetTransTable.
{
   if (currSTMndx < 0)
   {
      CString msg;
      msg.Format("STMengine::DeterministicTransition() currSTMndx = %d, idu = %d", currSTMndx, idu);
      Report::ErrorMsg(msg);
      return(currSTMndx);
   }

   int new_stm_ndx = currSTMndx;

   int end_age = m_detTransTable.GetAsInt(m_colDetENDAGE, currSTMndx);
   if (currAge >= end_age)
   { // Transition to the next state.
      int new_state = m_detTransTable.GetAsInt(m_colDetNEW_STATE, currSTMndx);
      new_stm_ndx = m_detTransTable.Find(m_colDetCURR_STATE, (VData)new_state, 0);
      if (new_stm_ndx < 0)
      {
         CString msg;
         msg.Format("STMengine::DeterministicTransition() new_stm_ndx = %d, currSTMndx = %d, idu = %d", new_stm_ndx, currSTMndx, idu);
         Report::ErrorMsg(msg);
         return(currSTMndx);
      }

   }

   return(new_stm_ndx);
} // end of DeterministicTransition()


int STMengine::ConditionalTransition(int idu, int currSTMndx, int currAge)
{ // Returns index of new state in the DetTransTable, or currSTMndx if no conditional transition is selected.
   // 1. Find the entries in the conditional transition table for the current state.  If none, return -1.
   // 2. In the order the transitions appear in the table, choose whether or not to use the transition.
   //    If the conditions aren't satisfied, don't use the transition.
   //    If the conditions are satisfied, evaluate the likelihood of the transition.
   //    Pick a random number from a uniform distribution on the unit interval.  
   //    If the random number is <= the likelihood of the conditional transition,
   //    return the index of the new state in the deterministic transition table..
   // 3. If no transition is selected, return -1.

   int new_stm_ndx = -1;
   int curr_state = m_detTransTable.GetAsInt(m_colDetCURR_STATE, currSTMndx);

   int num_entries = m_condTransTable.GetRowCount();
   int cond_trans_ndx = 0;
   while (new_stm_ndx < 0 && cond_trans_ndx < num_entries)
   {
      int source_state = m_condTransTable.GetAsInt(m_colCondCURR_STATE, cond_trans_ndx);

      if (source_state == curr_state && ConditionsAreMet(cond_trans_ndx, idu))
      {
         float probability = m_condTransTable.GetAsFloat(m_colCondPROBABILITY, cond_trans_ndx);
         double rand_num = (double)m_rn.RandValue(0., 1.);  //randum number between 0 and 1, uniform distribution                                                     //normalize probability list if disturbance. this means a disturbance transition will be selected. 
         if (rand_num <= probability)
         {
            int new_state = m_condTransTable.GetAsInt(m_colCondNEW_STATE, cond_trans_ndx);
            new_stm_ndx = m_detTransTable.Find(m_colDetCURR_STATE, (VData)new_state, 0);
         }
      }
      
      cond_trans_ndx++;
   } // end of while (new_stm_ndx < 0 && cond_trans_ndx < num_entries)

   if (new_stm_ndx >= 0) return(new_stm_ndx);
   else return(currSTMndx);
} // end of ConditionalTransition()


bool STMengine::ConditionsAreMet(int condTransNdx, int idu)
{
   double wet_frac = Att(idu, WET_FRAC);
   float wet_frac_at_least = m_condTransTable.GetAsFloat(m_colCondWET_FRACatLeast, condTransNdx);
   if (wet_frac < wet_frac_at_least) return(false);
   float wet_frac_less_than = m_condTransTable.GetAsFloat(m_colCondWET_FRAClessThan, condTransNdx);
   if (wet_frac >= wet_frac_less_than) return(false);

   double wet_longest = Att(idu, WETLONGEST);
   float wet_longest_at_least = m_condTransTable.GetAsFloat(m_colCondWETLONGESTatLeast, condTransNdx);
   if (wet_longest < wet_longest_at_least) return(false);
   float wet_longest_less_than = m_condTransTable.GetAsFloat(m_colCondWETLONGESTlessThan, condTransNdx);
   if (wet_longest >= wet_longest_less_than) return(false);

   double wet_avg_dpth = Att(idu, WETAVGDPTH);
   float wet_avg_dpth_at_least = m_condTransTable.GetAsFloat(m_colCondWETAVGDPTHatLeast, condTransNdx);
   if (wet_avg_dpth < wet_avg_dpth_at_least) return(false);
   float wet_avg_dpth_less_than = m_condTransTable.GetAsFloat(m_colCondWETAVGDPTHlessThan, condTransNdx);
   if (wet_avg_dpth >= wet_avg_dpth_less_than) return(false);

   return(true);
} // end of ConditionsAreMet()


bool STMengine::CW3Mwetland_DailyProcess(FlowContext* pFlowContext)
{
   int timing = pFlowContext->timing;

   if (timing & GMT_INIT)
   {
      m_pFlowContext = pFlowContext;
      m_pEnvContext = m_pFlowContext->pEnvContext;
      gIDUs = (IDUlayer*)m_pEnvContext->pMapLayer;
      gFM = m_pFlowContext->pFlowModel;
      gIDUs->SetColDataU(WET_FRAC, 0);
      gIDUs->SetColDataU(WETLONGEST, 0);
      gIDUs->SetColDataU(WET_LENGTH, 0);
      gIDUs->SetColDataU(WETAVGDPTH, 0);

      return(true);
   } // end of if (timing & GMT_INIT)

//   if (timing & GMT_INITRUN) return(CW3Mfire_InitRun());
   if (timing & GMT_START_YEAR) return(CW3Mwetland_StartYear());
   if (timing & GMT_END_STEP) return(CW3Mwetland_EndStep());

   return(true);
} // end of CW3Mwetland_daily_process()


bool STMengine::CW3Mwetland_StartYear() 
{ 
   int num_wetlands = gFM->m_wetlArray.GetSize();
   for (int wetl_ndx = 0; wetl_ndx < num_wetlands; wetl_ndx++)
   {
      Wetland* pWetl = gFM->m_wetlArray[wetl_ndx];
      int num_idus_in_wetland = pWetl->m_wetlIDUndxArray.GetSize();
      for (int idu_ndx_in_wetland = 0; idu_ndx_in_wetland < num_idus_in_wetland; idu_ndx_in_wetland++)
      {
         int idu_poly_ndx = pWetl->m_wetlIDUndxArray[idu_ndx_in_wetland];
         SetAtt(idu_poly_ndx, WET_FRAC, 0.);
         SetAttInt(idu_poly_ndx, WETLONGEST, 0);
         SetAttInt(idu_poly_ndx, WET_LENGTH, 0);
         SetAtt(idu_poly_ndx, WETAVGDPTH, 0.);
      } // end of loop thru the IDUs in this wetland
   } // end of loop thru wetlands

   return(true);
} // end of CW3Mwetland_StartYear()


bool STMengine::CW3Mwetland_EndStep() 
{ 
   int jday0 = m_pFlowContext->dayOfYear; // Jan 1 = 0
   int jday1 = jday0 + 1; // Jan 1 = 1
   int num_wetlands = gFM->m_wetlArray.GetSize();
   for (int wetl_ndx = 0; wetl_ndx < num_wetlands; wetl_ndx++)
   {
      Wetland* pWetl = gFM->m_wetlArray[wetl_ndx];
      int num_idus_in_wetland = pWetl->m_wetlIDUndxArray.GetSize();
      for (int idu_ndx_in_wetland = 0; idu_ndx_in_wetland < num_idus_in_wetland; idu_ndx_in_wetland++) 
      {
         int idu_poly_ndx = pWetl->m_wetlIDUndxArray[idu_ndx_in_wetland];
         double wetness = Att(idu_poly_ndx, WETNESS);
         double wet_frac = Att(idu_poly_ndx, WET_FRAC); // Fraction of days so far this year on which the IDU was inundated.
         int num_days_inundated = round(wet_frac * jday0);
         if (wetness <= 0.) SetAttInt(idu_poly_ndx, WET_LENGTH, 0); // The IDU is not inundated today.
         else
         { // The IDU is inundated today.
            double wetavgdpth = Att(idu_poly_ndx, WETAVGDPTH); // Average inundation depth on days when the IDU is inundated.
            double depth_sum = (wetavgdpth * num_days_inundated) + wetness;
            num_days_inundated++;
            wetavgdpth = depth_sum / num_days_inundated;
            SetAtt(idu_poly_ndx, WETAVGDPTH, wetavgdpth);

            int wet_length = Att(idu_poly_ndx, WET_LENGTH); // Number of days in a row that the IDU has been inundated.
            wet_length++;
            SetAttInt(idu_poly_ndx, WET_LENGTH, wet_length);

            int wet_longest = Att(idu_poly_ndx, WETLONGEST); // Length of longest interval of continuous inundation.
            if (wet_length > wet_longest) SetAttInt(idu_poly_ndx, WETLONGEST, wet_length);
         } // end of if (wetness <= 0.) ... else

         wet_frac = num_days_inundated / jday1;
         SetAtt(idu_poly_ndx, WET_FRAC, wet_frac);
      } // end of loop thru the IDUs in this wetland
    } // end of loop thru wetlands

   return(true);
} // end of CW3Mwetland_EndStep()


bool STMengine::LoadXml(LPCTSTR _filename)
{
   CString filename;
   if ( PathManager::FindPath( _filename, filename ) < 0 ) //  return value: > 0 = success; < 0 = failure (file not found), 0 = path fully qualified and found 
   {
      CString msg;
      msg.Format( "STMengine::LoadXml(): Input file '%s' not found - this process will be disabled", _filename );
      Report::ErrorMsg( msg );
      return false;
   }

   // start parsing input file
   TiXmlDocument doc;
   bool ok = doc.LoadFile(filename);
   if (!ok)
      {
      CString msg;
      msg.Format("STMengine::LoadXml(): Error reading input file %s:  %s", filename, doc.ErrorDesc());
      Report::ErrorMsg(msg);
      return false;
      }

   TiXmlElement *pXmlRoot = doc.RootElement(); 
   LPCTSTR condFileName=NULL, detFileName=NULL;
   XML_ATTR setAttrsveg[] = {
      // attr                      type           address              isReq  checkCol        
         { "deterministic_transitions_file",  TYPE_STRING, &detFileName,      true, 0 },
         { "conditional_transitions_file",    TYPE_STRING, &condFileName,     true, 0 },
         { NULL, TYPE_NULL, NULL, false, 0 } };

   ok = TiXmlGetAttributes(pXmlRoot, setAttrsveg, filename, NULL);
   if (!ok)
   {
      CString msg;
      msg.Format("STMengine::LoadXml(): Error interpreting input file %s", filename);
      Report::ErrorMsg(msg);
      return false;
   }


   int retVal = PathManager::FindPath(detFileName, m_detTransFileName);  //  return value: > 0 = success; < 0 = failure (file not found), 0 = path fully qualified and found 
   if (retVal < 0)
   {
      CString msg;
      msg.Format("STMengine: Deterministic Transition file '%s' not found.", detFileName);
      Report::ErrorMsg(msg);
      return false;
   }
   retVal = PathManager::FindPath( condFileName, m_condTransFileName);  //  return value: > 0 = success; < 0 = failure (file not found), 0 = path fully qualified and found 
   if ( retVal < 0 )
   {
      CString msg;
      msg.Format( "STMengine::LoadXml() conditional transition file '%s' not found.", condFileName );
      Report::ErrorMsg( msg );
      return false;
   }

   return true;
} // end of STMengine::LoadXml()


inline double STMengine::Att(int iduPolyNdx, int col)
{
   return(gIDUs->Att(iduPolyNdx, col));
} // end of STMengine::Att()


inline float STMengine::AttFloat(int iduPolyNdx, int col)
{
   return(gIDUs->AttFloat(iduPolyNdx, col));
} // end of STMengine::AttFloat()


inline int STMengine::AttInt(int iduPolyNdx, int col)
{
   return(gIDUs->AttInt(iduPolyNdx, col));
} // end of STMengine::AttInt()

inline void STMengine::SetAtt(int IDUpolyNdx, int col, double attValue)
{
   gIDUs->SetData(IDUpolyNdx, col, attValue);
} // end of STMengine::SetAtt()

inline void STMengine::SetAttFloat(int IDUpolyNdx, int col, float attValue)
{
   gIDUs->SetData(IDUpolyNdx, col, attValue);
} // end of STMengine::SetAttFloat()

inline void STMengine::SetAttInt(int IDUpolyNdx, int col, int attValue)
{
   gIDUs->SetData(IDUpolyNdx, col, attValue);
} // STMengine::SetAttInt()

