// EconModels.cpp 
//

#include "stdafx.h"
#include "APs.h"
#include <EnvEngine\EnvConstants.h>
#include <Maplayer.h>
#include <Map.h>
#include <math.h>
#include <UNITCONV.H>
#include <path.h>
#include <EnvEngine\EnvModel.h>
#include <DeltaArray.h>
#include <EnvInterface.h>
#include <direct.h>
#include <random>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BOOL APs::GetUrbanPopData()
// Called at InitRun
   {
   if (m_records_UPP>0)
      {
      ASSERT(m_colUPP_UGB_DH_ID>=0 && m_colUPP_UGB_name>=0 && m_colUPP_pop2010>=0);
      return true;
      }

   CString msg;
   bool err;

   CString pathAndFileName;
   pathAndFileName = ReplaceSubstring(m_UGApopProj_file, "POPULATION_SCENARIO_NAME", m_popScenarioName);
   m_records_UPP = m_UGApopProj_table.ReadAscii(pathAndFileName, ',', TRUE);

   err = m_records_UPP<=0;
   if (!err)
      {
      m_colUPP_UGB_DH_ID = m_UGApopProj_table.GetCol("UGB_DH_ID"); err |= m_colUPP_UGB_DH_ID<0; // ASSERT(!err);
      m_colUPP_UGB_name = m_UGApopProj_table.GetCol("UGB_name"); err |= m_colUPP_UGB_name<0; // ASSERT(!err);
      m_colUPP_pop2010 = m_UGApopProj_table.GetCol("pop2010"); err |= m_colUPP_pop2010<0; // ASSERT(!err);
      }
   if (err)
      {
      if (m_records_UPP<=0) msg.Format("APs_PopGrowth: missing or empty UGApop_file\n"
         "records_UPP = %d\n", m_records_UPP);
      else msg.Format("APs:popGrowth - One or more missing columns in the UGBpop file.\n"
         "m_colUPP_UGB_DH_ID, m_colUPP_UGB_name, m_colUPP_pop2010 = %d, %d, %d",
         m_colUPP_UGB_DH_ID, m_colUPP_UGB_name, m_colUPP_pop2010);
      Report::ErrorMsg(msg);
      return false;
      }

   return true;
   } // end of GetUrbanPopData()


BOOL APs::InitRunPG(EnvContext *pContext)
   {
   CString msg;
   bool err;

      {
      CString msg; msg.Format("*** InitRunPG(): m_currentPopScenarioIndex = %d", m_currentPopScenarioIndex);
      Report::LogMsg(msg);
      }

   InitPopDensCalcs( pContext );

   // Read the population projection data.
   err = !GetUrbanPopData();
   if (err) return(false);

   CString pathAndFileName;
   pathAndFileName = ReplaceSubstring(m_RPApopProj_file, "POPULATION_SCENARIO_NAME", m_popScenarioName);
   m_records_RPP = m_RPApopProj_table.ReadAscii(pathAndFileName, ',', TRUE);
   err = m_records_RPP<=0;
   if (!err)
      {
      m_colRPP_RR_DH_ID = m_RPApopProj_table.GetCol("RR_DH_ID"); err |= m_colRPP_RR_DH_ID<0; // ASSERT(!err);
      m_colRPP_pop2010 = m_RPApopProj_table.GetCol("pop2010"); err |= m_colRPP_pop2010<0; // ASSERT(!err);
      }
   if (err)
      {
      if (m_records_RPP<=0) msg.Format("APs_PopGrowth: missing or empty RRpop_file\n"
            "records_RPP = %d\n", m_records_RPP);
      else msg.Format("APs:popGrowth - One or more missing columns in the RRpop file.\n"
            "m_colRPP_RR_DH_ID, m_colRPP_pop2010 = %d, %d", 
            m_colRPP_RR_DH_ID, m_colRPP_pop2010);
      Report::ErrorMsg(msg);
      return false;
      }

   m_PGoutVars.ClearRows();
   CalculatePopGrowthOutputVariables(pContext);

   return true;
   } // end of InitRunPG()


BOOL APs::InitRunLandTrans(EnvContext *pContext)
   {

      {
      CString msg; msg.Format("*** InitRunLandTrans(): m_currentPopScenarioIndex = %d", m_currentPopScenarioIndex);
      Report::LogMsg(msg);
      }

   // Initialize output variables.
   m_LTag2devCount = m_LTag2forestCount = m_LTforest2devCount = m_LTforest2agCount = 0;
   m_LTagCount = m_LTforestCount = 0;

   m_LToutVars.ClearRows();
   CalculateLTOutputVariables(pContext);

   return true;
   } // end of InitRunLT()


int APs::UGBlookup(int ugbID)
// Transforms UGB_DH_ID values into WW2100 UGB codes
   {
   switch (ugbID)
      {
      case 1022: return 49; // Oakridge
      case 1023: return 69; // Westfir
      case 1024: return 13; // Cottage Grove
      case 1025: return 37; // Lowell
      case 1026: return 14; // Creswell
      case 1027: return 67; // Veneta
      case 1028: return 22; // Eugene/Springfield
      case 1029: return 11; // Coburg
      case 1030: return 34; // Junction City
      case 1031: return 29; // Harrisburg
      case 1032: return 45; // Monroe
      case 1033: return 28; // Halsey
      case 1034: return 8; // Brownsville
      case 1035: return 64; // Sweet Home
      case 1036: return 59; // Sodaville
      case 1037: return 68; // Waterloo
      case 1038: return 36; // Lebanon
      case 1039: return 65; // Tangent
      case 1040: return 12; // Corvallis/Philomath
      case 1041: return 1; // Adair Village
      case 1042: return 2; // Albany/Millersburg
      case 1043: return 55; // Scio
      case 1044: return 31; // Idanha
      case 1045: return 33; // Jefferson
      case 1046: return 17; // Detroit
      case 1047: return 41; // Mill City
      case 1048: return 26; // Gates
      case 1049: return 38; // Lyons
      case 1050: return 62; // Stayton/Sublimity
      case 1051: return 4; // Aumsville
      case 1052: return 66; // Turner
      case 1053: return 44; // Monmouth/Independence
      case 1054: return 23; // Falls City
      case 1055: return 15; // Dallas
      case 1056: return 58; // Silverton
      case 1057: return 51; // Salem/Keiser
      case 1058: return 56; // Scotts Mills
      case 1059: return 46; // Mt. Angel
      case 1060: return 70; // Willamina
      case 1061: return 57; // Sheridan
      case 1062: return 27; // Gervais
      case 1063: return 3; // Amity
      case 1064: return 43; // Molalla
      case 1065: return 71; // Woodburn
      case 1066: return 30; // Hubbard
      case 1067: return 61; // St. Paul
      case 1068: return 18; // Donald
      case 1069: return 16; // Dayton
      case 1070: return 5; // Aurora
      case 1071: return 39; // McMinnville
      case 1072: return 35; // Lafayette
      case 1073: return 7; // Barlow
      case 1074: return 19; // Dundee
      case 1075: return 9; // Canby
      case 1076: return 10; // Carlton
      case 1077: return 20; // Estacada
      case 1078: return 47; // Newberg
      case 1079: return 72; // Yamhill
      case 1080: return -1; // unknown 1
      case 1081: return -1; // unknown 2
      case 1082: return 52; // Sandy
      case 1083: return 25; // Gaston
      case 1084: return 48; // North Plains
      case 1085: return 6; // Banks
      case 1086: return 40; // Metro
      case 1087: return -1; // Scappoose - not represented in the WW2100 IDU layer as of 11/28/13
      case 1088: return -1; // St. Helens/Columbia City - not represented in the WW2100 IDU layer as of 11/28/13
      default: ASSERT(0);
      }
   return 0;
   } // end of UGBlookup()


double APs::LTdevUsePredVal( // developed use predicted value
   int UGB,
   float acres,
   float pop_den,
   float HH_inc_Kdollars,
   float citydist,
   int county_group )
   {
   m_LT_X[0] = 1; // beta[0] is the intercept
   m_LT_X[1] = UGB>0 ? 1 : 0;
   m_LT_X[2] = pop_den;
   m_LT_X[3] = log(HH_inc_Kdollars);
   m_LT_X[4] = m_LTdevImp.m_value;;
   m_LT_X[5] = citydist;
   m_LT_X[6] = citydist*citydist;
   m_LT_X[7] = county_group==BENTON_GROUP ? 1 : 0;
   m_LT_X[8] = county_group==LANE_GROUP ? 1 : 0;
   m_LT_X[9] = county_group==WASHINGTON_GROUP ? 1 : 0;

   double betaX = m_LTdevBeta[0]*m_LT_X[0];
   for (int i=1; i<=9; i++) betaX += m_LTdevBeta[i]*m_LT_X[i];
   if (m_LTtestMode==1) m_LTbetaX = (float)betaX;

   double corrected_val = exp(betaX)*m_LTdevCorrFactor.m_value;

   return(corrected_val);
   } // end of LTdevUsePredVal()


double APs::LTagUsePredVal( // agricultural use predicted value
   float acres,
   float slope,
   float citydist,
   int county_group,
   float farm_rent)
   {
   m_LT_X[0] = 1; // beta[0] is the intercept
   m_LT_X[1] = acres;
   m_LT_X[2] = slope;
   m_LT_X[3] = citydist;
   m_LT_X[4] = citydist*citydist;
   m_LT_X[5] = county_group==BENTON_GROUP ? 1 : 0;
   m_LT_X[6] = county_group==LANE_GROUP ? 1 : 0;
   m_LT_X[7] = county_group==WASHINGTON_GROUP ? 1 : 0;
   m_LT_X[8] = m_LTagImp.m_value;
   m_LT_X[9] = farm_rent;
   m_LT_X[10] = m_LTagMeanImp.m_value;

   double betaX = m_LTagBeta[0];
   for (int i=1; i<=10; i++) betaX += m_LTagBeta[i]*m_LT_X[i];
   if (m_LTtestMode==2) m_LTbetaX = (float)betaX;

   double corrected_val = exp(betaX)*m_LTagCorrFactor.m_value;

   return(corrected_val);
   } // end of LTagUsePredVal()


double APs::LTforUsePredVal( // agricultural use predicted value
   float acres,
   float slope,
   float elev,
   float riv_feet,
   bool PNI,
   float ugbdist,
   float citydist,
   int county_group)
   {
   m_LT_X[0] = 1; // beta[0] is the intercept
   m_LT_X[1] = acres;
   m_LT_X[2] = slope;
   m_LT_X[3] = elev;
   m_LT_X[4] = riv_feet;
   m_LT_X[5] = PNI ? 1 : 0;
   m_LT_X[6] = ugbdist;
   m_LT_X[7] = citydist;
   m_LT_X[8] = citydist*citydist;
   m_LT_X[9] = county_group==BENTON_GROUP ? 1 : 0;
   m_LT_X[10] = county_group==LANE_GROUP ? 1 : 0;
   m_LT_X[11] = county_group==WASHINGTON_GROUP ? 1 : 0;
   m_LT_X[12] = m_LTforestImp.m_value;
   m_LT_X[13] = m_LTforestMeanImp.m_value;

   double betaX = m_LTforestBeta[0];
   for (int i=1; i<=13; i++) betaX += m_LTforestBeta[i]*m_LT_X[i];
   if (m_LTtestMode==3) m_LTbetaX = (float)betaX;

   double corrected_val = exp(betaX)*m_LTforestCorrFactor.m_value;

   return(corrected_val);
   } // end of LTforUsePredVal()


void APs::UXcalcDevelopedFracs(EnvContext *pContext)
   {
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;
   for (int ugb = 0; ugb<=MAX_UGA_NDX; ugb++) m_UXdevelopedArea[ugb] = m_UXdevelopableArea[ugb] = 0.;

   ASSERT(m_colOWNER>=0);
   ASSERT(m_colUGB>=0);
   ASSERT(m_colLulcA>=0);
   ASSERT(m_colAREA>=0);
   for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
      {
      // If the IDU is not in private ownership, ignore it.
      int owner = -1;
      pLayer->GetData(idu, m_colOWNER, owner);

      if (owner!=OWNER_PRIVATE_INDUSTRIAL && owner!=OWNER_PNI) continue;

      // If the IDU is not in a UGA ignore it.
      int ugb = -1;
      pLayer->GetData(idu, m_colUGB, ugb);
      if (ugb<1 || ugb>MAX_UGA_NDX) continue;

      // If the IDU is not developed, ag, or forest, ignore it.
      int lulcA = 0;
      pLayer->GetData(idu, m_colLulcA, lulcA);
      if (!(lulcA==LULCA_DEVELOPED || lulcA==LULCA_AGRICULTURE || lulcA==LULCA_FOREST)) continue;

      // Accumulate the areas for the developed fractions.
      float area = 0; 
      pLayer->GetData(idu, m_colAREA, area);

      m_UXdevelopableArea[ugb] += area;
      if (lulcA==LULCA_DEVELOPED) m_UXdevelopedArea[ugb] += area;

      } // end of loop thru IDUs

   for (int ugb=1; ugb<=MAX_UGA_NDX; ugb++) 
      {
      ASSERT(m_UXdevelopedArea[ugb]<=m_UXdevelopableArea[ugb]);
      m_UGAdevelopedFrac[ugb] = (float)
         (m_UXdevelopableArea[ugb]>0. ? m_UXdevelopedArea[ugb]/m_UXdevelopableArea[ugb] : 0.);
      }
   } // end of CalculateUXoutputVariables()


