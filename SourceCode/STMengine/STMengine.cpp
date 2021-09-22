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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


STMengine::STMengine()
   : EnvAutoProcess()
, m_colVEGTRNTYPE(-1)

, m_colDetSTARTAGE(-1)
, m_colDetENDAGE(-1)
, m_colDetLAI(-1)

, m_colProbMINAGE(-1)
, m_colProbMAXAGE(-1)
, m_colProbTSD(-1)
, m_colProbRELATIVEAGE(-1)
, m_colProbKEEPRELAGE(-1)
, m_colProbPROPORTION(-1)
, m_colProbTSDMAX(-1)
, m_colProbRELTSD(-1)
, m_colProbPVTto(-1)
   ;

STMengine::~STMengine()
   {
   }

BOOL STMengine::Init(EnvContext *pEnvContext, LPCTSTR initStr)
{
   m_pEnvContext = pEnvContext;
   gIDUs = (IDUlayer *)pEnvContext->pMapLayer;

   // get input file names and input variables   
   if (m_staticsInitialized)
      return TRUE;

   m_staticsInitialized = true;

   clock_t start = clock();

   bool ok = LoadXml(initStr);

   clock_t finish = clock();
   double duration = (float)(finish - start) / CLOCKS_PER_SEC;
   if (!ok)
      {
      CString msg;
      msg = ("STMengine: Unable to find STMengine's .xml init file");

      Report::ErrorMsg(msg);
      return FALSE;
      }

   for (int i = 0; i < (int)m_outputArray.GetSize(); i++)
      {
      OUTPUT *pOutput = m_outputArray[i];
      AddOutputVar(pOutput->name, pOutput->value, "");
      }

   CString tempinistring;
   CString inputString(initStr);
   CString *pString = &inputString;


   // choose which climate change files to use if necessary
   // go to LoadProbCSV function for loading file into data object, filename set in .xml file
   start = clock();

   ok = LoadProbCSV(m_vegtransfile.probability_filename, pEnvContext);
   m_colCondNEW_STATE = m_condTransTable.GetCol("NEW_STATE");

   m_colProbMINAGE = m_inputtable.GetCol("MINAGE");
   m_colProbMAXAGE = m_inputtable.GetCol("MAXAGE");
   if (m_colProbMINAGE < 0 || m_colProbMAXAGE < 0 )
   {
      CString msg;
      msg.Format("STMengine::::Init() Missing column in probabilistic transition file %s", m_vegtransfile.probability_filename.GetString());
      Report::ErrorMsg(msg);
      return(false);
   }

   finish = clock();
   duration = (float)(finish - start) / CLOCKS_PER_SEC;
   CString msg;
   msg.Format("STMengine: Loaded Probability CSV file '%s' (%.2f seconds)", (LPCTSTR)m_vegtransfile.probability_filename, (float)duration);
   Report::LogMsg(msg);

   if (!ok)
      return FALSE;

   // go to LoadDeterministicTransCSV funtion for loading file into data object, filename set in .xml file
   start = clock();

   m_flagDeterministicFile = LoadDeterministicTransCSV(m_vegtransfile.deterministic_filename, pEnvContext);
   m_colDetCURR_STATE = m_detTransTable.GetCol("CURR_STATE");
   m_colDetNEW_STATE = m_detTransTable.GetCol("NEW_STATE");
   m_colDetSTARTAGE = m_detTransTable.GetCol("STARTAGE");
   m_colDetENDAGE = m_detTransTable.GetCol("ENDAGE");
   m_colDetLAI = m_detTransTable.GetCol("LAI");
   if (m_colDetCURR_STATE < 0 || m_colDetNEW_STATE || m_colDetSTARTAGE < 0 || m_colDetENDAGE < 0 || m_colDetLAI < 0)
   {
      CString msg;
      msg.Format("STMengine::::Init() Missing column in deterministic transition file %s: "
         "m_colDetCURR_STATE = %d, m_colDetNEW_STATE = %d, m_colDetSTARTAGE = %d, m_colDetENDAGE = %d, m_colDetLAI = %d",
         m_vegtransfile.deterministic_filename.GetString(), m_colDetCURR_STATE, m_colDetNEW_STATE, m_colDetSTARTAGE, m_colDetENDAGE, m_colDetLAI);
      Report::ErrorMsg(msg);
      return(false);
   }

   finish = clock();
   duration = (float)(finish - start) / CLOCKS_PER_SEC;
   msg.Format("STMengine: Loaded Deterministic Transition CSV file '%s' (%.2f seconds)", (LPCTSTR)m_vegtransfile.deterministic_filename, (float)duration);
   Report::LogMsg(msg);

   return(true);
   }


