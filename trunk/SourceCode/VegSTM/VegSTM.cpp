// VegSTM.cpp 
// James Sulzman wrote a state-and-transition engine for the WW2100 project called DynamicVeg.
// Dave Conklin simplified and extensively rewrote DynamicVeg for the OUWIN and INFEWS projects, and renamed it VegSTM.

#include "stdafx.h"
#pragma hdrstop

#include <PathManager.h>
#include <iostream>
#include "VegSTM.h"
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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

PtrArray< OUTPUT > VegSTM::m_outputArray;
VEGTRANSFILE VegSTM::m_vegtransfile;

INITIALIZER VegSTM::m_initializer;

PVT VegSTM::m_dynamic_update;

RandUniform   VegSTM::m_rn;

bool VegSTM::m_staticsInitialized = false;

VDataObj VegSTM::m_inputtable;
VDataObj VegSTM::m_deterministic_inputtable;
int VegSTM::m_colVEGCLASS = -1;
int VegSTM::m_colDisturb = -1;
int VegSTM::m_colAGECLASS = -1;
int VegSTM::m_colPVT = -1;
int VegSTM::m_colLAI = -1;
int VegSTM::m_colCarbon = -1;
int VegSTM::m_colTSD = -1;
int VegSTM::m_colManage = -1;
int VegSTM::m_colRegion = -1;
int VegSTM::m_colCalcStandAge = -1;
int VegSTM::m_mc1_output = -1;
int VegSTM::m_cursi = -1;
int VegSTM::m_futsi = -1;
int VegSTM::m_regen = 0;
int VegSTM::m_variant = -1;
int VegSTM::m_mc1row = -1;
int VegSTM::m_mc1col = -1;
int VegSTM::m_mc1pvt = -1;
int VegSTM::m_mc1Cell = -1;
int VegSTM::m_manage = -1;

float VegSTM::m_carbon_conversion_factor = 8.89563e-5f;
bool VegSTM::m_validFlag = false;
bool VegSTM::m_useProbMultiplier = false;
bool VegSTM::m_flagDeterministicFile = false;
bool VegSTM::m_pvtProbMultiplier = false;
bool VegSTM::m_vegClassProbMultipier = false;

vector<vector<vector<int> > > VegSTM::m_vpvt;  //3d vector to hold MC1 pvt data [year,row,column]

vector<vector<vector<float> > > VegSTM::m_vsi;  //3d vector to hold MC1 si data [year,row,column]

map<ProbKeyclass, std::vector< std::pair<int, float> >, probclasscomp> VegSTM::probmap;

map<ProbKeyclass, std::vector< std::pair<int, float> >, probclasscomp> VegSTM::probmap2;

map<ProbMultiplierPVTKeyclass, std::vector<float>, ProbMultiplierPVTClassComp> VegSTM::m_probMultiplierPVTMap;

map<ProbMultiplierVegClassKeyclass, std::vector<float>, ProbMultiplierVegClassClassComp> VegSTM::m_probMultiplierVegClassMap;

map<ProbIndexKeyclass, std::vector<int>, ProbIndexClassComp> VegSTM::m_probIndexMap;

map<TSDIndexKeyclass, std::vector<int>, TSDIndexClassComp> VegSTM::m_TSDIndexMap;

map<DeterminIndexKeyClass, std::vector<int>, DeterminIndexClassComp> VegSTM::m_determinIndexMap;

map<DeterministicKeyclass, std::vector< std::pair<int, int> >, deterministicclasscomp> VegSTM::m_deterministic_trans;

ProbKeyclass VegSTM::m_probInsertKey, VegSTM::m_probLookupKey;

DeterministicKeyclass VegSTM::m_deterministicInsertKey, VegSTM::m_deterministicLookupKey;

ProbIndexKeyclass VegSTM::m_probIndexInsertKey, VegSTM::m_probIndexLookupKey;

TSDIndexKeyclass VegSTM::m_TSDIndexInsertKey, VegSTM::m_TSDIndexLookupKey;

DeterminIndexKeyClass VegSTM::m_determinIndexInsertKey, VegSTM::m_determinIndexLookupKey;

ProbMultiplierPVTKeyclass VegSTM::m_probMultiplierPVTInsertKey, VegSTM::m_probMultiplierPVTLookupKey;

ProbMultiplierVegClassKeyclass  VegSTM::m_probMultiplierVegClassInsertKey, VegSTM::m_probMultiplierVegClassLookupKey;