bool APs::RunLandTrans( EnvContext *pContext )
/* The model domain is 
(LULC_A = ag, other veg, forest, and developed) 
and (OWNER = private)
and (CountyID = Benton, Clackamas, Columbia, Lane, Linn, Marion, Multnomah, Polk, Washington, or Yamhill)
Excluded counties are Douglas, Lincoln, Tillamook, Deschutes, Wasco.
Additionally, once an IDU is developed, it stays developed, i.e. no further transitions are allowed by the model.
*/
   {
   testMessage(pContext, _T("RunLandTrans"));
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;
   bool includedCounties[26] = {
         false, // [0] -13 Wasco
         false, false, false, false, false, false, false, false, // [1:8] -12 ... -5
         false, // [9] -4 Deschutes
         true,  // [10] -3 Columbia
         false, false, false, // [11:13] -2 ... 0
         true,  // [14] 1 Benton
         true,  // [15] 2 Clackamas
         false, // [16] 3 Douglas
         true,  // [17] 4 Lane
         false, // [18] 5 Lincoln
         true,  // [19] 6 Linn 
         true,  // [20] 7 Marion
         true,  // [21] 8 Multnomah
         true,  // [22] 9 Polk 
         false, // [23] 10 Tillamook
         true,  // [24] 11 Washington
         true,  // [25] 12 Yamhill
         };
   #define COUNTYID_OFFSET 13 
   int countyGroup[26] = {
         0, // [0] -13 Wasco
         0, 0, 0, 0, 0, 0, 0, 0, // [1:8] -12 ... -5
         0, // [9] -4 Deschutes
         WASHINGTON_GROUP,  // [10] -3 Columbia
         0, 0, 0, // [11:13] -2 ... 0
         BENTON_GROUP,  // [14] 1 Benton
         WASHINGTON_GROUP,  // [15] 2 Clackamas
         0, // [16] 3 Douglas
         LANE_GROUP,  // [17] 4 Lane
         0, // [18] 5 Lincoln
         MARION_GROUP,  // [19] 6 Linn 
         MARION_GROUP,  // [20] 7 Marion
         WASHINGTON_GROUP,  // [21] 8 Multnomah
         MARION_GROUP,  // [22] 9 Polk 
         0, // [23] 10 Tillamook
         WASHINGTON_GROUP,  // [24] 11 Washington
         WASHINGTON_GROUP,  // [25] 12 Yamhill
         };
   int countyGroupAG[26] = {
	   0, // [0] -13 Wasco
	   0, 0, 0, 0, 0, 0, 0, 0, // [1:8] -12 ... -5
	   0, // [9] -4 Deschutes
	   WASHINGTON_GROUP,  // [10] -3 Columbia
	   0, 0, 0, // [11:13] -2 ... 0
	   BENTON_GROUP,  // [14] 1 Benton
	   MARION_GROUP,  // [15] 2 Clackamas
	   0, // [16] 3 Douglas
	   LANE_GROUP,  // [17] 4 Lane
	   0, // [18] 5 Lincoln
	   MARION_GROUP,  // [19] 6 Linn 
	   MARION_GROUP,  // [20] 7 Marion
	   WASHINGTON_GROUP,  // [21] 8 Multnomah
	   MARION_GROUP,  // [22] 9 Polk 
	   0, // [23] 10 Tillamook
	   WASHINGTON_GROUP,  // [24] 11 Washington
	   WASHINGTON_GROUP,  // [25] 12 Yamhill
   };


   // Every so many years, recalculate the transition probabilities for the IDUs and
   // the developed fraction of each UGA.
   bool intervalFlag = (pContext->yearOfRun % m_LandUseInterval)==0;
   // Accumulate the areas for the developed fraction calculation as we loop thru the IDUs below.
   double developedArea[MAX_UGA_NDX+1];
   double developableArea[MAX_UGA_NDX+1];
   if (intervalFlag) 
         for (int ugb = 0; ugb<=MAX_UGA_NDX; ugb++) developedArea[ugb] = developableArea[ugb] = 0.;

   // loop through the IDUs
   ASSERT(m_colOWNER>=0);
   ASSERT(m_colCOUNTYID>=0); 
   ASSERT(m_colLulcA>=0);
   ASSERT(m_colPROBUSECHG>=0);
   ASSERT(m_colUGB>=0); 
   ASSERT(m_colAREA>=0); 
   ASSERT(m_colD_CITY_20K>=0); 
   ASSERT(m_colSLOPE_DEG>=0); 
   ASSERT(m_colFRMRNT_LR>=0); 
   ASSERT(m_colELEV>=0); 
   ASSERT(m_colSTREAM_LEN>=0);  
   ASSERT(m_colD_UGB_EE>=0);  
   ASSERT(m_colPROB_DEV>=0); 
   ASSERT(m_colPVT>=0); 
   ASSERT(m_colAGECLASS>=0); 
   ASSERT(m_colVEGCLASS>=0);
   ASSERT(m_colNRUGBndx>=0);  
   ASSERT(m_colPOP>=0);
   if (m_pHholdInc==NULL)
      {
      CString msg;
      msg.Format("APs: The household income file has not been loaded, so the Land-Use Transition model is unable to run.");
      Report::ErrorMsg(msg);
      return(false);
      }

   m_LTag2devCount = m_LTag2forestCount = m_LTforest2devCount = m_LTforest2agCount = 0;
   m_LTagCount = m_LTforestCount = 0;
   bool affectedUGBs[MAX_UGA_NDX + 1]; for (int ugb = 0; ugb<=MAX_UGA_NDX; ugb++) affectedUGBs[ugb] = false;
   bool NRUGBndx_error = false;
   for (MapLayer::Iterator idu = pLayer->Begin(); idu != pLayer->End(); idu++)
   {
	   int myIDU = idu;
		int new_lulcFine = 0;
	   // If the IDU is not in private ownership, ignore it.
	   int owner = -1;
	   pLayer->GetData(idu, m_colOWNER, owner);
	   if (owner == OWNER_PRIVATE_INDUSTRIAL || owner == OWNER_PNI )
			{
		   // If the IDU is not developed, ag, or forest, ignore it.
		   int lulcA = -1;
		   pLayer->GetData(idu, m_colLulcA, lulcA);

			if (lulcA == LULCA_DEVELOPED || lulcA == LULCA_AGRICULTURE || lulcA == LULCA_FOREST)
				{
				// If the IDU is in a UGA and this is every so many years, accumulate the areas for the developed fractions.
				int ugb = -1; pLayer->GetData(idu, m_colUGB, ugb);
				float area = 0; pLayer->GetData(idu, m_colAREA, area);
            int floodplain = 0; pLayer->GetData(idu, m_colFLOODPLAIN, floodplain);
            boolean fp_flag = floodplain == 1;
				if (ugb > 0 && ugb <= MAX_UGA_NDX && intervalFlag)
					{
					if (!fp_flag || lulcA == LULCA_DEVELOPED) developableArea[ugb] += area;
					if (lulcA == LULCA_DEVELOPED) developedArea[ugb] += area;
					} // end of if (ugb>0 && ugb<=MAX_UGA_NDX && intervalFlag)

				// IDUs which are already in developed use will stay that way, so now that the areas for
            // the developed fractions have been accumulated, we can ignore developed IDUs after we make sure
            // their PROB_DEV and PROBUSECHG attributes have been cleared.
            if (lulcA == LULCA_DEVELOPED)
               {
               // In the year of a transition, Pdev and Pchg are stored as
               // negative values in the PROB_DEV and PROBUSECHG attributes. 
               // For IDUs which have transitioned to Ag or Forest, the negative 
               // values are subsequently overwritten in the "else" clause below.  
               // For developed IDUs, here in this block
               // is where the negative values are overwritten in the year after
               // the transition.
               float prev_yr_Pdev = 0.f;
               pLayer->GetData(idu, m_colPROB_DEV, prev_yr_Pdev);
               if (prev_yr_Pdev < 0.f)
                  {
                  UpdateIDU(pContext, idu, m_colPROB_DEV, 0.f, true);
                  UpdateIDU(pContext, idu, m_colPROBUSECHG, 0.f, true);
                  }
               }
            else // lulcA != LULCA_DEVELOPED; must be Ag or Forest
               {

					// Skip over IDUs in counties which are not in the domain of the model;
               // also skip over undeveloped IDUs in the floodplain.
					int countyId = -1; pLayer->GetData(idu, m_colCOUNTYID, countyId);
					int countyNdx = countyId + COUNTYID_OFFSET;
					if ((countyNdx >= 0 && countyNdx < (sizeof(includedCounties) / sizeof(bool)) && includedCounties[countyNdx]) && !fp_flag)
						{
						switch (lulcA) // Count IDUs which are eligible to transition to a different use.
							{
							case LULCA_DEVELOPED: break;
							case LULCA_AGRICULTURE: m_LTagCount++; break;
							case LULCA_FOREST: m_LTforestCount++; break;
							default: ASSERT(false);
							}

						// Every so many years, recalculate the transition probabilities.
						// Also recalculate in the year following a transition from forest to ag or vice versa.
						bool recalculate = intervalFlag;
						double Pchg = -99.;
						double Pdev = -99.;
						if (!recalculate)
							{
							pLayer->GetData(idu, m_colPROBUSECHG, Pchg);
							recalculate = Pchg <= 0.;
							} // end of if (!recalculate)
						double APaa, APaf, APad, APff, APfa, APfd;


						float acres = area / M2_PER_ACRE;
						int nrcity_20k = 0; pLayer->GetData(idu, m_colNRUGBndx, nrcity_20k);
						ASSERT(nrcity_20k >= 0 && nrcity_20k<(sizeof(m_UGBadjPopDensities) / sizeof(float)));
						float pop_den;
                  if (nrcity_20k < 1 || nrcity_20k > MAX_UGA_NDX)
                  {
                     pop_den = 0.f;
                     if (!NRUGBndx_error)
                     {
                        CString msg;
                        msg.Format("LU Transitions: NRUGBndx out of bounds. idu = %d, NRUGBndx = %d",
                           (int)idu, nrcity_20k);
                        Report::ErrorMsg(msg);
                        NRUGBndx_error = true;
                     }
                  }
						else pop_den = m_UGBadjPopDensities[nrcity_20k] * M2_PER_ACRE;
						int row = pContext->currentYear - m_HholdInc_yr0;
						float HH_inc;
						if (ugb > 0 && ugb <= MAX_UGA_NDX) HH_inc = Get4UGB(m_pHholdInc, ugb, row);
						else
							{
							int hholdInc_col = CountyColFromCountyID(countyId);
							HH_inc = m_pHholdInc->GetAsFloat(hholdInc_col, row);
							}
						float HH_inc_Kdollars = HH_inc / 1000.f; // Convert to units of $K/household
						float citydist_meters = 0; pLayer->GetData(idu, m_colD_CITY_20K, citydist_meters);
						float citydist = citydist_meters*MI_PER_M;
						int county_group = countyGroup[countyId + COUNTYID_OFFSET];
						int county_groupAG = countyGroupAG[countyId + COUNTYID_OFFSET];
						float slope = 0; pLayer->GetData(idu, m_colSLOPE_DEG, slope);
						float farm_rent = 0; pLayer->GetData(idu, m_colFRMRNT_LR, farm_rent);
						float elev = 0; pLayer->GetData(idu, m_colELEV, elev);
						float riv_feet = 0; pLayer->GetData(idu, m_colSTREAM_LEN, riv_feet);
						riv_feet *= FT_PER_M;
						bool PNI = owner == OWNER_PNI;
						float ugbdist_meters = 0; pLayer->GetData(idu, m_colD_UGB_EE, ugbdist_meters);
						float ugbdist = ugbdist_meters*MI_PER_M;

						if (recalculate)
							{ // Calculate the transition probabilities.   
							// First, calculate the predicted values for the three uses.
							double dev_val = LTdevUsePredVal(ugb, acres, pop_den, HH_inc_Kdollars, citydist, county_group);
							if (m_LTtestMode == 1) for (int Xndx = 0; Xndx <= 11; Xndx++) UpdateIDU(pContext, idu, m_colX0 + Xndx, (float)m_LT_X[Xndx], true);

							double ag_val = LTagUsePredVal(acres, slope, citydist, county_groupAG, farm_rent);
							if (m_LTtestMode == 2) for (int Xndx = 0; Xndx <= 10; Xndx++) UpdateIDU(pContext, idu, m_colX0 + Xndx, (float)m_LT_X[Xndx], true);

							double for_val = LTforUsePredVal(acres, slope, elev, riv_feet, PNI, ugbdist, citydist, county_group);
							if (m_LTtestMode == 3) for (int Xndx = 0; Xndx <= 13; Xndx++) UpdateIDU(pContext, idu, m_colX0 + Xndx, (float)m_LT_X[Xndx], true);

                     UpdateIDU(pContext, idu, m_colDEV_VAL, (float)dev_val, true);
                     UpdateIDU(pContext, idu, m_colAG_VAL, (float)ag_val, true);
                     UpdateIDU(pContext, idu, m_colFOR_VAL, (float)for_val, true);

							// Second, scale and constrain the predicted values, according to the current use.
							// Naming conventions are adopted from usage in Andrew Plantinga's specification document:
							// YRx = scaled predicted economic return for an IDU in use x when changed to use Y
							// Pxy = five year probability of changing from use x to use y
							// APxy = one year probability of changing from use x to use y
							double DR = min(dev_val*m_LTfromAg.toDeveloped.scaleFactor, m_LTfromAg.toDeveloped.maxVal);
							double ARa, FRa, ARf, FRf;
							switch (lulcA)
								{
								case LULCA_AGRICULTURE:
									ARa = min(ag_val*m_LTfromAg.toAg.scaleFactor, m_LTfromAg.toAg.maxVal);
									FRa = min(for_val*m_LTfromAg.toForest.scaleFactor, m_LTfromAg.toForest.maxVal);
									break;
								case LULCA_FOREST:
									ARf = min(ag_val*m_LTfromForest.toAg.scaleFactor, m_LTfromForest.toAg.maxVal);
									FRf = min(for_val*m_LTfromForest.toForest.scaleFactor, m_LTfromForest.toForest.maxVal);
									break;
								default: ASSERT(false); break;
								} // end of switch (lulcA)

							if (m_LTtestMode > 0)
								{
								UpdateIDU(pContext, idu, m_colBETAX, m_LTbetaX, true);
								UpdateIDU(pContext, idu, m_colPROB0, (float)dev_val, true);
								UpdateIDU(pContext, idu, m_colPROB0 + 1, (float)ag_val, true);
								UpdateIDU(pContext, idu, m_colPROB0 + 2, (float)for_val, true);
								UpdateIDU(pContext, idu, m_colPROB0 + 3, (float)DR, true);
								UpdateIDU(pContext, idu, m_colPROB0 + 4, (float)(lulcA == LULCA_FOREST ? ARf : ARa), true);
								UpdateIDU(pContext, idu, m_colPROB0 + 5, (float)(lulcA == LULCA_FOREST ? FRf : FRa), true);
								} // end of if (m_LTtestMode>0)

							// Third, calculate and store the relevant probabilities.
							double DR_term = exp(m_LT_DR.m_offset_value + m_LT_DR.m_value*DR);
							double ARa_term, FRa_term, FRf_term, ARf_term, denominator;
							double Paa, Paf, Pad, Pff, Pfa, Pfd;
							double annual_exponent = 1. / m_LandUseInterval;
							switch (lulcA)
								{
								case LULCA_AGRICULTURE:
									ARa_term = exp(m_LTself.m_value*ARa);
									FRa_term = exp(m_LTcrossR.m_offset_value + m_LTcrossR.m_value*FRa);
									denominator = ARa_term + FRa_term + DR_term;
									Paa = ARa_term / denominator; ASSERT(0. <= Paa && Paa <= 1.);
									Paf = FRa_term / denominator; ASSERT(0. <= Paf && Paf <= 1.);
									Pad = 1. - Paa - Paf; ASSERT(0. <= Pad && Pad <= 1.);
									if (ugb<1 || ugb>MAX_UGA_NDX)
										{ // Rural: transition to developed is not allowed.
										Pad = 0.;
										// Renormalize the remaining possibilities to sum to 1. 
										double Ptot = Paa + Paf;
										Paa /= Ptot;
										Paf /= Ptot;
										}
									APaf = Paf < 1. ? (1. - pow(1. - Paf, annual_exponent)) : 1.;
									Pdev = APad = Pad < 1. ? (1. - pow(1. - Pad, annual_exponent)) : 1.;
									APaa = 1. - APaf - APad;
									Pchg = (APad + APaf);
									break;
								case LULCA_FOREST:
									FRf_term = exp(m_LTself.m_value*FRf);
									ARf_term = exp(m_LTcrossR.m_offset_value + m_LTcrossR.m_value*ARf);
									denominator = FRf_term + ARf_term + DR_term;
									Pff = FRf_term / denominator; ASSERT(0. <= Pff && Pff <= 1.);
									Pfa = ARf_term / denominator; ASSERT(0. <= Pfa && Pfa <= 1.);
									Pfd = 1. - Pff - Pfa; ASSERT(0. <= Pfd && Pfd <= 1.);
									if (ugb<1 || ugb>MAX_UGA_NDX)
										{ // Rural: transition to developed is not allowed.
										Pfd = 0.;
										// Renormalize the remaining possibilities to sum to 1. 
										double Ptot = Pfa + Pff;
										Pfa /= Ptot;
										Pff /= Ptot;
										}
									APfa = Pfa < 1. ? (1. - pow(1. - Pfa, annual_exponent)) : 1.;
									Pdev = APfd = Pfd < 1. ? (1. - pow(1. - Pfd, annual_exponent)) : 1.;
									APff = 1. - APfa - APfd;
									Pchg = (APfd + APfa);
									break;
								default: ASSERT(false); break;
								} // end of switch (lulcA)
							UpdateIDU(pContext, idu, m_colPROB_DEV, Pdev, true);
							UpdateIDU(pContext, idu, m_colPROBUSECHG, Pchg, true);

							} // end of if (recalculate)
						else
							{ // Use the stored probabilities.  
							Pdev = 0.; pLayer->GetData(idu, m_colPROB_DEV, Pdev);
							if (Pchg == -99.) pLayer->GetData(idu, m_colPROBUSECHG, Pchg);
							switch (lulcA)
								{
								case LULCA_AGRICULTURE:
									APad = Pdev;
									APaf = Pchg - Pdev;
									APaa = 1. - Pchg;
									break;
								case LULCA_FOREST:
									APfd = Pdev;
									APfa = Pchg - Pdev;
									APff = 1 - Pchg;
									break;
								default: ASSERT(false); break;
								} // end of switch (lulcA)
							} // end of if (recalculate) ... else

						// Now use the probabilities to decide whether to change the land use.
						double rand_num = m_LTrandomDraw.RandValue(0., 1.);
						new_lulcFine = 0;
						ASSERT(Pchg > Pdev || (Pchg == 0. && Pdev == 0.));
						if (rand_num <= Pdev)
							{ // Change to developed use.
							ASSERT(ugb > 0 && ugb <= MAX_UGA_NDX);
							new_lulcFine = LULC_FINE_DEVELOPED_UNDIFFERENTIATED;
							float newLAI = 1.0f;
							if (lulcA == LULCA_FOREST) m_LTforest2devCount++; else m_LTag2devCount++;

							float idu_pop; pLayer->GetData(idu, m_colPOP, idu_pop);
							m_UGBadjAreas[ugb] += area;
							m_UGBlegacyPops[ugb] -= idu_pop;
							affectedUGBs[ugb] = true;
							double trick_Pdev = (lulcA == LULCA_FOREST) ? (-Pdev - 1) : -Pdev;
							UpdateIDU(pContext, idu, m_colPROB_DEV, trick_Pdev, true); // Signal that a change to developed use has happened.
							if (lulcA == LULCA_AGRICULTURE) UpdateIDU(pContext, idu, m_colWR_PROB, 0, true);
							UpdateIDU(pContext, idu, m_colLAI, newLAI, true);
							}
						else if (rand_num <= Pchg)
							{ // Change from forest to ag or vice versa.
							if (lulcA == LULCA_FOREST)
								{
								m_LTforest2agCount++;
								new_lulcFine = VEGCLASS_FALLOW;
                        UpdateIDU(pContext, idu, m_colLAI, 0, true);

                        // Climate means for this IDU need to be initialized at the beginning of the next year.
                        float tmin_grow; pContext->pMapLayer->GetData(idu, m_colTMIN_GROW, tmin_grow);
                       if (!IDU_IN_AG_BASIN(tmin_grow))
                        {
                           // Update climate means for this IDU from now on.
                           UpdateIDU(pContext, idu, m_colTMIN_GROW, TOKEN_INIT_EXP_AVG, true);
								}
                     }
							else
								{ // Change from ag to forest.
								m_LTag2forestCount++;
								new_lulcFine = VEGCLASS_DF_GFp;
								int new_pvt = PVT_FWI;
								int new_standage = 0;
								UpdateIDU(pContext, idu, m_colPVT, new_pvt, true);
								UpdateIDU(pContext, idu, m_colAGECLASS, new_standage, true);
                        UpdateIDU(pContext, idu, m_colWR_PROB, 0, true);
                        UpdateIDU(pContext, idu, m_colLAI, 0, true);
                        } // end of if (lulcA==LULCA_FOREST) else 
							} // end of if (rand_num<=Pchg)
						if (new_lulcFine > 0)
							{ // A land use transition has occurred.
							UpdateIDU(pContext, idu, m_colVEGCLASS, new_lulcFine, true);
							UpdateIDU(pContext, idu, m_colPROBUSECHG, (double)-Pchg, true); // Signal that a change has happened.
							} // end of if (new_lulcFine>0)
						} //end if county is in modeled domain
					}// end if developed
				} //if developed ag or forest
	      }// endif owner == OWNER_PRIVATE_INDUSTRIAL && owner == OWNER_PNI

/*		 
		 //Inserted Hack by James; Suggest a rewrite of this method or move land value calulations to own method after UGB expansion	 
		 int ugb = -1; pLayer->GetData(idu, m_colUGB, ugb);
		 float area = 0; pLayer->GetData(idu, m_colAREA, area);
		 int countyId = -1; pLayer->GetData(idu, m_colCOUNTYID, countyId);

		 float acres = area / M2_PER_ACRE;
		 int nrcity_20k = 0; pLayer->GetData(idu, m_colNRUGBndx, nrcity_20k);
		 ASSERT(nrcity_20k >= 0 && nrcity_20k<(sizeof(m_UGBadjPopDensities) / sizeof(float)));
		 float pop_den; // persons per acre
		 if (nrcity_20k>0 && nrcity_20k <= MAX_UGA_NDX) pop_den = m_UGBadjPopDensities[nrcity_20k] * M2_PER_ACRE;
		 else
		 {
			 CString msg;
			 msg.Format("LU Transitions: nrcity_20k out of bounds. idu = %d, nrcity_20k = %d",
				 (int) idu, nrcity_20k);
			 Report::ErrorMsg(msg);
			 pop_den = 0.f;
		 }
		 int row = pContext->currentYear - m_HholdInc_yr0;
		 float HH_inc = 0;
		 if (ugb > 0 && ugb <= MAX_UGA_NDX) HH_inc = Get4UGB(m_pHholdInc, ugb, row);
		 else
		    {
			 int hholdInc_col = CountyColFromCountyID(countyId);			 
			 HH_inc = m_pHholdInc->GetAsFloat(hholdInc_col, row);
		    }
		 float HH_inc_Kdollars = HH_inc / 1000.f; //  $K/household
		 float citydist_meters = 0; // meters
		 pLayer->GetData(idu, m_colD_CITY_20K, citydist_meters);
		 float citydist = citydist_meters*MI_PER_M; // miles
		 int county_group = countyGroup[countyId + COUNTYID_OFFSET];
		 int county_groupAG = countyGroupAG[countyId + COUNTYID_OFFSET];
		 float slope = 0; pLayer->GetData(idu, m_colSLOPE_DEG, slope);
		 float farm_rent = 0; pLayer->GetData(idu, m_colFRMRNT_LR, farm_rent);
		 float elev = 0; pLayer->GetData(idu, m_colELEV, elev);
		 float riv_feet = 0; pLayer->GetData(idu, m_colSTREAM_LEN, riv_feet);
		 riv_feet *= FT_PER_M;
		 bool PNI = owner == OWNER_PNI;
		 float ugbdist_meters = 0; pLayer->GetData(idu, m_colD_UGB_EE, ugbdist_meters);
		 float ugbdist = ugbdist_meters*MI_PER_M;

		 double dev_val = LTdevUsePredVal(ugb, acres, pop_den, HH_inc_Kdollars, citydist, county_group); // $/acre
		 double ag_val = LTagUsePredVal(acres, slope, citydist, county_groupAG, farm_rent); // $/acre
		 double for_val = LTforUsePredVal(acres, slope, elev, riv_feet, PNI, ugbdist, citydist, county_group); // $/acre
		 int mytime = pContext->currentYear;
		 int lulcA = -1;
		 int newVegClass = 0;
       
		 pLayer->GetData(idu, m_colLulcA, lulcA);

		 if ( lulcA == 1 )
			UpdateIDU( pContext, idu, m_colDEV_VAL, (float)dev_val, true );

		 if ( lulcA == 2 )
			UpdateIDU( pContext, idu, m_colAG_VAL, (float)ag_val, true );
		 
		 if ( lulcA == 4 ) 
			UpdateIDU( pContext, idu, m_colFOR_VAL, (float)for_val, true );
*/		
       } // end of loop thru IDUs

   CString msg;
   msg.Format("Land-use transitions %d: %d from ag or other veg to developed, %d from ag or other veg to forest, %d from forest to developed, %d from forest to fallow.",
         pContext->currentYear, m_LTag2devCount, m_LTag2forestCount, m_LTforest2devCount, m_LTforest2agCount);
   Report::LogMsg(msg);
   CalculateLTOutputVariables(pContext);
   
   return true;
   } // end of RunLandTrans()
      

bool APs::RunUX(EnvContext *pContext)
   {
   testMessage(pContext, _T("RunUX"));
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;
   CString msg;

   ASSERT(m_colIDU_ID>=0);
   ASSERT(m_colAREA>=0);
   ASSERT(m_colUGB>=0);
   ASSERT(m_colLulcA>=0);
   ASSERT(m_colZONED_USE>=0);
   ASSERT(m_colMETRO_RSRV>=0);
   ASSERT(m_colRPA>=0);

   UXcalcDevelopedFracs(pContext);

   bool intervalFlag = (pContext->yearOfRun % m_LandUseInterval) == 0;
   if (intervalFlag) 
      { // Loop thru the UGBs to test whether any need to be expanded.
      QueryEngine *pQE = new QueryEngine(pLayer);  // checked
      for (int ugb=1; ugb<=MAX_UGA_NDX; ugb++) if (m_UXugaArray[ugb].id==ugb && ugb!=UGB_METRO && ugb!=UGB_EUGENESPRINGFIELD)
         {
         float threshold_frac = m_UXugaArray[ugb].threshold_pct/100.f; // as a fraction, not a %
         if (m_UGAdevelopedFrac[ugb]>=threshold_frac)
            {
            if (m_UXugaArray[ugb].id!=ugb) continue; // no info about the UGA was in the XML file
            bool over_threshold = true;
            float orig_dev_frac = m_UGAdevelopedFrac[ugb];
            float area_added = 0;
            float area_added_ac;
            int idus_added = 0;
 
            // while over the threshold
            CString query_str;
            bool successFlag;
			if (!(ugb == UGA_Metro && pContext->currentYear<2060 && m_currentUGAScenarioIndex == 0)) 
               { // General case, applies to everything except: Metro && before 2060 && reference scenario
               query_str.Format("NextTo(UGB = %d) and UGB=0 and "
                  "(OWNER=0 {Private non-industrial} or OWNER=1 {Private industrial forest}) and "
                  "(LULC_A=2 {ag} or LULC_A=4 {forest} or LULC_A=3 {other veg} or LULC_A=1 {developed}) and "
                  "(ZONED_USE!=1 {A/F} and ZONED_USE!=2 {Ag} and ZONED_USE!=4 {For})",
                  ugb);
               //Report::LogMsg(query_str);
               Query *pIdu_query = pQE->ParseQuery(query_str, -1, "UX query");
               ASSERT(pIdu_query!=NULL); 

               successFlag = UXaddIDUsToUGA(pContext, ugb, pIdu_query, &idus_added, &area_added);

               if (!successFlag)
                  { // Try again, but allow use of IDUs zoned for ag or forests.
                  query_str.Format("NextTo(UGB=%d) and UGB=0 and "
                        "(OWNER=0 {Private non-industrial} or OWNER=1 {Private industrial forest}) and "
                        "(LULC_A=2 {ag} or LULC_A=4 {forest} or LULC_A=3 {other veg} or LULC_A=1 {developed})", 
                        ugb);
                  //Report::LogMsg(query_str);
                  Query *pIdu_query = pQE->ParseQuery(query_str, -1, "UX query");
                  ASSERT(pIdu_query!=NULL); 

                  successFlag = UXaddIDUsToUGA(pContext, ugb, pIdu_query, &idus_added, &area_added);
                  }
               } // end of general case
            else
               { // Special case for: Metro && before 2060 && reference scenario
               query_str.Format("NextTo(UGB=40 {Metro}) and UGB=0 and METRO_RSRV=1 {urban reserve}");
               //Report::LogMsg(query_str);
               Query *pIdu_query = pQE->ParseQuery(query_str, -1, "UX query");
               ASSERT(pIdu_query!=NULL); 

               successFlag = UXaddIDUsToUGA(pContext, ugb, pIdu_query, &idus_added, &area_added);

               if (!successFlag)
                  { // Try again, but allow use of IDUs outside the urban reserves, 
                  // as long as they're not in the rural reserves.
                  query_str.Format("NextTo(UGB=40 {Metro}) and UGB=0 and (METRO_RSRV=1 {urban reserve} or " 
                        "((OWNER=0 {Private non-industrial} or OWNER=1 {Private industrial forest}) and "
                        "(LULC_A=2 {ag} or LULC_A=4 {forest} or LULC_A=3 {other veg} or LULC_A=1 {developed}) and "
                        "(ZONED_USE!=1 {A/F} and ZONED_USE!=2 {Ag} and ZONED_USE!=4 {For}) and "
                        "METRO_RSRV!=2 {rural reserve}))");
                  //Report::LogMsg(query_str);
                  Query *pIdu_query = pQE->ParseQuery(query_str, -1, "UX query");
                  ASSERT(pIdu_query!=NULL); 

                  successFlag = UXaddIDUsToUGA(pContext, ugb, pIdu_query, &idus_added, &area_added);
                  }

               if (!successFlag)
                  { // Try one more time, but allow use of IDUs zoned for ag or forests,
                  // as long as they're not in the rural reserves.
                  query_str.Format("NextTo(UGB=40 {Metro}) and UGB=0 and (METRO_RSRV=1 {urban reserve} or "
                        "((OWNER=0 {Private non-industrial} or OWNER=1 {Private industrial forest}) and "
                        "(LULC_A=2 {ag} or LULC_A=4 {forest} or LULC_A=3 {other veg} or LULC_A=1 {developed}) and " 
                        "METRO_RSRV!=2 {rural reserve}))");
                  //Report::LogMsg(query_str);
                  Query *pIdu_query = pQE->ParseQuery(query_str, -1, "UX query");
                  ASSERT(pIdu_query!=NULL); 

                  successFlag = UXaddIDUsToUGA(pContext, ugb, pIdu_query, &idus_added, &area_added);
                  }
               } // end of special case

            area_added_ac = area_added/M2_PER_ACRE;
            msg.Format("UGA expansion: UGB, idus_added, area_added_ac, orig_dev_frac, new_dev_frac = %s, %d, %f, %f, %f",
                  (LPCTSTR) m_UXugaArray[ugb].name, idus_added, area_added_ac, orig_dev_frac, m_UGAdevelopedFrac[ugb]);
            Report::LogMsg(msg);
            } // end of if (m_UGAdevelopedFrac[ugb]>=threshold_frac) 
         } // end of loop on ugb
 
      delete pQE;
      } // end of if (intervalFlag)

   UXsaveOutVars(pContext);
      
   return(true);
   } // end of RunUX()