BOOL STMengine::InitRun(EnvContext *pEnvContext, bool useInitSeed)
{
   for (MapLayer::Iterator idu = pEnvContext->pMapLayer->Begin(); idu != pEnvContext->pMapLayer->End(); idu++)
   {
      int curr_state = AttInt(idu, VEGCLASS);
      if (curr_state < STM_CLASS_MIN || STM_CLASS_MAX < curr_state) continue;

      int stm_index = m_detTransTable.Find(m_colDetCURR_STATE, (VData)curr_state, 0);
      SetAttInt(idu, STM_INDEX, stm_index);
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
      // VEGTRNTYPE records the outcome.  When a transition does occur, updated values are written to:
      // VEGCLASS, PVT, AGECLASS, LAI, ...
      int curr_stm_ndx = AttInt(idu, STM_INDEX);
      int curr_age = AttInt(idu, AGECLASS);
      int new_state = -1;

      int new_stm_ndx = ConditionalTransition(idu, curr_stm_ndx, curr_age); 
      if (new_stm_ndx == curr_stm_ndx) new_stm_ndx = DeterministicTransition(idu, curr_stm_ndx, curr_age);

      if (new_stm_ndx != curr_stm_ndx)
      { // Transition to a new state. Update STM_INDEX, VEGCLASS, AGECLASS, and LAI.
         SetAttInt(idu, STM_INDEX, new_stm_ndx);

         int new_state = m_detTransTable.GetAsInt(m_colDetCURR_STATE, new_stm_ndx);
         UpdateIDU(pEnvContext, idu, VEGCLASS, new_state, true);

         int new_age = m_detTransTable.GetAsInt(m_colDetSTARTAGE, new_stm_ndx);
         SetAttInt(idu, AGECLASS, new_age);

         float new_lai = m_detTransTable.GetAsFloat(m_colDetLAI, new_lai);
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

   }


//x bool STMengine::ProbabilisticTransition(EnvContext* pEnvContext, int idu, int vegclass, int pvt, int disturb, int stand_age, int tsd, int& selected_to_trans, int& selected_pvtto_trans, float& selected_probability)
int STMengine::ConditionalTransition(EnvContext* pEnvContext, int idu, int tgtSourceClass, int ageclass, float& selected_probability)
{ // Returns index of selected conditional transition, or -1 if no conditional transition is selected.
   // 1. Find the entries in the conditional transition table for the source state.  If none, return -1.
   // 2. In the order the transitions appear in the table, choose whether or not to use the transition.
   //    Evaluate the likelihood of the transition.pick a random number from a uniform distribution 
   //    on the unit interval.  If the random number is <= the likelihood of the conditional transition,
   //    return the index of the transition.
   // 3. If no transition is selected, return -1.


   CArray<int> sourceIndices;
   int source_ndx = 0;
   int found_count = 0;
   int last_found = -1;
   while (source_ndx < m_condTransSourceStates.GetCount())
   {
      if (m_condTransSourceStates[source_ndx] == tgtSourceClass)
      {
         m_condTransSourceStates.Add(source_ndx);
         found_count++;
         last_found = source_ndx;
      }
      source_ndx++;
   }

   int ndx_to_possible_transitions = 0;
   while (ndx_to_possible_transitions < found_count)
   {

      ndx_to_possible_transitions++;
   }
   
   return(-1); // No transition was selected.
} // end of ConditionalTransition()

   m_probLookupKey.si = 0;
//x   m_probLookupKey.pvt = pvt;
   m_probLookupKey.from = vegclass;
   m_probLookupKey.regen = 0;
//x   m_probLookupKey.region = 1;
//x   m_probLookupKey.disturb = disturb;
   std::vector< std::pair<int, float> > * final_probs = NULL;
   final_probs = &probmap[m_probLookupKey]; // Returns the proper probabilities from the lookup table

   if (final_probs->size() == 0) return(false);
/*x
   {
      if (disturb == 0 || (CC_DISTURB_LB <= disturb && disturb <= CC_DISTURB_UB)) 
         return(false);

      bool alreadySeen = false;
      CString msg;
      msg.Format("%i %i %i\n", vegclass, disturb, pvt);

      int list_length = (int)m_badVegTransClasses.GetSize();
      int max_list_length = 1000;
      if (0 < list_length && list_length < max_list_length)
      { 
         for (int i = 0; i < list_length; i++) if (m_badVegTransClasses[i] == msg)
         {
            alreadySeen = true;
            break;
         }
      }   
      if (!alreadySeen && list_length < max_list_length) m_badVegTransClasses.Add(msg);
      return(false);
   } // end of if ( m_final_probs->size() == 0 )
x*/

   std::vector< std::pair<int, float> > * original_probs;
   original_probs = &probmap2[m_probLookupKey]; // Get a second copy of the probabilities from the lookup table

   float probability_sum = 0.f;
   for (vector<pair<int, float>>::iterator it = final_probs->begin(); it != final_probs->end(); ++it)
      probability_sum += it->second;

   /*x
   if (disturb != 0)
   { // Normalize probability list if disturbance. This means a disturbance transition will be selected.  
      for (vector<pair<int, float>>::iterator it = final_probs->begin(); it != final_probs->end(); ++it)
         it->second = probability_sum > 0.0 ? it->second / probability_sum : 1.0f / final_probs->size();
      probability_sum = 1.f;
   }
   x*/

   double rand_num = (double)m_rn.RandValue(0., 1.);  //randum number between 0 and 1, uniform distribution                                                     //normalize probability list if disturbance. this means a disturbance transition will be selected. 
   if (rand_num > probability_sum) return(false);

   // Begin Selecting probability transition based on VDDT algorithm  
   selected_to_trans = selected_pvtto_trans = -1;

   if (rand_num <= probability_sum) 
   {
      float orig_probability = 0.f;
      int candidate_new_state = ChooseProbTrans(rand_num, probability_sum, final_probs, original_probs, orig_probability);

      // Transition to the new state, but only if the time since disturbance requirement is met or irrelevant
      m_probIndexLookupKey.si = 0;
      m_probIndexLookupKey.pvt = pvt;
      m_probIndexLookupKey.from = vegclass; //transition "from" veg class
      m_probIndexLookupKey.to = candidate_new_state;
      m_probIndexLookupKey.regen = 0;
      m_probIndexLookupKey.prob = orig_probability;
      m_probIndexLookupKey.region = 1;
      m_probIndexLookupKey.disturb = disturb;
      vector<int> *probmapindex = NULL;
      probmapindex = &m_probIndexMap[m_probIndexLookupKey]; //getting index into prob file for the line that transition was select from, for age info etc..	
      if (probmapindex->size() <= 0)
      { // There are no probability transitions for this state class. 
            bool alreadySeen = false;
            CString msg;
            msg.Format("%i %i %i\n", selected_to_trans, disturb, pvt);

            int list_length = (int)m_badDeterminVegTransClasses.GetSize();
            int max_list_length = 1000;
            if (0 < list_length && list_length < max_list_length)
               for (int i = 0; i < list_length; i++)
               {

                  if (m_badDeterminVegTransClasses[i] == msg)
                  {
                     alreadySeen = true;
                     break;
                  }
               }
            if (!alreadySeen && list_length < max_list_length) m_badDeterminVegTransClasses.Add(msg);
            return(false);
      }

      // If this transition is the result of a disturbance, then the time-since-disturbance requirement is irrelevant.
      // Otherwise, if TSD < MINTSD, then don't do the transition now.
      if (disturb == 0)
         { 
         int mintsd = 0; m_inputtable.Get(m_colProbTSD, probmapindex->at(0), mintsd);
         int minage = 0; m_inputtable.Get(m_colProbMINAGE, probmapindex->at(0), minage);
         int maxage = 0; m_inputtable.Get(m_colProbMAXAGE, probmapindex->at(0), maxage);
         if (tsd < mintsd || stand_age < minage || stand_age > maxage) return(false); // Time-since-disturbance requirement was not met; no transition now.
         }

      // Do the transition now.
      selected_to_trans = candidate_new_state;
      int pvtto = 0; m_inputtable.Get(m_colProbPVTto, probmapindex->at(0), pvtto);
      selected_pvtto_trans = pvtto;
      selected_probability = orig_probability;
      return(true);
   }
   else return(false);

} // end of ConditionalTransition()


int STMengine::ChooseProbTrans(double rand_num, float probability_sum, vector<pair<int, float> > *m_final_probs, std::vector< std::pair<int, float> > *m_original_final_probs, float & orig_probability)
// Returns the vegclass of the pair chosen.
   {

   double
      sum_of_trans_prob;

   int
      i = 0;

   int
      PTindex = 0;

   sum_of_trans_prob = 0.0;

   vector<pair<int, float>>::iterator ir = m_final_probs->begin();

   vector<pair<int, float>>::iterator orig_ir = m_original_final_probs->begin();

   //get first value of the second item in vector pair which is the probability 
   sum_of_trans_prob += ir->second;

   //Begin the selection of class transition based on VDDT algorithym
   while (rand_num > sum_of_trans_prob && PTindex < m_final_probs->size())
      {
      if (m_final_probs->size() > 1)
         {

         ++ir;

         ++orig_ir;

         PTindex++;

         if (PTindex >= 0)
            {

            sum_of_trans_prob += ir->second;

            }
         }
      else
         {
         sum_of_trans_prob = sum_of_trans_prob + rand_num;// there is only one probablility, so end this while loop
         }

      } //end while

   orig_probability = orig_ir->second;
   return (ir->first);

   } // end of ChooseProbTrans()


static int errAgeClassCount = 0;

bool STMengine::DeterministicTransition(EnvContext *pEnvContext, int idu, int vegclass, int pvt, int ageclass, int & selected_to_trans, int & selected_pvtto_trans)
   {
   //build the key for the lookup table
   m_determinIndexLookupKey.region = 1;
   m_determinIndexLookupKey.from = vegclass;
   m_determinIndexLookupKey.pvt = pvt;
   vector<int> * determmapindex = &m_determinIndexMap[m_determinIndexLookupKey];

   if ( determmapindex->size() > 1 )
      {
      CString msg;
      msg.Format("STMengine: multiple entries for the same state in the deterministic transition table: vegclass_from=%i pvt=%i ",
         vegclass, pvt);
      Report::LogMsg(msg, RT_ERROR);
      }

   selected_to_trans = selected_pvtto_trans = -1;
   if (determmapindex->size() >= 1)
      {
      int endage;
      int endage_col = m_deterministic_inputtable.GetCol("ENDAGE");
      m_deterministic_inputtable.Get(endage_col, determmapindex->at(0), endage);
      if (ageclass >= endage)
         {
         if (ageclass > endage && errAgeClassCount < 10)
            {
            CString msg;
            msg.Format("STMengine::: ageclass > endage.  year=%i idu=%i, ageclass=%i, endage=%i, vegclass=%i, pvt=%i",
               pEnvContext->currentYear, idu, ageclass, endage, vegclass, pvt);
            Report::LogMsg(msg, RT_WARNING);

            errAgeClassCount++;
            if (errAgeClassCount == 10)
               {
               msg.Format("Further 'ageclass > endage' messages will be suppressed.");
               Report::LogMsg(msg, RT_WARNING);
               }
            }

         int vto_col = m_deterministic_inputtable.GetCol("VEGCLASSto");
         int pvtto_col = m_deterministic_inputtable.GetCol("PVTto");
         m_deterministic_inputtable.Get(vto_col, determmapindex->at(0), selected_to_trans);
         m_deterministic_inputtable.Get(pvtto_col, determmapindex->at(0), selected_pvtto_trans);
         }
      }

   return(selected_to_trans >= 0);
   } // end of DeterministicTransition()


bool STMengine::LoadDeterministicTransCSV(CString deterministicfilename, EnvContext *pEnvContext)
   {

   const MapLayer *pLayer = pEnvContext->pMapLayer;

   CString msg;
   int rndStandAge;
   int startStandAge;
   int endStandAge;
   int tabvegclass;
   int tabpvt;
   int tabregion;
   float tablai;
   // float tabcarbon;
   int totemp;
   int pvttotemp;
   
   int records = m_deterministic_inputtable.ReadAscii(deterministicfilename, ',', TRUE);

   if (records <= 0 || deterministicfilename == "")
      {
      msg.Format("STMengine::LoadDeterministicTransCSV could not load deterministic transition .csv file specified in PVT.xml file");
      Report::ErrorMsg(msg);
      return false;
      }

   //get column numbers in prob input table
   int region_col = m_deterministic_inputtable.GetCol("REGION");
   int vfrom_col = m_deterministic_inputtable.GetCol("VEGCLASSfrom");
   int vto_col = m_deterministic_inputtable.GetCol("VEGCLASSto");
   int abbvfrom_col = m_deterministic_inputtable.GetCol("ABBREVfrom");
   int abbvto_col = m_deterministic_inputtable.GetCol("ABBREVto");
   int pvt_col = m_deterministic_inputtable.GetCol("PVT");
   int pvtto_col = m_deterministic_inputtable.GetCol("PVTto");
   int startage_col = m_deterministic_inputtable.GetCol("STARTAGE");
   int endage_col = m_deterministic_inputtable.GetCol("ENDAGE");
   int rndage_col = m_deterministic_inputtable.GetCol("RNDAGE");

   if (vfrom_col < 0 || vto_col < 0 || abbvfrom_col < 0 || abbvto_col < 0 || pvtto_col < 0 ||
      pvt_col < 0 || startage_col < 0 || endage_col < 0 || rndage_col < 0 || region_col < 0)
      {
      msg.Format("STMengine::LoadDeterministicTransCSV One or more column headings are incorrect in deterministic lookup file");
      Report::ErrorMsg(msg);
      return false;
      }

   int tabrows = m_deterministic_inputtable.GetRowCount();

   // loop through look up table  
   for (int j = 0; j < tabrows; j++)
      {

      //Build key
      m_deterministic_inputtable.Get(vfrom_col, j, m_determinIndexInsertKey.from);
      m_deterministic_inputtable.Get(pvt_col, j, m_determinIndexInsertKey.pvt);
      m_deterministic_inputtable.Get(region_col, j, m_determinIndexInsertKey.region);

      //build map, this map is used once a stateclass transition has happened. j will be the index into this map
      //so values used in Age Class determination can be used.
      m_determinIndexMap[m_determinIndexInsertKey].push_back(j);

      //Building key
      m_deterministic_inputtable.Get(vfrom_col, j, m_deterministicInsertKey.from);
      m_deterministic_inputtable.Get(vto_col, j, totemp);
      m_deterministic_inputtable.Get(pvtto_col, j, pvttotemp);
      m_deterministic_inputtable.Get(region_col, j, m_deterministicInsertKey.region);
      m_deterministic_inputtable.Get(pvt_col, j, m_deterministicInsertKey.pvt);
      m_deterministic_inputtable.Get(endage_col, j, m_deterministicInsertKey.endage);

      // Building map   
      m_deterministic_trans[m_deterministicInsertKey].push_back(std::make_pair(totemp, pvttotemp));

      }

   int lai_col = -1;

   lai_col = m_deterministic_inputtable.GetCol("LAI");
	int tsd_col = m_inputtable.GetCol("TSD");
	int minAge_col = m_inputtable.GetCol("MINAGE");
	int maxAge_col = m_inputtable.GetCol("MAXAGE");

   //if necessary intialize attributes in IDU layer
   for (MapLayer::Iterator idu = pEnvContext->pMapLayer->Begin(); idu != pEnvContext->pMapLayer->End(); idu++)
      {
		int rndTSD = 0;
		int meanTSD = 0;
		int meanMinAge = 0;
		int meanMaxAge = 0;
		int n = 0;
		int sumTSD = 0;
		int sumMinAge = 0;
		int sumMaxAge = 0;

		vector<int> *tsdIndex = 0;

      int region = 1; 
      int pvt = 0; pLayer->GetData(idu, m_colPVT, pvt);
      int vegclass = 0; pLayer->GetData(idu, m_colVEGCLASS, vegclass);
		int tsd = 0; pLayer->GetData(idu, m_colTSD, tsd );
		int disturb = 0; pLayer->GetData(idu, m_colDisturb, disturb );

		// the value for DISTURB in the IDU layer may be a negative value (historic)
		// this value is used as part of key into the probability lookup table. For
		// successional tranitions the value should be zero, so set equal to zero for the key
		if ( disturb < 0 ) disturb = 0;
      
		// loop through look up table  
      for (int j = 0; j < tabrows; j++)
         {
         double rand_num = (double)m_rn.RandValue(0., 1.);

         m_deterministic_inputtable.Get(rndage_col, j, rndStandAge);
         m_deterministic_inputtable.Get(startage_col, j, startStandAge);
         m_deterministic_inputtable.Get(endage_col, j, endStandAge);
         m_deterministic_inputtable.Get(vfrom_col, j, tabvegclass);
         m_deterministic_inputtable.Get(pvt_col, j, tabpvt);
         m_deterministic_inputtable.Get(region_col, j, tabregion);
         m_deterministic_inputtable.Get(lai_col, j, tablai);

         if (vegclass == tabvegclass && pvt == tabpvt && region == tabregion)
            {

            rndStandAge = (int)floor(startStandAge + ((endStandAge - startStandAge) * (rand_num)));

            // If specified by switch in the input .xml file, use one of these methods to intialize TSD in the IDU layer
				switch ( m_initializer.initTSD )
					{

					case 1:

						rndTSD = (int)floor(startStandAge + ((endStandAge - startStandAge) * (rand_num))) - startStandAge;

						break;

					case 2:

						if ( tsd >= startStandAge && tsd <= endStandAge )
							continue;

						rndTSD = (int)floor(startStandAge + ((endStandAge - startStandAge) * (rand_num))) - startStandAge;

						break;
					
					case 3:

						//just for proper lookup key, these keys are for future use
						m_TSDIndexLookupKey.si = 0;

						//generate a lookup key from current variables in the IDU layer for the prob lookup table
						m_TSDIndexLookupKey.from = vegclass;
						m_TSDIndexLookupKey.regen = 0;
						m_TSDIndexLookupKey.region = 1;
						m_TSDIndexLookupKey.pvt = pvt;
						m_TSDIndexLookupKey.disturb = disturb; // should always be zero in Init.

						tsdIndex = &m_TSDIndexMap[m_TSDIndexLookupKey]; 

						n = (int) tsdIndex->size();
													
						if ( n > 0 )
							{
							int initVal = tsdIndex->at(0);
							int uniqVals = 0;
							for ( int i = 0; i < n; i++ )
								{
								sumTSD += m_inputtable.GetAsInt( tsd_col, tsdIndex->at(i) );
								sumMinAge += m_inputtable.GetAsInt(minAge_col, tsdIndex->at(i));
								sumMaxAge += m_inputtable.GetAsInt(maxAge_col, tsdIndex->at(i));
								
								// just need to see if at least one is not the same
								if ( initVal != tsdIndex->at(i) ) 
									uniqVals++;	
								}

							( sumTSD > 0 ) ? meanTSD = (int) floor (sumTSD / n) : meanTSD = 0;
							( sumMinAge > 0 ) ? meanMinAge = (int)floor(sumMinAge / n) : meanMinAge = 0;
							( sumMaxAge > 0 ) ? meanMaxAge = (int)floor(sumMaxAge / n) : meanMaxAge = 0;
													
							rndTSD = (int) floor( (double)m_rn.RandValue(0., (double) meanTSD ) );
								
							}
						else // no  prob successional transition found, use initial IDU layer value
							{
							rndTSD = tsd;
							}

						break;
					
					default:

						break;
					}

            if (rndTSD < 0)
               rndTSD = 0;

            // In the .xml file, if user wants STMengine to initialize TSD
            if ( m_initializer.initTSD > 0 )
               ((MapLayer *)(pEnvContext->pMapLayer))->SetData(idu, m_colTSD, rndTSD);

            // In the .xml file, if user wants STMengine to initialize AGECLASS
            if ( m_initializer.initAgeClass > 0 )
               ((MapLayer *)(pEnvContext->pMapLayer))->SetData(idu, m_colAGECLASS, rndStandAge);


            ((MapLayer *)(pEnvContext->pMapLayer))->SetData(idu, m_colLAI, tablai);

            break;
            }
         }
      }

   return true;

   }


bool STMengine::LoadProbCSV(CString probfilename, EnvContext *pEnvContext)
   {
   const MapLayer *pLayer = pEnvContext->pMapLayer;

   CString msg;
   int totemp;
   float probtemp;

   //read in the the data in the .csv lookup table. index zero is MC1 probability lookup table
   int records = m_inputtable.ReadAscii(probfilename, ',', TRUE);

   if (records <= 0)
      {
      msg.Format("STMengine: could not load probability .csv file specified in PVT.xml file");
      Report::ErrorMsg(msg);
      return false;
      }

   msg.Format("Dynamic Veg:   Loaded %i records from '%s'", records, (LPCTSTR)probfilename);
   Report::LogMsg(msg);

   //get column numbers in prob input table
   int vfrom_col = m_inputtable.GetCol("VEGCLASSfrom");
   int vto_col = m_inputtable.GetCol("VEGCLASSto");
   int abbvfrom_col = m_inputtable.GetCol("ABBREVfrom");
   int abbvto_col = m_inputtable.GetCol("ABBREVto");
   int futsi_col = m_inputtable.GetCol("FUTSI");
   int pvt_col = m_inputtable.GetCol("PVT");
   int regen_col = m_inputtable.GetCol("REGEN");
   int p_col = m_inputtable.GetCol("P");
   int propor_col = m_inputtable.GetCol("pxPropor");
   int region_col = m_inputtable.GetCol("REGION");
   int disturb_col = m_inputtable.GetCol("DISTURB");
   int probtype_col = m_inputtable.GetCol("VDDTProbType");
   int minage_col = m_inputtable.GetCol("MINAGE");
   int maxage_col = m_inputtable.GetCol("MAXAGE");
   int tsd_col = m_inputtable.GetCol("TSD");
   int relage_col = m_inputtable.GetCol("RELATIVEAGE");
   int keeprel_col = m_inputtable.GetCol("KEEPRELAGE");
   int proport_col = m_inputtable.GetCol("PROPORTION");
   int tsdmax_col = m_inputtable.GetCol("TSDMAX");
   int reltsd_col = m_inputtable.GetCol("RELTSD");
   int rndage_col = m_inputtable.GetCol("RNDAGE");

   if (vfrom_col < 0 || vto_col < 0 ||
      pvt_col < 0 || regen_col < 0 || p_col < 0 ||
      propor_col < 0 || region_col < 0 || disturb_col < 0 ||
      minage_col < 0 || maxage_col < 0 || tsd_col < 0 || relage_col < 0 ||
      keeprel_col < 0 || proport_col < 0 || tsdmax_col < 0 || reltsd_col < 0)
      {
      msg.Format("STMengine::LoadProbCSV One or more column headings are incorrect in Probability lookup file");
      Report::ErrorMsg(msg);
      return false;
      }

    int tabrows = m_inputtable.GetRowCount();

   Report::LogMsg("STMengine:   iterating through input table");

   // loop through look up table  
   for (int j = 0; j < tabrows; j++)
      {
      //Building key for probabilities
      m_inputtable.Get(vfrom_col, j, m_probInsertKey.from);
      m_inputtable.Get(vto_col, j, totemp);
      m_inputtable.Get(futsi_col, j, m_probInsertKey.si);
      m_inputtable.Get(pvt_col, j, m_probInsertKey.pvt);
      m_inputtable.Get(regen_col, j, m_probInsertKey.regen);
      m_inputtable.Get(p_col, j, probtemp);
      m_inputtable.Get(region_col, j, m_probInsertKey.region);
      m_inputtable.Get(disturb_col, j, m_probInsertKey.disturb);

      //Building map     
      probmap[m_probInsertKey].push_back(std::make_pair(totemp, probtemp));
      probmap2[m_probInsertKey].push_back(std::make_pair(totemp, probtemp));

      //Building key for index into map
      m_inputtable.Get(vfrom_col, j, m_probIndexInsertKey.from);
      m_inputtable.Get(vto_col, j, m_probIndexInsertKey.to);
      m_inputtable.Get(region_col, j, m_probIndexInsertKey.region);
      m_inputtable.Get(pvt_col, j, m_probIndexInsertKey.pvt);
      m_inputtable.Get(p_col, j, m_probIndexInsertKey.prob);
      m_inputtable.Get(disturb_col, j, m_probIndexInsertKey.disturb);
      m_inputtable.Get(futsi_col, j, m_probIndexInsertKey.si);
      m_inputtable.Get(regen_col, j, m_probIndexInsertKey.regen);

      //Building map, this map is used once a stateclass transition has happened. j will be the index into this map
      //so values used in Age Class determination can be used.
      m_probIndexMap[m_probIndexInsertKey].push_back(j);

		//Building key for TSDindex into map
		m_inputtable.Get(vfrom_col, j,  m_TSDIndexInsertKey.from);
		m_inputtable.Get(region_col, j, m_TSDIndexInsertKey.region);
		m_inputtable.Get(pvt_col, j,    m_TSDIndexInsertKey.pvt);
		m_inputtable.Get(disturb_col, j,m_TSDIndexInsertKey.disturb);
		m_inputtable.Get(futsi_col, j,  m_TSDIndexInsertKey.si);
		m_inputtable.Get(regen_col, j,  m_TSDIndexInsertKey.regen);

		// used to initialize TSD in IDU layer with TSD threshold in Probability lookup table
		m_TSDIndexMap[m_TSDIndexInsertKey].push_back(j);

      }

   return true;

   }


bool STMengine::LoadXml(LPCTSTR _filename)
   {
   CString filename;
   if ( PathManager::FindPath( _filename, filename ) < 0 ) //  return value: > 0 = success; < 0 = failure (file not found), 0 = path fully qualified and found 
      {
      CString msg;
      msg.Format( "Dynamic Veg: Input file '%s' not found - this process will be disabled", _filename );
      Report::ErrorMsg( msg );
      return false;
      }

   // start parsing input file
   TiXmlDocument doc;
   bool ok = doc.LoadFile(filename);

   bool loadSuccess = true;

   if (!ok)
      {
      CString msg;
      msg.Format("Error reading input file %s:  %s", filename, doc.ErrorDesc());
      Report::ErrorMsg(msg);
      return false;
      }

   // start interating through the nodes
   TiXmlElement *pXmlRoot = doc.RootElement();  // <integrator

   XML_ATTR setAttrpvt[] = {
         { "dynamic_update", TYPE_INT, &(m_dynamic_update.dynamic_update), true, 0 },
         { NULL, TYPE_NULL, NULL, false, 0 } };

   ok = TiXmlGetAttributes(pXmlRoot, setAttrpvt, filename, NULL);

   // get vegtrans file info next  *****************************************************************************
   TiXmlElement *pXmlVegtransfiles = pXmlRoot->FirstChildElement("vegtransfiles");

   if (pXmlVegtransfiles == NULL)
      {
      CString msg("Unable to find <vegtransfiles> tag reading ");
      msg += filename;
      Report::ErrorMsg(msg);
      return false;
      }

   TiXmlElement *pXmlVegtransfile = pXmlVegtransfiles->FirstChildElement("vegtransfile");

   LPCTSTR probFilename=NULL, detFilename=NULL, probMultFilename=NULL;

   XML_ATTR setAttrsveg[] = {
      // attr                      type           address              isReq  checkCol        
         { "probability_filename",    TYPE_STRING, &probFilename,     true, 0 },
         { "deterministic_filename",  TYPE_STRING, &detFilename,      true, 0 },
         { NULL, TYPE_NULL, NULL, false, 0 } };

   ok = TiXmlGetAttributes(pXmlVegtransfile, setAttrsveg, filename, NULL);

   int retVal = PathManager::FindPath( probFilename, m_vegtransfile.probability_filename );  //  return value: > 0 = success; < 0 = failure (file not found), 0 = path fully qualified and found 
   if ( retVal < 0 )
      {
      CString msg;
      msg.Format( "STMengine: Probability Transition file '%s' not found - Dynamic Veg will be disabled...", probFilename );
      Report::ErrorMsg( msg );
      return false;
      }

   retVal = PathManager::FindPath( detFilename, m_vegtransfile.deterministic_filename );  //  return value: > 0 = success; < 0 = failure (file not found), 0 = path fully qualified and found 
   if ( retVal < 0 )
      {
      CString msg;
      msg.Format( "STMengine: Deterministic Transition file '%s' not found - Dynamic Veg will be disabled...", detFilename );
      Report::ErrorMsg( msg );
      return false;
      }
   
   // get initializer info next  *****************************************************************************
   TiXmlElement *pXmlInitializers = pXmlRoot->FirstChildElement("initializers");

   if (pXmlInitializers == NULL)
      {
      CString msg("Unable to find <initializers> tag reading ");
      msg += filename;
      Report::ErrorMsg(msg);
      return false;
      }

   TiXmlElement *pXmlInitializer = pXmlInitializers->FirstChildElement("initializer");

   XML_ATTR setAttrsini[] = {
      // attr                     type           address                            isReq  checkCol        
         { "age_class_initializer", TYPE_INT, &(m_initializer.initAgeClass), true, 0 },
         { "tsd_initializer", TYPE_INT, &(m_initializer.initTSD), true, 0 },
         { NULL, TYPE_NULL, NULL, false, 0 } };

   ok = TiXmlGetAttributes(pXmlInitializer, setAttrsini, filename, NULL);

   // get Outputs next ******************************************************************************************
   TiXmlElement *pXmlOutputs = pXmlRoot->FirstChildElement("outputs");

   if ( pXmlOutputs != NULL)
      {
      TiXmlElement *pXmlOutput = pXmlOutputs->FirstChildElement("output");

      while (pXmlOutput != NULL)
         {
         // you need some stucture to store these in
         OUTPUT *pOutput = new OUTPUT;
         XML_ATTR setAttrs[] = {
            // attr                    type          address                 isReq  checkCol
               { "name", TYPE_CSTRING, &(pOutput->name), true, 0 },
               { "query", TYPE_CSTRING, &(pOutput->query), true, 0 },
               { NULL, TYPE_NULL, NULL, false, 0 } };

         ok = TiXmlGetAttributes(pXmlOutput, setAttrs, filename, NULL);

         if (!ok)
            {
            CString msg;
            msg.Format(_T("Misformed element reading <output> attributes in input file %s"), filename);
            Report::ErrorMsg(msg);
            delete pOutput;
            }
         else
            {
            m_outputArray.Add(pOutput);
            }

         pXmlOutput = pXmlOutput->NextSiblingElement("output");
         }
      }

   return true;
   }


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