CArray< CString, CString > VegSTM::m_badVegTransClasses;

CArray< CString, CString > VegSTM::m_badDeterminVegTransClasses;

VegSTM::VegSTM()
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
   {
   };

VegSTM::~VegSTM()
   {
   }

BOOL VegSTM::Init(EnvContext *pEnvContext, LPCTSTR initStr)
   {
   m_pIDUlayer = (MapLayer *)pEnvContext->pMapLayer;

   // get input file names and input variables   
   if (m_staticsInitialized)
      return TRUE;

   m_staticsInitialized = true;

   clock_t start = clock();

   bool ok = LoadXml(initStr);

   clock_t finish = clock();
   double duration = (float)(finish - start) / CLOCKS_PER_SEC;
   CString msg;
   //msg.Format( "VegSTM: Loaded inputs file (%.2f seconds)", (float) duration );
   //Report::LogMsg( msg );

   if (!ok)
      {
      msg = ("VegSTM: Unable to find VegSTM's .xml init file");

      Report::ErrorMsg(msg);
      return FALSE;
      }

   CheckCol(m_pIDUlayer, m_colVEGTRNTYPE, "VEGTRNTYPE", TYPE_INT, CC_AUTOADD);

   for (int i = 0; i < (int)m_outputArray.GetSize(); i++)
      {
      OUTPUT *pOutput = m_outputArray[i];
      AddOutputVar(pOutput->name, pOutput->value, "");
      }

   CString tempinistring;
   CString inputString(initStr);
   CString *pString = &inputString;


   // choose which climate change files to use if necessary

   // check and store relevant columns
   CheckCol(m_pIDUlayer, m_colVEGCLASS, "VEGCLASS", TYPE_INT, CC_MUST_EXIST);
   CheckCol(m_pIDUlayer, m_colDisturb, "DISTURB", TYPE_INT, CC_MUST_EXIST);
   CheckCol(m_pIDUlayer, m_colPVT, "PVT", TYPE_INT, CC_MUST_EXIST);
   CheckCol(m_pIDUlayer, m_colTSD, "TSD", TYPE_INT, CC_MUST_EXIST);
   CheckCol(m_pIDUlayer, m_colRegion, "REGION", TYPE_INT, CC_MUST_EXIST);
   CheckCol(m_pIDUlayer, m_colAGECLASS, "AGECLASS", TYPE_INT, CC_MUST_EXIST);

   m_colLAI = m_pIDUlayer->GetFieldCol("LAI");
   m_colCarbon = m_pIDUlayer->GetFieldCol("FORESTC");

   // go to LoadProbCSV function for loading file into data object, filename set in .xml file
   start = clock();

   ok = LoadProbCSV(m_vegtransfile.probability_filename, pEnvContext);
   m_colProbMINAGE = m_inputtable.GetCol("MINAGE");
   m_colProbMAXAGE = m_inputtable.GetCol("MAXAGE");
   m_colProbTSD = m_inputtable.GetCol("TSD");
   m_colProbRELATIVEAGE = m_inputtable.GetCol("RELATIVEAGE");
   m_colProbKEEPRELAGE = m_inputtable.GetCol("KEEPRELAGE");
   m_colProbPROPORTION = m_inputtable.GetCol("PROPORTION");
   m_colProbTSDMAX = m_inputtable.GetCol("TSDMAX");
   m_colProbRELTSD = m_inputtable.GetCol("RELTSD");
   m_colProbPVTto = m_inputtable.GetCol("PVTto");
   if (m_colProbMINAGE < 0 || m_colProbMAXAGE < 0 || m_colProbTSD < 0 || m_colProbRELATIVEAGE < 0 || m_colProbKEEPRELAGE < 0
      || m_colProbPROPORTION < 0 || m_colProbTSDMAX < 0 || m_colProbRELTSD < 0 || m_colProbPVTto < 0)
   {
      CString msg;
      msg.Format("VegSTM::::Init() Missing column in probabilistic transition file %s", m_vegtransfile.probability_filename);
      Report::ErrorMsg(msg);
      return(false);
   }

   finish = clock();
   duration = (float)(finish - start) / CLOCKS_PER_SEC;
   msg.Format("VegSTM: Loaded Probability CSV file '%s' (%.2f seconds)", (LPCTSTR)m_vegtransfile.probability_filename, (float)duration);
   Report::LogMsg(msg);

   if (!ok)
      return FALSE;

   // go to LoadDeterministicTransCSV funtion for loading file into data object, filename set in .xml file
   start = clock();

   m_flagDeterministicFile = LoadDeterministicTransCSV(m_vegtransfile.deterministic_filename, pEnvContext);
   m_colDetSTARTAGE = m_deterministic_inputtable.GetCol("STARTAGE");
   m_colDetENDAGE = m_deterministic_inputtable.GetCol("ENDAGE");
   m_colDetLAI = m_deterministic_inputtable.GetCol("LAI");
   if (m_colDetSTARTAGE < 0 || m_colDetENDAGE < 0 || m_colDetLAI < 0)
   {
      CString msg;
      msg.Format("VegSTM::::Init() Missing column in deterministic transition file %s: m_colDetSTARTAGE = %d, m_colDetENDAGE = %d, m_colDetLAI = %d",
         m_vegtransfile.deterministic_filename, m_colDetSTARTAGE, m_colDetENDAGE, m_colDetLAI);
      Report::ErrorMsg(msg);
      return(false);
   }

   finish = clock();
   duration = (float)(finish - start) / CLOCKS_PER_SEC;
   msg.Format("VegSTM: Loaded Deterministic Transition CSV file '%s' (%.2f seconds)", (LPCTSTR)m_vegtransfile.deterministic_filename, (float)duration);
   Report::LogMsg(msg);

   return TRUE;
   }