bool APs::UXaddIDUsToUGA(EnvContext * pContext, int ugb, Query *pIdu_query, int *pIdus_added, float *pArea_added)
   {
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;
   float threshold_frac = m_UXugaArray[ugb].threshold_pct/100.f; // as a fraction, not a %
   float least_distance = 1.e9f; // initialize least_distance to a million km
   float prev_distance = -1.f; // temporary, imperfect mechanism for avoiding picking the same IDU twice
   int idu_to_add = -1;
   bool found_idu = m_UXugaArray[ugb].id==ugb; // true here means there is information about the UGA from the XML file
   bool over_threshold = true;
   while (over_threshold && found_idu)
      { // Find the best IDU to add and add it to the UGA
      found_idu = false;
      for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
         {               
         bool result = false;
         bool ok = pIdu_query->Run( idu, result );
         ASSERT(ok);
         if (result)
            { // Is this the best IDU found so far?
            bool best_idu = false;
            // Calculate the distance from the centroid of the IDU to the city center of the UGB.
            REAL centroidx = 0.; ((MapLayer *)(pContext->pMapLayer))->GetData(idu, m_colCENTROIDX, centroidx);
            REAL centroidy = 0.; ((MapLayer *)(pContext->pMapLayer))->GetData(idu, m_colCENTROIDY, centroidy);
            Vertex idu_centroid(centroidx, centroidy);
            if (idu_centroid.x != idu_centroid.x || idu_centroid.y != idu_centroid.y) continue; // detect and skip NaNs
            REAL dx = idu_centroid.x - m_UXugaArray[ugb].city_center.x;
            REAL dy = idu_centroid.y - m_UXugaArray[ugb].city_center.y;
            REAL d_cityCenter = sqrt(dx*dx + dy*dy);
            float d_hwy = 0.f; ((MapLayer *)(pContext->pMapLayer))->GetData(idu, m_colD_HWY, d_hwy);
            float d2minimize = (float)((d_cityCenter + d_hwy) / 2.);
            if (d2minimize>prev_distance) 
               { // This one hasn't been added already.
               if (!found_idu) found_idu = best_idu = true; // It's the first one found, so it's the best so far.
               else best_idu = d2minimize<least_distance;

               if (best_idu) 
                  {
                  least_distance = d2minimize;
                  idu_to_add = idu;
                  }
               }
            } // end if (result)
         } // end of loop thru IDUs

      if (found_idu)
         { // Add the idu to the UGA, and make the change to the IDU database immediately
               // so that it will be seen by subsequent queries in this same runUX() execution.
         bool readOnlyFlag = pContext->pMapLayer->m_readOnly;
         ((MapLayer *)(pContext->pMapLayer))->m_readOnly = false;
         ((MapLayer *)(pContext->pMapLayer))->SetData( idu_to_add, m_colUGB, ugb);
         UpdateIDU(pContext, idu_to_add, m_colUGB, ugb, true);
         ((MapLayer *)(pContext->pMapLayer))->m_readOnly = readOnlyFlag;
         
         // MAP_FIELD_INFO * pUgbNames = pLayer->GetFieldInfo(m_colUGB);
         prev_distance = least_distance;
         float idu_area; pLayer->GetData(idu_to_add, m_colAREA, idu_area);
         *pArea_added += idu_area;
         *pIdus_added += 1;
         m_UXdevelopableArea[ugb] += idu_area;
         int lulcA; pLayer->GetData(idu_to_add, m_colLulcA, lulcA);
         // ASSERT(lulcA!=LULCA_DEVELOPED);
         if (lulcA==LULCA_DEVELOPED) m_UXdevelopedArea[ugb] += idu_area;
         m_UGAdevelopedFrac[ugb] = (float)(m_UXdevelopedArea[ugb]/m_UXdevelopableArea[ugb]);
         over_threshold = m_UGAdevelopedFrac[ugb]>=threshold_frac;
         float area_added_ac = *pArea_added/M2_PER_ACRE;

         if (m_UXtestMode==1) 
            { CString msg;
            msg.Format("UGA expansion: UGB, idu, lulcA, area_added_ac, threshold_frac, new_dev_frac = %s, %d, %d, %f, %f, %f",
                  (LPCTSTR) m_UXugaArray[ugb].name, idu_to_add, lulcA, area_added_ac, threshold_frac, m_UGAdevelopedFrac[ugb]);
            Report::InfoMsg(msg);
            }

         // Was the idu in an RPA?
         int rpaID; pLayer->GetData(idu_to_add, m_colRPA, rpaID);
         if (rpaID>0)
            { // Remove the IDU from the RPA, since it is now in a UGA.
            UpdateIDU(pContext, idu_to_add, m_colRPA, 0, true);
            m_RPAareas[rpaID] -= idu_area;

            // Is there anything left of this RPA?
            if (m_RPAareas[rpaID]<=0)
               { // No, the RPA has disappeared entirely.
               m_RPAareas[rpaID] = 0;
               m_RPAdensities[rpaID] = 0;
               if (m_UXtestMode==1)
                  { CString msg; 
                  msg.Format("APs.UGA expansion: RPA with no area.  rpaID = %d", rpaID);
                  Report::LogMsg(msg);
                  } // end of if (m_UXtestMode==1)
               } // end of if (m_RPAareas[rpaID]<=0)
            } // end of if (rpaID>0)

         } // end of if (found_idu)

      } // end of while (over_threshold && found_idu)
   
   return(!over_threshold);
   } // end of UXaddIDUsToUGA()
   

bool APs::InitLandTrans( EnvContext *pContext )
   {
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;

   if (m_LTtestMode>0) 
      {
      InitializeTestMode(pLayer, (sizeof(m_LT_X)/sizeof(double)) - 1, 5);
      for (int i=0; i<(sizeof(m_LT_X)/sizeof(double)); i++) m_LT_X[i] = 0.;
      }

   return true;
   }


void APs::InitPopDensCalcs( EnvContext *pContext )
   {
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;

   ASSERT(m_colRPA>=0);
   ASSERT(m_colAREA>=0);
   ASSERT(m_colUGB>=0);
   ASSERT(m_colPOP>=0);
   ASSERT(m_colOWNER>=0);
   ASSERT(m_colLulcA>=0);
   // Calculate areas of population units, legacy populations of UGBs, and adjusted densities of UGBs.
   // A "legacy" population is population inside a UGB which is not privately-owned and/or not in a developed IDU.
   for (int i=0; i<MAX_RR_DH_ID; i++) m_RPAareas[i] = 0;
   float UGBtotPops[MAX_UGB];
   for (int i=0; i<MAX_UGB; i++) m_UGBadjAreas[i] = m_UGBlegacyPops[i] = UGBtotPops[i] = 0;
   // Loop thru the IDUs
   for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
      {
      float area;

      int rpaID, ugb;
      pLayer->GetData( idu, m_colUGB, ugb );
      if (ugb<1 || ugb>MAX_UGA_NDX) // Make sure the IDU is not in a UGB
         {
         pLayer->GetData( idu, m_colRPA,  rpaID );
         if (rpaID>0 && rpaID<MAX_RR_DH_ID)
            {
            pLayer->GetData( idu, m_colAREA,  area );
            m_RPAareas[rpaID] += area;
            }
         }
      else 
         {
         ASSERT(ugb>0 && ugb<=MAX_UGA_NDX);
         float pop;
         pLayer->GetData( idu, m_colPOP, pop );
         UGBtotPops[ugb] += pop;

         int owner, lulc_a;
         pLayer->GetData( idu, m_colOWNER, owner );
         pLayer->GetData( idu, m_colLulcA, lulc_a );
         if ((owner==OWNER_PNI || owner==OWNER_PRIVATE_INDUSTRIAL) && lulc_a==LULCA_DEVELOPED)
            {
            pLayer->GetData( idu, m_colAREA, area );
            m_UGBadjAreas[ugb] += area;
            } // end of if (owner==PRIVATELY_OWNED && lulc_a==DEVELOPED)
         else 
            { // Accumulate legacy population
            m_UGBlegacyPops[ugb] += pop;
            } // end of if (owner==PRIVATELY_OWNED && lulc_a==DEVELOPED) ... else

         } // end of if (rpaID>0 ...) else 
      } // end of loop on IDUs

   for (int ugb=0; ugb<MAX_UGB; ugb++) 
      {
      m_UGBtargetPops[ugb] = UGBtotPops[ugb];
      m_UGBdensities[ugb] = m_UGBadjAreas[ugb]>0.f ? (UGBtotPops[ugb] - m_UGBlegacyPops[ugb])/m_UGBadjAreas[ugb] : 0.f;
      m_UGBadjPopDensities[ugb] = m_UGBadjAreas[ugb]>0 ? UGBtotPops[ugb]/m_UGBadjAreas[ugb] : 0.f;
      }

   } // end of InitPopDensCalcs()


bool APs::RunPopGrowth( EnvContext *pContext )
   {
   testMessage(pContext, _T("RunPopGrowth"));

   InitPopDensCalcs( pContext );

   /* In each annual timestep, the popGrowth subprocess will:
   - look up or interpolate the prescribed populations for the UGAs and RPAs
   - determine the areas of the RPAs, from summing the AREA attributes of the IDUs in the RPAs
   - calculate the population density of each RPA, by dividing the prescribed population for the RPA by the total area of the RPA
   - for each IDU in every RPA, store the RPA's population density into the IDU's POPDENS attribute, and calculate a new value
     for the IDU's POP attribute equal to the product of POPDENS and AREA, rounded to an integer
   - for each county, calculate the total population of the RPAs in the county (note that this number may be smaller than the total rural population
     of the county, since some rural IDUs may be populated but not part of any RPA)
   - determine the total area of developed (LULC_A attribute = 'Developed'), privately-owned (OWNER attribute = "Private") land in each UGA
   - calculate the adjusted population density of each UGA, by dividing the prescribed population for the UGA by the total area of developed, 
     private land in the UGA
   - determine the total population within the portion of the UGA which is either undeveloped or not privately-owned or both, here called the 
     "legacy population" since it is not produced by the population growth model
   - for each UGA, calculate a new value for the POPDENS attribute of developed, private-owned IDUs, as
      (prescribed UGA population - UGA legacy population)/(area of private, developed land)
   - calculate and store a new value for the POP attribute of each IDU in the developed, privately-owned portion of every UGA, as the 
     product of the of the IDU's POPDENS and AREA attributes
     */

   if (pContext->currentYear==pContext->endYear) return(true);

   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;

   // Look up or interpolate the prescribed populations for the UGAs and RPAs, and calculate densities
   for (int rpaID=0; rpaID<MAX_RR_DH_ID; rpaID++) { m_RPAtargetPops[rpaID] = -1; m_RPAdensities[rpaID] = 0.; }
   for (int ugb=0; ugb<MAX_UGB; ugb++) { m_UGBtargetPops[ugb] = -1; m_UGBdensities[ugb] = 0.; }
   int yr_offset = (pContext->currentYear + 1) - YR_OF_FIRST_POP_PROJECTION;
   int multiple = yr_offset/POP_DATA_INTERVAL;
   int remainder = yr_offset - multiple*POP_DATA_INTERVAL;
   float fraction = (float)remainder/(float)POP_DATA_INTERVAL;
   
   // Do it for the RPAs
   int tgtCol = m_colRPP_pop2010 + multiple;
   if (tgtCol>=m_RPApopProj_table.GetColCount()) tgtCol = m_RPApopProj_table.GetColCount() - 1;
   for (int iRow=0; iRow<m_records_RPP; iRow++)
      {
      float tgtPop;
      float tgt0 = m_RPApopProj_table.GetAsFloat(tgtCol, iRow);
      if (remainder==0) tgtPop = tgt0;
      else
         {
         int tgt1Col = tgtCol + 1;
         if (tgt1Col>=m_RPApopProj_table.GetColCount()) tgt1Col = m_RPApopProj_table.GetColCount() - 1;
         float tgt1 = m_RPApopProj_table.GetAsFloat(tgt1Col, iRow);
         tgtPop = tgt0 + fraction*(tgt1 - tgt0);
         }
      int rpaID = m_RPApopProj_table.GetAsInt(m_colRPP_RR_DH_ID, iRow);
      m_RPAtargetPops[rpaID] = tgtPop;
      if (m_RPAareas[rpaID]>0) m_RPAdensities[rpaID] = tgtPop/m_RPAareas[rpaID];
      else 
         { // This situation, where there are no IDUs and hence no area associated
               // with an RPAid, can arise when the simulation is run for a subbasin
               // such as the Pudding River basin.  It can also arise when all the IDUs
               // which were originally in the RPA have been absorbed by an expanding UGA.
//         CString msg;
//         msg.Format("APs.popGrowth: RPA with no area.  rpaID = %d", rpaID);
         m_RPAdensities[rpaID] = 0.;
//         if (pContext->currentYear==pContext->startYear) 
//            Report::LogMsg(msg);
         }
      } // end of  for (iRow=0; iRow<m_records_RPP; iRow+)
   
   // Now do it for the UGAs
   tgtCol = m_colUPP_pop2010 + multiple;
   if (tgtCol>=m_UGApopProj_table.GetColCount()) tgtCol = m_UGApopProj_table.GetColCount() - 1;
   for (int iRow=0; iRow<m_records_UPP; iRow++)
      {
      float tgtPop;
      float tgt0 = m_UGApopProj_table.GetAsFloat(tgtCol, iRow);
      if (remainder==0) tgtPop = tgt0;
      else
         {
         int tgt1Col = tgtCol + 1;
         if (tgt1Col>=m_UGApopProj_table.GetColCount()) tgt1Col = m_UGApopProj_table.GetColCount() - 1;
         float tgt1 = m_UGApopProj_table.GetAsFloat(tgt1Col, iRow);
         tgtPop = tgt0 + fraction*(tgt1 - tgt0);
         }
      int ugbID = m_UGApopProj_table.GetAsInt(m_colUPP_UGB_DH_ID, iRow);
      int ugb = UGBlookup(ugbID);
      if (ugb<0) continue;
      m_UGBtargetPops[ugb] = tgtPop;
      m_UGBdensities[ugb] = m_UGBadjAreas[ugb]>0.f ? (tgtPop - m_UGBlegacyPops[ugb])/m_UGBadjAreas[ugb] : 0.f;
      m_UGBadjPopDensities[ugb] = m_UGBadjAreas[ugb]>0.f ? tgtPop/m_UGBadjAreas[ugb] : 0.f;
      } // end of  for (iRow=0; iRow<m_records_UPP; iRow+)

   CalculatePopGrowthOutputVariables(pContext);
      // m_EugeneSpringfieldAdjDens, m_SalemAdjDens, m_CorvallisAdjDens, m_AlbanyAdjDens, m_McMinnvilleAdjDens,
      //   m_NewbergAdjDens, m_WoodburnAdjDens; // output variables, persons per acre

   // Loop thru the IDUs, and set POP and POPDENS for the ones affected by the population growth model
   for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
      {
      float area;
      int pop;

      int rpaID;
      pLayer->GetData( idu, m_colRPA,  rpaID );
      if (rpaID>0 && rpaID<MAX_RR_DH_ID)
         {
         pLayer->GetData( idu, m_colAREA,  area );
         pop = (int)(area*m_RPAdensities[rpaID]);
         UpdateIDU( pContext, idu, m_colPopDens, m_RPAdensities[rpaID], true);
         UpdateIDU( pContext, idu, m_colPOP, pop, true);
         }
      else 
         {
         int ugb;
         pLayer->GetData( idu, m_colUGB, ugb );
         if (ugb>0 && ugb<=MAX_UGA_NDX) // && ugb != UGB_METRO && ugb != UGB_EUGENESPRINGFIELD)
            {
            int owner, lulc_a;
            pLayer->GetData( idu, m_colOWNER, owner );
            pLayer->GetData( idu, m_colLulcA, lulc_a );
            if ((owner==OWNER_PNI || owner==OWNER_PRIVATE_INDUSTRIAL) && lulc_a==LULCA_DEVELOPED)
               {
               pLayer->GetData( idu, m_colAREA, area );
               pop = (int)(area*m_UGBdensities[ugb]);
               UpdateIDU( pContext, idu, m_colPopDens, m_UGBdensities[ugb], true);
               UpdateIDU( pContext, idu, m_colPOP, pop, true);
              } // end of if (owner==PRIVATELY_OWNED && lulc_a==DEVELOPED)
            } // end of if (ugb>0 && ugb<MAX_UGB)
         } // end of if (rpaID>0 ...) else 
      } // end of loop on IDUs
     
   return true;
   } // end of RunPopGrowth()


void APs::CalculatePopGrowthOutputVariables(EnvContext *pContext)
   {
   // Output variables - UGB adjusted densities in persons per acre for the 8 biggest UGBs
   m_MetroAdjDens = m_UGBadjPopDensities[UGB_METRO]*M2_PER_ACRE;
   m_EugeneSpringfieldAdjDens = m_UGBadjPopDensities[UGB_EUGENESPRINGFIELD]*M2_PER_ACRE;
   m_SalemAdjDens = m_UGBadjPopDensities[UGB_SALEM]*M2_PER_ACRE;
   m_CorvallisAdjDens = m_UGBadjPopDensities[UGB_CORVALLIS]*M2_PER_ACRE;
   m_AlbanyAdjDens = m_UGBadjPopDensities[UGB_ALBANY]*M2_PER_ACRE;
   m_McMinnvilleAdjDens = m_UGBadjPopDensities[UGB_MCMINNVILLE]*M2_PER_ACRE;
   m_NewbergAdjDens = m_UGBadjPopDensities[UGB_NEWBERG]*M2_PER_ACRE;
   m_WoodburnAdjDens = m_UGBadjPopDensities[UGB_WOODBURN]*M2_PER_ACRE;

   int nRows = m_PGoutVars.GetRowCount();
   int rowNdx = pContext->yearOfRun<0 ? 0 : pContext->yearOfRun;
   while (nRows<=rowNdx) 
      {
      nRows = m_PGoutVars.AppendRows(1);
      if (m_PGtestMode>0)
         {
         CString msg;
         msg.Format("PopGrowth: nRows, rowNdx, yearOfRun, currentYear = %d, %d, %d, %d", nRows, rowNdx, pContext->yearOfRun, pContext->currentYear);
         Report::LogMsg(msg);
         }
      }
   m_PGoutVars.Set(0, rowNdx, (float)pContext->currentYear);
   m_PGoutVars.Set(1, rowNdx, m_MetroAdjDens);
   m_PGoutVars.Set(2, rowNdx, m_EugeneSpringfieldAdjDens);
   m_PGoutVars.Set(3, rowNdx, m_SalemAdjDens);
   m_PGoutVars.Set(4, rowNdx, m_CorvallisAdjDens);
   m_PGoutVars.Set(5, rowNdx, m_AlbanyAdjDens);
   m_PGoutVars.Set(6, rowNdx, m_McMinnvilleAdjDens);
   m_PGoutVars.Set(7, rowNdx, m_NewbergAdjDens);
   m_PGoutVars.Set(8, rowNdx, m_WoodburnAdjDens);

   } // end of CalculatePopGrowthOutputVariables()


void APs::CalculateLTOutputVariables(EnvContext *pContext)
   {
   int nRows = m_LToutVars.GetRowCount();
   int rowNdx = pContext->yearOfRun<0 ? 0 : pContext->yearOfRun;
   while (nRows<=rowNdx) 
      {
      nRows = m_LToutVars.AppendRows(1);
      if (m_LTtestMode>0)
         {
         CString msg;
         msg.Format("Land Use Transitions: nRows, rowNdx, yearOfRun, currentYear = %d, %d, %d, %d", nRows, rowNdx, pContext->yearOfRun, pContext->currentYear);
         Report::LogMsg(msg);
         }
      }
   m_LToutVars.Set(0, rowNdx, (float)pContext->currentYear);
   m_LToutVars.Set(1, rowNdx, m_LTagCount>0 ? (float)m_LTag2devCount/(float)m_LTagCount : 0.f);
   m_LToutVars.Set(2, rowNdx, m_LTagCount>0 ? (float)m_LTag2forestCount/(float)m_LTagCount : 0.f);
   m_LToutVars.Set(3, rowNdx, m_LTforestCount>0 ? (float)m_LTforest2devCount/(float)m_LTforestCount : 0.f);
   m_LToutVars.Set(4, rowNdx, m_LTforestCount>0 ? (float)m_LTforest2agCount/(float)m_LTforestCount : 0.f);

   } // end of CalculateLTOutputVariables()


bool APs::InitIrrChoice( EnvContext *pContext )
   {
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;
   bool recognized;
   CString msg;

   // Set the default values of the coefficients.
   m_IrrBeta[0] = 2.24871; // description="intercept"
   m_IrrBeta[1] = -0.09432; // description="Precip dev, June"
   m_IrrBeta[2] = -0.41341; // description="Precip dev, July"
   m_IrrBeta[3] = -0.15841; // description="Precip dev, August" />
   m_IrrBeta[4] = -0.0098; // description="Elevation (meters)" />
   m_IrrBeta[5] = 0.00083; // description="Elevation*spring precip (avg. Apr-Jun" />
   m_IrrBeta[6] = 0.43069; // description="EFU (0/1)" />
   m_IrrBeta[7] = 0.33113; // description="Groundwater right (0/1)" />
   m_IrrBeta[8] = -0.00842; // description="Poor drainage (%)" />
   m_IrrBeta[9] = -0.00949; // description="Groundwater-right*depth-to-groundwater (m)" />
   m_IrrBeta[10] = -0.21901; // description="Water holding capacity (cmH2O)" />
   m_IrrBeta[11] = -0.02615; // description="Distance to Willamette R. (km)" />
   m_IrrBeta[12] = -0.00873; // description="Distance to nearest major city (km)" />
   m_IrrBeta[13] = 0.83972; // description="LCC1 (0/1)" />
   m_IrrBeta[14] = 1.07992; // description="LCC2 (0/1)" />
   m_IrrBeta[15] = 0.82066; // description="LCC3 (0/1)" />
   m_IrrBeta[16] = 1.4981; // description="LCC4 (0/1)" />

   // Interpret the coefficients from the XML file.
   for (int i = 0; i<m_IrrCoefficientArray.GetSize(); i++)
      { // Interpret coefficients beta0, beta1, ..., beta16
      CString symbol = m_IrrCoefficientArray[i]->m_symbol;
      int n = symbol.GetLength();
      recognized = n>=5 && n<=6 && symbol.Left(4)=="beta"; // _T("beta") ?
      int tens_digit = n==5 ? 0 : symbol[4] - '0';
      int ones_digit = symbol[n==5 ? 4 : 5] - '0';
      recognized &= 0<=tens_digit && tens_digit<=1 && 0<=ones_digit && ones_digit<=9;
      int index = 10*tens_digit + ones_digit;
      recognized &= index<=16;
      if (!recognized)
         {
         CString msg;
         msg.Format("APs Irrigation Choice: unrecognized coefficient symbol %s.  Recognizable symbols are beta0, ... beta16", (LPCTSTR) symbol);
         Report::ErrorMsg(msg);
         return false;
         }
      ASSERT(0<=index && index<=16);
      m_IrrBeta[index] = m_IrrCoefficientArray[i]->m_coef_value;
      }

   // Get the one time-dependent coefficient for this model.
   recognized = m_TDcoefficient.m_symbol=="Eyr";
   if (!recognized)
      {
      msg.Format("APs Irrigation Choice: unrecognized time dependent coefficient symbol %s."  
            "  The only recognizable symbol is Eyr, the energy cost index.", (LPCTSTR) m_TDcoefficient.m_symbol);
      Report::ErrorMsg(msg);
      return false;
      }

   bool rtnFlag = recognized;

   // Deal with test mode
   if (m_IrrTestMode>0) InitializeTestMode(pLayer, 16, 0);

   return rtnFlag;
   } // end of InitIrrChoice()


bool APs::InitRunIrrChoice( EnvContext *pContext )
{
MapLayer *pLayer = (MapLayer*)pContext->pMapLayer;
CString msg;

if (m_currentIrrScenarioIndex == -1) m_currentIrrScenarioIndex = 0;

/*
// Deal with test mode
// This logic has been placed here in InitRunIrrChoice(), instead of in InitIrrChoice(), to ensure
// that it runs after Flow and WaterMaster have been initialized.
if (m_IrrTestMode == 1)
   { // Needs m_colWREXISTS, m_colIDU_ID, m_colAREA
   // Open wr_pous.csv (the POU file).
   int pou_records = 0;
   VDataObj pou_table;
   CString POU_file = "wr_pous.csv";
   msg.Format("APs: Opening the POU file %s", (LPCTSTR)POU_file);
   Report::LogMsg(msg, RT_INFO);
   pou_records = pou_table.ReadAscii(POU_file, ',', TRUE);
   msg.Format("APs: pou_records = %d", pou_records);
   Report::LogMsg(msg, RT_INFO);
   int pouColIDU_ID = pou_table.GetCol("IDU_ID");
   int pouColPOU_ID = pou_table.GetCol("POU_ID");
   int pouColAREA = pou_table.GetCol("AREA");
   int pouColPERCENT = pou_table.GetCol("PERCENT");

   // Open wr_pods.csv (the POD file).
   int pod_records = 0;
   VDataObj pod_table;
   CString POD_file = "wr_pods.csv";
   msg.Format("APs: Opening the POD file %s", (LPCTSTR)POD_file);
   Report::LogMsg(msg, RT_INFO);
   pod_records = pod_table.ReadAscii(POD_file, ',', TRUE);
   msg.Format("APs: pod_records = %d", pod_records);
   Report::LogMsg(msg, RT_INFO);
   int podColPOUID = pod_table.GetCol("POUID");
   int podColWATERRIGHTID = pod_table.GetCol("WATERRIGHTID");
   int podColPODID = pod_table.GetCol("PODID");
   int podColUSE = pod_table.GetCol("USE");
   int podColYEAR = pod_table.GetCol("YEAR");

   if (pou_records < 1 || pod_records < 1 || pouColIDU_ID < 0 || pouColPOU_ID < 0 || pouColAREA < 0 || pouColPERCENT < 0 ||
      podColPOUID < 0 || podColWATERRIGHTID < 0 || podColPODID < 0 || podColUSE < 0 || podColYEAR < 0)
      {
      msg.Format("APs: problem with POU or POD file. pou_records = %d, pod_records = %d, pouColIDU_ID = %d, pouColPOU_ID = %d, pouColAREA = %d, pouColPERCENT = %d,"
         " podColPOUID = %d, podColWATERRIGHTID = %d, podColPODID = %d, podColUSE = %d, podColYEAR = %d\n",
         pou_records, pod_records, pouColIDU_ID, pouColPOU_ID, pouColAREA, pouColPERCENT,
         podColPOUID, podColWATERRIGHTID, podColPODID, podColUSE, podColYEAR);
      Report::ErrorMsg(msg);
      }

   // Create "WaterRights.csv" (the output file).
   PCTSTR Irr_WR_file = (PCTSTR)"WaterRights.csv";
   FILE *oFile = NULL;
   int errNo = fopen_s(&oFile, Irr_WR_file, "w");
   if (errNo != 0)
      {
      CString msg(" APs:ColdStart -  ERROR: Could not open output file ");
      msg += Irr_WR_file;
      Report::ErrorMsg(msg);
      return false;
      }
   fprintf(oFile, "IDU_INDEX, IDU_AREA, POUID, OVERLAP_AREA, POU_PERCENT, WATERRIGHTID, PODID, USE, YEAR\n");

   // loop thru the IDUs: for each irrigable IDU, find its entries in the POU file
   //    write out as many rows as there are POUIDs in the POU file for this IDU: for each POUID, find its entries in the POD file
   //       in each row, write out these columns:
   //          IDU_INDEX
   //          POUID, AREA, PERCENT (from the POU file)
   //          WATERRIGHTID, PODID, USE, PRIORITY DATE (from the POD file)
   for (MapLayer::Iterator idu = pLayer->Begin(); idu != pLayer->End(); idu++)
      {
      int wr;
      pLayer->GetData(idu, m_colWREXISTS, wr);

      int idu_id = -1; pLayer->GetData(idu, m_colIDU_ID, idu_id);
      float idu_area = 0.f; pLayer->GetData(idu, m_colAREA, idu_area);

      bool found_pou = false;
      for (int pouRow = 0; pouRow < pou_records; pouRow++)
         {
         int pou_idu_id = pou_table.GetAsInt(pouColIDU_ID, pouRow);
         if (pou_idu_id==idu_id)
            {
            int pouid = -1; pouid = pou_table.GetAsInt(pouColPOU_ID, pouRow);
            float area = -1.f; area = pou_table.GetAsFloat(pouColAREA, pouRow);
            float percent = -1.f; percent = pou_table.GetAsFloat(pouColPERCENT, pouRow);
            found_pou = true;
            int count = 0;
            for (int podRow = 0; podRow < pod_records; podRow++)
               {
               int pod_pouid = -1; pod_pouid = pod_table.GetAsInt(podColPOUID, podRow);
               if (pod_pouid == pouid)
                  {
                  int waterrightid = -1; waterrightid = pod_table.GetAsInt(podColWATERRIGHTID, podRow);
                  int podid = -1; podid = pod_table.GetAsInt(podColPODID, podRow);
                  CString use; use = pod_table.GetAsString(podColUSE, podRow);
                  int year; year = pod_table.GetAsInt(podColYEAR, podRow);
                  fprintf(oFile, "%d, %f, %d, %f, %f, %d, %d, %s, %d\n",
                        idu_index, idu_area, pouid, area, percent, waterrightid, podid, (LPCSTR)use, year);
                  count++;
                  } // end of if (pod_pouid==pouid)
               } // end of loop on podRow
            msg.Format("APs: pouid %d for idu %d has %d PODs\n", pouid, idu_index, count);
            if (count<=0) Report::LogMsg(msg);
            } // end of if (pou_idu==idu_index) 
         } // end of loop on pouRow
      if (wr!=0 && !found_pou)
         {
         msg.Format("APs: WREXISTS!=0 and !found_pou (WREXISTS = 0x%X\n", wr);
         Report::WarningMsg(msg);
         }
      } // end of loop thru idus

   // close the output file
   fclose(oFile);
       
   } // end of if (m_IrrTestMode==1)
*/

   return(true); 
} // end of InitRunIrrChoice()


bool APs::RunIrrChoice( EnvContext *pContext )
   {
   testMessage(pContext, _T("RunIrrChoice"));
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;

   float Eyr = m_TDcoefficient.Get((float)pContext->currentYear);

   // Loop thru the IDUs, and set IRRIGATION to 1 or 0 for the ones for which LULC_A = {Agriculture}, IRP0>=$1, and there is an associated irrigation water right,
   // except when the crop scenario is 1 (all fallow), don't irrigate anything.
   // Monthly layers in the input file are indexed starting from zero.
   pLayer->SetColDataU(m_colIRRIGATION, 0);
   for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
      {
      int lulc_a;
      pLayer->GetData(idu, m_colLulcA, lulc_a);
      if (lulc_a != LULCA_AGRICULTURE) continue;

      float irp = 0.f;
      if (pContext->yearOfRun == 0) pLayer->GetData(idu, m_colIRP0, irp);
      else pLayer->GetData(idu, m_colIRP, irp);

      float min_irp = 1.0f;
      float min_irp_factor; pLayer->GetData(idu, m_colXIRP_MIN, min_irp_factor);

      for (int Xndx = 0; Xndx<=16; Xndx++)
         m_IrrX[Xndx] = 0.;
      
      double betaX_term = 0.;
      double P_Irr = 0.;

      int water_right;
      pLayer->GetData( idu, m_colWREXISTS,  water_right );
      float long_term_farmland_rent = 0.;
      if (IsIrrigable(water_right)) pLayer->GetData( idu, m_colFRMRNT_LR, long_term_farmland_rent );
      int irrChoice;
      if ((irp >= min_irp*min_irp_factor /* $ per acre */) && m_currentCropScenarioIndex == 0 && IsIrrigable(water_right) && (long_term_farmland_rent >= 1.))
         {
         // Populate the independent variables x0, x1, x2, ..., x16
         m_IrrX[0] = 1.;

         float prcp_jun; pLayer->GetData(idu, m_colPRCP_JUN, prcp_jun);
         float prcpjunavg; pLayer->GetData(idu, m_colPRCPJUNAVG, prcpjunavg);
         m_IrrX[1] = (prcp_jun - prcpjunavg)*IN_PER_MM;

         float prcp_jul; pLayer->GetData(idu, m_colPRCP_JUL, prcp_jul);
         float prcpjulavg; pLayer->GetData(idu, m_colPRCPJULAVG, prcpjulavg);
         m_IrrX[2] = (prcp_jul - prcpjulavg)*IN_PER_MM;

         float prcp_aug; pLayer->GetData(idu, m_colPRCP_AUG, prcp_aug);
         float prcpaugavg; pLayer->GetData(idu, m_colPRCPAUGAVG, prcpaugavg);
         m_IrrX[3] = (prcp_aug - prcpaugavg)*IN_PER_MM;

         float elev;
         pLayer->GetData( idu, m_colELEV, elev );
         m_IrrX[4] = elev;

         float prcpspring; pLayer->GetData(idu, m_colPRCPSPRING, prcpspring);
         m_IrrX[5] = elev*prcpspring/3.; // m_PrcpSpring is the total for the 3 months of spring

         int zone; 
         pLayer->GetData( idu, m_colZONED_USE,  zone );
         m_IrrX[6] = zone==ZONE_AG; 

         m_IrrX[7] = IsIrrigableFromGroundwater(water_right) ? 1 : 0;

         pLayer->GetData( idu, m_colDRAINAGE, m_IrrX[8] );

         if (IsIrrigableFromGroundwater(water_right)) 
            {
            float depth_to_groundwater;
            pLayer->GetData( idu, m_colD_GWATER, depth_to_groundwater );
            m_IrrX[9] = depth_to_groundwater*Eyr;
            }
         else m_IrrX[9] = 0;

         pLayer->GetData( idu, m_colAWS, m_IrrX[10] );

         double d_wriver_meters;
         pLayer->GetData( idu, m_colD_WRIVER, d_wriver_meters );
         m_IrrX[11] = d_wriver_meters/1000.; // Convert from m to km.

         double d_city_meters;
         pLayer->GetData( idu, m_colD_CITY_50K, d_city_meters );
         m_IrrX[12] = d_city_meters/1000.; // Convert from m to km.

         int lcc;
         pLayer->GetData( idu, m_colLCC, lcc );
         for (int i=13; i<=16; i++) m_IrrX[i] = lcc==(i-12) ? 1 : 0;

         for (int i=0; i<=16; i++) betaX_term += m_IrrBeta[i]*m_IrrX[i];

         double P_Irr; // Probability of irrigating
         double prob_of_not_irrigating;
         switch (m_currentIrrScenarioIndex)
            {
            case 0: // Ref (~2/3)
               P_Irr = 1. / (1. + exp(-betaX_term));
               break;
            case 1: // More (~5/6)
               prob_of_not_irrigating = (1. - (1. / (1. + exp(-betaX_term)))) / 2.;
               P_Irr = 1. - prob_of_not_irrigating;
               break;
            default:
            case 2: // None (0)
               P_Irr = 0;
               break;
            }
         
         double rand_num = (double) m_IrrRandomDraw.RandValue(0., 1. );  
         irrChoice = rand_num<P_Irr ? 1 : 0;
         } // end of if (IsIrrigable(water_right) && long_term_farmland_rent>=1.) 
      else irrChoice = 0;

      UpdateIDU( pContext, idu, m_colIRRIGATION, irrChoice, true );

      // Deal with test mode
      if (m_IrrTestMode==1)
         { // Stash the independent variable values into attributes X0, X1, ...
         for (int Xndx = 0; Xndx<=16; Xndx++) 
               UpdateIDU( pContext, idu, m_colX0+Xndx, (float)m_IrrX[Xndx], true );
         UpdateIDU( pContext, idu, m_colBETAX, (float)betaX_term, true ); 
         UpdateIDU( pContext, idu, m_colPROB0, (float)P_Irr, true ); 
         } // end of if (m_IrrTestMode==1)

      } // end of loop on IDUs

   return true;
   } // end of RunIrrChoice()


bool APs::InitFR( EnvContext *pContext ) 
   { 
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;

   bool rtnFlag;
   CString msg;

   // Set the default values of the coefficients.
   m_FR_a[0] = 1.206; // description="intercept"
   m_FR_a[1] = 104.7; // description="LCC1 (0/1)"
   m_FR_a[2] = 95.6; // description="LCC2 (0/1)"
   m_FR_a[3] = 69.9; // description="LCC3 (0/1)" />
   m_FR_a[4] = 66.6; // description="LCC4 (0/1)" />
   m_FR_a[5] = 0; // description="LCC5 (0/1)" />
   m_FR_a[6] = 20.5; // description="LCC6 (0/1)" />
   m_FR_a[7] = 19.9; // description="LCC7 (0/1)" />
   m_FR_a[8] = 38.9; // description="IRR*LCC1" />
   m_FR_a[9] = 39.1; // description="IRR*LCC2" />
   m_FR_a[10] = 17.8; // description="IRR*LCC3" />
   m_FR_a[11] = 22.1; // description="IRR*LCC4" />
   m_FR_a[12] = -0.04; // description="ElevDeMean" />
   m_FR_a[13] = 1.37; // description="PrDeMean" />
   m_FR_a[14] = 26.6; // description="MinTempDeMean" />
   m_FR_a[15] = 93.68; // description="mean elevation" />
   m_FR_a[16] = 13.49; // description="mean growing season precip, inH2O" />
   m_FR_a[17] = 8.581; // description="mean growing season tmin, degC" />
   m_FR_a[18] = 2.0; // description="placeholder coefficient in pumping cost expression" />

   rtnFlag = InterpretXmlCoefficients("a", m_FR_a, sizeof(m_FR_a)/sizeof(double), m_FRcoefficientArray);

   // Get the one time-dependent coefficient for this model.
   bool recognized = m_TDcoefficient.m_symbol=="Eyr";
   if (!recognized)
      {
      msg.Format("APs Farmland Rent: unrecognized time dependent coefficient symbol %s."  
            "  The only recognizable symbol is Eyr, the energy cost index.", (LPCTSTR) m_TDcoefficient.m_symbol);
      Report::ErrorMsg(msg);
      return false;
      }

   if (m_FRtestMode>0) InitializeTestMode(pLayer, 14, 0);

   rtnFlag = recognized;
   return(rtnFlag); 
   } // end of InitFR()


bool APs::InitRunFR( EnvContext *pContext ) 
   { 
   m_FR_Eyr0 = m_TDcoefficient.Get((float)pContext->startYear);
   m_FRoutVars.ClearRows();
   return(true); 
   }