BOOL VegSTM::InitRun(EnvContext *pEnvContext, bool useInitSeed)
   {
/*
   CString msg;
   msg.Format("VegSTM::InitRun() pEnvContext->id = %d", pEnvContext->id);
   Report::LogMsg(msg);
*/
   return(TRUE);
   }


// VEGTRNTYPE gets reset to TRANS_NONE (0) near the beginning of the annual step by a Modeler autonomous process.
BOOL VegSTM::Run(EnvContext *pEnvContext)
   {
   bool err_flag = false;
   m_probMultiplierPVTLookupKey.timestep = 
      m_probMultiplierVegClassLookupKey.timestep = pEnvContext->currentYear;

   for (MapLayer::Iterator idu = pEnvContext->pMapLayer->Begin(); idu != pEnvContext->pMapLayer->End(); idu++)
      {
      int idu_int = (int)idu; 
      int vegclass = -1; m_pIDUlayer->GetData(idu, m_colVEGCLASS, vegclass);
      if (vegclass < STM_VEGCLASS_MINIMUM) continue;

      int disturb = 0; m_pIDUlayer->GetData(idu, m_colDisturb, disturb);
      int pvt = 0; m_pIDUlayer->GetData(idu, m_colPVT, pvt);
      int tsd = 0; m_pIDUlayer->GetData(idu, m_colTSD, tsd);
      int ageclass = 0; m_pIDUlayer->GetData(idu, m_colAGECLASS, ageclass);

      //int cover_type = (int)floor(vegclass / 10000.0); //first three digits of VEGCLASS starting from left
      //int structural_stage = (int)floor(abs(((10000.0*cover_type) - vegclass) / 10.0)); //second three digits of VEGCLASS starting from left
      //int year_index = pEnvContext->yearOfRun;

      int selected_to_trans = -1;
      int selected_pvtto_trans = -1;
      float selected_probability = 0.f;

      // A disturbance (DISTURB > 0) will be handled in the logic for a probabilistic transition.
      // Check first for disturbance and probabilistic transitions, and then for deterministic transitions (aging out of 
      // one class into the next).  Finally, if no transitions occur, simply increment the stand age (AGECLASS).
      // VEGTRNTYPE records the outcome.  When a transition does occur, updated values are written to:
      // VEGCLASS, PVT, AGECLASS, LAI, ...
      int vegtrntype = TRANS_NONE;
      float new_lai = 0.f;
      int new_stand_age = 0;
      int new_tsd = 0;

      if (ProbabilisticTransition(pEnvContext, idu, vegclass, pvt, disturb, ageclass, tsd, selected_to_trans, selected_pvtto_trans, selected_probability))
         { // Transition to the new state.
         m_probIndexLookupKey.si = 0;
         m_probIndexLookupKey.pvt = pvt;
         m_probIndexLookupKey.from = vegclass; //transition "from" veg class
         m_probIndexLookupKey.to = selected_to_trans;
         m_probIndexLookupKey.regen = 0;
         m_probIndexLookupKey.prob = 0;
         m_probIndexLookupKey.region = 1;
         m_probIndexLookupKey.disturb = disturb;
         vector<int> *probmapindex = NULL;
         probmapindex = &m_probIndexMap[m_probIndexLookupKey]; //getting index into prob file for the line that transition was select from, for age info etc..	

         // int minage = 0; 
         // int maxage = 0; 
         // int mintsd = 0; 
         int relativeAge = 0;  
         int keepRelativeAge = 0; 
         // float proportion = 1.f; 
         // int tsdMax = 9999; 
         int relativeTsd = 0; 

         if (probmapindex->size() > 0)
            { 
            ASSERT(probmapindex->size() == 1);
            // m_inputtable.Get(m_colProbMINAGE, probmapindex->at(0), minage);
            // m_inputtable.Get(m_colProbMAXAGE, probmapindex->at(0), maxage);
            // m_inputtable.Get(m_colProbTSD, probmapindex->at(0), mintsd);
            m_inputtable.Get(m_colProbRELATIVEAGE, probmapindex->at(0), relativeAge); // used
            m_inputtable.Get(m_colProbKEEPRELAGE, probmapindex->at(0), keepRelativeAge); // 1=true // used
            // m_inputtable.Get(m_colProbPROPORTION, probmapindex->at(0), proportion);
            // m_inputtable.Get(m_colProbTSDMAX, probmapindex->at(0), tsdMax);
            m_inputtable.Get(m_colProbRELTSD, probmapindex->at(0), relativeTsd);
            }

         m_determinIndexLookupKey.region = 1;
         m_determinIndexLookupKey.from = selected_to_trans; //spot in table where the to is now the from for new age class
         m_determinIndexLookupKey.pvt = selected_pvtto_trans;
         vector<int> *determmapindex = NULL;
         determmapindex = &m_determinIndexMap[m_determinIndexLookupKey];
         ASSERT(determmapindex != NULL && determmapindex->size() > 0);

         int row = determmapindex->at(0);
         int startage = 0; m_deterministic_inputtable.Get(m_colDetSTARTAGE, row, startage);
         int endage = 0; m_deterministic_inputtable.Get(m_colDetENDAGE, row, endage);
         m_deterministic_inputtable.Get(m_colDetLAI, row, new_lai);

         if (keepRelativeAge == 1 || (selected_to_trans == vegclass && selected_pvtto_trans == pvt))
            // "keepRelativeAge" flag is set or transitions back to the same state => preserve relative age within the state
            new_stand_age = ageclass + relativeAge;
         else
            new_stand_age = startage + relativeAge; //relative age can be negative, holding a VEGCLASS longer in said class with respect to deterministic transitions.	

         if (new_stand_age > endage) new_stand_age = endage;
         else if (new_stand_age < startage) new_stand_age = startage;
 
         new_tsd = (relativeTsd != -9999) ? tsd + relativeTsd : 0;

         vegtrntype = disturb > 0 ? TRANS_DISTURBANCE : TRANS_PROBABILISTIC;
         UpdateIDU(pEnvContext, idu, m_colVEGCLASS, selected_to_trans, true);
         UpdateIDU(pEnvContext, idu, m_colPVT, selected_pvtto_trans, true);
         UpdateIDU(pEnvContext, idu, m_colAGECLASS, new_stand_age, true);
         UpdateIDU(pEnvContext, idu, m_colTSD, new_tsd, true);
         UpdateIDU(pEnvContext, idu, m_colLAI, new_lai, true);
         } // end of if (ProbabilisticTransition())

      if (vegtrntype == TRANS_NONE && DeterministicTransition(pEnvContext, idu, vegclass, pvt, ageclass, selected_to_trans, selected_pvtto_trans))
         {
         //getting index into deterministic file for the line that was transitioned to (now the from). for new age value
         m_determinIndexLookupKey.region = 1;
         m_determinIndexLookupKey.from = selected_to_trans; //spot in table where the to is now the from for new age class
         m_determinIndexLookupKey.pvt = selected_pvtto_trans;
         vector<int> *determmapindex = NULL;
         determmapindex = &m_determinIndexMap[m_determinIndexLookupKey];
         if (determmapindex != NULL && determmapindex->size() > 0)
            {
            int row = determmapindex->at(0);
            int startage = 0; m_deterministic_inputtable.Get(m_colDetSTARTAGE, row, startage);
            float new_lai = 0.f; m_deterministic_inputtable.Get(m_colDetLAI, row, new_lai);

            vegtrntype = TRANS_DETERMINISTIC;
            UpdateIDU(pEnvContext, idu, m_colVEGCLASS, selected_to_trans, true);
            UpdateIDU(pEnvContext, idu, m_colPVT, selected_pvtto_trans, true);
            UpdateIDU(pEnvContext, idu, m_colAGECLASS, startage, true);
            UpdateIDU(pEnvContext, idu, m_colLAI, new_lai, true);
         }
         else if (determmapindex == NULL)
            {
            bool alreadySeen = false;
            CString det_err_msg;
            det_err_msg.Format("%i %i %i\n", selected_to_trans, disturb, pvt);
            int list_length = (int)m_badDeterminVegTransClasses.GetSize();
            int max_list_length = 1000;
            if (0 < list_length && list_length < max_list_length)
               for (int i = 0; i < list_length; i++)
               {
                  if (m_badDeterminVegTransClasses[i] == det_err_msg)
                  {
                     alreadySeen = true;
                     break;
                  }
               }
            if (!alreadySeen && list_length < max_list_length) m_badDeterminVegTransClasses.Add(det_err_msg);
            err_flag = true;
            }
         else 
            { // determmapindex->size() == 0
            CString msg;
            msg.Format("VegSTM::Run() missing STM in Deterministic lookup .csv file: vegclass=%i, disturb=%i, pvt=%i ", selected_to_trans, disturb, pvt);
            Report::ErrorMsg(msg); err_flag = true;
            }
         } // end of if (vegtrntype == TRANS_NONE && DeterministicTransition())
      
      if (vegtrntype != TRANS_NONE) UpdateIDU(pEnvContext, idu, m_colVEGTRNTYPE, vegtrntype, true);
      else 
         { // No state transition this time.  Just increment the stand age and the time since disturbance.
         UpdateIDU(pEnvContext, idu, m_colAGECLASS, ageclass + 1, true);
         UpdateIDU(pEnvContext, idu, m_colTSD, tsd + 1, true);
         }

   } // end of IDU loop
       
   return(!err_flag);

   } // end of VegSTM::Run()