bool APs::RunFR( EnvContext *pContext ) 
   { 
   testMessage(pContext, _T("RunFR"));
   /*
   If necessary, update the gridded climate means.
   For each IDU such that LULC_A = 2 {Ag}
      Update WR_SH_NDX from WR_SHUTOFF
      Calculate FRMRNT_LR and FRMRNT_YR
   */
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;

   float tau = pContext->yearOfRun > m_tauWR ? m_tauWR : pContext->yearOfRun+1;
   float Eyr = m_TDcoefficient.Get((float)pContext->currentYear);
   float Eterm = (Eyr - m_FR_Eyr0)/m_FR_Eyr0;
   float Cw_ground = m_FR_Cw_ground.Get((float)pContext->currentYear);
   float Cw_surface = m_FR_Cw_surface.Get((float)pContext->currentYear);

   // for Dave Turner 2/3/16
   double winter_prcpXarea_accum = 0.;
   double area_accum = 0.;

   for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
      {
      // for Dave Turner 2/3/16
      float idu_area = 0.f; pLayer->GetData(idu, m_colAREA, idu_area);
      float prcpwinter = 0; pLayer->GetData(idu, m_colPRCPWINTER, prcpwinter);
      area_accum += (double)idu_area;
      winter_prcpXarea_accum += (double)(prcpwinter * idu_area);

      int lulc_a;
      pLayer->GetData( idu, m_colLulcA,  lulc_a );
      if (lulc_a!=LULCA_AGRICULTURE && lulc_a!=LULCA_FOREST) continue;

      int wr_shutoff;
      pLayer->GetData( idu, m_colWR_SHUTOFF,  wr_shutoff );
      float wr_sh_ndx;
      pLayer->GetData( idu, m_colWR_SH_NDX, wr_sh_ndx);

      //ASSERT(wr_shutoff==1 || wr_shutoff==0);

      if ( wr_shutoff > 1 )
         wr_shutoff = 1;

      wr_sh_ndx = wr_sh_ndx*exp(-1/tau) + wr_shutoff*(1 - exp(-1/tau));
      UpdateIDU( pContext, idu, m_colWR_SH_NDX, wr_sh_ndx, true);

      float elev, d_gwater, irr_h2o_cost;
      int lcc, wrexists;  
      pLayer->GetData( idu, m_colELEV, elev );
      pLayer->GetData( idu, m_colD_GWATER, d_gwater );
      pLayer->GetData( idu, m_colLCC, lcc );

      pLayer->GetData( idu, m_colWREXISTS, wrexists );
      float Cw = IsIrrigableFromGroundwater(wrexists) ? Cw_ground : Cw_surface; // Cw is this year's cost of irrigation water.
      pLayer->GetData( idu, m_colIRRH2OCOST, irr_h2o_cost ); // irr_h2o_cost is previous cost of irrigation water.

      // Calculate current irrigation rent premium
      float pc = IsIrrigableFromGroundwater(wrexists) ? (float)(Eterm*m_FR_a[18] * d_gwater) : 0; // pumping costs
      float irp0 = 0.f; pLayer->GetData(idu, m_colIRP0, irp0); // irrigation rent premium before costs
      float irp = irp0 - Cw - pc; if (irp < 0.f) irp = 0.f;
      pLayer->m_readOnly = false; pLayer->SetData(idu, m_colIRP, irp); pLayer->m_readOnly = true;
      UpdateIDU(pContext, idu, m_colIRP, irp, true); // Record the change in the delta array

      float ecc = 0.f; pLayer->GetData(idu, m_colECC, ecc); // extra conveyance cost
      float effective_irp = (IsIrrigable(wrexists) && irp>ecc) ? (irp - ecc) : 0.f;
      float lcc_term = (1 <= lcc && lcc <= 7) ? (float)m_FR_a[lcc] : 0.f;

      float prcpgroavg; pLayer->GetData(idu, m_colPRCPGROAVG, prcpgroavg);
      float tmingroavg; pLayer->GetData(idu, m_colTMINGROAVG, tmingroavg);
      float common_term1 = (float)(m_FR_a[0] // intercept
         + lcc_term
         + effective_irp // irrigation rent premium
         + m_FR_a[12] * (elev - m_FR_a[15]) // elevDeMean term
			+ (m_FR_a[13]*(prcpgroavg - m_BasinMeanPrcpGrowingSeason)) // PrDeMean term
			+ (m_FR_a[14]*(tmingroavg - m_BasinMeanTminGrowingSeason))); // MinTempDeMean term
      
      // Calculate and store the probability of adding a new stored water right.
      if (lulc_a == LULCA_AGRICULTURE && pContext->currentYear <= 2045 && pContext->currentYear >= 2015 && !IsIrrigable(wrexists))
         {
         float prob_of_new_WR;
         if (pContext->currentYear == 2045) prob_of_new_WR = 0.f;
         else
            {
            float lift = 0.f; pLayer->GetData(idu, m_colLIFT, lift);
            if (lift < 0.f) lift = 0.f;
            float distance = 0.f; pLayer->GetData(idu, m_colDISTANCE, distance);
            float pE = 0.06f; // $/kwh
            float ecc0 = Eterm * pE * (0.038f*distance + 0.0497f*lift) + 0.1148f*distance;
            if (m_dynamicWRType == 4) ecc0 /= 2.f;
            UpdateIDU(pContext, idu, m_colECC0, ecc0, true);
            float L = 0.25f; // 1.0f; // 0.25f; // 0.025f;
            float k = 0.2f; // 0.4f;
            float x0 = 10.f; // 20.f;
            float pBOR = 9.00f; // $/acre
            if (m_dynamicWRType == 4) pBOR = 0.f;
            float x = irp - ecc0 - pBOR;
            if (x < 1.f) prob_of_new_WR = 0.f;
            else prob_of_new_WR = L / (1.f + exp(-k*(x - x0)));
            }
         UpdateIDU(pContext, idu, m_colWR_PROB, prob_of_new_WR, true); 
         }

      float common_term2 = IsIrrigable(wrexists) ? irp : 0.f;

      float frmrnt_yr = common_term1 + (wr_shutoff==0 ? common_term2 : 0.0f);
      float frmrnt_lr = common_term1 + (1.f - wr_sh_ndx)*common_term2;
/*      
      int colIDU_INDEX, idu_index;
      CheckCol(pLayer, colIDU_INDEX, "IDU_INDEX", TYPE_INT, CC_MUST_EXIST);
      pLayer->GetData(idu, colIDU_INDEX, idu_index);
      if (idu_index == 69884)
      {
         CString msg; 
         msg.Format("*** RunFR() idu_index = %d, common_term1 = %f, common_term2 = %f, frmrnt_lr = %f, m_FR_a[12] = %f", 
               idu_index, common_term1, common_term2, frmrnt_lr, m_FR_a[12]);
         Report::LogMsg(msg);
      }
*/

      UpdateIDU( pContext, idu, m_colFRMRNT_YR, frmrnt_yr , true);
      UpdateIDU( pContext, idu, m_colFRMRNT_LR, frmrnt_lr, true );
      if (lulc_a==2 && Cw!=irr_h2o_cost) UpdateIDU( pContext, idu, m_colIRRH2OCOST, Cw, true );

      // Deal with test mode
      if (m_FRtestMode==1)
         { // Stash the independent variable values into attributes X0, X1, ...
         UpdateIDU( pContext, idu, m_colX0, 1.f, true );
         for (int Xndx = 1; Xndx<=7; Xndx++) 
               UpdateIDU( pContext, idu, m_colX0+Xndx, (float)(lcc==Xndx ? 1 : 0), true );
         for (int Xndx = 8; Xndx<=11; Xndx++) 
               UpdateIDU( pContext, idu, m_colX0+Xndx, (float)((lcc==(Xndx-7) && IsIrrigable(wrexists)) ? 1 : 0)*(1.f - pc), true );
         UpdateIDU( pContext, idu, m_colX0+12, (float)(elev - m_FR_a[15]), true );
			UpdateIDU(pContext, idu, m_colX0 + 13, (float)(prcpgroavg - m_BasinMeanPrcpGrowingSeason), true);
			UpdateIDU(pContext, idu, m_colX0 + 14, (float)(tmingroavg - m_BasinMeanTminGrowingSeason), true);
         } // end of if (m_FRtestMode==1)

      } // end of loop thru IDUs

   float winterPrcpOverWRB_mm = (float)(winter_prcpXarea_accum / area_accum);

   // Calculate econ metrics
   // 1) water scarcity at efficient margin
   // i.e. the minimum decrease in welfare associated with loss of the ability to irrigate from surface water.
   // Find the irrigable IDU with the minimum irrigation rent premium.
   // 2) water scarcity at legal margin for irrigation water rights
   // i.e. the irrigation rent premium for the IDU with the most junior surface irrigation water right
   // 3) welfare loss for irrigation at regulated margin
   // i.e. the sum of the (IRPxAREA) for the IDUs whose irrigation WRs have been shut off in the current year
   float irp_min = 1.e10f; // minimum irrigation rent premium value, $/acre
   int idu_with_irp_min = -1; // IDU having minimum irrigation rent premium
   float irp_min_LCC1thru4 = 1.e10f; // minimum irrigation rent premium value, $/acre, for IDUs with LCC=1,2,3,4
   int idu_with_irp_min_LCC1thru4 = -1; // IDU having minimum irrigation rent premium, for IDUs with LCC=1,2,3,4
   int most_jr_WR_seq_num = 0;
   int most_jr_WR_idu = -1; 
   float most_jr_WR_irp = -1.f; // $/ac
   float irrig_welfare_loss = 0.f; // $
   for (MapLayer::Iterator idu = pLayer->Begin(); idu != pLayer->End(); idu++)
      {
      int lulc_a = -1; pLayer->GetData(idu, m_colLulcA, lulc_a);
      int wr_irrig_s = -1;  pLayer->GetData(idu, m_colWR_IRRIG_S, wr_irrig_s);
      if (lulc_a != LULCA_AGRICULTURE || wr_irrig_s == 0) continue;

      float irp = -1;  pLayer->GetData(idu, m_colIRP, irp);
      int lcc = -1; pLayer->GetData(idu, m_colLCC, lcc);
      if (0 < lcc && irp < irp_min)
         {
         irp_min = irp;
         idu_with_irp_min = idu;
         }
      if (1 <= lcc && lcc <= 4 && irp < irp_min_LCC1thru4)
         {
         irp_min_LCC1thru4 = irp;
         idu_with_irp_min_LCC1thru4 = idu;
         }

      int seq_num = 0; pLayer->GetData(idu, m_colWRPRIORITY, seq_num);
      if (seq_num > most_jr_WR_seq_num)
         {
         most_jr_WR_seq_num = seq_num;
         most_jr_WR_idu = idu;
         most_jr_WR_irp = irp;
         }

      int wr_shutoff = 0; pLayer->GetData(idu, m_colWR_SHUTOFF, wr_shutoff);
      if (wr_shutoff == 0) continue;
      float area_m2 = 0.f; pLayer->GetData(idu, m_colAREA, area_m2);
      irrig_welfare_loss += irp * (area_m2 * ACRE_PER_M2);

      } // end of loop thru IDUs to calculate econ metrics
   
   CArray< float, float > rowOutVar;
   rowOutVar.SetSize(10);
   rowOutVar[0] = (float)pContext->currentYear;
   rowOutVar[1] = irp_min;
   rowOutVar[2] = (float)idu_with_irp_min;
   rowOutVar[3] = irp_min_LCC1thru4;
   rowOutVar[4] = (float)idu_with_irp_min_LCC1thru4;
   rowOutVar[5] = most_jr_WR_irp;
   rowOutVar[6] = (float)most_jr_WR_idu;
   rowOutVar[7] = (float)most_jr_WR_seq_num;
   rowOutVar[8] = irrig_welfare_loss;
   rowOutVar[9] = winterPrcpOverWRB_mm;
   m_FRoutVars.AppendRow(rowOutVar);

   return(true); 
   } // end of RunFR()


bool APs::InitCrop( EnvContext *pContext )
   {
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;
//   bool recognized;
   CString msg;

   m_records_crop = m_crop_table.ReadAscii(m_cropfile, ',', TRUE);
   if (m_records_crop<=0)
      {
      msg.Format("APs Crop Choice: missing or empty coefficients file\n"
         "m_records_crop = %d\n", m_records_crop);
      Report::ErrorMsg( msg );
      return false;
      }

   bool err = false;
   m_colCrop_beta = m_crop_table.GetCol("beta"); err |= m_colCrop_beta<0; // ASSERT(!err);
   m_cropCols[CROP_GRASS_SEED] = m_colCrop_grass_seed = m_crop_table.GetCol("grass seed"); err |= m_colCrop_grass_seed<0; // ASSERT(!err);
   m_cropCols[CROP_PASTURE] = m_colCrop_pasture = m_crop_table.GetCol("pasture"); err |= m_colCrop_pasture<0; // ASSERT(!err);
   m_cropCols[CROP_WHEAT] = m_colCrop_wheat = m_crop_table.GetCol("wheat"); err |= m_colCrop_wheat<0; // ASSERT(!err);
   m_cropCols[CROP_FALLOW] = m_colCrop_fallow = m_crop_table.GetCol("fallow"); err |= m_colCrop_fallow<0; // ASSERT(!err);
   m_cropCols[CROP_CORN] = m_colCrop_corn = m_crop_table.GetCol("corn"); err |= m_colCrop_corn<0; // ASSERT(!err);
   m_cropCols[CROP_CLOVER] = m_colCrop_clover = m_crop_table.GetCol("clover"); err |= m_colCrop_clover<0; // ASSERT(!err);
   m_cropCols[CROP_HAY] = m_colCrop_hay = m_crop_table.GetCol("hay"); err |= m_colCrop_hay<0; // ASSERT(!err);
   if (err)
      {
      msg.Format("APs:ColdStart - One or more missing columns in the crop choice coefficients file.\n"
            "m_colCrop_beta, _grass_seed, _pasture, _wheat, _fallow, _corn, _clover, _hay = %d, %d, %d, %d, %d, %d, %d, %d", 
            m_colCrop_beta, m_colCrop_grass_seed, m_colCrop_pasture, m_colCrop_wheat, m_colCrop_fallow, m_colCrop_corn, m_colCrop_clover, m_colCrop_hay);
      Report::ErrorMsg(msg);
      return false;
      }

   for (int betaNdx=0; betaNdx<=14; betaNdx++)
      {
      int tableNdx = m_crop_table.GetAsInt(m_colCrop_beta, betaNdx);
      if (betaNdx!=tableNdx) 
         {
         msg.Format("APs Crop Choice - crop coefficient table is out of order. betaNdx, tableNdx = %d, %d", betaNdx, tableNdx);
         Report::ErrorMsg(msg);
         return false;
         }

      m_cropBeta[betaNdx][CROP_GRASS_SEED] = m_crop_table.GetAsFloat(m_cropCols[CROP_GRASS_SEED], betaNdx);
      m_cropBeta[betaNdx][CROP_PASTURE] = m_crop_table.GetAsFloat(m_cropCols[CROP_PASTURE], betaNdx);
      m_cropBeta[betaNdx][CROP_WHEAT] = m_crop_table.GetAsFloat(m_cropCols[CROP_WHEAT], betaNdx);
      m_cropBeta[betaNdx][CROP_FALLOW] = m_crop_table.GetAsFloat(m_cropCols[CROP_FALLOW], betaNdx);
      m_cropBeta[betaNdx][CROP_CORN] = m_crop_table.GetAsFloat(m_cropCols[CROP_CORN], betaNdx);
      m_cropBeta[betaNdx][CROP_CLOVER] = m_crop_table.GetAsFloat(m_cropCols[CROP_CLOVER], betaNdx);
      m_cropBeta[betaNdx][CROP_HAY] = m_crop_table.GetAsFloat(m_cropCols[CROP_HAY], betaNdx);
      } // end of for loop on betaNdx

   // Find the time-dependent data.
   int numTDdatasets = (int)m_TDdatasetArray.GetSize();
   bool foundPG = false; 
   bool foundPW = false;
   bool foundSnowpack = false;
   bool foundAll = false;
   int PGndx, PWndx, snowpackNdx;
   int TDdatasetNdx = 0;
   while (TDdatasetNdx<numTDdatasets && !foundAll)
      {
      if (!foundPG && m_TDdatasetArray[TDdatasetNdx]->m_symbol=="PG") { foundPG = true; PGndx = TDdatasetNdx; }
      else if (!foundPW && m_TDdatasetArray[TDdatasetNdx]->m_symbol=="PW") { foundPW = true; PWndx = TDdatasetNdx; }
      else if (!foundSnowpack && m_TDdatasetArray[TDdatasetNdx]->m_symbol=="snowpack") { foundSnowpack = true; snowpackNdx = TDdatasetNdx; }
      foundAll = foundPG && foundPW && foundSnowpack;
      TDdatasetNdx++;
      } // end of while (TDdatasetNdx<numTDdatasets)
   if (!foundAll) 
      {
      msg.Format("APs Crop Choice - missing one or more time_dependent_datasets. foundPG, foundPW, foundSnowpack = %d, %d, %d", 
            foundPG, foundPW, foundSnowpack);
      Report::ErrorMsg(msg);
      return false;
      }
   m_pCropPG = &(*(m_TDdatasetArray[PGndx])); m_pCropPG->InterpretValueStr();
   m_pCropPW = &(*(m_TDdatasetArray[PWndx])); m_pCropPW->InterpretValueStr();

   if (m_CropTestMode>0) InitializeTestMode(pLayer, 16, 8);

   return true;

   } // end of InitCrop()


bool APs::InitRunCrop( EnvContext *pContext )
   {
   // Initialize mean snowpack.
   // Find the climate-scenario-dependent snowpack data.
   int numTDdatasets = (int)m_TDdatasetArray.GetSize();
   bool foundSnowpack = false;
   int snowpackNdx = -1;
   int TDdatasetNdx = 0;
   while (TDdatasetNdx<numTDdatasets && !foundSnowpack)
      {
      if (m_TDdatasetArray[TDdatasetNdx]->m_symbol.CompareNoCase( "snowpack" ) == 0) { foundSnowpack = true; snowpackNdx = TDdatasetNdx; }
      TDdatasetNdx++;
      } // end of while (TDdatasetNdx<numTDdatasets)

   if (!foundSnowpack || snowpackNdx < 0)
   {
      CString msg;
      msg.Format("InitRunCrop() foundSnowpack is false or snowpackNdx < 0.  foundSnowpack = %s, snowpackNdx = %d",
         foundSnowpack ? "true" : "false", snowpackNdx);
      Report::ErrorMsg(msg);
      return(false);
   }

   m_pCropSnowpack = &(*(m_TDdatasetArray[snowpackNdx]));
   Scenario * pScenario = ::EnvGetScenario(pContext->scenarioIndex);
   if (!m_pCropSnowpack->InterpretValueStr(pScenario->m_name)) return(false);

   m_CropMeanSnowpack = m_pCropSnowpack->Get((float)pContext->startYear - 10);
   for (int initial_tau = 1; initial_tau<10; initial_tau++)
      {
      float snowpack = m_pCropSnowpack->Get( (float)pContext->startYear - 10 + initial_tau );
      m_CropMeanSnowpack = m_CropMeanSnowpack*exp(-1/(float)initial_tau) + snowpack*(1 - exp(-1/(float)initial_tau)); 
      } // end of for loop on initial_tau
   
   return(true);
   } // end of InitRunCrop()


bool APs::RunCrop( EnvContext *pContext )
   { 
   testMessage(pContext, _T("RunCrop"));
   /* For each IDU
   Make the independent variables X[i] from: 
         LCC, ELEV_MEAN, SLOPE_DEG, growing season precip, growing season tmin, PG, PW, WREXISTS, snowpack, WR_SH_NDX 
   For each crop, calculate the crop's probability P[i] from X[] and m_cropBeta[].
   Normalize and scale the crop probabilities.
   Adjust for water rights.
   Shuffle and choose a crop with a random draw; store the choice in CROP using UpdateIDU().
   If test mode, store the X[i]s and P[cropNdx]s in X0, ..., and P0, ...
   */
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;
   double X[15];

   // Get the independent variables that don't vary by IDU.
   X[0] = 1.; // Intercept
   X[12] = m_pCropPG->Get((float)pContext->currentYear); // price of grass seed
   X[13] = m_pCropPW->Get((float)pContext->currentYear); // price of wheat
   float snowpack = m_pCropSnowpack->Get((float)pContext->currentYear);
   float SE = snowpack>=m_CropMeanSnowpack ? 1.f : 0.f;
   
   int colWRZone = pLayer->GetFieldCol( "WRZONE");

   ASSERT(CROP_OTHER==(NUM_CROPS-1));   
   //DeltaArray *p_deltaArray = pContext->pDeltaArray;// get a ptr to the delta array
   //INT_PTR deltaSize = p_deltaArray->GetSize();  
   for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
      {
      int lulcA; pLayer->GetData(idu, m_colLulcA, lulcA);
      if (lulcA!=LULCA_AGRICULTURE) continue;

      int old_vegclass; pLayer->GetData(idu, m_colVEGCLASS, old_vegclass);
      if (old_vegclass != VEGCLASS_CROP_NOT_YET_SELECTED) continue;

      int old_lulc_med; pLayer->GetData(idu, m_colLulcMed, old_lulc_med);
      if (old_lulc_med == LULC_MED_ORCHARDS_VINEYARDS_AND_TREE_FARMS && m_currentCropScenarioIndex == 0) continue;

      // Get the rest of the independent variables.
      // X[1]-X[7]
      int LCC;
      pLayer->GetData( idu, m_colLCC, LCC );
      for (int betaNdx=1; betaNdx<=7; betaNdx++) X[betaNdx] = LCC==betaNdx ? 1. : 0.;

      float floatVal;
      pLayer->GetData( idu, m_colELEV, floatVal);
      X[8] = floatVal - 97.2; // demeaned elevation, m

      pLayer->GetData( idu, m_colSLOPE_DEG, floatVal);
      X[9] = floatVal - 3.704; // demeaned slope in degrees from horizontal

      float prcpgroavg; pLayer->GetData(idu, m_colPRCPGROAVG, prcpgroavg);
      float tmingroavg; pLayer->GetData(idu, m_colTMINGROAVG, tmingroavg);
      X[10] = prcpgroavg - m_BasinMeanPrcpGrowingSeason; // - 13.63; // demeaned growing season prcp, inH2O
	   X[11] = tmingroavg - m_BasinMeanTminGrowingSeason; // - 8.556; // demeaned growing season tmin, deg C

      int wrexists=-1;
      pLayer->GetData( idu, m_colWREXISTS, wrexists);
      bool thisIDUisIrrigable = IsIrrigable(wrexists);
      if (!thisIDUisIrrigable) X[14] = 0.;
      else if (SE==1.) X[14] = 1.;
      else
         {
         float wr_sh_ndx=-1;
         pLayer->GetData( idu, m_colWR_SH_NDX, wr_sh_ndx);
         X[14] = 1. - wr_sh_ndx;
         }

      float betaXcrop[NUM_CROPS];
      double probability[NUM_CROPS];
      double totProbXceptOther = 0.;
      for (int cropNdx=0; cropNdx<CROP_OTHER; cropNdx++)
         {
         double betaX_term = 0;

         for (int i=0; i<=14; i++) 
            betaX_term += m_cropBeta[i][cropNdx]*X[i];

			double prob = betaX_term;
         if (prob<0.)
            prob = 0.;
         else if (prob>1.)
            prob = 1.;
         
         totProbXceptOther += prob;
         probability[cropNdx] = prob;
         betaXcrop[cropNdx] = (float)betaX_term;
         } // end of loop on cropNdx
      betaXcrop[CROP_OTHER] = 0.; // There is no betaX term for other crops.
      
      // If the IDU is irrigable, adjust for irrigation choice
      if (thisIDUisIrrigable)
         {
         int irr = -1;
         pLayer->GetData(idu, m_colIRRIGATION, irr);
         if (irr == 1)
            {
            totProbXceptOther -= (probability[CROP_FALLOW] + probability[CROP_WHEAT]);
            probability[CROP_FALLOW] = 0.;
            probability[CROP_WHEAT] = 0.;
            }
         else
            {
            totProbXceptOther -= probability[CROP_CORN];
            probability[CROP_CORN] = 0.;
            }
         } // end of if (thisIDUisIrrigible)

      if (totProbXceptOther <= 1.)
         probability[CROP_OTHER] = 1. - totProbXceptOther;
      else
         {
         double scale_factor = 1. / totProbXceptOther;
         for (int cropNdx = 0; cropNdx<CROP_OTHER; cropNdx++) probability[cropNdx] *= scale_factor;
         probability[CROP_OTHER] = 0.;
         }

      // Choose the crop.
      CROPS chosen_crop = CROP_OTHER; // Initialized to handle the possibility that rand_num comes up exactly 1.
      float long_term_farmrent = 0.0f;
      pLayer->GetData( idu, m_colFRMRNT_LR, long_term_farmrent);
      if (long_term_farmrent < 1.) chosen_crop = CROP_FALLOW;
      else if (m_currentCropScenarioIndex == 0)
         { // Default crop choice
         float rand_num = (float)m_CropRandomDraw.RandValue(0., 1. );  
         float prob_sum = 0;
         for (int cropNdx = 0; cropNdx<NUM_CROPS; cropNdx++)
            {
            prob_sum += (float)probability[cropNdx];
            if (prob_sum>rand_num) 
               {
               chosen_crop = (CROPS)cropNdx;
               break;
               }
            } // end of loop thru crops
         } // end of if (long_term_farmrent>=1.)
      else
         { // "All Fallow" crop choice
         chosen_crop = CROP_FALLOW;
         }

      int new_vegclass;
      switch (chosen_crop)
         {
         case CROP_GRASS_SEED: new_vegclass = 59; break; // Sod/Grass Seed
         case CROP_PASTURE: new_vegclass = 181; break; // Pasture/Hay
         case CROP_WHEAT: new_vegclass = 24; break; // Winter Wheat
         case CROP_FALLOW: new_vegclass = 61; break; // Fallow/Idle Cropland
         case CROP_CORN: new_vegclass = 1; break; // Corn
         case CROP_CLOVER: new_vegclass = 58; break; // Clover/Wildflowers
         case CROP_HAY: new_vegclass = 36; break; // Alfalfa
         case CROP_OTHER: new_vegclass = 14; break; // Mint
         default: ASSERT(0); new_vegclass = 0; break; // missing LULC_FINE data
         }
      if (new_vegclass != old_vegclass) UpdateIDU(pContext, idu, m_colVEGCLASS, new_vegclass, true);

      // Deal with test mode
      if (m_CropTestMode==1)
         { // Stash the independent variable values into attributes X0, X1, ...
         for (int Xndx = 0; Xndx<=14; Xndx++) UpdateIDU( pContext, idu, m_colX0+Xndx, (float)X[Xndx], true );

         UpdateIDU(pContext, idu, m_colX0 + 15, m_BasinMeanPrcpGrowingSeason, true );
         UpdateIDU(pContext, idu, m_colX0 + 16, m_BasinMeanTminGrowingSeason, true );

         UpdateIDU(pContext, idu, m_colBETAX, betaXcrop[chosen_crop], true);
         UpdateIDU( pContext, idu, m_colPROB0, 0.f, true );
         for (int cropNdx = 0; cropNdx<NUM_CROPS; cropNdx++) UpdateIDU( pContext, idu, m_colPROB0+cropNdx+1, (float)probability[cropNdx], true); 
         } // end of if (m_CropTestMode==1)

      } // end of loop thru IDUs
   
   return true;
   } // end of RunCrop()


   bool APs::CheckFCUpriceData()
      // Called by InitRunUrbanWater() for the FullCostUrban scenario
      {
      CString msg;
      bool err;

      if (m_UWrecords_FCUresPrices <= 0 || m_UWrecords_FCUindPrices <= 0)
         {
         CString msg;
         msg.Format("APs Urban Water: Missing data. m_UWrecords_FCUresPrices, m_UWrecords_FCUindPrices = %d, %d",
            m_UWrecords_FCUresPrices, m_UWrecords_FCUindPrices);
         Report::ErrorMsg(msg);
         return false;
         }

      err = m_pUW_FCUresPrices->GetCol("UGB_DH_ID") != 0;
      err |= m_pUW_FCUresPrices->GetCol("UGB_name") != 1;
      err |= m_pUW_FCUresPrices->GetCol("2010") != 2;
      err |= m_pUW_FCUindPrices->GetCol("UGB_DH_ID") != 0;
      err |= m_pUW_FCUindPrices->GetCol("UGB_name") != 1;
      err |= m_pUW_FCUindPrices->GetCol("2010") != 2;
      if (err)
         {
         msg.Format("APs:UrbanWater - One or more missing columns in the FullCostUrb residential or commercial/industrial price files.\n"
            "First three column labels should be 'UGB_DH_ID', 'UGB_name', and '2010'.");
         Report::ErrorMsg(msg);
         return false;
         }

      return true;
      } // end of CheckFCUpriceData()
   
   
   bool APs::InitUrbanWater(EnvContext *pContext)
   {
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;

   m_pUWcoolingCost = new FDataObj;
   m_UWrecords_coolingCost = m_pUWcoolingCost->ReadAscii(m_UWcoolingCost_file, ',', TRUE);
   if (m_UWrecords_coolingCost <= 0)
      {
      CString msg;
      msg.Format("APs Urban Water: Missing data. m_UWrecords_coolingCost = %d", m_UWrecords_coolingCost);
      Report::ErrorMsg(msg);
      return false;
      }
   if (!ConfirmCols(m_pUWcoolingCost))
      {
      CString msg;
      msg.Format("APs Urban Water: Columns missing or out of order. "
         "ConfirmCols(m_pUWcoolingCost) = %d", (int)ConfirmCols(m_pUWcoolingCost));
      Report::ErrorMsg(msg);
      return false;
      }

   m_UWcoolingCost_yr0 = m_pUWcoolingCost->GetAsInt(0, 0);

   for (int ugb=0; ugb<=MAX_UGA_NDX; ugb++)
      {
      m_uwiArray[ugb].bigFlag = 
            (ugb==UGA_Metro || ugb==UGA_SalemKeizer || ugb==UGA_Corvallis || ugb==UGA_EugeneSpringfield);
      m_uwiArray[ugb].ugaName = _T("");
      m_uwiArray[ugb].priceRes_t0 = m_uwiArray[ugb].priceNonRes_t0 = -1.;
      m_uwiArray[ugb].population_divisor = 1.f;
      }

   m_uwiArray[UGA_Metro].ugaName = _T("Portland Metro"); 
   m_uwiArray[UGA_SalemKeizer].ugaName = _T("Salem-Keizer");
   m_uwiArray[UGA_Corvallis].ugaName = _T("Corvallis");
   m_uwiArray[UGA_EugeneSpringfield].ugaName = _T("Eugene-Springfield");

   // Interpret the m_UWpriceArray.
   int ndx = 0; int num = (int)m_UWpriceArray.GetCount();
   while (ndx < num)
      {
      for (int ugb=1; ugb <= MAX_UGA_NDX; ugb++) 
         {
         if (m_UWpriceArray[ndx]->m_area.CompareNoCase( m_uwiArray[ugb].ugaName ) == 0 )
            {
            m_uwiArray[ugb].priceRes_t0 = m_UWpriceArray[ndx]->m_residential_price;
            m_uwiArray[ugb].priceNonRes_t0 = m_UWpriceArray[ndx]->m_non_residential_price;
            m_uwiArray[ugb].ibr = m_UWpriceArray[ndx]->m_IBRflag;
            m_uwiArray[ugb].population_divisor = m_UWpriceArray[ndx]->m_population_divisor;
            break;
            }
         }
      ndx++;
      }

   for (int ugb = 1; ugb<=MAX_UGA_NDX; ugb++)
      if (m_uwiArray[ugb].bigFlag && (m_uwiArray[ugb].priceRes_t0<0. || m_uwiArray[ugb].priceNonRes_t0<0.))
         {
         CString msg;
         msg.Format("APs Urban Water: Price missing or negative for UGA %d. priceRes_t0, priceNonRes_t0 = %f, %f",
               ugb, m_uwiArray[ugb].priceRes_t0, m_uwiArray[ugb].priceNonRes_t0);
         Report::ErrorMsg(msg);
         return false;
         }

   m_UW_pumpingCost = 0.3; // pumping cost term in rural residential demand = $0.30/ccf
   // m_UWruralDensityTerm = pow(768., 0.048); // rural density term = 768 people per sq. mile ^ 0.048
   m_UWruralDensity = 768.; // rural residential population density, people per sq. mile 

   m_pUW_FCUresPrices = new FDataObj;
   m_UWrecords_FCUresPrices = m_pUW_FCUresPrices->ReadAscii(m_UW_FCUresPrice_file, ',', TRUE);
   m_pUW_FCUindPrices = new FDataObj;
   m_UWrecords_FCUindPrices = m_pUW_FCUindPrices->ReadAscii(m_UW_FCUindPrice_file, ',', TRUE);

   return TRUE;
   } // end of InitUrbanWater()


bool APs::InitRunUrbanWater(EnvContext *pContext)
   {
   MapLayer *pLayer = (MapLayer*)pContext->pMapLayer;

      {
      CString msg; msg.Format("*** InitRunUrbanWater(): m_currentPopScenarioIndex = %d", m_currentPopScenarioIndex);
      Report::LogMsg(msg);
      }

   m_ECONincomeOutVar.ClearRows();

   // Do this next bit here instead of Init() so that a user can add the optional columns after Envision
   // starts up, and then use those columns in a simulation run without having to restart Envision.
   m_colUW_AC = pContext->pMapLayer->GetFieldCol("UW_AC");
   m_colUW_LRAC = pContext->pMapLayer->GetFieldCol("UW_LRAC");
   m_colH2OPRICRES = pContext->pMapLayer->GetFieldCol("H2OPRICRES");
   m_colH2OPRICIND = pContext->pMapLayer->GetFieldCol("H2OPRICIND");

   if (m_currentUWScenarioIndex == -1) m_currentUWScenarioIndex = 0;
   if (m_currentUWScenarioIndex == 1 /* FullCostUrb */ && !CheckFCUpriceData()) return(false);

   CString pathAndFileName;

   m_pUWmfgInc = new FDataObj;
   pathAndFileName = ReplaceSubstring(m_UWmfgInc_file, "POPULATION_SCENARIO_NAME", m_popScenarioName);
      {
      CString msg; msg.Format("*** InitRunUrbanWater(): for m_pUWmfgInc, pathAndFileName = %s", pathAndFileName);
      Report::LogMsg(msg);
      }
   m_UWrecords_mfgInc = m_pUWmfgInc->ReadAscii(pathAndFileName, ',', TRUE);

   m_pUWcommInc = new FDataObj;
   pathAndFileName = ReplaceSubstring(m_UWcommInc_file, "POPULATION_SCENARIO_NAME", m_popScenarioName);
      {
      CString msg; msg.Format("*** InitRunUrbanWater(): for m_pUWcommInc, pathAndFileName = %s", pathAndFileName);
      Report::LogMsg(msg);
      }
   m_UWrecords_commInc = m_pUWcommInc->ReadAscii(pathAndFileName, ',', TRUE);

   if (m_UWrecords_mfgInc <= 0 || m_UWrecords_commInc <= 0)
      {
      CString msg;
      msg.Format("APs Urban Water: Missing data. m_UWrecords_mfgInc, "
         "m_UWrecords_commInc = %d, %d",
         m_UWrecords_mfgInc, m_UWrecords_commInc);
      Report::ErrorMsg(msg);
      return false;
      }
   if (!(ConfirmCols(m_pUWmfgInc) && ConfirmCols(m_pUWcommInc)))
      {
      CString msg;
      msg.Format("APs Urban Water: Columns missing or out of order. "
         "ConfirmCols(m_pUWmfgInc), ConfirmCols(m_pUWcommInc) = %d, %d",
         (int)ConfirmCols(m_pUWmfgInc), (int)ConfirmCols(m_pUWcommInc));
      Report::ErrorMsg(msg);
      return false;
      }

   m_UWcommInc_yr0 = m_pUWcommInc->GetAsInt(0, 0);
   m_UWmfgInc_yr0 = m_pUWmfgInc->GetAsInt(0, 0);

   return(true);
   } // end of InitRunUrbanWater()

int APs::CountyIDtoUse(int countyIDin)
   // COUNTYID 0 is unknown - use data for Lane (there are 2 IDUs with COUNTYID on the eastern border of Lane county)
   // COUNTYID -3 is Columbia - use data for Washington
   // COUNTYID -4 is Deschutes - use data for Lane
   // COUNTYID -13 is Wasco - use data for Clackamas
   // COUNTYID 3 is Douglas - use data for Lane
   // COUNTYID 5 is Lincoln - use data for Benton
   // COUNTYID 10 is Tillamook - use data for Yamhill
   {
   int countyIDout;
   switch (countyIDin)
      {
      case 0: countyIDout = LaneCoID;
      case ColumbiaCoID: countyIDout = WashingtonCoID; break;
      case DeschutesCoID: countyIDout = LaneCoID; break;
      case WascoCoID: countyIDout = ClackamasCoID; break;
      case DouglasCoID: countyIDout = LaneCoID; break;
      case LincolnCoID: countyIDout = BentonCoID; break;
      case TillamookCoID: countyIDout = YamhillCoID; break;
      default: 
         countyIDout = (1<=countyIDin && countyIDin<=12) ? countyIDin : LaneCoID; 
         break;
      } // end of switch(countyIDin)

   return(countyIDout);
   } // end of CountyIDtoUse()


COUNTYcolumn APs::CountyColFromCountyID(int countyID)
   {
   const COUNTYcolumn countyColFromCountyID[13] =
      // 0        1             2        3        4      5        6          7             8        9      10            11          12
      { NoCol, BentonCol, ClackamasCol, NoCol, LaneCol, NoCol, LinnCol, MarionCol, MultnomahCol, PolkCol, NoCol, WashingtonCol, YamhillCol };

   int ID_to_use = CountyIDtoUse(countyID);
   COUNTYcolumn countyCol = countyColFromCountyID[ID_to_use];

   return(countyCol);
   } // end of CountyColFromCountyID()