BOOL VegSTM::EndRun(EnvContext*)
   {
   if (m_badVegTransClasses.GetSize() == 0)
      return TRUE;

   int count = (int)m_badVegTransClasses.GetSize();
   CString msg;
   msg.Format("VegSTM:  No probabilistic transitions were found for the following %d veg classes: \n"
      "m_vegClass,m_disturb,m_regen,m_si,m_pvt,m_region\n", count);

   for (int i = 0; i <= m_badVegTransClasses.GetUpperBound(); i++)
      {
      msg += "VegSTM missing probabilistic transition: ";
      msg += m_badVegTransClasses[i];
      }

   Report::LogMsg(msg);

   m_badVegTransClasses.RemoveAll();

   if (m_badDeterminVegTransClasses.GetSize() == 0)
      return TRUE;

   count = (int)m_badDeterminVegTransClasses.GetSize();

   msg.Format("VegSTM:  No deterministic transitions were found for the following %d veg classes: \n"
      "m_vegClass,m_disturb,m_regen,m_si,m_pvt,m_region\n", count);

   for (int i = 0; i <= m_badDeterminVegTransClasses.GetUpperBound(); i++)
      {
      msg += "VegSTM missing deterministic transition: ";
      msg += m_badDeterminVegTransClasses[i];
      }

   Report::LogMsg(msg);

   m_badDeterminVegTransClasses.RemoveAll();

   return TRUE;

   }