bool APs::RunUrbanWater(EnvContext *pContext)
   {
   testMessage(pContext, _T("RunUrbanWater"));

   // update the UWI variables
   MapLayer *pLayer = (MapLayer*)pContext->pMapLayer;
   int uga_idu_count[MAX_UGA_NDX + 1];
   double xurbh2oprc_accum[MAX_UGA_NDX + 1];

   // update temporally variable UWI info
   for (int ugb = 1; ugb <= MAX_UGA_NDX; ugb++)
      {
      m_uwiArray[ugb].population = 0;
      m_uwiArray[ugb].incomeRes = Get4UGB(m_pHholdInc, ugb, pContext->currentYear - m_HholdInc_yr0);
      m_uwiArray[ugb].incomeComm_Mdollars = Get4UGB(m_pUWcommInc, ugb, pContext->currentYear - m_UWcommInc_yr0);
      m_uwiArray[ugb].incomeInd_Mdollars = Get4UGB(m_pUWmfgInc, ugb, pContext->currentYear - m_UWmfgInc_yr0);
      m_uwiArray[ugb].coolingCost = Get4UGB(m_pUWcoolingCost, ugb, pContext->currentYear - m_UWcoolingCost_yr0);
      uga_idu_count[ugb] = 0;
      xurbh2oprc_accum[ugb] = 0.;
      }; // end of for( int ugb=1; ugb <= MAX_UGA_NDX; ugb++ )

   // iterate through IDUs, accumulating stats
   ASSERT(m_colUGB >= 0);
   ASSERT(m_colPOP >= 0);
   for (MapLayer::Iterator idu = pLayer->Begin(); idu < pLayer->End(); idu++)
      {
      int ugb = -1;
      pLayer->GetData(idu, m_colUGB, ugb);
      if (ugb <= 0 || ugb > MAX_UGA_NDX) continue;

      float iduPop = 0;
      pLayer->GetData(idu, m_colPOP, iduPop);
      m_uwiArray[ugb].population += iduPop;

      float idu_area_m2 = 0;
      pLayer->GetData(idu, m_colAREA, idu_area_m2);
      m_uwiArray[ugb].ugb_area_m2 += idu_area_m2;

      float idu_xurbh2oprc; pLayer->GetData(idu, m_colXURBH2OPRC, idu_xurbh2oprc);
      xurbh2oprc_accum[ugb] += idu_xurbh2oprc;
      uga_idu_count[ugb]++;
      } // end of loop thru all the IDUs
   for (int ugb = 1; ugb <= MAX_UGA_NDX; ugb++) m_uwiArray[ugb].urb_H2O_prc_multiplier = uga_idu_count[ugb]>0 ? (float)xurbh2oprc_accum[ugb]/uga_idu_count[ugb] : 1.f;

   // UWI values updated, calculate model inputs
   //
   // The model to predict changes in urban water demand (ccf/day, all scenarios except Extreme) or changes in price ($/ccf, Extreme scenario) is:
   //
   // Residential (big city equation):
   //   orig:     ln Q = -(2.931236 + 0.609114 IBR) - 0.6 ln p + ln Pop + (0.13 + 0.05 IBR) ln I - 0.048 ln D
   //   4/18/15:  ln Q = -(3.0159618 + 0.6506062 IBR) - 0.6 ln p + ln Pop + (0.13 + 0.05 IBR) ln I - 0.048 ln D
   //   or, for Extreme scenario only
   //   Q = (prescribed per capita daily use * Pop)*residential_fraction
   //   orig:     ln p = (1/0.6)*(-(2.931236 + 0.609114 IBR) + ln Pop + (0.13 + 0.05 IBR) ln I - 0.048 ln D - ln Q)
   //   04/18/15: ln p = (1/0.6)*(-(3.0159618 + 0.6506062 IBR) + ln Pop + (0.13 + 0.05 IBR) ln I - 0.048 ln D - ln Q)
   //
   // Non-residential:
   //   ln Q = - 2.727616 - 0.6 ln p + 0.11 ln Ind.I + 0.04 ln Comm.I + 0.85 ln Pop
   //   or, for Extreme scenario only
   //   Q = (prescribed per capita daily use * Pop)*(1 - residential_fraction)
   //   ln p = (1/0.6)*(- 2.727616 + 0.11 ln Ind.I + 0.04 ln Comm.I + 0.85 ln Pop - ln Q)
   //
   // where
   // Q = total daily water use for the entire city in hundreds of cubic feet (ccf).
   // p = price in $/ccf.
   // Pop = city population.
   // D = density (persons per mile2)
   // I = median household income
   // IBR = 1 if city has an Increasing Block Rate pricing structure, = 0 otherwise.  
   // Ind. I = total city industrial (manufacturing) income in thousands of $
   // Comm. I = total city commercial income in thousands of $
   // residential_fraction is calculated in the first year of the simulation
   // prescribed per capita daily use
   //   2010 = as calculated in reference scenario
   //   2011 thru 2049 = 2010 modeled per capita use - (2010 modeled per capita daily use - 100 gal per day)/39
   //   at 2050 and thereafter = 100 gals per day
   for (int ugb = 1; ugb <= MAX_UGA_NDX; ugb++)
      {
      float pop = m_uwiArray[ugb].population;
      if (pop <= 0.)
         {
         m_uwiArray[ugb].ugbCode = -1;
         continue;
         }
      m_uwiArray[ugb].ugbCode = ugb;

      // Cost calculation
      float cool = m_uwiArray[ugb].coolingCost;
      double ac, lrac;
      float pop_divisor = m_uwiArray[ugb].bigFlag ? m_uwiArray[ugb].population_divisor : 1.f;
      double lnTerm = log(pop/pop_divisor); double lnSqrd = lnTerm*lnTerm; double lnCubed = lnTerm*lnSqrd;
      double ac_lnTerm = 9.93 - 0.355*lnTerm + 0.030*lnSqrd - 0.001*lnCubed;
      ac = 0.748*exp(ac_lnTerm) + cool; ac /= 1000.;
      lrac = 0.748*exp(13.39 - 1.246*lnTerm + 0.117*lnSqrd - 0.004*lnCubed); lrac /= 1000.;

      if (pContext->yearOfRun == 0)
         {
         m_uwiArray[ugb].aveCost_t0 = (float)ac;
         if (!m_uwiArray[ugb].bigFlag) m_uwiArray[ugb].priceRes_t0 = (float)ac;
         }

      // Set the prices (most scenarios) or per capita consumption (Extreme scenario) for the current year.
      // Small cities and rural areas use a single price (priceRes).
      // The big cities use two prices, priceRes and priceNonRes.
      double priceRes = 1.f;
      double priceNonRes = 1.f;
      double lnPriceRes, lnPriceNonRes; lnPriceRes = lnPriceNonRes = 0.f;
      float QperCapita_galperday = 0.f; // gals per day per capita
      float QperCapita_ccf = 0.f; // ccf per day per capita

      if (m_currentUWScenarioIndex == 1 /* FullCostUrb */)
         { // Use the price information in the .csv files when possible.
         int tgtCol = 2 + pContext->yearOfRun;
         bool foundFlag = false;
         if (tgtCol >= m_pUW_FCUresPrices->GetColCount()) priceRes = lrac;
         else
            { // Try to read the residential price from the table
            int iRow = 0;
            while (!foundFlag && iRow < m_UWrecords_FCUresPrices)
               {
               int ugbID = m_pUW_FCUresPrices->GetAsInt(0, iRow);
               foundFlag = UGBlookup(ugbID) == ugb;
               if (!foundFlag) iRow++;
               }
            if (foundFlag) priceRes = (m_pUW_FCUresPrices->GetAsDouble(tgtCol, iRow))*m_uwiArray[ugb].urb_H2O_prc_multiplier;
            else
               {
               CString msg;
               msg.Format("APs Urban Water: Couldn't find uga %d in the FullCostUrb residential price table. "
                  "Setting residential water price to long range average cost and continuing.", ugb);
               if (pContext->yearOfRun == 0) Report::ErrorMsg(msg);
               priceRes = lrac*m_uwiArray[ugb].urb_H2O_prc_multiplier;
               }
            } // end of block to try to read the residential price from the table

         if (!m_uwiArray[ugb].bigFlag) priceNonRes = priceRes; 
         else if (tgtCol >= m_pUW_FCUindPrices->GetColCount()) priceNonRes = lrac;
         else
            { // Try to read the commercial/industrial price from the table
            foundFlag = false;
            int iRow = 0;
            while (!foundFlag && iRow < m_UWrecords_FCUindPrices)
               {
               int ugbID = m_pUW_FCUindPrices->GetAsInt(0, iRow);
               foundFlag = UGBlookup(ugbID) == ugb;
               if (!foundFlag) iRow++;
               }
            if (foundFlag) priceNonRes = m_pUW_FCUindPrices->GetAsDouble(tgtCol, iRow)*m_uwiArray[ugb].urb_H2O_prc_multiplier;
            else
               {
               CString msg;
               msg.Format("APs Urban Water: Couldn't find uga %d in the FullCostUrb industrial and commercial price table. "
                  "Setting non-residential water price to long range average cost and continuing.", ugb);
               if (pContext->yearOfRun == 0) Report::ErrorMsg(msg);
               priceNonRes = lrac*m_uwiArray[ugb].urb_H2O_prc_multiplier;
               }
            } // end of block to try to read the commercial/industrial price from the table
         } // end of block to use prices from tables in .csv files
      else if (m_currentUWScenarioIndex == 0 /* Ref */ || m_currentUWScenarioIndex == 2 /* Extreme */ 
               || (m_currentUWScenarioIndex == 3 /* Managed */ && pContext->yearOfRun == 0) || m_currentUWScenarioIndex == 4 /* Short Run Demand */)
         { // Set prices as for Reference scenario
         float priceMult;
         if (pContext->yearOfRun == 0)
            {
            priceMult = (float)(ac / m_uwiArray[ugb].aveCost_t0)*m_uwiArray[ugb].urb_H2O_prc_multiplier;
            priceRes = priceMult * m_uwiArray[ugb].priceRes_t0;
            // follwing "priceRes_lastyr" will be updated at the end of RunUrbanWater
            }
         else if (pContext->yearOfRun > 0 && pContext->yearOfRun <= 5) priceRes = m_uwiArray[ugb].priceRes_lastyr * 1.06;
         else if (pContext->yearOfRun > 5 && pContext->yearOfRun <= 15) priceRes = m_uwiArray[ugb].priceRes_lastyr * 1.015;
         else
            {
            priceMult = (float)(ac / m_uwiArray[ugb].ac_lastyr);
            priceRes = priceMult * m_uwiArray[ugb].priceRes_lastyr;
            }
         priceNonRes = priceRes;
         if (m_uwiArray[ugb].bigFlag)
            { // Calculate the price for commercial and industrial use.
            float priceNonResMult;
            if (pContext->yearOfRun == 0)
               {
               priceNonResMult = (float)(ac / m_uwiArray[ugb].aveCost_t0)*m_uwiArray[ugb].urb_H2O_prc_multiplier;
               priceNonRes = m_uwiArray[ugb].priceNonRes_t0 * priceNonResMult;
               }
            else if (pContext->yearOfRun > 0 && pContext->yearOfRun <= 5) priceNonRes = m_uwiArray[ugb].priceNonRes_lastyr * 1.06;
            else if (pContext->yearOfRun > 5 && pContext->yearOfRun <= 15) priceNonRes = m_uwiArray[ugb].priceNonRes_lastyr * 1.015;
            else
               {
               priceNonResMult = (float)(ac / m_uwiArray[ugb].ac_lastyr);
               priceNonRes = priceNonResMult * m_uwiArray[ugb].priceNonRes_lastyr;
               }
            } // end of block to calculate nonRes price for big cities
         } // end of if ... else if (m_currentUWScenarioIndex == 0 /* Ref */ || m_currentUWScenarioIndex == 2 /* Extreme */ 
               // || (m_currentUWScenarioIndex == 3 /* Managed */ && pContext->yearOfRun == 0) || m_currentUWScenarioIndex == 4 /* Short Run Demand Model */) ...
      else
         { // m_currentUWScenarioIndex = 3 (Managed) and yearORun>0
         QperCapita_galperday = (pContext->currentYear < 2050) ? 
               (m_uwiArray[ugb].origQperCapita_galperday + ((100.f - m_uwiArray[ugb].origQperCapita_galperday) / (2049 - pContext->startYear))*(pContext->yearOfRun))
               : 100.f;
         QperCapita_ccf = (float)(QperCapita_galperday / (GAL_PER_FT3*100.f));
         } // end of if (m_currentUWScenarioIndex == 1 /* FullCostUrb */) else if ... else ...
      lnPriceRes = log(priceRes);
      lnPriceNonRes = log(priceNonRes);

      float incomeRes = m_uwiArray[ugb].incomeRes;
      if (incomeRes <= 0.f)
         {
         CString msg;
         msg.Format("APs Urban Water: incomeRes is 0 or negative for UGA %d. yearOfRun, incomeRes = %d, %f\n"
            "Setting incomeRes to $90,000 and continuing",
            ugb, pContext->yearOfRun, incomeRes);
         Report::ErrorMsg(msg);
         incomeRes = 90000.f;
         }

      double density = m_UGBadjPopDensities[ugb]; // persons per m2 // not pop/m_uwiArray[ugb].areaTotal;
      density *= M2_PER_MI2; // Convert from persons per m2 to persons per square mile. 
      if (density <= 0. && pContext->currentYear == pContext->startYear)
         {
         CString msg;
         msg.Format("APs Urban Water: Adjusted population density is <=0 for UGA %d %s. adjusted density (persons/mi2), population = %f, %f.  "
            "This could be because there are no developed IDUs in the UGA. For urban water calculations, unadjusted density (i.e. UGA population / UGA area) will be used.",
            ugb, m_uwiArray[ugb].ugaName, density, m_uwiArray[ugb].population);
         Report::ErrorMsg(msg);
         }

      double lnD; // Will hold ln(density), provided density>0 

      if (m_uwiArray[ugb].bigFlag)
         { // Use the big city algorithms.
         ASSERT(density > 0.);
         lnD = density>0 ? log(density) : 0.;
         float ibr = (float)m_uwiArray[ugb].ibr;
         float incomeComm_Kdollars = m_uwiArray[ugb].incomeComm_Mdollars*1000.f;
         float incomeInd_Kdollars = m_uwiArray[ugb].incomeInd_Mdollars*1000.f;

         double lnPop = log(pop);
         double lnQRes, lnQNonRes;
         if (m_currentUWScenarioIndex != 3 /* != Managed */ || pContext->yearOfRun==0)
            {
            if (m_currentUWScenarioIndex == 4 /* Short Run Demand Model */)
               {
               lnQRes = -(3.3641831 + 0.50601 * ibr) - 0.33f * lnPriceRes + lnPop + (0.13 + 0.05 * ibr) * log(incomeRes) - 0.048 * lnD;
               lnQNonRes = -3.081824 - 0.33 * lnPriceNonRes + 0.11 * log(incomeInd_Kdollars) + 0.04 * log(incomeComm_Kdollars) + 0.85 * lnPop;
               }
            else
               { 
               lnQRes = -(3.0159618 + 0.6506062 * ibr) - 0.6f * lnPriceRes + lnPop + (0.13 + 0.05 * ibr) * log(incomeRes) - 0.048 * lnD; 
               lnQNonRes = -2.727616 - 0.6 * lnPriceNonRes + 0.11 * log(incomeInd_Kdollars) + 0.04 * log(incomeComm_Kdollars) + 0.85 * lnPop;
               }
            CString msg;
            msg.Format("*** UW for Metro: ibr = %d, lnPriceRes = %f, lnPop = %f, incomeRes = %f, lnD = %f, lnQRes = %f", ibr, lnPriceRes, lnPop, incomeRes, lnD, lnQRes);
            Report::LogMsg(msg);
            msg.Format("*** UW for Metro: lnPriceNonRes = %f, incomeInd_Kdollars = %f, incomeComm_Kdollars = %f, lnQNonRes = %f", lnPriceNonRes, incomeInd_Kdollars, incomeComm_Kdollars, lnQNonRes);
            Report::LogMsg(msg);
            msg.Format("UW for ugb = %d: m_uwiArray[ugb].incomeComm_Mdollars = %f, m_uwiArray[ugb].incomeInd_Mdollars = %f", 
               ugb, m_uwiArray[ugb].incomeComm_Mdollars, m_uwiArray[ugb].incomeInd_Mdollars);
            Report::LogMsg(msg);
            }
         else
            { // Managed scenario: Q per capita is prescribed, solve for price 
            float residential_fraction = m_uwiArray[ugb].origResFrac;
            float qRes = (QperCapita_ccf * pop)*residential_fraction;
            lnQRes = log(qRes);
            lnPriceRes = (1 / 0.6f)*(-(3.0159618 + 0.6506062 * ibr) + lnPop + (0.13 + 0.05 * ibr) * log(incomeRes) - 0.048 * lnD - lnQRes); 
            priceRes = (float)exp(lnPriceRes);
            float qNonRes = (QperCapita_ccf * pop)*(1 - residential_fraction);
            lnQNonRes = log(qNonRes);
            lnPriceNonRes = (1 / 0.6f)*(-2.727616 + 0.11 * log(incomeInd_Kdollars) + 0.04 * log(incomeComm_Kdollars) + 0.85 * lnPop - lnQNonRes);
            priceNonRes = (float)exp(lnPriceNonRes);
            }

         m_uwiArray[ugb].qRes = (float)exp(lnQRes);    // ccf/day
         m_uwiArray[ugb].qNonRes = (float)exp(lnQNonRes);
         } // end of block for big cities
      else
         { // Use the small city algorithms.
         double price = priceRes;
         if (m_currentUWScenarioIndex != 3 /* != Managed */ && (price <= 0.f || ac <= 0.f))
            {
            CString msg;
            msg.Format("APs Urban Water: Price or ac is 0 or negative for small city UGA %d. yearOfRun, price, ac, pricesmallcity_lastyr, ac_lastyr = "
               "%d, %f, %f, %f, %f\n",
               ugb, pContext->yearOfRun, price, ac, m_uwiArray[ugb].pricesmallcity_lastyr, m_uwiArray[ugb].ac_lastyr);
            Report::ErrorMsg(msg);
            }

         double lnPop = log(pop);
         if (m_currentUWScenarioIndex != 3 /* != Managed scenario */ || pContext->yearOfRun == 0)
            { 
            if (density <= 0.)
               {
               density = (m_uwiArray[ugb].population / m_uwiArray[ugb].ugb_area_m2) * M2_PER_MI2;
               if (density < m_UWruralDensity) density = m_UWruralDensity;
               CString msg;
               msg.Format("RunUrbanWater() ugb = %d, unadjusted density = %f persons/mi2", ugb, density);
               Report::LogMsg(msg);
               }
            ASSERT(density > 0.);
            lnD = log(density);
            double lnQtot = 1.;
            if (m_currentUWScenarioIndex == 4 /* Short Run Demand Model */)
                  lnQtot = -2.5385 - (0.33*log(price)) + lnPop + (0.13*log(incomeRes)) - (0.048*lnD);
            else lnQtot = -2.16432007 - (0.6*log(price)) + lnPop + (0.13*log(incomeRes)) - (0.048*lnD);
            m_uwiArray[ugb].qRes = (float)exp(lnQtot);
            }
         else
            { // Managed scenario.  Calculate price from prescribed consumption.
            if (density > 0.)
               {
               lnD = log(density);
               float Qtot = QperCapita_ccf * pop;
               m_uwiArray[ugb].qRes = Qtot;
               double lnPrice = (1 / 0.6f)*(-2.16432007 + lnPop + 0.13*log(incomeRes) - 0.048*lnD - log(Qtot));
               price = (float)exp(lnPrice);
               }
            else
               {  // Degenerate case.  No developed IDUs in the UGA, so adjusted population density is 0.
               m_uwiArray[ugb].qRes = 0.f;
               price = 0.f;
               }
            }
         priceRes = price;
         priceNonRes = 0.f;
         m_uwiArray[ugb].pricesmallcity_lastyr = price;
         m_uwiArray[ugb].qNonRes = 0.f;
         } // end of block for small cities

      m_uwiArray[ugb].aveCost = (float)ac;
      m_uwiArray[ugb].longRunAveCost = (float)lrac;

      // update the struct UWI value: priceRes_lastyr; ac_lastyr at the end of UGA loops
      // here the values of priceRes_lastyr and ac_lastyr are actually for next simulate year
      m_uwiArray[ugb].priceRes_lastyr = priceRes;
      m_uwiArray[ugb].priceNonRes_lastyr = priceNonRes;
      m_uwiArray[ugb].ac_lastyr = ac;
      if (pContext->yearOfRun == 0)
         {
         m_uwiArray[ugb].origQperCapita_galperday = (float)(((m_uwiArray[ugb].qRes + m_uwiArray[ugb].qNonRes) / pop)*(100.f*GAL_PER_FT3));
         m_uwiArray[ugb].origResFrac = m_uwiArray[ugb].qRes / (m_uwiArray[ugb].qRes + m_uwiArray[ugb].qNonRes);
         }
      } // end of loop on UGAs

   // Calculate rural residential demand by county.
   float qHholdRural[13]; // demand, ccf/household/day, indexed by CountyID attribute.
   int row = pContext->currentYear - m_HholdInc_yr0;
   for (int countyID = 1; countyID <= 12; countyID++)
      {
      COUNTYcolumn countyCol = CountyColFromCountyID(countyID);
      if (countyCol != NoCol)
         {
         double householdIncome = m_pHholdInc->GetAsFloat(countyCol, row);
         double ln_QperCapitaRural = -3.55f - 0.6*log(m_UW_pumpingCost) + 0.13f*log(householdIncome) - 0.048f*log(m_UWruralDensity); // ln(ccf/person/day)
         qHholdRural[countyID] = (float)exp(ln_QperCapitaRural)*PEOPLE_PER_HOUSEHOLD;
         }
      else qHholdRural[countyID] = 0.;
      }

   // iterate through IDUs, storing outputs
   ASSERT(m_colH2ORESIDNT >= 0 && m_colH2OINDCOMM >= 0);
   ASSERT(m_colCOUNTYID >= 0);
   float noDataValue = pLayer->GetNoDataValue();
   for (MapLayer::Iterator idu = pLayer->Begin(); idu < pLayer->End(); idu++)
      {
      float iduQres, iduQnonRes;

      int ugb = -1;
      pLayer->GetData(idu, m_colUGB, ugb);

      if (ugb > 0 && ugb <= MAX_UGA_NDX)
         { // this IDU is urban; store the values for its UGA.
         ASSERT(m_uwiArray[ugb].ugbCode == ugb);

         // Put all the demand in the IDUs in proportion to their population.
         float iduPop = 0.f; pLayer->GetData(idu, m_colPOP, iduPop);
         float iduPopFrac = m_uwiArray[ugb].population > 0.f ? iduPop / m_uwiArray[ugb].population : 0.f;
         iduQres = iduPopFrac * m_uwiArray[ugb].qRes; // ccf/day
         iduQnonRes = iduPopFrac * m_uwiArray[ugb].qNonRes; // ccf/day

         UpdateIDU(pContext, idu, m_colH2OINDCOMM, iduQnonRes, true);

         if (m_colUW_AC >= 0)
            UpdateIDU(pContext, idu, m_colUW_AC, m_uwiArray[ugb].aveCost, true);
         if (m_colUW_LRAC >= 0)
            UpdateIDU(pContext, idu, m_colUW_LRAC, m_uwiArray[ugb].longRunAveCost, true);
         if (m_colH2OPRICRES >= 0)
            UpdateIDU(pContext, idu, m_colH2OPRICRES, m_uwiArray[ugb].priceRes_lastyr, true);
         if (m_colH2OPRICIND >= 0)
            UpdateIDU(pContext, idu, m_colH2OPRICIND, m_uwiArray[ugb].priceNonRes_lastyr, true);

         } // end of urban IDU block
      else
         { // This IDU is rural; calculate and store the values for its county and population.
         float iduPop = 0;
         pLayer->GetData(idu, m_colPOP, iduPop);

         if (iduPop <= 0.) iduQres = 0;
         else
            {
            float iduHholds = iduPop / PEOPLE_PER_HOUSEHOLD;
            int countyID = 0;
            pLayer->GetData(idu, m_colCOUNTYID, countyID);
            countyID = CountyIDtoUse(countyID);
            if (countyID>=1 && countyID<=12) iduQres = qHholdRural[countyID] * iduHholds;
            else iduQres = 0;
            }
         if (pContext->yearOfRun == 0)
            {
            UpdateIDU(pContext, idu, m_colH2OINDCOMM, 0, true);
            // Originally these next two lines set the UW_AC and UW_LRAC attributes of rural IDUs to noDataValue,
            // but that caused a problem because noDataValue is -99 for the IDU layer, and the UGA Expansion 
            // process adds IDUs which were previously rural to the UGA, after the Urban Water process runs.
            // Changing this from noDataValue to 0 is a bandaid which will make the problem less obvious,
            // while a better solution is developed.
            if (m_colUW_AC >= 0) UpdateIDU(pContext, idu, m_colUW_AC, 0, true);
            if (m_colUW_LRAC >= 0) UpdateIDU(pContext, idu, m_colUW_LRAC, 0, true);
            if (m_colH2OPRICRES >= 0) UpdateIDU(pContext, idu, m_colH2OPRICRES, 0, true);
            if (m_colH2OPRICIND >= 0) UpdateIDU(pContext, idu, m_colH2OPRICIND, 0, true);
            }
         } // end of rural IDU block

      UpdateIDU(pContext, idu, m_colH2ORESIDNT, iduQres, true);

      } // end of loop thru all the IDUs

   m_metroDemandResidential = m_uwiArray[UGA_Metro].qRes;
   m_eugSprDemandResidential = m_uwiArray[UGA_EugeneSpringfield].qRes;
   m_salemKeiserDemandResidential = m_uwiArray[UGA_SalemKeizer].qRes;
   m_corvallisDemandResidential = m_uwiArray[UGA_Corvallis].qRes;
   m_albanyDemandResidential = m_uwiArray[UGA_Albany].qRes;
   m_mcMinnDemandResidential = m_uwiArray[UGA_McMinnville].qRes;
   m_newbergDemandResidential = m_uwiArray[UGA_Newberg].qRes;
   m_woodburnDemandResidential = m_uwiArray[UGA_Woodburn].qRes;

   m_metroDemandNonResidential = m_uwiArray[UGA_Metro].qNonRes;
   m_eugSprDemandNonResidential = m_uwiArray[UGA_EugeneSpringfield].qNonRes;
   m_salemKeiserDemandNonResidential = m_uwiArray[UGA_SalemKeizer].qNonRes;
   m_corvallisDemandNonResidential = m_uwiArray[UGA_Corvallis].qNonRes;

   int nRows = m_annualUrbanWaterDemand.GetRowCount();
   int rowNdx = pContext->yearOfRun<0 ? 0 : pContext->yearOfRun;

   while (nRows <= rowNdx)
      {
      nRows = m_annualUrbanWaterDemand.AppendRows(1);
      }

   m_annualUrbanWaterDemand.Set(0, rowNdx, (float)pContext->currentYear);
   m_annualUrbanWaterDemand.Set(1, rowNdx, m_metroDemandResidential);
   m_annualUrbanWaterDemand.Set(2, rowNdx, m_eugSprDemandResidential);
   m_annualUrbanWaterDemand.Set(3, rowNdx, m_salemKeiserDemandResidential);
   m_annualUrbanWaterDemand.Set(4, rowNdx, m_corvallisDemandResidential);
   m_annualUrbanWaterDemand.Set(5, rowNdx, m_albanyDemandResidential);
   m_annualUrbanWaterDemand.Set(6, rowNdx, m_mcMinnDemandResidential);
   m_annualUrbanWaterDemand.Set(7, rowNdx, m_newbergDemandResidential);
   m_annualUrbanWaterDemand.Set(8, rowNdx, m_woodburnDemandResidential);

   m_annualUrbanWaterDemand.Set(9, rowNdx, m_metroDemandNonResidential);
   m_annualUrbanWaterDemand.Set(10, rowNdx, m_eugSprDemandNonResidential);
   m_annualUrbanWaterDemand.Set(11, rowNdx, m_salemKeiserDemandNonResidential);
   m_annualUrbanWaterDemand.Set(12, rowNdx, m_corvallisDemandNonResidential);

   m_annualUrbanWaterDemand.Set(13, rowNdx, qHholdRural[BentonCoID]);
   m_annualUrbanWaterDemand.Set(14, rowNdx, qHholdRural[ClackamasCoID]);
   m_annualUrbanWaterDemand.Set(15, rowNdx, qHholdRural[LaneCoID]);
   m_annualUrbanWaterDemand.Set(16, rowNdx, qHholdRural[LinnCoID]);
   m_annualUrbanWaterDemand.Set(17, rowNdx, qHholdRural[MarionCoID]);
   m_annualUrbanWaterDemand.Set(18, rowNdx, qHholdRural[MultnomahCoID]);
   m_annualUrbanWaterDemand.Set(19, rowNdx, qHholdRural[PolkCoID]);
   m_annualUrbanWaterDemand.Set(20, rowNdx, qHholdRural[WashingtonCoID]);
   m_annualUrbanWaterDemand.Set(21, rowNdx, qHholdRural[YamhillCoID]);

   CArray< float, float > rowOutVar;
   rowOutVar.SetSize(34);
   rowOutVar[0] = (float)pContext->currentYear;
   rowOutVar[1] = m_pHholdInc->GetAsFloat(BentonCol, row);
   rowOutVar[2] = m_pHholdInc->GetAsFloat(ClackamasCol, row);
   rowOutVar[3] = m_pHholdInc->GetAsFloat(ColumbiaCol, row);
   rowOutVar[4] = m_pHholdInc->GetAsFloat(LaneCol, row);
   rowOutVar[5] = m_pHholdInc->GetAsFloat(LinnCol, row);
   rowOutVar[6] = m_pHholdInc->GetAsFloat(MarionCol, row);
   rowOutVar[7] = m_pHholdInc->GetAsFloat(MultnomahCol, row);
   rowOutVar[8] = m_pHholdInc->GetAsFloat(PolkCol, row);
   rowOutVar[9] = m_pHholdInc->GetAsFloat(WashingtonCol, row);
   rowOutVar[10] = m_pHholdInc->GetAsFloat(YamhillCol, row);
   rowOutVar[11] = m_pHholdInc->GetAsFloat(MetroCol, row);
   rowOutVar[12] = m_pUWmfgInc->GetAsFloat(BentonCol, row);
   rowOutVar[13] = m_pUWmfgInc->GetAsFloat(ClackamasCol, row);
   rowOutVar[14] = m_pUWmfgInc->GetAsFloat(ColumbiaCol, row);
   rowOutVar[15] = m_pUWmfgInc->GetAsFloat(LaneCol, row);
   rowOutVar[16] = m_pUWmfgInc->GetAsFloat(LinnCol, row);
   rowOutVar[17] = m_pUWmfgInc->GetAsFloat(MarionCol, row);
   rowOutVar[18] = m_pUWmfgInc->GetAsFloat(MultnomahCol, row);
   rowOutVar[19] = m_pUWmfgInc->GetAsFloat(PolkCol, row);
   rowOutVar[20] = m_pUWmfgInc->GetAsFloat(WashingtonCol, row);
   rowOutVar[21] = m_pUWmfgInc->GetAsFloat(YamhillCol, row);
   rowOutVar[22] = m_pUWmfgInc->GetAsFloat(MetroCol, row);
   rowOutVar[23] = m_pUWcommInc->GetAsFloat(BentonCol, row);
   rowOutVar[24] = m_pUWcommInc->GetAsFloat(ClackamasCol, row);
   rowOutVar[25] = m_pUWcommInc->GetAsFloat(ColumbiaCol, row);
   rowOutVar[26] = m_pUWcommInc->GetAsFloat(LaneCol, row);
   rowOutVar[27] = m_pUWcommInc->GetAsFloat(LinnCol, row);
   rowOutVar[28] = m_pUWcommInc->GetAsFloat(MarionCol, row);
   rowOutVar[29] = m_pUWcommInc->GetAsFloat(MultnomahCol, row);
   rowOutVar[30] = m_pUWcommInc->GetAsFloat(PolkCol, row);
   rowOutVar[31] = m_pUWcommInc->GetAsFloat(WashingtonCol, row);
   rowOutVar[32] = m_pUWcommInc->GetAsFloat(YamhillCol, row);
   rowOutVar[33] = m_pUWcommInc->GetAsFloat(MetroCol, row);
   m_ECONincomeOutVar.AppendRow(rowOutVar);

   return true;
   } // end of RunUrbanWater()


float APs::Get4UGB(FDataObj *pDataObj, int ugb, int row)
   {
   float rtnval;

   switch (ugb)
      {
      case UGA_AdairVillage /* 1 */ : rtnval = pDataObj->GetAsFloat(BentonCol, row); break;
      case UGA_Albany /* 2 */ :  rtnval = pDataObj->GetAsFloat(BentonCol, row)*0.158f
            +  pDataObj->GetAsFloat(LinnCol, row)*0.842f; break;
      case UGA_Amity /* 3 */ :  rtnval = pDataObj->GetAsFloat(YamhillCol, row); break;
      case UGA_Aumsville /* 4 */ :  rtnval = pDataObj->GetAsFloat(MarionCol, row); break;
      case UGA_Aurora /* 5 */ :  rtnval = pDataObj->GetAsFloat(ClackamasCol, row)*0.001f 
            +  pDataObj->GetAsFloat(MarionCol, row)*0.999f; break;
      case UGA_Banks /* 6 */ :  rtnval = pDataObj->GetAsFloat(WashingtonCol, row); break;
      case UGA_Barlow /* 7 */ :  rtnval = pDataObj->GetAsFloat(ClackamasCol, row); break;
      case UGA_Brownsville /* 8 */ :  rtnval = pDataObj->GetAsFloat(LinnCol, row); break;
      case UGA_Canby /* 9 */ :  rtnval = pDataObj->GetAsFloat(ClackamasCol, row); break;
      case UGA_Carlton /* 10 */ :  rtnval = pDataObj->GetAsFloat(YamhillCol, row); break;
      case UGA_Coburg /* 11 */ :  rtnval = pDataObj->GetAsFloat(LaneCol, row); break;
      case UGA_Corvallis /* 12 */ :  rtnval = pDataObj->GetAsFloat(BentonCol, row); break;
      case UGA_CottageGrove /* 13 */ :  rtnval = pDataObj->GetAsFloat(LaneCol, row); break;
      case UGA_Creswell /* 14 */ :  rtnval = pDataObj->GetAsFloat(LaneCol, row); break;
      case UGA_Dallas /* 15 */ : rtnval = pDataObj->GetAsFloat(PolkCol, row); break;
      case UGA_Dayton /* 16 */ : rtnval = pDataObj->GetAsFloat(YamhillCol, row); break;
      case UGA_Detroit /* 17 */ : rtnval = pDataObj->GetAsFloat(MarionCol, row); break;
      case UGA_Donald /* 18 */ : rtnval = pDataObj->GetAsFloat(MarionCol, row); break;
      case UGA_Dundee /* 19 */ : rtnval = pDataObj->GetAsFloat(YamhillCol, row); break;
      case UGA_Estacada /* 20 */ : rtnval = pDataObj->GetAsFloat(ClackamasCol, row); break;
      case UGA_UnknownLaneCo /* 21 */ : rtnval = pDataObj->GetAsFloat(LaneCol, row); break;
      case UGA_EugeneSpringfield /* 22 */ : rtnval = pDataObj->GetAsFloat(LaneCol, row); break;
      case UGA_FallsCity /* 23 */ : rtnval = pDataObj->GetAsFloat(PolkCol, row); break;
      case UGA_ForestGrove /* 24 */ : rtnval = pDataObj->GetAsFloat(WashingtonCol, row); break;
      case UGA_Gaston /* 25 */ : rtnval = pDataObj->GetAsFloat(WashingtonCol, row)*0.580f
            + pDataObj->GetAsFloat(YamhillCol, row)*0.420f; break;
      case UGA_Gates /* 26 */ : rtnval = pDataObj->GetAsFloat(LinnCol, row)*0.080f
            + pDataObj->GetAsFloat(MarionCol, row)*0.92f; break;
      case UGA_Gervais /* 27 */ : rtnval = pDataObj->GetAsFloat(MarionCol, row); break;
      case UGA_Halsey /* 28 */ : rtnval = pDataObj->GetAsFloat(LinnCol, row); break;
      case UGA_Harrisburg /* 29 */ : rtnval = pDataObj->GetAsFloat(LaneCol, row)*0.010f
            + pDataObj->GetAsFloat(LinnCol, row)*0.990f; break;
      case UGA_Hubbard /* 30 */ : rtnval = pDataObj->GetAsFloat(MarionCol, row); break;
      case UGA_Idanha /* 31 */ : rtnval = pDataObj->GetAsFloat(LinnCol, row)*0.131f
            + pDataObj->GetAsFloat(MarionCol, row)*0.869f; break;
      case UGA_Independence /* 32 */ : rtnval = pDataObj->GetAsFloat(PolkCol, row)*0.979f
            + pDataObj->GetAsFloat(MarionCol, row)*0.021f; break;
      case UGA_Jefferson /* 33 */ : rtnval = pDataObj->GetAsFloat(MarionCol, row); break;
      case UGA_JunctionCity /* 34 */ : rtnval = pDataObj->GetAsFloat(LaneCol, row); break;
      case UGA_Lafayette /* 35 */ : rtnval = pDataObj->GetAsFloat(YamhillCol, row); break;
      case UGA_Lebanon /* 36 */ : rtnval = pDataObj->GetAsFloat(LinnCol, row); break;
      case UGA_Lowell /* 37 */ : rtnval = pDataObj->GetAsFloat(LaneCol, row); break;
      case UGA_Lyons /* 38 */ : rtnval = pDataObj->GetAsFloat(LinnCol, row); break;
      case UGA_McMinnville /* 39 */ : rtnval = pDataObj->GetAsFloat(YamhillCol, row); break;
      case UGA_Metro /* 40 */ : rtnval = pDataObj->GetAsFloat(MetroCol, row); break;
      case UGA_MillCity /* 41 */ : rtnval = pDataObj->GetAsFloat(LinnCol, row)*0.821f
            + pDataObj->GetAsFloat(MarionCol, row)*0.179f; break;
      case UGA_Millersburg /* 42 */ : rtnval = pDataObj->GetAsFloat(LinnCol, row)*0.942f
            + pDataObj->GetAsFloat(BentonCol, row)*0.058f; break;
      case UGA_Molalla /* 43 */ : rtnval = pDataObj->GetAsFloat(ClackamasCol, row); break;
      case UGA_Monmouth /* 44 */ : rtnval = pDataObj->GetAsFloat(PolkCol, row); break;
      case UGA_Monroe /* 45 */ : rtnval = pDataObj->GetAsFloat(BentonCol, row); break;
      case UGA_MtAngel /* 46 */ : rtnval = pDataObj->GetAsFloat(MarionCol, row); break;
      case UGA_Newberg /* 47 */ : rtnval = pDataObj->GetAsFloat(YamhillCol, row); break;
      case UGA_NorthPlains /* 48 */ : rtnval = pDataObj->GetAsFloat(WashingtonCol, row); break;
      case UGA_Oakridge /* 49 */ : rtnval = pDataObj->GetAsFloat(LaneCol, row); break;
//      case UGA_Philomath /* 50 */ : rtnval = pDataObj->GetAsFloat(BentonCol, row); break;
      case UGA_SalemKeizer /* 51 */ : rtnval = pDataObj->GetAsFloat(MarionCol, row)*0.868f
            + pDataObj->GetAsFloat(PolkCol, row)*0.132f; break;
      case UGA_Sandy /* 52 */ : rtnval = pDataObj->GetAsFloat(ClackamasCol, row); break;
      case 53: rtnval = 0; break;
      case UGA_Scio /* 54 */ : rtnval = pDataObj->GetAsFloat(LinnCol, row); break;
      case UGA_ScottsMills /* 55 */ : rtnval = pDataObj->GetAsFloat(ClackamasCol, row)*0.009f
            + pDataObj->GetAsFloat(MarionCol, row)*0.991f; break;
      case UGA_Sheridan /* 56 */ : rtnval = pDataObj->GetAsFloat(YamhillCol, row); break;
//      case UGA_Sherwood /* 57 */ : rtnval = pDataObj->GetAsFloat(WashingtonCol, row); break;
      case UGA_Silverton /* 58 */ : rtnval = pDataObj->GetAsFloat(MarionCol, row); break;
      case UGA_Sodaville /* 59 */ : rtnval = pDataObj->GetAsFloat(LinnCol, row); break;
      case 60: rtnval = 0.; break;
      case UGA_StPaul /* 61 */ : rtnval = pDataObj->GetAsFloat(MarionCol, row); break;
      case UGA_Stayton /* 62 */ : rtnval = pDataObj->GetAsFloat(LinnCol, row)*0.004f
            + pDataObj->GetAsFloat(MarionCol, row)*0.996f; break;
//      case UGA_Sublimity /* 63 */ : rtnval = pDataObj->GetAsFloat(MarionCol, row); break;
      case UGA_SweetHome /* 64 */ : rtnval = pDataObj->GetAsFloat(LinnCol, row); break;
      case UGA_Tangent /* 65 */ : rtnval = pDataObj->GetAsFloat(LinnCol, row); break;
      case UGA_Turner /* 66 */ : rtnval = pDataObj->GetAsFloat(MarionCol, row); break;
      case UGA_Veneta /* 67 */ : rtnval = pDataObj->GetAsFloat(LaneCol, row); break;
      case UGA_Waterloo /* 68 */ : rtnval = pDataObj->GetAsFloat(LinnCol, row); break;
      case UGA_Westfir /* 69 */ : rtnval = pDataObj->GetAsFloat(LaneCol, row); break;
      case UGA_Willamina /* 70 */ : rtnval = pDataObj->GetAsFloat(PolkCol, row)*0.296f
            + pDataObj->GetAsFloat(YamhillCol, row)*0.704f; break;
      case UGA_Woodburn /* 71 */ : rtnval = pDataObj->GetAsFloat(MarionCol, row); break;
      case UGA_Yamhill /* 72 */ :  rtnval = pDataObj->GetAsFloat(YamhillCol, row); break;
      
      default: rtnval = 0.; break;
      }
   
   return rtnval;
   } // end of Get4UGB()


bool APs::ConfirmCols(FDataObj * pDataObj)
   {
/*
enum COUNTYcolumn
   {
   BentonCol=1,
   ClackamasCol=2,
   ColumbiaCol=3,
   LaneCol=4,
   LinnCol=5,
   MarionCol=6,
   MultnomahCol=7,
   PolkCol=8,
   WashingtonCol=9,
   YamhillCol=10,
   MetroCol=11
   }
*/
   bool ok;
   ok = pDataObj->GetCol(_T("Benton"))==BentonCol
      && pDataObj->GetCol(_T("Clackamas"))==ClackamasCol
      && pDataObj->GetCol(_T("Columbia"))==ColumbiaCol
      && pDataObj->GetCol(_T("Lane"))==LaneCol
      && pDataObj->GetCol(_T("Linn"))==LinnCol
      && pDataObj->GetCol(_T("Marion"))==MarionCol
      && pDataObj->GetCol(_T("Multnomah"))==MultnomahCol
      && pDataObj->GetCol(_T("Polk"))==PolkCol
      && pDataObj->GetCol(_T("Washington"))==WashingtonCol
      && pDataObj->GetCol(_T("Yamhill"))==YamhillCol
      && pDataObj->GetCol(_T("Portland Metro"))==MetroCol;

   int firstYear = pDataObj->GetAsInt(0, 0);
   ok &= 1900<=firstYear && firstYear<=2010;

   int numRows = pDataObj->GetRowCount();
   int lastYear = firstYear + numRows - 1;
   for (int year = firstYear; year<=lastYear;  year++) 
         ok &= pDataObj->GetAsInt(0, year - firstYear)==year;

   return(ok); 
   } // end of ConfirmCols()


bool APs::InitUX(EnvContext *pContext)
   {
   m_UXoutVars.Clear();
   m_UXoutVars.SetName(_T("UGAdevelopedFractions"));
   int active_UGAs = 0;
   for (int ugb=1; ugb<=MAX_UGA_NDX; ugb++) if (m_UXugaArray[ugb].id==ugb && m_UXugaArray[ugb].display_flag!=0) active_UGAs++;
   m_UXoutVars.AppendCols(active_UGAs+1);
   m_UXoutVars.SetLabel(0, "Time");
   int colNdx = 1;
   for (int ugb=1; ugb<=MAX_UGA_NDX; ugb++)  if (m_UXugaArray[ugb].id==ugb && m_UXugaArray[ugb].display_flag!=0)
      {
      CString column_name;
      column_name.Format("%d %s", ugb, (LPCTSTR) m_UXugaArray[ugb].name);
      m_UXoutVars.SetLabel(colNdx, column_name);
      colNdx++;
      }

   return(true);
   } // end InitUX()


void APs::UXsaveOutVars(EnvContext *pContext)
   {
   int nRows = m_UXoutVars.GetRowCount();
   int rowNdx = pContext->yearOfRun<0 ? 0 : pContext->yearOfRun;
   while (nRows<=rowNdx) 
      {
      nRows = m_UXoutVars.AppendRows(1);
      if (m_UXtestMode>0)
         {
         CString msg;
         msg.Format("Urban Expansion: nRows, rowNdx, yearOfRun, currentYear = %d, %d, %d, %d", nRows, rowNdx, pContext->yearOfRun, pContext->currentYear);
         Report::LogMsg(msg);
         }
      }
   m_UXoutVars.Set(0, rowNdx, (float)pContext->currentYear);

   int colNdx = 1;
   for (int ugb=1; ugb<=MAX_UGA_NDX; ugb++) if (m_UXugaArray[ugb].id==ugb && m_UXugaArray[ugb].display_flag!=0)
      {
      m_UXoutVars.Set(colNdx, rowNdx, m_UGAdevelopedFrac[ugb]);
      colNdx++;
      } // end of loop on ugb
   } // end UXsaveOutVars()


bool APs::InitRunUX(EnvContext *pContext)
   {
	CString msg;
	bool err;
	m_records_UGAthreshold = m_UGAthreshold_table.ReadAscii(m_UGAthreshold_file, ',', TRUE);

	// Read UGA threshold data.
	err = m_records_UGAthreshold <= 0;
	if (!err)
	{
		// m_colUGAthreshold_UGB is actually the UGB id
		m_colUGAthreshold_UGB = m_UGAthreshold_table.GetCol("UGB"); err |= m_colUGAthreshold_UGB < 0; // here return the column idx of corresponding column label
		m_colUGAthreshold_UGB_NAME = m_UGAthreshold_table.GetCol("UGB_NAME"); err |= m_colUGAthreshold_UGB_NAME < 0;
		m_colUGAthreshold_ref_thresh = m_UGAthreshold_table.GetCol("ref_thresh"); err |= m_colUGAthreshold_ref_thresh < 0;
		m_colUGAthreshold_alt_thresh1 = m_UGAthreshold_table.GetCol("alt_thresh1"); err |= m_colUGAthreshold_alt_thresh1 < 0;
	}
	if (err)
	{
		if (m_records_UGAthreshold <= 0) msg.Format("APs_UGA Expansion: missing or empty UGA_threshold_pct_file\n"
			"m_records_UGAthreshold = %d\n", m_records_UGAthreshold);
		else msg.Format("APs_UGA Expansion - One or more missing columns in the UGA_threshold_pct_file.\n"
			"m_colUGAthreshold_UGB, m_colUGAthreshold_UGB_NAME, m_colUGAthreshold_ref_thresh, m_colUGAthreshold_alt_thresh1 = %d, %d, %d, %d",
			m_colUGAthreshold_UGB, m_colUGAthreshold_UGB_NAME, m_colUGAthreshold_ref_thresh, m_colUGAthreshold_alt_thresh1);
		Report::ErrorMsg(msg);
		return false;
	}
	
	// here based on the UGA scenario index we read different column of threshold data
   if (m_currentUGAScenarioIndex == -1) m_currentUGAScenarioIndex = 0;

	int UGAthresh_col_idx; // this int value correspond to which column of UGA threshold data we're read in now
	if (m_currentUGAScenarioIndex == 0) UGAthresh_col_idx = m_colUGAthreshold_ref_thresh; // referece scenario	
	else if (m_currentUGAScenarioIndex == 1) UGAthresh_col_idx = m_colUGAthreshold_alt_thresh1;  // altUGA scenario 1
	else UGAthresh_col_idx = -1;

	err = UGAthresh_col_idx <= 0;
	if (!err)
	{
		msg.Format("APs::InitRunUX(): pContext->id, m_currentUGAScenarioIndex, UGAthresh_col_idx = %d, %d, %d",
			pContext->id, m_currentUGAScenarioIndex, UGAthresh_col_idx);
		Report::LogMsg(msg);
	}
	if (err)
	{
		msg.Format("APs_UGA Expansion: Current UGA Expansion scenario doesn't have corresponding threshold column in UGA_threshold.csv file.\n");
		Report::LogMsg(msg);
		return false;
	}

	// give threshold values to all UGAs
	for (int iRow = 0; iRow < m_records_UGAthreshold; iRow++)
	{
		for (int ugb = 1; ugb <= MAX_UGA_NDX; ugb++)
		{
			if (m_UXugaArray[ugb].id == m_UGAthreshold_table.GetAsInt(m_colUGAthreshold_UGB, iRow))
				m_UXugaArray[ugb].threshold_pct = m_UGAthreshold_table.GetAsFloat(UGAthresh_col_idx,iRow);
		}
	}

   m_UXoutVars.ClearRows();
   UXcalcDevelopedFracs(pContext);
   UXsaveOutVars(pContext);

   return(true);
   } // end InitRunUX()