bool VegSTM::ProbabilisticTransition(EnvContext * pEnvContext, int idu, int vegclass, int pvt, int disturb, int stand_age, int tsd, int & selected_to_trans, int & selected_pvtto_trans, float & selected_probability)
{
   m_probLookupKey.si = 0;
   m_probLookupKey.pvt = pvt;
   m_probLookupKey.from = vegclass;
   m_probLookupKey.regen = 0;
   m_probLookupKey.region = 1;
   m_probLookupKey.disturb = disturb;
   std::vector< std::pair<int, float> > * final_probs = NULL;
   final_probs = &probmap[m_probLookupKey]; // Returns the proper probabilities from the lookup table

   if (final_probs->size() == 0)
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

   std::vector< std::pair<int, float> > * original_probs;
   original_probs = &probmap2[m_probLookupKey]; // Get a second copy of the probabilities from the lookup table

   float probability_sum = 0.f;
   for (vector<pair<int, float>>::iterator it = final_probs->begin(); it != final_probs->end(); ++it)
      probability_sum += it->second;

   if (disturb != 0)
   { // Normalize probability list if disturbance. This means a disturbance transition will be selected.  
      for (vector<pair<int, float>>::iterator it = final_probs->begin(); it != final_probs->end(); ++it)
         it->second = probability_sum > 0.0 ? it->second / probability_sum : 1.0f / final_probs->size();
      probability_sum = 1.f;
   }

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

} // end of ProbabilisticTransition()


int VegSTM::ChooseProbTrans(double rand_num, float probability_sum, vector<pair<int, float> > *m_final_probs, std::vector< std::pair<int, float> > *m_original_final_probs, float & orig_probability)
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

bool VegSTM::DeterministicTransition(EnvContext *pEnvContext, int idu, int vegclass, int pvt, int ageclass, int & selected_to_trans, int & selected_pvtto_trans)
   {
   //build the key for the lookup table
   m_determinIndexLookupKey.region = 1;
   m_determinIndexLookupKey.from = vegclass;
   m_determinIndexLookupKey.pvt = pvt;
   vector<int> * determmapindex = &m_determinIndexMap[m_determinIndexLookupKey];

   if ( determmapindex->size() > 1 )
      {
      CString msg;
      msg.Format("VegSTM: multiple entries for the same state in the deterministic transition table: vegclass_from=%i pvt=%i ",
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
            msg.Format("VegSTM::: ageclass > endage.  year=%i idu=%i, ageclass=%i, endage=%i, vegclass=%i, pvt=%i",
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


bool VegSTM::LoadDeterministicTransCSV(CString deterministicfilename, EnvContext *pEnvContext)
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
      msg.Format("VegSTM::LoadDeterministicTransCSV could not load deterministic transition .csv file specified in PVT.xml file");
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
      msg.Format("VegSTM::LoadDeterministicTransCSV One or more column headings are incorrect in deterministic lookup file");
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

            // In the .xml file, if user wants VegSTM to initialize TSD
            if ( m_initializer.initTSD > 0 )
               ((MapLayer *)(pEnvContext->pMapLayer))->SetData(idu, m_colTSD, rndTSD);

            // In the .xml file, if user wants VegSTM to initialize AGECLASS
            if ( m_initializer.initAgeClass > 0 )
               ((MapLayer *)(pEnvContext->pMapLayer))->SetData(idu, m_colAGECLASS, rndStandAge);


            ((MapLayer *)(pEnvContext->pMapLayer))->SetData(idu, m_colLAI, tablai);

            break;
            }
         }
      }

   return true;

   }


bool VegSTM::LoadProbCSV(CString probfilename, EnvContext *pEnvContext)
   {
   const MapLayer *pLayer = pEnvContext->pMapLayer;

   CString msg;
   int totemp;
   float probtemp;

   //read in the the data in the .csv lookup table. index zero is MC1 probability lookup table
   int records = m_inputtable.ReadAscii(probfilename, ',', TRUE);

   if (records <= 0)
      {
      msg.Format("VegSTM: could not load probability .csv file specified in PVT.xml file");
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
      msg.Format("VegSTM::LoadProbCSV One or more column headings are incorrect in Probability lookup file");
      Report::ErrorMsg(msg);
      return false;
      }

    int tabrows = m_inputtable.GetRowCount();

   Report::LogMsg("VegSTM:   iterating through input table");

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


bool VegSTM::LoadXml(LPCTSTR _filename)
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
      msg.Format( "VegSTM: Probability Transition file '%s' not found - Dynamic Veg will be disabled...", probFilename );
      Report::ErrorMsg( msg );
      return false;
      }

   retVal = PathManager::FindPath( detFilename, m_vegtransfile.deterministic_filename );  //  return value: > 0 = success; < 0 = failure (file not found), 0 = path fully qualified and found 
   if ( retVal < 0 )
      {
      CString msg;
      msg.Format( "VegSTM: Deterministic Transition file '%s' not found - Dynamic Veg will be disabled...", detFilename );
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
